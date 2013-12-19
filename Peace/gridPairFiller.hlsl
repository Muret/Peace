

#include "simulationH.hlsli"

//fills the (gridkey - particleID) buffer



RWStructuredBuffer<GridKeyStructure> gridKeyBuffer : register( u0 );

StructuredBuffer<ParticleStructure> particleBuffer : register( t0 );


[numthreads(WARP_GROUP_SIZE, 1, 1)]
void main( uint3 Gid : SV_GroupID, 
                  uint3 DTid : SV_DispatchThreadID, 
                  uint3 GTid : SV_GroupThreadID, 
                  uint GI : SV_GroupIndex )
{
	uint particle_id = DTid.x;
	float3 particle_position = particleBuffer[particle_id].position.xyz;

	uint3 gridIndex = getGrindIndex(particle_position);
	uint gridKey = getGridKey(gridIndex);

	gridKeyBuffer[particle_id].gridKey = gridKey;
	gridKeyBuffer[particle_id].particleIndex = particle_id;
}