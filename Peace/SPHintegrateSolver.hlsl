#include "simulationH.hlsli"

RWStructuredBuffer<ParticleStructure> particleBuffer			: register(u0);

StructuredBuffer<ForceStructure> forceBuffer				: register(t0);



[numthreads(SIMULATION_BLOCK_SIZE, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	const unsigned int P_ID = DTid.x; // Particle ID to operate on

	float3 position = particleBuffer[P_ID].position.xyz;
	float3 velocity = particleBuffer[P_ID].velocity.xyz;
	float3 acceleration = forceBuffer[P_ID].acceleration.xyz;

	// Apply the forces from the map walls
	[unroll]
	for (unsigned int i = 0; i < 6; i++)
	{
		float dist = dot(float4(position,1), g_vPlanes[i]);
		float3 force_from_wall = min(dist, 0) * -g_fWallStiffness * g_vPlanes[i].xyz;
		//if (dist < 0)
		{
			//velocity = reflect(velocity, g_vPlanes[i].xyz) * 0.90;
		}
		acceleration += force_from_wall;
	}

	// Apply gravity
	acceleration += g_vGravity.xyz;

	// Integrate
	velocity += 0.003f * acceleration;
	position += 0.003f * velocity;


	// Update
	particleBuffer[P_ID].position = float4(position,1);
	particleBuffer[P_ID].velocity = float4(velocity,0);
}
