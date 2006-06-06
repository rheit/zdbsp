#ifndef __DOOMDATA_H__
#define __DOOMDATA_H__

#ifdef _MSC_VER
#pragma once
#endif

enum
{
	BOXTOP, BOXBOTTOM, BOXLEFT, BOXRIGHT
};

struct MapVertex
{
	short x, y;
};

struct WideVertex
{
	fixed_t x, y;
};

struct MapSideDef
{
	short	textureoffset;
	short	rowoffset;
	char	toptexture[8];
	char	bottomtexture[8];
	char	midtexture[8];
	WORD	sector;
};

struct MapLineDef
{
	WORD	v1;
	WORD	v2;
	short	flags;
	short	special;
	short	tag;
	WORD	sidenum[2];
};

struct MapLineDef2
{
	WORD	v1;
	WORD	v2;
	short	flags;
	unsigned char	special;
	unsigned char	args[5];
	WORD	sidenum[2];
};

struct MapSector
{
	short	floorheight;
	short	ceilingheight;
	char	floorpic[8];
	char	ceilingpic[8];
	short	lightlevel;
	short	special;
	short	tag;
};

struct MapSubsector
{
	WORD	numlines;
	WORD	firstline;
};

struct MapSubsectorEx
{
	DWORD	numlines;
	DWORD	firstline;
};

struct MapSeg
{
	WORD	v1;
	WORD	v2;
	WORD	angle;
	WORD	linedef;
	short	side;
	short	offset;
};

struct MapSegGL
{
	WORD	v1;
	WORD	v2;
	WORD	linedef;
	WORD	side;
	WORD	partner;
};

struct MapSegGLEx
{
	DWORD	v1;
	DWORD	v2;
	WORD	linedef;
	WORD	side;
	DWORD	partner;
};

#define NF_SUBSECTOR	0x8000
#define NFX_SUBSECTOR	0x80000000

struct MapNode
{
	short 	x,y,dx,dy;
	short 	bbox[2][4];
	WORD	children[2];
};

struct MapNodeEx
{
	short	x,y,dx,dy;
	short	bbox[2][4];
	DWORD	children[2];
};

struct MapThing
{
	short		x;
	short		y;
	short		angle;
	short		type;
	short		flags;
};

struct MapThing2
{
	unsigned short thingid;
	short		x;
	short		y;
	short		z;
	short		angle;
	short		type;
	short		flags;
	char		special;
	char		args[5];
};

struct FLevel
{
	FLevel ();
	~FLevel ();

	WideVertex *Vertices;		int NumVertices;
	MapSideDef *Sides;			int NumSides;
	MapLineDef2 *Lines;			int NumLines;
	MapSector *Sectors;			int NumSectors;
	MapSubsectorEx *Subsectors;	int NumSubsectors;
	MapSeg *Segs;				int NumSegs;
	MapNodeEx *Nodes;			int NumNodes;
	MapThing2 *Things;			int NumThings;
	WORD *Blockmap;				int BlockmapSize;
	BYTE *Reject;				int RejectSize;

	MapSubsectorEx *GLSubsectors;	int NumGLSubsectors;
	MapSegGLEx *GLSegs;				int NumGLSegs;
	MapNodeEx *GLNodes;				int NumGLNodes;
	WideVertex *GLVertices;			int NumGLVertices;
	BYTE *GLPVS;					int GLPVSSize;

	int NumOrgVerts;

	WORD *OrgSectorMap;			int NumOrgSectors;

	fixed_t MinX, MinY, MaxX, MaxY;

	void FindMapBounds ();
	void RemoveExtraLines ();
	void RemoveExtraSides ();
	void RemoveExtraSectors ();
};

const int BLOCKSIZE = 128;
const int BLOCKFRACSIZE = BLOCKSIZE<<FRACBITS;
const int BLOCKBITS = 7;
const int BLOCKFRACBITS = FRACBITS+7;

#endif //__DOOMDATA_H__
