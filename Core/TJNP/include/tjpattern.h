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

#include "tjnpinternal.h"
#include <iostream>

namespace tj {
	namespace np {
		namespace pattern {
			class NP_EXPORTED Pattern {
				public:
					class PatternException: public tj::shared::Exception {
						public:
							PatternException(const wchar_t* pattern, const wchar_t* test, const wchar_t* message);
							const wchar_t *pattern, *test;
					};

					static bool Match(const wchar_t* pattern, const wchar_t* test);

				private:
					const static wchar_t KOpenBrace;
					const static wchar_t KCloseBrace;
					const static wchar_t KNegate;
					const static wchar_t KRange;
					const static wchar_t KOpenBracket;
					const static wchar_t KCloseBracket;
					const static wchar_t KAny;
					const static wchar_t KKleene;
					const static wchar_t KAlternative;
					const static wchar_t KEnd;

					static bool MatchBrackets(const wchar_t* pattern, const wchar_t* test);
					static bool MatchAlternatives(const wchar_t* pattern, const wchar_t* test);
			};
		}
	}
}

#endif