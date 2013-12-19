
#include "sph.h"


//Smoothed Particle Hydrodynamics with D11 compute shader simulation

ID3D11VertexShader *final_vs;
ID3D11PixelShader *final_ps;
ID3D11GeometryShader *final_gs;
ID3D11ComputeShader *simulation_shader;

//grid-key pair
ID3D11ComputeShader *gridPairFiller_shader;
ID3D11UnorderedAccessView *gridkey_particleindex_buffer_UAV;
ID3D11ShaderResourceView *gridkey_particleindex_buffer_SRV;
ID3D11Buffer *gridkey_particleindex_buffer;

//grid-key pair double_buffer
ID3D11UnorderedAccessView *gridkey_particleindex_tempBuffer_UAV;
ID3D11ShaderResourceView *gridkey_particleindex_tempBuffer_SRV;
ID3D11Buffer *gridkey_particleindex_tempBuffer;

//sort-grid-keys
ID3D11ComputeShader *gridPairSorter_shader;
ID3D11ComputeShader *transposeMatrix_shader;
ID3D11Buffer *gridPairSorter_constant_buffer;

//arrange grid start-finish offsets
ID3D11ComputeShader *clearBorderIndices_shader;
ID3D11ComputeShader *gridBorderFinder_shader;
ID3D11Buffer *grid_border_buffer;
ID3D11UnorderedAccessView *grid_border_buffer_UAV;
ID3D11ShaderResourceView *grid_border_buffer_SRV;

//sph solvers
ID3D11ComputeShader *densitySolver_shader;
ID3D11ComputeShader *accelerationSolver_shader;
ID3D11ComputeShader *integrateSolver_shader;

//particle buffer (vertex only)
ID3D11Buffer *particle_buffer;
ID3D11UnorderedAccessView *particle_buffer_UAV;
ID3D11ShaderResourceView *particle_buffer_SRV;

//particle info buffers
ID3D11Buffer *density_buffer;
ID3D11UnorderedAccessView *density_buffer_UAV;
ID3D11ShaderResourceView *density_buffer_SRV;
ID3D11Buffer *force_buffer;
ID3D11UnorderedAccessView *force_buffer_UAV;
ID3D11ShaderResourceView *force_buffer_SRV;

//CPU side resources
GridCostantBuffer grid_constant_buffer_cpu;
SortConstantBuffer sort_constant_buffer_cpu;
SimulationConstantBuffer simulation_constant_buffer_cpu;

//constant buffers
ID3D11Buffer *testConstantBuffer;
ID3D11Buffer *gridConstantBuffer;
ID3D11Buffer *simulationConstantBuffer;

//debug buffers
ID3D11Buffer *debug_grid_index_buffer;
ID3D11Buffer *debug_grid_border_buffer;
ID3D11Buffer *debug_force_buffer;
ID3D11Buffer *debug_position_buffer;

//walls 
const float low_wall = 1;
const float left_wall = 1.5;
const float near_wall = 32 * g_fInitialParticleSpacing;

void init_particles()
{
	const UINT iStartingWidth = 32;// (UINT)sqrtf((FLOAT)PARTICLE_COUNT);
	VertexType* particles = new VertexType[(int)PARTICLE_COUNT];
	memset(particles, 0, sizeof(VertexType)* PARTICLE_COUNT);

	float middle_offset = iStartingWidth * g_fInitialParticleSpacing * 0.5f;
	for (UINT i = 0; i < PARTICLE_COUNT; i++)
	{
		// Arrange the particles in a nice square
		UINT x = i / (iStartingWidth * iStartingWidth);
		UINT y = (i / iStartingWidth ) % (iStartingWidth);
		UINT z = i % (iStartingWidth);
		particles[i].position.x = (float)x * g_fInitialParticleSpacing;
		particles[i].position.y = (float)y * g_fInitialParticleSpacing;
		particles[i].position.z = (float)z * g_fInitialParticleSpacing - low_wall;
		particles[i].position.w = 1;

		particles[i].velocity.x = 0;
		particles[i].velocity.y = 0;
		particles[i].velocity.z = 0;
		particles[i].velocity.w = 0;
	}

	const char name2[] = "particleVertexBuffer";
	CreateComputeResourceBuffer(&particle_buffer, &particle_buffer_UAV, &particle_buffer_SRV, PARTICLE_COUNT, (float*)particles, sizeof(VertexType));
	particle_buffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(name2)-1, name2);


}

