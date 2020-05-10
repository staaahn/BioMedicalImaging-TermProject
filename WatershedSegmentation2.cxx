#include "itkWatershedImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkScalarToRGBPixelFunctor.h"
#include "itkUnaryFunctorImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"

using namespace std;

int main(int argc, char * argv[]) {
  if (argc < 5) {
    cerr << "Missing Parameters " << endl;
    cerr << "Usage: " << argv[0];
    cerr << " inputImage  outputImage lowerThreshold  outputScaleLevel" << endl;
    return 0;
  }
 
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
 
  reader->SetFileName(argv[1]);
  writer->SetFileName(argv[2]);
 
  //Instantiate the GradientMagnitude image filter
  using GradientMagnitudeFilterType = itk::GradientMagnitudeRecursiveGaussianImageFilter<InternalImageType, InternalImageType>;
  GradientMagnitudeFilterType::Pointer gradienMagnitudeFilter = GradientMagnitudeFilterType::New();
  gradienMagnitudeFilter->SetInput(reader->GetOutput());
  gradienMagnitudeFilter->SetSigma(1.0);

  //Instantiate the Watershed filter
  using WatershedFilterType = itk::WatershedImageFilter<InternalImageType>;
  WatershedFilterType::Pointer watershedFilter = WatershedFilterType::New();
  watershedFilter->SetInput(gradienMagnitudeFilter->GetOutput());
  watershedFilter->SetThreshold(std::stod(argv[3]));
  watershedFilter->SetLevel(std::stod(argv[4]));
 

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
    std::cerr << "Exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return 0;
  }
  return 1;
}
