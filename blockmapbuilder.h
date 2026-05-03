#include "doomdata.h"
#include "workdata.h"
#include "tarray.h"

class FBlockmapBuilder
{
public:
	FBlockmapBuilder(FLevel &level);
	uint32_t *GetBlockmap(int &size) const;

private:
	FLevel &Level;
	TArray<uint32_t> BlockMap;

	void BuildBlockmap();
	void CreateUnpackedBlockmap(TArray<uint32_t> *blocks, int bmapwidth, int bmapheight);
	void CreatePackedBlockmap(TArray<uint32_t> *blocks, int bmapwidth, int bmapheight);
};
