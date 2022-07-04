#ifdef __cplusplus
extern "C" {
#endif

#pragma once
#include "../src/shapeAnalysis_params.h"

	void calcMeanShape(dataType **, Shapes *shapes, size_t height, size_t length, size_t width, size_t numShapes, shape_Analysis_Parameters params);
	void estimateSimilarShape(Shapes * inputShapes, dataType **shapeToEstimate, dataType ** estimatedShape, shape_Analysis_Parameters shapeParam, estimate_Params * estimateParams, dataType pcaThreshold, size_t height, size_t length, size_t width, size_t numShapes);

#ifdef __cplusplus
}
#endif