#include "../include/internal/tjscript.h"
using namespace tj::shared;
using namespace tj::script;

#pragma warning(push)
#pragma warning(disable: 4800 4503) // small thingy in Spirit header file, decorated names too long
#pragma inline_depth(255)
#pragma inline_recursion(on)
#include <boost/spirit.hpp>
#include <boost/spirit/core.hpp>
#include <stack>

using namespace tj::script;
using namespace tj::shared;
using namespace boost::spirit;

class ScriptGrammar;

// TODO: alle Script*** structs herschrijven zodat ze een ScriptGrammar* pakken en daar script/etc uithalen (getter methodes? inlined)
/*
var xx = delegate {
	if(true) {
		alert("T");
	}
	else {
		alert("F");
	}
};
*/

template<typename T> struct ScriptPush {
	ScriptPush(ScriptGrammar const* gram) {
		_stack = gram->_stack;
	}

	void operator()(char const* start, char const* end) const {
		std::istringstream is(std::string(start, end));
		T value;
		is >> value;
		
		ref<Op> op = GC::Hold(new OpPush(GC::Hold(new ScriptValue<T>(value))));
		_stack->Top()->Add(op);
	}

	void operator()(const T& value) const {		
		ref<Op> op = GC::Hold(new OpPush(GC::Hold(new ScriptValue<T>(value))));
		_stack->Top()->Add(op);
	}


	mutable ref<ScriptletStack> _stack;
};

template<typename T> struct ScriptPushValue {
	ScriptPushValue(ScriptGrammar const* grammar, T value) {
		_stack = grammar->_stack;
		_value = value;
	}

	template<typename X> void operator()(const X start, const X end) const {
		ref<Op> op = GC::Hold(new OpPush(GC::Hold(new ScriptValue<T>(_value))));
		_stack->Top()->Add(op);
	}

	mutable ref<ScriptletStack> _stack;
	mutable T _value;
};

struct ScriptPushNull {
	ScriptPushNull(ScriptGrammar const* grammar) {
		_grammar = grammar;
	}

	template<typename T> void operator()(const T start) const {
		ref<Op> op = GC::Hold(new OpPush(GC::Hold(new ScriptNull())));
		_stack->Top()->Add(op);
	}

	template<typename T> void operator()(const T start, const T end) const {
		ref<Op> op = GC::Hold(new OpPush(GC::Hold(new ScriptNull())));
		_grammar->_stack->Top()->Add(op);
	}

	mutable ScriptGrammar const* _grammar;
};

struct ScriptPushString {
	ScriptPushString(ScriptGrammar const* gram) {
		_grammar = gram;
	}

	template<typename T> void operator()(const T start, const T end) const {
		ref<Op> op = GC::Hold(new OpPush(GC::Hold(new ScriptValue<std::wstring>(std::wstring(start,end)))));
		_grammar->_stack->Top()->Add(op);
	}

	template<typename T> void operator()(const T str) const {
		ref<Op> op = GC::Hold(new OpPush(GC::Hold(new ScriptValue<std::wstring>(Wcs(std::string(str))))));
		_grammar->_stack->Top()->Add(op);
	}

	mutable ScriptGrammar const* _grammar;
};

struct ScriptCall {
	ScriptCall(ScriptGrammar const* g) {
		_grammar = g;
	}
	void operator()(char const* str, char const* end) const;
	mutable ScriptGrammar const* _grammar;
};

template<typename T> struct ScriptInstruction {
	ScriptInstruction(ScriptGrammar const* g) {
		_stack = g->_stack;
	}

	void operator()(...) const {
		_stack->Top()->Add(GC::Hold(new T()));
	}

	mutable ref<ScriptletStack> _stack;
};

struct ScriptLog {
	ScriptLog(std::wstring msg) {
		_msg = msg;
	}

	void operator()(char const* str, char const* end) const {
		Log::Write(L"TJScript/Parser", _msg+L": "+Wcs(std::string(str,end)));
	}

	template<typename T> void operator()(T x) const {
		Log::Write(L"TJScript/Parser", _msg);
	}

	mutable std::wstring _msg;
};


