#ifndef __HELPER_H__

#define __HELPER_H__

#include <errno.h>
#include <wchar.h>

#include "TypeDefs.h"

/*#ifndef min
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#endif*/

#ifdef APDCAM_UNICODE
# define _tprintf	wprintf
# define _stprintf	swprintf
# define _T(x)		L##x
# define TEXT(x)		L##x
#else
# define _tprintf	printf
# define _stprintf	sprintf
# define _T(x)		x
# define TEXT(x)	x
#endif
int memcpy_s(void *dest, size_t nelem, const void *src, size_t count);
//bool QueryPerformanceCounter(LARGE_INTEGER *li);
//bool QueryPerformanceFrequency(LARGE_INTEGER *li);
void Sleep(int64_t msec);
int Save(ADT_HANDLE handle, int sampleCount);

#endif  /* __HELPER_H__ */