void init_sph()
{
	final_vs = CreateVertexShader(L"../Debug/StandartV.cso");
	final_ps = CreatePixelShader(L"../Debug/StandartP.cso");
	final_gs = CreateGeometryShader(L"../Debug/fluidEnlarger.cso");
	simulation_shader = CreateComputeShader(L"../Debug/test.cso");


	init_particles();

	//---------------------constant buffers
	{
		gridConstantBuffer = CreateConstantBuffer(sizeof(GridCostantBuffer));
		simulationConstantBuffer = CreateConstantBuffer(sizeof(SimulationConstantBuffer));
	}

	//---------------------grid key pair builder
	{
		gridPairFiller_shader = CreateComputeShader(L"../Debug/gridPairFiller.cso");
		gridkey_particleindex_buffer = CreateStructuredBuffer(PARTICLE_COUNT, NULL, sizeof(GridKeyStructure));
		const char name2[] = "gridkey_particleindex_buffer";
		gridkey_particleindex_buffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(name2)-1, name2);
		gridkey_particleindex_buffer_UAV = CreateUnorderedAccessView(gridkey_particleindex_buffer);
		gridkey_particleindex_buffer_SRV = CreateShaderResourceView(gridkey_particleindex_buffer, PARTICLE_COUNT);	
	}

	//---------------------grid key pair sorter
	{
		gridPairSorter_shader = CreateComputeShader(L"../Debug/sortGridPairs.cso");
		gridPairSorter_constant_buffer = CreateConstantBuffer(sizeof(SortConstantBuffer));
	}

	//---------------------transpose matrix for sorting 
	{
		transposeMatrix_shader = CreateComputeShader(L"../Debug/transposeMatrix.cso");
		gridkey_particleindex_tempBuffer = CreateStructuredBuffer(PARTICLE_COUNT, NULL, sizeof(GridKeyStructure));
		const char name3[] = "gridkey_particleindex_tempBuffer";
		gridkey_particleindex_tempBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(name3)-1, name3);
		gridkey_particleindex_tempBuffer_UAV = CreateUnorderedAccessView(gridkey_particleindex_tempBuffer);
		gridkey_particleindex_tempBuffer_SRV = CreateShaderResourceView(gridkey_particleindex_tempBuffer, PARTICLE_COUNT);
	}

	//---------------------grid border finder
	{
		gridBorderFinder_shader = CreateComputeShader(L"../Debug/gridBorderFinder.cso");
		clearBorderIndices_shader = CreateComputeShader(L"../Debug/clearBorderIndices.cso");
		grid_border_buffer = CreateStructuredBuffer(GRID_DIMENSION * GRID_DIMENSION * GRID_DIMENSION, NULL, sizeof(GridBorderStructure));
		const char name4[] = "gridborder_buffer";
		grid_border_buffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(name4)-1, name4);
		grid_border_buffer_UAV = CreateUnorderedAccessView(grid_border_buffer);
		grid_border_buffer_SRV = CreateShaderResourceView(grid_border_buffer, GRID_DIMENSION * GRID_DIMENSION * GRID_DIMENSION);
	}

	//---------------------density solver
	{
		DensityStructure *data = new DensityStructure[PARTICLE_COUNT];
		for (int i = 0; i < PARTICLE_COUNT; i++)
		{
			data[i].density = 0.0f;
		}

		densitySolver_shader = CreateComputeShader(L"../Debug/SPHdensitySolver.cso");
		density_buffer = CreateStructuredBuffer(PARTICLE_COUNT, (float*)data, sizeof(DensityStructure));
		const char name5[] = "density_buffer";
		density_buffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(name5)-1, name5);
		density_buffer_UAV = CreateUnorderedAccessView(density_buffer);
		density_buffer_SRV = CreateShaderResourceView(density_buffer, PARTICLE_COUNT);
	}

	//---------------------force solver
	{
		ForceStructure *data = new ForceStructure[PARTICLE_COUNT];
		for (int i = 0; i < PARTICLE_COUNT; i++)
		{
			data[i].acceleration.x = 0.0f;
			data[i].acceleration.y = 0.0f;
			data[i].acceleration.z = 0.0f;
			data[i].acceleration.w = 0.0f;
		}

		force_buffer = CreateStructuredBuffer(PARTICLE_COUNT, (float*)data, sizeof(ForceStructure));
		const char name5[] = "force_buffer";
		force_buffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(name5)-1, name5);
		force_buffer_UAV = CreateUnorderedAccessView(force_buffer);
		force_buffer_SRV = CreateShaderResourceView(force_buffer, PARTICLE_COUNT);
	}

	//---------------------acceleration solver
	{
		accelerationSolver_shader = CreateComputeShader(L"../Debug/SPHforceSolver.cso");
	}

	//---------------------integrate solver
	{
		integrateSolver_shader = CreateComputeShader(L"../Debug/SPHintegrateSolver.cso");
	}

	//debug buffers
	{
		debug_grid_index_buffer = CreateReadableBuffer(sizeof(GridKeyStructure)* PARTICLE_COUNT, NULL);
		debug_grid_border_buffer = CreateReadableBuffer(sizeof(GridBorderStructure)* GRID_DIMENSION * GRID_DIMENSION * GRID_DIMENSION, NULL);
		debug_force_buffer = CreateReadableBuffer(sizeof(ForceStructure)*PARTICLE_COUNT, NULL);
		debug_position_buffer = CreateReadableBuffer(sizeof(VertexType)* PARTICLE_COUNT, NULL);
	}


	SetViewPort(1366,768);

}

