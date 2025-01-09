#include <iostream>
#include <sstream>  
#include <vector>
#include <string.h> 
#include "common_math.h"

#include<template_functions.h>
#include "../src/vtk_params.h"
#include "common_vtk.h"
#include "distanceForPathFinding.h"
#include "common_functions.h"
#include "../src/imageInterpolation.h"
#include "filtering.h"
#include "../src/segmentation3D_subsurf.h"
#include "../src/segmentation3d_gsubsurf.h"
#include "segmentation2d.h"
#include "../src/thresholding.h"
#include "../src/distance_function.h"
#include "morphological_change.h"
#include "Labelling.h"
#include "hough_transform.h"
#include "../src/edgedetection.h"
#include "enhancement.h"
#include "../src/fast_marching.h"

int main() {

	string inputPath = "C:/Users/Konan Allaly/Documents/Tests/input/";
	string outputPath = "C:/Users/Konan Allaly/Documents/Tests/output/";
	
	string loading_path, storing_path, extension;

	size_t i = 0, j = 0, k = 0, xd = 0;

	//===================== Load 3D patient data (.vtk) ================================
	
	/*
	We load the .vtk files containing the original pixels value
	and informations related the dimensions, the spacings and
	the origins. We also define the orientation matrix in 2D/3D
	needed when we need to perform interpolation.
	*/
	
	OrientationMatrix orientation;
	orientation.v1 = { 1.0, 0.0, 0.0 }; 
	orientation.v2 = { 0.0, 1.0, 0.0 }; 
	orientation.v3 = { 0.0, 0.0, 1.0 };

	OrientationMatrix2D orientation2D;
	orientation2D.v1 = { 1.0, 0.0 };
	orientation2D.v2 = { 0.0, 1.0 };

	Vtk_File_Info* ctContainer = (Vtk_File_Info*)malloc(sizeof(Vtk_File_Info));
	ctContainer->operation = copyFrom;
	
	loading_path = inputPath + "vtk/ct/Patient2_ct.vtk";
	readVtkFile(loading_path.c_str(), ctContainer);

	int Height = ctContainer->dimensions[2];
	int Length = ctContainer->dimensions[0];
	int Width = ctContainer->dimensions[1];
	int dim2D = Length * Width;
	std::cout << "CT image dim : " << ctContainer->dimensions[0] << " x " << ctContainer->dimensions[1] << " x " << ctContainer->dimensions[2] << "" << std::endl;

	std::cout << "CT origin : (" << ctContainer->origin[0] << ", " << ctContainer->origin[1] << ", " << ctContainer->origin[2] << ")" << std::endl;
	Point3D ctOrigin = { ctContainer->origin[0], ctContainer->origin[1], ctContainer->origin[2] };
	VoxelSpacing ctSpacing = { ctContainer->spacing[0], ctContainer->spacing[1], ctContainer->spacing[2] };
	std::cout << "CT spacing : (" << ctContainer->spacing[0] << ", " << ctContainer->spacing[1] << ", " << ctContainer->spacing[2] << ")" << std::endl; 

	////Find the minimum and maximum values to perform shiftting
	//dataType minData = ctContainer->dataPointer[0][0];
	//dataType maxData = ctContainer->dataPointer[0][0];
	//for (k = 0; k < Height; k++) {
	//	for (i = 0; i < Length * Width; i++) {
	//		if (ctContainer->dataPointer[k][i] > maxData) {
	//			maxData = ctContainer->dataPointer[k][i];
	//		}
	//		if (ctContainer->dataPointer[k][i] < minData) {
	//			minData = ctContainer->dataPointer[k][i];
	//		}
	//	}
	//}
	//std::cout << "Min = " << minData << ", Max = " << maxData << std::endl;

	//========================= Segment the lungs and the heart =================

	/*
	dataType** imageData = new dataType * [Height];
	for (k = 0; k < Height; k++) {
		imageData[k] = new dataType[dim2D]{ 0 };
	}
	loading_path = outputPath + "segmentation lungs/segment_lungs_p7.raw";
	manageRAWFile3D<dataType>(imageData, Length, Width, Height, loading_path.c_str(), LOAD_DATA, false);

	Image_Data inputData
	{
		Height,
		Length,
		Width,
		imageData,
		ctOrigin,
		ctSpacing,
		orientation
	};

	//size_t height_int = 866;//p2
	//size_t height_int = 1083;//p3
	//size_t height_int = 844;//p4, p5
	//size_t height_int = 1351;//p6
	size_t height_int = 932;//p7
	dataType** maskThreshold = new dataType * [height_int];
	dataType** interpolatedImage = new dataType * [height_int];
	for (k = 0; k < height_int; k++) {
		maskThreshold[k] = new dataType[dim2D]{ 0 };
		interpolatedImage[k] = new dataType[dim2D]{ 0 };
	}

	VoxelSpacing intSpacing = { ctSpacing.sx, ctSpacing.sy, ctSpacing.sx };
	Image_Data interpolatedData
	{
		height_int,
		Length,
		Width,
		maskThreshold,
		ctOrigin,
		intSpacing,
		orientation
	};

	imageInterpolation3D(inputData, interpolatedData, NEAREST_NEIGHBOR);

	//storing_path = outputPath + "interpolated_lungs_p7.raw";
	//manageRAWFile3D<dataType>(maskThreshold, Length, Width, height_int, storing_path.c_str(), STORE_DATA, false);

	//Find bounding box
	size_t k_min = height_int, k_max = 0;
	size_t i_min = Length, i_max = 0;
	size_t j_min = Width, j_max = 0;
	for (k = 0; k < height_int; k++) {
		for (i = 0; i < Length; i++) {
			for (j = 0; j < Width; j++) {
				if (maskThreshold[k][x_new(i, j, Length)] == 1.0) {
					if (i_min > i) {
						i_min = i;
					}
					if (j_min > j) {
						j_min = j;
					}
					if (k_min > k) {
						k_min = k;
					}

					if (i_max < i) {
						i_max = i;
					}
					if (j_max < j) {
						j_max = j;
					}
					if (k_max < k) {
						k_max = k;
					}
				}
			}
		}
	}

	//Adjust the bounding box to have all the voxels
	if (i_min > 0) {
		i_min -= 1;
	}
	if (j_min > 0) {
		j_min -= 1;
	}
	if (k_min > 0) {
		k_min -= 1;
	}
	if (i_max < Length - 1) {
		i_max += 1;
	}
	if (j_max < Width - 1) {
		j_max += 1;
	}
	if (k_max < height_int - 1) {
		k_max += 1;
	}

	std::cout << "i min = " << i_min << ", i max = " << i_max << std::endl;
	std::cout << "j min = " << j_min << ", j max = " << j_max << std::endl;
	std::cout << "k min = " << k_min << ", k max = " << k_max << std::endl;

	size_t height = k_max - k_min + 1;
	size_t length = i_max - i_min + 1;
	size_t width = j_max - j_min + 1;
	std::cout << "New dim : " << length << " x " << width << " x " << height << std::endl;

	Point3D cropped_origin = { i_min, j_min, k_min };
	cropped_origin = getRealCoordFromImageCoord3D(cropped_origin, ctOrigin, intSpacing, orientation);
	std::cout << "Cropped image origin : (" << cropped_origin.x << ", " << cropped_origin.y << ", " << cropped_origin.z << ")" << std::endl;

	dataType** maskLungs = new dataType * [height];
	dataType** ball = new dataType * [height];
	dataType** distanceMap = new dataType * [height];
	dataType** imageCropped = new dataType * [height];
	dataType** segment = new dataType * [height];
	for (k = 0; k < height; k++) {
		maskLungs[k] = new dataType[length * width]{ 0 };
		ball[k] = new dataType[length * width]{ 0 };
		distanceMap[k] = new dataType[length * width]{ 0 };
		imageCropped[k] = new dataType[length * width]{ 0 };
		segment[k] = new dataType[length * width]{ 0 };
	}

	//Cropping
	loading_path = inputPath + "raw/interpolated/filtered_p7.raw";
	manageRAWFile3D<dataType>(interpolatedImage, Length, Width, height_int, loading_path.c_str(), LOAD_DATA, false);
	size_t k_ext, i_ext, j_ext;
	for (k = 0, k_ext = k_min; k < height; k++, k_ext++) {
		for (i = 0, i_ext = i_min; i < length; i++, i_ext++) {
			for (j = 0, j_ext = j_min; j < width; j++, j_ext++) {
				imageCropped[k][x_new(i, j, length)] = interpolatedImage[k_ext][x_new(i_ext, j_ext, Length)];
				//Remove corners
				if (k >= 1 && k < (height - 1) && i >= 5 & i <= (length - 5) && j >= 5 && j <= (width - 5)) {
					if (maskThreshold[k_ext][x_new(i_ext, j_ext, Length)] == 1.0) {
						maskLungs[k][x_new(i, j, length)] = 0.0; //maskThreshold[k_ext][x_new(i_ext, j_ext, Length)];
					}
					else {
						maskLungs[k][x_new(i, j, length)] = 1.0;
					}
				}
			}
		}
	}

	//storing_path = outputPath + "cropped_image.raw";
	//manageRAWFile3D<dataType>(imageCropped, length, width, height, storing_path.c_str(), STORE_DATA, false);
	
	//storing_path = outputPath + "cropped_lungs_p7.raw";
	//manageRAWFile3D<dataType>(maskLungs, length, width, height, storing_path.c_str(), STORE_DATA, false);

	rouyTourinFunction_3D(distanceMap, maskLungs, 0.5, length, width, height, 0.4, ctSpacing.sx);
	//fastSweepingFunction_3D(distanceMap, maskLungs, length, width, height, ctSpacing.sx, 10000000.0, 0.0);
	//fastMarching(distanceMap, maskLungs, height, length, width, 0.0);

	Point3D point_max = getPointWithTheHighestValue(distanceMap, length, width, height);

	//storing_path = outputPath + "distance_map_corners.raw";
	//manageRAWFile3D<dataType>(distanceMap, length, width, height, storing_path.c_str(), STORE_DATA, false);

	Image_Data croppedImageData
	{
		height,
		length,
		width,
		imageCropped,
		cropped_origin,
		intSpacing,
		orientation
	};

	double radius = 10;
	regionGrowing(croppedImageData, segment, length, width, height, point_max, radius);

	storing_path = outputPath + "segmentedAreaP7.raw";
	manageRAWFile3D<dataType>(segment, length, width, height, storing_path.c_str(), STORE_DATA, false);

	////Draw ball for visualization
	//Point3D seed_real_coord = getRealCoordFromImageCoord3D(point_max, cropped_origin, intSpacing, orientation);
	//for (k = 0; k < height; k++) {
	//	for (i = 0; i < length; i++) {
	//		for (j = 0; j < width; j++) {
	//			Point3D current_point = { i, j, k };
	//			current_point = getRealCoordFromImageCoord3D(current_point, cropped_origin, intSpacing, orientation);
	//			double pDistance = getPoint3DDistance(current_point, seed_real_coord);
	//			if (pDistance <= radius) {
	//				ball[k][x_new(i, j, length)] = 1.0;
	//			}
	//		}
	//	}
	//}
	//storing_path = outputPath + "ball_15_RT_corners.raw";
	//manageRAWFile3D<dataType>(ball, length, width, height, storing_path.c_str(), STORE_DATA, false);

	
	//Save data information
	string store_information = outputPath + "Test_details_p7.txt";
	FILE* info_test;
	if (fopen_s(&info_test, store_information.c_str(), "w") != 0) {
		printf("Enable to open");
		return false;
	}
	//Original image
	fprintf(info_test, "Input : patient 7\n");
	fprintf(info_test, "Dimension : Length = %d, Width = %d, Height = %d\n", Length, Width, Height);
	fprintf(info_test, "Origin (%f, %f, %f)\n", ctOrigin.x, ctOrigin.y, ctOrigin.z);
	fprintf(info_test, "Spacing (%f, %f, %f)\n", ctSpacing.sx, ctSpacing.sy, ctSpacing.sz);
	//Description
	fprintf(info_test, "\nThe lungs segmentation is used to find lungs bounding box\n");
	fprintf(info_test, "In order to have better distance map, isotropic voxel is used\n");
	fprintf(info_test, "which means interpolation in z direction to have cubic like voxel\n");
	fprintf(info_test, "We used nearest neigbor interpolation, because the input is binary\n");
	fprintf(info_test, "\nInterpolated image dimension : Length = %d, Width = %d, Height = %d\n", Length, Width, height_int);
	//Description croppring
	fprintf(info_test, "The lungs pixels min and max coordinates are used (+-1, to have all the lungs pixels) to define the lungs bounding box\n");
	fprintf(info_test, "x_min = %d, x_max = %d\n", i_min, i_max);
	fprintf(info_test, "y_min = %d, y_max = %d\n", j_min, j_max);
	fprintf(info_test, "z_min = %d, z_max = %d\n", k_min, k_max);
	fprintf(info_test, "Cropped image dimension : Length = %d, Width = %d, Height = %d\n", length, width, height);
	fprintf(info_test, "Cropped image origin : (%f, %f, %f)\n", cropped_origin.x, cropped_origin.y, cropped_origin.z);
	//Description distance map
	fprintf(info_test, "The Rouy-Tourin is used to compute the distance map\n");
	fclose(info_test);

	for (k = 0; k < Height; k++) {
		delete[] imageData[k];
	}
	delete[] imageData;

	for (k = 0; k < height_int; k++) {
		delete[] maskThreshold[k];
		delete[] interpolatedImage[k];
	}
	delete[] maskThreshold;
	delete[] interpolatedImage;

	for (k = 0; k < height; k++) {
		delete[] maskLungs[k];
		delete[] ball[k];
		delete[] distanceMap[k];
		delete[] imageCropped[k];
		delete[] segment[k];
	}
	delete[] maskLungs;
	delete[] ball;
	delete[] distanceMap;
	delete[] imageCropped;
	delete[] segment;

	free(ctContainer);
	*/

	//========================= Lungs centroid ==================================
	
	/*
	dataType** imageData = new dataType * [Height] {0};
	dataType** maskLungs = new dataType * [Height] {0};
	dataType** segment = new dataType * [Height] {0};
	for (k = 0; k < Height; k++) {
		imageData[k] = new dataType[dim2D]{ 0 };
		maskLungs[k] = new dataType[dim2D]{ 0 };
		segment[k] = new dataType[dim2D]{ 0 };
	}

	loading_path = outputPath + "/segmentation lungs/segment_lungs_p3.raw"; // Find lungs centroid
	manageRAWFile3D<dataType>(imageData, Length, Width, Height, loading_path.c_str(), LOAD_DATA, false);

	Image_Data imageDataStr
	{
		Height,
		Length,
		Width,
		imageData,
		ctOrigin,
		ctSpacing,
		orientation
	};

	loading_path = inputPath + "raw/filtered/filteredGMC_p7.raw";// segment from centroid
	manageRAWFile3D<dataType>(imageData, Length, Width, Height, loading_path.c_str(), LOAD_DATA, false);

	dataType* centroid_lungs = new dataType[3]{ 0 };
	centroidImage(maskLungs, centroid_lungs, Height, Length, Width, 0.0);

	Point3D pCentroid = { centroid_lungs[0], centroid_lungs[1] , centroid_lungs[2] };
	double radius = 10;

	//pCentroid = getRealCoordFromImageCoord3D(pCentroid, ctOrigin, ctSpacing, orientation);
	////Draw ball for visualization -- original dimension
	//for (k = 0; k < Height; k++) {
	//	for (i = 0; i < Length; i++) {
	//		for (j = 0; j < Width; j++) {
	//			Point3D current_point = { i, j, k };
	//			current_point = getRealCoordFromImageCoord3D(current_point, ctOrigin, ctSpacing, orientation);
	//			double pDistance = getPoint3DDistance(current_point, pCentroid);
	//			if (pDistance <= radius) {
	//				ballOriginal[k][x_new(i, j, Length)] = 1.0;
	//			}
	//		}
	//	}
	//}

	regionGrowing(imageDataStr, segment, Length, Width, Height, pCentroid, radius);

	storing_path = outputPath + "segment_from_lungs_centroid_p7.raw";
	manageRAWFile3D<dataType>(segment, Length, Width, Height, storing_path.c_str(), STORE_DATA, false);

	for (k = 0; k < Height; k++) {
		delete[] imageData[k];
		delete[] maskLungs[k];
		delete[] segment[k];
	}
	delete[] imageData;
	delete[] maskLungs;
	delete[] segment;
	*/
	
	/*
	//const size_t hauteur = 866; //p2
	//const size_t hauteur = 1083;//p3
	const size_t hauteur = 844;//p4, p5
	//const size_t hauteur = 1351;//p6
	//const size_t hauteur = 932;//p7

	VoxelSpacing interpolatedSpacing = { ctSpacing.sx, ctSpacing.sy, ctSpacing.sx };
	
	dataType** maskLungs = new dataType * [hauteur];
	dataType** lungsData = new dataType * [hauteur];
	dataType** segmentHeart = new dataType * [hauteur];
	dataType** ball = new dataType * [hauteur];
	for (k = 0; k < hauteur; k++) {
		maskLungs[k] = new dataType[dim2D]{ 0 };
		lungsData[k] = new dataType[dim2D]{ 0 };
		segmentHeart[k] = new dataType[dim2D]{ 0 };
		ball[k] = new dataType[Length * Width]{ 0 };
	}

	loading_path = inputPath + "/raw/interpolated/filtered_p4.raw";
	manageRAWFile3D<dataType>(lungsData, Length, Width, hauteur, loading_path.c_str(), LOAD_DATA, false);

	Image_Data imageDataInt
	{
		hauteur,
		Length,
		Width,
		lungsData,
		ctOrigin,
		interpolatedSpacing,
		orientation
	};

	loading_path = outputPath + "/segmentation lungs/interpolated/lungs_interpolated_p4.raw";
	manageRAWFile3D<dataType>(maskLungs, Length, Width, hauteur, loading_path.c_str(), LOAD_DATA, false);

	dataType* centroid_lungs = new dataType[3]{ 0 };
	centroidImage(maskLungs, centroid_lungs, hauteur, Length, Width, 0.0);
	Point3D pCentroid = { centroid_lungs[0], centroid_lungs[1] , centroid_lungs[2] };
	double radius = 10;
	
	//regionGrowing(imageDataInt, segmentHeart, Length, Width, hauteur, pCentroid, radius);

	//storing_path = outputPath + "segment_from_lungs_centroid_p2.raw";
	//manageRAWFile3D<dataType>(segmentHeart, Length, Width, hauteur, storing_path.c_str(), STORE_DATA, false);

	////Draw ball for visualization
	//pCentroid = getRealCoordFromImageCoord3D(pCentroid, ctOrigin, interpolatedSpacing, orientation);
	//for (k = 0; k < hauteur; k++) {
	//	for (i = 0; i < Length; i++) {
	//		for (j = 0; j < Width; j++) {
	//			Point3D current_point = { i, j, k };
	//			current_point = getRealCoordFromImageCoord3D(current_point, ctOrigin, interpolatedSpacing, orientation);
	//			double pDistance = getPoint3DDistance(current_point, pCentroid);
	//			if (pDistance <= radius) {
	//				ball[k][x_new(i, j, Length)] = 1.0;
	//			}
	//		}
	//	}
	//}
	//storing_path = outputPath + "ball_lungs_centroid_p2.raw";
	//manageRAWFile3D<dataType>(ball, Length, Width, hauteur, storing_path.c_str(), STORE_DATA, false);

	//Find lungs bounding box
	size_t k_min = hauteur, k_max = 0;
	size_t i_min = Length, i_max = 0;
	size_t j_min = Width, j_max = 0;
	for (k = 0; k < hauteur; k++) {
		for (i = 0; i < Length; i++) {
			for (j = 0; j < Width; j++) {
				if (maskLungs[k][x_new(i, j, Length)] == 1.0) {
					if (i_min > i) {
						i_min = i;
					}
					if (j_min > j) {
						j_min = j;
					}
					if (k_min > k) {
						k_min = k;
					}

					if (i_max < i) {
						i_max = i;
					}
					if (j_max < j) {
						j_max = j;
					}
					if (k_max < k) {
						k_max = k;
					}
				}
			}
		}
	}

	//Adjust the bounding box to have all the voxels
	if (i_min > 0) {
		i_min -= 1;
	}
	if (j_min > 0) {
		j_min -= 1;
	}
	if (k_min > 0) {
		k_min -= 1;
	}
	if (i_max < Length - 1) {
		i_max += 1;
	}
	if (j_max < Width - 1) {
		j_max += 1;
	}
	if (k_max < hauteur - 1) {
		k_max += 1;
	}

	//cropped dimension
	const size_t height = k_max - k_min + 1;
	const size_t length = i_max - i_min + 1;
	const size_t width = j_max - j_min + 1;
	std::cout << "New dim : " << length << " x " << width << " x " << height << std::endl;

	Point3D croppedOrigin = { i_min, j_min, k_min };
	croppedOrigin = getRealCoordFromImageCoord3D(croppedOrigin, ctOrigin, interpolatedSpacing, orientation);
	std::cout << "Cropped image origin : (" << croppedOrigin.x << ", " << croppedOrigin.y << ", " << croppedOrigin.z << ")" << std::endl;

	dataType** croppedLungs = new dataType * [height];
	dataType** segmentCrop = new dataType * [height];
	dataType** distanceMap = new dataType * [height];
	dataType** maskThreshold = new dataType * [height];
	dataType** ballMaxDist = new dataType * [height];
	for (k = 0; k < height; k++) {
		croppedLungs[k] = new dataType[length * width]{ 0 };
		segmentCrop[k] = new dataType[length * width]{ 0 };
		distanceMap[k] = new dataType[length * width]{ 0 };
		maskThreshold[k] = new dataType[length * width]{ 0 };
		ballMaxDist[k] = new dataType[length * width]{ 0 };
	}

	//Find the lungs farest point to its centroid
	pCentroid = getRealCoordFromImageCoord3D(pCentroid, ctOrigin, interpolatedSpacing, orientation);
	double farest_distance = 0.0;
	Point3D farest_point;
	for (k = 0; k < hauteur; k++) {
		for (i = 0; i < Length; i++) {
			for (j = 0; j < Width; j++) {
				if (maskLungs[k][x_new(i, j, Length)] == 1.0) {
					Point3D current_point = { i, j, k };
					current_point = getRealCoordFromImageCoord3D(current_point, ctOrigin, interpolatedSpacing, orientation);
					double fDistance = getPoint3DDistance(current_point, pCentroid);
					if (farest_distance < fDistance) {
						farest_distance = fDistance;
						farest_point.x = i;
						farest_point.y = j;
						farest_point.z = k;
					}
				}
			}
		}
	}

	//Cropping
	farest_point = getRealCoordFromImageCoord3D(farest_point, ctOrigin, interpolatedSpacing, orientation);
	std::cout << "farest point : (" << farest_point.x << ", " << farest_point.y << ", " << farest_point.z << ")" << std::endl;
	size_t k_ext, i_ext, j_ext;
	for (k = 0, k_ext = k_min; k < height; k++, k_ext++) {
		for (i = 0, i_ext = i_min; i < length; i++, i_ext++) {
			for (j = 0, j_ext = j_min; j < width; j++, j_ext++) {

				//Cropping according lungs bounding box
				//maskThreshold[k][x_new(i, j, length)] = maskLungs[k_ext][x_new(i_ext, j_ext, Length)];
				
				////Cropping according lungs bounding box
				//// and invert foreground and background pixels
				//if (maskLungs[k_ext][x_new(i_ext, j_ext, Length)] == 1.0) {
				//	maskThreshold[k][x_new(i, j, length)] = 0.0;
				//}
				//else {
				//	maskThreshold[k][x_new(i, j, length)] = 1.0;
				//}

				//Cropping according lungs farest point to the centroid
				Point3D current_point = { i, j, k };
				current_point = getRealCoordFromImageCoord3D(current_point, croppedOrigin, interpolatedSpacing, orientation);
				double fDistance = getPoint3DDistance(current_point, pCentroid);
				if (fDistance <= farest_distance) {
					croppedLungs[k][x_new(i, j, length)] = lungsData[k_ext][x_new(i_ext, j_ext, Length)];
					if (maskLungs[k_ext][x_new(i_ext, j_ext, Length)] == 1.0) {
						maskThreshold[k][x_new(i, j, length)] = 0.0;
					}
					else {
						maskThreshold[k][x_new(i, j, length)] = 1.0;
					}
				}
			}
		}
	}

	storing_path = outputPath + "cropped_new_p4.raw";
	manageRAWFile3D<dataType>(maskThreshold, length, width, height, storing_path.c_str(), STORE_DATA, false);

	
	//Save data information
	string store_information = outputPath + "info_cropping_p4.txt";
	FILE* info_test;
	if (fopen_s(&info_test, store_information.c_str(), "w") != 0) {
		printf("Enable to open");
		return false;
	}
	//Original image
	fprintf(info_test, "Input : patient 4\n");
	fprintf(info_test, "Dimension : Length = %d, Width = %d, Height = %d\n", Length, Width, hauteur);
	fprintf(info_test, "Origin (%f, %f, %f)\n", ctOrigin.x, ctOrigin.y, ctOrigin.z);
	fprintf(info_test, "Spacing (%f, %f, %f)\n", ctSpacing.sx, ctSpacing.sy, ctSpacing.sx);
	//Description
	//fprintf(info_test, "\nThe lungs segmentation is used to find lungs bounding box\n");
	//fprintf(info_test, "In order to have better distance map, isotropic voxel is used\n");
	//fprintf(info_test, "which means interpolation in z direction to have cubic like voxel\n");
	//fprintf(info_test, "We used nearest neigbor interpolation, because the input is binary\n");
	//fprintf(info_test, "\nInterpolated image dimension : Length = %d, Width = %d, Height = %d\n", Length, Width, height_int);
	//Description croppring
	fprintf(info_test, "The lungs pixels min and max coordinates are used (+-1, to have all the lungs pixels) to define the lungs bounding box\n");
	fprintf(info_test, "x_min = %d, x_max = %d\n", i_min, i_max);
	fprintf(info_test, "y_min = %d, y_max = %d\n", j_min, j_max);
	fprintf(info_test, "z_min = %d, z_max = %d\n", k_min, k_max);
	fprintf(info_test, "Cropped image dimension : Length = %d, Width = %d, Height = %d\n", length, width, height);
	fprintf(info_test, "Cropped image origin : (%f, %f, %f)\n", croppedOrigin.x, croppedOrigin.y, croppedOrigin.z);
	//Description distance map
	//fprintf(info_test, "The Rouy-Tourin is used to compute the distance map\n");
	fclose(info_test);
	

	//storing_path = outputPath + "cropped_lungs_p2.raw";
	//manageRAWFile3D<dataType>(maskThreshold, length, width, height, storing_path.c_str(), STORE_DATA, false);

	rouyTourinFunction_3D(distanceMap, maskThreshold, 0.5, length, width, height, 0.4, ctSpacing.sx);

	storing_path = outputPath + "distance_map_cropped_updated_p4.raw";
	manageRAWFile3D<dataType>(distanceMap, length, width, height, storing_path.c_str(), STORE_DATA, false);

	Point3D point_max = getPointWithTheHighestValue(distanceMap, length, width, height);
	Point3D point_max_real_coord = getRealCoordFromImageCoord3D(point_max, croppedOrigin, interpolatedSpacing, orientation);

	//Draw ball for visualization
	for (k = 0; k < height; k++) {
		for (i = 0; i < length; i++) {
			for (j = 0; j < width; j++) {
				Point3D current_point = { i, j, k };
				current_point = getRealCoordFromImageCoord3D(current_point, croppedOrigin, interpolatedSpacing, orientation);
				double pDistance = getPoint3DDistance(current_point, point_max_real_coord);
				if (pDistance <= radius) {
					ballMaxDist[k][x_new(i, j, length)] = 1.0;
				}
			}
		}
	}
	storing_path = outputPath + "ball_max_distance_updated_p4.raw";
	manageRAWFile3D<dataType>(ballMaxDist, length, width, height, storing_path.c_str(), STORE_DATA, false);

	Image_Data imageDataCropU
	{
		height,
		length,
		width,
		croppedLungs,
		croppedOrigin,
		interpolatedSpacing,
		orientation
	};
	regionGrowing(imageDataCropU, segmentCrop, length, width, height, point_max, radius);

	storing_path = outputPath + "segment_from_max_distance_p4.raw";
	manageRAWFile3D<dataType>(segmentCrop, length, width, height, storing_path.c_str(), STORE_DATA, false);

	for (k = 0; k < height; k++) {
		delete[] croppedLungs[k];
		delete[] segmentCrop[k];
		delete[] distanceMap[k];	
		delete[] maskThreshold[k];
		delete[] ballMaxDist[k];
	}
	delete[] croppedLungs;
	delete[] segmentCrop;
	delete[] distanceMap;
	delete[] maskThreshold;
	delete[] ballMaxDist;
	
	delete[] centroid_lungs;
	for (k = 0; k < hauteur; k++) {
		delete[] maskLungs[k];
		delete[] lungsData[k];
		delete[] segmentHeart[k];
		delete[] ball[k];
	}
	delete[] maskLungs;
	delete[] lungsData;
	delete[] segmentHeart;
	delete[] ball;

	free(ctContainer);
	*/

	//========================= Region growing ==================================

	Height = 866;

	dataType** imageData = new dataType * [Height];
	dataType** segment = new dataType * [Height];
	dataType** ball = new dataType * [Height];
	dataType** potential = new dataType * [Height];
	for (k = 0; k < Height; k++) {
		imageData[k] = new dataType[dim2D]{ 0 };
		segment[k] = new dataType[dim2D]{ 0 };
		ball[k] = new dataType[dim2D]{ 0 };
		potential[k] = new dataType[dim2D]{ 0 };
	}

	//loading_path = inputPath + "/raw/filtered/filteredGMC_p2.raw";
	loading_path = inputPath + "/raw/interpolated/filtered_p2.raw";
	manageRAWFile3D<dataType>(imageData, Length, Width, Height, loading_path.c_str(), LOAD_DATA, false);

	VoxelSpacing interpolatedSpacing = { ctSpacing.sx, ctSpacing.sy, ctSpacing.sx };

	Image_Data ctImageData
	{
		Height,
		Length,
		Width,
		imageData,
		ctOrigin,
		interpolatedSpacing ,
		orientation
	};

	Potential_Parameters parameters = {
		0.005,//K
		0.01,//eps
		ctSpacing.sx,//h
		radius
	};

	//Point3D point_liver = {180, 275, 200};
	//Point3D point_aorta = { 262, 257, 147 };
	Point3D point_aorta = { 260, 257, 308 };
	double radius = 3.0;

	compute3DPotential(ctImageData, potential, point_aorta, parameters);

	ctImageData.imageDataPtr = potential;

	regionGrowing(ctImageData, segment, Length, Width, Height, point_aorta, radius);

	storing_path = outputPath + "segment_from_aorta_p2.raw";
	manageRAWFile3D<dataType>(segment, Length, Width, Height, storing_path.c_str(), STORE_DATA, false);

	//Point3D pBall = getRealCoordFromImageCoord3D(point_aorta, ctOrigin, interpolatedSpacing, orientation);
	//for (k = 0; k < hauteur; k++) {
	//	for (i = 0; i < Length; i++) {
	//		for (j = 0; j < Width; j++) {
	//			Point3D current_point = { i, j, k };
	//			current_point = getRealCoordFromImageCoord3D(current_point, ctOrigin, interpolatedSpacing, orientation);
	//			double pDistance = getPoint3DDistance(current_point, pBall);
	//			if (pDistance <= radius) {
	//				ball[k][x_new(i, j, Length)] = 1.0;
	//			}
	//		}
	//	}
	//}
	//storing_path = outputPath + "ball_seed_p2.raw";
	//manageRAWFile3D<dataType>(ball, Length, Width, hauteur, storing_path.c_str(), STORE_DATA, false);

	for (k = 0; k < Height; k++) {
		delete[] imageData[k];
		delete[] segment[k];
		delete[] ball[k];
		delete[] potential[k];
	}
	delete[] imageData;
	delete[] segment;
	delete[] ball;
	delete[] potential;

	free(ctContainer);

	//========================= Heart Segmentation by SUBSURF ===================
	
	/*
	//Get cropped image using segmented lungs

	dataType** imageData = new dataType * [Height];
	dataType** lungsData = new dataType * [Height];
	dataType** initialSegment = new dataType * [Height];
	for (k = 0; k < Height; k++) {
		imageData[k] = new dataType[dim2D]{ 0 };
		lungsData[k] = new dataType[dim2D]{ 0 };
		initialSegment[k] = new dataType[dim2D]{ 0 };
	}
	
	loading_path = inputPath + "raw/filtered/filteredGMC_p2.raw";
	manageRAWFile3D<dataType>(imageData, Length, Width, Height, loading_path.c_str(), LOAD_DATA, false);

	loading_path = inputPath + "lungs.raw";
	manageRAWFile3D<dataType>(lungsData, Length, Width, Height, loading_path.c_str(), LOAD_DATA, false);

	//Find lungs bounding box
	size_t i_min = Length, i_max = 0;
	size_t j_min = Width, j_max = 0;
	size_t k_min = Height, k_max = 0;
	for (k = 0; k < Height; k++) {
		for (i = 0; i < Length; i++) {
			for (j = 0; j < Width; j++) {
				xd = x_new(i, j, Length);
				if (lungsData[k][xd] == 1.0) {
					if (i < i_min) {
						i_min = i;
					}
					if (j < j_min) {
						j_min = j;
					}
					if (k < k_min) {
						k_min = k;
					}

					if (i > i_max) {
						i_max = i;
					}
					if (j > j_max) {
						j_max = j;
					}
					if (k > k_max) {
						k_max = k;
					}
				}
			}
		}
	}

	//adjust the parameters to have nicer dimensions
	i_min -= 3; i_max += 2;
	j_min -= 1;
	k_min -= 1;

	size_t length = i_max - i_min + 1;
	size_t width = j_max - j_min + 1;
	size_t height = k_max - k_min + 1;

	std::cout << "New dim : " << length << "x" << width << "x" << height << std::endl;
	Point3D newOrigin = { i_min, j_min, k_min };
	newOrigin = getRealCoordFromImageCoord3D(newOrigin, ctOrigin, ctSpacing, orientation);
	std::cout << "New origin : (" << newOrigin.x << "," << newOrigin.y << "," << newOrigin.z  << ")" << std::endl;

	dataType** croppedImage = new dataType * [height];
	for (k = 0; k < height; k++) {
		croppedImage[k] = new dataType[length * width];
	}
	
	//Load the initial segment
	loading_path = inputPath + "ball_threshold.raw";
	manageRAWFile3D<dataType>(initialSegment, Length, Width, Height, loading_path.c_str(), LOAD_DATA, false);

	//remove pixels out of the bounding box
	size_t kn, in, jn, xdn;
	for (kn = 0, k = k_min; kn < height; kn++, k++) {
		for (in = 0, i = i_min; in < length; in++, i++) {
			for (jn = 0, j = j_min; jn < width; jn++, j++) {
				xdn = x_new(in, jn, length);
				xd = x_new(i, j, Length);
				//croppedImage[kn][xdn] = imageData[k][xd];
				croppedImage[kn][xdn] = initialSegment[k][xd];
			}
		}
	}

	storing_path = outputPath + "initialSegment.raw";
	manageRAWFile3D<dataType>(croppedImage, length, width, height, storing_path.c_str(), STORE_DATA, false);

	for (k = 0; k < height; k++) {
		delete[] croppedImage[k];
	}
	delete[] croppedImage;

	for (k = 0; k < Height; k++) {
		delete[] imageData[k];
		delete[] lungsData[k];
		delete[] initialSegment[k];
	}
	delete[] imageData;
	delete[] lungsData;
	delete[] initialSegment;
	*/
	
	/*
	size_t height = 100;
	size_t length = 240;
	size_t width = 160;
	size_t dim2D = length * width;
	
	dataType** imageData = new dataType * [height];
	dataType** initialSegment = new dataType * [height];
	for (k = 0; k < height; k++) {
		imageData[k] = new dataType[dim2D]{ 0 };
		initialSegment[k] = new dataType[dim2D]{ 0 };
	}

	loading_path = inputPath + "croppedImage.raw";
	manageRAWFile3D<dataType>(imageData, length, width, height, loading_path.c_str(), LOAD_DATA, false);

	loading_path = inputPath + "initialSegment.raw";
	manageRAWFile3D<dataType>(initialSegment, length, width, height, loading_path.c_str(), LOAD_DATA, false);

	//Smoothing by heat equation, we don't need all the parameters
	Filter_Parameters smoothing_parameters{
		0.2, //tau
		1.0, //h
		0.0, //sigma
		0.0, //edge detector coeficient
		1.5, //omega
		1e-3,//tolerance
		0.0, // epsilon Evan-Spruck regularization
		0.0, // coef
		(size_t)1, //p
		(size_t)1, //time step number
		(size_t)100 //max number of solver iterations
	};

	Segmentation_Parameters segmentation_parameters{
		(size_t)30,    // Maximum number of Gauss - Seidel iterations
		500,         // K : edge detector coef
		1e-6,          //epsilon : Evan-Spruck regularization
		(size_t)7000,   //number of current time step
		(size_t)7000,   //max number of time step
		(size_t)10,     //frequence for saving
		1e-6,          //segmentation tolerance
		1.0 / 4.6875,    //tau
		4.6875,        //h
		1.5,           //omega
		1e-3,          //gauss_seidel tolerance
		1.0,          //convection coeff
		1.0           //diffusion coeff
	};

	Point3D pOrigin = { -131.25, 2.03125, -520 };
	Image_Data inputImage
	{
		height,
		length,
		width,
		imageData,
		pOrigin,
		ctSpacing,
		orientation
	};

	size_t h = height / 4, l = length / 4, w = width / 4;
	std::cout << h << "," << l << "," << w << std::endl;

 	dataType** imageDown = new dataType * [h];
	dataType** segmentDown = new dataType * [h];
	for (k = 0; k < h; k++) {
		imageDown[k] = new dataType[l * w];
		segmentDown[k] = new dataType[l * w];
	}
	VoxelSpacing intSpacing = { ctSpacing.sx * 4, ctSpacing.sy * 4, ctSpacing.sz * 4 };
	
	Image_Data imageForDownSampling
	{
		h,
		l,
		w,
		imageDown,
		pOrigin,
		intSpacing,
		orientation
	};

	imageInterpolation3D(inputImage, imageForDownSampling, NEAREST_NEIGHBOR);

	inputImage.imageDataPtr = initialSegment;
	imageForDownSampling.imageDataPtr = segmentDown;
	imageInterpolation3D(inputImage, imageForDownSampling, NEAREST_NEIGHBOR);

	Image_Data imageToBeSegmented
	{
		h,
		l,
		w,
		imageDown,
		pOrigin,
		intSpacing,
		orientation
	};

	Point3D* center_seg = new Point3D[1];
	center_seg->x = 41.0;
	center_seg->y = 15.0;
	center_seg->z = 7.0;

	unsigned char segmentPath[] = "C:/Users/Konan Allaly/Documents/Tests/output/meeting 12-12/segmentation/";
	//subsurfSegmentation(imageToBeSegmented, initialSegment, segmentation_parameters, smoothing_parameters, center_seg, 1, segmentPath);
	generalizedSubsurfSegmentation(imageToBeSegmented, segmentDown, segmentation_parameters, smoothing_parameters, center_seg, 1, segmentPath);

	delete[] center_seg;
	for (k = 0; k < h; k++) {
		delete[] imageDown[k];
		delete[] segmentDown[k];
	}
	delete[] imageDown;
	delete[] segmentDown;
	*/
	
	//for (k = 0; k < height; k++) {
	//	delete[] imageData[k];
	//	delete[] initialSegment[k];
	//}
	//delete[] imageData;
	//delete[] initialSegment;
	//free(ctContainer);
	
	//========================= Detection aorta bifurcation =====================

	/*
	//const size_t height = 293;
	dataType** imageData = new dataType * [Height];
	for (k = 0; k < Height; k++) {
		imageData[k] = new dataType[dim2D]{ 0 };
	}

	//loading_path = inputPath + "raw/interpolated/filtered_p2.raw";
	//manageRAWFile3D<dataType>(imageData, Length, Width, height, loading_path.c_str(), LOAD_DATA, false);

	if (minData < 0) {
		for (k = 195; k < 300; k++) {
			for (i = 0; i < dim2D; i++) {
				imageData[k][i] = ctContainer->dataPointer[k][i] - minData;
			}
		}
	}
	else {
		imageData[k][i] = ctContainer->dataPointer[k][i];
	}

	dataType mean_value = getImageMeanValue3D(imageData, Length, Width, Height);

	thresholding3dFunctionN(imageData, Length, Width, Height, 1062, 1062, 1.0, 0.0);

	storing_path = outputPath + "Foreground.raw";
	manageRAWFile3D(imageData, Length, Width, Height, storing_path.c_str(), STORE_DATA, false);

	//const size_t bins = 256;
	//storing_path = outputPath + "histogram/histogram_p2.csv";
	//size_t nb_peaks = histogramPeaks3D(imageData, Length, Width, Height, bins, storing_path.c_str());

	//dataType* imageSlice = new dataType[dim2D]{ 0 };
	//size_t idSlice = 240;
	//copyDataToAnother2dArray(imageData[idSlice], imageSlice, Length, Width);

	//storing_path = outputPath + "histogram/slice240.raw";
	//manageRAWFile2D<dataType>(imageSlice, Length, Width, storing_path.c_str(), STORE_DATA, false);

	//PixelSpacing sliceSpacing = { ctSpacing.sx, ctSpacing.sy };
	//HoughParameters parameters
	//{
	//	6.0,//minimum radius
	//	19.0,//maximum radius
	//	0.5,//radius step
	//	0.5,//epsilon
	//	1.0,//offset
	//	1000,//edge detector coefficient
	//	0.2,//threshold for edge detector
	//	sliceSpacing//spacing == finite volume size
	//};
	//houghTransform(imageData[idSlice], foundCirclePtr, Length, Width, parameters);
	//dataType mean_val = getImageMeanValue(imageData[idSlice], Length, Width);
	//std::cout << "mean = " << mean_val << std::endl;

	//size_t* histogram = new size_t[bins]{ 0 };
	////computeImageHistogram(imageSlice, Length, Width, bins, histogram);
	//computeHistogram(imageData, histogram, Length, Width, Height, bins);

	//storing_path = outputPath + "histogram/histogram_p2.csv";
	//FILE* file_peaks;
	//if (fopen_s(&file_peaks, storing_path.c_str(), "w") != 0) {
	//	printf("Enable to open");
	//	return false;
	//}
	//fprintf(file_peaks, "x,y\n");
	//
	//dataType step = (maxData - minData) / bins;
	//for (size_t n = 0; n < bins; n++) {
	//	dataType inf_data = n * step;
	//	fprintf(file_peaks, "%f,%d\n", inf_data, histogram[n]);
	//}
	//fclose(file_peaks);

	//size_t nb_peaks = findHistogramPeaks(imageSlice, Length, Width, bins, storing_path.c_str());

	//for (k = 0; k < Height; k++) {
	//	size_t nb_peaks = findHistogramPeaks(imageData[k], Length, Width, bins, storing_path.c_str());
	//	fprintf(file_peaks, "%d,%d\n", k, nb_peaks);
	//}
	//fclose(file_peaks);

	//thresholding2DFunction(imageData[idSlice], Length, Width, mean_val, mean_val);
	//storing_path = outputPath + "threshold.raw";
	//manageRAWFile2D<dataType>(imageData[idSlice], Length, Width, storing_path.c_str(), STORE_DATA, false);

	//delete[] imageSlice;
	for (k = 0; k < Height; k++) {
		delete[] imageData[k];
	}
	delete[] imageData;
	free(ctContainer);
	*/

	//========================= Test Clache ======================================

	/*
	const size_t Length = 512, Width = 512;
	size_t dim2D = Length * Width;
	dataType* imageData = new dataType[dim2D]{ 0 };

	loading_path = inputPath + "raw/slice/Image2.raw";
	manageRAWFile2D(imageData, Length, Width, (const char*)loading_path.c_str(), LOAD_DATA, false);

	const size_t bins = 256;
	size_t* histogram = new size_t[bins]{ 0 };
	computeImageHistogram(imageData, Length, Width, bins, histogram);

	storing_path = outputPath + "clache/imageHistogram.csv";
	FILE* file_histogram;
	if (fopen_s(&file_histogram, storing_path.c_str(), "w") != 0) {
		printf("Enable to open");
		return false;
	}
	fprintf(file_histogram, "x,y\n");
	for (size_t n = 0; n < bins; n++) {
		fprintf(file_histogram,"%d,%d\n", n, histogram[n]);
	}
	fclose(file_histogram);
	
	//const size_t div = 8;
	//CLACHE(imageData, Length, Width, div);
	//storing_path = outputPath + "region36.raw";
	//manageRAWFile2D(imageData, Length, Width, (const char*)storing_path.c_str(), STORE_DATA, false);

	delete[] histogram;
	delete[] imageData;
	*/

	//========================= Ajust image generated around found path ==========

	/*
	const size_t height = 1351;
	const size_t length = 512, width = 512;
	const size_t dim2D = length * width;

	dataType** pImageData = new dataType * [height];
	dataType** gImageData = new dataType * [height];
	for (k = 0; k < height; k++) {
		pImageData[k] = new dataType[dim2D]{ 0 };
		gImageData[k] = new dataType[dim2D]{ 0 };
	}

	////Load pImage
	//loading_path = "C:/Users/Konan Allaly/Documents/Tests/Curves/Output/path_aorta_p6.raw";
	//load3dArrayRAW<dataType>(pImageData, length, width, height, loading_path.c_str(), false);
	
	loading_path = "C:/Users/Konan Allaly/Documents/Tests/Curves/Output/pImage.raw";
	load3dArrayRAW<dataType>(gImageData, length, width, height, loading_path.c_str(), false);

	int** labelArray = new int*[height];
	bool** status = new bool* [height];
	dataType** pThreshold = new dataType * [height];
	for (k = 0; k < height; k++) {
		labelArray[k] = new int[dim2D] { 0 };
		status[k] = new bool[dim2D] {false};
		pThreshold[k] = new dataType[dim2D]{ 0 };
	}
	if (labelArray == NULL || status == NULL)
		return false;

	for (k = 0; k < height; k++) {
		for (i = 0; i < length; i++) {
			for (j = 0; j < width; j++) {
				xd = x_new(i, j, length);
				if (gImageData[k][xd] >= 0.257 && gImageData[k][xd] <= 0.27 && i > 200 && (k > 655 && k < 1067)) {
					pThreshold[k][xd] = 1.0;
				}
			}
		}
	}

	//dataType thresh = 0.257;
	//thresholding3dFunctionN(pThreshold, length, width, height, thresh, thresh, 1.0, 0.0);
	//erosion3D(pThreshold, length, width, height, 1.0, 0.0);
	//erosion3dHeighteenNeigbours(pThreshold, length, width, height, 1.0, 0.0);
	//erosion3dHeighteenNeigbours(pThreshold, length, width, height, 1.0, 0.0);
	erosion3dHeighteenNeigbours(pThreshold, length, width, height, 1.0, 0.0);

	//storing_path = outputPath + "threshold.raw";
	//store3dRawData<dataType>(pThreshold, length, width, height, storing_path.c_str());

	int numberOfRegionCells = countNumberOfRegionsCells(pThreshold, length, width, height, 1.0);

	labelling3D(pThreshold, labelArray, status, length, width, height, 1.0);

	//Counting
	int* countingArray = new int[numberOfRegionCells] {0};
	if (countingArray == NULL)
		return false;

	//Get regions sizes
	for (k = 0; k < height; k++) {
		for (i = 0; i < dim2D; i++) {
			if (labelArray[k][i] > 0) {
				countingArray[labelArray[k][i]]++;
			}
		}
	}

	//Number of regions
	int numberOfRegions = 0;
	for (i = 0; i < numberOfRegionCells; i++) {
		if (countingArray[i] > 0) {
			numberOfRegions++;
		}
	}
	//cout << numberOfRegions << " regions found" << endl;

	//Find the biggest region
	size_t regionSize = 0;
	for (k = 0; k < height; k++) {
		for (i = 0; i < dim2D; i++) {
			if (countingArray[labelArray[k][i]] > regionSize) {
				regionSize = countingArray[labelArray[k][i]];
			}
		}
	}

	//Keep and save the biggest region
	for (k = 0; k < height; k++) {
		for (i = 0; i < dim2D; i++) {
			if (countingArray[labelArray[k][i]] == regionSize) {
				pThreshold[k][i] = 1.0;
			}
			else {
				pThreshold[k][i] = 0.0;
			}
		}
	}

	storing_path = outputPath + "segmented.raw";
	store3dRawData<dataType>(pThreshold, length, width, height, storing_path.c_str());

	delete[] countingArray;
	for (k = 0; k < height; k++) {
		delete[] labelArray[k];
		delete[] status[k];
		delete[] pThreshold[k];
	}
	delete[] labelArray;
	delete[] status;
	delete[] pThreshold;

	for (k = 0; k < height; k++) {
		delete[] pImageData[k];
		delete[] gImageData[k];
	}
	delete[] pImageData;
	delete[] gImageData;
	*/

	//========================= Translate as path ================================
	
	/*
	Image_Data beforeTreatment = {
		ctContainer->dimensions[2],
		ctContainer->dimensions[0],
		ctContainer->dimensions[1],
		ctContainer->dataPointer,
		beforeOrigin,
		beforeSpacing,
		orientation
	};
	
	Vtk_File_Info* afterTreatmentContainer = (Vtk_File_Info*)malloc(sizeof(Vtk_File_Info));
	afterTreatmentContainer->operation = copyFrom;

	loading_path = inputPath + "vtk/ct/Patient3_ct.vtk";
	readVtkFile(loading_path.c_str(), afterTreatmentContainer);
	Point3D afterOrigin = { afterTreatmentContainer->origin[0], afterTreatmentContainer->origin[1], afterTreatmentContainer->origin[2] };
	VoxelSpacing afterSpacing = { afterTreatmentContainer->spacing[0], afterTreatmentContainer->spacing[1], afterTreatmentContainer->spacing[0] };

	Image_Data afterTreatment = {
		afterTreatmentContainer->dimensions[2],
		afterTreatmentContainer->dimensions[0],
		afterTreatmentContainer->dimensions[1],
		afterTreatmentContainer->dataPointer,
		afterOrigin,
		afterSpacing,
		orientation
	};

	//Point3D seedP2 = { 5.169647, 71.255859, -656.131958 };//path_ordered_p2.csv
	//Point3D seedP3 = { 257, 255, 522 };

	//Point3D seedP2 = { 5.233856, 71.174133, -660.895447 };//first point of path_points_p2.csv, conference Jasna
	Point3D seedP2 = { 260.466217, 257.001923, 308.569214 };//first point of final_points_p2.csv, conference Jasna
	//seedP2 = getRealCoordFromImageCoord3D(seedP2, beforeOrigin, beforeSpacing, orientation);
	Point3D seedP3 = { 257, 255, 522 }; //chosen manually inside patient 3 aorta 
	//seedP3 = getRealCoordFromImageCoord3D(seedP3, afterOrigin, afterSpacing, orientation); // get correponding point for patient 3

	Point3D translate = { seedP3.x - seedP2.x, seedP3.y - seedP2.y, seedP3.z - seedP2.z };

	//Load path point
	FILE* path_file;
	//loading_path = inputPath + "inputsForPhDStudentConference/path_ordered_p2.csv";
	loading_path = "C:/Users/Konan Allaly/Documents/Presentation Jasna/final_path_p2.csv";
	if (fopen_s(&path_file, loading_path.c_str(), "r") != 0) {
		printf("Enable to open");
		return false;
	}

	FILE* translated_path;
	loading_path = "C:/Users/Konan Allaly/Documents/Presentation Jasna/final_translated_p2.csv";
	if (fopen_s(&translated_path, loading_path.c_str(), "w") != 0) {
		printf("Enable to open");
		return false;
	}

	dataType x = 0, y = 0, z = 0;
	dataType xt = 0, yt = 0, zt = 0;
	size_t count_point = 0;

	while (feof(path_file) == 0) {
		fscanf_s(path_file, "%f", &x);
		fscanf_s(path_file, ",");
		fscanf_s(path_file, "%f", &y);
		fscanf_s(path_file, ",");
		fscanf_s(path_file, "%f", &z);
		fscanf_s(path_file, "\n");
		count_point++;

		xt = x + translate.x;
		yt = y + translate.y;
		zt = z + translate.z;

		fprintf(translated_path, "%f,%f,%f\n", xt, yt, zt);
		//if (count_point == 1) {
		//	std::cout << "The first point is : (" << x << "," << y << "," << z << ")" << endl;
		//}

	}
	fclose(path_file);
	fclose(translated_path);
	free(afterTreatmentContainer);
	*/

	//========================= Test function histogram =========================
	
	/*
	const size_t Length = 512, Width = 512, Height = 406;
	size_t dim2D = Length * Width;

	dataType** imageDataPtr = new dataType * [Height];
	dataType** histogramPtr = new dataType * [Height];
	for (k = 0; k < Height; k++) {
		imageDataPtr[k] = new dataType[dim2D]{ 0 };
		histogramPtr[k] = new dataType[dim2D]{ 0 };
	}

	loading_path = inputPath + "raw/ct/patient2_ct.raw";
	load3dArrayRAW<dataType>(imageDataPtr, Length, Width, Height, loading_path.c_str(), false);

	//const size_t binsCount = 6000;
	//computeHistogram(imageDataPtr, histogramPtr, Length, Width, Height, binsCount);
	//thresholdingOTSU(imageDataPtr, Length, Width, Height, 0.0, 1.0);
	//thresholdingOTSUNew(imageDataPtr, Length, Width, Height, binsCount, 0.0, 1.0);
	//storing_path = outputPath + "thresholdOtsu.raw";
	//store3dRawData<dataType>(imageDataPtr, Length, Width, Height, storing_path.c_str());

	//storing_path = outputPath + "histogram.raw";
	//store3dRawData<dataType>(histogramPtr, Length, Width, Height, storing_path.c_str());

	//2D OTSU method
	//const size_t Length = 260, Width = 260;
	//const size_t dim2D = Length * Width;

	dataType* image2D = new dataType[dim2D]{ 0 };
	dataType* threshold = new dataType[dim2D]{ 0 };

	//2D image extraction
	const size_t slice_number = 210;
	for (i = 0; i < dim2D; i++) {
		image2D[i] = imageDataPtr[slice_number][i];
	}

	//loading_path = inputPath + "raw/slice/memboa.raw";
	//loading_path = inputPath + "raw/slice/eye_1024_1024.raw";
	//load2dArrayRAW<dataType>(image2D, Length, Width, loading_path.c_str(), false);

	//storing_path = outputPath + "loaded.raw";
	//store2dRawData<dataType>(image2D, Length, Width, storing_path.c_str());

	//Point2D seed = {382, 454}; // eye image
	//Point2D seed = { 154, 77 }; // memboa
	//Point2D seed = { 188, 282 };
	Point2D seed1 = { 187, 283 };
	Point2D seed2 = { 249, 216 };
	double radius = 25;

	////thresholdingOTSU2D(image2D, 260, 260, 1.0, 0.0);
	//dataType thres1 = localOTSU2D(image2D, Length, Width, seed1, radius, 1.0, 0.0);
	//dataType thres2 = localOTSU2D(image2D, Length, Width, seed2, radius, 1.0, 0.0);

	//dataType thres = 0.0;
	//if (thres1 > thres2) {
	//	thres = thres1;
	//}
	//else {
	//	thres = thres2;
	//}
	//
	//for (i = 0; i < dim2D; i++) {
	//	if (image2D[i] <= thres) {
	//		threshold[i] = 0.0;
	//	}
	//	else {
	//		threshold[i] = 1.0;
	//	}
	//}

	double offset = 3;
	BoundingBox2D box = findBoundingBox2D(seed1, Length, Width, radius, offset);
	size_t i_min = box.i_min, i_max = box.i_max;
	size_t j_min = box.j_min, j_max = box.j_max;

	//Find the minimal and the maximal pixel value
	dataType min_data = 1000000, max_data = -1000000;
	for (i = i_min; i <= i_max; i++) {
		for (j = j_min; j <= j_max; j++) {
			xd = x_new(i, j, Length);
			if (image2D[xd] < min_data) {
				min_data = image2D[xd];
			}
			if (image2D[xd] > max_data) {
				max_data = image2D[xd];
			}
		}
	}
	cout << "min data = " << min_data << ", max data = " << max_data << endl;

	for (i = 0; i < dim2D; i++) {
		if (image2D[i] >= min_data && image2D[i] <= max_data) {
			threshold[i] = 1.0;
		}
		else {
			threshold[i] = 0.0;
		}
	}

	storing_path = outputPath + "threshold_box.raw";
	store2dRawData<dataType>(threshold, Length, Width, storing_path.c_str());

	delete[] image2D;
	delete[] threshold;

	for (k = 0; k < Height; k++) {
		delete[] imageDataPtr[k];
		delete[] histogramPtr[k];
	}
	delete[] imageDataPtr;
	delete[] histogramPtr;
	*/

	//========================= Load segmented liver and ajust dimension ========

	/*
	////Patient 2 : interpolated
	//size_t i_min = 120, i_max = 375;
	//size_t j_min = 160, j_max = 415;
	//size_t k_min = 335, k_max = 494;

	//Patient 3 : interpolated
	size_t i_min = 90, i_max = 345;
	size_t j_min = 130, j_max = 385;
	size_t k_min = 532, k_max = 711;

	Point3D LiverOrigin = { i_min, j_min, k_min };
	dataType sx = 4.6875;
	
	VoxelSpacing interpolSpacing = { ctSpacing.sx, ctSpacing.sy, ctSpacing.sx };
	LiverOrigin = getRealCoordFromImageCoord3D(LiverOrigin, ctOrigin, interpolSpacing, orientation);
	//const size_t length = 64, width = 64, height = 40; //P2
	const size_t length = 64, width = 64, height = 45; //P3

	dataType** liverShape = new dataType * [height];
	for (k = 0; k < height; k++) {
		liverShape[k] = new dataType[length * width]{ 0 };
	}

	loading_path = inputPath + "inputsForPhDStudentConference/_seg_func_5000_p3.raw";
	load3dArrayRAW<dataType>(liverShape, length, width, height, loading_path.c_str(), false);

	VoxelSpacing LiverSpacing = { sx, sx, sx };
	Image_Data LiverContainer = { height, length, width, liverShape, LiverOrigin, LiverSpacing, orientation };

	// Container for saving
	dataType** imageData = new dataType * [Height];
	for (k = 0; k < Height; k++) {
		imageData[k] = new dataType[dim2D]{ 0 };
	}
	Image_Data savingContainer = { Height, Length, Width, imageData, ctOrigin, ctSpacing, orientation };

	imageInterpolation3D(LiverContainer, savingContainer, NEAREST_NEIGHBOR);

	// Ajust the contour
	// We need to ajust the segmentation because it the direct result
	// GSUBSURF so, we have different pixel value

	for (k = 0; k < Height; k++) {
		for (i = 0; i < dim2D; i++) {
			if (imageData[k][i] < 0.2) {
				imageData[k][i] = 1.0;
			}
			else {
				imageData[k][i] = 0.0;
			}
		}
	}

	storing_path = outputPath + "segment_liver_p3.raw";
	store3dRawData<dataType>(imageData, Length, Width, Height, storing_path.c_str());

	for (k = 0; k < height; k++) {
		delete[] liverShape[k];
	}
	delete[] liverShape;

	for (k = 0; k < Height; k++) {
		delete[] imageData[k];
	}
	delete[] imageData;
	*/

	//======================== Automatic quantitative analysis ==================

	/*
	// Load PET
	Vtk_File_Info* petContainer = (Vtk_File_Info*)malloc(sizeof(Vtk_File_Info));
	petContainer->operation = copyFrom;

	loading_path = inputPath + "vtk/pet/Patient3_pet.vtk";
	readVtkFile(loading_path.c_str(), petContainer);

	int height = petContainer->dimensions[2];
	int length = petContainer->dimensions[0];
	int width = petContainer->dimensions[1];
	int dim2d = length * width;
	std::cout << "PET image dim : " << petContainer->dimensions[0] << " x " << petContainer->dimensions[1] << " x " << petContainer->dimensions[2] << "" << std::endl;

	std::cout << "PET origin : (" << petContainer->origin[0] << ", " << petContainer->origin[1] << ", " << petContainer->origin[2] << ")" << std::endl;
	Point3D petOrigin = { petContainer->origin[0], petContainer->origin[1], petContainer->origin[2] };
	VoxelSpacing petSpacing = { petContainer->spacing[0], petContainer->spacing[1], petContainer->spacing[2] };
	std::cout << "PET spacing : (" << petContainer->spacing[0] << ", " << petContainer->spacing[1] << ", " << petContainer->spacing[2] << ")" << std::endl;

	dataType** liverShape = new dataType * [Height];
	for (k = 0; k < Height; k++) {
		liverShape[k] = new dataType[dim2D]{ 0 };
	}

	// Load liver segment
	loading_path = inputPath + "inputsForPhDStudentConference/segment_liver_p3.raw";
	load3dArrayRAW<dataType>(liverShape, Length, Width, Height, loading_path.c_str(), false);

	//Find Liver centroid
	dataType* centroid = new dataType[3];
	centroidImage(liverShape, centroid, Height, Length, Width, 1.0);

	Point3D center_liver_img_coord = { centroid[0], centroid[1], centroid[2] };
	Point3D center_liver_real_coord = getRealCoordFromImageCoord3D(center_liver_img_coord, ctOrigin, ctSpacing, orientation);
	double radius_cetroid_liver = 20;
	
	////Draw ball inside the liver
	//dataType** ball = new dataType*[Height];
	//for (k = 0; k < Height; k++) {
	//	ball[k] = new dataType[dim2D]{0};
	//}

	//for (k = 0; k < Height; k++) {
	//	for (i = 0; i < Length; i++) {
	//		for (j = 0; j < Width; j++) {
	//			Point3D current_point = { i, j, k };
	//			current_point = getRealCoordFromImageCoord3D(current_point, ctOrigin, ctSpacing, orientation);
	//			double dist = getPoint3DDistance(center_liver_real_coord, current_point);
	//			if (dist <= radius_cetroid_liver) {
	//				ball[k][x_new(i, j, Length)] = 1.0;
	//			}
	//		}
	//	}
	//}
	//storing_path = outputPath + "ball_liver_p2.raw";
	//store3dRawData<dataType>(ball, Length, Width, Height, storing_path.c_str());

	//Get the SUV mean around the liver centroid
	BoundingBox3D box_liver = findBoundingBox3D(center_liver_img_coord, Length, Width, Height, radius_cetroid_liver, 2 * radius_cetroid_liver);
	dataType mean_liver = 1.0, sum_val = 0;
	int count_point = 0;
	for (k = box_liver.k_min; k <= box_liver.k_max; k++) {
		for (i = box_liver.i_min; i <= box_liver.i_max; i++) {
			for (j = box_liver.j_min; j <= box_liver.j_max; j++) {
				Point3D current_point = { i, j, k };
				current_point = getRealCoordFromImageCoord3D(current_point, ctOrigin, ctSpacing, orientation);
				double d = getPoint3DDistance(center_liver_real_coord, current_point);
				if (d <= radius_cetroid_liver) {
					Point3D point_pet = getImageCoordFromRealCoord3D(current_point, petOrigin, petSpacing, orientation);
					dataType val_pet = petContainer->dataPointer[(size_t)point_pet.z][x_new((size_t)point_pet.x, (size_t)point_pet.y, length)];
					count_point++;
					sum_val += val_pet;
				}
			}
		}
	}
	if (count_point != 0) {
		mean_liver = sum_val / (dataType)count_point;
	}
	else {
		mean_liver = 1.0;
	}

	cout << "Mean inside liver is : " << mean_liver << endl;
	
	// Load the path points
	FILE* path_file;
	////loading_path = inputPath + "inputsForPhDStudentConference/path_ordered_p2.csv";
	////loading_path = inputPath + "inputsForPhDStudentConference/translated.csv";
	//loading_path = "C:/Users/Konan Allaly/Documents/Presentation Jasna/path_points_p2.csv";
	//loading_path = "C:/Users/Konan Allaly/Documents/Presentation Jasna/final_path_p2.csv";
	loading_path = "C:/Users/Konan Allaly/Documents/Presentation Jasna/final_translated_p2.csv";
	if (fopen_s(&path_file, loading_path.c_str(), "r") != 0) {
		printf("Enable to open");
		return false;
	}

	dataType x = 0, y = 0, z = 0;
	vector<Point3D> path_points;
	while (feof(path_file) == 0) {
		fscanf_s(path_file, "%f", &x);
		fscanf_s(path_file, ",");
		fscanf_s(path_file, "%f", &y);
		fscanf_s(path_file, ",");
		fscanf_s(path_file, "%f", &z);
		fscanf_s(path_file, "\n");

		//Saved without the spacing
		//TODO: Export the path considering the spacing or save in real world coordinates 
		//x *= ctSpacing.sx;
		//y *= ctSpacing.sy;
		//z *= ctSpacing.sz;
		Point3D current_point = { x, y, z };

		//update point dimension
		current_point = getRealCoordFromImageCoord3D(current_point, ctOrigin, intSpacing, orientation);
		
		path_points.push_back(current_point);
	}
	fclose(path_file);

	//File for the ratio max
	double search_radius = 25;
	//string saving_csv = outputPath + "ratio_max_5_before.csv";
	string saving_csv = outputPath + "ratio_max_25_after.csv";
	FILE* file;
	if (fopen_s(&file, saving_csv.c_str(), "w") != 0) {
		printf("Enable to open");
		return false;
	}
	fprintf(file, "x,y\n");

	//File for the ratio mean
	//saving_csv = outputPath + "ratio_mean_5_before.csv";
	saving_csv = outputPath + "ratio_mean_25_after.csv";
	FILE* rmean;
	if (fopen_s(&rmean, saving_csv.c_str(), "w") != 0) {
		printf("Enable to open");
		return false;
	}
	fprintf(rmean, "x,y\n");

	dataType** shape_aorta_ct = new dataType * [Height];
	for (k = 0; k < Height; k++) {
		shape_aorta_ct[k] = new dataType[dim2D]{ 0 };
	}

	//Aorta Shape pet
	dataType** shape_aorta_pet = new dataType * [height];
	for (k = 0; k < height; k++) {
		shape_aorta_pet[k] = new dataType[dim2d]{ 0 };
	}

	BoundingBox3D box_path_point;
	size_t n = 0;
	dataType maxSuv = 0.0, sumSuv = 0.0, meanSuv = 0.0;
	dataType ratio_max_aorta_mean_liver = 0.0, ratio_mean_aorta_mean_liver = 0.0;
	vector<dataType> ratio_list;
	
	//vector<Point3D> points_max_list;

	Point3D point_ct;
	size_t i_pet, j_pet, k_pet, xd_pet;
	for (n = 0; n < path_points.size(); n++) {
		point_ct = getImageCoordFromRealCoord3D(path_points[n], ctOrigin, ctSpacing, orientation);
		box_path_point = findBoundingBox3D(point_ct, Length, Width, Height, search_radius, 2 * ctSpacing.sx);
		point_ct = getRealCoordFromImageCoord3D(point_ct, ctOrigin, ctSpacing, orientation);
		maxSuv = 0.0;
		sumSuv = 0.0;
		ratio_max_aorta_mean_liver = 0.0;
		count_point = 0;
		Point3D point_max = { 0.0, 0.0, 0.0 };
		for (k = box_path_point.k_min; k <= box_path_point.k_max; k++) {
			for (i = box_path_point.i_min; i <= box_path_point.i_max; i++) {
				for (j = box_path_point.j_min; j <= box_path_point.j_max; j++) {
					Point3D current_point = { i, j, k };
					current_point = getRealCoordFromImageCoord3D(current_point, ctOrigin, ctSpacing, orientation);
					double dist = getPoint3DDistance(current_point, point_ct);
					if (dist <= search_radius) {
						shape_aorta_ct[k][x_new(i, j, Length)] = 1.0; // save an visualize the shape in CT
						Point3D point_pet = getImageCoordFromRealCoord3D(current_point, petOrigin, petSpacing, orientation);
						i_pet = (size_t)point_pet.x;
						j_pet = (size_t)point_pet.y;
						k_pet = (size_t)point_pet.z;
						xd_pet = x_new(i_pet, j_pet, length);
						dataType val_pet = petContainer->dataPointer[k_pet][xd_pet];
						shape_aorta_pet[k_pet][xd_pet] = 1.0; // save an visualize the shape in PET
						sumSuv += val_pet;
						count_point++;
						if (maxSuv < val_pet) {
							maxSuv = val_pet;
						}
					}
				}
			}
		}
		meanSuv = sumSuv / (dataType)count_point;
		if (mean_liver != 0) {
			ratio_max_aorta_mean_liver = maxSuv / mean_liver;
			ratio_mean_aorta_mean_liver = meanSuv / mean_liver;
		}
		else {
			ratio_max_aorta_mean_liver = maxSuv;
			ratio_mean_aorta_mean_liver = meanSuv;
		}

		//ratio_list.push_back(ratio_aorta_liver);
		fprintf(file, "%d,%f\n", n, ratio_max_aorta_mean_liver);
		fprintf(rmean, "%d,%f\n", n, ratio_mean_aorta_mean_liver);

		//fprintf(rmean, "%d,%f\n", n, meanSuv);
		//dataType vis = 1.0;
		//fprintf(visual, "%d,%f\n", n, vis);
	}

	storing_path = outputPath + "shape_aorta_25_ct_p3.raw";
	store3dRawData<dataType>(shape_aorta_ct, Length, Width, Height, storing_path.c_str());

	storing_path = outputPath + "shape_aorta_25_pet_p3.raw";
	store3dRawData<dataType>(shape_aorta_pet, length, width, height, storing_path.c_str());

	fclose(file);
	fclose(rmean);
	//fclose(rpeak);

	////Find the max in the list
	//dataType max_ratio_list = 0;
	//for (n = 0; n < ratio_list.size(); n++) {
	//	if (max_ratio_list < ratio_list[n]) {
	//		max_ratio_list = ratio_list[n];
	//	}
	//}
	////Find points with max ratio and save them
	//saving_csv = outputPath + "ratio_max.csv";
	//FILE* file_ratio;
	//if (fopen_s(&file_ratio, saving_csv.c_str(), "w") != 0) {
	//	printf("Enable to open");
	//	return false;
	//}
	//fprintf(file_ratio, "x,y,z\n");
	//for (n = 0; n < ratio_list.size(); n++) {
	//	if (ratio_list[n] >= max_ratio_list - 0.15 && ratio_list[n] <= max_ratio_list + 0.15) {
	//		fprintf(file_ratio, "%f,%f,%f\n", path_points[n].x, path_points[n].y, path_points[n].z);
	//	}
	//}
	//fclose(file_ratio);
	
	for (k = 0; k < height; k++) {
		delete[] shape_aorta_pet[k];
	}
	delete[] shape_aorta_pet;

	delete[] centroid;
	for (k = 0; k < Height; k++) {
		delete[] liverShape[k];
		//delete[] ball[k];
		delete[] shape_aorta_ct[k];
	}
	delete[] liverShape;
	//delete[] ball;
	delete[] shape_aorta_ct;

	free(petContainer);
	*/

	//======================== Filtering =======================

	/*
	Filter_Parameters filter_parameters; filter_parameters.edge_detector_coefficient = 1;
	filter_parameters.h = 1.0; filter_parameters.timeStepSize = 1.2; filter_parameters.eps2 = 1e-6;
	filter_parameters.omega_c = 1.5; filter_parameters.tolerance = 1e-2; filter_parameters.maxNumberOfSolverIteration = 100;
	filter_parameters.timeStepsNum = 1; filter_parameters.coef = 1e-6; 
	filterImage(INTERPOL, filter_parameters, GEODESIC_MEAN_CURVATURE_FILTER);
	
	storing_path = outputPath + "filteredGMC.raw";
	manageRAWFile3D<dataType>(interpolated, Length, Width, height, storing_path.c_str(), STORE_DATA, false);

	for (k = 0; k < height; k++) {
		delete[] interpolated[k];
	}
	delete[] interpolated;
	*/

	//======================== Segment the Liver ===============

	/*
	dataType** maskThreshold = new dataType * [Height];
	for (k = 0; k < Height; k++) {
		maskThreshold[k] = new dataType[dim2D] {0};
	}
	if (maskThreshold == NULL)
		return false;

	// Copy
	for (k = 0; k < Height; k++) {
		for (i = 0; i < dim2D; i++) {
			maskThreshold[k][i] = ctContainer->dataPointer[k][i];
		}
	}
	
	thresholding3dFunctionN(maskThreshold, Length, Width, Height, thres_min, thres_max, 0.0, 1.0);

	size_t n = 6;
	for (i = 0; i < n; i++) {
		//erosion3dHeighteenNeigbours(maskThreshold, Length, Width, Height, 1.0, 0.0);
		erosion3D(maskThreshold, Length, Width, Height, 1.0, 0.0);
	}
	//storing_path = outputPath + "threshold_p3.raw";
	//manageRAWFile3D<dataType>(maskThreshold, Length, Width, Height, storing_path.c_str(), STORE_DATA, false);

	int** labelArray = new int* [Height];
	bool** status = new bool* [Height];
	for (k = 0; k < Height; k++) {
		labelArray[k] = new int[dim2D] {0};
		status[k] = new bool[dim2D];
	}
	if (labelArray == NULL || status == NULL)
		return false;

	//Initialization
	for (k = 0; k < Height; k++) {
		for (i = 0; i < dim2D; i++) {
			status[k][i] = false;
		}
	}

	int numberOfRegionCells = countNumberOfRegionsCells(maskThreshold, Length, Width, Height, 1.0);

	labelling3D(maskThreshold, labelArray, status, Length, Width, Height, 1.0);

	//Counting
	int* countingArray = new int[numberOfRegionCells] {0};
	if (countingArray == NULL)
		return false;

	//Get regions sizes
	for (k = 0; k < Height; k++) {
		for (i = 0; i < dim2D; i++) {
			if (labelArray[k][i] > 0) {
				countingArray[labelArray[k][i]]++;
			}
		}
	}

	//Number of regions
	int numberOfRegions = 0;
	for (i = 0; i < numberOfRegionCells; i++) {
		if (countingArray[i] > 0) {
			numberOfRegions++;
		}
	}
	cout << numberOfRegions << " regions found" << endl;

	//Find the biggest region
	size_t regionSize = 0;
	for (k = 0; k < Height; k++) {
		for (i = 0; i < dim2D; i++) {
			if (countingArray[labelArray[k][i]] > regionSize) {
				regionSize = countingArray[labelArray[k][i]];
			}
		}
	}

	//Keep and save the biggest region
	for (k = 0; k < Height; k++) {
		for (i = 0; i < dim2D; i++) {
			if (countingArray[labelArray[k][i]] == regionSize) {
				maskThreshold[k][i] = 1.0;
			}
			else {
				maskThreshold[k][i] = 0.0;
			}
		}
	}

	//dilatation3dHeighteenNeigbours(maskThreshold, Length, Width, Height, 1.0, 0.0);
	//dilatation3dHeighteenNeigbours(maskThreshold, Length, Width, Height, 1.0, 0.0);

	//dataType* centroid = (dataType*)malloc(3 * sizeof(dataType));
	//centroidImage(maskThreshold, centroid, Height, Length, Width, 0.0);
	//cout << "Liver centroid : (" << centroid[0] << "," << centroid[1] << "," << centroid[2] << ")" << endl;
	//free(centroid);

	storing_path = outputPath + "liver_thres_p3.raw";
	store3dRawData<dataType>(maskThreshold, Length, Width, Height, storing_path.c_str());

	for (k = 0; k < Height; k++) {
		delete[] maskThreshold[k];
		delete[] labelArray[k];
		delete[] status[k];
	}
	delete[] maskThreshold;
	delete[] labelArray;
	delete[] status;
	*/

	//======================== Interpolate input and segment ======================
		
	/*
	//We load the rough segmentation of the liver, obtained by thresholding
	//connected component labeling and erosion. We then interpolate the original
	//image and the segment in order to have the same spacing in x, y and z.
	//Information related to the dimension, sapacing and origin are avalaible
	//by loading the .vtk file.
	
	dataType** imageData = new dataType * [Height];
	dataType** segment = new dataType * [Height];
	for (k = 0; k < Height; k++) {
		imageData[k] = new dataType[dim2D]{ 0 };
		segment[k] = new dataType[dim2D]{ 0 };
	}

	for (k = 0; k < Height; k++) {
		for (i = 0; i < dim2D; i++) {
			imageData[k][i] = ctContainer->dataPointer[k][i];
		}
	}

	loading_path = inputPath + "liver/liver_p6.raw";
	load3dArrayRAW<dataType>(segment, Length, Width, Height, loading_path.c_str(), false);
	
	size_t height = (size_t)((ctSpacing.sz / ctSpacing.sx) * Height);
	cout << "interpol height : " << height << endl;

	dataType** interpolate = new dataType * [height];
	dataType** interpolateSegment = new dataType * [height];
	for (k = 0; k < height; k++) {
		interpolate[k] = new dataType[dim2D]{ 0 };
		interpolateSegment[k] = new dataType[dim2D]{ 0 };
	}

	Image_Data CT; 
	CT.imageDataPtr = imageData;
	CT.height = Height; 
	CT.length = Length; 
	CT.width = Width;
	CT.orientation = orientation; 
	CT.origin = ctOrigin; 
	CT.spacing.sx = ctSpacing.sx;
	CT.spacing.sy = ctSpacing.sy;
	CT.spacing.sz = ctSpacing.sz;

	Image_Data interpolated;
	interpolated.imageDataPtr = interpolate;
	interpolated.height = height; 
	interpolated.length = Length; 
	interpolated.width = Width;
	interpolated.orientation = orientation; 
	interpolated.origin = ctOrigin;
	interpolated.spacing.sx = ctSpacing.sx;
	interpolated.spacing.sy = ctSpacing.sy;
	interpolated.spacing.sz = ctSpacing.sx;
	
	//Interpolate the input image
	imageInterpolation3D(CT, interpolated, NEAREST_NEIGHBOR);

	storing_path = outputPath + "interpolate_p6.raw";
	store3dRawData<dataType>(interpolate, Length, Width, height, storing_path.c_str());

	// Interpolate the segment
	CT.imageDataPtr = segment;
	interpolated.imageDataPtr = interpolateSegment;

	imageInterpolation3D(CT, interpolated, NEAREST_NEIGHBOR);

	storing_path = outputPath + "interpolateSegment_p6.raw";
	store3dRawData<dataType>(interpolateSegment, Length, Width, height, storing_path.c_str());

	for (k = 0; k < Height; k++) {
		delete[] imageData[k];
		delete[] segment[k];
	}
	delete[] imageData;
	delete[] segment;

	for (k = 0; k < height; k++) {
		delete[] interpolate[k];
		delete[] interpolateSegment[k];
	}
	delete[] interpolate;
	delete[] interpolateSegment;
	*/
	
	//======================= Crop interpolated image data =======================

	/*
	////Patient 1 : interpolated
	//size_t i_min = 90, i_max = 359;
	//size_t j_min = 90, j_max = 359;
	//size_t k_min = 307, k_max = 496;
	
	////Patient 2 : interpolated
	//size_t i_min = 120, i_max = 375;
	//size_t j_min = 160, j_max = 415;
	//size_t k_min = 335, k_max = 494;

	//Patient 3 : interpolated
	size_t i_min = 90, i_max = 345;
	size_t j_min = 130, j_max = 385;
	size_t k_min = 532, k_max = 711;

	////Patient 4 : interpoolated
	//size_t i_min = 120, i_max = 375;
	//size_t j_min = 150, j_max = 405;
	//size_t k_min = 311, k_max = 518;

	////Patient 6 : interpolated
	//size_t i_min = 120, i_max = 375;
	//size_t j_min = 180, j_max = 435;
	//size_t k_min = 703, k_max = 902;

	////Patient 3 : input
	//size_t i_min = 90, i_max = 345;
	//size_t j_min = 130, j_max = 385;
	//size_t k_min = 250, k_max = 333;

	const size_t length_crop = i_max - i_min + 1;
	const size_t width_crop = j_max - j_min + 1;
	const size_t height_crop = k_max - k_min + 1;
	const size_t dim2D_crop = length_crop * width_crop;
	std::cout << "cropped dim : " << length_crop << "x" << width_crop << "x" << height_crop << std::endl;

	dataType** imageCrop = new dataType * [height_crop];
	dataType** segmentCrop = new dataType * [height_crop];
	for (k = 0; k < height_crop; k++) {
		imageCrop[k] = new dataType[dim2D_crop]{ 0 };
		segmentCrop[k] = new dataType[dim2D_crop]{ 0 };
	}

	//const dataType height_interp = 866; // patient2
	const dataType height_interp = 1083; // patient3
	dataType** imageDataPtr = new dataType * [height_interp];
	dataType** segmentInterpPtr = new dataType * [height_interp];
	for (k = 0; k < height_interp; k++) {
		imageDataPtr[k] = new dataType[dim2D]{ 0 };
		segmentInterpPtr[k] = new dataType[dim2D]{ 0 };
	}

	dataType** segmentPtr = new dataType * [Height];
	for (k = 0; k < Height; k++) {
		segmentPtr[k] = new dataType[dim2D]{ 0 };
	}

	VoxelSpacing interpolate_spacing = { ctSpacing.sx, ctSpacing.sy, ctSpacing.sx };
	Image_Data Interpolation = { 
		height_interp,
		Length,
		Width,
		segmentInterpPtr,
		ctOrigin,
		interpolate_spacing,
		orientation 
	};

	Image_Data inputStructure = {
	Height,
	Length,
	Width,
	segmentPtr,
	ctOrigin,
	ctSpacing,
	orientation
	};

	loading_path = inputPath + "raw/interpolated/patient3/filtered_p3.raw";
	load3dArrayRAW<dataType>(imageDataPtr, Length, Width, height_interp, loading_path.c_str(), false);

	loading_path = inputPath + "inputsForPhDStudentConference/liver_seg_p3.raw";
	load3dArrayRAW<dataType>(segmentPtr, Length, Width, Height, loading_path.c_str(), false);

	imageInterpolation3D(inputStructure, Interpolation, NEAREST_NEIGHBOR);

	//cropping
	size_t in = 0, jn = 0, kn = 0, xdn = 0;
	for (kn = 0, k = k_min; kn < height_crop; kn++, k++) {
		for (in = 0, i = i_min; in < length_crop; in++, i++) {
			for (jn = 0, j = j_min; jn < width_crop; jn++, j++) {
				xd = x_new(i, j, Length);
				xdn = x_new(in, jn, length_crop);
				imageCrop[kn][xdn] = imageDataPtr[k][xd];
				segmentCrop[kn][xdn] = segmentInterpPtr[k][xd];
			}
		}
	}

	Point3D newOrigin = { i_min, j_min, k_min };
	newOrigin = getRealCoordFromImageCoord3D(newOrigin, ctOrigin, interpolate_spacing, orientation);
	std::cout << "New origin : (" << newOrigin.x << "," << newOrigin.y << "," << newOrigin.z << ")" << std::endl;

	storing_path = outputPath + "cropImage_p3.raw";
	store3dRawData<dataType>(imageCrop, length_crop, width_crop, height_crop, storing_path.c_str());

	storing_path = outputPath + "cropSegment_p3.raw";
	store3dRawData<dataType>(segmentCrop, length_crop, width_crop, height_crop, storing_path.c_str());

	for (k = 0; k < height_crop; k++) {
		delete[] imageCrop[k];
		delete[] segmentCrop[k];
	}
	delete[] imageCrop;
	delete[] segmentCrop;

	for (k = 0; k < Height; k++) {
		delete[] segmentPtr[k];
	}
	delete[] segmentPtr;

	for (k = 0; k < height_interp; k++) {
		delete[] imageDataPtr[k];
		delete[] segmentInterpPtr[k];
	}
	delete[] imageDataPtr;
	delete[] segmentInterpPtr;
	*/

	//======================== Refinement by generalized subjective surface =======
	
	/*
	// We work with the cropped data to speed up the computation
	
	////Patient 1 : interpolated
	//size_t i_min = 90, i_max = 359;
	//size_t j_min = 90, j_max = 359;
	//size_t k_min = 307, k_max = 496;
	
	//Patient 2 : interpolated
	size_t i_min = 120, i_max = 375;
	size_t j_min = 160, j_max = 415;
	size_t k_min = 335, k_max = 494;

	////Patient 3 : interpolated
	//size_t i_min = 90, i_max = 345;
	//size_t j_min = 130, j_max = 385;
	//size_t k_min = 532, k_max = 711;

	////Patient 4 : interpolated
	//size_t i_min = 120, i_max = 375;
	//size_t j_min = 150, j_max = 405;
	//size_t k_min = 311, k_max = 518;

	////Patient 6: interpolated
	//size_t i_min = 120, i_max = 375;
	//size_t j_min = 180, j_max = 435;
	//size_t k_min = 703, k_max = 902;

	const size_t length = i_max - i_min + 1;
	const size_t width = j_max - j_min + 1;
	const size_t height = k_max - k_min + 1;
	const size_t dim2d = length * width;

	dataType** imageData = new dataType * [height];
	dataType** initialSegment = new dataType * [height];
	for (k = 0; k < height; k++) {
		imageData[k] = new dataType[dim2d]{0};
		initialSegment[k] = new dataType[dim2d]{0};
	}

	loading_path = inputPath + "inputsForPhDStudentConference/cropImage_p2.raw";
	load3dArrayRAW<dataType>(imageData, length, width, height, loading_path.c_str(), false);

	loading_path = inputPath + "inputsForPhDStudentConference/cropSegment_p2.raw";
	load3dArrayRAW<dataType>(initialSegment, length, width, height, loading_path.c_str(), false);

	VoxelSpacing ctInt = { ctContainer->spacing[0], ctContainer->spacing[1], ctContainer->spacing[0] };
	Point3D newOrigin = { i_min, j_min, k_min };
	newOrigin = getRealCoordFromImageCoord3D(newOrigin, ctOrigin, ctInt, orientation);
	std::cout << "New origin : (" << newOrigin.x << "," << newOrigin.y << "," << newOrigin.z << ")" << std::endl;

	Image_Data inputImage{
		height,
		length,
		width,
		imageData,
		newOrigin,
		ctInt,
		orientation
	};
	Image_Data inputSegment{
		height,
		length,
		width,
		initialSegment,
		newOrigin,
		ctInt,
		orientation
	};
	
	Point3D* center_seg = new Point3D[1];
	center_seg->x = 0.0, center_seg->y = 0.0; center_seg->z = 0.0;

	//Down sampling
	dataType down_factor = 4.0;
	const size_t h_down = (size_t)(height / down_factor);
	const size_t l_down = (size_t)(length / down_factor);
	const size_t w_down = (size_t)(width / down_factor);
	std::cout << "Downsampled image dim : " << l_down << " x " << w_down << " x " << h_down << "" << std::endl;
	dataType** imageDown = new dataType * [h_down];
	dataType** initialDown = new dataType * [h_down];
	for (k = 0; k < h_down; k++) {
		imageDown[k] = new dataType[l_down * w_down]{ 0 };
		initialDown[k] = new dataType[l_down * w_down]{ 0 };
	}

	VoxelSpacing downSampleSapcing = { ctInt.sx * down_factor, ctInt.sy * down_factor, ctInt.sz * down_factor };
	
	Image_Data downSampledImage{
	h_down,
	l_down,
	w_down,
	imageDown,
	newOrigin,
	downSampleSapcing,
	orientation
	};
	Image_Data downSampledSegment{
	h_down,
	l_down,
	w_down,
	initialDown,
	newOrigin,
	downSampleSapcing,
	orientation
	};
	std::cout << "Downsampled image spacing : (" << downSampleSapcing.sx << ", " << downSampleSapcing.sy << ", " << downSampleSapcing.sz << ")" << std::endl;

	imageInterpolation3D(inputImage, downSampledImage, NEAREST_NEIGHBOR);
	imageInterpolation3D(inputSegment, downSampledSegment, NEAREST_NEIGHBOR);

	//storing_path = outputPath + "downSeg.raw";
	//store3dRawData<dataType>(initialDown, l_down, w_down, h_down, storing_path.c_str());
	//storing_path = outputPath + "downImg.raw";
	//store3dRawData<dataType>(imageDown, l_down, w_down, h_down, storing_path.c_str());

	//Smoothing by heat equation, we don't need all the parameters
	Filter_Parameters smoothing_parameters{
		0.2, //tau
		1.0, //h
		0.0, //sigma
		0.0, //edge detector coeficient
		1.5, //omega
		1e-3,//tolerance
		0.0, // epsilon Evan-Spruck regularization
		0.0, // coef
		(size_t)1, //p
		(size_t)1, //time step number
		(size_t)100 //max number of solver iterations
	};

	Segmentation_Parameters segmentation_parameters{
		(size_t)30,  // Maximum number of Gauss - Seidel iterations
		2000,         // K : edge detector coef
		1e-6,         //epsilon : Evan-Spruck regularization
		(size_t)5000,   //number of current time step
		(size_t)5000,   //max number of time step
		(size_t)100,    //frequence au saving
		1e-4,         //segmentation tolerance
		down_factor,  //tau
		1.0,          //h
		1.5,          //omega
		1e-3,         //gauss_seidel tolerance
		0.01,         //convection coeff
		1.0           //diffusion coeff
	};
	
	unsigned char segmentPath[] = "C:/Users/Konan Allaly/Documents/Tests/output/segmentation/patient2/";
	//subsurfSegmentation(imageToBeSegmented, initialSegment, seg_params, smooth_params, center_seg, 1, segmentPath);
	generalizedSubsurfSegmentation(downSampledImage, initialDown, segmentation_parameters, smoothing_parameters, center_seg, 1, segmentPath);

	for (k = 0; k < h_down; k++) {
		delete[] imageDown[k];
		delete[] initialDown[k];
	}
	delete[] imageDown;
	delete[] initialDown;

	delete[] center_seg;
	for (k = 0; k < height; k++) {
		delete[] imageData[k];
		delete[] initialSegment[k];
	}
	delete[] imageData;
	delete[] initialSegment;
	*/

	//======================== Path finding Multiple seeds =======================================
	
	/*
	//const size_t height = 866; // P2
	//const size_t height = 844; // P4
	const size_t height = 1351; // P6
	//const size_t height = 1083; // P3
	//const size_t height = 932; // P7

	dataType** potential = new dataType * [height];
	dataType** actionField = new dataType * [height];
	dataType** liverData = new dataType * [height];
	dataType** imageData = new dataType * [height];
	for (k = 0; k < height; k++) {
		potential[k] = new dataType[dim2D]{ 0 };
		actionField[k] = new dataType[dim2D]{ 0 };
		liverData[k] = new dataType[dim2D]{ 0 };
		imageData[k] = new dataType[dim2D]{ 0 };
	}
	if (potential == NULL || actionField == NULL || liverData == NULL)
		return false;
	
	//Load segmented liver
	loading_path = inputPath + "raw/interpolated/filtered_p6.raw";
	load3dArrayRAW<dataType>(imageData, Length, Width, height, loading_path.c_str(), false);

	double radius = 3.0;
	dataType h = 1.0;
	Potential_Parameters parameters{
		0.005,//K
		0.01,//eps
		h,
		radius
	};

	Point3D* seeds = new Point3D[4];

	////Patient 2
	//Point3D seed1 = { 260, 257, 308 };
	//Point3D seed2 = { 295, 317, 540 };
	//Point3D seed3 = { 283, 277, 587 };
	//Point3D seed4 = { 251, 254, 540 };

	////Patient 4
	//Point3D seed1 = { 268, 232, 291 };
	//Point3D seed2 = { 289, 297, 591 };
	//Point3D seed3 = { 256, 224, 591 };

	//Patient 6
	Point3D seed1 = { 266, 242, 715 };
	Point3D seed2 = { 283, 283, 993 };
	Point3D seed3 = { 283, 234, 1034 };
	Point3D seed4 = { 233, 213, 993 };

	seeds[0] = seed1;
	seeds[1] = seed2;
	seeds[2] = seed3;
	seeds[3] = seed4;

	string saving_real = outputPath + "seed_p6.csv";
	FILE* file_seed;
	if (fopen_s(&file_seed, saving_real.c_str(), "w") != 0) {
		printf("Enable to open");
		return false;
	}
	fprintf(file_seed, "%f,%f,%f\n", seed1.x, seed1.y, seed1.z);
	fprintf(file_seed, "%f,%f,%f\n", seed2.x, seed2.y, seed2.z);
	fprintf(file_seed, "%f,%f,%f\n", seed3.x, seed3.y, seed3.z);
	fprintf(file_seed, "%f,%f,%f\n", seed4.x, seed4.y, seed4.z);
	fclose(file_seed);

	VoxelSpacing intSpacing = { ctSpacing.sx, ctSpacing.sy, ctSpacing.sx };
	Image_Data CT = { height, Length, Width, imageData, ctOrigin, intSpacing, orientation };

	////P7
	//Point3D seed1 = { 250, 299, 400 };
	//Point3D seed2 = { 277, 355, 672 };
	//Point3D seed3 = { 276, 320, 740 };
	//Point3D seed4 = { 262, 288, 672 };

	compute3DPotential(CT, potential, seed1, radius, parameters);

	storing_path = outputPath + "potential_p6.raw";
	//load3dArrayRAW<dataType>(potential, Length, Width, height, storing_path.c_str(), false);
	store3dRawData<dataType>(potential, Length, Width, height, storing_path.c_str());


	string saving_seed = outputPath + "path_points_p6.csv";
	FILE* file_real;
	if (fopen_s(&file_real, saving_seed.c_str(), "w") != 0) {
		printf("Enable to open");
		return false;
	}

	Point3D* seedPoints = new Point3D[2];
	seedPoints[0] = seed1;
	seedPoints[1] = seed2;

	//double maxLength = 100.0;
	//findPathTwoSteps(CT, seedPoints, parameters);
	//fastMarching3D_N(actionField, potential, Length, Width, height, seed1);
	partialFrontPropagation(actionField, potential, Length, Width, height, seedPoints);
	//partialPropagation(actionField, potential, Length, Width, height, seedPoints, maxLength);
	//std::cout << "Found point : (" << seedPoints[1].x << "," << seedPoints[1].y << "," << seedPoints[1].z << ")" << std::endl;

	//storing_path = outputPath + "action.raw";
	//store3dRawData<dataType>(actionField, Length, Width, height, storing_path.c_str());
	
	//double found_lenght = getPoint3DDistance(seedPoints[0], seedPoints[1]);
	//std::cout << "Found distance : " << found_lenght << std::endl;

	vector<Point3D> path_points;
	shortestPath3D(actionField, Length, Width, height, intSpacing, seedPoints, path_points);
	
	int n = 0;
	for (n = path_points.size() - 1; n > -1; n--) {
		path_points[n] = getRealCoordFromImageCoord3D(path_points[n], CT.origin, intSpacing, orientation);
		fprintf(file_real, "%f,%f,%f\n", path_points[n].x, path_points[n].y, path_points[n].z);
	}
	while (path_points.size() > 0) {
		path_points.pop_back();
	}

	seedPoints[0] = seed2;
	seedPoints[1] = seed3;
	partialFrontPropagation(actionField, potential, Length, Width, height, seedPoints);
	shortestPath3D(actionField, Length, Width, height, intSpacing, seedPoints, path_points);

	for (n = path_points.size() - 1; n > -1; n--) {
		path_points[n] = getRealCoordFromImageCoord3D(path_points[n], CT.origin, intSpacing, orientation);
		fprintf(file_real, "%f,%f,%f\n", path_points[n].x, path_points[n].y, path_points[n].z);
	}
	while (path_points.size() > 0) {
		path_points.pop_back();
	}

	seedPoints[0] = seed3;
	seedPoints[1] = seed4;
	partialFrontPropagation(actionField, potential, Length, Width, height, seedPoints);
	shortestPath3D(actionField, Length, Width, height, intSpacing, seedPoints, path_points);

	for (n = path_points.size() - 1; n > -1; n--) {
		path_points[n] = getRealCoordFromImageCoord3D(path_points[n], CT.origin, intSpacing, orientation);
		fprintf(file_real, "%f,%f,%f\n", path_points[n].x, path_points[n].y, path_points[n].z);
	}
	while (path_points.size() > 0) {
		path_points.pop_back();
	}

	fclose(file_real);
	
	delete[] seedPoints;
	for (k = 0; k < height; k++) {
		delete[] potential[k];
		delete[] actionField[k];
		delete[] liverData[k];
		delete[] imageData[k];
	}
	delete[] potential;
	delete[] actionField;
	delete[] liverData;
	delete[] imageData;
	*/

	//======================== 2D Hough transform on Slice ===========================
	
	/*
	const size_t Height = 406, Length = 512, Width = 512;
	const size_t dim2D = Length * Width;

	dataType** imageData = new dataType*[Height];
	for (k = 0; k < Height; k++) {
		imageData[k] = new dataType[dim2D]{ 0 };
	}
	
	//loading_path = inputPath + "raw/filtered/filteredGMC_p2.raw";
	loading_path = inputPath + "raw/ct/patient2_ct.raw";
	load3dArrayRAW(imageData, Length, Width, Height, loading_path.c_str(), false);
	
	dataType* imageSlice = new dataType[dim2D]{ 0 };
	bool* statusPixel = new bool[dim2D]{ false };

	size_t k_liver = 176; //--->P2
	for (i = 0; i < dim2D; i++) {
		imageSlice[i] = imageData[k_liver][i];
	}

	rescaleNewRange2D(imageSlice, Length, Width, 0.0, 1.0);
	
	storing_path = outputPath + "rescaled.raw";
	store2dRawData<dataType>(imageSlice, Length, Width, storing_path.c_str());

	//PixelSpacing spacing = { 1.171875, 1.171875 };
	PixelSpacing spacing = { 1.0, 1.0 };
	Image_Data2D IMAGE;
	IMAGE.imageDataPtr = imageSlice;
	IMAGE.height = Length; IMAGE.width = Width;
	
	HoughParameters h_parameters = 
	{ 
		7.5, //minimal radius
		19.5, //maximal radius
		0.5, //epsilon to search points belonging the circle
		3.0, //offset to find the bounding box
		1000.0, //edge detector coefficient K 
		1.0, //h, space discretization for gradient computation
		0.07, //threshold edge detector
		0.5, //radius step
		spacing //pixel size
	};

	//filtering parameters
	Filter_Parameters filter_parameters;
	filter_parameters.h = 1.0; filter_parameters.timeStepSize = 0.3; filter_parameters.eps2 = 1e-6;
	filter_parameters.omega_c = 1.5; filter_parameters.tolerance = 1e-3; filter_parameters.maxNumberOfSolverIteration = 1000;
	filter_parameters.timeStepsNum = 5; filter_parameters.coef = 1e-6;

	//Slice filtering
	heatImplicit2dScheme(IMAGE, filter_parameters);

	storing_path = outputPath + "filtered.raw";
	store2dRawData<dataType>(imageSlice, Length, Width, storing_path.c_str());

	dataType* gradientX = new dataType[dim2D]{ 0 };
	dataType* gradientY = new dataType[dim2D]{ 0 };
	//dataType* edgeDetector = new dataType[dim2D]{ 0 };
	dataType* threshold = new dataType[dim2D]{ 0 };
	computeImageGradient(imageSlice, gradientX, gradientY, Length, Width, 1.0);
	dataType norm_of_gradient = 0.0, edge_value;
	for (i = 0; i < dim2D; i++) {
		norm_of_gradient = sqrt(gradientX[i] * gradientX[i] + gradientY[i] * gradientY[i]);
		//edgeDetector[i] = gradientFunction(norm_of_gradient, 1000);
		edge_value = gradientFunction(norm_of_gradient, 1000);
		if (edge_value <= 0.1) {
			threshold[i] = 1.0;
		}
	}

	storing_path = outputPath + "threshold.raw";
	store2dRawData<dataType>(threshold, Length, Width, storing_path.c_str());

	Image_Data2D IMAGE_HOUGH
	{
		Length,
		Width,
		threshold
	};

	Point2D seed = {263, 264};
	localCircleDetection(IMAGE_HOUGH, seed, h_parameters);
	//circleDetection(IMAGE_HOUGH, h_parameters);

	//storing_path = outputPath + "smoothed.raw";
	//store2dRawData<dataType>(imageSlice, Length, Width, storing_path.c_str());
	
	//Point2D seed = { 248, 298 }; //---> P2 : automatic detection
	//Point2D seed = { 263, 257 }; //---> P2 : abdominal aorta
	//Point2D seed = { 263, 296 }; //---> P2
	//Point2D seed = { 263, 296 }; //---> P2
	//Point2D seed = { 262, 269 }; //---> P4
	//Point2D seed = { 257, 244 }; //---> P2, liver
	//Point2D found_point = { 0.0, 0.0 };
	//Point2D found1 = { 292,314 }, found2 = { 253,255 }; // p2
	//imageSlice[x_new(292, 314, Length)] = 0.0;
	//imageSlice[x_new(253,255, Length)] = 0.0;

	////loading_path = inputPath + "threshold.raw";
	//loading_path = inputPath + "loaded.raw";
	//load2dArrayRAW<dataType>(imageSlice, Length, Width, loading_path.c_str(), false);
	
	//houghSpace[x_new(292, 314, Length)] = 1.0;
	//houghSpace[x_new(253,255, Length)] = 1.0;
	//houghSpace[x_new(264, 263, Length)] = 1.0;//p2,liver
	//imageSlice[x_new(264, 263, Length)] = 0.0;//p2,liver
	//imageSlice[x_new(292, 314, Length)] = 0.0;
	//imageSlice[x_new(253,255, Length)] = 0.0;
	//storing_path = outputPath + "input_plus_centers.raw";
	//store2dRawData<dataType>(imageSlice, Length, Width, storing_path.c_str());

	//string saving_csv = outputPath + "file_.csv";
	//FILE* file;
	//if (fopen_s(&file, saving_csv.c_str(), "w") != 0) {
	//	printf("Enable to open");
	//	return false;
	//}
	
	//found_point = localHoughTransform(seed, imageSlice, houghSpace, foundCircle, Length, Width, parameters, storing_path.c_str());
	//cout << "found center : (" << found_point.x << "," << found_point.y << ")" << endl;
	//fclose(file);

	for (k = 0; k < Height; k++) {
		delete[] imageData[k];
	}
	delete[] imageData;

	delete[] imageSlice;
	delete[] statusPixel;
	delete[] gradientX;
	delete[] gradientY;
	delete[] threshold;

	//delete[] foundCircle;
	//delete[] houghSpace;
	*/

	//======================== Segment lungs conneted to trachea ===================================================================
	
	/*
	size_t k_min = 198, k_max = 303;
	const size_t height = k_max - k_min + 1;

	dataType** imageData = new dataType * [Height];
	for (k = 0; k < Height; k++) {
		imageData[k] = new dataType[dim2D]{ 0 };
	}

	//size_t x_min = Length, x_max = 0, y_min = 0, y_max = Width, z_min = height, z_max = 0;
	//for (k = 0; k < height; k++) {
	//	for (i = 0; i < Length; i++) {
	//		for (j = 0; j < Width; j++) {
	//			xd = x_new(i, j, Length);
	//			if (maskThreshold[k][xd] == 1.0) {
	//				//Min indexes
	//				if (x_min > i) {
	//					x_min = i;
	//				}
	//				if (y_min > j) {
	//					y_min = j;
	//				}
	//				if (z_min > k) {
	//					z_min = k;
	//				}
	//				//Max indexes
	//				if (x_max < i) {
	//					x_max = i;
	//				}
	//				if (y_max < j) {
	//					y_max = j;
	//				}
	//				if (z_max < k) {
	//					z_max = k;
	//				}
	//			}
	//		}
	//	}
	//}
	//std::cout << "x = (" << x_min << ", " << x_max << ")" << std::endl;
	//std::cout << "y = (" << y_min << ", " << y_max << ")" << std::endl;
	//std::cout << "z = (" << z_min << ", " << z_max << ")" << std::endl;

	dataType** maskThreshold = new dataType * [Height];
	dataType** maskLungs = new dataType * [Height];
	int** labelArray = new int* [Height];
	bool** status = new bool* [Height];
	for (k = 0; k < Height; k++) {
		maskThreshold[k] = new dataType[dim2D]{ 0 };
		maskLungs[k] = new dataType[dim2D]{ 0 };
		labelArray[k] = new int[dim2D] { 0 };
		status[k] = new bool[dim2D] {false};
	}

	size_t kf = 0;
	for (k = k_min; k <= k_max; k++) {
		for (i = 0; i < Length; i++) {
			for (j = 0; j < Width; j++) {
				xd = x_new(i, j, Length);
				//if (k >= z_min && k <= z_max && i >= x_min && i <= x_max && j >= y_min && j <= y_max) {
				//	xd = x_new(i, j, Length);
				//	if (ctContainer->dataPointer[kf][xd] >= thres1) {
				//		maskLungs[k][xd] = ctContainer->dataPointer[kf][xd] + 1024;
				//	}
				//}
				if (ctContainer->dataPointer[k][xd] >= -100 && ctContainer->dataPointer[k][xd] <= 100) {
					maskThreshold[k][xd] = 1.0;
				}
			}
		}
	}

	//size_t kf = 0;
	//for (k = 0, kf = k_min; k < height; k++, kf++) {
	//	for (i = 0; i < dim2D; i++) {
	//		if (ctContainer->dataPointer[kf][i] <= thres1) {
	//			maskThreshold[k][i] = 1.0;
	//		}
	//	}
	//}

	//storing_path = outputPath + "threshold.raw";
	//manageRAWFile3D<dataType>(maskThreshold, Length, Width, height, storing_path.c_str(), STORE_DATA, false);

	//storing_path = outputPath + "mask2.raw";
	//manageRAWFile3D<dataType>(maskLungs, Length, Width, height, storing_path.c_str(), STORE_DATA, false);
	
	
	labelling3D(maskThreshold, labelArray, status, Length, Width, height, 1.0);

	//number of region cells
	int numberOfRegionsCells = countNumberOfRegionsCells(maskThreshold, Length, Width, height, 1.0);
	//cout << "Objects voxels number : " << numberOfRegionsCells << endl;

	//Counting
	int* countingArray = new int[numberOfRegionsCells] {0};
	if (countingArray == NULL)
		return false;

	//Get regions sizes
	for (k = 0; k < height; k++) {
		for (i = 0; i < dim2D; i++) {
			if (labelArray[k][i] > 0) {
				countingArray[labelArray[k][i]]++;
			}
		}
	}

	//Find the biggest region
	int max1 = 0, max2 = 0, max3 = 0, max4 = 0;
	for (k = 0; k < height; k++) {
		for (i = 0; i < dim2D; i++) {
			if (countingArray[labelArray[k][i]] > max1) {
				max1 = countingArray[labelArray[k][i]];
			}
			if (countingArray[labelArray[k][i]] > max2 && countingArray[labelArray[k][i]] < max1) {
				max2 = countingArray[labelArray[k][i]];
			}
			if (countingArray[labelArray[k][i]] > max3 && countingArray[labelArray[k][i]] < max2) {
				max3 = countingArray[labelArray[k][i]];
			}
			if (countingArray[labelArray[k][i]] > max4 && countingArray[labelArray[k][i]] < max3) {
				max4 = countingArray[labelArray[k][i]];
			}
		}
	}
	//cout << "max1 = " << max1 << ", max2 = " << max2 << ", max3 = " << max3 << ", max4 = " << max4 << endl;

	//Keep biggest region
	for (k = 0; k < height; k++) {
		for (i = 0; i < dim2D; i++) {
			if (countingArray[labelArray[k][i]] == max3) {
				maskThreshold[k][i] = 1.0;
			}
			else {
				maskThreshold[k][i] = 0.0;
			}
		}
	}
	

	//storing_path = outputPath + "segment.raw";
	//manageRAWFile3D<dataType>(maskThreshold, Length, Width, height, storing_path.c_str(), LOAD_DATA, false);
	
	//size_t new_length = x_max - x_min + 1;
	//size_t new_width = y_max - y_min + 1;
	//size_t new_height = z_max - z_min + 1;

	//std::cout << "height = " << new_height << std::endl;
	//std::cout << "lenght = " << new_length << std::endl;
	//std::cout << "width = " << new_width << std::endl;

	//dataType** lungsCroped = new dataType * [new_height];
	//for (k = 0; k < new_height; k++) {
	//	lungsCroped[k] = new dataType[new_length * new_width]{0};
	//}

	storing_path = outputPath + "ThresholdHeart.raw";
	manageRAWFile3D<dataType>(maskThreshold, Length, Width, Height, storing_path.c_str(), STORE_DATA, false);

	//delete[] countingArray;
	for (k = 0; k < Height; k++) {
		delete[] maskThreshold[k];
		delete[] maskLungs[k];
		delete[] imageData[k];
		delete[] labelArray[k];
		delete[] status[k];
	}
	delete[] labelArray;
	delete[] status;
	delete[] maskThreshold;
	delete[] maskLungs;
	delete[] imageData;
	*/
	
	//======================== Segment Trachea ===========================================
	
	/*
	dataType** lungsAndTrachea = new dataType * [Height];
	dataType** trachea = new dataType * [Height];
	int** labelArray = new int* [Height];
	bool** status = new bool* [Height];
	for (k = 0; k < Height; k++) {
		lungsAndTrachea[k] = new dataType[dim2D]{ 0 };
		trachea[k] = new dataType[dim2D]{ 0 };
		labelArray[k] = new int[dim2D] {0};
		status[k] = new bool[dim2D];
	}

	loading_path = inputPath + "lungs_and_trachea.raw";
	load3dArrayRAW<dataType>(lungsAndTrachea, Length, Width, Height, loading_path.c_str(), false);

	loading_path = inputPath + "lungs.raw";
	load3dArrayRAW<dataType>(imageData, Length, Width, Height, loading_path.c_str(), false);

	//Difference
	for (k = 0; k < Height; k++) {
		for (i = 0; i < dim2D; i++) {
			if (imageData[k][i] == 1.0) {
				trachea[k][i] = 0.0;
			}
			else {
				trachea[k][i] = lungsAndTrachea[k][i];
			}
		}
	}

	erosion3dHeighteenNeigbours(trachea, Length, Width, Height, 1.0, 0.0);

	//storing_path = outputPath + "difference.raw";
	//store3dRawData<dataType>(trachea, Length, Width, Height, storing_path.c_str());

	//number of region cells
	int numberOfRegionsCells = countNumberOfRegionsCells(trachea, Length, Width, Height, 1.0);

	labelling3D(trachea, labelArray, status, Length, Width, Height, 1.0);

	//Counting
	int* countingArray = new int[numberOfRegionsCells] {0};
	if (countingArray == NULL)
		return false;

	//Get region size
	for (k = 0; k < Height; k++) {
		for (i = 0; i < dim2D; i++) {
			if (labelArray[k][i] > 0) {
				countingArray[labelArray[k][i]]++;
			}
		}
	}

	//Find the biggest region
	int max1 = 0, max2 = 0;
	for (k = 0; k < Height; k++) {
		for (i = 0; i < dim2D; i++) {
			if (countingArray[labelArray[k][i]] > max1) {
				max1 = countingArray[labelArray[k][i]];
			}
			if (countingArray[labelArray[k][i]] > max2 && countingArray[labelArray[k][i]] < max1) {
				max2 = countingArray[labelArray[k][i]];
			}
		}
	}

	//Keep biggest region
	for (k = 0; k < Height; k++) {
		for (i = 0; i < dim2D; i++) {
			if (countingArray[labelArray[k][i]] == max1) {
				trachea[k][i] = 1.0;
			}
			else {
				trachea[k][i] = 0.0;
			}
		}
	}

	//dilatation to fill trachea
	dilatation3dHeighteenNeigbours(trachea, Length, Width, Height, 1.0, 0.0);

	storing_path = outputPath + "difference.raw";
	store3dRawData<dataType>(trachea, Length, Width, Height, storing_path.c_str());

	for (k = 0; k < Height; k++) {
		delete[] lungsAndTrachea[k];
		delete[] trachea[k];
		delete[] labelArray[k];
		delete[] status[k];
	}
	delete[] lungsAndTrachea;
	delete[] trachea;
	delete[] labelArray;
	delete[] status;
	*/
	
	//======================== Detect carina =============================================
	
	/*
	dataType** imageData = new dataType * [Height];
	for (k = 0; k < Height; k++) {
		imageData[k] = new dataType[dim2D]{ 0 };
	}

	loading_path = inputPath + "trachea.raw";
	load3dArrayRAW<dataType>(imageData, Length, Width, Height, loading_path.c_str(), false);
	
	int* labelArray = new int[dim2D];
	bool* status = new bool[dim2D];
	dataType* mask = new dataType[dim2D];
	if (labelArray == NULL || status == NULL || mask == NULL)
		return false;

	size_t n, count_slice = 0, slice_of_interest = 0;
	bool previous_slice = false;

	storing_path = outputPath + "info_regions.csv";
	FILE* file;
	if (fopen_s(&file, storing_path.c_str(), "w") != 0) {
		printf("Enable to open");
		return false;
	}
	fprintf(file, "slice number,number of regions\n");

	//size_t k_previous = 0, k_current = 0, k_next = 0;

	for (k = 0; k < Height; k++) {

		//if (k == 0) {
		//	k_previous = 0;
		//	k_current = k_previous;
		//	k_next = 1;
		//}
		//else {
		//	if (k == Height - 1) {
		//		k_previous = Height - 1;
		//		k_current = Height - 1;
		//		k_next = k_current;
		//	}
		//	else {
		//		k_previous = k - 1;
		//		k_current = k;
		//		k_next = k + 1;
		//	}
		//}

		//cout << "Slice " << k << ":" << endl;
		fprintf(file, "%d,", k);
		
		//Initialization
		for (i = 0; i < dim2D; i++) {
			mask[i] = imageData[k][i];
			labelArray[i] = 0;
			status[i] = false;
		}

		labeling(mask, labelArray, status, Length, Width, 1.0);

		//number of region cells
		int numberOfRegionsCells = 0;
		for (i = 0; i < dim2D; i++) {
			if (mask[i] == 1.0) {
				numberOfRegionsCells++;
			}
		}

		if (numberOfRegionsCells != 0) {


			int* countingArray = new int[numberOfRegionsCells] {0};

			//Get regions sizes
			for (i = 0; i < dim2D; i++) {
				if (labelArray[i] > 0) {
					countingArray[labelArray[i]]++;
				}
			}

			//Get number of regions
			int numb_region = 0, label = 1;
			for (i = 0; i < dim2D; i++) {
				if (labelArray[i] == label) {
					numb_region++;
					label++;
				}
			}
			
			////Print number of elements per region
			//for (i = 1; i <= numb_region; i++) {
			//	std::cout << "region " << i << " has " << countingArray[i] << " elements" << std::endl;
			//}
			//std::cout << "############" << std::endl;

			//std::cout << "Slice " << k  << " has " << numb_region << " region(s)" << std::endl;
			fprintf(file, "%d\n", numb_region);

			if (numb_region > 1) {
				count_slice++;
				previous_slice = true;
			}
			else {
				count_slice = 0;
				previous_slice = false;
			}

			if (count_slice == 5 && previous_slice == true) {
				slice_of_interest = k - 5;
				count_slice = 0;
				std::cout << "The slice of interest is : " << slice_of_interest << std::endl;
			}

			delete[] countingArray;

		}
		else {
			fprintf(file, "%d\n", numberOfRegionsCells);
			previous_slice = false;
			count_slice = 0;
		}
		//std::cout << "count slice " << count_slice << std::endl;

	}

	fclose(file);
	delete[] labelArray;
	delete[] status;
	*/
	
	//======================== 2D path finding ==================================
	
	/*
	//const size_t height = 256, width = 256, dim2D = height * width;
	const size_t height = 1024, width = 1024, dim2D = height * width;

	dataType* imageData = new dataType[dim2D]{ 0 };
	dataType* actionMap = new dataType[dim2D]{ 0 };
	dataType* potential = new dataType[dim2D]{ 0 };
	dataType* path = new dataType[dim2D]{ 0 };

	//loading_path = inputPath + "artificial.raw";
	//loading_path = inputPath + "raw/slice/eye_1024_1024.raw";
	//loading_path = inputPath + "raw/slice/eye_rescaled.raw";
	loading_path = inputPath + "raw/slice/eye_filtered.raw";
	//loading_path = inputPath + "raw/slice/threshold_eye.raw";
	//loading_path = inputPath + "raw/slice/distanceMap.raw";
	load2dArrayRAW<dataType>(imageData, height, width, loading_path.c_str(), false);

	//storing_path = outputPath + "loaded.raw";
	//store2dRawData<dataType>(imageData, height, width, storing_path.c_str());

	Point2D* seedPoints = new Point2D[2];
	////Point2D seed1 = { 8, 128 }, seed2 = { 131, 66 }; // artificial
	////Point2D seed2 = { 284, 540 }, seed1 = { 22, 470 };
	////Point2D seed2 = { 356, 304 }, seed1 = { 558, 102 }; //small vessel
	
	Point2D seed1 = { 314, 537 }, seed2 = { 509, 824 }; //bigger vessel
	seedPoints[0] = seed1; 
	seedPoints[1] = seed2;

	storing_path = outputPath + "seed.csv";
	FILE* file_seed;
	if (fopen_s(&file_seed, storing_path.c_str(), "w") != 0) {
		printf("Enable to open");
		return false;
	}
	fprintf(file_seed, "x,y\n");
	fprintf(file_seed, "%f,%f\n", seed1.x, seed1.y);
	fprintf(file_seed, "%f,%f\n", seed1.x, seed1.y + 1);
	fprintf(file_seed, "%f,%f\n", seed1.x, seed1.y - 1);
	fprintf(file_seed, "%f,%f\n", seed1.x - 1, seed1.y - 1);
	fprintf(file_seed, "%f,%f\n", seed1.x - 1, seed1.y + 1);
	fprintf(file_seed, "%f,%f\n", seed1.x - 1, seed1.y);
	fprintf(file_seed, "%f,%f\n", seed1.x + 1, seed1.y);
	fprintf(file_seed, "%f,%f\n", seed1.x + 1, seed1.y - 1);
	fprintf(file_seed, "%f,%f\n", seed1.x + 1, seed1.y + 1);
	fprintf(file_seed, "%f,%f\n", seed2.x, seed2.y);
	fprintf(file_seed, "%f,%f\n", seed2.x, seed2.y + 1);
	fprintf(file_seed, "%f,%f\n", seed2.x, seed2.y - 1);
	fprintf(file_seed, "%f,%f\n", seed2.x - 1, seed2.y - 1);
	fprintf(file_seed, "%f,%f\n", seed2.x - 1, seed2.y + 1);
	fprintf(file_seed, "%f,%f\n", seed2.x - 1, seed2.y);
	fprintf(file_seed, "%f,%f\n", seed2.x + 1, seed2.y - 1);
	fprintf(file_seed, "%f,%f\n", seed2.x + 1, seed2.y + 1);
	fprintf(file_seed, "%f,%f\n", seed2.x + 1, seed2.y);
	fclose(file_seed);

	////generate stat potential
	//dataType radiusInitial = 2.0, radiusMax = 4.0, radiusStep = 1.0;
	//Statistics stats = { 0.0, 0.0, 0.0, 0.0, 0.0 };
	//Point2D origin = { 0.0, 0.0 };
	//PixelSpacing spacing = { 1.0, 1.0 };
	//Image_Data2D ctImageData = { height, width, imageData, origin, spacing, orientation2D };
	
	//stats = get2DPointNeighborhoodStats(ctImageData, seed1, radI);
	//dataType meanI = stats.mean_data / radI, varianceI = stats.variance / radI;
	//for (i = 0; i < height; i++) {
	//	for (j = 0; j < width; j++) {
	//		Point2D current = { i, j };
	//		stats = get2DPointNeighborhoodStats(ctImageData, current, radI);
	//		stats.mean_data = stats.mean_data / radI;
	//		stats.variance = stats.variance / radI;
	//		potential[x_new(i, j, height)] = 0.00001 + 1.0 * pow(meanI - stats.mean_data, 2) + 1.0 * pow(varianceI - stats.variance, 2);
	//	}
	//}

	//double start_time = clock();
	//computePotentialMeanVariance(ctImageData, potential, seed1, radiusInitial, radiusMax, radiusStep);
	//double end_time = clock();
	//std::cout << "The computation lasts : " << (end_time - start_time) / CLOCKS_PER_SEC << " seconds." << std::endl;

	partialFrontPropagation2D(imageData, actionMap, potential, height, width, seedPoints);

	storing_path = outputPath + "action.raw";
	store2dRawData<dataType>(actionMap, height, width, storing_path.c_str());
	
	shortestPath2d(actionMap, path, height, width, 1.0, seedPoints);

	//storing_path = outputPath + "loaded.raw";
	//store2dRawData<dataType>(imageData, height, width, storing_path.c_str());

	//fastSweepingFunction_2D(actionMap, imageData, height, width, 1.0, 1000000.0, 0.0);
	//dataType* gradientVectorX = new dataType[dim2D]{ 0 };
	//dataType* gradientVectorY = new dataType[dim2D]{ 0 };
	//computeImageGradient(imageData, gradientVectorX, gradientVectorY, height, width, 1.0);
	//dataType norm_of_gradient = 0.0, ux = 0.0, uy = 0.0;
	//for (k = 0; k < dim2D; k++) {
	//	ux = gradientVectorX[k];
	//	uy = gradientVectorY[k];
	//	norm_of_gradient = sqrt(ux * ux + uy * uy);
	//	actionMap[k] = gradientFunction(norm_of_gradient, 1000);
	//}
	//for (k = 0; k < dim2D; k++) {
	//	if (actionMap[k] < 0.15) {
	//		actionMap[k] = 1.0;
	//	}
	//	else {
	//		actionMap[k] = 0.0;
	//	}
	//}
	//delete[] gradientVectorX;
	//delete[] gradientVectorY;

	//rescaleNewRange2D(imageData, height, width, 0, 1);
	////filtering parameters
	//Filter_Parameters filter_parameters;
	//filter_parameters.h = 1.0; filter_parameters.timeStepSize = 0.25; filter_parameters.eps2 = 1e-6;
	//filter_parameters.omega_c = 1.5; filter_parameters.tolerance = 1e-3; filter_parameters.maxNumberOfSolverIteration = 100;
	//filter_parameters.timeStepsNum = 10; filter_parameters.coef = 1e-6;
	//Image_Data2D IMAGE;
	//IMAGE.imageDataPtr = imageData;
	//IMAGE.height = height; IMAGE.width = width;
	//heatImplicit2dScheme(IMAGE, filter_parameters);
	//storing_path = outputPath + "filtered.raw";
	//store2dRawData<dataType>(imageData, height, width, storing_path.c_str());

	//The potential is computed inside the fast marching function 
	//fastMarching2D(imageData, actionMap, potential, height, width, seedPoints);
	//double dPoint1 = 0.0, dPoint2 = 0.0;
	//for (i = 0; i < height; i++) {
	//	for (j = 0; j < width; j++) {
	//		Point2D current = { i, j };
	//		dPoint1 = getPoint2DDistance(seed1, current);
	//		dPoint2 = getPoint2DDistance(seed2, current);
	//		if (dPoint1 <= 10) {
	//			potential[x_new(i, j, height)] = 1.0;
	//		}
	//		if (dPoint2 <= 10) {
	//			potential[x_new(i, j, height)] = 1.0;
	//		}
	//	}
	//}

	//const size_t radiusLength = 10;
	//dataType radiusInitial = 3.0, radiusFinal = 8.0, radiusScale = 1.0;

	//dataType** potentialPtr = new dataType * [radiusLength];
	//dataType** actionPtr = new dataType * [radiusLength];
	//dataType** pathPtr = new dataType * [radiusLength];
	//for (k = 0; k < radiusLength; k++) {
	//	potentialPtr[k] = new dataType[dim2D] {0};
	//	actionPtr[k] = new dataType[dim2D]{0};
	//	pathPtr[k] = new dataType[dim2D]{ 0 };
	//}

	//Statistics stats = { 0.0, 0.0, 0.0, 0.0, 0.0 };
	//double radI = 5.0;
	//Point2D origin = { 0.0, 0.0 };
	//PixelSpacing spacing = { 1.0, 1.0 };
	//Image_Data2D ctImageData = { height, width, imageData, origin, spacing, orientation2D };
	//stats = get2DPointNeighborhoodStats(ctImageData, seed1, radI);

	//computePotentialMeanVariance(ctImageData, potentialPtr, radiusLength, seed1, radiusInitial, radiusScale);
	//storing_path = outputPath + "potential3D.raw";
	//store3dRawData<dataType>(potentialPtr, height, width, radiusLength, storing_path.c_str());
	//load3dArrayRAW<dataType>(potentialPtr, height, width, radiusLength, storing_path.c_str(), false);

	//computePotentialMeanVariance(ctImageData, potentialPtr, radiusLength, seed1, radiusInitial, radiusScale);
	//Point3D p1 = { seed1.x, seed1.y, radiusInitial }, p2 = { seed2.x, seed2.y, radiusFinal };
	//Point3D* seeds = new Point3D[2];
	//seeds[0] = p1; seeds[1] = p2;
	//partialFrontPropagation(actionPtr, potentialPtr, height, width, radiusLength, seeds);

	//storing_path = outputPath + "action3D.raw";
	//store3dRawData<dataType>(actionPtr, height, width, radiusLength, storing_path.c_str());
	//load3dArrayRAW<dataType>(actionPtr, height, width, radiusLength, storing_path.c_str(), false);

	//vector<Point3D> path_points;
	//shortestPath3D(actionPtr, pathPtr, height, width, radiusLength, 1.0, seeds, path_points);
	//string saving_name = outputPath + "path2DPlus.csv";
	//FILE* file;
	//if (fopen_s(&file, saving_name.c_str(), "w") != 0) {
	//	printf("Enable to open");
	//	return false;
	//}
	//fprintf(file, "x,y,r\n");
	//for (k = 0; k < path_points.size(); k++) {
	//	fprintf(file, "%f,%f,%f\n", path_points[k].x, path_points[k].y, path_points[k].z);
	//}
	//fclose(file);
	
	//for (i = 0; i < 192; i++) {
	//	for (j = 0; j < width; j++) {
	//		if (j == 128) {
	//			imageData[x_new(i, j - 2, height)] = 1.0;
	//			imageData[x_new(i, j - 1, height)] = 1.0;
	//			imageData[x_new(i, j, height)] = 1.0;
	//			imageData[x_new(i, j + 1, height)] = 1.0;
	//			imageData[x_new(i, j + 2, height)] = 1.0;
	//		}
	//	}
	//}
	//size_t i_min = 128, i_max = 192;
	//size_t j_min = 64, j_max = 128;
	//size_t in = 0, jn = 0;
	//for (i = i_min, in = 0; i < i_max; i++, in++) {
	//	for (j = j_min, jn = 0; j < j_max; j++, jn++) {
	//		if (in == jn) {
	//			imageData[x_new(i, j - 2, height)] = 1.0;
	//			imageData[x_new(i, j - 1, height)] = 1.0;
	//			imageData[x_new(i, j, height)] = 1.0;
	//			imageData[x_new(i, j + 1, height)] = 1.0;
	//			imageData[x_new(i, j + 2, height)] = 1.0;
	//		}
	//	}
	//}
	//i_min = 128, i_max = 192;
	//j_min = 128, j_max = 192;
	//for (i = i_max, in = 0; i > i_min; i--, in++) {
	//	for (j = j_min, jn = 0; j < j_max; j++, jn++) {
	//		if (in == jn) {
	//			imageData[x_new(i, j - 2, height)] = 1.0;
	//			imageData[x_new(i, j - 1, height)] = 1.0;
	//			imageData[x_new(i, j, height)] = 1.0;
	//			imageData[x_new(i, j + 1, height)] = 1.0;
	//			imageData[x_new(i, j + 2, height)] = 1.0;
	//		}
	//	}
	//}
	//storing_path = outputPath + "artificial.raw";
	//store2dRawData<dataType>(imageData, height, width, storing_path.c_str());
	
	//for (k = 0; k < radiusLength; k++) {
	//	delete[] potentialPtr[k];
	//	delete[] actionPtr[k];
	//	delete[] pathPtr[k];
	//}
	//delete[] potentialPtr;
	//delete[] actionPtr;
	//delete[] pathPtr;

	//delete[] seeds;
	//delete[] seedPoints;

	delete[] seedPoints;
	delete[] imageData;
	delete[] actionMap;
	delete[] potential;
	delete[] path;
	*/
	
	return EXIT_SUCCESS;
}