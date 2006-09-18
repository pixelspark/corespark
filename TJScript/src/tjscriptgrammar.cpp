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

template<typename T> struct ScriptPush {
	ScriptPush(ref<ScriptletStack> stack) {
		_stack = stack;
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
	ScriptPushValue(ref<ScriptletStack> stack, T value) {
		_stack = stack;
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
	ScriptPushNull(ref<ScriptletStack> stack) {
		_stack = stack;
	}

	template<typename T> void operator()(const T start) const {
		ref<Op> op = GC::Hold(new OpPush(GC::Hold(new ScriptNull())));
		_stack->Top()->Add(op);
	}

	template<typename T> void operator()(const T start, const T end) const {
		ref<Op> op = GC::Hold(new OpPush(GC::Hold(new ScriptNull())));
		_stack->Top()->Add(op);
	}

	mutable ref<ScriptletStack> _stack;
};

struct ScriptPushString {
	ScriptPushString(ref<ScriptletStack> stack) {
		_stack = stack;
	}

	template<typename T> void operator()(const T start, const T end) const {
		ref<Op> op = GC::Hold(new OpPush(GC::Hold(new ScriptValue<std::wstring>(std::wstring(start,end)))));
		_stack->Top()->Add(op);
	}

	template<typename T> void operator()(const T str) const {
		ref<Op> op = GC::Hold(new OpPush(GC::Hold(new ScriptValue<std::wstring>(Wcs(std::string(str))))));
		_stack->Top()->Add(op);
	}

	mutable ref<ScriptletStack> _stack;
};

struct ScriptCall {
	ScriptCall(ref<ScriptletStack> s) {
		_stack = s;
	}

	void operator()(char const* str, char const* end) const {
		_stack->Top()->Add(GC::Hold(new OpCall()));
	}

	mutable ref<ScriptletStack> _stack;
};

template<typename T> struct ScriptInstruction {
	ScriptInstruction(ref<ScriptletStack> s) {
		_stack = s;
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

	mutable std::wstring _msg;
};


struct ScriptPushScriptlet {
	ScriptPushScriptlet(ref<ScriptletStack> stack, ref<CompiledScript> script, Scriptlet::ScriptletType t) {
		_script = script;
		_stack = stack;
		_function = t;
	}

	template<typename T> void operator()(T str, T end) const {
		ref<Scriptlet> s = _script->CreateScriptlet(_function);
		_stack->Push(s,_script->GetScriptletIndex(s));
	}

	template<typename T> void operator()(T val) const {
		ref<Scriptlet> s = _script->CreateScriptlet(_function);
		_stack->Push(s,_script->GetScriptletIndex(s));
	}

	mutable ref<ScriptletStack> _stack;
	mutable ref<CompiledScript> _script;
	mutable Scriptlet::ScriptletType _function;
};

struct ScriptLoadScriptlet {
	ScriptLoadScriptlet(ref<ScriptletStack> stack) {
		_stack = stack;
	}

	template<typename T> void operator()(T str, T end) const {
		int idx = _stack->GetCurrentIndex();
		ref<Scriptlet> dlg = _stack->Pop();

		ref<Op> op = GC::Hold(new OpLoadScriptlet(idx));
		_stack->Top()->Add(op);
	}

	mutable ref<ScriptletStack> _stack;
};

struct ScriptIf {
	ScriptIf(ref<ScriptletStack> stack, ref<CompiledScript> script) {
		_script = script;
		_stack = stack;
	}

	template<typename T> void operator()(T str, T end) const {
		ref<Scriptlet> s = _stack->Pop();
		int idx = _script->GetScriptletIndex(s);
		ref<Scriptlet> main = _stack->Top();
		main->Add(GC::Hold(new OpBranchIf(idx)));
	}
	
	mutable ref<ScriptletStack> _stack;
	mutable ref<CompiledScript> _script;
};

struct ScriptIterate {
	ScriptIterate(ref<ScriptletStack> stack, ref<CompiledScript> script) {
		_script = script;
		_stack = stack;
	}

	template<typename T> void operator()(T str, T end) const {
		ref<Scriptlet> s = _stack->Pop();
		int idx = _script->GetScriptletIndex(s);
		ref<Scriptlet> main = _stack->Top();
		main->Add(GC::Hold(new OpIterate(idx)));
	}
	
	mutable ref<ScriptletStack> _stack;
	mutable ref<CompiledScript> _script;
};

/* grammar */

distinct_parser<> keyword_p("a-zA-Z0-9_");

struct ScriptGrammar : public grammar<ScriptGrammar> {
    ScriptGrammar(ref<CompiledScript> script) {
		_script = script;
		_stack = GC::Hold(new ScriptletStack());
		ref<Scriptlet> s = _script->CreateScriptlet(Scriptlet::ScriptletFunction);
		_stack->Push(s, _script->GetScriptletIndex(s));
	}

    template <typename ScannerT> struct definition {
        definition(ScriptGrammar const& self) {
			/** String/integer literals or result from nested stuff **/
			comment = 
				comment_p("/*","*/") | comment_p("//") | space_p;

			stringValue =
				 lexeme_d[confix_p('"', (*c_escape_ch_p)[ScriptPushString(self._stack)], '"')];

			intValue =
				int_p[ScriptPush<int>(self._stack)];

			boolValue =
				lexeme_d[str_p("true")[ScriptPushValue<bool>(self._stack, true)] | str_p("false")[ScriptPushValue<bool>(self._stack, false)]];

			doubleValue = 
				real_p[ScriptPush<double>(self._stack)];

			nullValue =
				lexeme_d[str_p("null")[ScriptPushNull(self._stack)]];

			value = 
				stringValue | doubleValue | intValue | boolValue | nullValue;

			/** Variable names etc **/
			identifier = 
				lexeme_d[(alpha_p >> *(alnum_p|ch_p('_')))[ScriptPushString(self._stack)]];

			declaredParameter =
				lexeme_d[(alpha_p >> *(alnum_p|ch_p('_')))];

			breakStatement = 
				keyword_p("break")[ScriptInstruction<OpBreak>(self._stack)];

			keyValuePair = 
				identifier >> (ch_p('=')|ch_p(':')) >> expression;

			parameterList = 
				(keyValuePair[ScriptInstruction<OpParameter>(self._stack)] % ch_p(','));

			assignment = 
				lexeme_d[keyword_p("var")] >> identifier >> 
				((ch_p('=') >> expression)[ScriptInstruction<OpSave>(self._stack)]|eps_p[ScriptPushNull(self._stack)][ScriptInstruction<OpSave>(self._stack)]);

			methodCall =
				 (identifier >> !(ch_p('(')[ScriptInstruction<OpPushParameter>(self._stack)] >> !parameterList >> ')'));

			methodCallConstruct = 
				methodCall[ScriptInstruction<OpCallGlobal>(self._stack)] >> *indexOperator >> !(ch_p(".") >> ((methodCall[ScriptInstruction<OpCall>(self._stack)] >> *(indexOperator)) % ch_p('.')));

			/* Operators */
			equalsOperator = 
				(str_p("==") >> expression)[ScriptInstruction<OpEquals>(self._stack)];

			orOperator = 
				(str_p("||") >> expression)[ScriptInstruction<OpOr>(self._stack)];

			andOperator =
				(str_p("&&") >> expression)[ScriptInstruction<OpAnd>(self._stack)];

			xorOperator = 
				(str_p("^^") >> expression)[ScriptInstruction<OpXor>(self._stack)];

			notEqualsOperator =
				(str_p("!=") >> expression)[ScriptInstruction<OpEquals>(self._stack)][ScriptInstruction<OpNegate>(self._stack)];

			plusOperator =
				(str_p("+") >> term)[ScriptInstruction<OpAdd>(self._stack)];

			minOperator =
				(str_p("-") >> term)[ScriptInstruction<OpSub>(self._stack)];

			divOperator =
				(str_p("/") >> factor)[ScriptInstruction<OpDiv>(self._stack)];

			mulOperator =
				(str_p("*") >> factor)[ScriptInstruction<OpMul>(self._stack)];

			/* If/else */
			ifConstruct =
				keyword_p("if") >> ch_p('(') >> expression >> ch_p(')') >> block[ScriptIf(self._stack, self._script)] >> 
				!(keyword_p("else")[ScriptInstruction<OpNegate>(self._stack)] >> block[ScriptIf(self._stack, self._script)]) >>
				eps_p[ScriptInstruction<OpPop>(self._stack)];

			// Something that returns a value (methodCall must be last in this rule because of the
			// eps_p, which otherwise pushes a global even when the rest of methodCall doesn't match
			expression = 
				function | (term >> *(plusOperator | minOperator | equalsOperator | notEqualsOperator | orOperator | andOperator | xorOperator));

			// declared with var something = function() {..}
			function =
				keyword_p("function") >> ch_p('(') >> !(declaredParameter % ch_p(',')) >> ch_p(')') >> blockInFunction[ScriptLoadScriptlet(self._stack)];

			// declared with function something() {...}
			functionConstruct =
				(keyword_p("function") 
				>> identifier >> ch_p('(') >> !(declaredParameter % ch_p(',')) >> ch_p(')')
				>> blockInFunction[ScriptLoadScriptlet(self._stack)])[ScriptInstruction<OpSave>(self._stack)];

			term = 
				factor >> *(mulOperator|divOperator);

			indexOperator = 
				ch_p('[') >> expression >> ch_p(']')[ScriptInstruction<OpIndex>(self._stack)];

			factor =
				(newConstruct | negatedFactor | (ch_p('(') >> expression >> ch_p(')')) | value | methodCallConstruct);

			negatedFactor = 
				((ch_p('!')|ch_p('-')) >> factor)[ScriptInstruction<OpNegate>(self._stack)];

			statement = 
				functionConstruct | returnConstruct | breakStatement | assignment | expression;

			blockInFunction =
				ch_p('{')[ScriptPushScriptlet(self._stack, self._script,Scriptlet::ScriptletFunction)] >> *((blockConstruct >> *eol_p)|comment) >> ch_p('}');

			blockInFor =
				ch_p('{')[ScriptPushScriptlet(self._stack, self._script,Scriptlet::ScriptletLoop)] >> *((blockConstruct >> *eol_p)|comment) >> ch_p('}');

			block =
				ch_p('{')[ScriptPushScriptlet(self._stack, self._script,Scriptlet::ScriptletAny)] >> *((blockConstruct >> *eol_p)|comment) >> ch_p('}');

			blockConstruct =
				ifConstruct | forConstruct | (statement >> !ch_p(';'));

			returnConstruct =
				(keyword_p("return") >> ((expression[ScriptInstruction<OpReturnValue>(self._stack)]) | eps_p[ScriptInstruction<OpReturn>(self._stack)]));

			forConstruct =
				((keyword_p("for") >> ch_p('(') >> keyword_p("var") >> identifier >> ch_p(':') >> expression >> ch_p(')')) >> blockInFor) [ScriptIterate(self._stack, self._script)];

			newConstruct = 
				(keyword_p("new") >> identifier >> !(ch_p('(')[ScriptInstruction<OpPushParameter>(self._stack)] >> !parameterList >> ')'))[ScriptInstruction<OpNew>(self._stack)];

			script = 
				*((blockConstruct >> *eol_p)|comment) >> end_p;
        }

		// values
        rule<ScannerT> stringValue, intValue, boolValue, doubleValue, nullValue;

		// constructs
		rule<ScannerT> block, function, functionConstruct, ifConstruct, comment, assignment, value, identifier, declaredParameter, keyValuePair, parameterList, methodCall, expression, statement, blockConstruct, blockInFunction, blockInFor, script, returnConstruct, breakStatement, forConstruct, newConstruct, methodCallConstruct;
		
		// operators
		rule<ScannerT> term, factor, negatedFactor, indexOperator, equalsOperator, notEqualsOperator, plusOperator, minOperator, divOperator, mulOperator, orOperator, andOperator, xorOperator;		

		rule<ScannerT> const& start() const { 
			return script;
		}
    };

    mutable ref<CompiledScript> _script;
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