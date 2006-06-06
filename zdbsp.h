#ifndef __ZDBSP_H__
#define __ZDBSP_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <limits.h>
#include <exception>
#include <stdexcept>

#ifdef _MSC_VER
typedef unsigned __int32 uint32_t;
typedef __int32 int32_t;
#else
#include <stdint.h>
#endif

#define ZDBSP_VERSION	"1.7"

enum EBlockmapMode
{
	EBM_Rebuild,
	EBM_Create0
};

enum ERejectMode
{
	ERM_DontTouch,
	ERM_CreateZeroes,
	ERM_Create0,
	ERM_Rebuild
};

extern const char		*Map;
extern const char		*InName;
extern const char		*OutName;
extern bool				 BuildNodes, BuildGLNodes, ConformNodes, GLOnly;
extern bool				 NoPrune;
extern EBlockmapMode	 BlockmapMode;
extern ERejectMode		 RejectMode;
extern int				 MaxSegs;
extern int				 SplitCost;
extern int				 AAPreference;
extern bool				 CheckPolyobjs;
extern bool				 ShowMap;
extern bool				 CompressNodes, CompressGLNodes, V5GLNodes;
extern bool				 HaveSSE2;


#define FIXED_MAX		INT_MAX
#define FIXED_MIN		INT_MIN

#define FRACBITS		16

typedef int fixed_t;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef   signed short SWORD;
#ifdef _WIN32
typedef unsigned long DWORD;
#else
typedef uint32_t DWORD;
#endif
typedef uint32_t angle_t;

angle_t PointToAngle (fixed_t x, fixed_t y);

static const WORD NO_INDEX = 0xffff;
static const angle_t ANGLE_MAX = 0xffffffff;
static const DWORD DWORD_MAX = 0xffffffff;
static const angle_t ANGLE_180 = (1u<<31);
static const angle_t ANGLE_EPSILON = 5000;

void Warn (const char *format, ...);

#if defined(_MSC_VER) && defined(_M_IX86)

#pragma warning (disable: 4035)

inline fixed_t Scale (fixed_t a, fixed_t b, fixed_t c)
{
	__asm mov eax,a
	__asm mov ecx,c
	__asm imul b
	__asm idiv ecx
}

inline fixed_t DivScale30 (fixed_t a, fixed_t b)
{
	__asm mov edx,a
	__asm sar edx,2
	__asm mov eax,a
	__asm shl eax,30
	__asm idiv b
}

inline fixed_t MulScale30 (fixed_t a, fixed_t b)
{
	__asm mov eax,a
	__asm imul b
	__asm shrd eax,edx,30
}

inline fixed_t DMulScale32 (fixed_t a, fixed_t b, fixed_t c, fixed_t d)
{
	__asm mov eax,a
	__asm imul b
	__asm mov ebx,eax
	__asm mov eax,c
	__asm mov esi,edx
	__asm imul d
	__asm add eax,ebx
	__asm adc edx,esi
	__asm mov eax,edx
}

#pragma warning (default: 4035)

#elif defined(__GNUC__)

inline fixed_t Scale (fixed_t a, fixed_t b, fixed_t c)
{
	fixed_t result, dummy;

	asm volatile
		("imull %3\n\t"
		 "idivl %4"
		 : "=a,a,a,a,a,a" (result),
		   "=&d,&d,&d,&d,d,d" (dummy)
		 :   "a,a,a,a,a,a" (a),
		     "m,r,m,r,d,d" (b),
		     "r,r,m,m,r,m" (c)
		 : "%cc"
			);

	return result;
}

inline fixed_t DivScale30 (fixed_t a, fixed_t b)
{
	fixed_t result, dummy;
	asm volatile
		("idivl %4"
		: "=a,a" (result),
		  "=d,d" (dummy)
		: "a,a" (a<<30),
		  "d,d" (a>>2),
		  "r,m" (b) \
		: "%cc");
	return result;
}

inline fixed_t MulScale30 (fixed_t a, fixed_t b)
{
	return ((int64_t)a * b) >> 30;
}

inline fixed_t DMulScale30 (fixed_t a, fixed_t b, fixed_t c, fixed_t d)
{
	return (((int64_t)a * b) + ((int64_t)c * d)) >> 30;
}

inline fixed_t DMulScale32 (fixed_t a, fixed_t b, fixed_t c, fixed_t d)
{
	return (((int64_t)a * b) + ((int64_t)c * d)) >> 32;
}

#else

inline fixed_t Scale (fixed_t a, fixed_t b, fixed_t c)
{
	return (fixed_t)(double(a)*double(b)/double(c));
}

inline fixed_t DivScale30 (fixed_t a, fixed_t b)
{
	return (fixed_t)(double(a)/double(b)*double(1<<30));
}

inline fixed_t MulScale30 (fixed_t a, fixed_t b)
{
	return (fixed_t)(double(a)*double(b)/double(1<<30));
}

inline fixed_t DMulScale30 (fixed_t a, fixed_t b, fixed_t c, fixed_t d)
{
	return (fixed_t)((double(a)*double(b)+double(c)*double(d))/double(1<<30));
}

inline fixed_t DMulScale32 (fixed_t a, fixed_t b, fixed_t c, fixed_t d)
{
	return (fixed_t)((double(a)*double(b)+double(c)*double(d))/4294967296.0);
}

#endif

// FIXME: No macros defined for big-endian machines.
#define LittleShort(x)	(x)
#define LittleLong(x)	(x)

#endif //__ZDBSP_H__
