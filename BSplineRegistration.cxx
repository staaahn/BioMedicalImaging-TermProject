#include "itkImageRegistrationMethod.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkTimeProbesCollectorBase.h"
#include "itkMemoryProbesCollectorBase.h"
#include "itkBSplineTransform.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkSquaredDifferenceImageFilter.h"
#include "itkTransformFileReader.h"
#include "header.h"
#include "itkCommand.h"

class CommandIterationUpdate : public itk::Command {
public:
  typedef  CommandIterationUpdate   Self;
  typedef  itk::Command             Superclass;
  typedef itk::SmartPointer<Self>   Pointer;
  itkNewMacro(Self);

protected:
  CommandIterationUpdate() {};

public:
  typedef itk::RegularStepGradientDescentOptimizer OptimizerType;
  typedef   const OptimizerType *                  OptimizerPointer;

  void Execute(itk::Object *caller, const itk::EventObject & event) ITK_OVERRIDE {
    Execute( (const itk::Object *)caller, event);
    }

  void Execute(const itk::Object * object, const itk::EventObject & event) ITK_OVERRIDE {
    OptimizerPointer optimizer = static_cast< OptimizerPointer >(object);
    std::cout << optimizer->GetCurrentIteration() << "   ";
    std::cout << optimizer->GetValue() << "   ";
    std::cout << std::endl;
    }
};

