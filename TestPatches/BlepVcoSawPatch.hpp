#ifndef __BlepVcoSawPatch_hpp__
#define __BlepVcoSawPatch_hpp__

#include "StompBox.h"

class BlepVcoSawPatch : public Patch {
  VCO_blepsaw saw;
private:
  const float VOLTAGE_MULTIPLIER = -4.40f;
  const float VOLTAGE_OFFSET = -0.0585f;
  const float div;
  float pos;
  float sample2volts(float s){
    return (s-VOLTAGE_OFFSET) * VOLTAGE_MULTIPLIER;
  }
  float volts2hz(float v){
    return 440.f * powf(2, v);
  }
public:
  BlepVcoSawPatch(): div(getSampleRate()) {
    //  saw.setSampleRate(getSampleRate());
    registerParameter(PARAMETER_A, "gain");
    FloatArray dummy = FloatArray::create(getBlockSize());
    dummy.setAll(0);
    saw.setSyncOutBuffer(dummy);
    saw.setSyncInBuffer(dummy);
    saw.setFrequencyBuffer(dummy);
    saw.setExponentialModulationBuffer(dummy);
    saw.setLinearModulationBuffer(dummy);
    saw.setLinearGain(0);
    saw.setExponentialGain(0);
    saw.setOctave(2);
    saw.setTune(0);
    saw.active(true);
  }

  void processAudio(AudioBuffer &buffer){
    float tune = getParameterValue(PARAMETER_A)*6.0 - 3.0;
    float detune = getParameterValue(PARAMETER_B);
    float mix = getParameterValue(PARAMETER_C);
    float gain = getParameterValue(PARAMETER_D);
    gain = gain*gain*1.2;
    gain *= 0.2+mix*0.8;
    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    // saw.setMix(mix);
    // saw.setDetune(detune);
    float volts = sample2volts(left[0]);
    float frequency = volts2hz(volts+tune);
    // saw.setFrequency(frequency);
    saw.setTune(tune);
    saw.getSamples(left);
  }
};

#endif // __BlepVcoSawPatch_hpp__
