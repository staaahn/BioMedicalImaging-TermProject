#include "itkWatershedImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkScalarToRGBPixelFunctor.h"
#include "itkUnaryFunctorImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"

using namespace std;

int main(char* input, char* output, char* l_Threshold, char* scale) {
 
  using InternalPixelType = float;
  using RGBPixelType = itk::RGBPixel<unsigned char>;
  constexpr unsigned int Dimension = 3;
  using InternalImageType = itk::Image<InternalPixelType, Dimension>;
  using RGBImageType = itk::Image<RGBPixelType, Dimension>;
 
 
  // Instantiate reader and writer types
  using ReaderType = itk::ImageFileReader<InternalImageType>;
  using WriterType = itk::ImageFileWriter<RGBImageType>;
 
  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();
 
  reader->SetFileName(input);
  writer->SetFileName(output);
 
  //Instantiate the GradientMagnitude image filter
  using GradientMagnitudeFilterType = itk::GradientMagnitudeRecursiveGaussianImageFilter<InternalImageType, InternalImageType>;
  GradientMagnitudeFilterType::Pointer gradienMagnitudeFilter = GradientMagnitudeFilterType::New();
  gradienMagnitudeFilter->SetInput(reader->GetOutput());
  gradienMagnitudeFilter->SetSigma(1.0);

  //Instantiate the Watershed filter
  using WatershedFilterType = itk::WatershedImageFilter<InternalImageType>;
  WatershedFilterType::Pointer watershedFilter = WatershedFilterType::New();
  watershedFilter->SetInput(gradienMagnitudeFilter->GetOutput());
  watershedFilter->SetThreshold(std::stod(l_Threshold));
  watershedFilter->SetLevel(std::stod(scale));
 

  //  Instantiate the filter that will encode the label image into a color image (random color attribution).
  using ColormapFunctorType = itk::Functor::ScalarToRGBPixelFunctor<unsigned long>;
  using LabeledImageType = WatershedFilterType::OutputImageType;
  using ColormapFilterType = itk::UnaryFunctorImageFilter<LabeledImageType, RGBImageType, ColormapFunctorType>;
  ColormapFilterType::Pointer colorMapFilter = ColormapFilterType::New();
  colorMapFilter->SetInput(watershedFilter->GetOutput());
  writer->SetInput(colorMapFilter->GetOutput());
 
  try {
    writer->Update(); }
  catch (const itk::ExceptionObject & excep) {
    cerr << "Exception caught !" << endl;
    cerr << excep << endl;
    return 0;
  }
  return 1;
}
