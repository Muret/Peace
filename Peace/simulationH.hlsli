
#include "simulationConstants.h"

//constant variables
cbuffer GridCostantBuffer: register (b0)
{
	float g_GridDimensionCube;
	float g_GridDimensionSquare;
	float g_GridDimension;
	float3 g_InverseGridDimension;
	float3 g_MiddleGridOffset;
	float3 padding;
};

cbuffer SortConstantBuffer : register (b1)
{
    unsigned int g_sortLevel;
    unsigned int g_sortAlternateMask;
    unsigned int g_iWidth;
    unsigned int g_iHeight;

	float4 padding2;
};

cbuffer cbSimulationConstants : register(b2)
{
	float g_fTimeStep;
	float g_fDensityCoef;
	float g_fGradPressureCoef;
	float g_fLapViscosityCoef;

	float4 g_vGravity;
	float4 g_vPlanes[6];
};




//buffer structures
struct ParticleStructure
{
	float4 position;
	float4 velocity;
};

//grid key structure
struct GridKeyStructure
{
	uint gridKey;
	uint particleIndex;
};

//grid border structure
struct GridBorderStructure
{
	uint gridStart;
	uint gridEnd;
};

struct ForceStructure
{
	float4 acceleration;
};

struct DensityStructure
{
	float density;
};

uint getGridKey(uint3 gridIndex)
{
	uint x_value = gridIndex.x;
	uint y_value = GRID_DIMENSION * gridIndex.y;
	uint z_value = (GRID_DIMENSION * GRID_DIMENSION) * gridIndex.z;

	return uint(x_value + y_value +  z_value);
}


uint3 getGrindIndex(float3 particle_position)
{
	uint half_dimension = GRID_DIMENSION * 0.5f;
	float3 scaler = float3(1.0f / g_fSmoothlen, 1.0f / g_fSmoothlen, 1.0f / g_fSmoothlen);
		uint3 grid_cord = float3(scaler * particle_position) + uint3(half_dimension, half_dimension, half_dimension);
	
	return clamp(grid_cord, 0, uint3(GRID_DIMENSION - 1, GRID_DIMENSION - 1, GRID_DIMENSION - 1));
}
