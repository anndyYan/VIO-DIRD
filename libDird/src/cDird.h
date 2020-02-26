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

#pragma once

#if _MSC_VER <= 1500
typedef unsigned char uint8_t;
#else
#include <stdint.h>
#endif

#include <vector>

namespace DIRD
{

  /*@class cDird
   *
   * This class can extract DIRD features from an image.
   * Dird is an Illumination Robust Descriptor 
   *
   * First the entire image from which features shall be extracted
   * needs to be processed (see processes()).
   * Thereafter a feature vector can be computed for any pixel position
   * of the image (see get()).
   *
   */
  class cDird
  {

    public: /* public classes/enums/types etc... */

    public: /* public methods */

      /**
       * construct a cDird object from scratch
       */
      cDird(int width, int height);

      /**
       * destruct a cDird object
       */
      ~cDird();

      /**
       * @brief pre-processes the entire image for quick feature computation thereafter (see get()-method)
       * @return true if processing went ok, false otherwise 
       * @param img_data a 2d array of size as specified during construction (see c'tor) 
       */
      bool process( uint8_t * img_data );

      /**
       * @brief computes a DIRD feature vector for specified pixel position. Can only be called after process(). 
       * @return true if processing went ok, false otherwise 
       * @param u column index
       * @param v row index
       * @param feature_vector feature vector for pixel position (u,v)
       */
      bool get( int u, int v, std::vector<uint8_t> & feature_vector );

      /**
       * @brief compute linear index
       * @return linear index into image
       * @param u column index
       * @param v row index
       */
      inline long getIdx( int u, int v ) 
      {
        return v * iWidth_ + u; 
      }


    public: /* attributes */

      /**
       * @brief width of the expected image
       */
      int iWidth_;

      /**
       * @brief height of the expected image
       */
      int iHeight_;

      /**
       * @brief number of decomposition levels for the wavelet features 
       * IMPORTANT: if you change the values below then the parameters
       * of the logistic function in cPlaceRecognizer.cpp need to be adjusted!
       */
      static const int numLevels_ = 4;

      /**
       * @brief dimension of the feature vector
       */
      static const int iDim_ = numLevels_ * 9 * 6;

      /**
       * @brief the integral image of the input image
       */
      long * img_integral;

      /**
       * @brief a stack of wavelet transformed images
       */
      double ** arrWaveletStack_;

      /**
       * @brief safety flag whether image has been pre-processed or not
       */
      bool bIsProcessed_;

      /**
       * @brief this is the final feature vector for a pixel position _before_ squeezed into an uint8_t
       */
      double * featureVector_;

  };
}
