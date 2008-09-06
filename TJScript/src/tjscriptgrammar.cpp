#include "../include/internal/tjscript.h"

#pragma warning(push)
#pragma warning(disable: 4800 4503 4244) // small thingy in Spirit header file, decorated names too long
#pragma inline_depth(255)
#pragma inline_recursion(on)
#undef WIN32_LEAN_AND_MEAN // Defined in file_iterator.ipp
#include <boost/spirit.hpp>
#include <boost/spirit/core.hpp>
#include <stack>

using namespace tj::shared;
using namespace tj::script;
using namespace boost::spirit;

namespace tj {
	namespace script {
		namespace parser {
			class ScriptGrammar;

			inline void ReplaceAll(std::wstring& data, const std::wstring& find, const std::wstring& replace) {
				std::wstring::size_type it = data.find(find);
				while(it!=std::wstring::npos) {
					data.replace(it, find.size(), replace);
					it = data.find(find, it);
				}
			}

			struct ScriptWriteDouble {
				ScriptWriteDouble(ScriptGrammar const* gram);

				inline void operator()(char const* start, char const* end) const {
					std::istringstream is;
					double value;
					is >> value;

					LiteralIdentifier li = _stack->Top()->StoreLiteral(GC::Hold(new ScriptDouble(value)));
					_stack->Top()->Add<LiteralIdentifier>(li);
				}

				inline void operator()(const double& value) const {		
					LiteralIdentifier li = _stack->Top()->StoreLiteral(GC::Hold(new ScriptDouble(value)));
					_stack->Top()->Add<LiteralIdentifier>(li);
				}

				mutable ref<ScriptletStack> _stack;
			};

			struct ScriptWriteDoubleValue {
				ScriptWriteDoubleValue(ScriptGrammar const* gram, double val);

				template<typename T> inline void operator()(T,T) const {	
					LiteralIdentifier li = _stack->Top()->StoreLiteral(GC::Hold(new ScriptDouble(_value)));
					_stack->Top()->Add<LiteralIdentifier>(li);
				}

				mutable ref<ScriptletStack> _stack;
				double _value;
			};

			// Ints can only be written by the parser itself, not from the file
			struct ScriptWriteInt {
				int _value;

				ScriptWriteInt(ScriptGrammar const* gram, int i);

				template<typename T> inline void operator()(T,T) const {
					LiteralIdentifier li = _stack->Top()->StoreLiteral(GC::Hold(new ScriptInt(_value)));
					_stack->Top()->Add<LiteralIdentifier>(li);
				}

				template<typename T> inline void operator()(const T&) const {		
					LiteralIdentifier li = _stack->Top()->StoreLiteral(GC::Hold(new ScripInt(_value)));
					_stack->Top()->Add<LiteralIdentifier>(li);
				}

				mutable ref<ScriptletStack> _stack;
			};

			struct ScriptWriteString {
				ScriptWriteString(ScriptGrammar const* gram);

				template<typename T> inline void operator()(const T start, const T end) const {
					std::wstring value(start,end);
					ReplaceAll(value, L"\\\"", L"\"");
					ReplaceAll(value, L"\\r", L"\r");
					ReplaceAll(value, L"\\n", L"\n");
					ReplaceAll(value, L"\\t", L"\t");

					int li = _stack->Top()->StoreLiteral(GC::Hold(new ScriptString(value)));
					_stack->Top()->Add<LiteralIdentifier>(li);
				}

				template<typename T> inline void operator()(const T v) const {	
					std::wstring value(v);
					ReplaceAll(value, L"\\\"", L"\"");
					ReplaceAll(value, L"\\r", L"\r");
					ReplaceAll(value, L"\\n", L"\n");
					ReplaceAll(value, L"\\t", L"\t");

					int li = _stack->Top()->StoreLiteral(GC::Hold(new ScriptString(value)));
					_stack->Top()->Add<LiteralIdentifier>(li);
				}

				mutable ref<ScriptletStack> _stack;
			};

			struct ScriptWriteHash {
				ScriptWriteHash(ScriptGrammar const* gram);

				template<typename T> inline void operator()(const T start, const T end) const {
					std::wstring value(start,end);
					
					Hash h;
					_stack->Top()->Add<unsigned int>(h.Calculate(value));
				}

				template<typename T> inline void operator()(const T v) const {	
					std::wstring value(v);

					Hash h;
					_stack->Top()->Add<unsigned int>(h.Calculate(value));
				}

