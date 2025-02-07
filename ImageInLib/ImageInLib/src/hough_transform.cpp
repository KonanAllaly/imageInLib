#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h> 
#include <memory.h>
#include <math.h>
#include <string>
#include "common_math.h"

#include"hough_transform.h"
#include"morphological_change.h"
#include"../src/imageInterpolation.h"

#define foreground 1.0
#define background 0.0

void initialize2dArrayBool(bool* array2D, const size_t length, const size_t width) {
	for (size_t i = 0; i < length * width; i++) {
		array2D[i] = false;
	}
}

void initialize2dArray(dataType* array2D, const size_t length, const size_t width) {
	for (size_t i = 0; i < length * width; i++) {
		array2D[i] = 0.0;
	}
}

Point2D getPointWithMaximalValue2D(dataType* arrayPtr2D, const size_t length, const size_t width) {
	
	dataType x = 0.0, y = 0.0;
	dataType max_value = 0.0;
	for (size_t i = 0; i < length; i++) {
		for (size_t j = 0; j < width; j++) {
			size_t xd = x_new(i, j, length);
			if (arrayPtr2D[xd] >= max_value) {
				max_value = arrayPtr2D[xd];
				x = i;
				y = j;
			}
		}
	}
	Point2D maxPoint = { x, y };
	return maxPoint;
}

