#include "itkGDCMImageIO.h"
#include "itkNumericSeriesFileNames.h"
#include "itkImageFileReader.h"
#include "itkImageSeriesWriter.h"
#include "itkMetaDataObject.h"
#include <vector>
#include "itksys/SystemTools.hxx"
#include "header.h"

int
main(char * input, char * output){
 if (argc < 3){
    std::cerr << "Usage: " << argv[0];
    std::cerr << " InputImage  OutputDicomDirectory" << std::endl;
    return EXIT_FAILURE;
  }
  
  using PixelType = signed short;
  constexpr unsigned int Dimension = 3;
  using ImageType = itk::Image<PixelType, Dimension>;
  using ReaderType = itk::ImageFileReader<ImageType>;
  
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(input);
  try {
   reader->Update();
  }
  catch (const itk::ExceptionObject & excp){
    std::cerr << "Exception thrown while writing the image" << std::endl;
    std::cerr << excp << std::endl;
    return EXIT_FAILURE;
  }
  
  using ImageIOType = itk::GDCMImageIO;
  using NamesGeneratorType = itk::NumericSeriesFileNames;
  
  ImageIOType::Pointer gdcmIO = ImageIOType::New();
  const char * outputDirectory = output;
  itksys::SystemTools::MakeDirectory(outputDirectory);
  using OutputPixelType = signed short;
  constexpr unsigned int OutputDimension = 2;
  
  using Image2DType = itk::Image<OutputPixelType, OutputDimension>;
  using SeriesWriterType = itk::ImageSeriesWriter<ImageType, Image2DType>;
  
  NamesGeneratorType::Pointer namesGenerator = NamesGeneratorType::New();
  itk::MetaDataDictionary & dict = gdcmIO->GetMetaDataDictionary();
  std::string tagkey, value;
  tagkey = "0008|0060"; 
  value = "MR";
  itk::EncapsulateMetaData<std::string>(dict, tagkey, value);
  tagkey = "0008|0008"; 
  value = "DERIVED\\SECONDARY";
  itk::EncapsulateMetaData<std::string>(dict, tagkey, value);
  tagkey = "0008|0064"; 
  value = "DV";
  itk::EncapsulateMetaData<std::string>(dict, tagkey, value);
  SeriesWriterType::Pointer seriesWriter = SeriesWriterType::New();
  
  seriesWriter->SetInput(reader->GetOutput());
  seriesWriter->SetImageIO(gdcmIO);
  
  ImageType::RegionType region = reader->GetOutput()->GetLargestPossibleRegion();
  ImageType::IndexType start = region.GetIndex();
  ImageType::SizeType  size = region.GetSize();
  
  std::string format = outputDirectory;
 
  format += "/image%03d.dcm";
  namesGenerator->SetSeriesFormat(format.c_str());
  namesGenerator->SetStartIndex(start[2]);
  namesGenerator->SetEndIndex(start[2] + size[2] - 1);
  namesGenerator->SetIncrementIndex(1);
  seriesWriter->SetFileNames(namesGenerator->GetFileNames());
  try{
    seriesWriter->Update();
  }
  catch (const itk::ExceptionObject & excp){
    std::cerr << "Exception thrown while writing the series " << std::endl;
    std::cerr << excp << std::endl;
    return EXIT_FAILURE;
  }
 
  return EXIT_SUCCESS;
}
