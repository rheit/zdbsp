#include "doomdata.h"
#include "workdata.h"
#include "tarray.h"

struct FEventInfo
{
	int Vertex;
	DWORD FrontSeg;
};

struct FEvent
{
	FEvent *Parent, *Left, *Right;
	enum { RED, BLACK } Color;
	double Distance;
	FEventInfo Info;
};

class FEventTree
{
public:
	FEventTree ();
	~FEventTree ();

	FEvent *GetMinimum ();
	FEvent *GetSuccessor (FEvent *event) const { FEvent *node = Successor(event); return node == &Nil ? NULL : node; }
	FEvent *GetPredecessor (FEvent *event) const { FEvent *node = Predecessor(event); return node == &Nil ? NULL : node; }

	FEvent *GetNewNode ();
	void Insert (FEvent *event);
	void Delete (FEvent *event);
	FEvent *FindEvent (double distance) const;
	void DeleteAll ();

	void PrintTree () const { PrintTree (Root); }

private:
	FEvent Nil;
	FEvent *Root;
	FEvent *Spare;

	void LeftRotate (FEvent *event);
	void RightRotate (FEvent *event);
	void DeleteFixUp (FEvent *event);
	void DeletionTraverser (FEvent *event);
	FEvent *Successor (FEvent *event) const;
	FEvent *Predecessor (FEvent *event) const;

	void PrintTree (const FEvent *event) const;
};

class FNodeBuilder
{
	struct FPrivSeg
	{
		int v1, v2;
		int sidedef;
		int linedef;
		int frontsector;
		int backsector;
		DWORD next;
		DWORD nextforvert;
		DWORD nextforvert2;
		int loopnum;		// loop number for split avoidance (0 means splitting is okay)
		DWORD partner;		// seg on back side
		DWORD storedseg;	// seg # in the GL_SEGS lump
		angle_t angle;
		fixed_t offset;

		int planenum;
		bool planefront;
		FPrivSeg *hashnext;
	};
	struct FPrivVert
	{
		fixed_t x, y;
		DWORD segs;		// segs that use this vertex as v1
		DWORD segs2;	// segs that use this vertex as v2

		bool operator== (const FPrivVert &other)
		{
			return x == other.x && y == other.y;
		}
	};
	struct FSimpleLine
	{
		fixed_t x, y, dx, dy;
	};
	union USegPtr
	{
		DWORD SegNum;
		FPrivSeg *SegPtr;
	};
	struct FSplitSharer
	{
		double Distance;
		DWORD Seg;
		bool Forward;
	};

	// Like a blockmap, but for vertices instead of lines
	class FVertexMap
	{
	public:
		FVertexMap (FNodeBuilder &builder, fixed_t minx, fixed_t miny, fixed_t maxx, fixed_t maxy);
		~FVertexMap ();

		int SelectVertexExact (FPrivVert &vert);
		int SelectVertexClose (FPrivVert &vert);

	private:
		FNodeBuilder &MyBuilder;
		TArray<int> *VertexGrid;

		fixed_t MinX, MinY, MaxX, MaxY;
		int BlocksWide, BlocksTall;

		enum { BLOCK_SIZE = 256 << FRACBITS };
		int InsertVertex (FPrivVert &vert);
		inline int GetBlock (fixed_t x, fixed_t y)
		{
			assert (x >= MinX);
			assert (y >= MinY);
			assert (x <= MaxX);
			assert (y <= MaxY);
			return (x - MinX) / BLOCK_SIZE + ((y - MinY) / BLOCK_SIZE) * BlocksWide;
		}
	};


public:
	struct FPolyStart
	{
		int polynum;
		fixed_t x, y;
	};

	FNodeBuilder (FLevel &level,
		TArray<FPolyStart> &polyspots, TArray<FPolyStart> &anchors,
		const char *name, bool makeGLnodes);
	~FNodeBuilder ();

	void GetVertices (WideVertex *&verts, int &count);
	void GetNodes (MapNodeEx *&nodes, int &nodeCount,
		MapSeg *&segs, int &segCount,
		MapSubsectorEx *&ssecs, int &subCount);

	void GetGLNodes (MapNodeEx *&nodes, int &nodeCount,
		MapSegGLEx *&segs, int &segCount,
		MapSubsectorEx *&ssecs, int &subCount);

	//  < 0 : in front of line
	// == 0 : on line
	//  > 0 : behind line

