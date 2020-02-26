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
#include "cDird.h"
#include <math.h>
#include <string.h>

namespace DIRD
{
  cDird::cDird(int width, int height)
    : iWidth_(width), iHeight_(height), bIsProcessed_(false)
  {

    // alloc some memory for an integral image
    img_integral = new long[iWidth_ * iHeight_] ;

    // this is a stack of images which stores intermediate values
    arrWaveletStack_ = new double*[ numLevels_ * 3 ];
    for (int i = 0; i < numLevels_*3; ++i)
    {
      arrWaveletStack_[i] = new double[ iWidth_ * iHeight_ ];
      memset( arrWaveletStack_[i], 0, sizeof(double) * iWidth_ * iHeight_ );
    }

    featureVector_ = new double[ iDim_ ];
  }

  cDird::~cDird()
  {
    delete [] img_integral;
    for (int i = 0; i < numLevels_*3; ++i)
    {
      delete [] arrWaveletStack_[i];
    }
    delete [] arrWaveletStack_;
    delete [] featureVector_;
  }

  bool cDird::process( uint8_t * img_data )
  {

    // compute integral image
    for (int v = 0; v <  iHeight_; ++v)
    {
      long sum = 0;
      for (int u = 0; u <  iWidth_; ++u)
      {
        long idx = getIdx(u,v);
        sum += img_data[idx];
        img_integral[idx] = sum;
      }
    }
    for (int u = 0; u <  iWidth_; ++u)
    {
      long sum = 0;
      for (int v = 0; v <  iHeight_; ++v)
      {
        long idx = getIdx(u,v);
        sum += img_integral[idx];
        img_integral[idx] = sum;
      }
    }

    // compute wavelet feature for every pixel position
    int maxMargin = (int)pow(2.0, numLevels_);

    if (maxMargin >= iHeight_ || maxMargin >= iWidth_)
    {
      return false;
    }

    for (int y = maxMargin/2; y < iHeight_ - maxMargin/2; ++y)
    {
      for (int x = maxMargin/2; x < iWidth_ - maxMargin/2; ++x)
      {

        double squared_norm = 0.0;
        long idx = getIdx(x,y);

        for (int i = 0; i < numLevels_; ++i)
        {
          int blockSize = (int)pow(2.0, i+1);

          // first haar wavelet
          long sum1, sum2, sum3, sum4, sum5, sum6;
          sum1 = img_integral[ getIdx(x - blockSize/2, y - blockSize/2) ];
          sum2 = img_integral[ getIdx(x              , y - blockSize/2) ];
          sum3 = img_integral[ getIdx(x + blockSize/2, y - blockSize/2) ];
          sum4 = img_integral[ getIdx(x + blockSize/2, y + blockSize/2) ];
          sum5 = img_integral[ getIdx(x              , y + blockSize/2) ];
          sum6 = img_integral[ getIdx(x - blockSize/2, y + blockSize/2) ];
          arrWaveletStack_[ i*3 ][ idx ] = 2 * sum5 - 2 * sum2 - sum6 + sum1 - sum4 + sum3;
          arrWaveletStack_[ i*3 ][ idx ] /= blockSize*blockSize;

          // second haar wavelet
          sum1 = img_integral[ getIdx(x - blockSize/2, y - blockSize/2) ];
          sum2 = img_integral[ getIdx(x + blockSize/2, y - blockSize/2) ];
          sum3 = img_integral[ getIdx(x + blockSize/2, y              ) ];
          sum4 = img_integral[ getIdx(x + blockSize/2, y + blockSize/2) ];
          sum5 = img_integral[ getIdx(x - blockSize/2, y + blockSize/2) ];
          sum6 = img_integral[ getIdx(x - blockSize/2, y              ) ];
          arrWaveletStack_[ i*3 + 1 ][ idx ] = 2*sum3 - sum2 - 2*sum6 + sum1 - sum4 + sum5;
          arrWaveletStack_[ i*3 + 1 ][ idx ] /= blockSize*blockSize;

          // third haar wavelet
          double sum7, sum8, sum9;
          sum7 = img_integral[ getIdx(x, y - blockSize/2) ];
          sum8 = img_integral[ getIdx(x, y + blockSize/2) ];
          sum9 = img_integral[ getIdx(x, y) ];
          arrWaveletStack_[ i*3 + 2 ][ idx ] = 4*sum9 - 2*sum7 - 2*sum6 + sum1 + sum4 - 2*sum3 - 2*sum8 + sum2 + sum5;
          arrWaveletStack_[ i*3 + 2 ][ idx ] /= blockSize*blockSize;

          squared_norm += 
            arrWaveletStack_[ i*3 ][ idx ] * arrWaveletStack_[ i*3 ][ idx ] +
            arrWaveletStack_[ i*3 + 1 ][ idx ] * arrWaveletStack_[ i*3 + 1 ][ idx ] + 
            arrWaveletStack_[ i*3 + 2 ][ idx ] * arrWaveletStack_[ i*3 + 2 ][ idx ];

        }

        // normalize to unit length
        if (squared_norm > .001)
        {
          squared_norm = (long)sqrt((double)squared_norm);
          for (int d = 0; d < numLevels_*3; ++d)
          {
            arrWaveletStack_[d][ idx ] /= squared_norm;
          }
        }

      }
    }

    bIsProcessed_ = true;
    return true;
  }

