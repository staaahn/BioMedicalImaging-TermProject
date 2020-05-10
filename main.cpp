#include <stdio.h>
#include <string.h>
using namespace std;


void main( int argc, char *argv[] ) {
    while (input != 0) {
    cout << "Please select an option: \n"
         << "1. Perform WatershedSegmentation\n" 
         << "EXIT: 0" << endl;
    cin >> choice;
    cout << endl;

    if(input == 1){
        cout << "You selected Watershed Segmentation" << endl << "Working..." << endl;
        cout << "Please fill the following fields in order: Input Image, Output Image, Lower Threshold, Output Scale Level" << endl;
        cin >> inputImage >> outputImage >> lowerThreshold >> outputScaleLevel;

        WatershedSegmentation2(inputImage.c_str(), outputImage.c_str(), lowerThreshold.c_str(), outputScaleLevel.c_str());
    }
  }
}