struct ScriptPushScriptlet {
	ScriptPushScriptlet(ScriptGrammar const* gram, ScriptletType t) {
		_grammar = gram;
		_function = t;
	}

	template<typename T> void operator()(T str, T end) const {
		ref<Scriptlet> s = _grammar->_script->CreateScriptlet(_function);
		_grammar->_stack->Push(s,_grammar->_script->GetScriptletIndex(s));
	}

	template<typename T> void operator()(T val) const {
		ref<Scriptlet> s = _grammar->_script->CreateScriptlet(_function);
		_grammar->_stack->Push(s,_grammar->_script->GetScriptletIndex(s));
	}

	mutable ScriptletType _function;
	mutable ScriptGrammar const* _grammar;
};

struct ScriptLoadScriptlet {
	ScriptLoadScriptlet(ScriptGrammar const* g) {
		_grammar = g;
	}

	template<typename T> void operator()(T str, T end) const {
		int idx = _grammar->_stack->GetCurrentIndex();
		ref<Scriptlet> dlg = _grammar->_stack->Pop();

		ref<Op> op = GC::Hold(new OpLoadScriptlet(idx));
		_grammar->_stack->Top()->Add(op);
	}

	mutable ScriptGrammar const* _grammar;
};

struct ScriptIf {
	ScriptIf(ScriptGrammar const* g) {
		_grammar = g;
	}

	template<typename T> void operator()(T str, T end) const {
		ref<Scriptlet> s = _grammar->_stack->Pop();
		int idx = _grammar->_script->GetScriptletIndex(s);
		ref<Scriptlet> main = _grammar->_stack->Top();
		main->Add(GC::Hold(new OpBranchIf(idx)));
	}
	
	mutable ScriptGrammar const* _grammar;
};

struct ScriptIterate {
	ScriptIterate(ScriptGrammar const* grammar) {
		_grammar = grammar;
	}

	template<typename T> void operator()(T str, T end) const {
		ref<Scriptlet> s = _grammar->_stack->Pop();
		int idx = _grammar->_script->GetScriptletIndex(s);
		ref<Scriptlet> main = _grammar->_stack->Top();
		main->Add(GC::Hold(new OpIterate(idx)));
	}
	
	mutable ScriptGrammar const* _grammar;
};

struct ScriptBeginDelegate {
	ScriptBeginDelegate(ScriptGrammar const* g) {
		_grammar = g;
	}

	void operator()(char x) const;

	mutable ScriptGrammar const* _grammar;
};

struct ScriptEndDelegate {
	ScriptEndDelegate(ScriptGrammar const* g) {
		_grammar = g;
	}

	void operator()(char x) const;

	mutable ScriptGrammar const* _grammar;
};

/* grammar */
distinct_parser<> keyword_p("a-zA-Z0-9_");

class ScriptGrammar : public grammar<ScriptGrammar> {
	public:
		ScriptGrammar(ref<CompiledScript> script) {
			_script = script;
			_stack = GC::Hold(new ScriptletStack());
			ref<Scriptlet> s = _script->CreateScriptlet(ScriptletFunction);
			_stack->Push(s, _script->GetScriptletIndex(s));
		}

