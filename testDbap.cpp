#include "LibSource/dbap.hpp"
#include <stdio.h>

int main(){
	Dbap dbap;
	int numSources = 4;
	int numSpeakers = 4;
	FloatArray amplitudes = FloatArray::create(numSpeakers);
	FloatArray speakersX = FloatArray::create(numSpeakers);
	FloatArray speakersY = FloatArray::create(numSpeakers);
	speakersX[0] = 1;
	speakersY[0] = 1;
	speakersX[1] = -1;
	speakersY[1] = 1;
	speakersX[2] = -1;
	speakersY[2] = -1;
	speakersX[3] = 1;
	speakersY[3] = -1;
	
	dbap.setNumSources(numSources);
	dbap.setSpeakers(speakersX, speakersY);
	float xs, ys;
	
	int source;

	float spread;
	printf("With %u speakers in:\n", numSpeakers);
	for(unsigned int n = 0; n < numSpeakers; ++n){
		printf("[%u]: %.3f %.3f\n", n, speakersX[n], speakersY[n]);
	}
	for(float spread = 0; spread < 2; spread += 0.5){
		dbap.setSpread(spread);

		printf("\n\n==============spread: %.3f=====\n", spread);
		source = 0;
		xs = 0.99;
		ys = 0.99;
		dbap.setSourcePosition(source, xs, ys);
		dbap.getAmplitudes(source, amplitudes);
		printf("\nFor source in %.3f, gains: %.3f\n", xs, ys);
		printf("%.3f    %.3f\n\n", amplitudes[1], amplitudes[0]);
		printf("%.3f    %.3f\n\n", amplitudes[2], amplitudes[3]);

		source = 1;
		xs = 1;
		ys = 0;
		dbap.setSourcePosition(source, xs, ys);
		dbap.getAmplitudes(source, amplitudes);
		printf("For source in %.3f, %.3f, gains:\n", xs, ys);
		printf("%.3f    %.3f\n\n", amplitudes[1], amplitudes[0]);
		printf("%.3f    %.3f\n\n", amplitudes[2], amplitudes[3]);

		source = 2;
		xs = -1;
		ys = -0.9;
		dbap.setSourcePosition(source, xs, ys);
		dbap.getAmplitudes(source, amplitudes);
		printf("For source in %.3f, %.3f, gains:\n", xs, ys);
		printf("%.3f    %.3f\n\n", amplitudes[1], amplitudes[0]);
		printf("%.3f    %.3f\n\n", amplitudes[2], amplitudes[3]);
		
		source = 3;
		xs = -1;
		ys = 1;
		dbap.setSourcePosition(source, xs, ys);
		dbap.getAmplitudes(source, amplitudes);
		printf("For source in %.3f, %.3f, gains:\n", xs, ys);
		printf("%.3f    %.3f\n\n", amplitudes[1], amplitudes[0]);
		printf("%.3f    %.3f\n\n", amplitudes[2], amplitudes[3]);
	}	
	return 0;
}

