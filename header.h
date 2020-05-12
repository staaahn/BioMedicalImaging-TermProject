#include <stdio.h>
#include <string.h>
#include "WatershedSegmentation2.cxx"
#include "BSplineRegistration.cxx"
using namespace std;

void WatershedSegmentation2(const char* input_image, const char* output_image, const char* lower_threshold, const char* output_scalelevel );
void BSplineRegistration(const char* fixedImageFile, const char* movingImageFile, const char* outputImageFile);
