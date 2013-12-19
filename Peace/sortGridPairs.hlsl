

#include "simulationH.hlsli"

RWStructuredBuffer<GridKeyStructure> gridKeyBuffer : register( u0 );

groupshared GridKeyStructure shared_data[WARP_GROUP_SIZE];


[numthreads(WARP_GROUP_SIZE , 1, 1)]
void main( uint3 Gid : SV_GroupID, 
                  uint3 DTid : SV_DispatchThreadID, 
                  uint3 GTid : SV_GroupThreadID, 
                  uint GI : SV_GroupIndex )
{
	

    // Load shared data
    shared_data[GI] = gridKeyBuffer[DTid.x];
    GroupMemoryBarrierWithGroupSync();

	// Sort the shared data
	[loop]
    for (unsigned int j = g_sortLevel >> 1 ; j > 0 ; j >>= 1)
    {
		bool which_to_take = ( (shared_data[GI & ~j].gridKey <= shared_data[GI | j].gridKey) == (bool)(g_sortAlternateMask &  DTid.x));

        unsigned int gridKeyResult = which_to_take ? shared_data[GI ^ j].gridKey : shared_data[GI].gridKey;
        unsigned int particleIndexResult = which_to_take ? shared_data[GI ^ j].particleIndex : shared_data[GI].particleIndex;


		GroupMemoryBarrierWithGroupSync();
        
		shared_data[GI].gridKey = gridKeyResult;
		shared_data[GI].particleIndex = particleIndexResult;

		GroupMemoryBarrierWithGroupSync();
    }
    
	// Store shared data
    gridKeyBuffer[DTid.x] = shared_data[GI];

}