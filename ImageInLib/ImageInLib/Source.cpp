#pragma warning(disable : 6011)
#pragma warning(disable : 4244)
#pragma warning(disable : 4477)
#pragma warning(disable : 4267)
#pragma warning(disable : 4700)

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <math.h>
#include <time.h>

#include "file.h"
#include "common_functions.h"
#include "../src/data_initialization.h"
#include "filtering.h"
#include "../src/data_load.h"
#include "../src/data_storage.h"
#include "../src/thresholding.h"
#include "Labelling.h"
#include "template_functions.h"
#include "../src/endianity_bl.h"
#include "../include/morphological_change.h"
#include "../src/segmentation3D_subsurf.h"
#include "../src/segmentation3d_gsubsurf.h"
#include "../src/image_difference.h"
#include "../src/noise_generator.h"
#include "../src/imageInterpolation.h"
#include "distanceForPathFinding.h"
#include "distanceMaps.h"
#include "segmentation.h"
#include "segmentation2d.h"

#define thresmin 995
#define thresmax 1213

#define sx 1.171875
#define sy 1.171875
#define sz 2.5
#define s_pet 4.0


int main() {

	size_t i, j, k;

	//image Dimensions
	size_t WidthPET = 144;
	size_t LengthPET = 144;
	const size_t dim2DPET = WidthPET * LengthPET;
	const size_t HeightPET = 255;

	std::string inputPath = "C:/Users/Konan Allaly/Documents/Tests/input/";
	std::string outputPath = "C:/Users/Konan Allaly/Documents/Tests/output/";

	dataType** imageDataPET = new dataType* [HeightPET];
	double** image = new double * [HeightPET];
	for (k = 0; k < HeightPET; k++) {
		imageDataPET[k] = new dataType[dim2DPET];
		image[k] = new double[dim2DPET];
	}
	if (imageDataPET == NULL || image == NULL)
		return false;

	for (k = 0; k < HeightPET; k++) {
		for (i = 0; i < WidthPET * LengthPET; i++) {
			imageDataPET[k][i] = 0.0;
			image[k][i] = 0.0;
		}
	}
	
	//std::string inputImagePath = inputPath + "pet_slice_150.raw";
	//std::string inputImagePath = "C:/Users/Konan Allaly/Documents/ProjectFor2dTest/output/filtered_gmc.raw";
	//std::string inputImagePath = "C:/Users/Konan Allaly/Documents/MPI/input/Image.raw";
	//std::string inputImagePath = outputPath + "resized2d.raw";
	Operation operation = LOAD_DATA;

	std::string inputImagePath = inputPath + "patient2_pet.raw";

	//manageRAWFile3D<double>(image, LengthPET, WidthPET, HeightPET, inputImagePath.c_str(), operation, false);
	manageRAWFile3D<dataType>(imageDataPET, LengthPET, WidthPET, HeightPET, inputImagePath.c_str(), operation, false);

	//for (k = 0; k < HeightPET; k++) {
	//	for (i = 0; i < WidthPET * LengthPET; i++) {
	//		imageDataPET[k][i] = (dataType)image[k][i];
	//	}
	//}

	for (k = 0; k < HeightPET; k++) {
		delete[] image[k];
	}
	delete[]image;

	operation = STORE_DATA;
	std::string outputImagePath = outputPath + "load_petVolume.raw";
	//manageRAWFile3D<dataType>(imageDataPET,LengthPET, WidthPET, HeightPET, outputImagePath.c_str(), operation, false);

	size_t WidthCT = 512;
	size_t LengthCT = 512;
	size_t HeightCT = 406;
	dataType** imageDataCT = new dataType* [HeightCT];
	for (k = 0; k < HeightCT; k++) {
		imageDataCT[k] = new dataType[WidthCT * LengthCT];
	}

	if (imageDataCT == NULL)
		return false;

	for (k = 0; k < HeightCT; k++) {
		for (i = 0; i < WidthCT * LengthCT; i++) {
			imageDataCT[k][i] = 0.0;
		}
	}

	//manageRAWFile2D<dataType>(imageDataCT, LengthCT, WidthCT, inputImagePath.c_str(), operation, false);

	Image_Data img1;
	img1.imageDataPtr = imageDataPET; img1.height = HeightPET; img1.width = WidthPET, img1.length = LengthPET;
	img1.origin = { -286.586, -216.586, -1021.40 }; img1.spacing = { s_pet, s_pet, s_pet };
	//img1.origin = { 0.0, 0.0, 0.0 }; img1.spacing = { s_pet, s_pet, s_pet };
	img1.orientation.v1 = { 1.0, 0.0, 0.0 }; img1.orientation.v2 = { 0.0, 1.0, 0.0 }, img1.orientation.v3 = { 0.0, 0.0, 1.0 };

	Image_Data img2;
	img2.imageDataPtr = imageDataCT; img2.height = HeightCT; img2.width = WidthCT, img2.length = LengthCT;
	img2.origin = { -300.0, -230.0, -1022.5 }; img2.spacing = { sx, sy, sz };
	//img2.origin = { 0.0, 0.0, 0.0 }; img2.spacing = { sx, sy, sz };
	img2.orientation.v1 = { 1.0, 0.0, 0.0 }; img2.orientation.v2 = { 0.0, 1.0, 0.0 }, img2.orientation.v3 = { 0.0, 0.0, 1.0 };

	interpolationMethod method = NEAREST_NEIGHBOR;
	//interpolationMethod method = BILINEAR;
	//interpolationMethod method = TRILINEAR;
	 
	imageInterpolation3D(img1, img2, method);

	//imageInterpolation3DVersion2(img1, img2, method);

	operation = STORE_DATA;
	outputImagePath = outputPath + "TransformPET_NN.raw";
	manageRAWFile3D<dataType>(imageDataCT, LengthCT, WidthCT, HeightCT, outputImagePath.c_str(), operation, false);
	//manageRAWFile2D<dataType>(imageDataPET, LengthPET, WidthPET, outputImagePath.c_str(), operation, false);

	//=========== generate circle ========

	//for (i = 0; i < Length; i++) {
	//	for (j = 0; j < Width; j++) {
	//		if (sqrt((i - 32) * (i - 32) + (j - 32) * (j - 32)) < 25) {
	//			imageData[x_new(i, j, Length)] = 1;
	//		}
	//		else {
	//			imageData[x_new(i, j, Length)] = 0;
	//		}
	//	}
	//	for (j = 0; j < Width; j++) {
	//		if (sqrt((i - 32) * (i - 32) + (j - 32) * (j - 32)) < 23) {
	//			imageData[x_new(i, j, Length)] = 0;
	//		}
	//	}
	//}

	//=======================

	//Image_Data2D imageToBeSegmented;
	//imageToBeSegmented.imageDataPtr = imageData; imageToBeSegmented.height = Length; imageToBeSegmented.width = Width;
	//dataType* initialSegment = new dataType[dim2D];
	//if (initialSegment == NULL) return false;
	//initialize2dArrayD(initialSegment, Length, Width, 0.0);
	//point2d* center = new point2d[1];
	//center->x = 187; center->y = 254;
	//generateInitialSegmentationFunction(initialSegment, Length, Width, center, 0.5, 10.0);
	//std::string segmentationPath = outputPath + "/segmentation/";
	//std::string initialSegmPath = segmentationPath + "_seg_func_0000.raw";
	//operation = STORE_DATA;
	//manageRAWFile2D<dataType>(initialSegment, Length, Width, initialSegmPath.c_str(), operation, false);
	//Filter_Parameters filtering_parameters;
	//filtering_parameters.timeStepSize = 1.0; filtering_parameters.h = 1.0;
	//filtering_parameters.omega_c = 1.4; filtering_parameters.tolerance = 1e-3;
	//filtering_parameters.maxNumberOfSolverIteration = 100; filtering_parameters.timeStepsNum = 1;
	//filtering_parameters.eps2 = 0.000001; filtering_parameters.edge_detector_coefficient = 100;
	//heatImplicit2dScheme(imageToBeSegmented, filtering_parameters);
	//std::string filteredImagePath = outputPath + "filtered.raw";
	//manageRAWFile2D<dataType>(imageToBeSegmented.imageDataPtr, Length, Width, filteredImagePath.c_str(), operation, false);

	//Segmentation_Parameters segmentation_parameters;
	//segmentation_parameters.h = 1.0; segmentation_parameters.coef = 100; segmentation_parameters.eps2 = 0.00001;
	//segmentation_parameters.maxNoGSIteration = 100; segmentation_parameters.omega_c = 1.5; segmentation_parameters.segTolerance = 1e-3;
	//segmentation_parameters.tau = 10.0; segmentation_parameters.numberOfTimeStep = 100; segmentation_parameters.maxNoOfTimeSteps = 100;
	//segmentation_parameters.mod = 1; segmentation_parameters.coef_conv = 5.0, segmentation_parameters.coef_dif = 1.0;
	//gsubsurf(imageToBeSegmented, initialSegment, segmentationPath.c_str(), filtering_parameters, segmentation_parameters);
	//subsurf(imageToBeSegmented, initialSegment, segmentationPath.c_str(), filtering_parameters, segmentation_parameters);
	//=======================

	//std::string outputImagePath = outputPath + "loaded.raw";
	//operation = STORE_DATA;
	//manageRAWFile2D<dataType>(imageData, Length, Width, outputImagePath.c_str(), operation, false);
	
	for (k = 0; k < HeightPET; k++) {
		delete[] imageDataPET[k];
		delete[] imageDataCT[k];
	}
	delete[] imageDataPET;
	delete[] imageDataCT;

	return EXIT_SUCCESS;
}
