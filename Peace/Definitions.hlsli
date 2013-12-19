
#include "simulationConstants.h"

/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer : register (b0)
{
	matrix WorldViewProjectionMatrix;
	matrix inverseWorldViewProjectionMatrix;

	float4 right_direction;
	float4 up_direction;
	float4 view_direction;
	float4 camera_position;

	float4 screen_texture_half_pixel;

};


//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
	float4 position ;
	float4 velocity ;
};

struct GeometryInputType
{
	float4 position : SV_POSITION;
	float4 color	: COLOR;
};

struct PixelInputType
{
	float4 position : SV_POSITION;

	float4 color	: COLOR;
	float4 tex_coord: TEX_COORD0;
};

