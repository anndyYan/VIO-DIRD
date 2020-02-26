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

#include <stdlib.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include <map>

// check for the new c++11 standard
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#include <unordered_map>
#endif

#include <emmintrin.h>

namespace DIRD
{
  class cPlaceRecognizer
  {

    public: /* public classes/enums/types etc... */

      // check for the new c++11 standard
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
      typedef std::unordered_map<long,float> tMap;
#else
      typedef std::map<long,float> tMap;
#endif

      // some homegrown/lightweight sparse matrix type
      struct tSparseMatrix
        : public tMap 
      {

        tSparseMatrix( int size )
          : size_(size), tMap()
        {

        }

        /** read a sparse matrix from text file */
        tSparseMatrix( std::string file_name )
          : tMap(),size_(0)
        {

          std::ifstream file( file_name.c_str() );
          if (file.is_open())
          {
            std::string line;
            getline (file,line);
            size_ = atoi(line.c_str());
            while (file.good())
            {
              std::string word,line;
              getline (file,line);
              std::istringstream iss(line, std::istringstream::in);

              iss >> word;
              int i = atoi(word.c_str());
              iss >> word;
              int j = atoi(word.c_str());
              iss >> word;
              float f = (float)atof(word.c_str());

              if ( f>0.0000001 )
              {
                (*this)(i,j) = f;
              }

            }
          }
        }

        /**
         * @brief computes a (debug) image of the matrix
         * @return true on success, false otherwise
         * @param img a pre-allocated array of size size*size
         * @param img_size size of image (=width=height)
         */
        bool toImage( uint8_t * img, int img_size );

        /**
         * @brief dumps the matrix to a text file (ASCII format)
         * @return true on success, false otherwise
         * @param file_name name of file
         */
        bool toFile( std::string file_name );

        /**
         * @brief translate a 2d matrix index into 1d
         * @return linear index
         * @param i index 1
         * @param j index 2
         */
        inline long getIdx( int i, int j )
        {
          return i * size_ + j;
        }

        /**
         * @brief 2d index operator
         * @return value at position (i,j) 
         * @param i index 1
         * @param j index 2
         */
        inline float & operator()( int i, int j )
        {
          return (*this)[ getIdx(i,j) ];
        }

        /**
         * @brief safely reads matrix element. A default value is returned if it doesnt exist
         * @return value at matrix position if it exists, default value otherwise
         * @param i index 1
         * @param j index 2
         * @param default_value value which is returned if index doesnt exist
         */
        inline float at( int i, int j, int default_value = 0 )
        {
          long idx = getIdx(i,j);
          iterator iter = find(idx);
          if ( iter == end() )
          {
            return (float)default_value;
          }
          return iter->second;
        }

        /**
         * @brief size (=width=height) of matrix
         */
        int size_;


      };

      typedef tSparseMatrix::iterator tSparseMatrixIterator;

    public: /* public methods */

      /**
       * construct a cPlaceRecognizer object from scratch
       */
      cPlaceRecognizer( uint8_t * feature_vectors, int num_features, int dim_feature );

      /**
       * destruct a cPlaceRecognizer object
       */
      ~cPlaceRecognizer();

      /**
       * @brief compute the similarity between any two poses
       * @return true on success, false otherwise
       * @param safety_margin minimum number of frames for a loop (say 100 or so) 
       */
      bool computePairwiseSimilarity( int safety_margin );

      /**
       * @brief computes all pairs of features that belong to the same place
       * @return true on success, false otherwise
       * @param segment_length length of segments that shall be matched 
       */
      bool postProcessSimilarities( int segment_length );

      /**
       * @brief computes loops from post processed similarity matrix (basically a non-maxima supression)
       * @return true on success, false otherwise
       * @param non_max size of non-maxima supression region in [number of places] 
       */
      bool computeLoops( int non_max );

      /**
       * @brief computes the distance (SAD) between two feature vectors
       * @return distance between two feature vectors
       * @param i index of first feature vector
       * @param j index of second feature vector
       */
      inline long dist( int i, int j )
      {
        long sum = 0;
        uint8_t * feature1 = &feature_vectors_[ i * dim_feature_ ];
        uint8_t * feature2 = &feature_vectors_[ j * dim_feature_ ];

        // some SSE magic
        // inspired by code of libViso2 (http://www.cvlibs.net/software/libviso2.html)
        __m128i xmm1_1, xmm1_2;
        __m128i xmm2_1, xmm2_2;
        for (int k = 0; k < dim_feature_; k+=32)
        {

          xmm1_1 = _mm_load_si128((__m128i*)&feature1[k]);
          xmm2_1 = _mm_load_si128((__m128i*)&feature2[k]);
          xmm2_1 = _mm_sad_epu8 (xmm1_1, xmm2_1);

          xmm1_2 = _mm_load_si128((__m128i*)&feature1[k+16]);
          xmm2_2 = _mm_load_si128((__m128i*)&feature2[k+16]);
          xmm2_2 = _mm_sad_epu8 (xmm1_2, xmm2_2);

          xmm2_2 = _mm_add_epi16(xmm2_1, xmm2_2);

          sum += _mm_extract_epi16(xmm2_2,0) + _mm_extract_epi16(xmm2_2,4);

        }

        return sum;
      }



    public: /* attributes */

      /**
       * @brief a pointer to the chunk of feature vectors (each describing one place)
       */
      uint8_t * feature_vectors_;

      /**
       * @brief dimension of one feature vector in feature_vectors_
       */
      int dim_feature_;

      /**
       * @brief a matrix storing pairwise similarities 
       */
      tSparseMatrix matSimilarity_;

      /**
       * @brief a matrix storing matched segment scores
       */
      tSparseMatrix matDynamicProgramming_;

      /**
       * @brief a matrix storing nonzero entries only for detected loops
       */
      tSparseMatrix matLoopClosures_;

      /**
       * @brief number of feature vectors (= number of places)
       */
      int num_features_;



  };

}
