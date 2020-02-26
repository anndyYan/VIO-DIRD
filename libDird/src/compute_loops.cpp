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

#if _MSC_VER <= 1500
typedef unsigned char uint8_t;
#else
#include <stdint.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <fstream>

#include "cImage.h"
#include "cDird.h"
#include "cPlaceRecognizer.h"

using namespace std;

bool loadFeatureFromFile( string fileName, uint8_t * feature, int dim );
void saveToPng( uint8_t * img, int img_size, string fileName );

/*
 * A folder of image features is traversed, image features are
 * read from disk and loop closures are detected. Matrizes describing
 * the pairwise similarity between images are saved to disk in text
 * and image format for easy viewing. 
 *
 * Basically the cPlaceRecognizer is wrapped and file io is handled.
 */
int main (int argc, char** argv) 
{


  if (argc<3) 
  {
    cout << "\n\n";
    cout << "Features of an input folder containing features are read from disk.             \n";
    cout << "Thereafter loop closures (image pairs showing the same place) are computed.       \n";
    cout << "The output will be sparse matrices of size num_features x num_features showing pairwise\n";
    cout << "similarities. Matrices are stored in text files as well as images for easy viewing.\n";
    cout << "Two matrices (\"similarity\",\"dyn_prog\") are mere intermediate values and saved only for debugging.\n";
    cout << "The finally detected loop closures are stored in the matrix \"loops\".\n";
    cout << "                                                                                   \n";
    cout << "                                                                                   \n";
    cout << "\33[1mUsage\33[0m:\n  ./compute_loops  <path/to/feature_folder>  <path/to/matrix_folder> [size_of_matrix_image=1200]\n";
    cout << "                                                                                   \n";
    cout << "                                                                                   \n";
    cout << "  \33[1m<path/to/feature_folder> \33[0m                                                \n";
    cout << "                                                                                   \n";
    cout << "    A folder containing text files. Each text file needs to contain one feature vector.\n";
    cout << "    This folder was filled previously by ./compute_features.\n";
    cout << "    The folder needs to contain the feature files in the following format:     \n";
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
    cout << "  \33[1m<path/to/matrix_folder> \33[0m                                               \n";
    cout << "                                                                                   \n";
    cout << "    An initially empty output folder. Results will be stored here.\n";
    cout << "    The images in <matrix_folder> show pairwise similarities between indicies.\n";
    cout << "                                                                                   \n";
    cout << "    The sparse matrix text file format is as follows:      \n";
    cout << "                                                                                   \n";
    cout << "        number_of_rows_which_is_number_of_columns_of_matrix                        \n";
    cout << "        row_index_1 column_index_1 place_equality_measure_1                    \n";
    cout << "        row_index_2 column_index_2 place_equality_measure_2                    \n";
    cout << "        row_index_3 column_index_3 place_equality_measure_3                      \n";
    cout << "        row_index_4 column_index_4 place_equality_measure_4                          \n";
    cout << "            ........                                                               \n";
    cout << "            ........                                                               \n";
    cout << "            ........                                                               \n";
    cout << "                                                                                   \n";
    cout << "    The finally detected loop closures are stored in loops.txt in the <matrix_folder>\n";
    cout << "                                                                                   \n";
    cout << "                                                                                   \n";
    cout << "  \33[1m[size_of_matrix_image]\33[0m                                                         \n";
    cout << "                                                                                   \n";
    cout << "    An integer specifying the size of the output matrix images.\n";
    cout << "    The value is optional and its default is 1200 px.\n";
    cout << "    A sensible value seems something like num_features/4 or so.\n";
    cout << "                                                                                   \n";
    cout << "                                                                                   \n";
    cout << "                                                                                   \n";
    cout << "\33[1mExample\33[0m:\n  ./compute_loops path/to/threefold/features path/to/threefold/matrices\n";
    cout << "\n";
    return 1;
  }

  int img_size = 0;
  if (argc<4)
  {
    img_size = 1200;
  }
  else
  {
    img_size = atoi(argv[3]);
  }

  // sequence directory
  string dir = argv[1];
  string dump_dir = argv[2];
  int num_features = 100000;

  // allocate some memory large enough to hold all feature vectors
  static const int max_num_features = 100000; // adjust to your needs
  static const int dim_feature = DIRD::cDird::iDim_ * 16 /* assuming a tiling of 4x4 */; 
  uint8_t * feature_vectors = (uint8_t*)_mm_malloc(sizeof(uint8_t) *  max_num_features * dim_feature, 16); 

  // loop over all features and load them from disk
  cout << "Loading features from disk:" << "\n";
  for (int i = 0; i <= num_features; i++) 
  {

    // input file names
    char base_name[256]; 

#ifdef _MSC_VER
    sprintf_s(base_name, 256, "%06d.txt",i);
#else
	sprintf(base_name,"%06d.txt",i);
#endif

    string feature_file_name  = dir + "/" + base_name;
    cout << "\rLoading feature " << feature_file_name;

    // load feature vector from file
    if (!loadFeatureFromFile( feature_file_name, &feature_vectors[ i * dim_feature ], dim_feature ))
    {
      cerr << "\nError reading feature from file " << feature_file_name << ". Does file exist?\n";
      break;
    }

    num_features = i + 1;

  }

  cout << "\n";

  // compute loop closures
  DIRD::cPlaceRecognizer place_recognizer( feature_vectors, num_features, dim_feature );
  if (!place_recognizer.computePairwiseSimilarity( 200 ))
  {
    cerr << "Computing pairwise similarities failed. Exiting.\n";
    return 1;
  }
  if (!place_recognizer.postProcessSimilarities( 20 ))
  {
    cerr << "Postprocessing similarities failed. Exiting.\n";
    return 1;
  }
  if (!place_recognizer.computeLoops( 60 ))
  {
    cerr << "Computing loops failed. Exiting.\n";
    return 1;
  }

  // dump some stuff to disk
  uint8_t * img = new uint8_t[ img_size * img_size ];

  // ***********************************************************************************************************
  // dump initial pairwise similarity matrix
  string name = "step1_similarity";
  if (!place_recognizer.matSimilarity_.toFile( dump_dir + "/" + name + ".txt" ))
  {
    cerr << "Error writing similarity to " << dump_dir + "/" + name + ".txt\n" << "Does folder exist?\n";
  }
  else
  {
    cout << "Output written to " << dump_dir + "/" + name + ".txt\n";
  }
  place_recognizer.matSimilarity_.toImage( img, img_size );
  saveToPng( img, img_size, dump_dir + "/" + name + ".png");
  cout << "Output written to " << dump_dir + "/" + name + ".png\n";
  // ***********************************************************************************************************

  // ***********************************************************************************************************
  // dump initial post processed (dynamic programming) similarity matrix
  name = "step2_dyn_prog";
  if (!place_recognizer.matDynamicProgramming_.toFile( dump_dir + "/" + name + ".txt" ))
  {
    cerr << "Error writing similarity to " << dump_dir + "/" + name + ".txt\n" << "Does folder exist?\n";
  }
  else
  {
    cout << "Output written to " << dump_dir + "/" + name + ".txt\n";
  }
  place_recognizer.matDynamicProgramming_.toImage( img, img_size );
  saveToPng( img, img_size, dump_dir + "/" + name + ".png");
  cout << "Output written to " << dump_dir + "/" + name + ".png\n";
  // ***********************************************************************************************************

  // ***********************************************************************************************************
  // dump the non-maxima surpressed matrix containing only loops
  name = "step3_loops";
  if (!place_recognizer.matLoopClosures_.toFile( dump_dir + "/" + name + ".txt" ))
  {
    cerr << "Error writing similarity to " << dump_dir + "/" + name + ".txt\n" << "Does folder exist?\n";
  }
  else
  {
    cout << "Output written to " << dump_dir + "/" + name + ".txt\n";
  }
  place_recognizer.matLoopClosures_.toImage( img, img_size );
  saveToPng( img, img_size, dump_dir + "/" + name + ".png");
  cout << "Output written to " << dump_dir + "/" + name + ".png\n";
  // ***********************************************************************************************************

  // all finished 
  cout << "Loop closure detection complete! Exiting ..." << endl;

  // exit
  delete [] img;
  _mm_free(feature_vectors);
  return 0;
}

