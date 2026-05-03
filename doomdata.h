#ifndef __DOOMDATA_H__
#define __DOOMDATA_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "zdbsp.h"
#include "tarray.h"

enum
{
	BOXTOP, BOXBOTTOM, BOXLEFT, BOXRIGHT
};

struct UDMFKey
{
	const char *key;
	const char *value;
};

struct MapVertex
{
	int16_t x, y;
};

struct WideVertex
{
	fixed_t x, y;
	int32_t index;
};

struct MapSideDef
{
	int16_t	textureoffset;
	int16_t	rowoffset;
	char	toptexture[8];
	char	bottomtexture[8];
	char	midtexture[8];
	uint16_t sector;
};

struct IntSideDef
{
	// the first 5 values are only used for binary format maps
	int16_t	textureoffset;
	int16_t	rowoffset;
	char	toptexture[8];
	char	bottomtexture[8];
	char	midtexture[8];

	int sector;

	TArray<UDMFKey> props;
};

struct MapLineDef
{
	uint16_t	v1;
	uint16_t	v2;
	int16_t		flags;
	int16_t		special;
	int16_t		tag;
	uint16_t	sidenum[2];
};

struct MapLineDef2
{
	uint16_t	v1;
	uint16_t	v2;
	int16_t		flags;
	uint8_t		special;
	uint8_t		args[5];
	uint16_t	sidenum[2];
};

struct IntLineDef
{
	uint32_t v1;
	uint32_t v2;
	int flags;
	int special;
	int args[5];
	uint32_t sidenum[2];

	TArray<UDMFKey> props;
};

struct MapSector
{
	int16_t	floorheight;
	int16_t	ceilingheight;
	char	floorpic[8];
	char	ceilingpic[8];
	int16_t	lightlevel;
	int16_t	special;
	int16_t	tag;
};

struct IntSector
{
	// none of the sector properties are used by the node builder
	// so there's no need to store them in their expanded form for
	// UDMF. Just storing the UDMF keys and leaving the binary fields
	// empty is enough
	MapSector data;

	TArray<UDMFKey> props;
};

struct MapSubsector
{
	uint16_t	numlines;
	uint16_t	firstline;
};

struct MapSubsectorEx
{
	uint32_t	numlines;
	uint32_t	firstline;
};

struct MapSeg
{
	uint16_t	v1;
	uint16_t	v2;
	uint16_t	angle;
	uint16_t	linedef;
	int16_t		side;
	int16_t		offset;
};

struct MapSegEx
{
	uint32_t	v1;
	uint32_t	v2;
	uint16_t	angle;
	uint16_t	linedef;
	int16_t		side;
	int16_t		offset;
};

struct MapSegGL
{
	uint16_t	v1;
	uint16_t	v2;
	uint16_t	linedef;
	uint16_t	side;
	uint16_t	partner;
};

struct MapSegGL5
{
	uint32_t	v1;
	uint32_t	v2;
	uint16_t	linedef;
	uint16_t	side;
	uint32_t	partner;
};

struct MapSegGLEx
{
	uint32_t	v1;
	uint32_t	v2;
	uint32_t	linedef;
	uint16_t	side;
	uint32_t	partner;
};

#define NF_SUBSECTOR	0x8000
#define NFX_SUBSECTOR	0x80000000

struct MapNode
{
	int16_t		x,y,dx,dy;
	int16_t		bbox[2][4];
	uint16_t	children[2];
};

struct MapNodeExO
{
	int16_t		x,y,dx,dy;
	int16_t		bbox[2][4];
	uint32_t	children[2];
};

struct MapNodeEx
{
	int32_t		x,y,dx,dy;
	int16_t		bbox[2][4];
	uint32_t	children[2];
};

struct MapThing
{
	int16_t		x;
	int16_t		y;
	int16_t		angle;
	int16_t		type;
	int16_t		flags;
};

struct MapThing2
{
	uint16_t	thingid;
	int16_t		x;
	int16_t		y;
	int16_t		z;
	int16_t		angle;
	int16_t		type;
	int16_t		flags;
	uint8_t		special;
	uint8_t		args[5];
};

struct IntThing
{
	uint16_t	thingid;
	fixed_t		x;	// full precision coordinates for UDMF support
	fixed_t		y;
	// everything else is not needed or has no extended form in UDMF
	int16_t		z;
	int16_t		angle;
	int16_t		type;
	int16_t		flags;
	uint8_t		special;
	uint8_t		args[5];

	TArray<UDMFKey> props;
};

struct IntVertex
{
	TArray<UDMFKey> props;
};

struct FLevel
{
	~FLevel ();

	WideVertex		  *Vertices = nullptr;		int NumVertices = 0;
	TArray<IntVertex>  VertexProps;
	TArray<IntSideDef> Sides;
	TArray<IntLineDef> Lines;
	TArray<IntSector>  Sectors;
	TArray<IntThing>   Things;
	MapSubsectorEx	  *Subsectors = nullptr;	int NumSubsectors = 0;
	MapSegEx		  *Segs = nullptr;			int NumSegs = 0;
	MapNodeEx		  *Nodes = nullptr;			int NumNodes = 0;
	uint32_t		  *Blockmap = nullptr;		int BlockmapSize = 0;
	uint8_t			  *Reject = nullptr;		int RejectSize = 0;

	MapSubsectorEx	  *GLSubsectors = nullptr;	int NumGLSubsectors = 0;
	MapSegGLEx		  *GLSegs = nullptr;		int NumGLSegs = 0;
	MapNodeEx		  *GLNodes = nullptr;		int NumGLNodes = 0;
	WideVertex		  *GLVertices = nullptr;	int NumGLVertices = 0;
	uint8_t			  *GLPVS = nullptr;			int GLPVSSize = 0;

	int NumOrgVerts = 0;

	uint32_t		  *OrgSectorMap = nullptr;	int NumOrgSectors = 0;

	fixed_t MinX = 0, MinY = 0, MaxX = 0, MaxY = 0;

	TArray<UDMFKey> props;

	void FindMapBounds ();
	void RemoveExtraLines ();
	void RemoveExtraSides ();
	void RemoveExtraSectors ();

	int NumSides() const { return Sides.Size(); }
	int NumLines() const { return Lines.Size(); }
	int NumSectors() const { return Sectors.Size(); }
	int NumThings() const { return Things.Size(); }
};

const int BLOCKSIZE = 128;
const int BLOCKFRACSIZE = BLOCKSIZE<<FRACBITS;
const int BLOCKBITS = 7;
const int BLOCKFRACBITS = FRACBITS+7;

#endif //__DOOMDATA_H__