Point2D localHoughTransform(Image_Data2D imageDataStr, Point2D seed, dataType* houghSpacePtr, dataType* foundCirclePtr, HoughParameters params) {

	const size_t length = imageDataStr.height;
	const size_t width = imageDataStr.width;
	const size_t dim2D = length * width;
	size_t i_new, j_new, xd_new;
	BoundingBox2D box = { 0, 0, 0, 0 };
	Point2D center_circle = { 0.0, 0.0 };
	Point2D current_point = { 0.0, 0.0 };
	Point2D found_center = { 0.0, 0.0 };

	double dist_point = 0.0, found_radius = 0.0, radius = 0.0;
	bool test_max = false;
	
	dataType* edgeDetector = (dataType*)malloc(sizeof(dataType) * dim2D);
	dataType* maskThreshold = (dataType*)malloc(sizeof(dataType) * dim2D);
	dataType* houghSpaceMax = (dataType*)malloc(sizeof(dataType) * dim2D);
	dataType* gradientX = (dataType*)malloc(sizeof(dataType) * dim2D);
	dataType* gradientY = (dataType*)malloc(sizeof(dataType) * dim2D);

	initialize2dArray(gradientX, length, width);
	initialize2dArray(gradientY, length, width);

	//compute the edge detector
	initialize2dArray(edgeDetector, length, width);
	computeImageGradient(imageDataStr, gradientX, gradientY);
	dataType norm_of_gradient = 0.0;
	for (size_t i = 0; i < dim2D; i++) {
		norm_of_gradient = sqrt(gradientX[i] * gradientX[i] + gradientY[i] * gradientY[i]);
		edgeDetector[i] = gradientFunction(norm_of_gradient, params.K);
		//threshold of edge detector
		if (edgeDetector[i] <= params.thres) {
			maskThreshold[i] = 1.0;
		}
		else {
			maskThreshold[i] = 0.0;
		}
	}

	//std::string outputPath = "C:/Users/Konan Allaly/Documents/Tests/output/threshold.raw";
	//manageRAWFile2D<dataType>(maskThreshold, length, width, outputPath.c_str(), STORE_DATA, false);

	////copy edge detector to mask threshold
	//initialize2dArray(maskThreshold, length, width);
	//copyDataToAnother2dArray(edgeDetector, maskThreshold, length, width);
	////thresholding of the edge detector
	//thresholding2DFunction(maskThreshold, length, width, params.thres, params.thres);

	/*
	//==================
	//create mask in threshold for second circle detection
	//double r_max = 8.5; //--> P2
	//Point2D mask_center = { 294, 315 };
	//double r_max = 12.0; //--> P3
	//Point2D mask_center = { 252, 252 }; //--> P3
	//double r_max = 9.0; //--> P1
	//Point2D mask_center = { 279, 296 }; //--> P1
	//double r_max = 14.0; //--> P4
	//Point2D mask_center = { 255, 222 }; //--> P4
	//double r_max = 12.0; //--> P5
	//Point2D mask_center = { 270, 233 }; //--> P5
	//double r_max = 12.5; //--> P6
	//Point2D mask_center = { 281, 284 }; //--> P6
	//double r_max = 13.5; //--> P7
	//Point2D mask_center = { 249, 294 }; //--> P7
	//double r_max = 12.0; //--> P2 : automatic
	//Point2D mask_center = { 254, 248 }; //--> P2 automatic
	//double r_max = 13.5; //--> P2 : for poster (v1)
	//Point2D mask_center = { 253, 248 }; //--> P2 for poster (v1)
	//double r_max = 12; //--> P2 : for poster
	//Point2D mask_center = { 292, 314 }; //--> P2 for poster
	double r_max = 14; //--> P4 : for poster
	Point2D mask_center = { 287, 288 }; //--> P4 for poster
	mask_center = getRealCoordFromImageCoord2D(mask_center, originImage, params.spacing, orientation);
	for (i = 0; i < length; i++) {
		for (j = 0; j < width; j++) {
			Point2D current_point_mask = { (dataType)i, (dataType)j };
			current_point_mask = getRealCoordFromImageCoord2D(current_point_mask, originImage, params.spacing, orientation);
			double dis_mask = getPoint2DDistance(mask_center, current_point_mask);
			if (dis_mask <= r_max) {
				maskThreshold[x_new(i, j, length)] = 1.0;
			}
		}
	}
	//==================
	*/

	initialize2dArray(foundCirclePtr, length, width);

	int count_vote = 0, count_neighbor = 0, total_neighbor = 0;
	dataType hough_ratio = 0.0, found_ratio = 0.0, max_ratio = 0.0;
	size_t i_min = 0, i_max = 0, j_min = 0, j_max = 0;
	radius = params.radius_min;
	
	double x = 0.0, y = 0.0;

	initialize2dArray(houghSpaceMax, length, width);

	while (radius <= params.radius_max) {
		
		initialize2dArray(houghSpacePtr, length, width);
		
		//find the bounding box of the seed point
		box = findBoundingBox2D(seed, length, width, radius, params.offset);

		////small bounding box
		//x = box.i_max - radius - params.epsilon;
		//if (x < 0) {
		//	i_max = 0;
		//}
		//else {
		//	i_max = (size_t)x;
		//}
		//i_min = (size_t)(box.i_min + radius + params.epsilon);
		//if (i_min >= length - 1) {
		//	i_min = length - 1;
		//}
		//
		//y = box.j_max - radius - params.epsilon;
		//if (y < 0) {
		//	j_max = 0;
		//}
		//else {
		//	j_max = (size_t)y;
		//}
		//j_min = (size_t)(box.j_min + radius + params.epsilon);
		//if (j_min >= width - 1) {
		//	j_min = width - 1;
		//}
		
		//visit all the pixels in the bounding box
		for (size_t i = box.i_min; i <= box.i_max; i++) {
			for (size_t j = box.j_min; j <= box.j_max; j++) {

				size_t xd = x_new(i, j, length);
				center_circle.x = (dataType)i; 
				center_circle.y = (dataType)j;

				//Take into account the spacing
				center_circle = getRealCoordFromImageCoord2D(center_circle, imageDataStr.origin, imageDataStr.spacing, imageDataStr.orientation);
				
				//vote
				count_vote = 0;
				count_neighbor = 0;
				total_neighbor = 0;
				
				if (maskThreshold[xd] == background) {
					for (size_t i_new = box.i_min; i_new <= box.i_max; i_new++) {
						for (size_t j_new = box.j_min; j_new <= box.j_max; j_new++) {
							xd_new = x_new(i_new, j_new, length);

							current_point.x = (dataType)i_new; 
							current_point.y = (dataType)j_new;
							current_point = getRealCoordFromImageCoord2D(current_point, imageDataStr.origin, imageDataStr.spacing, imageDataStr.orientation);
							dist_point = getPoint2DDistance(center_circle, current_point);
							
							//count foreground pixels in the band
							if (dist_point >= (radius - params.epsilon) && dist_point <= (radius + params.epsilon)) {
								total_neighbor++;
								if (maskThreshold[xd_new] == foreground) {
									count_vote++;
								}
							}

							//count foreground pixels close to the current center
							if (dist_point < (radius - params.epsilon) && maskThreshold[xd_new] == foreground) {
								count_neighbor++;
							}

						}
					}
				}

				hough_ratio = (dataType)count_vote / (dataType)total_neighbor;

				if (count_neighbor == 0) {
					houghSpacePtr[xd] = hough_ratio;
				}
			}
		}

		//keep the maximal ratio for each pixel
		for (size_t i = 0; i < length * width; i++) {
			if (houghSpacePtr[i] >= houghSpaceMax[i]) {
				houghSpaceMax[i] = houghSpacePtr[i];
			}
		}

		//find the maximal ratio for current radius
		max_ratio = getTheMaxValue(houghSpaceMax, length, width);
		
		//find the maximal ratio for whole radius range
		if (max_ratio > found_ratio) {
			found_ratio = max_ratio;
			found_radius = radius;
			test_max = true;
		}
		
		//update the found_center if the value of found_ratio was updated
		if (test_max == true) {
			found_center = getPointWithMaximalValue2D(houghSpaceMax, length, width);
			test_max = false;
		}

		radius += params.radius_step;

	}

	//find the point with max ratio
	found_center = getPointWithMaximalValue2D(houghSpaceMax, length, width);

	printf("Found radius = %lf\n", found_radius);
	printf("Found center : (%f, %f)\n", found_center.x, found_center.y);

	if (found_center.x != 0 && found_center.y != 0) {
		//add found center
		foundCirclePtr[x_new((size_t)found_center.x, (size_t)found_center.y, length)] = 1.0;
		Point2D draw_point = getRealCoordFromImageCoord2D(found_center, imageDataStr.origin, imageDataStr.spacing, imageDataStr.orientation);
		
		//draw the found circle
		box = findBoundingBox2D(found_center, length, width, found_radius, params.offset);
		for (size_t i = box.i_min; i <= box.i_max; i++) {
			for (size_t j = box.j_min; j <= box.j_max; j++) {
				size_t xd = x_new(i, j, length);
				current_point.x = (dataType)i; 
				current_point.y = (dataType)j;
				current_point = getRealCoordFromImageCoord2D(current_point, imageDataStr.origin, imageDataStr.spacing, imageDataStr.orientation);
				dist_point = getPoint2DDistance(draw_point, current_point);
				
				//Draw disque
				if (dist_point <= (found_radius + params.epsilon)) {
					foundCirclePtr[xd] = 1.0;
				}
			}
		}
	}

	//copy back
	copyDataToAnother2dArray(houghSpaceMax, houghSpacePtr, length, width);

	//max_ratio = getTheMaxValue(houghSpaceMax, length, width);

	//Draw search domain
	box = findBoundingBox2D(seed, length, width, params.radius_max, params.offset);
	for (size_t i = box.i_min; i <= box.i_max; i++) {
		for (size_t j = box.j_min; j <= box.j_max; j++) {
			if ((i == box.i_min || i == box.i_max) || (j == box.j_min || j == box.j_max)) {
				foundCirclePtr[x_new(i, j, length)] = 1.0;
			}
		}
	}

	////add the seed
	//imageDataPtr[x_new((size_t)found_center.x, (size_t)found_center.y, length)] = 0.0;

	free(edgeDetector);
	free(maskThreshold);
	free(houghSpaceMax);
	free(gradientX);
	free(gradientY);

	return found_center;
}

