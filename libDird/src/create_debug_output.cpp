/*
Copyright 2012. All rights reserved.
Institute of Measurement and Control Systems
Karlsruhe Institute of Technology, Germany

This file is part of libDird.
Authors: Henning Lategahn
Website: http://www.mrt.kit.edu/libDird.php

libDird is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or any later version.

libDird is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
libDird; if not, write to the Free Software Foundation, Inc., 51 Franklin
Street, Fifth Floor, Boston, MA 02110-1301, USA 
*/

/*
 
   If you use this code for your research we kindly ask you 
   to cite the following article.

  @inproceedings{lategahn2013HowTo,
    Address = {Gold Coast, Australia},
    Author = {Henning Lategahn and Johannes Beck and Bernd Kitt and Christoph Stiller},
    Booktitle = {IEEE Intelligent Vehicles Symposium},
    Title = {How to Learn an Illumination Robust Image Feature for Place Recognition (submitted)},
    Year = {2013}}

*/
#include <iostream>
#include <string>
#include <vector>


#if _MSC_VER <= 1500
typedef unsigned char uint8_t;
#else
#include <stdint.h>
#endif

#include "cImage.h"
#include "cPlaceRecognizer.h"

using namespace std;
void saveToPng( uint8_t * img, int width, int height, string fileName );
typedef DIRD::cPlaceRecognizer::tSparseMatrix tSparseMatrix;
typedef DIRD::cPlaceRecognizer::tSparseMatrixIterator tSparseMatrixIterator;

int main (int argc, char** argv) 
{

  if (argc<4) 
  {
cout << "\n\n";
    cout << "Debug images showing detected loops are created after loops have been detected by compute_loops.\n";
    cout << "                                                                                   \n";
    cout << "                                                                                   \n";
    cout << "\33[1mUsage\33[0m:\n  ./create_debug_output  <path/to/matrix> <path/to/image_sequence> <path/to/dump_folder>\n";
    cout << "                                                                                   \n";
    cout << "                                                                                   \n";
    cout << "  \33[1m<path/to/matrix> \33[0m                                              \n";
    cout << "                                                                                   \n";
    cout << "    The matrix for which detected loops shall be shown together.\n";
    cout << "    Of the three matrices that are created by compute_loops the\n";
    cout << "    \"step3_loops.txt\" matrix should be read.                            \n";
    cout << "                                                                                   \n";
    cout << "                                                                                   \n";
    cout << "  \33[1m<path/to/image_sequence> \33[0m                                            \n";
    cout << "                                                                                   \n";
    cout << "    A folder containing the images of the initial input sequence.            \n";
    cout << "    It needs to contain images in the following format:                \n";
    cout << "                                                                                   \n";
    cout << "    <image_sequence>                                                               \n";
    cout << "        ├── 000000.png                                                             \n";
    cout << "        ├── 000001.png                                                             \n";
    cout << "        ├── 000002.png                                                             \n";
    cout << "        ├── 000003.png                                                             \n";
    cout << "        ├── 000004.png                                                             \n";
    cout << "        ├── 000005.png                                                             \n";
    cout << "        └── ........                                                               \n";
    cout << "                                                                                   \n";
    cout << "    Only PNGs are supported. Indicies need to be contiuous.                        \n";
    cout << "    Maximally allowed are 100000 images.                                           \n";
    cout << "                                                                                   \n";
    cout << "                                                                                   \n";
    cout << "  \33[1m<path/to/dump_folder>\33[0m                                             \n";
    cout << "                                                                                   \n";
    cout << "    An initially empty folder where output images will be stored. Note that depending \n";
    cout << "    on the number of detected loop closures there may be a lot of images created.\n";
    cout << "                                                                                   \n";
    cout << "                                                                                   \n";
    cout << "\33[1mExample\33[0m:\n  ./create_debug_output path/to/threefold/matrices/step3_loops.txt path/to/threefold/image_0 path/to/threefold/debug \n";
    cout << "\n";

    return 1;
  }


  // handle arguements
  string matrix_file_name = argv[1];
  string img_dir = argv[2];
  string dump_dir = argv[3];

  // load matrix
  tSparseMatrix matrix(matrix_file_name);
  if (matrix.size_ == 0)
  {
    cerr << "Couldnt load matrix file " << matrix_file_name << "\n";
  }

  // dimension of output image
  int width_down = 600;
  int height_down = 300;

  int num_loops = matrix.size();
  int iCounter = 0;

  // loop over all non-zero entries
  for (tSparseMatrixIterator iter = matrix.begin(); 
      iter != matrix.end(); 
      iter++)
  {

    // compute 2d index
    int j = iter->first % matrix.size_;
    int i = (iter->first - j) / matrix.size_;

    // input file names
    const int bufSize = 256;
    char base_name[bufSize]; 

#ifdef _MSC_VER
    sprintf_s(base_name, bufSize, "%06d.png",i);
#else
    sprintf(base_name,"%06d.png",i);
#endif
    string img_file_name1  = img_dir + "/" + base_name;

#ifdef _MSC_VER
    sprintf_s(base_name, bufSize, "%06d.png",j);
#else
    sprintf(base_name,"%06d.png",j);
#endif
    string img_file_name2  = img_dir + "/" + base_name;

#ifdef _MSC_VER
    sprintf_s(base_name, bufSize, "%06d_%06d.png",i,j);
#else
    sprintf(base_name,"%06d_%06d.png",i,j);
#endif

    try 
    {

      // load input image
      DIRD::cImage image1(img_file_name1);
      DIRD::cImage image2(img_file_name2);

      // image dimensions
      int width  = image1.getWidth();
      int height = image2.getHeight();

      // compute scaling factors for down sampling
      float scale_hor = ((float)width) / ((float) width_down );
      float scale_ver = ((float)height) / ((float) height_down );

      // down sample and concatinate the image
      uint8_t* img_data  = new uint8_t[ width_down * height_down * 2];
      int k = 0;
      for (int v = 0; v < height_down; v++) 
      {
        for (int u = 0; u < width_down; u++) 
        {
          int uu = (int)(((float)u) * scale_hor);
          int vv = (int)(((float)v) * scale_ver);

          image1.getPixel( uu, vv, img_data[k] );
          image2.getPixel( uu, vv, img_data[k + width_down * height_down ]);
          k++;
        }
      }

      string file_out = dump_dir + "/" + string(base_name);
      cout << "Saving " << file_out << " (" <<  iCounter++ << " of " << num_loops << ")\n";
      saveToPng( img_data, width_down, height_down * 2, file_out);

    }
    catch (...) 
    {
      cerr << "\nERROR: Processing files " << img_file_name1 << " or " <<  img_file_name2 << endl;
      continue;
    }

  }

  // output
  cout << "\nDone creating debug output! Exiting ..." << endl;

  // exit
  return 0;
}

void saveToPng( uint8_t * img, int width, int height, string fileName )
{
  DIRD::cImage image(width, height, 8);
  for (int v = 0; v <  height; ++v)
  {
    for (int u = 0; u <  width; ++u)
    {
      uint8_t value = img[v * width + u];

      image.setPixel(u, v, value );
    }
  }
  image.write(fileName);
}


