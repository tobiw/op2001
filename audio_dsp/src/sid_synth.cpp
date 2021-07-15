/*
 * MOS6581 driver code for register writes to a real or emulated SID chip.
 *
 * Code mostly taken from https://github.com/CharlotteGore/MOS6581
 * and adapted to drive FrankB's Teensy-reSID (https://github.com/FrankBoesing/Teensy-reSID)
 * License for CharlotteGore's MOS6581:
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "sid_synth.h"

MOS6581::MOS6581(AudioPlaySID *sid) {
  sid_ptr = sid;
  voice1_register = B00100000;
  voice2_register = B00100000;
  voice3_register = B00100000;
  filter_register = B00000000;
  mode_register = B00000000;
}

void MOS6581::reset(){
  // iterate through all the registers and reset them
  for(int i = 0; i < 25; i++){
    transfer(i, 0);
  }
}

void MOS6581::transfer(byte address, byte value){
  sid_ptr->setreg(address, value);
}

void MOS6581::voiceFrequency(byte lowAddress, byte highAddress, word frequency) {
  transfer(lowAddress, lowByte(frequency));
  transfer(highAddress, highByte(frequency));

}

void MOS6581::voicePulseWidth(byte lowAddress, byte highAddress, word frequency) {
  transfer(lowAddress, lowByte(frequency));
  transfer(highAddress, highByte(frequency) & B00001111);
}

void MOS6581::setFrequency(byte voice, word frequency){
  if (voice == 0) {
    voiceFrequency(SID_V1_FL, SID_V1_FH, frequency);
  } else if (voice == 1) {
    voiceFrequency(SID_V2_FL, SID_V2_FH, frequency);
  } else if (voice == 2) {
    voiceFrequency(SID_V3_FL, SID_V3_FH, frequency);
  }
}

void MOS6581::setPulseWidth(byte voice, word frequency) {
  if (voice == 0) {
    voicePulseWidth(SID_V1_PWL, SID_V1_PWH, frequency);
  } else if (voice == 1) {
    voicePulseWidth(SID_V2_PWL, SID_V2_PWH, frequency);
  } else if (voice == 2) {
    voicePulseWidth(SID_V3_PWL, SID_V3_PWH, frequency);
  }
}

void MOS6581::filterFrequency(word frequency) {
  // shift the number along 5 bits to get the high frequency
  transfer(SID_FL_FL, lowByte(frequency) & B00000111);
  transfer(SID_FL_FH, highByte(frequency << 5));
}

void MOS6581::filterResonance(byte frequency) {
  filter_register &= B00001111;
  filter_register |= (frequency << 4);
  transfer(SID_FL_RES_CT, filter_register);
}

void MOS6581::_setFilterOnOff(byte voice, byte *reg, boolean on) {
  if (on) {
    *reg |= voice;  
  } else {
    *reg &= ~voice;
  }
}

void MOS6581::setFilter(byte voice, boolean on) {
  if(voice == 0) {
    _setFilterOnOff(SID_FILT_VOICE1, &filter_register, on);
  } else if (voice == 1) {
    _setFilterOnOff(SID_FILT_VOICE2, &filter_register, on);
  } else if (voice == 2) {
    _setFilterOnOff(SID_FILT_VOICE3, &filter_register, on);
  }
  transfer(SID_FL_RES_CT, filter_register);
}

void MOS6581::_setVoiceOnOff(byte *reg, boolean on) {
  if (on) {
    *reg |= 1;
  } else {
    *reg &= 0xfe; // ~1
  }
}

void MOS6581::setVoice(byte voice, boolean on) {
  if(voice == 0) {
    _setVoiceOnOff(&voice1_register, on);
    transfer(SID_V1_CT, voice1_register);
  } else if (voice == 1) {
    _setVoiceOnOff(&voice2_register, on);
    transfer(SID_V2_CT, voice2_register);
  } else if (voice == 2) {
    _setVoiceOnOff(&voice3_register, on);
    transfer(SID_V3_CT, voice3_register);
  }
}

void MOS6581::setMode(byte voice, byte mode) {
  if (voice == 0) {
    voice1_register &= B00000001;
    voice1_register |= mode;
    transfer(SID_V1_CT, voice1_register);
  } else if (voice == 1) {
    voice2_register &= B00000001;
    voice2_register |= mode;
    transfer(SID_V2_CT, voice2_register);
  } else if (voice == 2) {
    voice3_register &= B00000001;
    voice3_register |= mode;
    transfer(SID_V3_CT, voice3_register);
  }
}

void MOS6581::setADEnvelope(byte voice, byte attack, byte decay) {
  if (voice == 0) {
    transfer(SID_V1_AD,(decay & B00001111) | (attack << 4));
  } else if (voice == 1) {
    transfer(SID_V2_AD,(decay & B00001111) | (attack << 4));
  } else if (voice == 2) {
    transfer(SID_V3_AD,(decay & B00001111) | (attack << 4));
  }
}

void MOS6581::setSREnvelope(byte voice, byte sustain, byte release) {
  if (voice == 0) {
    transfer(SID_V1_SR,(release & B00001111) | (sustain << 4));
  } else if (voice == 1) {
    transfer(SID_V2_SR,(release & B00001111) | (sustain << 4));
  } else if (voice == 2) {
    transfer(SID_V3_SR,(release & B00001111) | (sustain << 4));
  }
}

void MOS6581::volume(byte level) {
  mode_register &= B11110000;
  mode_register |= (level & B00001111);
  transfer(SID_FL_MD_VL, mode_register);
}

void MOS6581::setFilterMode(byte mode) {
  mode_register &= B00001111;
  mode_register |= mode;
  transfer(SID_FL_MD_VL, mode_register);
}