Point2D localHoughWithCanny(Image_Data2D imageDataStr, Point2D seed, dataType* houghSpacePtr, dataType* foundCirclePtr, HoughParameters params) {

	const size_t length = imageDataStr.height;
	const size_t width = imageDataStr.width;

	size_t i, j, xd, dim2D = length * width;
	size_t i_new, j_new, xd_new;
	BoundingBox2D box = { 0, 0, 0, 0 };
	Point2D center_circle = { 0.0, 0.0 }, current_point = { 0.0, 0.0 }, found_center = { 0.0, 0.0 };

	double dist_point = 0.0, found_radius, radius = 0.0;
	bool test_max = false;

	dataType* maskThreshold = (dataType*)malloc(sizeof(dataType) * dim2D);
	dataType* houghSpaceMax = (dataType*)malloc(sizeof(dataType) * dim2D);
	dataType* gradientX = (dataType*)malloc(sizeof(dataType) * dim2D);
	dataType* gradientY = (dataType*)malloc(sizeof(dataType) * dim2D);
	dataType* gradientAngle = (dataType*)malloc(sizeof(dataType) * dim2D);
	dataType* normGradient = (dataType*)malloc(sizeof(dataType) * dim2D);
	size_t* statusPixel = (size_t*)malloc(sizeof(size_t) * dim2D);

	initialize2dArray(maskThreshold, length, width);
	initialize2dArray(houghSpaceMax, length, width);
	initialize2dArray(gradientX, length, width);
	initialize2dArray(gradientY, length, width);
	initialize2dArray(gradientAngle, length, width);
	initialize2dArray(normGradient, length, width);
	initialize2dArray(foundCirclePtr, length, width);

	computeImageGradient(imageDataStr, gradientX, gradientY);
	computeAngleFromGradient(gradientAngle, gradientX, gradientY, length, width);
	
	//norm of gradient
	for (i = 0; i < dim2D; i++) {
		normGradient[i] = sqrt(gradientX[i] * gradientX[i] + gradientY[i] * gradientY[i]);
	}
	
	nonMaximumSuppression(maskThreshold, normGradient, gradientAngle, length, width);
	thresholdByHyteresis(maskThreshold, normGradient, statusPixel, length, width, 0.005, 0.009);

	//std::string outputPath = "C:/Users/Konan Allaly/Documents/Tests/output/threshold.raw";
	//manageRAWFile2D<dataType>(maskThreshold, length, width, outputPath.c_str(), STORE_DATA, false);

	size_t count_vote = 0, count_neighbor = 0, total_neighbor = 0;
	dataType hough_ratio = 0.0, found_ratio = 0.0, max_ratio = 0.0;
	size_t i_min = 0, i_max = 0, j_min = 0, j_max = 0;
	radius = params.radius_min;
	found_radius = 0.0;

	while (radius <= params.radius_max) {
		
		//initialize the 2D array
		initialize2dArray(houghSpacePtr, length, width);
		//find the bounding box of the seed point
		box = findBoundingBox2D(seed, length, width, radius, params.offset);
		
		//small bounding box
		i_max = (size_t)(box.i_max + radius + params.epsilon);
		if (i_max >= length - 1) {
			i_max = length - 1;
		}
		if (box.i_min >= radius) {
			i_min = (size_t)(box.i_min - radius - params.epsilon);
		}
		j_max = (size_t)(box.j_max + radius + params.epsilon);
		if (j_max >= width - 1) {
			j_max = width - 1;
		}
		if (box.j_min >= radius) {
			j_min = (size_t)(box.j_min - radius - params.epsilon);
		}

		//visit all the pixels in the bounding box
		for (i = i_min; i <= i_max; i++) {
			for (j = j_min; j <= j_max; j++) {
				xd = x_new(i, j, length);

				center_circle.x = (dataType)i;
				center_circle.y = (dataType)j;
				center_circle = getRealCoordFromImageCoord2D(center_circle, imageDataStr.origin, imageDataStr.spacing, imageDataStr.orientation);

				//vote
				count_vote = 0;
				count_neighbor = 0;
				total_neighbor = 0;
				if (maskThreshold[xd] == background) {
					for (i_new = box.i_min; i_new <= box.i_max; i_new++) {
						for (j_new = box.j_min; j_new <= box.j_max; j_new++) {
							xd_new = x_new(i_new, j_new, length);
							
							current_point.x = (dataType)i_new;
							current_point.y = (dataType)j_new;
							current_point = getRealCoordFromImageCoord2D(current_point, imageDataStr.origin, imageDataStr.spacing, imageDataStr.orientation);
							dist_point = getPoint2DDistance(center_circle, current_point);

							//count foreground pixels in the band
							if (dist_point >= (radius - params.epsilon) && dist_point <= (radius + params.epsilon)) {
								total_neighbor++;
								if (maskThreshold[xd_new] == foreground) {
									count_vote++;
								}
							}
							//count foreground pixels close to the current center
							if (dist_point < (radius - params.epsilon) && maskThreshold[xd_new] == foreground) {
								count_neighbor++;
							}

						}
					}
				}

				//compute ratios
				hough_ratio = (dataType)count_vote / (dataType)total_neighbor;
				if (count_neighbor == 0) {
					houghSpacePtr[xd] = hough_ratio;
				}
			}
		}

		//keep the maximal ratio for each pixel
		for (i = box.i_min; i <= box.i_max; i++) {
			for (j = box.j_min; j <= box.j_max; j++) {
				xd = x_new(i, j, length);
				if (houghSpacePtr[xd] > houghSpaceMax[xd]) {
					houghSpaceMax[xd] = houghSpacePtr[xd];
				}
			}
		}

		//find the maximal ratio for current radius
		max_ratio = getTheMaxValue(houghSpacePtr, length, width);
		//find the maximal ratio for whole radius range
		if (max_ratio > found_ratio) {
			found_ratio = max_ratio;
			found_radius = radius;
			test_max = true;
		}
		//update the found_center if the value of found_ratio was updated
		if (test_max == true) {
			found_center = getPointWithMaximalValue2D(houghSpaceMax, length, width);
			test_max = false;
		}
		radius += params.radius_step;
	}

	printf("Found radius = %lf\n", found_radius);

	//add the seed
	foundCirclePtr[x_new((size_t)seed.x, (size_t)seed.y, length)] = 1.0;

	//draw maximal bounding box
	double radius_max_box = params.radius_max + params.epsilon;
	box = findBoundingBox2D(seed, length, width, radius_max_box, params.offset);
	for (i = box.i_min; i <= box.i_max; i++) {
		for (j = box.j_min; j <= box.j_max; j++) {
			xd = x_new(i, j, length);
			if (i == box.i_min || i == box.i_max || j == box.j_min || j == box.j_max) {
				foundCirclePtr[xd] = 1.0;
			}
		}
	}

	Point2D draw_circle = { 0.0, 0.0 };

	double d_point = 0.0;
	if (found_center.x != 0 && found_center.y != 0) {

		//save distance between found center en seed point
		Point2D f_point = getRealCoordFromImageCoord2D(found_center, imageDataStr.origin, imageDataStr.spacing, imageDataStr.orientation);
		Point2D s_point = getRealCoordFromImageCoord2D(seed, imageDataStr.origin, imageDataStr.spacing, imageDataStr.orientation);
		d_point = getPoint2DDistance(f_point, s_point);
		
		//add found center
		foundCirclePtr[x_new((size_t)found_center.x, (size_t)found_center.y, length)] = 1.0;
		
		//draw the found circle
		box = findBoundingBox2D(found_center, length, width, found_radius, params.offset);
		
		draw_circle = getRealCoordFromImageCoord2D(found_center, imageDataStr.origin, imageDataStr.spacing, imageDataStr.orientation);

		for (i = box.i_min; i <= box.i_max; i++) {
			for (j = box.j_min; j <= box.j_max; j++) {
				xd = x_new(i, j, length);
				
				current_point.x = (dataType)i;
				current_point.y = (dataType)j;
				current_point = getRealCoordFromImageCoord2D(current_point, imageDataStr.origin, imageDataStr.spacing, imageDataStr.orientation);

				dist_point = getPoint2DDistance(draw_circle, current_point);

				if (dist_point >= (found_radius - params.epsilon) && dist_point <= (found_radius + params.epsilon)) {
					foundCirclePtr[xd] = 1.0;
				}
			}
		}
	}

	//copy back
	//copyDataToAnother2dArray(houghSpaceMax, houghSpacePtr, length, width);
	//max_ratio = getTheMaxValue(houghSpaceMax, length, width);

	free(maskThreshold);
	free(houghSpaceMax);
	free(gradientX);
	free(gradientY);
	free(normGradient);
	free(gradientAngle);
	free(statusPixel);

	return found_center;
}