				mutable ref<ScriptletStack> _stack;
			};

			struct ScriptLog {
				inline ScriptLog(std::wstring msg) {
					_msg = msg;
				}

				inline void operator()(char const* str, char const* end) const {
					Log::Write(L"TJScript/Parser", _msg+L": "+Wcs(std::string(str,end)));
				}

				template<typename T> inline void operator()(T x) const {
					Log::Write(L"TJScript/Parser", _msg);
				}

				mutable std::wstring _msg;
			};


			struct ScriptPushScriptlet {
				inline ScriptPushScriptlet(ScriptGrammar const* gram, ScriptletType t) {
					_grammar = gram;
					_function = t;
				}

				template<typename T> inline void operator()(T str, T end) const {
					ref<Scriptlet> s = _grammar->_script->CreateScriptlet(_function);
					_grammar->_stack->Push(s,_grammar->_script->GetScriptletIndex(s));
				}

				template<typename T> inline void operator()(T val) const {
					ref<Scriptlet> s = _grammar->_script->CreateScriptlet(_function);
					_grammar->_stack->Push(s,_grammar->_script->GetScriptletIndex(s));
				}

				mutable ScriptletType _function;
				mutable ScriptGrammar const* _grammar;
			};

			struct ScriptLoadScriptlet {
				inline ScriptLoadScriptlet(ScriptGrammar const* g) {
					_grammar = g;
				}

				template<typename T> inline void operator()(T str, T end) const {
					int idx = _grammar->_stack->GetCurrentIndex();
					ref<Scriptlet> dlg = _grammar->_stack->Pop();

					_grammar->_stack->Top()->AddInstruction(Ops::OpLoadScriptlet);
					_grammar->_stack->Top()->Add<int>(idx);
				}

				mutable ScriptGrammar const* _grammar;
			};

			struct ScriptIf {
				inline ScriptIf(ScriptGrammar const* g) {
					_grammar = g;
				}

				template<typename T> inline void operator()(T str, T end) const {
					ref<Scriptlet> s = _grammar->_stack->Pop();
					int idx = _grammar->_script->GetScriptletIndex(s);
					ref<Scriptlet> main = _grammar->_stack->Top();

					main->AddInstruction(Ops::OpBranchIf);
					main->Add<int>(idx);
				}
				
				mutable ScriptGrammar const* _grammar;
			};

			struct ScriptIterate {
				inline ScriptIterate(ScriptGrammar const* grammar) {
					_grammar = grammar;
				}

				template<typename T> inline void operator()(T str, T end) const {
					ref<Scriptlet> s = _grammar->_stack->Pop();
					int idx = _grammar->_script->GetScriptletIndex(s);
					ref<Scriptlet> main = _grammar->_stack->Top();
					
					main->AddInstruction(Ops::OpIterate);
					main->Add<int>(idx);
				}
				
				mutable ScriptGrammar const* _grammar;
			};

			struct ScriptBeginDelegate {
				inline ScriptBeginDelegate(ScriptGrammar const* g) {
					_grammar = g;
				}

				void operator()(char x) const;

				mutable ScriptGrammar const* _grammar;
			};

			struct ScriptEndDelegate {
				inline ScriptEndDelegate(ScriptGrammar const* g) {
					_grammar = g;
				}

				void operator()(char x) const;

				mutable ScriptGrammar const* _grammar;
			};

			/* grammar */
			distinct_parser<> keyword_p("a-zA-Z0-9_");

			class ScriptGrammar : public grammar<ScriptGrammar> {
				public:
					ScriptGrammar(ref<CompiledScript> script, ref<ScriptContext> context) {
						_script = script;
						_stack = GC::Hold(new ScriptletStack());
						ref<Scriptlet> s = _script->CreateScriptlet(ScriptletFunction);
						_stack->Push(s, _script->GetScriptletIndex(s));
						_context = context;
					}

					virtual ~ScriptGrammar() {
					}