		template <typename ScannerT> struct definition {
			definition(ScriptGrammar const& self) {
				/** String/integer literals or result from nested stuff **/
				comment = 
					comment_p("/*","*/") | comment_p("//") | space_p;

				stringValue =
					 lexeme_d[confix_p('"', (*c_escape_ch_p)[ScriptPushString(&self)], '"')];

				intValue =
					int_p[ScriptPush<int>(&self)];

				boolValue =
					lexeme_d[str_p("true")[ScriptPushValue<bool>(&self, true)] | str_p("false")[ScriptPushValue<bool>(&self, false)]];

				doubleValue = 
					real_p[ScriptPush<double>(&self)];

				nullValue =
					lexeme_d[str_p("null")[ScriptPushNull(&self)]];

				value = 
					stringValue | doubleValue | intValue | boolValue | nullValue;

				/** Variable names etc **/
				identifier = 
					lexeme_d[(alpha_p >> *(alnum_p|ch_p('_')))[ScriptPushString(&self)]];

				declaredParameter =
					lexeme_d[(alpha_p >> *(alnum_p|ch_p('_')))];

				breakStatement = 
					keyword_p("break")[ScriptInstruction<OpBreak>(&self)];

				keyValuePair = 
					eps_p(lexeme_d[alpha_p >> *(alnum_p|ch_p('_'))] >> (ch_p('=')|ch_p(':')) >> (~ch_p('='))) 
					>> identifier >> (ch_p('=')|ch_p(':')) >> expression;

				parameterList = 
					( (keyValuePair[ScriptInstruction<OpParameter>(&self)]|expression[ScriptInstruction<OpNamelessParameter>(&self)]) % ch_p(','));

				assignment = 
					lexeme_d[keyword_p("var")] >> identifier >> 
					((ch_p('=') >> expression)[ScriptInstruction<OpSave>(&self)]|eps_p[ScriptPushNull(&self)][ScriptInstruction<OpSave>(&self)]);

				methodCall =
					 (identifier >> !(ch_p('(')[ScriptInstruction<OpPushParameter>(&self)] >> !parameterList >> ')'));

				methodCallConstruct = 
					methodCall[ScriptInstruction<OpCallGlobal>(&self)] >> followingMethodCall;

				followingMethodCall = 
					*indexOperator >> !(ch_p(".") >> ((methodCall[ScriptInstruction<OpCall>(&self)] >> *(indexOperator)) % ch_p('.')));

				/* Operators */
				equalsOperator = 
					(str_p("==") >> expression)[ScriptInstruction<OpEquals>(&self)];

				orOperator = 
					(str_p("||") >> expression)[ScriptInstruction<OpOr>(&self)];

				andOperator =
					(str_p("&&") >> expression)[ScriptInstruction<OpAnd>(&self)];

				xorOperator = 
					(str_p("^^") >> expression)[ScriptInstruction<OpXor>(&self)];

				notEqualsOperator =
					(str_p("!=") >> expression)[ScriptInstruction<OpEquals>(&self)][ScriptInstruction<OpNegate>(&self)];

				plusOperator =
					(ch_p('+') >> term)[ScriptInstruction<OpAdd>(&self)];

				minOperator =
					(ch_p('-') >> term)[ScriptInstruction<OpSub>(&self)];

				divOperator =
					(ch_p('/') >> factor)[ScriptInstruction<OpDiv>(&self)];

				mulOperator =
					(ch_p('*') >> factor)[ScriptInstruction<OpMul>(&self)];

				gtOperator = 
					(ch_p('>') >> expression)[ScriptInstruction<OpGreaterThan>(&self)];

				ltOperator = 
					(ch_p('<') >> expression)[ScriptInstruction<OpLessThan>(&self)];

				/* If/else */
				ifConstruct =
					keyword_p("if") >> ch_p('(') >> expression >> ch_p(')') >> block[ScriptIf(&self)] >> 
					!(keyword_p("else")[ScriptInstruction<OpNegate>(&self)] >> block[ScriptIf(&self)]) >>
					eps_p[ScriptInstruction<OpPop>(&self)];

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
					>> blockInFunction[ScriptLoadScriptlet(&self)])[ScriptInstruction<OpSave>(&self)];

				term = 
					factor >> *(mulOperator|divOperator);

				indexOperator = 
					ch_p('[') >> expression >> ch_p(']')[ScriptInstruction<OpIndex>(&self)];

				factor =
					(delegateConstruct | newConstruct | negatedFactor | (ch_p('(') >> expression >> ch_p(')')) | value | methodCallConstruct);

				negatedFactor = 
					((ch_p('!')|ch_p('-')) >> factor)[ScriptInstruction<OpNegate>(&self)];


				/* after methodCallConstruct completed, add a OpPop so there's nothing left on the stack; otherwise
					for(var i: new Range(0,100)) { log(i); } will work for the first iteration, but will fail the next,
					because the null returned from log is still on the stack where it shouldn't be.
				*/
				statement = 
					functionConstruct | returnConstruct | breakStatement | assignment | methodCallConstruct[ScriptInstruction<OpPop>(&self)];

				blockInFunction =
					ch_p('{')[ScriptPushScriptlet(&self,ScriptletFunction)] >> *((blockConstruct >> *eol_p)|comment) >> ch_p('}');

				blockInFor =
					ch_p('{')[ScriptPushScriptlet(&self,ScriptletLoop)] >> *((blockConstruct >> *eol_p)|comment) >> ch_p('}');

				block =
					ch_p('{')[ScriptPushScriptlet(&self,ScriptletAny)] >> *((blockConstruct >> *eol_p)|comment) >> ch_p('}');

				blockConstruct =
					ifConstruct | forConstruct | (statement >> !ch_p(';'));

				returnConstruct =
					(keyword_p("return") >> ((expression[ScriptInstruction<OpReturnValue>(&self)]) | eps_p[ScriptInstruction<OpReturn>(&self)]));

				forConstruct =
					((keyword_p("for") >> ch_p('(') >> keyword_p("var") >> identifier >> ch_p(':') >> expression >> ch_p(')')) >> blockInFor) [ScriptIterate(&self)];

				newConstruct = 
					((keyword_p("new") >> identifier >> !(ch_p('(')[ScriptInstruction<OpPushParameter>(&self)] >> !parameterList >> ')'))[ScriptInstruction<OpNew>(&self)]) >> followingMethodCall;

				delegateConstruct =
					keyword_p("delegate") >> ch_p('{')[ScriptBeginDelegate(&self)] >> scriptBody >> ch_p('}')[ScriptEndDelegate(&self)] >> followingMethodCall;

				scriptBody = 
					*((blockConstruct >> *eol_p)|comment);

				script = 
					scriptBody >> end_p;
			}