//simulation helpers
void build_grid_key_buffer()
{
	Flush();
	DebugginEvent event(L"build grid key pairs");

	grid_constant_buffer_cpu.g_GridDimension = GRID_DIMENSION;
	grid_constant_buffer_cpu.g_GridDimensionCube = GRID_DIMENSION * GRID_DIMENSION * GRID_DIMENSION;
	grid_constant_buffer_cpu.g_GridDimensionSquare = GRID_DIMENSION * GRID_DIMENSION;
	grid_constant_buffer_cpu.g_InverseGridDimension[0] = float(GRID_DIMENSION) * 0.5f;
	grid_constant_buffer_cpu.g_InverseGridDimension[1] = float(GRID_DIMENSION) * 0.5f;
	grid_constant_buffer_cpu.g_InverseGridDimension[2] = float(GRID_DIMENSION) * 0.5f;
	grid_constant_buffer_cpu.g_MiddleGridOffset[0] = (float)GRID_DIMENSION * 0.5f;
	grid_constant_buffer_cpu.g_MiddleGridOffset[1] = (float)GRID_DIMENSION * 0.5f;
	grid_constant_buffer_cpu.g_MiddleGridOffset[2] = (float)GRID_DIMENSION * 0.5f;

	UpdateBuffer((float*)&grid_constant_buffer_cpu, sizeof(GridCostantBuffer), gridConstantBuffer);
	setComputeConstantBuffer(gridConstantBuffer, 0);
	setCShaderUAVResources(&gridkey_particleindex_buffer_UAV,1);
	SetCShaderRV(&particle_buffer_SRV,1);
	SetComputeShader(gridPairFiller_shader);

	DispatchComputeShader(PARTICLE_COUNT / WARP_GROUP_SIZE,1,1);
}

