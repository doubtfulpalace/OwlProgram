#ifndef __BlepVcoSawPatch_hpp__
#define __BlepVcoSawPatch_hpp__

#include "StompBox.h"
#include "minblep_tables.c"
#include "blepvco.c"
class BlepVcoSawPatch : public Patch {
  //  VCO_blepsaw saw;
private:
  VCO_blepsaw_t vco;
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
    registerParameter(PARAMETER_A, "tune");
    registerParameter(PARAMETER_B, "detune");
    registerParameter(PARAMETER_C, "mix");
    registerParameter(PARAMETER_D, "gain");
    //  saw.setSyncOutBuffer(dummy);
    //  saw.setSyncInBuffer(dummy);
    //  saw.setFrequencyBuffer(dummy);
    //  saw.setExponentialModulationBuffer(dummy);
    //  saw.setLinearModulationBuffer(dummy);
    //  saw.setLinearGain(0);
    //  saw.setExponentialGain(0);
    //  saw.setOctave(2);
    //  saw.setTune(0);
    //  saw.active(true);
    VCO_blepsaw_Init(&vco);
  }

  void processAudio(AudioBuffer &buffer){
    float tune = getParameterValue(PARAMETER_A)*20000;
    float detune = getParameterValue(PARAMETER_B);
    float mix = getParameterValue(PARAMETER_C);
    float gain = getParameterValue(PARAMETER_D);
    vco.freq = tune;
    gain = gain*gain*1.2;
    gain *= 0.2+mix*0.8;
    float* left = buffer.getSamples(0);
    // saw.setMix(mix);
    // saw.setDetune(detune);
    float volts = sample2volts(left[0]);
    float frequency = volts2hz(volts+tune);
    // saw.setFrequency(frequency);
    //  saw.setOctave(detune*3);
    //  saw.setTune(tune);
    //  saw.getSamples(left);

    for(int n = 0; n < buffer.getSize(); n++){
      left[n] = VCO_blepsaw_SampleCompute(&vco) * gain;
    }
    debugMessage("frequency", tune);
  }
};

#endif // __BlepVcoSawPatch_hpp__