void houghTransform(Image_Data2D imageDataStr, dataType* foundCirclePtr, HoughParameters parameters) {
	
	const size_t length = imageDataStr.height;
	const size_t width = imageDataStr.width;
	size_t dim2D = length * width;
	
	Point2D center_circle = { 0.0, 0.0 }, current_point = {0.0, 0.0};

	size_t count_neighbor = 0, total_neighbor = 0;
	dataType hough_ratio = 0.0, found_ratio = 0.0, max_ratio = 0.0;
	size_t i_min = 0, i_max = 0, j_min = 0, j_max = 0;
	double dist_point = 0.0;
	BoundingBox2D box = { 0, 0, 0, 0 };
	bool test_max = false;

	dataType* maskThreshold = new dataType[dim2D]{ 0 };
	dataType* houghSpacePtr = new dataType[dim2D]{ 0 };
	dataType* houghSpaceMax = new dataType[dim2D]{ 0 };
	size_t* statusPixel = new size_t[dim2D]{ 0 };

	//Computation of edge image
	Point2D grad;
	for (size_t i = 0; i < length; i++) {
		for (size_t j = 0; j < width; j++) {
			//Compute gradient
			getGradient2D(imageDataStr.imageDataPtr, length, width, i, j, imageDataStr.spacing, &grad);
			//Compute norm of the gradient
			dataType norm_gradient = sqrt(grad.x * grad.x + grad.y * grad.y);
			//Compute the corresponding edge value
			dataType edge_value = gradientFunction(norm_gradient, parameters.K);
			//Threshold of edge detector
			if (edge_value < parameters.thres) {
				maskThreshold[x_new(i, j, length)] = 1.0;
			}
		}
	}

	//const char savingPath[] = "C:/Users/Konan Allaly/Documents/Tests/output/threshold.raw";
	//manageRAWFile2D<dataType>(maskThreshold, length, width, savingPath, STORE_DATA, false);

	for (size_t p = 0; p < length; p++) {
		for (size_t q = 0; q < width; q++) {
			
			//Only background pixel can be center
			//Therefore we test only the background pixels
			size_t count_vote = 0;
			if (maskThreshold[x_new(p, q, length)] == 0.0) {
				
				//initialize2dArray(houghSpaceMax, length, width);
				Point2D test_center = { (dataType)p , (dataType)q };
				Point2D test_center_real_cord = getRealCoordFromImageCoord2D(test_center, imageDataStr.origin, imageDataStr.spacing, imageDataStr.orientation);
				//Point2D found_center = { 0.0, 0.0 };
				//double found_radius = 0.0;

				double radius = parameters.radius_min;
				while (radius <= parameters.radius_max) {
					box = findBoundingBox2D(test_center, length, width, radius, parameters.offset);
					for (size_t i = 0; i < length; i++) {
						for (size_t j = 0; j < width; j++) {
							Point2D current_point = { i, j };
							current_point = getRealCoordFromImageCoord2D(current_point, imageDataStr.origin, imageDataStr.spacing, imageDataStr.orientation);
							double distance_point = getPoint2DDistance(test_center_real_cord, current_point);
							//Count foreground pixels in the band
							if (distance_point != 0 && distance_point >= (radius - parameters.epsilon) && (distance_point <= radius + parameters.epsilon)) {
								if (maskThreshold[x_new(i, j, length)] == 1.0) {
									count_vote++;
								}
							}
						}
					}
					radius += parameters.radius_step;
				}
				
				/*
				while (radius <= parameters.radius_max) {

					//initialize the 2D array
					initialize2dArray(houghSpacePtr, length, width);

					//find the bounding box of the current center
					box = findBoundingBox2D(test_center, length, width, radius, parameters.offset);

					//small bounding box
					i_min = (size_t)(box.i_min + radius + parameters.epsilon);
					if (box.i_max >= radius) {
						i_max = (size_t)(box.i_max - radius - parameters.epsilon);
					}

					j_min = (size_t)(box.j_min + radius + parameters.epsilon);
					if (box.j_max >= radius) {
						j_max = (size_t)(box.j_max - radius - parameters.epsilon);
					}

					//visit all the pixels in the bounding box
					for (size_t i = i_min; i <= i_max; i++) {
						for (size_t j = j_min; j <= j_max; j++) {

							size_t xd = x_new(i, j, length);

							center_circle.x = (dataType)i;
							center_circle.y = (dataType)j;
							getRealCoordFromImageCoord2D(center_circle, originImage, parameters.spacing, orientation);

							//vote
							count_vote = 0;
							count_neighbor = 0;
							total_neighbor = 0;

							if (maskThreshold[x_new(i, j, length)] == background) {
								for (size_t i_new = box.i_min; i_new <= box.i_max; i_new++) {
									for (size_t j_new = box.j_min; j_new <= box.j_max; j_new++) {

										size_t xd_new = x_new(i_new, j_new, length);

										current_point.x = (dataType)i_new;
										current_point.y = (dataType)j_new;

										getRealCoordFromImageCoord2D(current_point, originImage, parameters.spacing, orientation);
										dist_point = getPoint2DDistance(center_circle, current_point);

										//count foreground pixels in the band
										if (dist_point >= (radius - parameters.epsilon) && dist_point <= (radius + parameters.epsilon)) {
											total_neighbor++;
											if (maskThreshold[xd_new] == foreground) {
												count_vote++;
											}
										}
										//count foreground pixels close to the current center
										if (dist_point < (radius - parameters.epsilon) && maskThreshold[xd_new] == foreground) {
											count_neighbor++;
										}

									}
								}
							}

							//compute ratios
							hough_ratio = (dataType)count_vote / (dataType)total_neighbor;
							if (count_neighbor == 0) {
								houghSpacePtr[xd] = hough_ratio;
							}
						}
					}

					//keep the maximal ratio for each pixel
					for (size_t i = box.i_min; i <= box.i_max; i++) {
						for (size_t j = box.j_min; j <= box.j_max; j++) {
							size_t xd = x_new(i, j, length);
							if (houghSpacePtr[xd] > houghSpaceMax[xd]) {
								houghSpaceMax[xd] = houghSpacePtr[xd];
							}
						}
					}

					//find the maximal ratio for current radius
					max_ratio = getTheMaxValue(houghSpacePtr, length, width);
					//find the maximal ratio for whole radius range
					if (max_ratio > found_ratio) {
						found_ratio = max_ratio;
						found_radius = radius;
						test_max = true;
					}
					//update the found_center if the value of found_ratio was updated
					if (test_max == true) {
						found_center = getPointWithMaximalValue2D(houghSpaceMax, length, width);
						test_max = false;
					}
					radius += parameters.radius_step;

				}
				*/

			}
			foundCirclePtr[x_new(p, q, length)] = count_vote;

			//if (found_radius != 0.0) {
			//	printf("Found radius = %lf\n", found_radius);
			//}

			////Draw found circle
			//if (found_radius != 0.0) {
			//	//add found center
			//	foundCirclePtr[x_new((size_t)found_center.x, (size_t)found_center.y, length)] = 1.0;
			//	//bounding box for found circle
			//	box = findBoundingBox2D(found_center, length, width, found_radius, parameters.offset);
			//	Point2D pt_draw = getRealCoordFromImageCoord2D(found_center, originImage, parameters.spacing, orientation);
			//	for (size_t i = box.i_min; i <= box.i_max; i++) {
			//		for (size_t j = box.j_min; j <= box.j_max; j++) {
			//			current_point.x = (dataType)i;
			//			current_point.y = (dataType)j;
			//			current_point = getRealCoordFromImageCoord2D(current_point, originImage, parameters.spacing, orientation);
			//			dist_point = getPoint2DDistance(pt_draw, current_point);
			//			if (dist_point >= (found_radius - parameters.epsilon) && dist_point <= (found_radius + parameters.epsilon)) {
			//				foundCirclePtr[x_new(i, j, length)] = 1.0;
			//			}
			//		}
			//	}
			//}
		}
	}

	const char saving_path[] = "C:/Users/Konan Allaly/Documents/Tests/output/ratio_radius6V2.raw";
	manageRAWFile2D<dataType>(foundCirclePtr, length, width, saving_path, STORE_DATA, false);
	
	delete[] houghSpacePtr;
	delete[] houghSpaceMax;
	delete[] maskThreshold;
	delete[] statusPixel;
}

