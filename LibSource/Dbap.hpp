#ifndef _Dbap_h_
#define _Dbap_h_
//#include "Patch.h"
#include "FloatArray.h"
#include <math.h>
#include <stdio.h>

class DbapSource{
public:
	DbapSource():
	DbapSource(6)
	{}

	DbapSource(float newRolloff):
	spread(0),
	ys(0),
	rSquared(0),
	rolloff(newRolloff)
	{
		computeA();
	}

	~DbapSource(){};

	void setSpread(float newSpread){
		rSquared = newSpread * newSpread;
	}

	void setPosition(float newX, float newY){
		xs = newX;
		ys = newY;
	}

	void setPositionPolar(float angle, float r){
		xs = r * cosf(angle);
		ys = r * sinf(angle);
	}

	void getAmplitudes(FloatArray& outputs, FloatArray speakersX, FloatArray speakersY){
		// Assumes all the FloatArrays have the same size.
		float sum = 0;
		for(unsigned int n = 0; n < outputs.getSize(); ++n){
			float xi = speakersX[n];
			float yi = speakersY[n];
			// use output as temporary storage 
			// for distances d_i
			float xDiff = xi - xs;
			float yDiff = yi - ys;
			outputs[n] = sqrtf(xDiff * xDiff + yDiff * yDiff + rSquared);
			sum += 1 / powf(outputs[n], 2 * a);
		}
		// k = \frac{1}{sqrt{sum{1}{N} \frac{1}{d_i^(2a)}}}
		float k = 1 / sqrtf(sum);
		for(unsigned int n = 0; n < outputs.getSize(); ++n){
			// v_i = \frac{k}{d_i^a}
			outputs[n] = k / powf(outputs[n], a);
		}
	}
private:
	float spread;
	float xs;
	float ys;
	float rSquared;
	float rolloff;
	float a;
	void computeA(){
		a = rolloff / (20 * log10f(2));
	}
};

class Dbap{
public:
	Dbap():
	sources(NULL),
	numSources(0),
	speakersX(NULL, 0),
	speakersY(NULL, 0)
	{}

	~Dbap(){
		dealloc();
	}

	void setNumSources(unsigned int newNumSources){
		dealloc();
		sources = new DbapSource[newNumSources];
		numSources = newNumSources;
	}

	void setSpeakers(FloatArray newSpeakersX, FloatArray newSpeakersY){
		// Argumnet check: pick the smallest of the two sizes to
		// ensure they have the same size.
		unsigned int size = speakersX.getSize() < speakersY.getSize() ? 
			speakersX.getSize() : speakersY.getSize();
		speakersX = FloatArray(newSpeakersX.getData(), size);
		speakersY = FloatArray(newSpeakersY.getData(), size);
	}

	void setSourcePosition(unsigned int source, float x, float y){
		if(source < numSources){
			sources[source].setPosition(x, y);
		}
	}

	void setSourcePosition(FloatArray sourceX, FloatArray sourceY){
		// TODO: maybe we should be less conservative and 
		// skip all this parameter check / realloc
		unsigned int size = sourceX.getSize() < sourceY.getSize() ?
			sourceX.getSize() : sourceY.getSize();
		if(size >= numSources){
			setNumSources(size);
		}
		for(unsigned int n = 0; n < sourceX.getSize(); ++n){
			sources[n].setPosition(sourceX[n], sourceY[n]);
		}
	}

	void setSpread(float spread){
		for(unsigned int n = 0; n < numSources; ++n){
			sources[n].setSpread(spread);	
		}
	}

	void setSourcePositionPolar(unsigned int source, float angle, float r){
		if(source < numSources){
			sources[source].setPositionPolar(angle, r);
		}
	}

	void getAmplitudes(unsigned int source, FloatArray output){
	// TODO: check that output.getSize() <= speakersX.getSize()
		if(source < numSources){
			sources[source].getAmplitudes(output, speakersX, speakersY);
		}
	}
private:
	void dealloc(){
		delete [] sources;
	}
	DbapSource* sources;
	unsigned int numSources;
	FloatArray speakersX;
	FloatArray speakersY;

};
#endif /*  _Dbap_h_ */
