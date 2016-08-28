#ifndef ENVELOPE_HPP
#define ENVELOPE_HPP

#include "Patch.h"
#include "FloatArray.h"

class Envelope {
public:
  virtual void trigger(){
    trigger(true, 0);
  }
  virtual void trigger(bool state){
    trigger(state, 0);
  }
  virtual void trigger(bool state, int triggerDelay){}
  virtual void gate(bool state){
    gate(state, 0);
  }
  virtual void gate(bool state, int gateDelay){}
  virtual void setTimeBase(unsigned int newTimeBase){}
};

/**
 * Linear ADSR Envelope
 */
class AdsrEnvelope : public Envelope {
private:
  enum EnvelopeStage { kAttack, kDecay, kSustain, kRelease, kIdle };
  enum EnvelopeTrigger { kGate, kTrigger };

public:
  AdsrEnvelope();
  AdsrEnvelope(unsigned int timeBase);
  void setSampleRate(float sampleRate){
    samplePeriod = 1.0/sampleRate;
  }
  void setAttack(float newAttack);
  void setDecay(float newDecay);
  void setRelease(float newRelase);
  void setSustain(float newSustain);
  void setTimeBase(unsigned int timeBase);
  void trigger();
  void trigger(bool state);
  void trigger(bool state, int triggerDelay);
  void setRetrigger(bool on);
  void gate(bool state);
  void gate(bool state, int gateDelay);
  void setLevel(float newLevel);
  float getNextSample(); // increments envelope one step
  void getEnvelope(FloatArray output); // increments envelope by output buffer length
  void attenuate(FloatArray buf); // increments envelope by buffer length
private:
  static const float minTime;
  float samplePeriod;
  EnvelopeStage stage;
  EnvelopeTrigger trig;
  bool retrigger;
  float level;
  float attackIncrement;
  float decayIncrement;
  float releaseIncrement;
  float sustain;
  bool gateState;
  int gateTime;
  unsigned int timeBase;
};

class EnvelopeStage{
private:
  float value;
  float rate;
  float increment;
public:
  void setRate(float value);
  void setLevel(float value);
  void trigger();
  void getNextSample();
  void setTimeBase(unsigned int timeBase);
};

class MultipointEnvelope : public Envelope {
private:
  enum EnvelopeTrigger { kGate, kTrigger };
  unsigned int numStages;
  float* states;
  float currentLevel;
  int currentPoint;
  float samplePeriod;
  float getLevel(unsigned int point);
  float getIncrement(unsigned int point);
  float getSustainLevel();
  unsigned int sustainPoint;
  bool checkPointReached();
  EnvelopeTrigger trig;
  int gateTime;
  bool newGateState;
  bool gateState;
  bool retrigger;
  void updateCurrentLevel();
  unsigned int getSustainPoint();
public:
  MultipointEnvelope(float* data, unsigned int numStages);
  MultipointEnvelope(float* data, unsigned int numStages, unsigned int timeBase);
  static MultipointEnvelope* create(unsigned int points);
  static void destroy(MultipointEnvelope* pointer);

  void setRate(unsigned int point, float seconds);
  void setLevel(unsigned int point, float level);
  void setPoint(unsigned int point, float seconds, float level);
  void setSustainPoint(unsigned int point);

  /**
   * Sets the timebase for the envelope.
   *
   * After a call to setTimeBase(), all rates have to be reset.
   */
  void setTimeBase(unsigned int newTimeBase);

  void getSamples(FloatArray array);
  float getNextSample();

  void trigger();
  void trigger(bool state);
  void trigger(bool state, int triggerDelay);
  void setRetrigger(bool on);
  void gate(bool state);
  void gate(bool state, int gateDelay);
  void setLevel(float newLevel);
};
#endif /* ENVELOPE_HPP */