bool circleDetection(Image_Data2D imageDataPtr, const HoughParameters hParameters) {
	
	if (imageDataPtr.imageDataPtr == NULL)
	{
		return false;
	}

	size_t height = imageDataPtr.height;
	size_t width = imageDataPtr.width;
	size_t dim2D = height * width;
	double radius;
	double offset = 1.0, perimeter, step, phi, distance_between_points = 1.171875;
	size_t indx = 0, indy = 0, number_of_circle_points;

	double radius_range = hParameters.radius_max - hParameters.radius_min;
	size_t number_of_radiuses = (size_t)(radius_range / hParameters.radius_step);
	dataType** accumulation = new dataType * [number_of_radiuses];
	for (size_t k = 0; k < number_of_radiuses; k++) {
		accumulation[k] = new dataType[dim2D]{ 0 };
		if (accumulation[k] == NULL)
			return false;
	}
	if (accumulation == NULL)
		return false;

	//bool* status = new bool[dim2D]{false};
	//initialize2dArrayBool(status, height, width);

	size_t xd = 0;
	double coordx = 0.0, coordy = 0.0;
	dataType accumulation_ratio = 0.0;

	for(size_t i = 0; i < height; i++)
	{
		for (size_t j = 0; j < width; j++) 
		{
			size_t xd = x_new(i, j, height);

			if (imageDataPtr.imageDataPtr[xd] != 1) {
				Point2D point_center = { i, j };
				point_center = getRealCoordFromImageCoord2D(point_center, imageDataPtr.origin, imageDataPtr.spacing, imageDataPtr.orientation);
				for (size_t k = 0; k < number_of_radiuses; k++)
				{
					radius = hParameters.radius_min + k * hParameters.radius_step;
					perimeter = 2 * M_PI * radius;
					number_of_circle_points = (size_t)(perimeter / distance_between_points + 0.5);
					step = 2 * M_PI / (double)number_of_circle_points;

					for (size_t np = 0; np < number_of_circle_points; np++) 
					{

						phi = (double)np * step;

						coordx = 0.5 + point_center.x + radius * cos(phi);
						if (coordx < 0) {
							indx = 0;
						}
						else {
							if (coordx > height - 1) {
								indx = height - 1;
							}
							else {
								indx = (size_t)coordx;
							}
						}

						coordy = 0.5 + point_center.y + radius * sin(phi);
						if (coordy < 0) {
							indy = 0;
						}
						else {
							if (coordy > width - 1) {
								indy = width - 1;
							}
							else {
								indy = (size_t)coordy;
							}
						}

						//TODO : the same point can be found multiple time
						//       it is need to find the condition to skip a point already found

						if (imageDataPtr.imageDataPtr[x_new(indx, indy, height)] == 1.0) {
							accumulation[k][x_new(i, j, height)] += 1;
						}

					}

					//accumulation_ratio = accumulation[k][x_new(i, j, height)] / (dataType)number_of_circle_points;
					//if (accumulation_ratio < 0.5) 
					//{
					//	accumulation[k][x_new(i, j, height)] = 0;
					//}
				}
			}
			
			/*
			Point2D point_center = { i, j };
			for (size_t k = 0; k < number_of_radiuses; k++)
			{
				radius = hParameters.radius_min + k * hParameters.radius_step;
				perimeter = 2 * M_PI * radius;
				number_of_circle_points = (size_t)(perimeter / distance_between_points + 0.5);
				step = 2 * M_PI / (double)number_of_circle_points;

				for (size_t np = 0; np < number_of_circle_points; np++)
				{

					phi = (double)np * step;

					coordx = 0.5 + point_center.x + radius * cos(phi);
					if (coordx < 0) {
						indx = 0;
					}
					else {
						if (coordx > height - 1) {
							indx = height - 1;
						}
						else {
							indx = (size_t)coordx;
						}
					}

					coordy = 0.5 + point_center.y + radius * sin(phi);
					if (coordy < 0) {
						indy = 0;
					}
					else {
						if (coordy > width - 1) {
							indy = width - 1;
						}
						else {
							indy = (size_t)coordy;
						}
					}

					//TODO : the same point can be found several time
					//       it is need to find the condition to skip a point already found

					if (imageDataPtr.imageDataPtr[x_new(indx, indy, height)] == 1.0) {
						accumulation[k][x_new(i, j, height)] += 1;
					}

				}

				//accumulation_ratio = accumulation[k][x_new(i, j, height)] / (dataType)number_of_circle_points;
				//if (accumulation_ratio < 0.5) 
				//{
				//	accumulation[k][x_new(i, j, height)] = 0;
				//}
			}
			*/
		}
	}

	std::string outputPath = "C:/Users/Konan Allaly/Documents/Tests/output/";
	std::string storing_path = outputPath + "accumulation.raw";
	store2dRawData<dataType>(accumulation[1], height, width, storing_path.c_str());

	////Find the maximal accumulation
	//dataType maxAccum = 0;
	//double radius_max = 0.0;
	//Point2D center_max = { 0.0, 0.0 };
	//for (size_t k = 0; k < number_of_radiuses; k++) {
	//	for (size_t i = 0; i < height; i++) {
	//		for (size_t j = 0; j < width; j++) {
	//			size_t xd = x_new(i, j, height);
	//			if (accumulation[k][xd] > maxAccum) {
	//				maxAccum = accumulation[k][xd];
	//				center_max.x = i;
	//				center_max.y = j;
	//				radius_max = hParameters.radius_min + k * hParameters.radius_step;
	//			}
	//		}
	//	}
	//}
	
	//std::cout << "The maximal accumulation is : " << maxAccum << std::endl;
	//std::cout << "The point with max vote is : (" << center_max.x << "," << center_max.y << ")" << std::endl;
	//std::cout << "The radius of found circle is : " << radius_max << std::endl;

	for (size_t k = 0; k < number_of_radiuses; k++) {
		delete[] accumulation[k];
	}
	delete[] accumulation;

	return true;
}