  bool cDird::get( int u, int v, std::vector<uint8_t> & feature_vector )
  {

    // check if pre-processing was done
    if (!bIsProcessed_)
    {
      return false;
    }

    // features cannot be computed closer to the image border than 20 pixels
    if ( u<20 || v<20 || u>=iWidth_-20 || v>=iWidth_-20)
    {
      return false;
    }

    // this is the pixel position that needs to be described
    long idx = getIdx(u, v);

    // we sum several of the wavelet features (see process() and arrWaveletStack_)
    // around (u,v). We also sum the absolut values.
    int offset0  = idx;
    int offset1  = idx - 3 * iWidth_ + 0; 
    int offset2  = idx - 2 * iWidth_ - 2; 
    int offset3  = idx - 1 * iWidth_ - 3; 
    int offset4  = idx + 2 * iWidth_ - 2; 
    int offset5  = idx + 5 * iWidth_ + 0; 
    int offset6  = idx - 1 * iWidth_ + 1; 
    int offset7  = idx + 1; 
    int offset8  = idx + 2 * iWidth_ + 1; 
    int offset9  = idx + 2 * iWidth_ + 2; 
    int offset10 = idx - 1 * iWidth_ + 3; 
    int offset11 = idx - 1 * iWidth_ + 5; 

    // finally we concatinate nine such feature vectors to form the final DIRD
    static const int Off = 13;
    int coarseOffsets[] = 
    {
      -Off * iWidth_ - Off,
      0 * iWidth_ - Off,
      Off * iWidth_ - Off,
      -Off * iWidth_,
      0 * iWidth_,
      Off * iWidth_,
      -Off * iWidth_ + Off,
      0 * iWidth_ + Off,
      Off * iWidth_ + Off,
    }; 

    // clear the feature vector
    memset( featureVector_, 0, sizeof(double) * iDim_ );

    // loop over these nine feature vectors which shall be concatinated
    for (int C = 0; C < sizeof(coarseOffsets)/sizeof(coarseOffsets[0]); C++)
    {
      // compute the summation
      for (size_t i = 0; i < numLevels_*3; ++i)
      {
        featureVector_[ 2 * (i + C * numLevels_*3) ] = 
          arrWaveletStack_[i][ offset0  + coarseOffsets[C] ] +
          arrWaveletStack_[i][ offset1  + coarseOffsets[C] ] +
          arrWaveletStack_[i][ offset2  + coarseOffsets[C] ] +
          arrWaveletStack_[i][ offset3  + coarseOffsets[C] ] +
          arrWaveletStack_[i][ offset4  + coarseOffsets[C] ] +
          arrWaveletStack_[i][ offset5  + coarseOffsets[C] ] +
          arrWaveletStack_[i][ offset6  + coarseOffsets[C] ] +
          arrWaveletStack_[i][ offset7  + coarseOffsets[C] ] +
          arrWaveletStack_[i][ offset8  + coarseOffsets[C] ] +
          arrWaveletStack_[i][ offset9  + coarseOffsets[C] ] +
          arrWaveletStack_[i][ offset10 + coarseOffsets[C] ] +
          arrWaveletStack_[i][ offset11 + coarseOffsets[C] ] 
          ;

        featureVector_[ 2 * (i + C * numLevels_*3) + 1 ] = 
          fabs(arrWaveletStack_[i][ offset0  + coarseOffsets[C] ] )+
          fabs(arrWaveletStack_[i][ offset1  + coarseOffsets[C] ] )+
          fabs(arrWaveletStack_[i][ offset2  + coarseOffsets[C] ] )+
          fabs(arrWaveletStack_[i][ offset3  + coarseOffsets[C] ] )+
          fabs(arrWaveletStack_[i][ offset4  + coarseOffsets[C] ] )+
          fabs(arrWaveletStack_[i][ offset5  + coarseOffsets[C] ] )+
          fabs(arrWaveletStack_[i][ offset6  + coarseOffsets[C] ] )+
          fabs(arrWaveletStack_[i][ offset7  + coarseOffsets[C] ] )+
          fabs(arrWaveletStack_[i][ offset8  + coarseOffsets[C] ] )+
          fabs(arrWaveletStack_[i][ offset9  + coarseOffsets[C] ] )+
          fabs(arrWaveletStack_[i][ offset10 + coarseOffsets[C] ]) +
          fabs(arrWaveletStack_[i][ offset11 + coarseOffsets[C] ]) 
          ;
      }
    }

    // finally we squeeze the "contiuous" feature into uint8_t by scaling (and adding offsets)
    feature_vector.clear();
    for (int d = 0; d <  iDim_; ++d)
    {
      int val = (int) ( 10 * featureVector_[d] + 100 );
      if (val < 0)
      {
        val = 0;
      }
      if (val > 255)
      {
        val = 255;
      }

      feature_vector.push_back( (uint8_t) val );
    }

    return true;
  }
}
