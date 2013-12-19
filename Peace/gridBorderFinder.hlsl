

#include "simulationH.hlsli"

StructuredBuffer<GridKeyStructure> Input : register( t0 );

RWStructuredBuffer<GridBorderStructure> Data : register(u0);

[numthreads(WARP_GROUP_SIZE, 1, 1)]
void main( uint3 Gid : SV_GroupID, 
                  uint3 DTid : SV_DispatchThreadID, 
                  uint3 GTid : SV_GroupThreadID, 
                  uint GI : SV_GroupIndex )
{
	const unsigned int G_ID = DTid.x; // Grid ID to operate on
	
	unsigned int G_ID_PREV = (G_ID == 0) ? PARTICLE_COUNT : G_ID; G_ID_PREV--;
	unsigned int G_ID_NEXT = (G_ID + 1);

	if (G_ID_NEXT == PARTICLE_COUNT)
	{
		G_ID_NEXT = 0;
	}

	unsigned int cell = Input[G_ID].gridKey;
	
	unsigned int cell_prev = Input[G_ID_PREV].gridKey;
	if (cell != cell_prev)
	{
		// I'm the start of a cell
		Data[cell].gridStart = G_ID;
	}

	unsigned int cell_next = Input[G_ID_NEXT].gridKey;
	if (cell != cell_next)
	{
		// I'm the end of a cell
		Data[cell].gridEnd = G_ID + 1;
	}

}