			// values
			rule<ScannerT> stringValue, intValue, boolValue, doubleValue, nullValue;

			// constructs
			rule<ScannerT> scriptBody, block, function, functionConstruct, ifConstruct, comment, assignment, value, identifier, declaredParameter, keyValuePair, parameterList, methodCall, expression, statement, blockConstruct, blockInFunction, blockInFor, script, returnConstruct, breakStatement, forConstruct, newConstruct, methodCallConstruct, followingMethodCall, delegateConstruct;
			
			// operators
			rule<ScannerT> term, factor, negatedFactor, indexOperator, equalsOperator, notEqualsOperator, plusOperator, minOperator, divOperator, mulOperator, orOperator, andOperator, xorOperator, gtOperator, ltOperator;		

			rule<ScannerT> const& start() const { 
				return script;
			}
		};

		mutable ref<CompiledScript> _script;
		mutable std::deque< ref<CompiledScript> > _delegateStack;
		mutable ref<ScriptletStack> _stack;
};


ref<CompiledScript> ScriptContext::Compile(std::wstring source) {
	ref<CompiledScript> script = GC::Hold(new CompiledScript());

	ScriptGrammar parser(script);
	parse_info<> info = parse(Mbs(source).c_str(), parser, space_p);
	if(!info.full) {
		throw ParserException(std::wstring(L"Parsing stopped at")+Wcs(info.stop));
	}

	if(_optimize) {
		script->Optimize();
	}

	return script;
}

ref<CompiledScript> ScriptContext::CompileFile(std::wstring fn) {
	ref<CompiledScript> script = GC::Hold(new CompiledScript());

	std::string fns = Mbs(fn);
	ScriptGrammar parser(script);
	file_iterator<char> begin(fns.c_str());
	file_iterator<char> end = begin.make_end();

	parse_info< file_iterator<char> > info = parse(begin,end, parser, space_p);
	if(!info.full) {
		throw ParserException(std::wstring(L"Parsing stopped"));
	}

	if(_optimize) {
		script->Optimize();
	}

	return script;
}

void ScriptBeginDelegate::operator()(char x) const {
	ref<CompiledScript> dlg = GC::Hold(new CompiledScript());
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
	ref<ScriptDelegate> scriptDelegate = GC::Hold(new ScriptDelegate(dlg));
	current->Add(GC::Hold(new OpPush(scriptDelegate)));
}

void ScriptCall::operator()(char const* str, char const* end) const {
	_grammar->_stack->Top()->Add(GC::Hold(new OpCall()));
}