#include "mex.h"
#include <iostream>
#include <string.h>
#include <emmintrin.h>

#include "../src/cDird.h"
#include "../src/cPlaceRecognizer.h"

using namespace std;
typedef unsigned char uint8_t;
    

void mexFunction (int nlhs,mxArray *plhs[],int nrhs,const mxArray *prhs[]) 
{

  // check arguments
  if (nrhs!=1)
  {
    mexErrMsgTxt("one arguement required");
  }

  if (!mxIsUint8(prhs[0]) || mxGetNumberOfDimensions(prhs[0])!=2)
  {
    mexErrMsgTxt("Input image must be a uint8_t matrix.");
  }

  uint8_t* input_data = (uint8_t*)mxGetPr(prhs[0]);
  const int *dims = mxGetDimensions(prhs[0]);

  static const int max_num_features = 100000; // adjust to your needs
  static const int dim_feature = DIRD::cDird::iDim_ * 16 /* assuming a tiling of 4x4 */; 
  int num_features = dims[1];

  if(dim_feature != dims[0])
  {
    mexErrMsgTxt("Input data contains features of wrong dimension");
  }

  // copy to sse memory
  uint8_t * feature_vectors = (uint8_t*)_mm_malloc(sizeof(uint8_t) *  max_num_features * dim_feature, 16); 
  memcpy( feature_vectors, input_data, sizeof(uint8_t) * dims[0] * dims[1] );
  
  // compute loop closures
  DIRD::cPlaceRecognizer place_recognizer( feature_vectors, num_features, dim_feature );
  cout << "Computing pairwise similarity" << "\n";
  if (!place_recognizer.computePairwiseSimilarity( 200 ))
  {
    mexErrMsgTxt("Computing pairwise similarities failed. Exiting.");
  }
  cout << "Computing dynamic programming sweep" << "\n";
  if (!place_recognizer.postProcessSimilarities( 20 ))
  {
    mexErrMsgTxt("Postprocessing similarities failed. Exiting.");
  }
  cout << "Non-maxima supression" << "\n";
  if (!place_recognizer.computeLoops( 60 ))
  {
    mexErrMsgTxt("Computing loops failed. Exiting.");
  }
    
  if (nlhs >= 1)
  {
    const int32_t DIMS[] = {3, place_recognizer.matLoopClosures_.size()};
    plhs[0] = mxCreateNumericArray(2,DIMS,mxDOUBLE_CLASS,mxREAL);
    double* loops_out = (double*)mxGetPr(plhs[0]);
    int k = 0;
    for (DIRD::cPlaceRecognizer::tSparseMatrixIterator iter = place_recognizer.matLoopClosures_.begin(); 
        iter != place_recognizer.matLoopClosures_.end(); 
        iter++)
    {
      // compute 2d index
      int j = iter->first % place_recognizer.matLoopClosures_.size_;
      int i = (iter->first - j) / place_recognizer.matLoopClosures_.size_;
      loops_out[k++] = i;
      loops_out[k++] = j;
      loops_out[k++] = iter->second;
    }
  }

  if (nlhs >= 2)
  {
    const int32_t DIMS[] = {3, place_recognizer.matDynamicProgramming_.size()};
    plhs[1] = mxCreateNumericArray(2,DIMS,mxDOUBLE_CLASS,mxREAL);
    double* loops_out = (double*)mxGetPr(plhs[1]);
    int k = 0;
    for (DIRD::cPlaceRecognizer::tSparseMatrixIterator iter = place_recognizer.matDynamicProgramming_.begin(); 
        iter != place_recognizer.matDynamicProgramming_.end(); 
        iter++)
    {
      // compute 2d index
      int j = iter->first % place_recognizer.matDynamicProgramming_.size_;
      int i = (iter->first - j) / place_recognizer.matDynamicProgramming_.size_;
      loops_out[k++] = i;
      loops_out[k++] = j;
      loops_out[k++] = iter->second;
    }
  }

  if (nlhs >= 3)
  {
    const int32_t DIMS[] = {3, place_recognizer.matSimilarity_.size()};
    plhs[2] = mxCreateNumericArray(2,DIMS,mxDOUBLE_CLASS,mxREAL);
    double* loops_out = (double*)mxGetPr(plhs[2]);
    int k = 0;
    for (DIRD::cPlaceRecognizer::tSparseMatrixIterator iter = place_recognizer.matSimilarity_.begin(); 
        iter != place_recognizer.matSimilarity_.end(); 
        iter++)
    {
      // compute 2d index
      int j = iter->first % place_recognizer.matSimilarity_.size_;
      int i = (iter->first - j) / place_recognizer.matSimilarity_.size_;
      loops_out[k++] = i;
      loops_out[k++] = j;
      loops_out[k++] = iter->second;
    }
  }

}
