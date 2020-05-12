#include <stdio.h>
#include <string.h>
using namespace std;

int WatershedSegmentation2(const char* input_image, const char* output_image, const char* lower_threshold, const char* output_scalelevel );
void BSplineRegistration(const char* fixedImageFile, const char* movingImageFile, const char* outputImageFile);
void ImageReadDicomSeriesWrite(const char * input, const char * output);
void ImageReadImageSeriesWrite(const char * input, const char * output, const char * extension);
