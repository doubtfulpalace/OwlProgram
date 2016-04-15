#ifndef __DualSynthVoicePatch_hpp__
#define __DualSynthVoicePatch_hpp__

#include "StompBox.h"
#include "Envelope.h"
#include "VoltsPerOctave.h"
#include "PolyBlepOscillator.h"
#include "BiquadFilter.h"

/*
 * A: Pitch
 * B: Fc exp
 * C: Resonance
 * D: Envelope / amplitude
 * E: Waveshape
 * Left: Pitch
 * Right: Fc lin
 */
class DualSynthVoicePatch : public Patch {
private:
  PolyBlepOscillator osc1;
  PolyBlepOscillator osc2;
  VoltsPerOctave hz1;
  VoltsPerOctave hz2;
  FloatArray temp;
public:
  DualSynthVoicePatch() : osc1(getSampleRate()), osc2(getSampleRate()) {
    registerParameter(PARAMETER_A, "Tune1");
    registerParameter(PARAMETER_B, "Tune2");
    registerParameter(PARAMETER_C, "Cross-modulation");
    registerParameter(PARAMETER_D, "Waveform");
    temp = FloatArray::create(getBlockSize());
  }
  ~DualSynthVoicePatch(){
    FloatArray::destroy(temp);
  }
  void processAudio(AudioBuffer &buffer) {
    float tune1 = getParameterValue(PARAMETER_A)*10.0 - 6.0;
    float tune2 = getParameterValue(PARAMETER_B)*10.0 - 6.0;
    float modulationIndex = (getParameterValue(PARAMETER_C));
    modulationIndex = modulationIndex < 0.1 ? 0 : (modulationIndex - 0.1) / 0.9;
    float shape = getParameterValue(PARAMETER_D) * 2.0;
    float pw = 0.5;
    if(shape > 1.0){
      pw += 0.49*(shape-1.0); // pw 0.5 to 0.99
      shape = 1.0; // square wave
    }
    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    FloatArray right = buffer.getSamples(RIGHT_CHANNEL);

    hz1.setTune(tune1);
    osc1.setShape(shape);
    osc1.setPulseWidth(pw);

    hz2.setTune(tune2);
    osc2.setShape(shape);
    osc2.setPulseWidth(pw);
    
    for(int n = 0; n < left.getSize(); n++){
      // osc2 is frequency modulated by the right input
      osc2.setFrequency(hz2.getFrequency(right[n]));
      right[n] =  osc2.getNextSample();
      
      // osc1 is frequency modulated by osc2 and left input
      osc1.setFrequency(hz1.getFrequency(left[n] + 2.5*modulationIndex*right[n]));
      left[n] = osc1.getNextSample();
    }
    right.add(left);
  }
};

#endif   // __DualSynthVoicePatch_hpp__
