#include "simulationH.hlsli"

RWStructuredBuffer<GridBorderStructure> Data : register(u0);

[numthreads(1024, 1, 1)]
void main( uint3 Gid : SV_GroupID, 
                  uint3 DTid : SV_DispatchThreadID, 
                  uint3 GTid : SV_GroupThreadID, 
                  uint GI : SV_GroupIndex )
{
	Data[DTid.x].gridStart = 0;
	Data[DTid.x].gridEnd = 0;
}