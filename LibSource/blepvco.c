/**
 ******************************************************************************
 * File Name          	: blepvco.c
 * Author				: Xavier Halgand + Sean Bolton
 * Date               	:
 * Description        	:
 ******************************************************************************
 */

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

#define _BSD_SOURCE    1
#define _SVID_SOURCE   1
#define _ISOC99_SOURCE 1

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "minblep_table.h"
#include "blepvco.h"
/**======================================================================================================**/

VCO_blepsaw_t	mbSawOsc _CCM_;

/**======================================================================================================**/
float_value_delta* step_dd_table;
float* slope_dd_table;


void place_step_dd(float *buffer, int index, float phase, float w, float scale)
{
	float r;
	int i;

	r = MINBLEP_PHASES * phase / w;
	i = lrintf(r - 0.5f);
	r -= (float)i;
	i &= MINBLEP_PHASE_MASK;  /* extreme modulation can cause i to be out-of-range */
	/* this would be better than the above, but more expensive:
	 *  while (i < 0) {
	 *    i += MINBLEP_PHASES;
	 *    index++;
	 *  }
	 */

	while (i < MINBLEP_PHASES * STEP_DD_PULSE_LENGTH) {
		buffer[index] += scale * (step_dd_table[i].value + r * step_dd_table[i].delta);
		i += MINBLEP_PHASES;
		index++;
	}
}
//----------------------------------------------------------------------------------------------------------

void place_slope_dd(float *buffer, int index, float phase, float w, float slope_delta)
{
	float r;
	int i;

	r = MINBLEP_PHASES * phase / w;
	i = lrintf(r - 0.5f);
	r -= (float)i;
	i &= MINBLEP_PHASE_MASK;  /* extreme modulation can cause i to be out-of-range */

	slope_delta *= w;

	while (i < MINBLEP_PHASES * SLOPE_DD_PULSE_LENGTH) {
		buffer[index] += slope_delta * (slope_dd_table[i] + r * (slope_dd_table[i + 1] - slope_dd_table[i]));
		i += MINBLEP_PHASES;
		index++;
	}
}
//----------------------------------------------------------------------------------------------------------

/* ==== hard-sync-capable sawtooth oscillator ==== */
void VCO_blepsaw_Init(VCO_blepsaw_t *vco)
{
	
  if(vco->_init == true)
    return;
  vco->_init = false;
	vco->amp = 1.0f;
	vco->freq = 440.f;
	vco->syncin = 0.0f;
	vco->_z = 0.0f;
	vco->_j = 0;
	memset (vco->_f, 0, (FILLEN + STEP_DD_PULSE_LENGTH) * sizeof (float));
  
  step_dd_table = (float_value_delta*)malloc(sizeof(float_value_delta)*(MINBLEP_PHASES * STEP_DD_PULSE_LENGTH + 1));
  slope_dd_table = (float*)malloc(sizeof(float)*(MINBLEP_PHASES * SLOPE_DD_PULSE_LENGTH + 1));
  
  //generate the blep discontinuity and slope discontinuity delta tables
  double t1, t2, a, d;
  double *shiftblep = minblep_table;
  /* Generate the step discontinuity delta table, which is the minBLEP minus the
   * 'ideal' unit step.  Note that since the step delta is discontinuous,
   * the common interpolation form:
   *
   *     value = sample[index] + fraction * (sample[index + 1] - sample[index])
   *
   * would fail at the discontinuity.  So instead, we store the table as
   * pairs are numbers, the first being the delta's value at a particular
   * index 'n', and the second being the difference between that value and
   * the limit of the delta's value as the index approaches n + 1.
   */
  for (int i = 0; i < period * oversampling; i++) {
  
    /* Alas, more empirically derived constants....  The full minBLEP is
     * period * oversampling (2048000) oversamples long, but we only need
     * the first part of it approximately as long as the window width
     * (4553) (approximate because the minimum-phase reconstruction smears
     * the window a bit.)  I examined the full step delta and found a zero
     * crossing at the 4606th oversample, beyond which the delta stays
     * below -150dB, and truncated the pulse there, which, padded with zeros,
     * yields a 72-sample pulse. */
    if (i <= 4605) {
      t1 = shiftblep[i];
      t2 = shiftblep[i + 1];
      if (i == 4605) t2 = 1.0;
      if (i >= 4 * oversampling) {  /* subtract the ideal step */
        t1 -= 1.0;
        t2 -= 1.0;
      }
      step_dd_table[i].value = t1;
      step_dd_table[i].delta = t2 - t1;
    } else if (i < 72 * 64) {
      /* pad remainder of last sample with zeros */
      step_dd_table[i].value = 0;
      step_dd_table[i].delta = 0;
    } else if (i == 72 * 64) {
      step_dd_table[i].value = 0;
      step_dd_table[i].delta = 0;
      /* guard point */
      break;
    }
  }
  /* Dump the slope discontinuity delta table, which is the integral of the
   * minBLEP minus the 'ideal' unit slope change.  While its slope, or first
   * differential, is discontinuous, the delta itself is continuous, so we
   * can just store the table as single values.
   */
  a = 0.0;
  for (int i = 0; i < period * oversampling; i++) {
    /* More emperically derived constants  (local minimum at 4513th
     * oversample, yielding a 71-sample pulse).... */
    if (i <= 4512) {

      a += shiftblep[i] / (double)oversampling;
      if (i < 4 * oversampling) {
          d = a;
      } else {
          d = a - (double)(i - 4 * oversampling) / (double)oversampling;   /* subtract ideal unit slope */
      }
      slope_dd_table[i] = d;

    } else if (i < 71 * 64) {
      /* pad remainder of last sample with zeros */
      slope_dd_table[i] = 0;
    } else if (i == 71 * 64) {
      slope_dd_table[i] = 0; /* guard point */
      break;
    }
  }
  vco->_init = true;
  
}

