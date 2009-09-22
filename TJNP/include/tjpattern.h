/** Generic OSC pattern matcher with support for Unicode for TJShow input
(C) Tommy van der Vorst, 2008-2009, All rights reserved.

Permission to use, copy, modify, distribute, and distribute modified versions
of this software and its documentation without fee and without a signed
licensing agreement, is hereby granted, provided that the above copyright
notice, this paragraph and the following two paragraphs appear in all copies,
modifications, and distributions.

IN NO EVENT SHALL THE AUTHOR BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE AUTHOR HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHOR SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
HEREUNDER IS PROVIDED "AS IS". THE AUTHOR HAS NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

Adapted from OSC-pattern-match.c by Matt Wright, 3/16/98, 
which in turn was adapted from oscpattern.c, by Matt Wright and Amar Chaudhury
**/

#ifndef _TJ_PATTERN_H
#define _TJ_PATTERN_H

#include <iostream>

namespace tj {
	namespace np {
		namespace pattern {
			template<typename CharType> class PatternCharTraits {
				public:
					const static CharType KOpenBrace;
					const static CharType KCloseBrace;
					const static CharType KNegate;
					const static CharType KRange;
					const static CharType KOpenBracket;
					const static CharType KCloseBracket;
					const static CharType KAny;
					const static CharType KKleene;
					const static CharType KAlternative;
					const static CharType KEnd;
			};

			template< typename CharType = char, typename CharTraits = PatternCharTraits<CharType> > class Pattern {
				public:
					class PatternException {
						public:
							PatternException(const CharType* pattern, const CharType* test, const wchar_t* message);
							const CharType *pattern, *test;
							const wchar_t* message;
					};


					static bool Match(const CharType* pattern, const CharType* test);

				private:
					static bool MatchBrackets(const CharType* pattern, const CharType* test);
					static bool MatchAlternatives(const CharType* pattern, const CharType* test);
			};

			template<typename CharType, typename Traits> Pattern<CharType, Traits>::PatternException::PatternException(const CharType* p, const CharType* t, const wchar_t* m): pattern(p), test(t), message(m) {
			}

