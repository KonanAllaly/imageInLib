#pragma warning(disable : 6011)
#pragma warning(disable : 4244)
#pragma warning(disable : 4477)
#pragma warning(disable : 4267)
#pragma warning(disable : 4700)

#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <math.h>
#include <time.h>

#include "common_vtk.h"
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
#include "../src/shape_registration.h"
#include "../src/segmentation3D_subsurf.h"
#include "../src/segmentation3d_gsubsurf.h"
#include "../src/image_difference.h"
#include "../src/noise_generator.h"
#include "../src/image_norm.h"
#include "../src/imageInterpolation.h"
#include "distanceForPathFinding.h"

#define thresmin 995
#define thresmax 1213

#define epsilonFast 0.001


int main() {

	size_t i, j, k, x;

	//image Dimensions
	const size_t Width = 512;
	const size_t Length = 512;
	const size_t Height = 406;
	const size_t dim2D = Width * Length;
	
	//Preparation of image data pointers
	dataType** imageData = (dataType**)malloc(Height * sizeof(dataType*));
	short** image = (short**)malloc(Height * sizeof(short*));
	for (k = 0; k < Height; k++) {
		imageData[k] = (dataType*)malloc(dim2D * sizeof(dataType));
		image[k] = (short*)malloc(dim2D * sizeof(short));
	}

	if (imageData == NULL || image == NULL) {
		return false;
	}

	std::string inputPath = "C:/Users/Konan Allaly/Documents/Tests/input/";
	std::string outputPath = "C:/Users/Konan Allaly/Documents/Tests/output/";
	std::string inputImagePath = inputPath +  "patient2.raw";

	if (load3dArrayRAW<short>(image, Length, Width, Height, inputImagePath.c_str()) == false)
	{
		printf("inputImagePath does not exist\n");
	}

	for (k = 0; k < Height; k++) {
		for (i = 0; i < Length; i++) {
			for (j = 0; j < Width; j++) {
				x = x_new(i, j, Length);
				imageData[k][x] = (dataType)image[k][x];
			}
		}
	}

	//std::string loadedImagePath = outputPath + "loaded.raw";
	//store3dRawData<dataType>(imageData, Length, Width, Height, loadedImagePath.c_str());

	//Extract sagital slice
	//----------------------------------
	dataType * OneSliceImage = (dataType*)malloc(Height * Width * sizeof(dataType));
	dataType * rotatedImage = (dataType*)malloc(Height * Width * sizeof(dataType));
	dataType * distanceMap = (dataType*)malloc(Height * Width * sizeof(dataType));
	dataType * potentialPtr = (dataType*)malloc(Height * Width * sizeof(dataType));
	size_t cst = 238; i = 0;
	for (k = 0; k < Height; k++) {
		for (j = 0; j < Width; j++) {
			x = x_new(cst, j, Length);
			OneSliceImage[i] = imageData[k][x];
			distanceMap[i] = 0;
			i++;
		}
	}

	for (i = 0; i < Height; i++) {
		for (j = 0; j < Width; j++) {
			x = x_new(i, j, Height);
			rotatedImage[x] = OneSliceImage[x_new(Height - i - 1, Width - j - 1, Height)];
			potentialPtr[x] = 0;
		}
	}

	dataType* extendedImage = (dataType*)malloc( (Height + 2) * (Width + 2) * sizeof(dataType));

	for (i = 0; i < (Height + 2) * (Width + 2); i++) {
		extendedImage[i] = 0;
	}

	//copyDataTo2dExtendedArea(rotatedImage, extendedImage, Height, Width);
	//reflection2D(extendedImage, Height + 2, Width + 2);
	//copyDataTo2dReducedArea(rotatedImage, extendedImage, Height, Width);

	//std::string sagitalView = outputPath + "testImage.raw";
	//store2dRawData<dataType>(extendedImage, Height + 2, Width + 2, sagitalView.c_str());

	////Manual thresholding
	//for (k = 0; k < Height; k++) {
	//	for (j = 0; j < Width; j++) {
	//		if (rotatedImage[x_new(k, j, Height)] >= thresmin && rotatedImage[x_new(k, j, Height)] <= thresmax) {
	//			rotatedImage[x_new(k, j, Height)] = 1;
	//		}
	//		else {
	//			rotatedImage[x_new(k, j, Height)] = 0;
	//		}
	//	}
	//}

	//std::string thresOutPut = outputPath + "thresholdedImage.raw";
	//store2dRawData<dataType>(rotatedImage, Height, Width, thresOutPut.c_str());

	Point2D * startingPoint = (Point2D*)malloc(2 * sizeof(Point2D));
	//startingPoint->x = (size_t)(Width / 2); startingPoint->y = (size_t)(Height / 2);
	startingPoint->x = 246; startingPoint->y = 277;
	//startingPoint->x = 0; startingPoint->y = 0;

	////Manual rescalling
	//dataType scale_factor = 1.0 / 4000.0;
	//for (i = 0; i < Height; i++) {
	//	for (j = 0; j < Width; j++) {
	//		x = x_new(j, i, Width);
	//		rotatedImage[x] = scale_factor * (rotatedImage[x] - 4000) + 1;
	//	}
	//}
	
	fastMarching2d(rotatedImage, distanceMap, potentialPtr, Height, Width, startingPoint);

	//bruteForce2d(rotatedImage, distanceMap, Height, Width, 0);
	//computeImageGradient(rotatedImage, distanceMap, Height, Width, 1.0);

	std::string outPutDistance = outputPath + "distanceMap.raw";
	store2dRawData<dataType>(distanceMap, Height, Width, outPutDistance.c_str());
	outPutDistance = outputPath + "potential.raw";
	store2dRawData<dataType>(potentialPtr, Height, Width, outPutDistance.c_str());

	//free memory
	for (k = 0; k < Height; k++) {
		free(imageData[k]); free(image[k]);
	}
	free(imageData); free(image);

	free(OneSliceImage); free(rotatedImage); free(distanceMap); free(startingPoint);
	free(potentialPtr); 
	free(extendedImage);

	return EXIT_SUCCESS;
}
