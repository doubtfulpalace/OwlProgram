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
    precision = 14;
    fastPow.setup(precision);
    registerParameter(PARAMETER_A, "toggleFast");
    registerParameter(PARAMETER_B, "pow1/2");
    registerParameter(PARAMETER_C, "computeError");
    registerParameter(PARAMETER_D, "precision");
  }
  ~FastTransTestPatch(){
  }
  void processAudio(AudioBuffer &buffer){
    static float maxError1 = -1; 
    bool fast = getParameterValue(PARAMETER_A) > 0.5;
    bool numbertwo = getParameterValue(PARAMETER_B) > 0.5;
    bool computeError = getParameterValue(PARAMETER_C) > 0.5;
    float p = getParameterValue(PARAMETER_D);
    static float pp = 0;
    
    static bool oldComputeError = false;
    FloatArray faL=buffer.getSamples(0);
    FloatArray faR=buffer.getSamples(1);
    if(!computeError){
      for(int k = 0; k < 15; k++){
        if(fast){
          if(!numbertwo){
            for(int n = 0; n < faL.getSize(); n++){
              float base = faR[n];
              float exponent = faL[n];
              fastPow.setBase(base);
              faR[n] = fastPow.getPow(exponent);
            }
            debugMessage("fast, setBase, precision", precision);
          } else { /* if(numbertwo) */ //use whatever the last base was
            for(int n = 0; n < faL.getSize(); n++){
              float base = faR[n];
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
      } /* for */      
    } else {
      if(oldComputeError == false){
        maxError1 = -1;
      }
      // this is just to evaluate accuracy and check that fastpowf and fastexpf work properly.
      // As such, we traverse only half of the array to make sure we do not exceed 100% CPU 
      static float eb, ee;
      faL.noise();
      faL.add(2);
      faR.noise();
      faR.add(2);
      float error1;
      float base;
      float exponent;
      for(int n = 0; n < faL.getSize()/4; n++){ 
        base = faR[n];
        exponent = faL[n];
        float exact = powf(base, exponent);
        float approx = fastPow.pow(base, exponent);
        error1 = fabsf(approx - exact) / exact * 100;
        if(maxError1 < error1 && base != 0){
          maxError1 = error1;
          eb = base;
          ee = exponent;
        }
        ASSERT(fastPow.pow(base, exponent) == fastpowf(base, exponent), "fastPow.getPow!=fastpowf");
        ASSERT(fastPow.pow(exp(1),exponent) == fastexpf(exponent), "fastPow.getPow!=fastexpf");
      }
      //  debugMessage("MaximumPercentageError, precision ", maxError1, eb*10000000, ee*10000000);
      debugMessage("MaximumPercentageError, precision ", maxError1, base, exponent);
    }
    oldComputeError = computeError;
  }
};

#endif // __FastTransTestPatch_hpp__
