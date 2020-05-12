
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include "itkDemonsRegistrationFilter.h"
#include "itkHistogramMatchingImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkWarpImageFilter.h"
#include "header.h"

  
class CommandIterationUpdate : public itk::Command
  {
  public:
    typedef  CommandIterationUpdate Self;
    typedef  itk::Command Superclass;
    typedef  itk::SmartPointer<CommandIterationUpdate> Pointer;
    
    itkNewMacro( CommandIterationUpdate );
  
  protected:
    CommandIterationUpdate() {};
    
    typedef itk::Image<float, 2> InternalImageType;
    typedef itk::Vector<float, 2> VectorPixelType;
    typedef itk::Image<VectorPixelType, 2> DisplacementFieldType;
    typedef itk::DemonsRegistrationFilter<InternalImageType,InternalImageType,DisplacementFieldType> RegistrationFilterType;
 
 public:
    void Execute(itk::Object *caller, const itk::EventObject & event) ITK_OVERRIDE {
        Execute((const itk::Object *)caller, event);
      }
    void Execute(const itk::Object * object, const itk::EventObject & event) ITK_OVERRIDE {
         const RegistrationFilterType * filter = static_cast< const RegistrationFilterType * >(object);
        if(!(itk::IterationEvent().CheckEvent(&event))){
          return;
          }
        std::cout << filter->GetMetric() << std::endl;
      }
  };