	static int PointOnSide (int x, int y, int x1, int y1, int dx, int dy);

private:
	FVertexMap *VertexMap;

	TArray<node_t> Nodes;
	TArray<subsector_t> Subsectors;
	TArray<DWORD> SubsectorSets;
	TArray<FPrivSeg> Segs;
	TArray<FPrivVert> Vertices;
	TArray<USegPtr> SegList;
	TArray<BYTE> PlaneChecked;
	TArray<FSimpleLine> Planes;
	size_t InitialVertices;	// Number of vertices in a map that are connected to linedefs

	TArray<int> Touched;	// Loops a splitter touches on a vertex
	TArray<int> Colinear;	// Loops with edges colinear to a splitter
	FEventTree Events;		// Vertices intersected by the current splitter
	TArray<FSplitSharer> SplitSharers;	// Segs collinear with the current splitter

	DWORD HackSeg;			// Seg to force to back of splitter
	DWORD HackMate;			// Seg to use in front of hack seg
	FLevel &Level;
	bool GLNodes;

	// Progress meter stuff
	int SegsStuffed;
	const char *MapName;

	void FindUsedVertices (WideVertex *vertices, int max);
	void BuildTree ();
	void MakeSegsFromSides ();
	int CreateSeg (int linenum, int sidenum);
	void GroupSegPlanes ();
	void FindPolyContainers (TArray<FPolyStart> &spots, TArray<FPolyStart> &anchors);
	bool GetPolyExtents (int polynum, fixed_t bbox[4]);
	int MarkLoop (DWORD firstseg, int loopnum);
	void AddSegToBBox (fixed_t bbox[4], const FPrivSeg *seg);
	DWORD CreateNode (DWORD set, fixed_t bbox[4]);
	DWORD CreateSubsector (DWORD set, fixed_t bbox[4]);
	void CreateSubsectorsForReal ();
	bool CheckSubsector (DWORD set, node_t &node, DWORD &splitseg, int setsize);
	bool CheckSubsectorOverlappingSegs (DWORD set, node_t &node, DWORD &splitseg);
	bool ShoveSegBehind (DWORD set, node_t &node, DWORD seg, DWORD mate);
	int SelectSplitter (DWORD set, node_t &node, DWORD &splitseg, int step, bool nosplit);
	void SplitSegs (DWORD set, node_t &node, DWORD splitseg, DWORD &outset0, DWORD &outset1);
	DWORD SplitSeg (DWORD segnum, int splitvert, int v1InFront);
	int Heuristic (node_t &node, DWORD set, bool honorNoSplit);
	int ClassifyLine (node_t &node, const FPrivSeg *seg, int &sidev1, int &sidev2);
	int CountSegs (DWORD set) const;

	void FixSplitSharers ();
	double AddIntersection (const node_t &node, int vertex);
	void AddMinisegs (const node_t &node, DWORD splitseg, DWORD &fset, DWORD &rset);
	DWORD CheckLoopStart (fixed_t dx, fixed_t dy, int vertex1, int vertex2);
	DWORD CheckLoopEnd (fixed_t dx, fixed_t dy, int vertex2);
	void RemoveSegFromVert1 (DWORD segnum, int vertnum);
	void RemoveSegFromVert2 (DWORD segnum, int vertnum);
	DWORD AddMiniseg (int v1, int v2, DWORD partner, DWORD seg1, DWORD splitseg);
	void SetNodeFromSeg (node_t &node, const FPrivSeg *pseg) const;

	int RemoveMinisegs (MapNodeEx *nodes, TArray<MapSeg> &segs, MapSubsectorEx *subs, int node, short bbox[4]);
	int StripMinisegs (TArray<MapSeg> &segs, int subsector, short bbox[4]);
	void AddSegToShortBBox (short bbox[4], const FPrivSeg *seg);
	int CloseSubsector (TArray<MapSegGLEx> &segs, int subsector);
	DWORD PushGLSeg (TArray<MapSegGLEx> &segs, const FPrivSeg *seg);
	void PushConnectingGLSeg (int subsector, TArray<MapSegGLEx> &segs, int v1, int v2);
	int OutputDegenerateSubsector (TArray<MapSegGLEx> &segs, int subsector, bool bForward, double lastdot, FPrivSeg *&prev); 

	static int SortSegs (const void *a, const void *b);

	double InterceptVector (const node_t &splitter, const FPrivSeg &seg);

	void PrintSet (int l, DWORD set);
};
