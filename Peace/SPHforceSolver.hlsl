
#include "simulationH.hlsli"

RWStructuredBuffer<ForceStructure> forceBuffer	: register(u0);

StructuredBuffer<ParticleStructure> particleBuffer			: register(t0);
StructuredBuffer<GridKeyStructure> gridKeyBuffer			: register(t1);
StructuredBuffer<GridBorderStructure> gridBorderBuffer		: register(t2);
StructuredBuffer<DensityStructure> densityBuffer			: register(t3);

float CalculatePressure(float density)
{
	// Implements this equation:
	// Pressure = B * ((rho / rho_0)^y  - 1)
	float pow_inner = density / g_fRestDensity;
	float max_inner = pow(pow_inner , 3.0f);
	return g_fPressureStiffness * max(max_inner - 1.0f, 0);
}

float3 CalculateGradPressure(float r, float P_pressure, float N_pressure, float N_density, float3 diff)
{
	const float h = g_fSmoothlen;
	float avg_pressure = 0.5f * (N_pressure + P_pressure);
	// Implements this equation:
	// W_spkiey(r, h) = 15 / (pi * h^6) * (h - r)^3
	// GRAD( W_spikey(r, h) ) = -45 / (pi * h^6) * (h - r)^2
	// g_fGradPressureCoef = fParticleMass * -45.0f / (PI * fSmoothlen^6)
	return g_fGradPressureCoef * avg_pressure / N_density * (h - r) * (h - r) / r * (diff);
}

float3 CalculateLapVelocity(float r, float3 P_velocity, float3 N_velocity, float N_density)
{
	const float h = g_fSmoothlen;
	float3 vel_diff = (N_velocity - P_velocity);

	// Implements this equation:
	// W_viscosity(r, h) = 15 / (2 * pi * h^3) * (-r^3 / (2 * h^3) + r^2 / h^2 + h / (2 * r) - 1)
	// LAPLACIAN( W_viscosity(r, h) ) = 45 / (pi * h^6) * (h - r)
	// g_fLapViscosityCoef = fParticleMass * fViscosity * 45.0f / (PI * fSmoothlen^6)
	return (g_fLapViscosityCoef / N_density) * (h - r) * vel_diff;
}

[numthreads(SIMULATION_BLOCK_SIZE, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	const unsigned int P_ID = DTid.x; // Particle ID to operate on

	float3 P_position = particleBuffer[P_ID].position.xyz;
		float3 P_velocity = particleBuffer[P_ID].velocity.xyz;
	float P_density = densityBuffer[P_ID].density;
	float P_pressure = CalculatePressure(P_density);

	const float h_sq = g_fSmoothlen * g_fSmoothlen;

	float3 acceleration = float3(0.0f, 0.0f, 0.0f);

		// Calculate the acceleration based on neighbors from the 8 adjacent cells + current cell
		uint3 G_XYZ = getGrindIndex(P_position);
		float count = 0;
	[loop]
	for (uint Y = max(G_XYZ.y - 1, 0); Y <= min(G_XYZ.y + 1, GRID_DIMENSION - 1); Y++)
	{
		[loop]
		for (uint X = max(G_XYZ.x - 1, 0); X <= min(G_XYZ.x + 1, GRID_DIMENSION - 1); X++)
		{
			[loop]
			for (uint Z = max(G_XYZ.z - 1, 0); Z <= min(G_XYZ.z + 1, GRID_DIMENSION - 1); Z++)
			{
				unsigned int G_CELL = getGridKey(uint3(X, Y, Z));
				uint gridStart = gridBorderBuffer[G_CELL].gridStart;
				uint gridEnd = gridBorderBuffer[G_CELL].gridEnd;
				[loop]
				for (unsigned int N_ID = gridStart; N_ID < gridEnd; N_ID++)
				{
					uint N_P_ID = gridKeyBuffer[N_ID].particleIndex;
					float3 N_position = particleBuffer[N_P_ID].position.xyz;

						float3 diff = N_position - P_position;
						float r_sq = dot(diff, diff);
					if (r_sq < h_sq && P_ID != N_P_ID)
					{
						float3 N_velocity = particleBuffer[N_P_ID].velocity.xyz;
						float N_density = densityBuffer[N_P_ID].density;
						float N_pressure = CalculatePressure(N_density);
						float r = sqrt(r_sq);

						// Pressure Term
	
						acceleration += CalculateGradPressure(r, P_pressure, N_pressure, N_density, diff);
						acceleration += CalculateLapVelocity(r, P_velocity, N_velocity, N_density);

						//count++;
					}
				}
			}
		}
	}

	acceleration /= float3(P_density, P_density, P_density);
	//acceleration.y = 0;
	//acceleration = float3(count, count, count);
	forceBuffer[P_ID].acceleration = float4(acceleration, 0);

}