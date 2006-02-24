/*
    Routines for extracting usable data from the new BSP tree.
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
#include <string.h>
#include <stdio.h>

#include "zdbsp.h"
#include "nodebuild.h"
#include "templates.h"

void FNodeBuilder::GetGLNodes (MapNodeEx *&outNodes, int &nodeCount,
	MapSegGLEx *&outSegs, int &segCount,
	MapSubsectorEx *&outSubs, int &subCount)
{
	TArray<MapSegGLEx> segs (Segs.Size()*5/4);
	int i, j, k;

	nodeCount = Nodes.Size ();
	outNodes = new MapNodeEx[nodeCount];
	for (i = 0; i < nodeCount; ++i)
	{
		const node_t *orgnode = &Nodes[i];
		MapNodeEx *newnode = &outNodes[i];

		newnode->x = short(orgnode->x >> FRACBITS);
		newnode->y = short(orgnode->y >> FRACBITS);
		newnode->dx = short(orgnode->dx >> FRACBITS);
		newnode->dy = short(orgnode->dy >> FRACBITS);

		for (j = 0; j < 2; ++j)
		{
			for (k = 0; k < 4; ++k)
			{
				newnode->bbox[j][k] = orgnode->bbox[j][k] >> FRACBITS;
			}
			newnode->children[j] = orgnode->intchildren[j];
		}
	}

	subCount = Subsectors.Size();
	outSubs = new MapSubsectorEx[subCount];
	for (i = 0; i < subCount; ++i)
	{
		int numsegs = CloseSubsector (segs, i);
		outSubs[i].numlines = numsegs;
		outSubs[i].firstline = segs.Size() - numsegs;
	}

	segCount = segs.Size ();
	outSegs = new MapSegGLEx[segCount];
	memcpy (outSegs, &segs[0], segCount*sizeof(MapSegGLEx));

	for (i = 0; i < segCount; ++i)
	{
		if (outSegs[i].partner != DWORD_MAX)
		{
			outSegs[i].partner = Segs[outSegs[i].partner].storedseg;
		}
	}
}

int FNodeBuilder::CloseSubsector (TArray<MapSegGLEx> &segs, int subsector)
{
	FPrivSeg *seg, *prev;
	angle_t prevAngle;
	double accumx, accumy;
	fixed_t midx, midy;
	int i, j, first, max, count, firstVert;

	first = Subsectors[subsector].firstline;
	max = first + Subsectors[subsector].numlines;
	count = 0;

	accumx = accumy = 0.0;

	for (i = first; i < max; ++i)
	{
		seg = &Segs[SegList[i].SegNum];
		accumx += double(Vertices[seg->v1].x) + double(Vertices[seg->v2].x);
		accumy += double(Vertices[seg->v1].y) + double(Vertices[seg->v2].y);
	}

	midx = fixed_t(accumx / (max - first) / 2);
	midy = fixed_t(accumy / (max - first) / 2);

	seg = &Segs[SegList[first].SegNum];
	prevAngle = PointToAngle (Vertices[seg->v1].x - midx, Vertices[seg->v1].y - midy);
	seg->storedseg = PushGLSeg (segs, seg);
	count = 1;
	prev = seg;
	firstVert = seg->v1;

#if 0
	printf("--%d--\n", subsector);
	for (j = first; j < max; ++j)
	{
		seg = &Segs[SegList[j].SegNum];
		angle_t ang = PointToAngle (Vertices[seg->v1].x - midx, Vertices[seg->v1].y - midy);
		printf ("%d: %5d(%5d,%5d)->%5d(%5d,%5d) - %3.3f\n", j,
			seg->v1, Vertices[seg->v1].x>>16, Vertices[seg->v1].y>>16,
			seg->v2, Vertices[seg->v2].x>>16, Vertices[seg->v2].y>>16,
			double(ang/2)*180/(1<<30));
	}
#endif

	for (i = first + 1; i < max; ++i)
	{
		angle_t bestdiff = ANGLE_MAX;
		FPrivSeg *bestseg = NULL;
		int bestj = -1;
		for (j = first; j < max; ++j)
		{
			seg = &Segs[SegList[j].SegNum];
			angle_t ang = PointToAngle (Vertices[seg->v1].x - midx, Vertices[seg->v1].y - midy);
			angle_t diff = prevAngle - ang;
			if (seg->v1 == prev->v2)
			{
				bestdiff = diff;
				bestseg = seg;
				bestj = j;
				break;
			}
			if (diff < bestdiff && diff > 0)
			{
				bestdiff = diff;
				bestseg = seg;
				bestj = j;
			}
		}
		if (bestseg != NULL)
		{
			seg = bestseg;
		}
		if (prev->v2 != seg->v1)
		{
			// Add a new miniseg to connect the two segs
			PushConnectingGLSeg (subsector, segs, prev->v2, seg->v1);
			count++;
		}
#if 0
		printf ("+%d\n", bestj);
#endif
		prevAngle -= bestdiff;
		seg->storedseg = PushGLSeg (segs, seg);
		count++;
		prev = seg;
		if (seg->v2 == firstVert)
		{
			prev = seg;
			break;
		}
	}
#if 0
	printf ("\n");
#endif

	if (prev->v2 != firstVert)
	{
		PushConnectingGLSeg (subsector, segs, prev->v2, firstVert);
		count++;
	}

	return count;
}

DWORD FNodeBuilder::PushGLSeg (TArray<MapSegGLEx> &segs, const FPrivSeg *seg)
{
	MapSegGLEx newseg;

	newseg.v1 = seg->v1;
	newseg.v2 = seg->v2;
	newseg.linedef = seg->linedef;
	newseg.side = newseg.linedef != NO_INDEX
		? Level.Lines[newseg.linedef].sidenum[1] == seg->sidedef ? 1 : 0
		: 0;
	newseg.partner = seg->partner;
	return segs.Push (newseg);
}

void FNodeBuilder::PushConnectingGLSeg (int subsector, TArray<MapSegGLEx> &segs, int v1, int v2)
{
	MapSegGLEx newseg;

	Warn ("Unclosed subsector %d, from (%d,%d) to (%d,%d)\n", subsector,
		Vertices[v1].x >> FRACBITS, Vertices[v1].y >> FRACBITS,
		Vertices[v2].x >> FRACBITS, Vertices[v2].y >> FRACBITS);

	newseg.v1 = v1;
	newseg.v2 = v2;
	newseg.linedef = NO_INDEX;
	newseg.side = 0;
	newseg.partner = DWORD_MAX;
	segs.Push (newseg);
}

void FNodeBuilder::GetVertices (WideVertex *&verts, int &count)
{
	count = Vertices.Size ();
	verts = new WideVertex[count];

	for (int i = 0; i < count; ++i)
	{
		verts[i].x = Vertices[i].x;
		verts[i].y = Vertices[i].y;
	}
}

void FNodeBuilder::GetNodes (MapNodeEx *&outNodes, int &nodeCount,
	MapSeg *&outSegs, int &segCount,
	MapSubsectorEx *&outSubs, int &subCount)
{
	short bbox[4];
	TArray<MapSeg> segs (Segs.Size());

	// Walk the BSP and create a new BSP with only the information
	// suitable for a standard tree. At a minimum, this means removing
	// all minisegs. As an optional step, I also recompute all the
	// nodes' bounding boxes so that they only bound the real segs and
	// not the minisegs.

	nodeCount = Nodes.Size ();
	outNodes = new MapNodeEx[nodeCount];

	subCount = Subsectors.Size ();
	outSubs = new MapSubsectorEx[subCount];

	RemoveMinisegs (outNodes, segs, outSubs, Nodes.Size() - 1, bbox);

	segCount = segs.Size ();
	outSegs = new MapSeg[segCount];
	memcpy (outSegs, &segs[0], segCount*sizeof(MapSeg));
}

int FNodeBuilder::RemoveMinisegs (MapNodeEx *nodes,
	TArray<MapSeg> &segs, MapSubsectorEx *subs, int node, short bbox[4])
{
	if (node & NFX_SUBSECTOR)
	{
		int subnum = node == -1 ? 0 : node & ~NFX_SUBSECTOR;
		int numsegs = StripMinisegs (segs, subnum, bbox);
		subs[subnum].numlines = numsegs;
		subs[subnum].firstline = segs.Size() - numsegs;
		return NFX_SUBSECTOR | subnum;
	}
	else
	{
		const node_t *orgnode = &Nodes[node];
		MapNodeEx *newnode = &nodes[node];

		int child0 = RemoveMinisegs (nodes, segs, subs, orgnode->intchildren[0], newnode->bbox[0]);
		int child1 = RemoveMinisegs (nodes, segs, subs, orgnode->intchildren[1], newnode->bbox[1]);


		newnode->x = orgnode->x >> FRACBITS;
		newnode->y = orgnode->y >> FRACBITS;
		newnode->dx = orgnode->dx >> FRACBITS;
		newnode->dy = orgnode->dy >> FRACBITS;
		newnode->children[0] = child0;
		newnode->children[1] = child1;

		bbox[BOXTOP] = MAX(newnode->bbox[0][BOXTOP], newnode->bbox[1][BOXTOP]);
		bbox[BOXBOTTOM] = MIN(newnode->bbox[0][BOXBOTTOM], newnode->bbox[1][BOXBOTTOM]);
		bbox[BOXLEFT] = MIN(newnode->bbox[0][BOXLEFT], newnode->bbox[1][BOXLEFT]);
		bbox[BOXRIGHT] = MAX(newnode->bbox[0][BOXRIGHT], newnode->bbox[1][BOXRIGHT]);

		return node;
	}
}

int FNodeBuilder::StripMinisegs (TArray<MapSeg> &segs, int subsector, short bbox[4])
{
	int count, i, max;

	// The bounding box is recomputed to only cover the real segs and not the
	// minisegs in the subsector.
	bbox[BOXTOP] = -32768;
	bbox[BOXBOTTOM] = 32767;
	bbox[BOXLEFT] = 32767;
	bbox[BOXRIGHT] = -32768;

	i = Subsectors[subsector].firstline;
	max = Subsectors[subsector].numlines + i;

	for (count = 0; i < max; ++i)
	{
		const FPrivSeg *org = &Segs[SegList[i].SegNum];

		// Because of the ordering guaranteed by SortSegs(), all mini segs will
		// be at the end of the subsector, so once one is encountered, we can
		// stop right away.
		if (org->linedef == -1)
		{
			break;
		}
		else
		{
			MapSeg newseg;

			AddSegToShortBBox (bbox, org);

			newseg.v1 = org->v1;
			newseg.v2 = org->v2;
			newseg.angle = org->angle >> 16;
			newseg.offset = org->offset >> FRACBITS;
			newseg.linedef = org->linedef;
			newseg.side = Level.Lines[org->linedef].sidenum[1] == org->sidedef ? 1 : 0;
			segs.Push (newseg);
			++count;
		}
	}
	return count;
}

void FNodeBuilder::AddSegToShortBBox (short bbox[4], const FPrivSeg *seg)
{
	const FPrivVert *v1 = &Vertices[seg->v1];
	const FPrivVert *v2 = &Vertices[seg->v2];

	short v1x = v1->x >> FRACBITS;
	short v1y = v1->y >> FRACBITS;
	short v2x = v2->x >> FRACBITS;
	short v2y = v2->y >> FRACBITS;

	if (v1x < bbox[BOXLEFT])	bbox[BOXLEFT] = v1x;
	if (v1x > bbox[BOXRIGHT])	bbox[BOXRIGHT] = v1x;
	if (v1y < bbox[BOXBOTTOM])	bbox[BOXBOTTOM] = v1y;
	if (v1y > bbox[BOXTOP])		bbox[BOXTOP] = v1y;

	if (v2x < bbox[BOXLEFT])	bbox[BOXLEFT] = v2x;
	if (v2x > bbox[BOXRIGHT])	bbox[BOXRIGHT] = v2x;
	if (v2y < bbox[BOXBOTTOM])	bbox[BOXBOTTOM] = v2y;
	if (v2y > bbox[BOXTOP])		bbox[BOXTOP] = v2y;
}
