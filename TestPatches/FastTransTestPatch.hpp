////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 
 
 LICENSE:
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.


*/

/* created by the OWL team 2016 */


////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef __FastTransTestPatch_hpp__
#define __FastTransTestPatch_hpp__

#include "StompBox.h"
#include "FastTrans.h"

class FastTransTestPatch : public Patch {
  FastPow fastPow;
  int precision;
public:
  FastTransTestPatch()
  {
    precision = 11;
    fastPow.setup(precision);
    registerParameter(PARAMETER_A, "toggleFast");
    registerParameter(PARAMETER_B, "toggleSetBase");
    registerParameter(PARAMETER_C, "computeError");
    registerParameter(PARAMETER_D, "precision");
  }
  ~FastTransTestPatch(){
  }
  void processAudio(AudioBuffer &buffer){
    static float maxError = -1; 
    bool fast = getParameterValue(PARAMETER_A) > 0.5;
    bool setBaseOn = getParameterValue(PARAMETER_B) > 0.5;
    bool computeError = getParameterValue(PARAMETER_C) > 0.5;
    float p = getParameterValue(PARAMETER_D);
    static float pp = 0;
    if(fabsf(pp-p) > 0.03){ //filter out noisy readings
      precision = p * 20;
      pp = p;
      fastPow.setup(precision);
    }
    
    static bool oldComputeError = false;
    FloatArray faL=buffer.getSamples(0);
    FloatArray faR=buffer.getSamples(1);
    for(int k = 0; k < 4; k++)
      if(fast){
        if(setBaseOn){
          for(int n = 0; n < faL.getSize(); n++){
            float base = faR[n];
            float exponent = faL[n];
            fastPow.setBase(base);
            faR[n] = fastPow.getPow(exponent);
          }
          debugMessage("fast, setBase, precision", precision);
        } else { /* if(setBaseOn) */ //use whatever the last base was
          for(int n = 0; n < faL.getSize(); n++){
            float exponent = faL[n];
            faR[n] = fastPow.getPow(exponent);
          }
          debugMessage("fast, noSetBase, precision", precision);
        }
      } else { /* if(fast) */
        for(int n = 0; n < faL.getSize(); n++){
          float base = faR[n];
          float exponent = faL[n];
          faR[n] = powf(base, exponent);
        }
        debugMessage("regular, precision", precision);
      }
      
    if(computeError){
      if(oldComputeError == false){
        maxError = -1;
      }
      // this is just to evaluate accuracy and check that fastpowf and fastexpf work properly
      // as such, we traverse only half of the array to make sure we do not exceed 100% CPU 
      for(int n = 0; n < faL.getSize()/2; n++){ 
        float base = faR[n];
        float exponent = faL[n];
        fastPow.setBase(base);
        float approx = fastPow.getPow(exponent);
        float exact = powf(base, exponent);
        float error = fabsf(approx - exact) / exact * 100;
        maxError = maxError > error ? maxError : error;
        if(precision == 14){ //14 is the default precision of fastpowf
          ASSERT(fastPow.pow(base, exponent) == fastpowf(base, exponent), "fastPow.getPow!=fastpowf");
          ASSERT(fastPow.pow(exp(1),exponent) == fastexpf(exponent), "fastPow.getPow!=fastexpf");
        }
        ASSERT(approx == fastPow.pow(base, exponent), "fastPow.getPow!=fastPow.pow");
      }
      debugMessage("MaximumPercentageError, precision ", maxError, precision);
    }
    oldComputeError = computeError;
  }
};

#endif // __FastTransTestPatch_hpp__
