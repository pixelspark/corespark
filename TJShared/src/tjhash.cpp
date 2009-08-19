#include "../include/tjshared.h"
using namespace tj::shared;

/* Fowler/Noll/Vo hash
 *
 * The basis of this hash algorithm was taken from an idea sent
 * as reviewer comments to the IEEE POSIX P1003.2 committee by:
 *
 *      Phong Vo (http://www.research.att.com/info/kpv/)
 *      Glenn Fowler (http://www.research.att.com/~gsf/)
 *
 * In a subsequent ballot round:
 *
 *      Landon Curt Noll (http://www.isthe.com/chongo/)
 *
 * improved on their algorithm.  Some people tried this hash
 * and found that it worked rather well.  In an EMail message
 * to Landon, they named it the ``Fowler/Noll/Vo'' or FNV hash.
 *
 * FNV hashes are designed to be fast while maintaining a low
 * collision rate. The FNV speed allows one to quickly hash lots
 * of data while maintaining a reasonable collision rate.  See:
 *
 *      http://www.isthe.com/chongo/tech/comp/fnv/index.html
 *
 * for more details as well as other forms of the FNV hash.
 *
 * Please do not copyright this code.  This code is in the public domain.
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * By:
 *	chongo <Landon Curt Noll> /\oo/\
 *      http://www.isthe.com/chongo/
 *
 * Share and Enjoy!	:-)
 */

const int FNVMagicPrime = 0x01000193;
const int FNVInit = 0x811c9dc5;

int FNVHash(void *buf, size_t len, int hval) {
    unsigned char *bp = (unsigned char*)buf;
    unsigned char *be = bp + len;

    while (bp < be) {
		/* multiply by the 32 bit FNV magic prime mod 2^32 */
		hval *= FNVMagicPrime;

		/* xor the bottom with the current octet */
		hval ^= (int)*bp++;
    }

    return hval;
}

Hash::Hash() {
}

Hash::~Hash() {
}

int Hash::Calculate(const String& s) {
	#ifdef TJ_OS_WIN
		wchar_t* buffer = _wcsdup(s.c_str());
	#endif
	
	#ifdef TJ_OS_MAC
		size_t len = wcslen(s.c_str())+1;
		wchar_t* buffer = new wchar_t[len];
		memcpy(buffer, s.c_str(), len);
	#endif

	int hash = FNVHash(buffer, sizeof(wchar_t)*s.length(), FNVInit);
	delete[] buffer;
	return hash;
}

/** Hasheable **/
Hasheable::~Hasheable() {
}