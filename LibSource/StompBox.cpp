#include <cstddef>
#include <string.h>
#include "StompBox.h"
#include "device.h"
#include "ProgramVector.h"
#include "PatchProcessor.h"
#include "basicmaths.h"

AudioBuffer::~AudioBuffer(){}

template<>
PatchParameter<float>::PatchParameter(PatchParameterId id, const char* name, float min, float max)
  :pid(id), minValue(min), maxValue(max) {
  getProgramVector()->registerPatchParameter(pid, name);
}

template<>
PatchParameter<int>::PatchParameter(PatchParameterId id, const char* name, int min, int max)
  :pid(id), minValue(min), maxValue(max) {
  getProgramVector()->registerPatchParameter(pid, name);
}

// float PatchProcessor::getParameterValue(PatchParameterId pid){
//   if(pid < NOF_ADC_VALUES)
//     return parameterValues[pid]/4096.0f;
//   else
//     return 0.0f;
// }

// PatchParameter& PatchParameter::setRange(float mn, float mx){
//   min = mn;
//   max = mx;
//   return *this;
// }

template<>
float PatchParameter<float>::getValue(){
  uint16_t value = 0;
  if(pid < getProgramVector()->parameters_size)
    value = getProgramVector()->parameters[pid];
  return (value/4096.0f)*(maxValue - minValue) + minValue;
}

template<>
int PatchParameter<int>::getValue(){
  uint16_t value = 0;
  if(pid < getProgramVector()->parameters_size)
    value = getProgramVector()->parameters[pid];
  return value*(maxValue - minValue)/4096 + minValue;
}

PatchProcessor* getInitialisingPatchProcessor();

Patch::Patch() : processor(getInitialisingPatchProcessor()){
}

Patch::~Patch(){}

void Patch::registerParameter(PatchParameterId pid, const char* name){
  if(getProgramVector()->registerPatchParameter != NULL)
    getProgramVector()->registerPatchParameter(pid, name);
}

double Patch::getSampleRate(){
  return getProgramVector()->audio_samplingrate;
}

int Patch::getBlockSize(){
  return getProgramVector()->audio_blocksize;
}

float Patch::getParameterValue(PatchParameterId pid){
  return processor->getParameterValue(pid);
}

AudioBuffer* Patch::createMemoryBuffer(int channels, int samples){
  return AudioBuffer::create(channels, samples);
}

void Patch::setButton(PatchButtonId bid, bool pressed){
  if(pressed)
    getProgramVector()->buttons |= 1<<bid;
  else
    getProgramVector()->buttons &= ~(1<<bid);
}

bool Patch::isButtonPressed(PatchButtonId bid){
  return getProgramVector()->buttons & (1<<bid);
}

int Patch::getSamplesSinceButtonPressed(PatchButtonId bid){
  int index = bid+PARAMETER_F;
  return index <= getProgramVector()->parameters_size ? 
    getProgramVector()->parameters[index] : 0;
}

#define DWT_CYCCNT ((volatile unsigned int *)0xE0001004)

float Patch::getElapsedBlockTime(){
  return (*DWT_CYCCNT)/getBlockSize()/3500.0;
}

int Patch::getElapsedCycles(){
  return *DWT_CYCCNT;
}

#include "MemoryBuffer.hpp"
AudioBuffer* AudioBuffer::create(int channels, int samples){
  return new ManagedMemoryBuffer(channels, samples);
}
