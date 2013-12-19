

#include "simulationH.hlsli"

RWStructuredBuffer<DensityStructure> particleDensityBuffer	: register(u0);

StructuredBuffer<ParticleStructure> particleBuffer	: register(t0);
StructuredBuffer<GridKeyStructure> gridKeyBuffer	: register(t1);
StructuredBuffer<GridBorderStructure> gridBorderBuffer	: register(t2);


float CalculateDensity(float r_sq)
{
	const float h_sq = g_fSmoothlen * g_fSmoothlen;
	// Implements this equation:
	// W_poly6(r, h) = 315 / (64 * pi * h^9) * (h^2 - r^2)^3
	// g_fDensityCoef = fParticleMass * 315.0f / (64.0f * PI * fSmoothlen^9)
	float distance = max(h_sq - r_sq,0);
	return max( g_fDensityCoef * (distance) * (distance) * (distance) , 0);
}


[numthreads(SIMULATION_BLOCK_SIZE, 1, 1)]
void main( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
	const unsigned int P_ID = DTid.x;
    const float h_sq = g_fSmoothlen * g_fSmoothlen;
	float3 P_position = particleBuffer[P_ID].position;
	uint3 grid_tuple = getGrindIndex(P_position);

    float density = 0;
    // Calculate the density based on neighbors from the 27 adjacent cells + current cell
	[loop]
	for (int Y = max(grid_tuple.y - 1, 0); Y <= min(grid_tuple.y + 1, GRID_DIMENSION - 1); Y++)
    {
		[loop]
		for (uint X =  max(grid_tuple.x - 1, 0); X <= min(grid_tuple.x + 1, GRID_DIMENSION - 1); X++)
        {
			[loop]
			for (uint Z =  max(grid_tuple.z - 1, 0); Z <=  min(grid_tuple.z + 1, GRID_DIMENSION - 1); Z++)
			{

				uint cell_id = getGridKey(uint3(X, Y, Z));
				uint grid_border_start = gridBorderBuffer[cell_id].gridStart;
				uint grid_border_end = gridBorderBuffer[cell_id].gridEnd;
				[loop]
				for (unsigned int N_ID = grid_border_start; N_ID < grid_border_end; N_ID++)
				{
					uint particle_id = gridKeyBuffer[N_ID].particleIndex;
					float3 N_position = particleBuffer[particle_id].position;
                
					float3 diff = N_position - P_position;
					float r_sq = dot(diff, diff);
					if (r_sq < h_sq)
					{
						density += CalculateDensity(r_sq);
					}
				}
			}

        }
    }

	particleDensityBuffer[P_ID].density = density;
}