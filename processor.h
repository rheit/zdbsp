#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "wad.h"
#include "doomdata.h"
#include "workdata.h"
#include "tarray.h"
#include "nodebuild.h"
#include "blockmapbuilder.h"
#include <zlib.h>

class ZLibOut
{
public:
	ZLibOut (FWadWriter &out);
	~ZLibOut ();

	ZLibOut &operator << (BYTE);
	ZLibOut &operator << (WORD);
	ZLibOut &operator << (SWORD);
	ZLibOut &operator << (DWORD);
	ZLibOut &operator << (fixed_t);
	void Write (BYTE *data, int len);

private:
	enum { BUFFER_SIZE = 8192 };

	z_stream Stream;
	BYTE Buffer[BUFFER_SIZE];

	FWadWriter &Out;
};

class FProcessor
{
public:
	FProcessor (FWadReader &inwad, int lump);

	void Write (FWadWriter &out);

private:
	void LoadThings ();
	void LoadLines ();
	void LoadVertices ();
	void LoadSides ();
	void LoadSectors ();
	void GetPolySpots ();

	MapNodeEx *NodesToEx (const MapNode *nodes, int count);
	MapSubsectorEx *SubsectorsToEx (const MapSubsector *ssec, int count);
	MapSegGLEx *SegGLsToEx (const MapSegGL *segs, int count);

	void WriteLines (FWadWriter &out);
	void WriteVertices (FWadWriter &out, int count);
	void WriteSectors (FWadWriter &out);
	void WriteSides (FWadWriter &out);
	void WriteSegs (FWadWriter &out);
	void WriteSSectors (FWadWriter &out) const;
	void WriteNodes (FWadWriter &out) const;
	void WriteBlockmap (FWadWriter &out);
	void WriteReject (FWadWriter &out);

	void WriteGLVertices (FWadWriter &out);
	void WriteGLSegs (FWadWriter &out);
	void WriteGLSSect (FWadWriter &out);
	void WriteGLNodes (FWadWriter &out);

	void WriteBSPZ (FWadWriter &out, const char *label);
	void WriteGLBSPZ (FWadWriter &out, const char *label);

	void WriteVerticesZ (ZLibOut &out, const WideVertex *verts, int orgverts, int newverts);
	void WriteSubsectorsZ (ZLibOut &out, const MapSubsectorEx *subs, int numsubs);
	void WriteSegsZ (ZLibOut &out, const MapSeg *segs, int numsegs);
	void WriteGLSegsZ (ZLibOut &out, const MapSegGLEx *segs, int numsegs);
	void WriteNodesZ (ZLibOut &out, const MapNodeEx *nodes, int numnodes);

	void WriteNodes2 (FWadWriter &out, const char *name, const MapNodeEx *zaNodes, int count) const;
	void WriteSSectors2 (FWadWriter &out, const char *name, const MapSubsectorEx *zaSubs, int count) const;

	FLevel Level;

	TArray<FNodeBuilder::FPolyStart> PolyStarts;
	TArray<FNodeBuilder::FPolyStart> PolyAnchors;

	bool Extended;

	FWadReader &Wad;
	int Lump;
};

#endif //__PROCESSOR_H__
