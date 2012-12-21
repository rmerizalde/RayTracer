#ifndef _PLATFORM_H_
#define _PLATFORM_H_

typedef signed char        S8;
typedef unsigned char      U8;

typedef signed short       S16;
typedef unsigned short     U16;

typedef signed int         S32;
typedef unsigned int       U32;

typedef float              F32;
typedef double             F64;

#ifndef NULL
#  define NULL 0
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

inline U32 convertLEndianToBEndian(U32 i)
{
	return ((i >> 24) & 0x000000ff) |
			 ((i >> 8)  & 0x0000ff00) |
			 ((i << 8)  & 0x00ff0000) |
			 ((i << 24) & 0xff000000);
}

inline U32 convertBEndianToLEndian(U32 i)
{
   return ((i << 24) & 0xff000000) |
          ((i <<  8) & 0x00ff0000) |
          ((i >>  8) & 0x0000ff00) |
          ((i >> 24) & 0x000000ff);
}



#endif