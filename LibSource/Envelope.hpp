#ifndef ENVELOPE_HPP
#define ENVELOPE_HPP
#include "message.h"
enum{kAttack, kDecay, kSustain, kRelease, kIdle}; //TODO: make this private to AdsrEnvelope 

class AdsrEnvelope {
public:
  AdsrEnvelope(float newSampleRate);
  void setAttack(float newAttack);
  void setDecay(float newDecay);
  void setRelease(float newRelase);
  void setSustain(float newSustain);
  void trigger();
  void trigger(int triggerDelay);
  void gate(bool status);
  void gate(bool status, int gateDelay);
  void setLevel(float newLevel);
  //  void setstage(stage); jump to the given stage, optionally with delay
  float getSamples(); // increments envelope one step
  void getSamples(FloatArray &output); // increments envelope by buf.length
private:
  float level;
  int count;
  int stage;
  float attack;
  float attackIncrement;
  float decay;
  float decayIncrement;
  float release;
  float releaseIncrement;
  float sustain;
  bool gateState;
  bool nextGateState;
  int triggerTime;
  int triggerHold;
  int gateTime;
  float samplePeriod;
  int secondStage;
  bool selfTrigger;
};

AdsrEnvelope::AdsrEnvelope(float sampleRate){
  stage = kIdle;
  level = 0;
  samplePeriod = 1 / sampleRate;
  triggerHold = 0;
  attack = 0;
  decay = 0;
  sustain = 1;
  release = 0;
}
void AdsrEnvelope::setAttack(float newAttack){
  attack = newAttack;
  attackIncrement = attack * samplePeriod;
}
void AdsrEnvelope::setDecay(float newDecay){
  decay = newDecay;
  decayIncrement = - decay * samplePeriod;
}
void AdsrEnvelope::setRelease(float newRelease){
  release = newRelease;
  releaseIncrement = - release * samplePeriod;
}
void AdsrEnvelope::setSustain(float newSustain){
  sustain = newSustain;
}
void AdsrEnvelope::trigger(){
  trigger(0);
}
void AdsrEnvelope::trigger(int triggerDelay){
  triggerTime = -triggerDelay;
}
void AdsrEnvelope::gate(bool status){
  gate(status, 0);
}
void AdsrEnvelope::gate(bool status, int gateDelay){
  nextGateState = status;
  gateTime = -gateDelay;
}
void AdsrEnvelope::setLevel(float newLevel){
  level = newLevel;
}

void AdsrEnvelope::getSamples(FloatArray &output){
  for(int n = 0; n < output.getSize(); n++){
    output[n] = getSamples();
  }
  //  debugMessage("stage, level, decayIncrement", (float)stage, level, (float)gateState);
}

float AdsrEnvelope::getSamples(){
  
  // start an ADSR
  if(triggerTime == 0){
    if(triggerHold == 0){
      stage = kAttack;
      secondStage = kRelease;
    } else { // create a fictious gate 
      // TODO: maybe this should actually skip the decay?
      gateTime = 0;
      nextGateState = true;
    }
  }
  if(gateTime == 0){
    gateState = nextGateState;
    if(gateState == true){
      stage = kAttack;
      secondStage = kDecay;
    } else {
      stage = kRelease;
      //  secondStage = selfTrigger ? kAttack : kIdle; // TODO: implmement retrigger
    }
  }
  
  // state transistions
  if(stage == kAttack && level >= 1){
    stage = secondStage;
    debugMessage ("stage: ", stage);
  }
  if(stage == kDecay && level <= sustain){
    stage = kSustain;
  }
  if (stage == kRelease && level <= 0){
    stage = kIdle;
  }
    
  if(stage == kAttack){
    // attack ramp
    level += attackIncrement;
  }
  if (stage == kDecay){
    // decay ramp
    level += decayIncrement;
  } 
  if (stage == kSustain){
    level = sustain; // TODO: in the real world, you would probably get to sustain at a rate determined by either decay or attack
  }
  if (stage == kRelease ){
    // release ramp
    level += releaseIncrement;
  }
  if (stage == kIdle){
    level = 0;
  }
  // only update the times if we are waiting for an event
  // this prevents overflows when you leave your synth on
  //  overnight!  
  if(gateTime <= 0)
    gateTime ++;
  if(triggerTime <= triggerHold) 
    triggerTime ++;
  if(triggerTime == triggerHold){
    nextGateState = false;
    gateTime = 0;
  }
  return level < 0 ? 0 : level;
}
#endif /* ENVELOPE_HPP */