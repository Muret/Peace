

#include "Definitions.hlsli"

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////

StructuredBuffer<VertexInputType> particleBuffer : register(t0);

PixelInputType main(uint ID : SV_VertexID)
{
	PixelInputType output;

	// Change the position vector to be 4 units for proper matrix calculations.
	float4 position = float4(particleBuffer[ID].position.xyz, 1);

		//position = mul(position, viewMatrix);
		output.position = mul(position, WorldViewProjectionMatrix);
	output.color = float4(particleBuffer[ID].velocity.xyz * 15, 1);
	output.tex_coord = 0;
	return output;
}