					template <typename ScannerT> struct definition {
						definition(ScriptGrammar const& self) {
							/** String/integer literals or result from nested stuff **/
							comment = 
								comment_p("/*","*/") | comment_p("//") | space_p;

							stringValue =
								lexeme_d[confix_p('"', ((*c_escape_ch_p)[ScriptStringLiteral(&self)]), '"')];

							boolValue =
								lexeme_d[str_p("true")[ScriptInstruction(&self, Ops::OpPushTrue)] | str_p("false")[ScriptInstruction(&self, Ops::OpPushFalse)]];

							doubleValue = 
								  lexeme_d[str_p("NaN")[ScriptInstruction(&self, Ops::OpPushDouble)][ScriptWriteDoubleValue(&self, std::numeric_limits<double>::quiet_NaN())]]
								| lexeme_d[str_p("Inf")[ScriptInstruction(&self, Ops::OpPushDouble)][ScriptWriteDoubleValue(&self, std::numeric_limits<double>::infinity())]]
								| real_p[ScriptInstruction(&self, Ops::OpPushDouble)][ScriptWriteDouble(&self)];

							nullValue =
								lexeme_d[str_p("null")[ScriptInstruction(&self, Ops::OpPushNull)]];

							value = 
								stringValue | doubleValue | boolValue | nullValue;

							/** Variable names etc **/
							identifier = 
								typeIdentifier | rawIdentifier[ScriptInstruction(&self, Ops::OpPushString)][ScriptWriteString(&self)];

							typeIdentifier = 
								ch_p('<') >> rawIdentifier[ScriptInstruction(&self, Ops::OpPushString)][ScriptWriteString(&self)][ScriptInstruction(&self,Ops::OpType)] >> ch_p('>');

							rawIdentifier = 
								lexeme_d[(alpha_p >> *(alnum_p|ch_p('_')))];

							qualifiedIdentifier = 
								lexeme_d[(alpha_p >> *(alnum_p|ch_p('_')|ch_p(':')>>ch_p(':')))[ScriptInstruction(&self, Ops::OpPushString)][ScriptWriteString(&self)]];

							declaredParameter =
								rawIdentifier;

							breakStatement = 
								keyword_p("break")[ScriptInstruction(&self, Ops::OpBreak)];

							keyValuePair = 
								eps_p(lexeme_d[(alpha_p >> *(alnum_p|ch_p('_')))] >> (ch_p('=')|ch_p(':')) >> (~ch_p('='))) 
								>> identifier >> (ch_p('=')|ch_p(':')) >> expression;

							parameterList = 
								( (keyValuePair[ScriptInstruction(&self, Ops::OpParameter)]|expression[ScriptInstruction(&self, Ops::OpNamelessParameter)]) % ch_p(','));

							assignment = 
								assignmentWithVar | assignmentWithoutVar;

							assignmentWithVar = 
								lexeme_d[keyword_p("var")] >> identifier >> 
								  ((ch_p('=') >> expression)[ScriptInstruction(&self, Ops::OpSave)] |
								  eps_p[ScriptInstruction(&self, Ops::OpPushNull)][ScriptInstruction(&self, Ops::OpSave)]);

							assignmentWithoutVar =
								eps_p(lexeme_d[(alpha_p >> *(alnum_p|ch_p('_')))] >> ch_p('=')) 
								>> identifier >> ch_p('=') >> expression[ScriptInstruction(&self, Ops::OpSave)];

							/* identifier is pushed twice, once for OpCallGlobal and once for OpSave */
							incrementByOperator = 
								eps_p(lexeme_d[ ((alpha_p >> *(alnum_p|ch_p('_')))) ] >> keyword_p("+="))
								>> identifier[ScriptInstruction(&self, Ops::OpPushString)][ScriptWriteString(&self)][ScriptInstruction(&self, Ops::OpCallGlobal)] >> keyword_p("+=") >> expression[ScriptInstruction(&self, Ops::OpAdd)][ScriptInstruction(&self, Ops::OpSave)];

							decrementByOperator = 
								eps_p(lexeme_d[ ((alpha_p >> *(alnum_p|ch_p('_')))) ] >> keyword_p("-="))
								>> identifier[ScriptInstruction(&self, Ops::OpPushString)][ScriptWriteString(&self)][ScriptInstruction(&self, Ops::OpCallGlobal)] >> keyword_p("-=") >> expression[ScriptInstruction(&self, Ops::OpSub)][ScriptInstruction(&self, Ops::OpSave)];

							incrementOneOperator = 
								eps_p(lexeme_d[(alpha_p >> *(alnum_p|ch_p('_')))] >> keyword_p("++"))
								>> identifier[ScriptInstruction(&self, Ops::OpPushString)][ScriptWriteString(&self)][ScriptInstruction(&self, Ops::OpCallGlobal)] >> keyword_p("++")[ScriptInstruction(&self, Ops::OpPushInt)][ScriptWriteInt(&self,1)][ScriptInstruction(&self, Ops::OpAdd)][ScriptInstruction(&self, Ops::OpSave)];

							decrementOneOperator = 
								eps_p(lexeme_d[(alpha_p >> *(alnum_p|ch_p('_')))] >> keyword_p("--"))
								>> identifier[ScriptInstruction(&self, Ops::OpPushString)][ScriptWriteString(&self)][ScriptInstruction(&self, Ops::OpCallGlobal)] >> keyword_p("--")[ScriptInstruction(&self, Ops::OpPushInt)][ScriptWriteInt(&self,1)][ScriptInstruction(&self, Ops::OpSub)][ScriptInstruction(&self, Ops::OpSave)];

							methodCall =
								(identifier >> !(ch_p('(')[ScriptInstruction(&self, Ops::OpPushParameter)] >> !parameterList >> ')'));

							methodCallConstruct = 
								(typeIdentifier | methodCall[ScriptInstruction(&self, Ops::OpCallGlobal)]) >> followingMethodCall >> !followingAssignment;


							followingMethodCall = 
								*indexOperator >> !(ch_p('.') >> ((followingAssignment |(methodCall[ScriptInstruction(&self, Ops::OpCall)] >> *(indexOperator))) % ch_p('.')));

							followingAssignment =
								eps_p(lexeme_d[ ((alpha_p >> *(alnum_p|ch_p('_'))))] >> ch_p('=')) >> identifier >> ch_p('=') >> expression[ScriptInstruction(&self, Ops::OpSetField)];

							/* Operators */
							equalsOperator = 
								(str_p("==") >> expression)[ScriptInstruction(&self, Ops::OpEquals)];

							orOperator = 
								(str_p("||") >> expression)[ScriptInstruction(&self, Ops::OpOr)];

							andOperator =
								(str_p("&&") >> expression)[ScriptInstruction(&self, Ops::OpAnd)];

							xorOperator = 
								(str_p("^^") >> expression)[ScriptInstruction(&self, Ops::OpXor)];

							notEqualsOperator =
								(str_p("!=") >> expression)[ScriptInstruction(&self, Ops::OpEquals)][ScriptInstruction(&self, Ops::OpNegate)];

							plusOperator =
								(ch_p('+') >> term)[ScriptInstruction(&self, Ops::OpAdd)];

							minOperator =
								(ch_p('-') >> term)[ScriptInstruction(&self, Ops::OpSub)];

							divOperator =
								(ch_p('/') >> factor)[ScriptInstruction(&self, Ops::OpDiv)];

							mulOperator =
								(ch_p('*') >> factor)[ScriptInstruction(&self, Ops::OpMul)];

							gtOperator = 
								(ch_p('>') >> expression)[ScriptInstruction(&self, Ops::OpGreaterThan)];

							ltOperator = 
								(ch_p('<') >> expression)[ScriptInstruction(&self, Ops::OpLessThan)];

							/* If/else */
							ifConstruct =
								keyword_p("if") >> ch_p('(') >> expression >> ch_p(')') >> block[ScriptIf(&self)] >> 
								!(keyword_p("else")[ScriptInstruction(&self, Ops::OpNegate)] >> block[ScriptIf(&self)]) >>
								eps_p[ScriptInstruction(&self, Ops::OpPop)];

							// Something that returns a value (methodCall must be last in this rule because of the
							// eps_p, which otherwise pushes a global even when the rest of methodCall doesn't match
							expression = 
								function | (term >> *(plusOperator | minOperator | gtOperator | ltOperator | equalsOperator | notEqualsOperator | orOperator | andOperator | xorOperator));

							// declared with var something = function() {..}
							function =
								keyword_p("function") >> ch_p('(') >> !(declaredParameter % ch_p(',')) >> ch_p(')') >> blockInFunction[ScriptLoadScriptlet(&self)];

							// declared with function something() {...}
							functionConstruct =
								(keyword_p("function") 
								>> identifier >> ch_p('(') >> !(declaredParameter % ch_p(',')) >> ch_p(')')
								>> blockInFunction[ScriptLoadScriptlet(&self)])[ScriptInstruction(&self, Ops::OpSave)];

							term = 
								factor >> *(mulOperator|divOperator);

							indexOperator = 
								ch_p('[') >> expression >> ch_p(']')[ScriptInstruction(&self, Ops::OpIndex)];

							factor =
								value | (arrayConstruct | delegateConstruct | newConstruct | negatedFactor | (ch_p('(') >> expression >> ch_p(')')) | methodCallConstruct);

							negatedFactor = 
								((ch_p('!')|ch_p('-')) >> factor)[ScriptInstruction(&self, Ops::OpNegate)];


							/* after methodCallConstruct completed, add a OpPop so there's nothing left on the stack; otherwise
								for(var i: new Range(0,100)) { log(i); } will work for the first iteration, but will fail the next,
								because the null returned from log is still on the stack where it shouldn't be.
							*/
							statement = 
								returnConstruct | functionConstruct | breakStatement | incrementOneOperator | decrementOneOperator | decrementByOperator | incrementByOperator | assignment | methodCallConstruct[ScriptInstruction(&self, Ops::OpPop)];

							blockInFunction =
								ch_p('{')[ScriptPushScriptlet(&self,ScriptletFunction)] >> *((blockConstruct >> *eol_p)|comment) >> ch_p('}');

							blockInFor =
								ch_p('{')[ScriptPushScriptlet(&self,ScriptletLoop)] >> *((blockConstruct >> *eol_p)|comment) >> ch_p('}');

							block =
								ch_p('{')[ScriptPushScriptlet(&self,ScriptletAny)] >> *((blockConstruct >> *eol_p)|comment) >> ch_p('}');

							blockConstruct =
								ifConstruct | forConstruct | (statement >> !ch_p(';'));

							arrayConstruct = 
								eps_p('[') >> (ch_p('[')[ScriptInstruction(&self, Ops::OpPushArray)] >> !(expression[ScriptInstruction(&self, Ops::OpAddToArray)] % ch_p(',')) >> ch_p(']'));

							returnConstruct =
								(keyword_p("return") >> ((expression[ScriptInstruction(&self, Ops::OpReturnValue)]) | eps_p[ScriptInstruction(&self, Ops::OpReturn)]));

							forConstruct =
								((keyword_p("for") >> ch_p('(') >> keyword_p("var") >> identifier >> ch_p(':') >> expression >> ch_p(')')) >> blockInFor) [ScriptIterate(&self)];

							newConstruct = 
								((keyword_p("new") >> qualifiedIdentifier >> !(ch_p('(')[ScriptInstruction(&self, Ops::OpPushParameter)] >> !parameterList >> ')'))[ScriptInstruction(&self, Ops::OpNew)]) >> followingMethodCall;

							delegateConstruct =
								keyword_p("delegate") >> ch_p('{')[ScriptBeginDelegate(&self)] >> scriptBody >> ch_p('}')[ScriptEndDelegate(&self)] >> followingMethodCall;

							scriptBody = 
								*((blockConstruct >> *eol_p)|comment);

							script = 
								scriptBody >> end_p;
						}

						// values
						rule<ScannerT> stringValue, boolValue, doubleValue, nullValue;

						// constructs
						rule<ScannerT> scriptBody, block, arrayConstruct, function, functionConstruct, rawIdentifier, ifConstruct, comment, assignment, assignmentWithVar, assignmentWithoutVar, followingAssignment, value, identifier, declaredParameter, keyValuePair, parameterList, methodCall, expression, statement, blockConstruct, blockInFunction, blockInFor, script, returnConstruct, breakStatement, forConstruct, newConstruct, methodCallConstruct, followingMethodCall, typeIdentifier, delegateConstruct, qualifiedIdentifier;
						
						// operators
						rule<ScannerT> term, factor, negatedFactor, indexOperator, equalsOperator, notEqualsOperator, plusOperator, minOperator, divOperator, mulOperator, orOperator, andOperator, xorOperator, gtOperator, ltOperator, incrementByOperator, decrementByOperator, incrementOneOperator, decrementOneOperator;

						rule<ScannerT> const& start() const { 
							return script;
						}
					};

