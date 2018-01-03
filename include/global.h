#ifndef BASELIB_GLOBAL_
#define BASELIB_GLOBAL_

#include<assert.h>

using namespace std;
namespace chx
{
#if defined(__GNUC__)
	inline bool likely(bool x) { return __builtin_expect((x), true); }
	inline bool unlikely(bool x) { return __builtin_expect((x), false); }
#else
	inline bool likely(bool x) { return x; }
	inline bool unlikely(bool x) { return x; }

#endif
}

#endif // BASELIB_GLOBAL_
