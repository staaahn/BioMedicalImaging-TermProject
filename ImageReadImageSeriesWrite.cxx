
 
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageSeriesWriter.h"
#include "itkNumericSeriesFileNames.h"
 
int
main(int argc, char * argv[])
{
  if (argc < 4)
  {
    std::cerr
      << "Usage: ImageReadImageSeriesWrite inputFile outputPrefix outputExtension"
      << std::endl;
    return EXIT_FAILURE;
  }
 

  using ImageType = itk::Image<unsigned char, 3>;
  using ReaderType = itk::ImageFileReader<ImageType>;
  
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(argv[1]);
  
  using Image2DType = itk::Image<unsigned char, 2>;
 
  using WriterType = itk::ImageSeriesWriter<ImageType, Image2DType>;
 
  WriterType::Pointer writer = WriterType::New();
 
  writer->SetInput(reader->GetOutput());
  
  using NameGeneratorType = itk::NumericSeriesFileNames;
 
  NameGeneratorType::Pointer nameGenerator = NameGeneratorType::New();

  std::string format = argv[2];
  format += "%03d.";
  format += argv[3]; // filename extension
 
  nameGenerator->SetSeriesFormat(format.c_str());
  
  try
  {
    reader->Update();
  }
  catch (const itk::ExceptionObject & excp)
  {
    std::cerr << "Exception thrown while reading the image" << std::endl;
    std::cerr << excp << std::endl;
  }
  // Software Guide : EndCodeSnippet
 
 
  ImageType::ConstPointer inputImage = reader->GetOutput();
  ImageType::RegionType   region = inputImage->GetLargestPossibleRegion();
  ImageType::IndexType    start = region.GetIndex();
  ImageType::SizeType     size = region.GetSize();
  
  const unsigned int firstSlice = start[2];
  const unsigned int lastSlice = start[2] + size[2] - 1;
 
  nameGenerator->SetStartIndex(firstSlice);
  nameGenerator->SetEndIndex(lastSlice);
  nameGenerator->SetIncrementIndex(1);
  
  writer->SetFileNames(nameGenerator->GetFileNames());
 
  try
  {
    writer->Update();
  }
  catch (const itk::ExceptionObject & excp)
  {
    std::cerr << "Exception thrown while reading the image" << std::endl;
    std::cerr << excp << std::endl;
  }
  
  return EXIT_SUCCESS;
}
