
#include "simulationH.hlsli"

#define PI 3.14f



RWStructuredBuffer<ParticleStructure> particleBuffer : register(t0);


[numthreads(WARP_GROUP_SIZE, 1, 1)]
void main( uint3 Gid : SV_GroupID, 
                  uint3 DTid : SV_DispatchThreadID, 
                  uint3 GTid : SV_GroupThreadID, 
                  uint GI : SV_GroupIndex )
{
	uint index = DTid.x;

	float phase = (float)DTid.x * 2.0f * PI / PARTICLE_COUNT;

	float2 displacementV = float2 ( sin(phase)  , cos(phase)) * 0.001;
		
	float2 old_position = particleBuffer[index].position.xy;
	
	float2 new_position = old_position + displacementV;

    particleBuffer[index].position.xy = new_position;

}