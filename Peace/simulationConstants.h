
#ifndef SIMULATION_CONSTANTS_
#define SIMULATION_CONSTANTS_

//constants
#define g_fInitialParticleSpacing (0.0052f)
#define g_fSmoothlen 0.012f
#define g_fPressureStiffness 200.0f
#define g_fRestDensity 1000.0f
#define g_fParticleMass 0.0002f
#define g_fViscosity 0.7f
#define g_fMaxAllowableTimeStep 0.005f
#define g_fParticleRenderSize 0.003f
#define g_fWallStiffness 3000.0f

#define PARTICLE_COUNT 32 * 1024
#define WARP_GROUP_SIZE 1024
#define TRANSPOSE_BLOCK_SIZE 16
#define GRID_DIMENSION 256
#define HALF_GRID_SIZE ((float)GRID_DIMENSION * g_fSmoothlen * 0.5)
#define SIMULATION_BLOCK_SIZE 1024

#define PI 3.14159265359

#endif