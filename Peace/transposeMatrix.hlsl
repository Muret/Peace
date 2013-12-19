

#include "simulationH.hlsli"


StructuredBuffer<GridKeyStructure> Input : register( t0 );
RWStructuredBuffer<GridKeyStructure> Data : register( u0 );


//--------------------------------------------------------------------------------------
// Matrix Transpose Compute Shader
//--------------------------------------------------------------------------------------
//groupshared GridKeyStructure transpose_shared_data[TRANSPOSE_BLOCK_SIZE * TRANSPOSE_BLOCK_SIZE];

[numthreads(1, 1, 1)]
void main(uint3 Gid : SV_GroupID,
	uint3 DTid : SV_DispatchThreadID,
	uint3 GTid : SV_GroupThreadID,
	uint GI : SV_GroupIndex)
{
	//transpose_shared_data[GI].gridKey = Input[DTid.y * g_iWidth + DTid.x].gridKey;
	//transpose_shared_data[GI].particleIndex = Input[DTid.y * g_iWidth + DTid.x].particleIndex;
	
	GridKeyStructure temp = Input[DTid.y * g_iWidth + DTid.x];

	//GroupMemoryBarrierWithGroupSync();

	//uint2 XY = DTid.yx - GTid.yx + GTid.xy;
		//Data[XY.y * g_iHeight + XY.x].gridKey = transpose_shared_data[GTid.x * TRANSPOSE_BLOCK_SIZE + GTid.y].gridKey;
		//Data[XY.y * g_iHeight + XY.x].particleIndex = transpose_shared_data[GTid.x * TRANSPOSE_BLOCK_SIZE + GTid.y].particleIndex;
	Data[DTid.x * g_iHeight + DTid.y] = temp;
}