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
  void gate(bool state);
  void gate(bool state, int gateDelay);
  void setLevel(float newLevel);
  //  void setstage(stage); jump to the given stage, optionally with delay
  float getSamples(); // increments envelope one step
  void getSamples(FloatArray &output); // increments envelope by buf.length
private:
  static constexpr float minTime = 0.005;
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
  int triggerTime;
  int triggerHold;
  int gateTime;
  float samplePeriod;
  int secondStage;
  bool selfTrigger;
  bool shouldStart;
  bool prevGateState;
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
  triggerTime = 1;
  gateTime = 1;
  gateState = false;
  shouldStart = false;
  prevGateState = false;
}
void AdsrEnvelope::setAttack(float newAttack){
  attack = newAttack > minTime ? newAttack : minTime;
  attackIncrement = samplePeriod / attack;
}
void AdsrEnvelope::setDecay(float newDecay){
  decay = newDecay > minTime ? newDecay : minTime;
  decayIncrement = - samplePeriod / decay;
}
void AdsrEnvelope::setRelease(float newRelease){
  release = newRelease > minTime ? newRelease : minTime;
  releaseIncrement = - samplePeriod / release;
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
void AdsrEnvelope::gate(bool state){
  gate(state, 0);
}
int flag = -1;
void AdsrEnvelope::gate(bool state, int gateDelay){
  prevGateState = gateState;
  gateState = state;
  if(prevGateState != gateState){
    flag = 1;
  }
  gateTime = -gateDelay;
}
void AdsrEnvelope::setLevel(float newLevel){
  level = newLevel;
}

void AdsrEnvelope::getSamples(FloatArray &output){
  for(int n = 0; n < output.getSize(); n++){
    output[n] = getSamples();
  }
  static int count = 0;
  count++;
  if(stage == kAttack){
    debugMessage("ATTACK, flag, susLevel, level", (float)(stage+((int)flag)*10), sustain, (float)level);
  }
  if(stage == kDecay){
    debugMessage("DECAY, flag, susLevel, level", (float)(stage+((int)flag)*10), sustain, (float)level);
  }

  if(stage == kSustain){
    debugMessage("SUSTAIN, flag, susLevel, level", (float)(stage+((int)flag)*10), sustain, (float)level);
  }
  if(stage == kRelease){
    debugMessage("RELEASE, flag, susLevel, level", (float)(stage+((int)flag)*10), sustain, (float)level);
  }

}
float AdsrEnvelope::getSamples(){
  
  // start an ADSR
  /*if(triggerTime == 0){
    if(triggerHold == 0){
      stage = kAttack;
      secondStage = kRelease;
    } else { // create a fictious gate 
      // TODO: maybe this should actually skip the decay?
      gateTime = 0;
      nextGateState = true;
    }
  }*/
  if(gateState == true && prevGateState == false){ //positive edge
    stage = kAttack;
    secondStage = kDecay;
  } else if (gateState == false && prevGateState == true) { //negative edge 
    stage = kRelease;
    flag = 2;
    //  lastStage = selfTrigger ? kAttack : kIdle; // TODO: implement retrigger
  }
  
  // stage transistions
  // reversed in order to make sure only one happens for each call
  if (stage == kRelease && level <= 0){
    stage = kIdle;
  }
  if(stage == kSustain && gateState == false){
    stage = kRelease;
    flag = 3;
  }
  if(stage == kDecay && level <= sustain){
    stage = kSustain;
  }
  if(stage == kAttack && level >= 1){
    stage = secondStage;
  }
    
  //  update the state variable
  switch (stage) {
  case kAttack:
    // attack ramp
    level += attackIncrement;
    if (level > 1)
      level = 1;
    break;
  case kDecay:
    // decay ramp
    level += decayIncrement;
    if(level < sustain)
      level = sustain;
    break;
  case kSustain:
    level = sustain; // TODO: in the real world, you would probably glide to the new sustain level at a rate determined by either decay or attack
    break;
  case kRelease :
    // release ramp
    level += releaseIncrement;
    if (level < 0)
      level = 0;
    break;
  case kIdle:
    level = 0;
    break;
  }
  // only update the times if we are waiting for an event
  // this prevents overflows when you leave your synth on
  //  overnight!  
  /*
  if(triggerTime <= triggerHold) 
    triggerTime ++;
  if(triggerTime == triggerHold){
    nextGateState = false;
    gateTime = 0;
  }
  */
  return level < 0 ? 0 : level;
}
#endif /* ENVELOPE_HPP */