void saveToPng( uint8_t * img, int img_size, string fileName )
{
  uint8_t color_map[256][3] =
  { 
    #include "color1.map"
  };

  DIRD::cImage image(img_size, img_size, 24);
  for (int i = 0; i <  img_size; ++i)
  {
    for (int j = 0; j <  img_size; ++j)
    {
      int idx = img[i * img_size + j];

	  unsigned char bgr[] = {color_map[idx][0], color_map[idx][1], color_map[idx][2]};
	  image.setPixel(i, j, bgr);
    }
  }
  image.write(fileName);
}

bool loadFeatureFromFile( string fileName, uint8_t * feature, int dim )
{

  ifstream file( fileName.c_str() );
  if (!file.is_open())
  {
    return false;
  }

  string word,line;
  getline (file,line);
  istringstream iss(line, istringstream::in);
  for (int i = 0; i < dim; ++i)
  {
    iss >> word;
    int v = atoi(word.c_str());
    if (v < 0 || v > 255)
    {
      cerr << "Trying to load a feature vector which is not uint8_t\n";
      return false;
    }
    feature[i] = (uint8_t) v;
    if (!iss.good())
    {
      cerr << "Trying to load a feature vector which is lower dimensional than " << dim << "\n";
      return false;
    }
  }

  return true;
}
