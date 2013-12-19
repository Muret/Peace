
#include "Definitions.hlsli"


static const float2 g_positions[4] = { float2(-1, 1), float2(1, 1), float2(-1, -1), float2(1, -1) };

[maxvertexcount(5)]
void main(point GeometryInputType In[1], inout TriangleStream<PixelInputType> SpriteStream)
{
	[unroll]
	for (int i = 0; i < 4; i++)
	{
		PixelInputType Out;
		float4 position = float4(In[0].position);
		position.xyz += g_fParticleRenderSize * (float3(right_direction.xyz * g_positions[i].x) + float3(up_direction.xyz * g_positions[i].y));
		Out.position = mul(position, WorldViewProjectionMatrix);
		//Out.position /= Out.position.w;
		Out.color = In[0].color;
		Out.tex_coord = float4(g_positions[i],0,0);

		SpriteStream.Append(Out);
	}

	SpriteStream.RestartStrip();
}