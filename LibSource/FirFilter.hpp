#ifndef __FirFilter_h__
#define __FirFilter_h__

#include "FloatArray.h"
#ifdef ARM_CORTEX
#undef ARM_CORTEX
#endif
class FirFilter {
private:
  FloatArray coefficients;
  FloatArray states;
  int blockSize;
#ifdef ARM_CORTEX
  arm_fir_instance_f32 instance;
#else
  
#endif /* ARM_CORTEX */

  void processBlock(float* source, float* destination, int size){
#ifdef ARM_CORTEX
  arm_fir_f32(&instance, source, destination, size);
#else
  //naive implemementation
  float* y = destination;
  float* x = source;
  float* b = coefficients;
  for(int n = 0; n < size; n++){
    y[n] = 0;
    for (int i = 0; i < coefficients.getSize(); i++){
      y[n] += b[i] * x[n-i];
    }
  }
#endif /* ARM_CORTEX */
  }
  
public:
  FirFilter(){};
  
  FirFilter(int numTaps, int aBlockSize){
    init(numTaps,aBlockSize);
  };
  
  ~FirFilter(){
    FloatArray::destroy(coefficients);
  }

  void init(int numTaps, int aBlockSize){
    coefficients=FloatArray::create(numTaps);
    blockSize=aBlockSize;
    states=FloatArray::create(numTaps + blockSize - 1);
#ifdef ARM_CORTEX
    arm_fir_init_f32(&instance, coefficients.getSize(), coefficients.getData(), states.getData(), blockSize);
#else
    ASSERT(0, "TODO");
#endif /* ARM_CORTEX */
  }
  
  void processBlock(FloatArray buffer){
    ASSERT(buffer.getSize()<=blockSize, "Too large"); //TODO: check that in-place actually works properly
    processBlock(buffer.getData(), buffer.getData(), buffer.getSize());
  }
  
  void processBlock(FloatArray source, FloatArray destination){
    ASSERT(source.getSize()<=blockSize, "Too large");
    ASSERT(source.getSize()==destination.getSize(), "Sizes don't match");
    processBlock(source.getData(), destination.getData(), destination.getSize());
  }
  
  FloatArray* getCoefficients(){
    return &coefficients;
  };
  
  /**
    Copies coefficients value from an array.
  */
  void setCoefficients(FloatArray newCoefficients){
    ASSERT(coefficients.getSize()==newCoefficients.getSize(), "wrong size");
    coefficients.copyFrom(newCoefficients);
  }
  
  static FirFilter* create(int aNumTaps, int aMaxBlockSize){
    return new FirFilter(aNumTaps, aMaxBlockSize);
  }

  static void destroy(FirFilter* filter){
    delete filter->coefficients;
    delete filter->states;
    delete filter;
  }
};
#endif // __FirFilter_h__
