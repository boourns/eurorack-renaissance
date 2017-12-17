/*
 * Copyright 2012 Google Inc.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define _USE_MATH_DEFINES
#include <math.h>

#include "synth.h"
#include "sin.h"

#define R (1 << 29)

#ifndef SIN_INLINE

const int SHIFT = 24 - SIN_LG_N_SAMPLES; // 14
int sin_lowbits;
int sin_phase_int;
int sin_dy, sin_y0;

int32_t Sin::lookup(int32_t phase) {
  return Sin::compute(phase);

  // #define SIN_LG_N_SAMPLES 10
  // #define SIN_N_SAMPLES (1 << SIN_LG_N_SAMPLES) == 0x
  // table size is SIN_N_SAMPLES << 1 == 2048
  sin_lowbits = phase & ((1 << SHIFT) - 1); // & 0x1fff
  // phase & ((1 << 10) - 1)
  // phase & ((1024) - 1) == phase & 1023 == phase & 0x3FF

#ifdef SIN_DELTA
  sin_phase_int = (phase >> (SHIFT - 1)) & ((SIN_N_SAMPLES - 1) << 1); 
  // phase >> 13 & ((1023) << 1)
  // phase >> 13 & 0x7FE

  // if (sin_phase_int < 0) { sin_phase_int = 0; }
  // if (sin_phase_int > 2047) { sin_phase_int = 2047; }
  sin_dy = sintab[sin_phase_int];
  //int dy = sintab[2046];

  //if (sin_phase_int + 1 > 2047) { sin_phase_int -= 2048; }
  sin_phase_int += 1;
  sin_y0 = sintab[sin_phase_int];

  return sin_y0 + (((int64_t)sin_dy * (int64_t)sin_lowbits) >> SHIFT);
  //return y0 + (((int32_t) ((float) dy * (float) lowbits)) >> SHIFT);

#else 
  int sin_phase_int = (phase >> SHIFT) & (SIN_N_SAMPLES - 1);
  int y0 = sintab[sin_phase_int];
  int y1 = sintab[sin_phase_int + 1];

  return y0 + (((int64_t)(y1 - y0) * (int64_t)sin_lowbits) >> SHIFT);
#endif
}
#endif


#if 0
// The following is an implementation designed not to use any lookup tables,
// based on the following implementation by Basile Graf:
// http://www.rossbencina.com/static/code/sinusoids/even_polynomial_sin_approximation.txt

#define C0 (1 << 24)
#define C1 (331121857 >> 2)
#define C2 (1084885537 >> 4)
#define C3 (1310449902 >> 6)

int32_t Sin::compute(int32_t phase) {
  int32_t x = (phase & ((1 << 23) - 1)) - (1 << 22);
  int32_t x2 = ((int64_t)x * (int64_t)x) >> 22;
  int32_t x4 = ((int64_t)x2 * (int64_t)x2) >> 24;
  int32_t x6 = ((int64_t)x2 * (int64_t)x4) >> 24;
  int32_t y = C0 -
    (((int64_t)C1 * (int64_t)x2) >> 24) +
    (((int64_t)C2 * (int64_t)x4) >> 24) -
    (((int64_t)C3 * (int64_t)x6) >> 24);
  y ^= -((phase >> 23) & 1);
  return y;
}
#endif

#if 1
// coefficients are Chebyshev polynomial, computed by compute_cos_poly.py
#define C8_0 16777216
#define C8_2 -331168742
#define C8_4 1089453524
#define C8_6 -1430910663
#define C8_8 950108533

int32_t Sin::compute(int32_t phase) {
  int32_t x = (phase & ((1 << 23) - 1)) - (1 << 22);
  int32_t x2 = ((int64_t)x * (int64_t)x) >> 16;
  int32_t y = (((((((((((((int64_t)C8_8
    * (int64_t)x2) >> 32) + C8_6)
    * (int64_t)x2) >> 32) + C8_4)
    * (int64_t)x2) >> 32) + C8_2)
    * (int64_t)x2) >> 32) + C8_0);
  y ^= -((phase >> 23) & 1);
  return y;
}
#endif

#define C10_0 (1 << 30)
#define C10_2 -1324675874  // scaled * 4
#define C10_4 1089501821
#define C10_6 -1433689867
#define C10_8 1009356886
#define C10_10 -421101352
int32_t Sin::compute10(int32_t phase) {
  int32_t x = (phase & ((1 << 29) - 1)) - (1 << 28);
  int32_t x2 = ((int64_t)x * (int64_t)x) >> 26;
  int32_t y = ((((((((((((((((int64_t)C10_10
    * (int64_t)x2) >> 34) + C10_8)
    * (int64_t)x2) >> 34) + C10_6)
    * (int64_t)x2) >> 34) + C10_4)
    * (int64_t)x2) >> 32) + C10_2)
    * (int64_t)x2) >> 30) + C10_0);
  y ^= -((phase >> 29) & 1);
  return y;
}