					mutable ref<CompiledScript> _script;
					mutable std::deque< ref<CompiledScript> > _delegateStack;
					mutable ref<ScriptletStack> _stack;
					mutable ref<ScriptContext> _context;
			};

			void ScriptBeginDelegate::operator()(char x) const {
				ref<CompiledScript> dlg = GC::Hold(new CompiledScript(0));
				_grammar->_delegateStack.push_back(_grammar->_script);
				_grammar->_script = dlg;

				// create main scriptlet
				ref<Scriptlet> s = dlg->CreateScriptlet(ScriptletFunction);
				_grammar->_stack->Push(s, dlg->GetScriptletIndex(s));
			}

			void ScriptEndDelegate::operator()(char x) const {
				ref<CompiledScript> dlg = _grammar->_script;
				_grammar->_script = *(_grammar->_delegateStack.rbegin());
				_grammar->_delegateStack.pop_back();
				_grammar->_stack->Pop();
				
				ref<Scriptlet> current = _grammar->_stack->Top();
				ref<ScriptDelegate> scriptDelegate = GC::Hold(new ScriptDelegate(dlg, _grammar->_context));
				LiteralIdentifier li = current->StoreLiteral(scriptDelegate);
				current->AddInstruction(Ops::OpPushDelegate);
				current->Add<LiteralIdentifier>(li);
			}