void sort_grid_keys_pairs()
{
	SetComputeShader(gridPairSorter_shader);

	ID3D11ShaderResourceView* cleanerSRV = NULL;
	SetCShaderRV(&cleanerSRV,1);

	const UINT NUM_ELEMENTS = PARTICLE_COUNT;
	const UINT MATRIX_WIDTH = WARP_GROUP_SIZE;
	const UINT MATRIX_HEIGHT = PARTICLE_COUNT / MATRIX_WIDTH;

	for(UINT sortIteration = 2; sortIteration <= WARP_GROUP_SIZE; sortIteration <<= 1 )
	{
		Flush();
		DebugginEvent debug_event(L"sort");

		sort_constant_buffer_cpu.g_sortLevel = sortIteration;
		sort_constant_buffer_cpu.g_sortAlternateMask = sortIteration;

		UpdateBuffer((float*)&sort_constant_buffer_cpu,sizeof(SortConstantBuffer), gridPairSorter_constant_buffer);
		setComputeConstantBuffer(gridPairSorter_constant_buffer,1);
		setCShaderUAVResources(&gridkey_particleindex_buffer_UAV,1);
	
		DispatchComputeShader( PARTICLE_COUNT / WARP_GROUP_SIZE ,1,1);
		Flush();
	}


	for( UINT level = (WARP_GROUP_SIZE << 1) ; level <= PARTICLE_COUNT ; level <<= 1 )
	{
		ID3D11ShaderResourceView* cleanerSRV = NULL;
		
		{
			DebugginEvent debug_event(L"transpose to temp");

			// Transpose the data from buffer 1 into buffer 2s
			sort_constant_buffer_cpu.g_sortLevel = level / WARP_GROUP_SIZE;
			sort_constant_buffer_cpu.g_sortAlternateMask = (level & ~NUM_ELEMENTS) / WARP_GROUP_SIZE;
			sort_constant_buffer_cpu.g_iWidth = MATRIX_WIDTH;
			sort_constant_buffer_cpu.g_iHeight = MATRIX_HEIGHT;

			SetCShaderRV(&cleanerSRV,1);
			UpdateBuffer((float*)&sort_constant_buffer_cpu,sizeof(SortConstantBuffer), gridPairSorter_constant_buffer);
			setComputeConstantBuffer(gridPairSorter_constant_buffer, 1);

			SetComputeShader(transposeMatrix_shader);
			setCShaderUAVResources(&gridkey_particleindex_tempBuffer_UAV,1);
			SetCShaderRV(&gridkey_particleindex_buffer_SRV,1);
			Flush();
			DispatchComputeShader(MATRIX_WIDTH, MATRIX_HEIGHT , 1);
			Flush();
		}
		
		{
			DebugginEvent debug_event(L"sort the temp");
			// Sort the transposed column data
			SetComputeShader(gridPairSorter_shader);
			Flush();
			DispatchComputeShader(PARTICLE_COUNT / WARP_GROUP_SIZE, 1, 1);
			Flush();

		}
		
		{
			DebugginEvent debug_event(L"transpose to original");
			// Transpose the data from buffer 2 back into buffer 1
			sort_constant_buffer_cpu.g_sortLevel = WARP_GROUP_SIZE;
			sort_constant_buffer_cpu.g_sortAlternateMask = level;
			sort_constant_buffer_cpu.g_iWidth = MATRIX_HEIGHT;
			sort_constant_buffer_cpu.g_iHeight = MATRIX_WIDTH;

			SetCShaderRV(&cleanerSRV,1);
			SetComputeShader(transposeMatrix_shader);
			UpdateBuffer((float*)&sort_constant_buffer_cpu,sizeof(SortConstantBuffer), gridPairSorter_constant_buffer);
			setComputeConstantBuffer(gridPairSorter_constant_buffer, 1);
			
			setCShaderUAVResources(&gridkey_particleindex_buffer_UAV,1);
			SetCShaderRV(&gridkey_particleindex_tempBuffer_SRV,1);
			Flush();
			DispatchComputeShader(MATRIX_HEIGHT , MATRIX_WIDTH , 1);

			Flush();
		}

		{
			DebugginEvent debug_event(L"sort the original");
			// Sort the row data
			SetComputeShader(gridPairSorter_shader);
			Flush();
			DispatchComputeShader(PARTICLE_COUNT / WARP_GROUP_SIZE, 1, 1);
			Flush();
		}

	}

}

void validate_clear_grid_borders()
{
	CopyResourceContents(debug_grid_border_buffer, grid_border_buffer);
	GridBorderStructure *borders = (GridBorderStructure*)MapBuffer(debug_grid_border_buffer);

	for (int i = 0; i < GRID_DIMENSION * GRID_DIMENSION * GRID_DIMENSION; i++)
	{
		if (borders[i].gridEnd != 0 || borders[i].gridStart != 0)
		{
			_asm int 3;
		}
	}

	UnMapBuffer(debug_grid_border_buffer);

}

UINT debug_get_cell_id(VertexType v, UINT &X , UINT &Y, UINT &Z)
{
	UINT half_dimension = GRID_DIMENSION * 0.5f;
	float scaler = 1.0f / g_fSmoothlen;
	
	X = v.position.x * scaler + half_dimension;
	if (X < 0) X = 0; if (X > GRID_DIMENSION - 1) X = GRID_DIMENSION - 1;
	Y = v.position.y * scaler + half_dimension;
	if (Y < 0) Y = 0; if (Y > GRID_DIMENSION - 1) Y = GRID_DIMENSION - 1;
	Z = v.position.z * scaler + half_dimension;
	if (Z < 0) Z = 0; if (Z > GRID_DIMENSION - 1) Z = GRID_DIMENSION - 1;



	return UINT(X + Y * GRID_DIMENSION + Z * GRID_DIMENSION * GRID_DIMENSION);

}