			/** When called from Match: pattern[0]=='[' and test[0]!=0 **/
			template<typename CharType, typename Traits> bool Pattern<CharType, Traits>::MatchBrackets(const CharType* pattern, const CharType* test) {
				bool negated = false;
				bool result = false;
				const CharType* p = pattern;

				if(pattern[1]==Traits::KEnd) {
					throw PatternException(pattern, test, L"Unmatched opening bracket");
				}

				if(pattern[1]==Traits::KNegate) {
					negated = true;
					result = negated;
					p = &(pattern[1]);
				}

				while(p[0] != Traits::KCloseBracket) {
					if(p[0] == Traits::KEnd) {
						throw PatternException(p, test, L"Unmatched opening bracket");
					}
					
					if(p[1] == Traits::KRange && p[2] != Traits::KEnd) {
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
					while(p[0]!=Traits::KCloseBracket) {
						if(p[0]==Traits::KEnd) {
							throw PatternException(p, test, L"Unmatched opening bracket");
							return false;
						}
						p = &(p[1]);
					}
				}

				return Match(&(p[1]), &(test[1]));
			}

			template<typename CharType, typename Traits> bool Pattern<CharType, Traits>::MatchAlternatives(const CharType* pattern, const CharType* test) {
				// Find where the alternative list ends
				const CharType* rest = pattern;
				while(rest[0]!=Traits::KCloseBrace) {
					if(rest[0]==Traits::KEnd) {
						throw PatternException(pattern, test, L"Unmatched opening curly brace");
					}
					rest = &(rest[1]);
				}
				rest = &(rest[1]);
				pattern = &(pattern[1]); // skip opening curly brace
				const CharType* tp = test;
				
				while(true) {
					if(pattern[0]==Traits::KAlternative) {
						if(Match(rest, tp)) {
							return true;
						}
						else {
							tp = test;
							pattern = &(pattern[1]);
						}
					}
					else if(pattern[0]==Traits::KCloseBrace) {
						return Match(rest, tp);
					}
					else if(pattern[0]==tp[0]) {
						pattern = &(pattern[1]);
						tp = &(tp[1]);
					}
					else {
						// Skip to next alternative
						tp = test;
						while(pattern[0]!=Traits::KAlternative && pattern[0]!=Traits::KCloseBrace) {
							pattern = &(pattern[1]);
						}
						if(pattern[0]==Traits::KAlternative) {
							pattern = &(pattern[1]);
						}
					}
				}
				return false;
			}

			template<typename CharType, typename Traits> bool Pattern<CharType, Traits>::Match(const CharType* pattern, const CharType* test) {
				///std::wcout << L"Match t=" << test << L" p=" << pattern << std::endl;

				// If the pattern is empty, only an empty test string matches
				if(pattern == 0L || pattern[0] == Traits::KEnd) {
					return (test == 0L) || (test[0] == Traits::KEnd);
				}
				
				// If the test string is empty, we only match patterns that end with **'s
				if(test[0] == Traits::KEnd) {
					if (pattern[0] == Traits::KKleene) {
						return Match(&(pattern[1]), test);
					}
					else {
						return false;
					}
				}

				switch(pattern[0]) {
					case 0:
						return test[0] == Traits::KEnd;
					
					case Traits::KAny:
						return Match(&(pattern[1]), &(test[1]));

					case Traits::KKleene:
						if(Match(&(pattern[1]), test)) {
							return true;
						}
						else {
							return Match(pattern, &(test[1]));
						}

					case  Traits::KOpenBracket:
						return MatchBrackets(pattern, test);

					case Traits::KOpenBrace:
						return MatchAlternatives(pattern, test);

					case Traits::KCloseBracket:
					case Traits::KCloseBrace:
						throw PatternException(pattern, test, L"Syntax error, unmatched closing ] or }");

					default:
						return (pattern[0] == test[0]) && Match(&(pattern[1]), &(test[1]));			
				}
			}
			
			template<typename CharTraits> const wchar_t PatternCharTraits<wchar_t>::KOpenBrace =	L'{';
			template<typename CharTraits> const wchar_t PatternCharTraits<wchar_t>::KCloseBrace =		L'}';
			template<typename CharTraits> const wchar_t PatternCharTraits<wchar_t>::KOpenBracket =	L'[';
			template<typename CharTraits> const wchar_t PatternCharTraits<wchar_t>::KCloseBracket =	L']';
			template<typename CharTraits> const wchar_t PatternCharTraits<wchar_t>::KNegate =			L'!';
			template<typename CharTraits> const wchar_t PatternCharTraits<wchar_t>::KRange =			L'-';
			template<typename CharTraits> const wchar_t PatternCharTraits<wchar_t>::KAny =			L'?';
			template<typename CharTraits> const wchar_t PatternCharTraits<wchar_t>::KKleene =			L'*';
			template<typename CharTraits> const wchar_t PatternCharTraits<wchar_t>::KAlternative =	L',';
			template<typename CharTraits> const wchar_t PatternCharTraits<wchar_t>::KEnd =			0;

			template<typename CharTraits> const char PatternCharTraits<char>::KOpenBrace =			'{';
			template<typename CharTraits> const char PatternCharTraits<char>::KCloseBrace =			'}';
			template<typename CharTraits> const char PatternCharTraits<char>::KOpenBracket =			'[';
			template<typename CharTraits> const char PatternCharTraits<char>::KCloseBracket =			']';
			template<typename CharTraits> const char PatternCharTraits<char>::KNegate =				'!';
			template<typename CharTraits> const char PatternCharTraits<char>::KRange =				'-';
			template<typename CharTraits> const char PatternCharTraits<char>::KAny =					'?';
			template<typename CharTraits> const char PatternCharTraits<char>::KKleene =				'*';
			template<typename CharTraits> const char PatternCharTraits<char>::KAlternative =			',';
			template<typename CharTraits> const char PatternCharTraits<char>::KEnd =					0;
		}
	}
}

#endif