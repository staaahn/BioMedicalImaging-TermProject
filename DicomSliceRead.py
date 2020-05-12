import itk
   26 import sys
   27  
   28 if len(sys.argv) < 3:
   29     print('Usage: ' + sys.argv[0] + ' inputFile.dcm outputFile.png')
   30     sys.exit(1)
   31  
   32
   36 InputImageType  = itk.Image.SS2
   37 OutputImageType = itk.Image.UC2
   38  
   39 reader = itk.ImageFileReader[InputImageType].New()
   40 writer = itk.ImageFileWriter[OutputImageType].New()
   41  
   42 filter = itk.RescaleIntensityImageFilter[InputImageType, OutputImageType].New()
   43 filter.SetOutputMinimum( 0 )
   44 filter.SetOutputMaximum(255)
   45  
   46 filter.SetInput( reader.GetOutput() )
   47 writer.SetInput( filter.GetOutput() )
   48  
   49 reader.SetFileName( sys.argv[1] )
   50 writer.SetFileName( sys.argv[2] )
   51  
   52 writer.Update()
