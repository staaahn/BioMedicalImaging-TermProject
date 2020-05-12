#include "header.h"

void main( int argc, char *argv[] ) {
    while (input != 0) {
    cout << "Please select an option: \n"
         << "1. WatershedSegmentation\n" 
         << "2. Registration (B-Spline)\n"
        <<"3. Image Processing\n"
         << "EXIT: 0" << endl;
    cin >> input;
    cout << endl;

    if(input == 1){
        cout << "You selected Watershed Segmentation" << endl << "Working..." << endl;
        cout << "Please fill the following fields in order: Input Image, Output Image, Lower Threshold, Output Scale Level" << endl;
        cin >> inputImage >> outputImage >> lowerThreshold >> outputScaleLevel;

        WatershedSegmentation2(inputImage.c_str(), outputImage.c_str(), lowerThreshold.c_str(), outputScaleLevel.c_str());
    }
  
    if (input == 2){
        cout << "You selected B-Spline Registration" << endl << "Working..." << endl;
        cout << "Please fill the following fields in order: Fixed Image, Moving Image, Output Image" << endl;
        string fixed, moving, output;
        cin >> fixed >> moving >> output;

        BSplineRegistration(fixed.c_str(), moving.c_str(), output.c_str());
    }

    if (input == 3) { 
        cout << "You selected image processing" << endl << "Please select an option below: " << endl;
        cout << "1. Convert 3D to Dicom series\n"
            << "2. Convert 3D to PNG" << endl;

        cin >> input;

        if( input == 1 ){
          string input, output;
          cout << "You selected 3D -> Dicom series" << endl << "Working..." << endl;
          cout << "Please fill the following fields in order: input image, output directory" << endl;
          cin >> input, output;
          ImageReadDicomSeriesWrite(input.c_str(), output.c_str());
        }
        if(input == 2) {
          string input, output, extension;
          cout << "You selected 3D -> PNG series" << endl << "Working..." << endl;
          cout << "Please fill the following fields in order: 3D image file, output, extension" << endl;
          cin >> input, output, extension;
          ImageReadImageSeriesWrite(input.c_str(), output.c_str(), extension.c_str());
       }
    }
  }
}