UINT debug_get_cell_id(UINT X, UINT Y, UINT Z)
{
	return UINT(X + Y * GRID_DIMENSION + Z * GRID_DIMENSION * GRID_DIMENSION);
}

void validate_sort_and_grid_borders()
{
	CopyResourceContents(debug_grid_index_buffer, gridkey_particleindex_buffer);
	CopyResourceContents(debug_grid_border_buffer, grid_border_buffer);
	CopyResourceContents(debug_force_buffer, force_buffer);
	CopyResourceContents(debug_position_buffer, particle_buffer);
	
	GridKeyStructure *indices = (GridKeyStructure*)MapBuffer(debug_grid_index_buffer);
	GridBorderStructure *borders = (GridBorderStructure*)MapBuffer(debug_grid_border_buffer);
	ForceStructure *forces = (ForceStructure*)MapBuffer(debug_force_buffer);
	VertexType *p_poses = (VertexType*)MapBuffer(debug_position_buffer);

	vector<GridKeyStructure> indice_vector;
	map<UINT, int> ids;
	map<UINT, int> pid_vector;
	{
		
		for (int i = 0; i < PARTICLE_COUNT; i++)
		{
			GridKeyStructure gks = indices[i];
			indice_vector.push_back(indices[i]);
			if (indices[i].particleIndex >= PARTICLE_COUNT)
			{
				_asm int 3;
			}
			if (indices[i].particleIndex < 0)
			{
				_asm int 3;
			}
			if (i < PARTICLE_COUNT - 1 && indices[i].gridKey > indices[i + 1].gridKey)
			{
				_asm int 3;
			}
			if (ids.find(indices[i].particleIndex) != ids.end())
			{
				_asm int 3;
			}
			ids[indices[i].particleIndex] = i;
			pid_vector[i] = indices[i].particleIndex;
		}
	}
	ids.clear();
	vector<GridBorderStructure> border_vector;
	{
		int p_count = 0;
		
		for (int i = 0; i < GRID_DIMENSION * GRID_DIMENSION * GRID_DIMENSION; i++)
		{

			int gridStart = borders[i].gridStart;
			int gridEnd = borders[i].gridEnd;

			if (gridStart < gridEnd)
				border_vector.push_back(borders[i]);

			for (int j = gridStart; j < gridEnd; j++)
			{
				p_count++;
				if (i != indices[j].gridKey)
				{
					_asm int 3;
				}
				if (ids.find(indices[j].particleIndex) != ids.end())
				{
					_asm int 3;
				}
				ids[indices[j].particleIndex] = i;
			}
		}

		for (int i = 0; i < PARTICLE_COUNT; i++)
		{
			if (ids.find(i) == ids.end())
			{
				_asm int 3;
			}
		}

		if (p_count != PARTICLE_COUNT)
		{
			_asm int 3;
		}

	}

	/*{
		for (int i = 0; i < PARTICLE_COUNT; i++)
		{
			map<UINT, UINT> get_from_grids;
			VertexType particle_pos = p_poses[i];
			D3DXVECTOR3 p_position(particle_pos.position.x, particle_pos.position.y, particle_pos.position.z);
			int count = 0;

			const float h_sq = g_fSmoothlen * g_fSmoothlen;

			UINT X, Y, Z;
			UINT cell_id = debug_get_cell_id(particle_pos, X , Y , Z);

			for (int grid_index_x = X - 1; grid_index_x <= X + 1; grid_index_x++)
			{
				if (grid_index_x < 0) continue;
				if (grid_index_x > GRID_DIMENSION - 1) continue;

				for (int grid_index_z = Z - 1; grid_index_z <= Z + 1; grid_index_z++)
				{
					if (grid_index_z < 0) continue;
					if (grid_index_z > GRID_DIMENSION - 1) continue;

					UINT neighbour_cell_id = debug_get_cell_id(grid_index_x, Y, grid_index_z);

					UINT gridStart = borders[neighbour_cell_id].gridStart;
					UINT gridEnd = borders[neighbour_cell_id].gridEnd;

					for (UINT cur_par = gridStart; cur_par < gridEnd; cur_par++)
					{
						UINT n_p_id = indices[cur_par].particleIndex;
						VertexType N_P = p_poses[n_p_id];
						D3DXVECTOR3 n_position(N_P.position.x, N_P.position.y, N_P.position.z);

						D3DXVECTOR3 diff = n_position - p_position;
						float r_sq = D3DXVec3Dot(&diff, &diff);
						if (r_sq < h_sq && i != n_p_id)
						{
							count++;
							get_from_grids[n_p_id] = neighbour_cell_id;
						}

					}
				}
			}

			int all_count = 0;
			for (int j = 0; j < PARTICLE_COUNT; j++)
			{
				UINT n_p_id = j;
				VertexType N_P = p_poses[n_p_id];
				D3DXVECTOR3 n_position(N_P.position.x, N_P.position.y, N_P.position.z);

				D3DXVECTOR3 diff = n_position - p_position;
				float r_sq = D3DXVec3Dot(&diff, &diff);
				if (r_sq < h_sq && i != n_p_id)
				{
					all_count++;
					if (get_from_grids.find(n_p_id) == get_from_grids.end())
					{
						_asm int 3;
					}
				}
			}

			if (count != forces[i].acceleration.x || count != all_count)
			{
				_asm int 3;
			}


		}

	}*/


	UnMapBuffer(debug_grid_border_buffer);
	UnMapBuffer(debug_grid_index_buffer);
	UnMapBuffer(debug_force_buffer);
	UnMapBuffer(debug_position_buffer);
}

