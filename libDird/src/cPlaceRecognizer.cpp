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
#include "cPlaceRecognizer.h"
#include <iostream>
#include <vector>
#include <set>
#include <math.h>
#include <string.h>
#include <fstream>

#include <omp.h>
#include <algorithm>

using namespace std;

namespace DIRD
{
  cPlaceRecognizer::cPlaceRecognizer( uint8_t * feature_vectors, int num_features, int dim_feature )
    : feature_vectors_(feature_vectors), 
    num_features_(num_features),
    matSimilarity_(num_features), 
    matDynamicProgramming_(num_features), 
    matLoopClosures_(num_features), 
    dim_feature_(dim_feature)
  {

  }

  cPlaceRecognizer::~cPlaceRecognizer()
  {

  }

  bool cPlaceRecognizer::computePairwiseSimilarity( int safety_margin )
  {

    matSimilarity_.clear();

    // parameters of logistic function (sigmoid)
    double factor = 4.0/7.0;  // lower number = sharper cut off, higher number = smoother cut off
    float sig_max = 7e4f;
    float sig_par_1 = sig_max / 1.85f;
    float sig_par_2 = sig_max / 10.0f * (float)factor;
    float tau_1 = 0.05f;

    cout << "Computing vector distance for features: " << "\n";

    // loop over all pairs of poses
    for (int i = 0; i < num_features_; ++i)
    {

      // progress bar
      if ( i % 200 == 0)
      {
        cout << "\r[";
        for (int j = 0; j <  i/200; ++j)
        {
          cout << "#";
        }
        for (int j = i/200; j < num_features_/200; ++j)
        {
          cout << " ";
        }
        cout << "] " << i << " of " << num_features_;
        cout.flush();
      }


      // skip poses very near by (safety_margin)
      for (int j = i + safety_margin; j < num_features_; ++j)
      {
        // compute vector distance ...
        long distance = dist(i,j);
        // ... and translate it into a similarity score (0 ... 1) by a logistic function (sigmoid)
        float similarity = 1.0f - 1.0f/( 1.0f + exp( -( ((float)distance) - sig_par_1)/sig_par_2 ) );
        // dont polute similarity matrix and
        // store only those values which seem somewhat promising. 
        if (similarity > tau_1) // tau_1 is a very conservative threshold
        {
          matSimilarity_(i,j) = similarity;
        }
      }
    }

    cout << "\n";

    return true;
  }

  bool cPlaceRecognizer::postProcessSimilarities( int segment_length )
  {

    matDynamicProgramming_.clear();

    float tau_3 = 0.05f;
    float tau_2 = tau_3;

    // we use a 0-1-2-3 model
    vector<int> steps;
    for (int i = 0; i <= 3; ++i)
    {
      steps.push_back(i);
    }

    // start dynamic programming sweep
    set<int> startIndicies;
    tSparseMatrix DP(num_features_);
    int numHypos = matSimilarity_.size();
    int counter = 0;

    // loop overe all (non-zero) entries of the similarity matrix
    for (tSparseMatrixIterator iter = matSimilarity_.begin(); 
        iter != matSimilarity_.end(); 
        iter++)
    {

      if ( counter++ % 1000 == 0 )
      {
        cout << "\rProcessing loop closure  hypothesis " << counter << " of " << numHypos;
        cout.flush();
      }

      // skip unpromissing pairs
      if (iter->second < tau_2)
      {
        continue;
      }

      // compute 2d index
      int j = iter->first % num_features_;
      int i = (iter->first - j) / num_features_;

      startIndicies.clear();
      startIndicies.insert(i);

      DP.clear();
      DP(i,j) = iter->second;

      float maxValue = 0;
      for ( int jDP = j; jDP > j-segment_length+1; jDP-- )
      {
        if (jDP < 1)
        {
          break;
        }

        set<int> startIndiciesCopy = startIndicies;
        for ( set<int>::iterator iterIdx = startIndiciesCopy.begin(); 
            iterIdx != startIndiciesCopy.end(); 
            iterIdx++)
        {
          long idx = *iterIdx;
          for ( vector<int>::iterator iterS = steps.begin(); 
              iterS != steps.end(); 
              iterS++ )
          {
            int s = *iterS;
            if (idx-s < 0)
            {
              break;
            }

            startIndicies.insert(idx-s);

            // safely access elements
            float dp_source = DP.at( idx, jDP );
            float dp_dest = DP.at( idx-s, jDP-1 );
            float arr_dest = matSimilarity_.at( idx-s, jDP-1 );

            DP(idx-s, jDP-1) = max( dp_dest, dp_source + arr_dest );
            maxValue = max( DP(idx-s, jDP-1), maxValue);
          }
        }
      }

      // store value in loop closure matrix
      if ( maxValue > tau_3 * segment_length )
      {
        matDynamicProgramming_(i,j) = maxValue;
      }

    }

    cout << "" << "\n";

    return true;
  }

