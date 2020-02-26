/*
Copyright 2012. All rights reserved.
Institute of Measurement and Control Systems
Karlsruhe Institute of Technology, Germany

This file is part of libDird.
Authors: Henning Lategahn
         Johannes Beck
         Bernd Kitt
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
#include <fstream>

#if _MSC_VER <= 1500
typedef unsigned char uint8_t;
#else
#include <stdint.h>
#endif

// #define TIMING

#ifdef TIMING
#include <sys/time.h>
#endif

#include "cDird.h"
#include "cImage.h"

using namespace std;

/*
 * This file computes DIRD features for every image of the input sequence.
 * Feature vectors will be stored in human readable form in text files 
 * for loop closure detection (see compute_loops).
 *
 * Basically file io is handled and image data is passed to cDird. That's it.
 *
 * Loop Closures can be computed only after features have been computed for a sequence.
 * To this end one needs to launch compute_loops as a second step.
 *
 */
int main (int argc, char** argv) 
{

  if (argc<3) 
  {

    cout << "\n\n";
    cout << "A folder containing images of a sequence will be read, feature vectors             \n";
    cout << "are extracted for every image and stored in a destination folder. Thereafter       \n";
    cout << "./compute_loops can be run on this feature folder to compute loop closures.        \n";
    cout << "                                                                                   \n";
    cout << "                                                                                   \n";
    cout << "\33[1mUsage\33[0m:\n  ./compute_features <path/to/image_sequence> <path/to/feature_folder>\n";
    cout << "                                                                                   \n";
    cout << "                                                                                   \n";
    cout << "  \33[1m<path/to/image_sequence> \33[0m                                            \n";
    cout << "                                                                                   \n";
    cout << "    A folder containing the images of a sequence for which places shall            \n";
    cout << "    be matched. It needs to contain images in the following format:                \n";
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
    cout << "                                                                                   \n";
    cout << "  \33[1m<path/to/feature_folder> \33[0m                                            \n";
    cout << "                                                                                  \n";
    cout << "    An initially empty folder. For every image in <image_sequence> one text        \n";
    cout << "    file will be stored in <feature_folder>. After running compute_features it will\n";
    cout << "    contain the following files storing feature vectors for every input image.     \n";
    cout << "                                                                                   \n";
    cout << "    <feature_folder>                                                               \n";
    cout << "        ├── 000000.txt                                                             \n";
    cout << "        ├── 000001.txt                                                             \n";
    cout << "        ├── 000002.txt                                                             \n";
    cout << "        ├── 000003.txt                                                             \n";
    cout << "        ├── 000004.txt                                                             \n";
    cout << "        ├── 000005.txt                                                             \n";
    cout << "        └── ........                                                               \n";
    cout << "                                                                                   \n";
    cout << "    Files contain 3456 dimensional DIRD based features in human readable ASCII format.\n";
    cout << "                                                                                   \n";
    cout << "                                                                                   \n";
    cout << "\33[1mExample\33[0m:\n  ./compute_features path/to/threefold/image_0 path/to/threefold/features\n";
    cout << "\n";
    return 1;
  }


  int num_images = 100000;

  // IMPORTANT: if you change the values below then the parameters
  // of the logistic function in cPlaceRecognizer.cpp need to be adjusted!

  // number of pixels for one tile
  int tile_size = 48;
  // number of tiles horizontally
  int num_tiles_hor = 4;
  // number of tiles vertically
  int num_tiles_ver = 4;

  // dimension of down-sampled image
  int width_down = tile_size * num_tiles_hor;
  int height_down = tile_size * num_tiles_ver;
  uint8_t* img_data  = new uint8_t[ width_down * height_down ];
  DIRD::cDird dird( width_down, height_down );

  // sequence directory
  string img_dir = argv[1];
  string feat_dir = argv[2];

  // loop over all frames 
  for (int i = 0; i <= num_images; i++) 
  {

    // input file names
    const int bufSize = 256;
    char base_name[bufSize];

#ifdef _MSC_VER
    sprintf_s(base_name, bufSize, "%06d.png", i);
#else
    sprintf(base_name,"%06d.png",i);
#endif

    string img_file_name  = img_dir + "/" + base_name;

    // feature file that will be created
#ifdef _MSC_VER
    sprintf_s(base_name, bufSize, "%06d.txt",i);
#else
    sprintf(base_name,"%06d.txt",i);
#endif

    string feature_file_name  = feat_dir + "/" + base_name;

    // catch image read/write errors here
    try 
    {

      // load input image
      DIRD::cImage image(img_file_name);
      cout << "\rComputing feature for " << img_file_name;

      // image dimensions
      int width  = image.getWidth();
      int height = image.getHeight();

      // compute scaling factors for down sampling
      float scale_hor = ((float)width) / ((float) width_down );
      float scale_ver = ((float)height) / ((float) height_down );

      // down sample the image
      int k = 0;
      for (int v = 0; v < height_down; v++) 
      {
        for (int u = 0; u < width_down; u++) 
        {
          int uu = (int)(((float)u) * scale_hor);
          int vv = (int)(((float)v) * scale_ver);
          image.getPixel( uu, vv, img_data[k] );
          k++;
        }
      }

#ifdef TIMING
      struct timeval TIME;    
      gettimeofday(&TIME, NULL);    
      double TIME_START = TIME.tv_sec + ((double) TIME.tv_usec)/1000000.0;
#endif

      // initialize the DIRD extractor
      if (!dird.process( img_data ))
      {
        cerr << "Couldnt pre-process image for DIRD extraction\n";
        continue;
      }


      // loop over tiles and compute DIRD feature
      ofstream feature_file;
      feature_file.open(feature_file_name.c_str());
      if (!feature_file.is_open())
      {
        cerr << "Couldnt create file " << feature_file_name << ". Does feature directory exist?\n";
        continue;
      }

      vector<uint8_t> feature_vector;
      for (int x = 0; x <  num_tiles_hor; ++x)
      {
        for (int y = 0; y <  num_tiles_ver; ++y)
        {
          // compute DIRD feature ... 
          if (!dird.get(x * tile_size + tile_size/2, y * tile_size + tile_size/2, feature_vector))
          {
            cerr << "Couldn't extract DIRD feature for pixel position " << x * tile_size + tile_size/2 << " " <<  y * tile_size + tile_size/2 << "\n";
            cerr << "Writing nothing to feature file " << feature_file_name << "! Re-run compute_features\n";
            feature_file.close();
            continue;
          }
          // ... and dump to file
          for (int d = 0; d < dird.iDim_; ++d)
          {
            feature_file << (int)feature_vector[d] << " ";
          }
        }
      }
      feature_file.close();

#ifdef TIMING
      gettimeofday(&TIME, NULL);    
      double TIME_END = TIME.tv_sec + ((double) TIME.tv_usec)/1000000.0;
      cout << "\nTime in [ms]: " << (TIME_END - TIME_START) * 1000 << "\n";
#endif

    } 
    catch (...) 
    {
      cerr << "\nERROR: Couldn't read input files " << img_file_name << endl;
      return 1;
    }

  }

  // output
  cout << "\nDIRD extraction complete! Exiting ..." << endl;

  // exit
  delete [] img_data;
  return 0;
}

