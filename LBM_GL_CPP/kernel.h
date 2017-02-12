#define SMAG_CONST 1.f

#include "cuda_runtime.h"
#include "cuda.h"
#include "device_launch_parameters.h"
#include <stdio.h>
#include <cstring>
#include "common.h"
#include "SimulationParameters.h"

void InitializeDomain(float4* vis, float* f_d, int* im_d, float uMax);

void SetObstructionVelocitiesToZero(Obstruction* obst_h, Obstruction* obst_d);

void MarchSolution(float4* vis, float* fA_d, float* fB_d, int* im_d, Obstruction* obst_d, 
                        ContourVariable contVar, float contMin, float contMax, ViewMode viewMode, float uMax, float omega, int tStep);

void UpdateDeviceObstructions(Obstruction* obst_d, int targetObstID, Obstruction newObst);

void CleanUpDeviceVBO(float4* vis);

void LightSurface(float4* vis, Obstruction* obst_d, float3 cameraPosition);

void InitializeFloor(float4* vis, float* floor_d);

void LightFloor(float4* vis, float* floor_d, float* floorFiltered_d, Obstruction* obst_d, float3 cameraPosition);

int RayCastMouseClick(float3 &selectedElementCoord, float4* vis, float3 rayOrigin, float3 rayDir, Obstruction* obst_d);
