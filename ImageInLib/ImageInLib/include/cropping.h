#pragma once

#include "common_functions.h"

typedef struct {
	Point3D origin;
	VoxelSpacing spacing;
	size_t dimension[3];
}imageMetaData;

/// <summary>
/// This function find automatically the bounding box of input binary image
/// </summary>
/// <param name="ctImageData"> : structure holding the image to be cropped</param>
/// <param name="offset"> : offset</param>
/// <returns> : the meta data of the image cropped image</returns>
imageMetaData croppImage3D(Image_Data ctImageData, const size_t offset);