//----------------------------------------------------------------------------------------------------------

float VCO_blepsaw_SampleCompute(VCO_blepsaw_t *vco)
{
	int    j;
	float  freq, syncin;
	float  a, p, t, w, dw, z;
	syncin  = vco->syncin;
	freq = vco->freq;

	p = vco->_p;  /* phase [0, 1) */
	w = vco->_w;  /* phase increment */
	z = vco->_z;  /* low pass filter state */
	j = vco->_j;  /* index into buffer _f */

	if (vco->_init) {
		p = 0.5f;
		w = freq / SAMPLERATE;
		if (w < 1e-5) w = 1e-5;
		if (w > 0.5) w = 0.5;
		/* if we valued alias-free startup over low startup time, we could do:
		 *   p -= w;
		 *   place_slope_dd(_f, j, 0.0f, w, -1.0f); */
		vco->_init = false;
	}

	//a = 0.2 + 0.8 * vco->_port [FILT];
	a = 0.5f; // when a = 1, LPfilter is disabled

	t = freq / SAMPLERATE;
	if (t < 1e-5) t = 1e-5;
	if (t > 0.5) t = 0.5;
	dw = (t - w); // n= 1
	w += dw;
	p += w;

	if (syncin >= 1e-20f) {  /* sync to master */

		float eof_offset = (syncin - 1e-20f) * w;
		float p_at_reset = p - eof_offset;
		p = eof_offset;

		/* place any DD that may have occurred in subsample before reset */
		if (p_at_reset >= 1.0f) {
			p_at_reset -= 1.0f;
			place_step_dd(vco->_f, j, p_at_reset + eof_offset, w, 1.0f);
		}

		/* now place reset DD */
		place_step_dd(vco->_f, j, p, w, p_at_reset);

		vco->syncout = syncin;  /* best we can do is pass on upstream sync */

	} else if (p >= 1.0f) {  /* normal phase reset */

		p -= 1.0f;
		vco->syncout = p / w + 1e-20f;
		place_step_dd(vco->_f, j, p, w, 1.0f);

	} else {

		vco->syncout = 0.0f;
	}
	vco->_f[j + DD_SAMPLE_DELAY] += 0.5f - p;

	z += a * (vco->_f[j] - z); // LP filtering
	vco->out = vco->amp * z;

	if (++j == FILLEN)
	{
		j = 0;
		memcpy (vco->_f, vco->_f + FILLEN, STEP_DD_PULSE_LENGTH * sizeof (float));
		memset (vco->_f + STEP_DD_PULSE_LENGTH, 0,  FILLEN * sizeof (float));
	}

	vco->_p = p;
	vco->_w = w;
	vco->_z = z;
	vco->_j = j;

	return vco->out;
}

