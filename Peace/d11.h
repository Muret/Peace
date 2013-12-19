
#ifndef _D11_H_
#define _D11_H_

#include "includes.h"



//constant variables
struct MatrixBuffer
{
	float g_GridDimensionCube;
	float g_GridDimensionSquare;
	float g_GridDimension;
	float g_InverseGridDimension[3];
	float g_MiddleGridOffset[3];
};

struct GridCostantBuffer
{
	float g_GridDimensionCube;
	float g_GridDimensionSquare;
	float g_GridDimension;
	float g_InverseGridDimension[3];
	float g_MiddleGridOffset[3];
	float padding[3];
};

__declspec(align(16)) struct SortConstantBuffer
{
	UINT g_sortLevel;
	UINT g_sortAlternateMask;
	UINT g_iWidth;
	UINT g_iHeight;
	D3DXVECTOR4 padding;
};

struct SimulationConstantBuffer
{
	float g_fTimeStep;
	float g_fDensityCoef;
	float g_fGradPressureCoef;
	float g_fLapViscosityCoef;
	float g_vGravity[4];
	D3DXVECTOR4 g_vPlanes[6];
};

//declarations
struct RenderConstantsBuffer
{
	D3DXMATRIX WorldViewProjectionMatrix;
	D3DXMATRIX inverseWorldViewProjectionMatrix;
	D3DXVECTOR4 right_direction;
	D3DXVECTOR4 up_direction;
	D3DXVECTOR4 view_direction;
	D3DXVECTOR4 camera_position;
	D3DXVECTOR4 screen_texture_half_pixel;
};

struct VertexType
{
	D3DXVECTOR4 position;
	D3DXVECTOR4 velocity;
};

struct ComputeBuffer
{
	float number_of_particles;
	float time;
	float padding1;
	float padding2;
};

//grid key structure
struct GridKeyStructure
{
	UINT gridKey;
	UINT particleIndex;
};

struct ForceStructure
{
	D3DXVECTOR4 acceleration;
};

//grid border structure
struct GridBorderStructure
{
	UINT gridStart;
	UINT gridEnd;
};


struct DensityStructure
{
	float density;
};

extern ID3D11DeviceContext *g_deviceContext;
extern ID3DUserDefinedAnnotation *pPerf;

struct DebugginEvent
{
	
	DebugginEvent(LPCWSTR name)
	{
		pPerf->BeginEvent( name );
	}
	~DebugginEvent()
	{
		pPerf->EndEvent( );

	}

};

enum depthState
{
	depth_state_enable_test_enable_write,
	depth_state_enable_test_disable_write,
	depth_state_disable_test_enable_write,
	depth_state_disable_test_disable_write,
	number_of_depth_states
};

enum blendState
{
	blend_state_disable_color_write,
	blend_state_enable_color_write,
	number_of_blend_states
};

using namespace std;

bool initializeEngine();

void closeEngine();

void BeginScene();

void EndScene();

void clearScreen(float *color);

void CreateDepthStencilStates();

void CreateBlendStates();

void CreateSamplerStates();

ID3D11Query* CreateQuery(D3D11_QUERY type);

ID3D11Buffer* CreateConstantBuffer(int byteWidth);

ID3D11VertexShader* CreateVertexShader(LPCTSTR name);

ID3D11PixelShader* CreatePixelShader(LPCTSTR name);

ID3D11GeometryShader* CreateGeometryShader(LPCTSTR name);

ID3D11ComputeShader* CreateComputeShader(LPCTSTR name);

ID3D11Buffer *CreateVertexBuffer(int vertex_count, float *data);

ID3D11Buffer *CreateStructuredBuffer(int vertex_count, float* data, int byteWidth);

ID3D11UnorderedAccessView *CreateUnorderedAccessView( ID3D11Buffer* pBuffer);

ID3D11ShaderResourceView *CreateShaderResourceView(ID3D11Buffer* pBuffer , int vertex_count);	

ID3D11Buffer *CreateReadableBuffer(int buffer_size, float* data);	

ID3D11ShaderResourceView *CreateTextureResourceView(ID3D11Texture2D *text);

void CreateComputeResourceBuffer(ID3D11Buffer** pBuffer, ID3D11UnorderedAccessView** uav, ID3D11ShaderResourceView **srv, int vertex_count, float* data, int bytewidth);

void CopyResourceContents(ID3D11Resource *to, ID3D11Resource *from);

void BeginQuery(ID3D11Query *query_object);

void EndQuery(ID3D11Query *query_object);

void* MapBuffer(ID3D11Buffer *buffer);

void UnMapBuffer(ID3D11Buffer *buffer);

UINT64 GetQueryData(ID3D11Query *query_object);

void SetViewPort(int width, int height);

void SetVertexBuffer(ID3D11Buffer *  vertex_buffer);

void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY state);

void SetCameraBuffer(D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix);

void SetShaders(ID3D11VertexShader *vertex_shader, ID3D11GeometryShader *geometry_shader, ID3D11PixelShader *pixel_shader);

void SetComputeShader(ID3D11ComputeShader *compute_shader);

void SetConstantBufferForRendering(int start_slot, ID3D11Buffer *buffer_to_set);

void UpdateBuffer(float *data , int byteWidth, ID3D11Buffer* buffer);

void setComputeConstantBuffer(ID3D11Buffer* buffer, int index);

void SetDepthState(int depth_state);

void SetBlendState(int blend_state);

void SetSamplerState();

ID3D11ShaderResourceView *GetDepthResource();

void setPShaderUAVResources(ID3D11ShaderResourceView **srv_list, int number_of_resources);

void SetDepthView(ID3D11DepthStencilView *view);

ID3D11DepthStencilView* GetDefaultDepthView();

void setCShaderUAVResources(ID3D11UnorderedAccessView **uav_list, int number_of_resources);

void SetCShaderRV(ID3D11ShaderResourceView **uav_list, int number_of_resources);

void setUAVResourcesVertexShader(ID3D11ShaderResourceView **uav_list, int number_of_resources);

void DispatchComputeShader(int thread_count_x, int thread_count_y, int thread_count_z);

void Render(int vertext_count);

void drawText(int fps, int y_pos);

void Flush();

#endif