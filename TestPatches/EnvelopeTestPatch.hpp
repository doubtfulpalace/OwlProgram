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


/* created by the OWL team 2013 */


////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef __EnvelopeTestPatch_hpp__
#define __EnvelopeTestPatch_hpp__

#include "Patch.h"
#include "Envelope.h"
#include "SineOscillator.hpp"

class EnvelopeTestPatch : public Patch {
public:
  static const unsigned int numEnvs = 2;
  AdsrEnvelope env[numEnvs];
  MultipointEnvelope *mp;
  FloatArray envBuffer;
  SineOscillator osc;
  EnvelopeTestPatch() 
  {
	int numPoints = 3;
	mp = MultipointEnvelope::create(numPoints);
    registerParameter(PARAMETER_A, "Attack");
    registerParameter(PARAMETER_B, "Decay");
    registerParameter(PARAMETER_C, "Sustain");
    registerParameter(PARAMETER_D, "Release");
    envBuffer = FloatArray::create(getBlockSize());
    env[0].setTimeBase(1); // run at sampling rate
    env[1].setTimeBase(getBlockSize()); //run at block rate
  }
  ~EnvelopeTestPatch(){
    FloatArray::destroy(envBuffer);
	MultipointEnvelope::destroy(mp);
  }
  void processAudio(AudioBuffer &buffer){
	float attack = getParameterValue(PARAMETER_A)*4;
	float decay = getParameterValue(PARAMETER_B)*4;
	float sustain = getParameterValue(PARAMETER_C);
	float release = getParameterValue(PARAMETER_D)*4;
    for(int n = 0; n < numEnvs; ++n){
      env[n].setAttack(attack);
      env[n].setDecay(decay);
      env[n].setSustain(sustain);
      env[n].setRelease(release);
    }
	mp->setPoint(0, attack, 1);
	mp->setPoint(1, decay, sustain);
	mp->setPoint(2, release, 0);

    FloatArray fa = buffer.getSamples(0);
    FloatArray fb = buffer.getSamples(1);
    static int lastButton = GREEN_BUTTON; //to avoid automatic triggering on startup
    int button;
    if(isButtonPressed(GREEN_BUTTON))
      button = GREEN_BUTTON;
    else 
      button = RED_BUTTON;
    
    // use next two lines to test gate. Gate will be on as long as the red light is on
    bool gate = (button == RED_BUTTON);
    for(int n = 0; n < numEnvs; ++n){
      env[n].gate(gate);
    }
	mp->gate(gate);
	
    
    // use next 10 lines to test trigger
    //bool trigger = (lastButton != button);
    //if(trigger){
	//  for(unsigned int n = 0; n < numEnvs; ++n){
    //    env[n].setRetrigger(false);
    //    env[n].trigger();
	//  }
	//  mp->setRetrigger(false);
	//  mp->trigger();
    //}
    //lastButton = button;
    
    osc.setFrequency(500);
    osc.getSamples(fa);
    fb.copyFrom(fa);
    //env[0] is at sampling rate 
    //env[0].getEnvelope(envBuffer);

	mp->getSamples(envBuffer);
    fa.multiply(envBuffer);
    
    // env[1] is at block rate, note the zipper noise ! 
    float amplitude = env[1].getNextSample();
    fb.multiply(amplitude);

    fa.multiply(0.2);
    fb.multiply(0.2);
  }
};

#endif // __EnvelopeTestPatch_hpp__