  bool cPlaceRecognizer::tSparseMatrix::toImage( uint8_t * img, int img_size )
  {
    // clear image
    memset( img, 0, sizeof(uint8_t) * img_size * img_size );
    // compute down/up-sampling scaling factor
    float scale = ((float)img_size) / ((float)size_);

    // compute max value of matrix
    float max_value = 0;
    for (iterator iter = this->begin(); 
        iter != this->end(); 
        iter++)
    {
      max_value = max(max_value, iter->second);
    }

    // loop over all non-zero matrix entries and fill image
    for (iterator iter = this->begin(); 
        iter != this->end(); 
        iter++)
    {
      // compute 2d index
      int j = iter->first % size_;
      int i = (iter->first - j) / size_;

      int ii = (int)((float)i * scale);
      int jj = (int)((float)j * scale);

      img[ ii * img_size + jj] = (uint8_t)max(iter->second / max_value * 255, (float)img[ii * img_size + j]);
    }

    return true;

  }


  bool cPlaceRecognizer::tSparseMatrix::toFile( string file_name )
  {
    // loop over tiles and compute DIRD feature
    ofstream matrix_file;
    matrix_file.open(file_name.c_str());
    if (!matrix_file.is_open())
    {
      return false;
    }

    matrix_file << size_ << "\n";

    // loop over all non-zero matrix entries and fill image
    for (iterator iter = this->begin(); 
        iter != this->end(); 
        iter++)
    {
      // compute 2d index
      int j = iter->first % size_;
      int i = (iter->first - j) / size_;

      // dump to file
      matrix_file << i << " " << j << " " << iter->second << "\n";
    }

    matrix_file.close();

    return true;
  }

  bool cPlaceRecognizer::computeLoops( int non_max )
  {

    matLoopClosures_ = matDynamicProgramming_;

    for (tSparseMatrixIterator iter = matLoopClosures_.begin(); 
        iter != matLoopClosures_.end(); 
        iter++)
    {

      // compute 2d index
      int j = iter->first % num_features_;
      int i = (iter->first - j) / num_features_;

      // find maximum
      vector<float> vecVals;
      for (int k = -non_max+1; k < non_max; ++k)
      {
        vecVals.push_back( matLoopClosures_.at(i-k, j+k) );
      }

      // keep only the top 2 and discard others
      sort(vecVals.begin(), vecVals.end());
      if (vecVals.size() >= 2)
      {
        float tau = vecVals[ vecVals.size() - 2 ];
        // surpress non-maximum
        for (int k = -non_max+1; k < non_max; ++k)
        {
          if ( matLoopClosures_.at(i-k, j+k) != 0 && matLoopClosures_.at(i-k, j+k) < tau )
          {
            matLoopClosures_(i-k,j+k) = 0;
          }
        }
      }

    }

    return true;
  }

}
