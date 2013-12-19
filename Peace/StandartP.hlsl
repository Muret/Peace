
#include "Definitions.hlsli"


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////

Texture2D depth_texture : register(t0);
SamplerState depth_sampler : register(s0);

float4 main(PixelInputType input) : SV_TARGET
{
	float distance = dot(input.tex_coord.xy, input.tex_coord.xy);
	if (distance > 1) clip(-1);

	float4 pixel_position = input.position;
	pixel_position.xy *= screen_texture_half_pixel.xy;
	pixel_position.z = depth_texture.Sample(depth_sampler, pixel_position.xy);
	//return float4(pixel_position.z, pixel_position.z, pixel_position.z, 1);
	pixel_position.xy = pixel_position.xy * 2.0f - 1.0;
	//pixel_position *= pixel_position.z;
	float4 pixel_world_position = mul(pixel_position, inverseWorldViewProjectionMatrix);
	float3 ddx, ddy;

	{
		float4 pixel_position_neighbour = input.position;
		pixel_position_neighbour.xy *= screen_texture_half_pixel.xy;
		pixel_position_neighbour.x += screen_texture_half_pixel.x * 2.0f;
		pixel_position_neighbour.z = depth_texture.Sample(depth_sampler, pixel_position_neighbour.xy);
		pixel_position_neighbour.xy = pixel_position_neighbour.xy * 2.0f - 1.0;
		//pixel_position_neighbour *= pixel_position_neighbour.z;
		ddx = mul(pixel_position_neighbour, inverseWorldViewProjectionMatrix) - pixel_world_position;
	}

	{
		float4 pixel_position_neighbour = input.position;
		pixel_position_neighbour.xy *= screen_texture_half_pixel.xy;
		pixel_position_neighbour.y += screen_texture_half_pixel.y * 2.0f;
		pixel_position_neighbour.z = depth_texture.Sample(depth_sampler, pixel_position_neighbour.xy);
		pixel_position_neighbour.xy = pixel_position_neighbour.xy * 2.0f - 1.0;
		//pixel_position_neighbour *= pixel_position_neighbour.z;
		ddy = mul(pixel_position_neighbour, inverseWorldViewProjectionMatrix) - pixel_world_position;
	}

	float3 world_normal = cross(ddx, ddy);
		return float4(abs(normalize(world_normal)), 1);
}