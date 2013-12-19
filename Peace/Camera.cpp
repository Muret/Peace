
#include "includes.h"
#include "d11.h"

//camera parameters
D3DXVECTOR3 camera_position(0, -0.2, -0.8);
D3DXVECTOR3 view_direction(0, 1, 0);
D3DXVECTOR3 right_vector(1, 0, 0);
D3DXVECTOR3 up_vector(0, 0, 1);

float y_camera_bearing;
float x_camera_bearing;

//interaction
char keys[256];
bool is_moving_camera;
D3DXVECTOR2 last_mouse_position;

ID3D11Buffer* render_constantsBuffer;

RenderConstantsBuffer render_constantsBuffer_cpu;

void init()
{
	memset(keys, 0, sizeof(char)* 256);
	x_camera_bearing = 0;
	y_camera_bearing = 0;

	render_constantsBuffer = CreateConstantBuffer(sizeof(RenderConstantsBuffer));
}

void handle_user_input_down(char key)
{
	keys[key] = true;
}

void handle_user_input_up(char key)
{
	keys[key] = false;
}

void tick_user_inputs()
{
	if (is_moving_camera)
	{
		POINT  p;
		GetCursorPos(&p);
		y_camera_bearing -= (p.y - last_mouse_position.y) * 0.006;
		if (y_camera_bearing > PI)
		{
			y_camera_bearing -= PI;
		}
		x_camera_bearing -= (p.x - last_mouse_position.x) * 0.006;

		last_mouse_position = D3DXVECTOR2(p.x, p.y);
	}

	D3DXVECTOR3 original_view_direction(0, 1, 0);
	D3DXVECTOR3 original_right_vector(1, 0, 0);
	D3DXVECTOR3 original_up_vector(0, 0, 1);

	D3DXVECTOR3 view_temp;
	D3DXVECTOR3 view_temp2;
	D3DXVECTOR3 right_temp;
	D3DXVECTOR3 up_temp;

	D3DXMATRIX m_1;
	D3DXMATRIX m_2;

	D3DXMatrixRotationAxis(&m_2, &original_up_vector, x_camera_bearing);
	D3DXVec3TransformCoord(&view_temp2, &original_view_direction, &m_2);
	D3DXVec3TransformCoord(&right_temp, &original_right_vector, &m_2);

	D3DXMatrixRotationAxis(&m_1, &right_vector, y_camera_bearing);
	D3DXVec3TransformCoord(&view_temp, &view_temp2, &m_1);
	D3DXVec3TransformCoord(&up_temp, &original_up_vector, &m_1);

	D3DXVec3Normalize(&up_vector, &up_temp);
	D3DXVec3Normalize(&view_direction, &view_temp);
	D3DXVec3Normalize(&right_vector, &right_temp);

	if (keys['W'])
	{
		camera_position += 0.006 * view_direction;
	}

	if (keys['S'])
	{
		camera_position -= 0.006 * view_direction;
	}

	if (keys['D'])
	{
		camera_position += 0.011 * right_vector;
	}

	if (keys['A'])
	{
		camera_position -= 0.011 * right_vector;
	}

	if (keys['Q'])
	{
		camera_position += 0.011 * up_vector;
	}

	if (keys['E'])
	{
		camera_position -= 0.011 * up_vector;
	}
}

void startMovingCamera()
{
	if (!is_moving_camera)
	{
		POINT  p;
		GetCursorPos(&p);
		last_mouse_position = D3DXVECTOR2(p.x, p.y);

		is_moving_camera = true;

	}
}

void stopMovingCamera()
{
	is_moving_camera = false;
}

void set_render_constants_buffer()
{
	D3DXMATRIX view, projection, world, inverseViewProjection;
	D3DXVECTOR3 lookat = camera_position + view_direction;

	D3DXMatrixLookAtRH(&view, &camera_position, &lookat, &up_vector);

	D3DXMatrixPerspectiveFovRH(&projection, PI * 0.25, 1366.0f / 768.0f, 0.1f, 100.0f);

	float determinant;
	D3DXMATRIX mWorldViewProjection = view * projection;

	D3DXMatrixInverse(&inverseViewProjection, &determinant, &mWorldViewProjection);

	// Transpose the matrices to prepare them for the shader.
	D3DXMatrixTranspose(&mWorldViewProjection, &mWorldViewProjection);
	D3DXMatrixTranspose(&inverseViewProjection, &inverseViewProjection);


	render_constantsBuffer_cpu.WorldViewProjectionMatrix = mWorldViewProjection;
	render_constantsBuffer_cpu.right_direction = D3DXVECTOR4(right_vector,0);
	render_constantsBuffer_cpu.up_direction = D3DXVECTOR4(up_vector, 0);
	render_constantsBuffer_cpu.view_direction = D3DXVECTOR4(view_direction, 0);
	render_constantsBuffer_cpu.camera_position = D3DXVECTOR4(camera_position, 0);
	render_constantsBuffer_cpu.screen_texture_half_pixel = D3DXVECTOR4((1.0f / 1366.0f), (1.0f / 768.0f), 0, 0);
	render_constantsBuffer_cpu.inverseWorldViewProjectionMatrix = inverseViewProjection;


	UpdateBuffer((float*)&render_constantsBuffer_cpu, sizeof(RenderConstantsBuffer), render_constantsBuffer);

	SetConstantBufferForRendering(0, render_constantsBuffer);

}