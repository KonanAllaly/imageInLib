#ifdef __cplusplus
extern "C" {
#endif

#include "common_functions.h"
#pragma once
	typedef struct
	{
		dataType timeStepSize;
		dataType h;
		dataType sigma;
		dataType edge_detector_coefficient;
		dataType omega_c;
		dataType tolerance;
		dataType eps2;
		dataType coef;
		size_t p;
		size_t timeStepsNum;
		size_t maxNumberOfSolverIteration;
	} Filter_Parameters;
	// Parameters used in filtering

	typedef struct
	{
		dataType timeStepSize;
		dataType hx, hy, hz; // voxel size
		dataType sigma;
		dataType edge_detector_coefficient;
		dataType omega_c;
		dataType tolerance;
		dataType eps2;
		dataType coef;
		size_t timeStepsNum;
		size_t maxNumberOfSolverIteration;
	} Filtering_Parameters;

#ifdef __cplusplus
}
#endif