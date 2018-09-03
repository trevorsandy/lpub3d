#ifndef __TCMACROS_H__
#define __TCMACROS_H__

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif // WIN32
#include <math.h>

#ifndef M_PI

#define M_PI (3.1415926535897932384626433832795)

#endif

#define EPSILON 0.0001
//#define EPSILON 1E-5
#define myabs(x) ((x) < 0 ? -(x) : (x))
#define fEq(x, y) (myabs((x)-(y)) < EPSILON)
#define fEq2(x, y, ep) (myabs((x)-(y)) < (ep))
#define fSwap(x, y)	\
{					\
	float tmp = x;	\
	x = y;			\
	y = tmp;		\
}
#define deg2rad(x) ((x) * M_PI / 180.0)
#define rad2deg(x) ((x) * 180.0 / M_PI)
#define sqr(x) ((x) * (x))
#define sqrAmp(x) ((x) * myabs(x))
#define sqrtAmp(x) ((x) < 0 ? -sqrt(-x) : sqrt(x))
#define simMin(x,y) ((x) < (y) ? (x) : (y))
#define simMax(x,y) ((x) > (y) ? (x) : (y))
#define strupper(s)						\
{												\
	for (int _i = 0; (s)[_i]; _i++)	\
		(s)[_i] = toupper((s)[_i]);	\
}

#define flipBitsB(n) ((((n) & 0x01) << 7) | \
							 (((n) & 0x02) << 5) | \
							 (((n) & 0x04) << 3) | \
							 (((n) & 0x08) << 1) | \
							 (((n) & 0x10) >> 1) | \
							 (((n) & 0x20) >> 3) | \
							 (((n) & 0x40) >> 5) | \
							 (((n) & 0x80) >> 7))

#endif // __TCMACROS_H__
