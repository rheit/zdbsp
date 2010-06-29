#ifndef __WORKDATA_H__
#define __WORKDATA_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "zdbsp.h"
#include <xmmintrin.h>

struct vertex_t
{
	fixed_t x, y;
};

struct node_t
{
	union
	{
		struct { fixed_t x, y; };
		__m64 p64;
	};
	union
	{
		struct { fixed_t dx, dy; };
		__m64 d64;
	};
	fixed_t bbox[2][4];
	unsigned int intchildren[2];
};

struct subsector_t
{
	DWORD numlines;
	DWORD firstline;
};


#endif //__WORKDATA_H__
