#ifndef __BlepVcosPatch_hpp__
#define __BlepVcosPatch_hpp__

#include "StompBox.h"
#include "minblep_tables.c"
#include "blepvco.c"
class BlepVcosPatch : public Patch {
  //  VCO_blepsaw saw;
private:
  VCO_blepsaw_t saw;
  VCO_bleprect_t rec;
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
  BlepVcosPatch(): div(getSampleRate()) {
    registerParameter(PARAMETER_A, "tuneSaw");
    registerParameter(PARAMETER_B, "tuneRec");
    registerParameter(PARAMETER_C, "mix");
    registerParameter(PARAMETER_D, "gain");
    VCO_blepsaw_Init(&saw);
    VCO_bleprect_Init(&rec);
  }

  void processAudio(AudioBuffer &buffer){
    float tuneSaw = getParameterValue(PARAMETER_A)*3000 + 50;
    float tuneRec = getParameterValue(PARAMETER_B)*3000 + 50;
    float mix = getParameterValue(PARAMETER_C);
    float gain = getParameterValue(PARAMETER_D)*0.5;
    saw.freq = tuneSaw;
    rec.freq = tuneRec;
    float* left = buffer.getSamples(0);
    float volts = sample2volts(left[0]);
    //  float frequency = volts2hz(volts+tune);

    for(int n = 0; n < buffer.getSize(); n++){
      saw.syncin = rec.syncout;
      left[n] = VCO_blepsaw_SampleCompute(&saw) * (1 - mix) * gain;
      left[n] += VCO_bleprect_SampleCompute(&rec) * mix * gain;
    }
    debugMessage("frequency", tuneSaw, tuneRec);
  }
};

#endif // __BlepVcosPatch_hpp__
