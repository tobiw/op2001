#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "USBHost_t36.h"

USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
USBHub hub3(myusb);
KeyboardController keyboard1(myusb);
KeyboardController keyboard2(myusb);
MIDIDevice midi1(myusb);

// GUItool: begin automatically generated code
AudioSynthWaveformSine   sine1;          //xy=86,329
AudioSynthWaveformSine   sine2;          //xy=88,401
AudioSynthSimpleDrum     drum1;          //xy=90,472
AudioMixer4              mixer1;         //xy=310,404
AudioFilterStateVariable filter1;        //xy=475,399
AudioEffectEnvelope      envelope1;      //xy=660,398
AudioOutputI2S           i2s1;           //xy=828,397
AudioConnection          patchCord1(sine1, 0, mixer1, 0);
AudioConnection          patchCord2(sine2, 0, mixer1, 1);
AudioConnection          patchCord3(drum1, 0, mixer1, 2);
AudioConnection          patchCord4(mixer1, 0, filter1, 0);
AudioConnection          patchCord5(filter1, 0, envelope1, 0);
AudioConnection          patchCord6(envelope1, 0, i2s1, 0);
AudioConnection          patchCord7(envelope1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=155,215
// GUItool: end automatically generated code


void OnPress(int key)
{
  Serial.print("key '");
  Serial.print((char)key);
  Serial.print("'  ");
  Serial.println(key);
  //Serial.print("key ");
  //Serial.print((char)keyboard1.getKey());
  //Serial.print("  ");
  //Serial.print((char)keyboard2.getKey());
  //Serial.println();
}

void OnRawPress(uint8_t keycode)
{
  Serial.print("raw key press: ");
  Serial.println((int)keycode);
}

void OnRawRelease(uint8_t keycode)
{
  Serial.print("raw key release: ");
  Serial.println((int)keycode);
}

void OnNoteOn(byte channel, byte note, byte velocity)
{
  Serial.print("Note On, ch=");
  Serial.print(channel);
  Serial.print(", note=");
  Serial.print(note);
  Serial.print(", velocity=");
  Serial.print(velocity);
  Serial.println();
}

void OnNoteOff(byte channel, byte note, byte velocity)
{
  Serial.print("Note Off, ch=");
  Serial.print(channel);
  Serial.print(", note=");
  Serial.print(note);
  //Serial.print(", velocity=");
  //Serial.print(velocity);
  Serial.println();
}

void OnControlChange(byte channel, byte control, byte value)
{
  Serial.print("Control Change, ch=");
  Serial.print(channel);
  Serial.print(", control=");
  Serial.print(control);
  Serial.print(", value=");
  Serial.print(value);
  Serial.println();
}

void request_receive_i2c(int *pos, uint8_t *buttons)
{
  Wire.requestFrom(0x08, 9);

  byte b[9] = {};
  int i = 0;

  while (Wire.available()) {
    b[i++] = Wire.read();
  }

  pos[0] = (b[0] << 8) | b[1];
  pos[1] = (b[2] << 8) | b[3];
  pos[2] = (b[4] << 8) | b[5];
  pos[3] = (b[6] << 8) | b[7];
  pos[4] = pos[5] = pos[6] = pos[7] = 0;

  buttons[0] = (b[8] & 8) >> 3;
  buttons[1] = (b[8] & 4) >> 2;
  buttons[2] = (b[8] & 2) >> 1;
  buttons[3] = b[8] & 1;
  buttons[4] = buttons[5] = buttons[6] = buttons[7] = 1; // HIGH = not pressed
}

void setup() {
  Serial.begin(9600);
  Serial4.begin(9600); // bi-directional communication with ESP32

  // Audio board setup
  AudioMemory(512);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);

  // Synth setup
  drum1.frequency(110);
  sine1.frequency(440);
  sine1.amplitude(0.6);
  sine2.frequency(440);
  sine2.amplitude(0.4);

  // USB host shield setup
  myusb.begin();
  keyboard1.attachPress(OnPress);
  keyboard1.attachRawPress(OnRawPress);
  keyboard1.attachRawRelease(OnRawRelease);
  keyboard2.attachPress(OnPress);
  midi1.setHandleNoteOff(OnNoteOff);
  midi1.setHandleNoteOn(OnNoteOn);
  midi1.setHandleControlChange(OnControlChange);
}

int pos[8], prev_pos[8];
uint8_t buttons[8], prev_buttons[8];
bool pos_update[8], button_update[8];

void set_osc_freq(int i) {
  static AudioSynthWaveformSine *sine_ptr[2] = { &sine1, &sine2 };
  const int freq = 200 + (pos[i] * 10);
  Serial.println("Setting freq to "); Serial.println(freq);
  sine_ptr[i]->frequency(freq); 
}

void on_pos_update(int i) {
  Serial.println(pos[0]);

  set_osc_freq(i);

  char buf[8];
  sprintf(buf, "e%d:%d;\r\n", i, pos[i]);
  Serial4.print(buf);
}

void update_inputs() {
  int i;
  for (i = 0; i < 8; i++) {
    if (pos[i] != prev_pos[i]) {
      pos_update[i] = true;
      prev_pos[i] = pos[i];
    }

    if (buttons[i] != prev_buttons[i]) {
      button_update[i] = true;
      prev_buttons[i] = buttons[i];
    }
  }

  if (pos_update[0]) on_pos_update(0);
  if (pos_update[1]) on_pos_update(1);

  memset(pos_update, 0, sizeof (pos_update));
  memset(button_update, 0, sizeof (button_update));
}

uint32_t current_beat = 0;
uint32_t cur_seq_step = 0;
uint32_t last_note = 0;
bool note_on_sent = false;

void play_notes_sequence() {
  uint32_t t = millis();
  if (t - last_note > 1200) {
    envelope1.noteOff();
    Serial.println("Note off");
    last_note = millis();
    note_on_sent = false;
  } else if (t - last_note > 700 && !note_on_sent) {
    envelope1.noteOn();
    note_on_sent = true;
    Serial.println("Note on");
  }
}

void play_drum_sequence() {
  const int beat = 500;
  const int beat8 = beat / 2;

  drum1.noteOn();
  delay(beat);
  drum1.noteOn();
  delay(beat);
  drum1.noteOn();
  delay(beat);
  drum1.noteOn();
  delay(beat8);
  drum1.noteOn();
  delay(beat8);
}

void loop() {
  myusb.Task();
  midi1.read();
  
  request_receive_i2c(pos, buttons);
  update_inputs();
  
  play_notes_sequence();
}