void build_grid_border_indices()
{
	{
		Flush();
		DebugginEvent name(L"clear border indices");
		SetComputeShader(clearBorderIndices_shader);
		setCShaderUAVResources(&grid_border_buffer_UAV,1);
		Flush();
		DispatchComputeShader(GRID_DIMENSION * GRID_DIMENSION * GRID_DIMENSION / (1024), 1, 1);
	}

	//validate_clear_grid_borders();

	{
		Flush();
		DebugginEvent name(L"find border indices");
		SetComputeShader(gridBorderFinder_shader);
		SetCShaderRV(&gridkey_particleindex_buffer_SRV,1);
		setCShaderUAVResources(&grid_border_buffer_UAV,1);
		Flush();
		DispatchComputeShader(PARTICLE_COUNT / WARP_GROUP_SIZE, 1 , 1);
	}
	//validate_sort_and_grid_borders();
}

void update_simulation_constants()
{
	// Simulation Constants
	
	// Clamp the time step when the simulation runs slowly to prevent numerical explosion
	simulation_constant_buffer_cpu.g_fTimeStep = 0.001;// min(g_fMaxAllowableTimeStep, 0.7);
	simulation_constant_buffer_cpu.g_fDensityCoef = g_fParticleMass * 315.0f / (64.0f * PI * pow(g_fSmoothlen, 9));
	simulation_constant_buffer_cpu.g_fGradPressureCoef = g_fParticleMass * -45.0f / (PI * pow(g_fSmoothlen, 6));
	simulation_constant_buffer_cpu.g_fLapViscosityCoef = g_fParticleMass * g_fViscosity * 45.0f / (PI * pow(g_fSmoothlen, 6));

	simulation_constant_buffer_cpu.g_vGravity[0] = 0;
	simulation_constant_buffer_cpu.g_vGravity[1] = 0;
	simulation_constant_buffer_cpu.g_vGravity[2] = -0.5f;
	simulation_constant_buffer_cpu.g_vGravity[3] = 0;
	
	// Collision information for the map
	simulation_constant_buffer_cpu.g_vPlanes[0] = D3DXVECTOR4(0, 0, 1, low_wall);
	simulation_constant_buffer_cpu.g_vPlanes[1] = D3DXVECTOR4(0, 0, -1, low_wall);
	simulation_constant_buffer_cpu.g_vPlanes[2] = D3DXVECTOR4(0, 1, 0, 0);
	simulation_constant_buffer_cpu.g_vPlanes[3] = D3DXVECTOR4(0, -1, 0, 32 * g_fInitialParticleSpacing);
	simulation_constant_buffer_cpu.g_vPlanes[4] = D3DXVECTOR4(1, 0, 0, 0);
	simulation_constant_buffer_cpu.g_vPlanes[5] = D3DXVECTOR4(-1, 0, 0, 32 * g_fInitialParticleSpacing);

	UpdateBuffer((float*)&simulation_constant_buffer_cpu, sizeof(SimulationConstantBuffer) , simulationConstantBuffer);
}