Point2D localCircleDetection(Image_Data2D imageDataPtr, dataType* foundCirclePtr, Point2D seed, const HoughParameters hParameters) {

	//if (imageDataPtr.imageDataPtr == NULL)
	//{
	//	return;
	//}

	size_t height = imageDataPtr.height;
	size_t width = imageDataPtr.width;
	size_t dim2D = height * width;
	double radius;
	double offset = 1.0, perimeter, step, phi, distance_between_points = 1.0;
	size_t indx = 0, indy = 0, number_of_circle_points;

	double radius_range = hParameters.radius_max - hParameters.radius_min;
	size_t number_of_radiuses = (size_t)(radius_range / hParameters.radius_step);
	dataType** accumulation = new dataType * [number_of_radiuses];
	for (size_t k = 0; k < number_of_radiuses; k++) {
		accumulation[k] = new dataType[dim2D]{ 0 };
	}

	size_t xd = 0;
	double coordx = 0.0, coordy = 0.0;
	dataType accumulation_ratio = 0.0;
	size_t count_neight_center = 0;
	size_t number_of_potential_center = 0;

	//Restrict the search in small region of the image given by the offset 
	BoundingBox2D box = findBoundingBox2D(seed, height, width, hParameters.radius_max, hParameters.offset);

	//show the seed point
	foundCirclePtr[x_new((size_t)seed.x, (size_t)seed.y, height)] = 1.0;

	for (size_t i = box.i_min; i <= box.i_max; i++)
	{
		for (size_t j = box.j_min; j <= box.j_max; j++)
		{
			size_t xd = x_new(i, j, height);

			//Show the search box
			if (i == box.i_min || i == box.i_max || j == box.j_min || j == box.j_max) {
				foundCirclePtr[xd] = 1.0;
			}
		
			//The potential center can not be foreground pixel
			if (imageDataPtr.imageDataPtr[xd] != 1.0) {

				Point2D point_center = { i, j };

				for (size_t k = 0; k < number_of_radiuses; k++)
				{

					//Compute the total number of points on given circle 
					radius = hParameters.radius_min + k * hParameters.radius_step;

					//Check current center neighborhood
					BoundingBox2D box_neighbor = findBoundingBox2D(point_center, height, width, radius, 0.5);
					count_neight_center = 0;
					for (size_t i_n = box_neighbor.i_min; i_n <= box_neighbor.i_max; i_n++) {
						for (size_t j_n = box_neighbor.j_min; j_n <= box_neighbor.j_max; j_n++) {
							Point2D center_real = getRealCoordFromImageCoord2D(point_center, imageDataPtr.origin, imageDataPtr.spacing, imageDataPtr.orientation);
							Point2D current_p = { i_n, j_n };
							current_p = getRealCoordFromImageCoord2D(current_p, imageDataPtr.origin, imageDataPtr.spacing, imageDataPtr.orientation);
							double d_n = getPoint2DDistance(center_real, current_p);
							if (d_n < radius && imageDataPtr.imageDataPtr[x_new(i_n, j_n, height)] == 1.0) {
								count_neight_center++;
							}
						}
					}
					
					if (count_neight_center == 0) {
						
						perimeter = 2 * M_PI * radius;
						number_of_circle_points = (size_t)(perimeter / distance_between_points + 0.5);
						step = 2 * M_PI / (double)number_of_circle_points;

						for (size_t np = 0; np < number_of_circle_points; np++)
						{

							phi = (double)np * step;

							coordx = 0.5 + point_center.x + radius * cos(phi);
							if (coordx < 0) {
								indx = 0;
							}
							else {
								if (coordx > height - 1) {
									indx = height - 1;
								}
								else {
									indx = (size_t)coordx;
								}
							}

							coordy = 0.5 + point_center.y + radius * sin(phi);
							if (coordy < 0) {
								indy = 0;
							}
							else {
								if (coordy > width - 1) {
									indy = width - 1;
								}
								else {
									indy = (size_t)coordy;
								}
							}

							//TODO : the same point can be found multiple time
							//       it is need to find the condition to skip a point already found

							if (imageDataPtr.imageDataPtr[x_new(indx, indy, height)] == 1.0) {
								accumulation[k][xd] += 1;
							}

						}

						accumulation_ratio = accumulation[k][xd] / (dataType)number_of_circle_points;
						if (accumulation_ratio < 0.5)
						{
							//set the accumulation for given radius to 0 if there are few points belonging to the circle
							accumulation[k][xd] = 0;
						}
						
					}
					else {
						//set the accumulation for given radius to 0 if there are foreground pixels inside the circle
						accumulation[k][xd] = 0;
					}

				}

			}
		}
	}

	//std::string outputPath = "C:/Users/Konan Allaly/Documents/Tests/output/";
	//std::string storing_path = outputPath + "accumulation4.raw";
	//store2dRawData<dataType>(accumulation[4], height, width, storing_path.c_str());

	//Find the maximal accumulation
	dataType maxAccum = 0;
	double radius_max = 0.0;
	Point2D center_max = { 0.0, 0.0 };
	for (size_t k = 0; k < number_of_radiuses; k++) {
		for (size_t i = 0; i < height; i++) {
			for (size_t j = 0; j < width; j++) {
				size_t xd = x_new(i, j, height);
				if (accumulation[k][xd] > maxAccum) {
					maxAccum = accumulation[k][xd];
					center_max.x = i;
					center_max.y = j;
					radius_max = hParameters.radius_min + k * hParameters.radius_step;
				}
			}
		}
	}

	//std::cout << "The maximal accumulation is : " << maxAccum << std::endl;
	//std::cout << "The point with max vote is : (" << center_max.x << "," << center_max.y << ")" << std::endl;
	std::cout << "The radius of found circle is : " << radius_max << std::endl;
	//std::cout << "The found center is : (" << center_max.x << ","  << center_max.y << ")" << std::endl;

	//Point2D found_center = getPointWithMaximalValue2D(accumulation[4], height, width);

	if (center_max.x != 0 && center_max.y != 0) {
		//add found center
		foundCirclePtr[x_new((size_t)center_max.x, (size_t)center_max.y, height)] = 1.0;
		Point2D draw_point = getRealCoordFromImageCoord2D(center_max, imageDataPtr.origin, imageDataPtr.spacing, imageDataPtr.orientation);

		//draw the found circle
		box = findBoundingBox2D(center_max, height, width, radius_max, hParameters.offset);
		for (size_t i = box.i_min; i <= box.i_max; i++) {
			for (size_t j = box.j_min; j <= box.j_max; j++) {
				size_t xd = x_new(i, j, height);
				Point2D current_point = { (dataType)i , (dataType)j };
				current_point = getRealCoordFromImageCoord2D(current_point, imageDataPtr.origin, imageDataPtr.spacing, imageDataPtr.orientation);
				double dist_point = getPoint2DDistance(draw_point, current_point);
				//Draw disque
				if (dist_point <= (radius_max + hParameters.epsilon)) {
					foundCirclePtr[xd] = 1.0;
				}
			}
		}

	}

	//std::string outputPath = "C:/Users/Konan Allaly/Documents/Tests/output/found_circle_new.raw";
	//manageRAWFile2D<dataType>(foundCirclePtr, height, width, outputPath.c_str(), STORE_DATA, false);

	for (size_t k = 0; k < number_of_radiuses; k++) {
		delete[] accumulation[k];
	}
	delete[] accumulation;

	return center_max;
}

Point2D get2dImagecentroid(dataType* imageDataPtr, size_t length, size_t width, dataType imageBackground)
{
	size_t counts = 0;
	dataType x = 0.0, y = 0.0, z = 0.0;
	for (size_t i = 0; i < length; i++)
	{
		for (size_t j = 0; j < width; j++)
			if (imageDataPtr[x_new(i, j, length)] != imageBackground)
			{
				x += i;
				y += j;
				counts++;
			}
	}
	
	// Check K incase no 0 was found - not a shape
	if (counts == 0)
	{
		counts = 1;
	}
	// Set the Centers of the shape
	Point2D imageCentroid = { x / counts, y / counts };
	
	return imageCentroid;
}