/* blepvco - minBLEP-based, hard-sync-capable LADSPA VCOs.
 *
 * Copyright (C) 2004-2005 Sean Bolton.
 *
 * Much of the LADSPA framework used here comes from VCO-plugins
 * 0.3.0, copyright (c) 2003-2004 Fons Adriaensen.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef _BLEPVCO_H
#define _BLEPVCO_H

#include "FloatArray.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "minblep_table.h"
#include "message.h"

typedef struct { float value, delta; } float_value_delta;

class minBLEP_VCO
{
public:
  enum { FILLEN = 256 };

  minBLEP_VCO ();
  virtual void place_step_dd(float *buffer, int index, float phase, float w, float scale);
  virtual void place_slope_dd(float *buffer, int index, float phase, float w, float slope_delta);
  virtual ~minBLEP_VCO (void) {};
protected:
  static float_value_delta* step_dd_table;
  static float* slope_dd_table;
  static bool initialized;
};

class VCO_blepsaw : public minBLEP_VCO
{
public:

    enum { OUTP, SYNCOUT, FREQ, EXPM, LINM, SYNCIN, OCTN, TUNE, EXPG, LING, FILT, NPORT };

    VCO_blepsaw (): minBLEP_VCO(){};
    void active (bool act);
    //  virtual void runproc (unsigned long len, bool add);
    void getSamples (FloatArray outputBuffer);
    virtual ~VCO_blepsaw (void) {}
    void setSyncOutBuffer(float* buffer){
        syncOutBuffer = buffer;
    };
    void setSyncInBuffer(float* buffer){
        syncInBuffer = buffer;
    };
    void setFrequencyBuffer(float* buffer){
        frequencyBuffer = buffer;
    };
    void setExponentialModulationBuffer(float* buffer){
        exponentialModulationBuffer = buffer;
    };
    void setLinearModulationBuffer(float* buffer){
        linearModulationBuffer = buffer;
    };
    void setLinearGain(float gain){
        linearGain = gain;
    };
    void setExponentialGain(float gain){
        exponentialGain = gain;
    };
    void setOctave(float newOctave){
        octave = newOctave;
    };
    void setTune(float newTune){
        tune = newTune;
    };
private:
    
    float   *_port [NPORT];
    float   _p, _w, _z;
    float   _f [FILLEN + STEP_DD_PULSE_LENGTH];
    int     _j, _init;
    float linearGain;
    float exponentialGain;
    float octave;
    float tune;
    float* syncOutBuffer;
    float* syncInBuffer;
    float* frequencyBuffer;
    float* exponentialModulationBuffer;
    float* linearModulationBuffer;
};


#endif /* _BLEPVCO_H */
