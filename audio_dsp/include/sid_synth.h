/*
 * MOS6581 driver code for register writes to a real or emulated SID chip.
 *
 * Code mostly taken from https://github.com/CharlotteGore/MOS6581
 * and adapted to drive FrankB's Teensy-reSID (https://github.com/FrankBoesing/Teensy-reSID)
 * License for CharlotteGore's MOS6581:
 * Copyright (c) 2013 Charlotte Gore
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef _SID_SYNTH_H
#define _SID_SYNTH_H

#include "reSID.h"
#include "sid_registers.h"

#define SID_NOISE 128
#define SID_SQUARE 64
#define SID_RAMP 32
#define SID_TRIANGLE 16
#define SID_TEST 8
#define SID_RING 20
#define SID_SYNC 66
#define SID_OFF 0

#define SID_3OFF 128
#define SID_FILT_HP 64
#define SID_FILT_BP 32
#define SID_FILT_LP 16
#define SID_FILT_OFF 0

#define SID_FILT_VOICE1 1
#define SID_FILT_VOICE2 2
#define SID_FILT_VOICE3 4
#define SID_FILT_EXT 8

class MOS6581
{
  AudioPlaySID *sid_ptr;

  // create some internal registers for binary manipulation
  byte voice1_register;
  byte voice2_register;
  byte voice3_register;
  byte filter_register;
  byte mode_register;

  void voiceFrequency(byte low, byte high, word frequency);
  void voicePulseWidth(byte low, byte high, word frequency);
  void _setFilterOnOff(byte voice, byte *reg, boolean on);
  void _setVoiceOnOff(byte *reg, boolean on);

public:
  MOS6581(AudioPlaySID *sid);
  void reset(void);
  void transfer(byte address, byte value);

  void setVoice(byte voice, boolean on);
  void setFilter(byte voice, boolean on);

  // fundamental frequency of waveform generator. 16bit number
  void setFrequency(byte voice, word frequency);

  // duty cycle of square waves. 12bit number
  void setPulseWidth(byte voice, word frequency);

  void setMode(byte voice, byte mode);

  void setADEnvelope(byte voice, byte attack, byte decay);
  void setSREnvelope(byte voice, byte sustain, byte release);

  // filter volume and output
  void volume(byte value);

  void setFilterMode(byte mode);

  // set the filter frequency. 11bit number
  void filterFrequency(word frequency);
  void filterResonance(byte resonance);
};

#endif
