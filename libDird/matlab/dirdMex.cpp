#include "mex.h"
#include <iostream>
#include <string.h>

#include "../src/cDird.h"

using namespace std;

typedef unsigned char uint8_t;
    
static DIRD::cDird * pDird = 0;

void mexFunction (int nlhs,mxArray *plhs[],int nrhs,const mxArray *prhs[]) {

  // read command
  char command[128];
  mxGetString(prhs[0],command,128);

  // init
  if (!strcmp(command,"process")) 
  {

    // check arguments
    if (nrhs!=2)
    {
      mexErrMsgTxt("one arguement required");
    }

    if (!mxIsUint8(prhs[1]) || mxGetNumberOfDimensions(prhs[1])!=2)
    {
      mexErrMsgTxt("Input image must be a uint8_t matrix.");
    }

    uint8_t* img_data = (uint8_t*)mxGetPr(prhs[1]);
    const int *dims = mxGetDimensions(prhs[1]);

    delete pDird;
    pDird = 0;

    pDird = new DIRD::cDird( dims[0], dims[1] );
    if (!pDird->process( img_data ))
    {
      cerr << "Couldnt pre-process image for DIRD extraction\n";
      return;
    }

  } 
  else if (!strcmp(command,"close")) 
  {
    delete pDird; 
    pDird = 0;
  } 
  else if (!strcmp(command,"get")) 
  {
    // check arguments
    if (nrhs!=3)
    {
      mexErrMsgTxt("two arguement required");
    }
    // check if inited. 
    if (!pDird)
    {
      mexErrMsgTxt("run process first");
    }

    int u = (int)*((double*)mxGetPr(prhs[ 1]));
    int v = (int)*((double*)mxGetPr(prhs[ 2]));

    vector<uint8_t> feature_vector;
    if (!pDird->get(u, v, feature_vector))
    {
      cerr << "Couldn't extract DIRD feature for pixel position " << u << " " <<  v << "\n";
      return;
    }

    const int32_t DIMS[] = {DIRD::cDird::iDim_};
    plhs[0] = mxCreateNumericArray(1,DIMS,mxDOUBLE_CLASS,mxREAL);
    double* feature_out = (double*)mxGetPr(plhs[0]);

    for (int d = 0; d <  DIRD::cDird::iDim_; ++d)
    {
      feature_out[d] = feature_vector[d];
    }

  }
    
}
