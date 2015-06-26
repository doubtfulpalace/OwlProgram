#ifndef __EfficientLoopPatch_hpp__
#define __EfficientLoopPatch_hpp__

#include "StompBox.h"
extern int doneFft;
class FastFourierTransform {
private:
  arm_rfft_fast_instance_f32 instance;

public:
  void init(int len){
    arm_rfft_fast_init_f32(&instance, len);
// Supported FFT Lengths are 32, 64, 128, 256, 512, 1024, 2048, 4096.
  }
  void fft(float* in, float* out){ //be careful: the input buffer will be messed up by the fft
    arm_rfft_fast_f32(&instance, in, out, 0);
  }
  void ifft(float* in, float* out){
    arm_rfft_fast_f32(&instance, in, out, 1);
  }
};
typedef enum { kDont=0, kDoFft=1, kDoProcess=2, kDoIfft=3} FftStatus;
#define numTdBuffers 4
class EfficientLoopPatch : public Patch {
private:
  FastFourierTransform transform;
  float *fdBuffer;
  float *tdBuffers[numTdBuffers];
  int fftSize;
  int numBuffers;
  int count;
  int nextInBuffer;
  int nextOutBuffer;
  int inPointers[8];
  int outPointers[8];
  float windowGain;
  float *window;
  FftStatus fftStatus;
  void resetPointers(){
    // countIn=0;
    for(int n=0; n<numBuffers; n++){
      inPointers[n]=-1;
      outPointers[n]=-1;
    }
    inPointers[0]=0; //activate the first buffer, to make sure we start storing samples immediately
    nextInBuffer=1;
    nextOutBuffer=0;
    fftStatus=kDont;
  }
public:
  EfficientLoopPatch(){
    fftSize=1024;
    transform.init(fftSize);
    AudioBuffer *audioBuffers=createMemoryBuffer(numTdBuffers, fftSize);
    for(int n=0; n<numTdBuffers; n++){
      tdBuffers[n]=audioBuffers->getSamples(n);
    }
    fdBuffer=createMemoryBuffer(1, fftSize)->getSamples(0);
    window=createMemoryBuffer(1, fftSize)->getSamples(0);
    for(int n=0; n<fftSize/2; n++){
      window[n]=n/(fftSize/2.0);
    }
    windowGain=2; //gain to compensate for the window's gain loss
    for(int n=fftSize/2; n<fftSize; n++){
      window[n]=(fftSize-n)/(fftSize/2.0); //triangular window
      // window[n]=1;//rectangular window
    }
    count=fftSize;
    registerParameter(PARAMETER_A, "Gain");
    registerParameter(PARAMETER_B, "Mode");
    registerParameter(PARAMETER_C, "nBins");
    registerParameter(PARAMETER_D, "Step");
  }
  void processAudio(AudioBuffer &buffer){
    float gain = getParameterValue(PARAMETER_A);
    float mode = getParameterValue(PARAMETER_B);
    float nBins = getParameterValue(PARAMETER_C);
    float* left = buffer.getSamples(0);
    float* right = buffer.getSamples(1);
    int size=getBlockSize();
    static int dropoutsCount=0;

    static int j=0; //write/read pointer in the respective buffer
    static int processing=0;//fft buffer to process
    static int processed=1;//fft buffer processed
    static int read=2;//fft buffer to read from
    static int write=3;//fft buffer to write to
    //do your I/O stuff first, e.g.: filling the fft buffer and reading from the one that has already been computed
    for(int n=0; n<size; n++){ //again, here we assume that j will never get past fftSize , that is the fftSize must be a multiple of blocksize
      tdBuffers[write][j+n]=left[n]; //write to fft buffer
      left[n]=tdBuffers[read][j+n]; //read from fft buffer and write to output;
      // try to save your hearing: clip first, and apply gain after
      left[n]=left[n]>1?1:left[n]; 
      left[n]=left[n]<-1?-1:left[n];
      left[n]*=gain;
    }
    j+=size;
        
    if(j==fftSize){ //if we got to the end of the fft buffer,
      //swap buffers, remember we initialized them as:
            // static int read=0;//fft buffer to read from
            // static int process=1;//fft buffer to process
            // static int write=2;//fft buffer to write to
      processing=write; //the buffer that was being written to becomes the buffer to process
      write=read;  //the buffer that was being read from will be overwritten by the new input signal
      read=processed; //the buffer that was being processed becomes the buffer to read from
      processed=processing;
      count=0; //reset processing pointer
      fftStatus=kDoFft; //schedule an fft
      j=0; //reset read/write pointer
      if(count!=fftSize){
        // if(dropoutsCount&127==1)
          // debugMessage("The processing did not finish on time",count,dropoutsCount);
        dropoutsCount++;
      }
    }
//the following block allows to split ffts and frequency-domain computations across multiple audioBlocks. We are using double buffering for ffts.
    //A single buffer could be used only if it takes less than an audio block to compute it.
    int fftCost=500; //cost in samples to be computed according to blockSize and fftSize, unit is costOfTheInKDoProcessBlock
    int available=2000; // should be smaller than floor((3500-costOfTheLinesAboveThis)/(costOfTheInKDoProcessBlock))
    static int currentProcess=0;
    float divide=1/(fftSize/2.0);
    while(available>=0){
      if(fftStatus==kDoFft && available>=fftCost){//if the fft was scheduled, do it
        transform.fft(tdBuffers[processing],fdBuffer);
        doneFft=getBlockSize();
        fftStatus=kDoProcess;
        available-=500;
        currentProcess=processing;
        continue;
      }
      if(fftStatus==kDoIfft && available>=fftCost){
        fdBuffer[0]=0;
        transform.ifft(fdBuffer,tdBuffers[processing]);
        fftStatus=kDont;
        if(processing==currentProcess){
          // debugMessage("yes");
        }
        else debugMessage("no");
        available-=500;
        break;
      }
      if(fftStatus==kDoProcess){
        //do the heavy, expensive transformation on fdBuffer[count]
        // float abc=count;
        // abc*=sqrtf(sqrtf(abc)); //just a way to wast time while keeping pristine audio quality
        // fdBuffer[0]=abc;
        fdBuffer[count]=sqrt(fdBuffer[count]*fdBuffer[count] + fdBuffer[count+1]*fdBuffer[count+1]);
        fdBuffer[count+1]=0;
        available--;
        count+=2; //count is a static variable, so at every call we can just over from where we left.
        if(count==fftSize){ //finished processing, schedule an ifft
          fftStatus=kDoIfft;
        }
        continue;
      }
      break;
      available--;
    } //end while
    //placing the following block here simplifies the code but requires the hopsize to be a multiple of the period size
  }
};

#endif // __EfficientLoopPatch_hpp__
