#include "Envelope.h"
#include "message.h"

/*
void EnvelopeGenerator::calculateMultiplier(double startLevel,
                                            double endLevel,
                                            unsigned long long lengthInSamples) {
    multiplier = 1.0 + (log(endLevel) - log(startLevel)) / (lengthInSamples);
}
*/

const float AdsrEnvelope::minTime = 0.001;

AdsrEnvelope::AdsrEnvelope() : AdsrEnvelope(1) {}

AdsrEnvelope::AdsrEnvelope(unsigned int timeBase) : 
  stage(kIdle),
  trig(kGate),
  level(0.0),
  gateState(false),
  gateTime(-1),
  timeBase(1)
{
  setTimeBase(timeBase);
  setAttack(0.0);
  setDecay(0.0);
  setSustain(1.0);
  setRelease(0.0);
  setRetrigger(false);
}

void AdsrEnvelope::setAttack(float newAttack){
  newAttack = newAttack > minTime ? newAttack : minTime;
  attackIncrement = samplePeriod / newAttack;
}

void AdsrEnvelope::setDecay(float newDecay){
  newDecay = newDecay > minTime ? newDecay : minTime;
  decayIncrement = - samplePeriod / newDecay;
}

void AdsrEnvelope::setRelease(float newRelease){
  newRelease = newRelease > minTime ? newRelease : minTime;
  releaseIncrement = - samplePeriod / newRelease;
}

void AdsrEnvelope::setSustain(float newSustain){
  sustain = newSustain;
 // TODO: in the real world, you would probably glide to the new sustain level at a rate determined by either decay or attack
}

void AdsrEnvelope::setRetrigger(bool state){
  retrigger = state;
}

void AdsrEnvelope::trigger(){
  trigger(true, 0);
}

void AdsrEnvelope::trigger(bool state){
  trigger(state, 0);
}

void AdsrEnvelope::trigger(bool state, int delay){
  gate(state, delay);
  trig = kTrigger;
}

void AdsrEnvelope::gate(bool state){
  gate(state, 0);
}

void AdsrEnvelope::gate(bool state, int delay){
  if(gateState != state){
    gateTime = delay;
    gateState = state;
  }
  trig = kGate;
}

void AdsrEnvelope::setTimeBase(unsigned int newTimeBase){
  attackIncrement = attackIncrement * newTimeBase / timeBase;
  decayIncrement = decayIncrement * newTimeBase / timeBase;
  releaseIncrement = releaseIncrement * newTimeBase / timeBase;
  timeBase = newTimeBase;
  samplePeriod = timeBase / Patch::getSampleRate();
}

void AdsrEnvelope::setLevel(float newLevel){
  level = newLevel;
}

void AdsrEnvelope::attenuate(FloatArray output){
  for(int n = 0; n < output.getSize(); n++)
    output[n] *= getNextSample();
}

void AdsrEnvelope::getEnvelope(FloatArray output){
  for(int n = 0; n < output.getSize(); n++)
    output[n] = getNextSample();
}

float AdsrEnvelope::getNextSample(){
  if(gateTime == 0){
    stage = kAttack;
    if(trig == kTrigger){
      gateState = false;
    }
  }
  if(gateTime >= 0){
    gateTime--; // this will stop at -1
  }
  switch (stage) {
  case kAttack:
    // attack ramp
    level += attackIncrement;
    if(level >= 1.0){
      level = 1.0;
      stage = kDecay;
    } else if(gateState == false && trig == kGate){
      stage = kRelease;
    }
    break;
  case kDecay: // idle
    // decay ramp
    level += decayIncrement;
    if(level <= sustain){
      level = sustain;
      if(trig == kGate){
        stage = kSustain;
      } else { // (trig == kTrigger)
        stage = kRelease;
      }
    } else if(gateState == false && trig == kGate){
      stage = kRelease;
    }
    break;
  case kSustain:
    level = sustain;
    if(gateState == false){
      stage = kRelease;
    }
    break;
  case kRelease:
    // release ramp
    level += releaseIncrement;
    if(level <= 0.0){
      level = 0.0;
      if (retrigger == true)
        trigger();
      else // (retrigger == false)
        stage = kIdle;
    } else if(gateState == true ){ // if during release the gate is on again, start over from the current level
      stage = kAttack;
    }
    break;
  case kIdle:
    level = 0.0;
    if(gateState == true)
      stage = kAttack;
    break;
  }
  return level;
}

