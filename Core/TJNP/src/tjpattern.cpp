/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #include "../include/tjpattern.h"
using namespace tj::shared;
using namespace tj::np::pattern;

const wchar_t Pattern::KOpenBrace =	L'{';
const wchar_t Pattern::KCloseBrace =		L'}';
const wchar_t Pattern::KOpenBracket =	L'[';
const wchar_t Pattern::KCloseBracket =	L']';
const wchar_t Pattern::KNegate =			L'!';
const wchar_t Pattern::KRange =			L'-';
const wchar_t Pattern::KAny =			L'?';
const wchar_t Pattern::KKleene =			L'*';
const wchar_t Pattern::KAlternative =	L',';
const wchar_t Pattern::KEnd =			0;

Pattern::PatternException::PatternException(const wchar_t* p, const wchar_t* t, const wchar_t* m): Exception(m, ExceptionTypeError), pattern(p), test(t) {
}

/** When called from Match: pattern[0]=='[' and test[0]!=0 **/
bool Pattern::MatchBrackets(const wchar_t* pattern, const wchar_t* test) {
	bool negated = false;
	bool result = false;
	const wchar_t* p = pattern;

	if(pattern[1]==KEnd) {
		throw PatternException(pattern, test, L"Unmatched opening bracket");
	}

	if(pattern[1]==KNegate) {
		negated = true;
		result = negated;
		p = &(pattern[1]);
	}

	while(p[0] != KCloseBracket) {
		if(p[0] == KEnd) {
			throw PatternException(p, test, L"Unmatched opening bracket");
		}
		
		if(p[1] == KRange && p[2] != KEnd) {
			if(test[0] >= p[0] && test[0] <= p[2]) {
				result = !negated;
				break;
			}
		}

		if(p[0] == test[0]) {
			result = !negated;
			break;
		}
		p = &(p[1]);
	}

	if(!result) {
		return false;
	}
	else {
		while(p[0]!=KCloseBracket) {
			if(p[0]==KEnd) {
				throw PatternException(p, test, L"Unmatched opening bracket");
				return false;
			}
			p = &(p[1]);
		}
	}

	return Match(&(p[1]), &(test[1]));
}

bool Pattern::MatchAlternatives(const wchar_t* pattern, const wchar_t* test) {
	// Find where the alternative list ends
	const wchar_t* rest = pattern;
	while(rest[0]!=KCloseBrace) {
		if(rest[0]==KEnd) {
			throw PatternException(pattern, test, L"Unmatched opening curly brace");
		}
		rest = &(rest[1]);
	}
	rest = &(rest[1]);
	pattern = &(pattern[1]); // skip opening curly brace
	const wchar_t* tp = test;
	
	while(true) {
		if(pattern[0]==KAlternative) {
			if(Match(rest, tp)) {
				return true;
			}
			else {
				tp = test;
				pattern = &(pattern[1]);
			}
		}
		else if(pattern[0]==KCloseBrace) {
			return Match(rest, tp);
		}
		else if(pattern[0]==tp[0]) {
			pattern = &(pattern[1]);
			tp = &(tp[1]);
		}
		else {
			// Skip to next alternative
			tp = test;
			while(pattern[0]!=KAlternative && pattern[0]!=KCloseBrace) {
				pattern = &(pattern[1]);
			}
			if(pattern[0]==KAlternative) {
				pattern = &(pattern[1]);
			}
		}
	}
	return false;
}

bool Pattern::Match(const wchar_t* pattern, const wchar_t* test) {
	// If the pattern is empty, only an empty test string matches
	if(pattern == 0L || pattern[0] == KEnd) {
		return (test == 0L) || (test[0] == KEnd);
	}
	
	// If the test string is empty, we only match patterns that end with **'s
	if(test[0] == KEnd) {
		if (pattern[0] == KKleene) {
			return Match(&(pattern[1]), test);
		}
		else {
			return false;
		}
	}

	if(pattern[0]==0) {
		return test[0] == KEnd;
	}
	else if(pattern[0]==KAny) {
		return Match(&(pattern[1]), &(test[1]));
	}
	else if(pattern[0]==KKleene) {
		if(Match(&(pattern[1]), test)) {
			return true;
		}
		else {
			return Match(pattern, &(test[1]));
		}
	}
	else if(pattern[0]==KOpenBracket) {
		return MatchBrackets(pattern, test);
	}
	else if(pattern[0]==KOpenBrace) {
		return MatchAlternatives(pattern, test);
	}
	else if(pattern[0]==KCloseBracket || pattern[0]==KCloseBrace) {
		throw PatternException(pattern, test, L"Syntax error, unmatched closing ] or }");
	}

	else {
		return (pattern[0] == test[0]) && Match(&(pattern[1]), &(test[1]));			
	}
}