			// Writes an instruction to the code
			struct ScriptInstruction {
				inline ScriptInstruction(ScriptGrammar const* gram, Ops::Codes op) {
					_stack = gram->_stack;
					_op = op;
				}

				template<typename T> inline void operator()(T,T) const {
					_stack->Top()->AddInstruction(_op);
				}

				template<typename Q> inline void operator()(const Q& value) const {		
					_stack->Top()->AddInstruction(_op);
				}

				Ops::Codes _op;
				mutable ref<ScriptletStack> _stack;
			};

			struct ScriptStringLiteral {
				inline ScriptStringLiteral(ScriptGrammar const* gram) {
					_stack = gram->_stack;
				}

				template<typename T> inline void operator()(const T start,const T end) const {
					_stack->Top()->AddInstruction(Ops::OpPushString);
					std::wstring value(start,end);
					ReplaceAll(value, L"\\\"", L"\"");
					ReplaceAll(value, L"\\r", L"\r");
					ReplaceAll(value, L"\\n", L"\n");
					ReplaceAll(value, L"\\t", L"\t");

					LiteralIdentifier li = _stack->Top()->StoreLiteral(GC::Hold(new ScriptString(value)));
					_stack->Top()->Add<LiteralIdentifier>(li);
				}

				template<typename Q> inline void operator()(const Q v) const {		
					_stack->Top()->AddInstruction(Ops::OpPushString);
					std::wstring value(v);
					ReplaceAll(value, L"\\\"", L"\"");
					ReplaceAll(value, L"\\r", L"\r");
					ReplaceAll(value, L"\\n", L"\n");
					ReplaceAll(value, L"\\t", L"\t");

					LiteralIdentifier li = _stack->Top()->StoreLiteral(GC::Hold(new ScriptString(value)));
					_stack->Top()->Add<LiteralIdentifier>(li);
				}