void main_sph_simulation()
{
	update_simulation_constants();

	//solve densities
	{
		Flush();
		DebugginEvent name(L"solve density");
		SetComputeShader(densitySolver_shader);

		ID3D11UnorderedAccessView *cleanerUAV = 0;
		setCShaderUAVResources(&cleanerUAV, 1);
		setCShaderUAVResources(&density_buffer_UAV, 1);

		ID3D11ShaderResourceView *views[3];
		views[0] = particle_buffer_SRV;
		views[1] = gridkey_particleindex_buffer_SRV;
		views[2] = grid_border_buffer_SRV;
		SetCShaderRV(views, 3);
		
		setComputeConstantBuffer(simulationConstantBuffer, 2);

		DispatchComputeShader(PARTICLE_COUNT / SIMULATION_BLOCK_SIZE, 1, 1);

		views[0] = 0;
		views[1] = 0;
		views[2] = 0;
		SetCShaderRV(views, 3);


	}

	//solve forces
	{
		Flush();
		DebugginEvent name(L"solve forces");
		SetComputeShader(accelerationSolver_shader);

		ID3D11UnorderedAccessView *cleanerUAV = 0;
		setCShaderUAVResources(&cleanerUAV, 1);
		setCShaderUAVResources(&force_buffer_UAV, 1);

		ID3D11ShaderResourceView *views[4];
		views[0] = particle_buffer_SRV;
		views[1] = gridkey_particleindex_buffer_SRV;
		views[2] = grid_border_buffer_SRV;
		views[3] = density_buffer_SRV;
		SetCShaderRV(views, 4);

		setComputeConstantBuffer(simulationConstantBuffer, 2);

		DispatchComputeShader(PARTICLE_COUNT / SIMULATION_BLOCK_SIZE, 1, 1);

		views[0] = 0;
		views[1] = 0;
		views[2] = 0;
		views[3] = 0;
		SetCShaderRV(views, 4);

		//validate_sort_and_grid_borders();
	}

	//integrate 
	{
		Flush();
		DebugginEvent name(L"integrate");
		SetComputeShader(integrateSolver_shader);

		setCShaderUAVResources(&particle_buffer_UAV, 1);
		SetCShaderRV(&force_buffer_SRV, 1);

		setComputeConstantBuffer(simulationConstantBuffer, 2);

		DispatchComputeShader(PARTICLE_COUNT / SIMULATION_BLOCK_SIZE, 1, 1);
		Flush();


		ID3D11UnorderedAccessView *views[1];
		views[0] = NULL;
		setCShaderUAVResources(views, 1);

	}

}

//main simulation loop
void sph_simulation()
{
	build_grid_key_buffer();

	sort_grid_keys_pairs();

	build_grid_border_indices();

	main_sph_simulation();

	ID3D11UnorderedAccessView* cleaner = NULL;
	ID3D11ShaderResourceView* cleanerSRV = NULL;

	setCShaderUAVResources(&cleaner,1);
	SetCShaderRV(&cleanerSRV,1);

	SetVertexBuffer(NULL);

	setCShaderUAVResources(&cleaner,1);

}

void render_sph()
{
	sph_simulation();

	ID3D11ShaderResourceView* cleaner = NULL;

	Flush();
	
	setUAVResourcesVertexShader(&particle_buffer_SRV,1);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	SetViewPort(1366,768);


	{

		SetShaders(final_vs, NULL, final_ps);
		SetBlendState(blend_state_disable_color_write);
		SetDepthState(depth_state_enable_test_enable_write);

		SetSamplerState();
		
		Render(PARTICLE_COUNT);
	}

	{
		SetShaders(final_vs, NULL, final_ps);
		
		SetDepthView(NULL);
		ID3D11ShaderResourceView *depth_srv = GetDepthResource();
		ID3D11ShaderResourceView *cleaner = NULL;
		setPShaderUAVResources(&depth_srv, 1);
		
		SetSamplerState();
		
		SetBlendState(blend_state_enable_color_write);
		SetDepthState(depth_state_enable_test_disable_write);
		
		Render(PARTICLE_COUNT);

		setPShaderUAVResources(&cleaner, 1);
		SetDepthView(GetDefaultDepthView());
	}

	setUAVResourcesVertexShader(&cleaner,1);

}