int main(int argc, char *argv[]){
  if(argc < 4){
    std::cerr << "Missing Parameters " << std::endl;
    std::cerr << "Usage: " << argv[0];
    std::cerr << " fixedImageFile movingImageFile ";
    std::cerr << " outputImageFile " << std::endl;
    std::cerr << " [outputDisplacementFieldFile] " << std::endl;
    return EXIT_FAILURE;
    }
  
  const unsigned int Dimension = 2;
  typedef unsigned short PixelType;
  typedef itk::Image< PixelType, Dimension > FixedImageType;
  typedef itk::Image< PixelType, Dimension > MovingImageType;
  typedef itk::ImageFileReader< FixedImageType > FixedImageReaderType;
  typedef itk::ImageFileReader< MovingImageType > MovingImageReaderType;
  
  FixedImageReaderType::Pointer fixedImageReader = FixedImageReaderType::New();
  MovingImageReaderType::Pointer movingImageReader = MovingImageReaderType::New();
  
  fixedImageReader->SetFileName(argv[1]);
  movingImageReader->SetFileName(argv[2]);
 
  typedef float InternalPixelType;
  typedef itk::Image< InternalPixelType, Dimension > InternalImageType;
  typedef itk::CastImageFilter< FixedImageType,InternalImageType > FixedImageCasterType;
  typedef itk::CastImageFilter< MovingImageType,InternalImageType > MovingImageCasterType;
  
  FixedImageCasterType::Pointer fixedImageCaster = FixedImageCasterType::New();
  MovingImageCasterType::Pointer movingImageCaster= MovingImageCasterType::New();
  fixedImageCaster->SetInput( fixedImageReader->GetOutput() );
  movingImageCaster->SetInput( movingImageReader->GetOutput() );
  
  typedef itk::HistogramMatchingImageFilter<InternalImageType, InternalImageType> MatchingFilterType;
  MatchingFilterType::Pointer matcher = MatchingFilterType::New();
  
  matcher->SetInput(movingImageCaster->GetOutput());
  matcher->SetReferenceImage(fixedImageCaster->GetOutput());
  matcher->SetNumberOfHistogramLevels(1024);
  matcher->SetNumberOfMatchPoints(7);
  matcher->ThresholdAtMeanIntensityOn();
  
  typedef itk::Vector< float, Dimension > VectorPixelType;
  typedef itk::Image< VectorPixelType, Dimension > DisplacementFieldType;
  typedef itk::DemonsRegistrationFilter<InternalImageType,InternalImageType,DisplacementFieldType> RegistrationFilterType;
  
  RegistrationFilterType::Pointer filter = RegistrationFilterType::New();
  CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
  
  filter->AddObserver(itk::IterationEvent(), observer);
  filter->SetFixedImage(fixedImageCaster->GetOutput());
  filter->SetMovingImage(matcher->GetOutput());
  filter->SetNumberOfIterations(50);
  filter->SetStandardDeviations(1.0);
  filter->Update();
  
  typedef itk::WarpImageFilter<MovingImageType,MovingImageType,DisplacementFieldType> WarperType;
  typedef itk::LinearInterpolateImageFunction<MovingImageType,double> InterpolatorType;
  
  WarperType::Pointer warper = WarperType::New();
  InterpolatorType::Pointer interpolator = InterpolatorType::New();
  FixedImageType::Pointer fixedImage = fixedImageReader->GetOutput();
  
  warper->SetInput(movingImageReader->GetOutput());
  warper->SetInterpolator(interpolator);
  warper->SetOutputSpacing(fixedImage->GetSpacing());
  warper->SetOutputOrigin(fixedImage->GetOrigin());
  warper->SetOutputDirection(fixedImage->GetDirection());
  warper->SetDisplacementField(filter->GetOutput());
  
  typedef  unsigned char OutputPixelType;
  typedef itk::Image< OutputPixelType, Dimension > OutputImageType;
  typedef itk::CastImageFilter< MovingImageType,OutputImageType > CastFilterType;
  typedef itk::ImageFileWriter< OutputImageType >  WriterType;
  
  WriterType::Pointer writer = WriterType::New();
  CastFilterType::Pointer caster = CastFilterType::New();
  
  writer->SetFileName(argv[3]);
  caster->SetInput(warper->GetOutput());
  writer->SetInput(caster->GetOutput());
  writer->Update();
 
  if(argc > 4) {
  typedef itk::ImageFileWriter< DisplacementFieldType > FieldWriterType;
  FieldWriterType::Pointer fieldWriter = FieldWriterType::New();
  fieldWriter->SetFileName(argv[4]);
  fieldWriter->SetInput(filter->GetOutput());
  fieldWriter->Update();
   }
  if(argc > 5) {
  typedef DisplacementFieldType VectorImage2DType;
  typedef DisplacementFieldType::PixelType Vector2DType;
  
  VectorImage2DType::ConstPointer vectorImage2D = filter->GetOutput();
  VectorImage2DType::RegionType region2D = vectorImage2D->GetBufferedRegion();
  VectorImage2DType::IndexType index2D  = region2D.GetIndex();
  VectorImage2DType::SizeType size2D   = region2D.GetSize();
  
  typedef itk::Vector< float,3 >  Vector3DType;
  typedef itk::Image< Vector3DType, 3 >  VectorImage3DType;
  typedef itk::ImageFileWriter< VectorImage3DType > VectorImage3DWriterType;
  
  VectorImage3DWriterType::Pointer writer3D = VectorImage3DWriterType::New();
  VectorImage3DType::Pointer vectorImage3D = VectorImage3DType::New();
  VectorImage3DType::RegionType region3D;
  VectorImage3DType::IndexType index3D;
  VectorImage3DType::SizeType size3D;
  
  index3D[0] = index2D[0];
  index3D[1] = index2D[1];
  index3D[2] = 0;
  size3D[0]  = size2D[0];
  size3D[1]  = size2D[1];
  size3D[2]  = 1;
  region3D.SetSize(size3D);
  region3D.SetIndex(index3D);
  vectorImage3D->SetRegions(region3D);
  vectorImage3D->Allocate();
  
  typedef itk::ImageRegionConstIterator< VectorImage2DType > Iterator2DType;
  typedef itk::ImageRegionIterator< VectorImage3DType > Iterator3DType;
  
  Iterator2DType it2(vectorImage2D,region2D);
  Iterator3DType it3(vectorImage3D,region3D);
  it2.GoToBegin();
  it3.GoToBegin();
  Vector2DType vector2D;
  Vector3DType vector3D;
  vector3D[2] = 0; 
  while(!it2.IsAtEnd()){
    vector2D = it2.Get();
    vector3D[0] = vector2D[0];
    vector3D[1] = vector2D[1];
    it3.Set(vector3D);
    ++it2;
    ++it3;
    }
  writer3D->SetInput(vectorImage3D);
  writer3D->SetFileName(argv[5]);
  try{
    writer3D->Update();
    }
  catch(itk::ExceptionObject & excp){
    std::cerr << excp << std::endl;
    return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}