				mutable ref<ScriptletStack> _stack;
			};

			ScriptWriteDouble::ScriptWriteDouble(const ScriptGrammar *gram) {
				_stack = gram->_stack;
			}

			ScriptWriteDoubleValue::ScriptWriteDoubleValue(ScriptGrammar const* gram, double val) {
				_stack = gram->_stack;
				_value = val;
			}

			ScriptWriteInt::ScriptWriteInt(const ScriptGrammar* gram, int i) {
				_stack = gram->_stack;
				_value = i;
			}

			ScriptWriteString::ScriptWriteString(const ScriptGrammar *gram) {
				_stack = gram->_stack;
			}
		}
	}
}

ref<CompiledScript> ScriptContext::Compile(std::wstring source) {
	ref<CompiledScript> script = GC::Hold(new CompiledScript(this));

	parser::ScriptGrammar sparser(script, this);
	parse_info<> info = parse(Mbs(source).c_str(), sparser, space_p);
	if(!info.full) {
		throw ParserException(std::wstring(L"Parsing stopped at")+Wcs(info.stop));
	}

	if(_optimize) {
		script->Optimize();
	}

	return script;
}

ref<CompiledScript> ScriptContext::CompileFile(std::wstring fn) {
	ref<CompiledScript> script = GC::Hold(new CompiledScript(this));

	std::string fns = Mbs(fn);
	parser::ScriptGrammar sparser(script, this);
	file_iterator<char> begin(fns.c_str());
	file_iterator<char> end = begin.make_end();

	parse_info< file_iterator<char> > info = parse(begin,end, sparser, space_p);
	if(!info.full) {
		throw ParserException(std::wstring(L"Parsing stopped"));
	}

	if(_optimize) {
		script->Optimize();
	}

	return script;
}