MultipointEnvelope* MultipointEnvelope::create(unsigned int points){
  FloatArray array = FloatArray::create(points * 2);
  return new MultipointEnvelope(array.getData(), points);
}

void MultipointEnvelope::destroy(MultipointEnvelope* pointer){
  FloatArray::destroy(FloatArray(pointer->states, pointer->numStages / 2));
  delete pointer;
}

MultipointEnvelope::MultipointEnvelope(float* data, unsigned int numStages) :
  MultipointEnvelope(data, numStages, 1)
{}

MultipointEnvelope::MultipointEnvelope(float* data, unsigned int numStages, unsigned int timeBase) :
   numStages(numStages), states(data), currentLevel(0.f), currentPoint(numStages), trig(kGate), gateTime(-1), gateState(false), retrigger(false), sustainPoint(numStages - 1)
{
  setTimeBase(timeBase);
}

void MultipointEnvelope::setTimeBase(unsigned int timeBase){
  samplePeriod = timeBase / Patch::getSampleRate();
}

void MultipointEnvelope::setRate(unsigned int point, float seconds){
  float start = point == 0 ? 0 : getLevel(point - 1);
  float stop = getLevel(point);
  float diff = stop - start; // this is negative when decreasing
  float increment;
  if(diff > 0)
    increment = samplePeriod / seconds;
  else
    increment = -samplePeriod / seconds;
  states[point * 2] = increment;
}

void MultipointEnvelope::setLevel(unsigned int point, float level){
  states[point * 2 + 1] = level;
}

float MultipointEnvelope::getIncrement(unsigned int point){
  return states[point  * 2];
}

float MultipointEnvelope::getLevel(unsigned int point){
  return states[point  * 2 + 1];
}

void MultipointEnvelope::setPoint(unsigned int point, float seconds, float level){
  setRate(point, seconds);
  setLevel(point, level);
}

void MultipointEnvelope::getSamples(FloatArray output){
  for(int n = 0; n < output.getSize(); ++n){
    output[n] = getNextSample();
  }
  debugMessage("state:", currentLevel, currentPoint);
  debugMessage("increments:", 1000*getIncrement(0), 1000*getIncrement(1), 1000*getIncrement(2));
}

float MultipointEnvelope::getNextSample(){
  if(gateTime == 0){
    gateState = newGateState;
    if(gateState == true){
      currentPoint = 0;
    } else {
      currentPoint = getSustainPoint();
    }
    if(trig == kTrigger){
      gateState = false;
    }
  }
  if(gateTime >= 0){
    --gateTime;
  }
  if(getSustainPoint() == currentPoint && gateState == true){ // the SUSTAIN
    currentLevel = getSustainLevel(); 
  } else if(currentPoint < numStages){ // any other stage
    updateCurrentLevel();
    if(checkPointReached()){
      currentLevel = getLevel(currentPoint);
      ++currentPoint;
      if(currentPoint == numStages){ // idle
        currentLevel = 0;
        if(retrigger == true){
          currentPoint = 0; // restart! 
        }
      }
    }
  }
  return currentLevel;
}

void MultipointEnvelope::updateCurrentLevel(){
  currentLevel += getIncrement(currentPoint);
}

float MultipointEnvelope::getSustainLevel(){
  return getLevel(sustainPoint - 1);
}

void MultipointEnvelope::setSustainPoint(unsigned int newSustainPoint){
  sustainPoint = newSustainPoint;
}

unsigned int MultipointEnvelope::getSustainPoint(){
  return sustainPoint;
}

bool MultipointEnvelope::checkPointReached(){
  float inc = getIncrement(currentPoint);
  if( // we are incrementing and we are above the level
    (inc > 0 && currentLevel >= getLevel(currentPoint)) 
     || // we are decrementing and we are below the level
    (inc <= 0 && currentLevel <= getLevel(currentPoint))
  ) 
    return true;
  else 
    return false;
}

void MultipointEnvelope::setRetrigger(bool state){
  retrigger = state;
}

void MultipointEnvelope::trigger(){
  trigger(true, 0);
}

void MultipointEnvelope::trigger(bool state){
  trigger(state, 0);
}

void MultipointEnvelope::trigger(bool state, int delay){
  gate(state, delay);
  trig = kTrigger;
}

void MultipointEnvelope::gate(bool state){
  gate(state, 0);
}

void MultipointEnvelope::gate(bool state, int delay){
  if(gateState != state){
    gateTime = delay;
    newGateState = state;
  }
  trig = kGate;
}
