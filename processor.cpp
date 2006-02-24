/*
    Reads wad files, builds nodes, and saves new wad files.
    Copyright (C) 2002,2003 Randy Heit

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include "processor.h"
//#include "rejectbuilder.h"

extern void ShowView (FLevel *level);

enum
{
	// Thing numbers used in Hexen maps
    PO_HEX_ANCHOR_TYPE = 3000,
    PO_HEX_SPAWN_TYPE,
    PO_HEX_SPAWNCRUSH_TYPE,

    // Thing numbers used in Doom and Heretic maps
    PO_ANCHOR_TYPE = 9300,
    PO_SPAWN_TYPE,
    PO_SPAWNCRUSH_TYPE
};

FLevel::FLevel ()
{
	memset (this, 0, sizeof(*this));
}

FLevel::~FLevel ()
{
	if (Things)			delete[] Things;
	if (Lines)			delete[] Lines;
	if (Vertices)		delete[] Vertices;
	if (Sides)			delete[] Sides;
	if (Sectors)		delete[] Sectors;
	if (Subsectors)		delete[] Subsectors;
	if (Segs)			delete[] Segs;
	if (Nodes)			delete[] Nodes;
	if (Blockmap)		delete[] Blockmap;
	if (Reject)			delete[] Reject;
	if (GLSubsectors)	delete[] GLSubsectors;
	if (GLSegs)			delete[] GLSegs;
	if (GLNodes)		delete[] GLNodes;
	if (GLPVS)			delete[] GLPVS;
}

FProcessor::FProcessor (FWadReader &inwad, int lump)
:
  Wad (inwad), Lump (lump)
{
	printf ("----%s----\n", Wad.LumpName (Lump));

	Extended = Wad.MapHasBehavior (lump);
	LoadThings ();
	LoadVertices ();
	LoadLines ();
	LoadSides ();
	LoadSectors ();

	if (Level.NumLines == 0 || Level.NumVertices == 0 || Level.NumSides == 0 || Level.NumSectors == 0)
	{
		printf ("   Map is incomplete\n");
	}
	else
	{
		// Removing extra vertices is done by the node builder.
		Level.RemoveExtraLines ();
		if (!NoPrune)
		{
			Level.RemoveExtraSides ();
			Level.RemoveExtraSectors ();
		}

		if (BuildNodes)
		{
			GetPolySpots ();
		}
	}
}

void FProcessor::LoadThings ()
{
	if (Extended)
	{
		ReadMapLump<MapThing2> (Wad, "THINGS", Lump, Level.Things, Level.NumThings);

		for (int i = 0; i < Level.NumThings; ++i)
		{
			Level.Things[i].x = SHORT(Level.Things[i].x);
			Level.Things[i].y = SHORT(Level.Things[i].y);
			Level.Things[i].angle = SHORT(Level.Things[i].angle);
			Level.Things[i].type = SHORT(Level.Things[i].type);
			Level.Things[i].flags = SHORT(Level.Things[i].flags);
		}
	}
	else
	{
		MapThing *mt;
		ReadMapLump<MapThing> (Wad, "THINGS", Lump, mt, Level.NumThings);

		Level.Things = new MapThing2[Level.NumThings];
		memset (Level.Things, 0, sizeof(*Level.Things)*Level.NumThings);
		for (int i = 0; i < Level.NumThings; ++i)
		{
			Level.Things[i].x = SHORT(mt[i].x);
			Level.Things[i].y = SHORT(mt[i].y);
			Level.Things[i].angle = SHORT(mt[i].angle);
			Level.Things[i].type = SHORT(mt[i].type);
			Level.Things[i].flags = SHORT(mt[i].flags);
		}
		delete[] mt;
	}
}

void FProcessor::LoadLines ()
{
	if (Extended)
	{
		ReadMapLump<MapLineDef2> (Wad, "LINEDEFS", Lump, Level.Lines, Level.NumLines);

		for (int i = 0; i < Level.NumLines; ++i)
		{
			Level.Lines[i].v1 = SHORT(Level.Lines[i].v1);
			Level.Lines[i].v2 = SHORT(Level.Lines[i].v2);
			Level.Lines[i].flags = SHORT(Level.Lines[i].flags);
			Level.Lines[i].sidenum[0] = SHORT(Level.Lines[i].sidenum[0]);
			Level.Lines[i].sidenum[1] = SHORT(Level.Lines[i].sidenum[1]);
		}
	}
	else
	{
		MapLineDef *ml;
		ReadMapLump<MapLineDef> (Wad, "LINEDEFS", Lump, ml, Level.NumLines);

		Level.Lines = new MapLineDef2[Level.NumLines];
		memset (Level.Lines, 0, sizeof(*Level.Lines)*Level.NumLines);
		for (int i = 0; i < Level.NumLines; ++i)
		{
			Level.Lines[i].v1 = SHORT(ml[i].v1);
			Level.Lines[i].v2 = SHORT(ml[i].v2);
			Level.Lines[i].flags = SHORT(ml[i].flags);
			Level.Lines[i].sidenum[0] = SHORT(ml[i].sidenum[0]);
			Level.Lines[i].sidenum[1] = SHORT(ml[i].sidenum[1]);

			// Store the special and tag in the args array so we don't lose them
			short t = SHORT(ml[i].special);
			Level.Lines[i].args[2] = t & 255;
			Level.Lines[i].args[3] = t >> 8;
			t = SHORT(ml[i].tag);
			Level.Lines[i].args[0] = t & 255;
			Level.Lines[i].args[1] = t >> 8;
		}
		delete[] ml;
	}
}

void FProcessor::LoadVertices ()
{
	MapVertex *verts;
	ReadMapLump<MapVertex> (Wad, "VERTEXES", Lump, verts, Level.NumVertices);

	Level.Vertices = new WideVertex[Level.NumVertices];

	for (int i = 0; i < Level.NumVertices; ++i)
	{
		Level.Vertices[i].x = SHORT(verts[i].x) << FRACBITS;
		Level.Vertices[i].y = SHORT(verts[i].y) << FRACBITS;
	}
}

void FProcessor::LoadSides ()
{
	ReadMapLump<MapSideDef> (Wad, "SIDEDEFS", Lump, Level.Sides, Level.NumSides);

	for (int i = 0; i < Level.NumSides; ++i)
	{
		Level.Sides[i].sector = SHORT(Level.Sides[i].sector);
	}
}

void FProcessor::LoadSectors ()
{
	ReadMapLump<MapSector> (Wad, "SECTORS", Lump, Level.Sectors, Level.NumSectors);
}

void FLevel::RemoveExtraLines ()
{
	int i, newNumLines;

	// Extra lines are those with 0 length. Collision detection against
	// one of those could cause a divide by 0, so it's best to remove them.

	for (i = newNumLines = 0; i < NumLines; ++i)
	{
		if (Vertices[Lines[i].v1].x != Vertices[Lines[i].v2].x ||
			Vertices[Lines[i].v1].y != Vertices[Lines[i].v2].y)
		{
			if (i != newNumLines)
			{
				Lines[newNumLines] = Lines[i];
			}
			++newNumLines;
		}
	}
	if (newNumLines < NumLines)
	{
		int diff = NumLines - newNumLines;

		printf ("   Removed %d line%s with 0 length.\n", diff, diff > 1 ? "s" : "");
	}
	NumLines = newNumLines;
}

void FLevel::RemoveExtraSides ()
{
	BYTE *used;
	WORD *remap;
	int i, newNumSides;

	// Extra sides are those that aren't referenced by any lines.
	// They just waste space, so get rid of them.

	used = new BYTE[NumSides];
	memset (used, 0, NumSides*sizeof(*used));
	remap = new WORD[NumSides];

	// Mark all used sides
	for (i = 0; i < NumLines; ++i)
	{
		if (Lines[i].sidenum[0] != NO_INDEX)
		{
			used[Lines[i].sidenum[0]] = 1;
		}
		else
		{
			printf ("   Line %d needs a front sidedef before it will run with ZDoom.\n", i);
		}
		if (Lines[i].sidenum[1] != NO_INDEX)
		{
			used[Lines[i].sidenum[1]] = 1;
		}
	}

	// Shift out any unused sides
	for (i = newNumSides = 0; i < NumSides; ++i)
	{
		if (used[i])
		{
			if (i != newNumSides)
			{
				Sides[newNumSides] = Sides[i];
			}
			remap[i] = newNumSides++;
		}
		else
		{
			remap[i] = NO_INDEX;
		}
	}

	if (newNumSides < NumSides)
	{
		int diff = NumSides - newNumSides;

		printf ("   Removed %d unused sidedef%s.\n", diff, diff > 1 ? "s" : "");
		NumSides = newNumSides;

		// Renumber side references in lines
		for (i = 0; i < NumLines; ++i)
		{
			if (Lines[i].sidenum[0] != NO_INDEX)
			{
				Lines[i].sidenum[0] = remap[Lines[i].sidenum[0]];
			}
			if (Lines[i].sidenum[1] != NO_INDEX)
			{
				Lines[i].sidenum[1] = remap[Lines[i].sidenum[1]];
			}
		}
	}

	delete[] used;
	delete[] remap;
}

void FLevel::RemoveExtraSectors ()
{
	BYTE *used;
	WORD *remap;
	int i, newNumSectors;

	// Extra sectors are those that aren't referenced by any sides.
	// They just waste space, so get rid of them.

	used = new BYTE[NumSectors];
	memset (used, 0, NumSectors*sizeof(*used));
	remap = new WORD[NumSectors];

	// Mark all used sectors
	for (i = 0; i < NumSides; ++i)
	{
		if (Sides[i].sector != NO_INDEX)
		{
			used[Sides[i].sector] = 1;
		}
		else
		{
			printf ("   Sidedef %d needs a front sector before it will run with ZDoom.\n", i);
		}
	}

	// Shift out any unused sides
	for (i = newNumSectors = 0; i < NumSectors; ++i)
	{
		if (used[i])
		{
			if (i != newNumSectors)
			{
				Sectors[newNumSectors] = Sectors[i];
			}
			remap[i] = newNumSectors++;
		}
		else
		{
			remap[i] = NO_INDEX;
		}
	}

	if (newNumSectors < NumSectors)
	{
		int diff = NumSectors - newNumSectors;
		printf ("   Removed %d unused sector%s.\n", diff, diff > 1 ? "s" : "");
		NumSectors = newNumSectors;

		// Renumber sector references in sides
		for (i = 0; i < NumSides; ++i)
		{
			if (Sides[i].sector != NO_INDEX)
			{
				Sides[i].sector = remap[Sides[i].sector];
			}
		}
	}

	delete[] used;
	delete[] remap;
}

void FProcessor::GetPolySpots ()
{
	if (Extended && CheckPolyobjs)
	{
		int spot1, spot2, anchor, i;

		// Determine if this is a Hexen map by looking for things of type 3000
		// Only Hexen maps use them, and they are the polyobject anchors
		for (i = 0; i < Level.NumThings; ++i)
		{
			if (Level.Things[i].type == PO_HEX_ANCHOR_TYPE)
			{
				break;
			}
		}

		if (i < Level.NumThings)
		{
			spot1 = PO_HEX_SPAWN_TYPE;
			spot2 = PO_HEX_SPAWNCRUSH_TYPE;
			anchor = PO_HEX_ANCHOR_TYPE;
		}
		else
		{
			spot1 = PO_SPAWN_TYPE;
			spot2 = PO_SPAWNCRUSH_TYPE;
			anchor = PO_ANCHOR_TYPE;
		}

		for (i = 0; i < Level.NumThings; ++i)
		{
			if (Level.Things[i].type == spot1 ||
				Level.Things[i].type == spot2 ||
				Level.Things[i].type == anchor)
			{
				FNodeBuilder::FPolyStart newvert;
				newvert.x = Level.Things[i].x << FRACBITS;
				newvert.y = Level.Things[i].y << FRACBITS;
				newvert.polynum = Level.Things[i].angle;
				if (Level.Things[i].type == anchor)
				{
					PolyAnchors.Push (newvert);
				}
				else
				{
					PolyStarts.Push (newvert);
				}
			}
		}
	}
}

void FProcessor::Write (FWadWriter &out)
{
	if (Level.NumLines == 0 || Level.NumSides == 0 || Level.NumSectors == 0 || Level.NumVertices == 0)
	{
		// Map is empty, so just copy it as-is
		out.CopyLump (Wad, Lump);
		out.CopyLump (Wad, Wad.FindMapLump ("THINGS", Lump));
		out.CopyLump (Wad, Wad.FindMapLump ("LINEDEFS", Lump));
		out.CopyLump (Wad, Wad.FindMapLump ("SIDEDEFS", Lump));
		out.CopyLump (Wad, Wad.FindMapLump ("VERTEXES", Lump));
		out.CreateLabel ("SEGS");
		out.CreateLabel ("SSECTORS");
		out.CreateLabel ("NODES");
		out.CopyLump (Wad, Wad.FindMapLump ("SECTORS", Lump));
		out.CreateLabel ("REJECT");
		out.CreateLabel ("BLOCKMAP");
		if (Extended)
		{
			out.CopyLump (Wad, Wad.FindMapLump ("BEHAVIOR", Lump));
			out.CopyLump (Wad, Wad.FindMapLump ("SCRIPTS", Lump));
		}
		return;
	}

	bool compress, compressGL;

#ifdef BLOCK_TEST
	int size;
	BYTE *blockmap;
	ReadLump<BYTE> (Wad, Wad.FindMapLump ("BLOCKMAP", Lump), blockmap, size);
	if (blockmap)
	{
		FILE *f = fopen ("blockmap.lmp", "wb");
		if (f)
		{
			fwrite (blockmap, 1, size, f);
			fclose (f);
		}
		delete[] blockmap;
	}
#endif

	if (BuildNodes)
	{
		FNodeBuilder *builder = NULL;
		
		try
		{
			builder = new FNodeBuilder (Level, PolyStarts, PolyAnchors, Wad.LumpName (Lump), BuildGLNodes);
			if (builder == NULL)
			{
				throw exception("   Not enough memory to build nodes!");
			}

			delete[] Level.Vertices;
			builder->GetVertices (Level.Vertices, Level.NumVertices);

			if (ConformNodes)
			{
				// When the nodes are "conformed", the normal and GL nodes use the same
				// basic information. This creates normal nodes that are less "good" than
				// possible, but it makes it easier to compare the two sets of nodes to
				// determine the correctness of the GL nodes.
				builder->GetNodes (Level.Nodes, Level.NumNodes,
					Level.Segs, Level.NumSegs,
					Level.Subsectors, Level.NumSubsectors);
				builder->GetVertices (Level.GLVertices, Level.NumGLVertices);
				builder->GetGLNodes (Level.GLNodes, Level.NumGLNodes,
					Level.GLSegs, Level.NumGLSegs,
					Level.GLSubsectors, Level.NumGLSubsectors);
			}
			else
			{
				if (BuildGLNodes)
				{
					builder->GetVertices (Level.GLVertices, Level.NumGLVertices);
					builder->GetGLNodes (Level.GLNodes, Level.NumGLNodes,
						Level.GLSegs, Level.NumGLSegs,
						Level.GLSubsectors, Level.NumGLSubsectors);

					if (!GLOnly)
					{
						// Now repeat the process to obtain regular nodes
						delete builder;
						builder = new FNodeBuilder (Level, PolyStarts, PolyAnchors, Wad.LumpName (Lump), false);
						if (builder == NULL)
						{
							throw exception("   Not enough memory to build regular nodes!");
						}
						delete[] Level.Vertices;
						builder->GetVertices (Level.Vertices, Level.NumVertices);
					}
				}
				if (!GLOnly)
				{
					builder->GetNodes (Level.Nodes, Level.NumNodes,
						Level.Segs, Level.NumSegs,
						Level.Subsectors, Level.NumSubsectors);
				}
			}
			delete builder;
			builder = NULL;
		}
		catch (...)
		{
			if (builder != NULL)
			{
				delete builder;
			}
			throw;
		}
	}

	FBlockmapBuilder bbuilder (Level);
	WORD *blocks = bbuilder.GetBlockmap (Level.BlockmapSize);
	Level.Blockmap = new WORD[Level.BlockmapSize];
	memcpy (Level.Blockmap, blocks, Level.BlockmapSize*sizeof(WORD));

	Level.RejectSize = (Level.NumSectors*Level.NumSectors + 7) / 8;
	Level.Reject = NULL;

	switch (RejectMode)
	{
	case ERM_Rebuild:
		//FRejectBuilder reject (Level);
		//Level.Reject = reject.GetReject ();
		printf ("   Rebuilding the reject is unsupported.\n");
		// Intentional fall-through

	case ERM_DontTouch:
		{
			int lump = Wad.FindMapLump ("REJECT", Lump);

			if (lump >= 0)
			{
				ReadLump<BYTE> (Wad, lump, Level.Reject, Level.RejectSize);
				if (Level.RejectSize != (Level.NumSectors*Level.NumSectors + 7) / 8)
				{
					// If the reject is the wrong size, don't use it.
					delete[] Level.Reject;
					Level.Reject = NULL;
					if (Level.RejectSize != 0)
					{ // Do not warn about 0-length rejects
						printf ("   REJECT is the wrong size, so it will be removed.\n");
					}
					Level.RejectSize = 0;
				}
			}
		}
		break;

	case ERM_Create0:
		break;

	case ERM_CreateZeroes:
		Level.Reject = new BYTE[Level.RejectSize];
		memset (Level.Reject, 0, Level.RejectSize);
		break;
	}

	if (ShowMap)
	{
		ShowView (&Level);
	}

	if (Level.GLNodes != NULL )
	{
		compressGL = CompressGLNodes ||
			(Level.NumVertices > 32767) ||
			(Level.NumGLVertices > 32767) ||
			(Level.NumGLSegs > 65535) ||
			(Level.NumGLNodes > 32767) ||
			(Level.NumGLSubsectors > 32767);
	}
	else
	{
		compressGL = false;
	}

	// If the GL nodes are compressed, then the regular nodes must also be compressed.
	compress = CompressNodes || compressGL ||
		(Level.NumVertices > 65535) ||
		(Level.NumSegs > 65535) ||
		(Level.NumSubsectors > 32767) ||
		(Level.NumNodes > 32767);

	out.CopyLump (Wad, Lump);
	out.CopyLump (Wad, Wad.FindMapLump ("THINGS", Lump));
	WriteLines (out);
	WriteSides (out);
	WriteVertices (out, compress || GLOnly ? Level.NumOrgVerts : Level.NumVertices);
	if (BuildNodes)
	{
		if (!compress)
		{
			if (!GLOnly)
			{
				WriteSegs (out);
				WriteSSectors (out);
				WriteNodes (out);
			}
			else
			{
				out.CreateLabel ("SEGS");
				out.CreateLabel ("SSECTORS");
				out.CreateLabel ("NODES");
			}
		}
		else
		{
			out.CreateLabel ("SEGS");
			if (compressGL)
			{
				WriteGLBSPZ (out, "SSECTORS");
			}
			else
			{
				out.CreateLabel ("SSECTORS");
			}
			if (!GLOnly)
			{
				WriteBSPZ (out, "NODES");
			}
			else
			{
				out.CreateLabel ("NODES");
			}
		}
	}
	else
	{
		out.CopyLump (Wad, Wad.FindMapLump ("SEGS", Lump));
		out.CopyLump (Wad, Wad.FindMapLump ("SSECTORS", Lump));
		out.CopyLump (Wad, Wad.FindMapLump ("NODES", Lump));
	}
	WriteSectors (out);
	WriteReject (out);
	WriteBlockmap (out);
	if (Extended)
	{
		out.CopyLump (Wad, Wad.FindMapLump ("BEHAVIOR", Lump));
		out.CopyLump (Wad, Wad.FindMapLump ("SCRIPTS", Lump));
	}
	if (Level.GLNodes != NULL && !compressGL)
	{
		char glname[9];
		glname[0] = 'G';
		glname[1] = 'L';
		glname[2] = '_';
		glname[8] = 0;
		strncpy (glname+3, Wad.LumpName (Lump), 5);
		out.CreateLabel (glname);
		WriteGLVertices (out);
		WriteGLSegs (out);
		WriteGLSSect (out);
		WriteGLNodes (out);
	}
}

MapNodeEx *FProcessor::NodesToEx (const MapNode *nodes, int count)
{
	if (count == 0)
	{
		return NULL;
	}

	MapNodeEx *Nodes = new MapNodeEx[Level.NumNodes];
	int x;

	for (x = 0; x < count; ++x)
	{
		WORD child;
		int i;

		for (i = 0; i < 4+2*4; ++i)
		{
			*((WORD *)&Nodes[x] + i) = SHORT(*((WORD *)&nodes[x] + i));
		}
		for (i = 0; i < 2; ++i)
		{
			child = SHORT(nodes[x].children[i]);
			if (child & NF_SUBSECTOR)
			{
				Nodes[x].children[i] = child + (NFX_SUBSECTOR - NF_SUBSECTOR);
			}
			else
			{
				Nodes[x].children[i] = child;
			}
		}
	}
	return Nodes;
}

MapSubsectorEx *FProcessor::SubsectorsToEx (const MapSubsector *ssec, int count)
{
	if (count == 0)
	{
		return NULL;
	}

	MapSubsectorEx *out = new MapSubsectorEx[Level.NumSubsectors];
	int x;

	for (x = 0; x < count; ++x)
	{
		out[x].numlines = SHORT(ssec[x].numlines);
		out[x].firstline = SHORT(ssec[x].firstline);
	}
	
	return out;
}

MapSegGLEx *FProcessor::SegGLsToEx (const MapSegGL *segs, int count)
{
	if (count == 0)
	{
		return NULL;
	}

	MapSegGLEx *out = new MapSegGLEx[count];
	int x;

	for (x = 0; x < count; ++x)
	{
		out[x].v1 = SHORT(segs[x].v1);
		out[x].v2 = SHORT(segs[x].v2);
		out[x].linedef = SHORT(segs[x].linedef);
		out[x].side = SHORT(segs[x].side);
		out[x].partner = SHORT(segs[x].partner);
	}

	return out;
}

void FProcessor::WriteVertices (FWadWriter &out, int count)
{
	int i;
	fixed_t *vertdata = (fixed_t *)Level.Vertices;

	count *= 2;
	short *verts = new short[count];

	for (i = 0; i < count; ++i)
	{
		verts[i] = SHORT(vertdata[i] >> FRACBITS);
	}
	out.WriteLump ("VERTEXES", verts, sizeof(*verts)*count);
	delete[] verts;

	if (count >= 65536)
	{
		printf ("   VERTEXES is past the normal limit. (%d vertices)\n", count/2);
	}
}

void FProcessor::WriteLines (FWadWriter &out)
{
	int i;

	if (Extended)
	{
		for (i = 0; i < Level.NumLines; ++i)
		{
			Level.Lines[i].v1 = SHORT(Level.Lines[i].v1);
			Level.Lines[i].v2 = SHORT(Level.Lines[i].v2);
			Level.Lines[i].flags = SHORT(Level.Lines[i].flags);
			Level.Lines[i].sidenum[0] = SHORT(Level.Lines[i].sidenum[0]);
			Level.Lines[i].sidenum[1] = SHORT(Level.Lines[i].sidenum[1]);
		}
		out.WriteLump ("LINEDEFS", Level.Lines, Level.NumLines*sizeof(*Level.Lines));
	}
	else
	{
		MapLineDef *ld = new MapLineDef[Level.NumLines];

		for (i = 0; i < Level.NumLines; ++i)
		{
			short t;

			ld[i].v1 = SHORT(Level.Lines[i].v1);
			ld[i].v2 = SHORT(Level.Lines[i].v2);
			ld[i].flags = SHORT(Level.Lines[i].flags);
			ld[i].sidenum[0] = SHORT(Level.Lines[i].sidenum[0]);
			ld[i].sidenum[1] = SHORT(Level.Lines[i].sidenum[1]);

			t = Level.Lines[i].args[2] + (Level.Lines[i].args[3]<<8);
			ld[i].special = SHORT(t);

			t = Level.Lines[i].args[0] + (Level.Lines[i].args[1]<<8);
			ld[i].tag = SHORT(t);
		}
		out.WriteLump ("LINEDEFS", ld, Level.NumLines*sizeof(*ld));
		delete[] ld;
	}
}

void FProcessor::WriteSides (FWadWriter &out)
{
	int i;

	for (i = 0; i < Level.NumSides; ++i)
	{
		Level.Sides[i].sector = SHORT(Level.Sides[i].sector);
	}
	out.WriteLump ("SIDEDEFS", Level.Sides, Level.NumSides*sizeof(*Level.Sides));
}

void FProcessor::WriteSectors (FWadWriter &out)
{
	out.WriteLump ("SECTORS", Level.Sectors, Level.NumSectors*sizeof(*Level.Sectors));
}

void FProcessor::WriteSegs (FWadWriter &out)
{
	int i, count;
	short *segdata;

	segdata = (short *)Level.Segs;
	count = Level.NumSegs*sizeof(MapSeg)/sizeof(*segdata);

	for (i = 0; i < count; ++i)
	{
		segdata[i] = SHORT(segdata[i]);
	}
	out.WriteLump ("SEGS", segdata, sizeof(*segdata)*count);

	count /= sizeof(MapSeg)/sizeof(*segdata);
	if (count >= 65536)
	{
		printf ("   SEGS is too big for any port. (%d segs)\n", count);
	}
	else if (count >= 32768)
	{
		printf ("   SEGS is too big for vanilla Doom and most ports. (%d segs)\n", count);
	}
}

void FProcessor::WriteSSectors (FWadWriter &out) const
{
	WriteSSectors2 (out, "SSECTORS", Level.Subsectors, Level.NumSubsectors);
}

void FProcessor::WriteSSectors2 (FWadWriter &out, const char *name, const MapSubsectorEx *subs, int count) const
{
	int i;
	MapSubsector *ssec;

	ssec = new MapSubsector[count];

	for (i = 0; i < count; ++i)
	{
		ssec[i].firstline = SHORT((WORD)subs[i].firstline);
		ssec[i].numlines = SHORT((WORD)subs[i].numlines);
	}
	out.WriteLump (name, ssec, sizeof(*ssec)*count);
	FILE *f = fopen (name, "wb");
	if (f)
	{
		fwrite (ssec, count, sizeof(*ssec), f);
		fclose (f);
	}
	delete[] ssec;

	if (count >= 65536)
	{
		printf ("   %s is too big. (%d subsectors)\n", name, count);
	}
}

void FProcessor::WriteNodes (FWadWriter &out) const
{
	WriteNodes2 (out, "NODES", Level.Nodes, Level.NumNodes);
}

void FProcessor::WriteNodes2 (FWadWriter &out, const char *name, const MapNodeEx *zaNodes, int count) const
{
	int i, j;
	short *onodes, *nodes;

	nodes = onodes = new short[count * sizeof(MapNode)/2];

	for (i = 0; i < count; ++i)
	{
		short *inodes = (short *)&zaNodes[i];
		for (j = 0; j < 4+2*4; ++j)
		{
			nodes[j] = SHORT(inodes[j]);
		}
		nodes += j;
		for (j = 0; j < 2; ++j)
		{
			DWORD child = zaNodes[i].children[j];
			if (child & NFX_SUBSECTOR)
			{
				*nodes++ = SHORT(WORD(child - (NFX_SUBSECTOR + NF_SUBSECTOR)));
			}
			else
			{
				*nodes++ = SHORT((WORD)child);
			}
		}
	}
	out.WriteLump (name, onodes, count * sizeof(MapNode));
	delete[] onodes;

	if (count >= 32768)
	{
		printf ("   %s is too big. (%d nodes)\n", name, count);
	}
}

void FProcessor::WriteBlockmap (FWadWriter &out)
{
	if (BlockmapMode == EBM_Create0)
	{
		out.CreateLabel ("BLOCKMAP");
		return;
	}

	size_t i, count;
	WORD *blocks;

	count = Level.BlockmapSize;
	blocks = Level.Blockmap;

	for (i = 0; i < count; ++i)
	{
		blocks[i] = SHORT(blocks[i]);
	}
	out.WriteLump ("BLOCKMAP", blocks, sizeof(*blocks)*count);

#ifdef BLOCK_TEST
	FILE *f = fopen ("blockmap.lm2", "wb");
	if (f)
	{
		fwrite (blocks, count, sizeof(*blocks), f);
		fclose (f);
	}
#endif

	for (i = 0; i < count; ++i)
	{
		blocks[i] = SHORT(blocks[i]);
	}

	if (count >= 65536)
	{
		printf ("   BLOCKMAP is so big that ports will have to recreate it.\n"
				"   Vanilla Doom cannot handle it at all. If this map is for ZDoom 2+,\n"
				"   you should use the -b switch to save space in the wad.\n");
	}
	else if (count >= 32768)
	{
		printf ("   BLOCKMAP is too big for vanilla Doom.\n");
	}
}

void FProcessor::WriteReject (FWadWriter &out)
{
	if (RejectMode == ERM_Create0 || Level.Reject == NULL)
	{
		out.CreateLabel ("REJECT");
	}
	else
	{
		out.WriteLump ("REJECT", Level.Reject, Level.RejectSize);
	}
}

void FProcessor::WriteGLVertices (FWadWriter &out)
{
	int i, count = (Level.NumGLVertices - Level.NumOrgVerts) * 2;
	fixed_t *vertdata = (fixed_t *)Level.GLVertices + Level.NumOrgVerts * 2;

	fixed_t *verts = new fixed_t[count+1];
	char *magic = (char *)verts;
	magic[0] = 'g';
	magic[1] = 'N';
	magic[2] = 'd';
	magic[3] = '2';

	for (i = 0; i < count; ++i)
	{
		verts[i+1] = LONG(vertdata[i]);
	}
	out.WriteLump ("GL_VERT", verts, sizeof(*verts)*(count+1));
	delete[] verts;

	if (count > 65536)
	{
		printf ("   GL_VERT is too big. (%d GL vertices)\n", count/2);
	}
}

void FProcessor::WriteGLSegs (FWadWriter &out)
{
	int i, count;
	MapSegGL *segdata;

	count = Level.NumGLSegs;
	segdata = new MapSegGL[count];

	for (i = 0; i < count; ++i)
	{
		if (Level.GLSegs[i].v1 < Level.NumOrgVerts)
		{
			segdata[i].v1 = SHORT(Level.GLSegs[i].v1);
		}
		else
		{
			segdata[i].v1 = SHORT(0x8000 | (Level.GLSegs[i].v1 - Level.NumOrgVerts));
		}
		if (Level.GLSegs[i].v2 < Level.NumOrgVerts)
		{
			segdata[i].v2 = SHORT(Level.GLSegs[i].v2);
		}
		else
		{
			segdata[i].v2 = SHORT(0x8000 | (Level.GLSegs[i].v2 - Level.NumOrgVerts));
		}
		segdata[i].linedef = SHORT(Level.GLSegs[i].linedef);
		segdata[i].side = SHORT(Level.GLSegs[i].side);
		segdata[i].partner = SHORT((WORD)Level.GLSegs[i].partner);
	}
	out.WriteLump ("GL_SEGS", segdata, sizeof(MapSegGL)*count);

	if (count >= 65536)
	{
		printf ("   GL_SEGS is too big for any port. (%d GL segs)\n", count);
	}
	else if (count >= 32768)
	{
		printf ("   GL_SEGS is too big for some ports. (%d GL segs)\n", count);
	}
}

void FProcessor::WriteGLSSect (FWadWriter &out)
{
	WriteSSectors2 (out, "GL_SSECT", Level.GLSubsectors, Level.NumGLSubsectors);
}

void FProcessor::WriteGLNodes (FWadWriter &out)
{
	WriteNodes2 (out, "GL_NODES", Level.GLNodes, Level.NumGLNodes);
}

void FProcessor::WriteBSPZ (FWadWriter &out, const char *label)
{
	ZLibOut zout (out);

	if (!CompressNodes)
	{
		printf ("   Nodes are so big that compression has been forced.\n");
	}

	out.StartWritingLump (label);
	out.AddToLump ("ZNOD", 4);
	WriteVerticesZ (zout, &Level.Vertices[Level.NumOrgVerts], Level.NumOrgVerts, Level.NumVertices - Level.NumOrgVerts);
	WriteSubsectorsZ (zout, Level.Subsectors, Level.NumSubsectors);
	WriteSegsZ (zout, Level.Segs, Level.NumSegs);
	WriteNodesZ (zout, Level.Nodes, Level.NumNodes);
}

void FProcessor::WriteGLBSPZ (FWadWriter &out, const char *label)
{
	ZLibOut zout (out);

	if (!CompressGLNodes)
	{
		printf ("   GL Nodes are so big that compression has been forced.\n");
	}

	out.StartWritingLump (label);
	out.AddToLump ("ZGLN", 4);
	WriteVerticesZ (zout, &Level.GLVertices[Level.NumOrgVerts], Level.NumOrgVerts, Level.NumGLVertices - Level.NumOrgVerts);
	WriteSubsectorsZ (zout, Level.GLSubsectors, Level.NumGLSubsectors);
	WriteGLSegsZ (zout, Level.GLSegs, Level.NumGLSegs);
	WriteNodesZ (zout, Level.GLNodes, Level.NumGLNodes);
}

void FProcessor::WriteVerticesZ (ZLibOut &out, const WideVertex *verts, int orgverts, int newverts)
{
	out << (DWORD)orgverts << (DWORD)newverts;

	for (int i = 0; i < newverts; ++i)
	{
		out << verts[i].x << verts[i].y;
	}
}

void FProcessor::WriteSubsectorsZ (ZLibOut &out, const MapSubsectorEx *subs, int numsubs)
{
	out << (DWORD)numsubs;

	for (int i = 0; i < numsubs; ++i)
	{
		out << (DWORD)subs[i].numlines;
	}
}

void FProcessor::WriteSegsZ (ZLibOut &out, const MapSeg *segs, int numsegs)
{
	out << (DWORD)numsegs;

	for (int i = 0; i < numsegs; ++i)
	{
		out << (DWORD)segs[i].v1
			<< (DWORD)segs[i].v2
			<< (WORD)segs[i].linedef
			<< (BYTE)segs[i].side;
	}
}

void FProcessor::WriteGLSegsZ (ZLibOut &out, const MapSegGLEx *segs, int numsegs)
{
	out << (DWORD)numsegs;

	for (int i = 0; i < numsegs; ++i)
	{
		out << (DWORD)segs[i].v1
			<< (DWORD)segs[i].partner
			<< (WORD)segs[i].linedef
			<< (BYTE)segs[i].side;
	}
}

void FProcessor::WriteNodesZ (ZLibOut &out, const MapNodeEx *nodes, int numnodes)
{
	out << (DWORD)numnodes;

	for (int i = 0; i < numnodes; ++i)
	{
		out << (SWORD)nodes[i].x
			<< (SWORD)nodes[i].y
			<< (SWORD)nodes[i].dx
			<< (SWORD)nodes[i].dy;
		for (int j = 0; j < 2; ++j)
		{
			for (int k = 0; k < 4; ++k)
			{
				out << (SWORD)nodes[i].bbox[j][k];
			}
		}
		out << (DWORD)nodes[i].children[0]
			<< (DWORD)nodes[i].children[1];
	}
}

// zlib lump writer ---------------------------------------------------------

ZLibOut::ZLibOut (FWadWriter &out)
	: Out (out)
{
	int err;

	Stream.next_in = Z_NULL;
	Stream.avail_in = 0;
	Stream.zalloc = Z_NULL;
	Stream.zfree = Z_NULL;
	err = deflateInit (&Stream, 9);

	if (err != Z_OK)
	{
		throw exception("Could not initialize deflate buffer.");
	}

	Stream.next_out = Buffer;
	Stream.avail_out = BUFFER_SIZE;
}

ZLibOut::~ZLibOut ()
{
	int err;

	for (;;)
	{
		err = deflate (&Stream, Z_FINISH);
		if (err != Z_OK)
		{
			break;
		}
		if (Stream.avail_out == 0)
		{
			Out.AddToLump (Buffer, BUFFER_SIZE);
			Stream.next_out = Buffer;
			Stream.avail_out = BUFFER_SIZE;
		}
	}
	deflateEnd (&Stream);
	if (err != Z_STREAM_END)
	{
		throw exception("Error deflating data.");
	}
	Out.AddToLump (Buffer, BUFFER_SIZE - Stream.avail_out);
}

void ZLibOut::Write (BYTE *data, int len)
{
	int err;

	Stream.next_in = data;
	Stream.avail_in = len;
	err = deflate (&Stream, 0);
	while (Stream.avail_out == 0 && err == Z_OK)
	{
		Out.AddToLump (Buffer, BUFFER_SIZE);
		Stream.next_out = Buffer;
		Stream.avail_out = BUFFER_SIZE;
		if (Stream.avail_in != 0)
		{
			err = deflate (&Stream, 0);
		}
	}
	if (err != Z_OK)
	{
		throw exception("Error deflating data.");
	}
}

ZLibOut &ZLibOut::operator << (BYTE val)
{
	Write (&val, 1);
	return *this;
}

ZLibOut &ZLibOut::operator << (WORD val)
{
	val = SHORT(val);
	Write ((BYTE *)&val, 2);
	return *this;
}

ZLibOut &ZLibOut::operator << (SWORD val)
{
	val = SHORT(val);
	Write ((BYTE *)&val, 2);
	return *this;
}

ZLibOut &ZLibOut::operator << (DWORD val)
{
	val = LONG(val);
	Write ((BYTE *)&val, 4);
	return *this;
}

ZLibOut &ZLibOut::operator << (fixed_t val)
{
	val = LONG(val);
	Write ((BYTE *)&val, 4);
	return *this;
}