void BSplineRegistration( const char* fixedImageFile, const char* movingImageFile, const char* outputImageFile ){
  const    unsigned int    ImageDimension = 3;
  typedef  signed short    PixelType;

  typedef itk::Image< PixelType, ImageDimension >  FixedImageType;
  typedef itk::Image< PixelType, ImageDimension >  MovingImageType;

  const unsigned int SpaceDimension = ImageDimension;
  const unsigned int SplineOrder = 3;
  typedef double CoordinateRepType;

  typedef itk::BSplineTransform<CoordinateRepType, SpaceDimension, SplineOrder> TransformType;

  typedef itk::RegularStepGradientDescentOptimizer OptimizerType;

  typedef itk::MattesMutualInformationImageToImageMetric<FixedImageType, MovingImageType > MetricType;

  typedef itk:: LinearInterpolateImageFunction<MovingImageType, double> InterpolatorType;

  typedef itk::ImageRegistrationMethod<FixedImageType, MovingImageType> RegistrationType;

  MetricType::Pointer metric = MetricType::New();
  OptimizerType::Pointer optimizer = OptimizerType::New();
  InterpolatorType::Pointer interpolator = InterpolatorType::New();
  RegistrationType::Pointer registration = RegistrationType::New();

  registration->SetMetric(metric);
  registration->SetOptimizer(optimizer);
  registration->SetInterpolator(interpolator);

  TransformType::Pointer transform = TransformType::New();
  registration->SetTransform(transform);

  typedef itk::ImageFileReader<FixedImageType> FixedImageReaderType;
  typedef itk::ImageFileReader<MovingImageType> MovingImageReaderType;

  FixedImageReaderType::Pointer fixedImageReader  = FixedImageReaderType::New();
  MovingImageReaderType::Pointer movingImageReader = MovingImageReaderType::New();

  fixedImageReader->SetFileName(fixedImageFile);
  movingImageReader->SetFileName(movingImageFile);

  FixedImageType::ConstPointer fixedImage = fixedImageReader->GetOutput();

  registration->SetFixedImage(fixedImage);
  registration->SetMovingImage(movingImageReader->GetOutput());

  fixedImageReader->Update();

  FixedImageType::RegionType fixedRegion = fixedImage->GetBufferedRegion();

  registration->SetFixedImageRegion(fixedRegion);

  unsigned int numberOfGridNodesInOneDimension = 5;

  
  TransformType::PhysicalDimensionsType   fixedPhysicalDimensions;
  TransformType::MeshSizeType meshSize;
  TransformType::OriginType fixedOrigin;

  for(unsigned int i=0; i< SpaceDimension; i++) {
    fixedOrigin[i] = fixedImage->GetOrigin()[i];
    fixedPhysicalDimensions[i] = fixedImage->GetSpacing()[i] * static_cast<double>(fixedImage->GetLargestPossibleRegion().GetSize()[i] - 1 );
    }
  meshSize.Fill(numberOfGridNodesInOneDimension - SplineOrder);

  transform->SetTransformDomainOrigin( fixedOrigin );
  transform->SetTransformDomainPhysicalDimensions(fixedPhysicalDimensions);
  transform->SetTransformDomainMeshSize(meshSize);
  transform->SetTransformDomainDirection(fixedImage->GetDirection());

  typedef TransformType::ParametersType ParametersType;

  const unsigned int numberOfParameters = transform->GetNumberOfParameters();
  ParametersType parameters(numberOfParameters);
  parameters.Fill( 0.0 );
  transform->SetParameters( parameters );
  registration->SetInitialTransformParameters( transform->GetParameters() );
  parameters.Fill( 0.0 );
  transform->SetParameters( parameters );
  registration->SetInitialTransformParameters( transform->GetParameters() );
  optimizer->SetMaximumStepLength( 10.0 );
  optimizer->SetMinimumStepLength(  0.01 );
  optimizer->SetRelaxationFactor( 0.7 );
  optimizer->SetNumberOfIterations( 50 );
  CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
  optimizer->AddObserver( itk::IterationEvent(), observer );
  metric->SetNumberOfHistogramBins( 50 );
  const unsigned int numberOfSamples = static_cast<unsigned int>( fixedRegion.GetNumberOfPixels() * 20.0 / 100.0 );
  metric->SetNumberOfSpatialSamples( numberOfSamples );
  metric->ReinitializeSeed( 76926294 );

  itk::TimeProbesCollectorBase chronometer;
  itk::MemoryProbesCollectorBase memorymeter;

  std::cout << std::endl << "Starting Registration" << std::endl;

  try {
    memorymeter.Start( "Registration" );
    chronometer.Start( "Registration" );

    registration->Update();

    chronometer.Stop( "Registration" );
    memorymeter.Stop( "Registration" );

    std::cout << "Optimizer stop condition = "
              << registration->GetOptimizer()->GetStopConditionDescription()
              << std::endl;
    }
  catch( itk::ExceptionObject & err ) {
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
    }

  OptimizerType::ParametersType finalParameters = registration->GetLastTransformParameters();

  chronometer.Report( std::cout );
  memorymeter.Report( std::cout );

  transform->SetParameters( finalParameters );

  typedef itk::ResampleImageFilter<MovingImageType,FixedImageType > ResampleFilterType;

  ResampleFilterType::Pointer resample = ResampleFilterType::New();

  resample->SetTransform( transform );
  resample->SetInput( movingImageReader->GetOutput() );

  resample->SetSize(fixedImage->GetLargestPossibleRegion().GetSize());
  resample->SetOutputOrigin(fixedImage->GetOrigin());
  resample->SetOutputSpacing(fixedImage->GetSpacing());
  resample->SetOutputDirection(fixedImage->GetDirection());

  
  resample->SetDefaultPixelValue(0);
  typedef  signed short  OutputPixelType;
  typedef itk::Image< OutputPixelType, ImageDimension > OutputImageType;
  typedef itk::CastImageFilter<FixedImageType,OutputImageType > CastFilterType;

  typedef itk::ImageFileWriter<OutputImageType> WriterType;

  WriterType::Pointer writer =  WriterType::New();
  CastFilterType::Pointer caster =  CastFilterType::New();

  writer->SetFileName(outputImageFile);

  caster->SetInput(resample->GetOutput());
  writer->SetInput(caster->GetOutput());

  try { writer->Update(); }
  catch( itk::ExceptionObject & err ) {
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
    }

  typedef itk::SquaredDifferenceImageFilter<FixedImageType, FixedImageType, OutputImageType> DifferenceFilterType;

  DifferenceFilterType::Pointer difference = DifferenceFilterType::New();

  WriterType::Pointer writer2 = WriterType::New();
  writer2->SetInput( difference->GetOutput() );
}
