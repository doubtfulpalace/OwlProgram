#ifndef __PatchProcessor_h__
#define __PatchProcessor_h__

#include <stdint.h>
#include "StompBox.h"
#include "device.h"
#include "message.h"

#if 0
class BasicParameter : public PatchParameter {
public:
  uint16_t value;
  float min = 0.0f;
  float max = 1.0f;
  void setRange(float mn, float mx){
    min = mn;
    max = mx;
  }
  float getValue(){
    return ((value/4096.0f) + min)*(max - min);
  }
};
#endif

class PatchProcessor {
public:  
  PatchProcessor();
  ~PatchProcessor();
  void clear();
  void setPatch(Patch* patch);
  float getParameterValue(PatchParameterId pid){
    ASSERT(pid < NOF_ADC_VALUES, "invalid parameter id");
    //    return parameters[pid].getValue();
    return parameters[pid];
  }
  int getBlockSize();
  double getSampleRate();
  AudioBuffer* createMemoryBuffer(int channels, int samples);
  void setParameterValues(uint16_t *parameters);
  Patch* patch;
  uint8_t index;
  /* PatchParameter getPatchParameter(PatchParameterId pid){ */
  /*   ASSERT(pid < NOF_ADC_VALUES, "invalid parameter id"); */
  /*   return PatchParameter(pid); */
  /*   //    return parameters[pid]; */
  /* } */
private:
  uint8_t bufferCount;
  AudioBuffer* buffers[MAX_BUFFERS_PER_PATCH];
  uint16_t parameters[NOF_ADC_VALUES];
};

#endif // __PatchProcessor_h__
