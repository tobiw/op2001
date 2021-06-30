#include <Arduino.h>
#include <RotaryEncoder.h>
#include <Wire.h>

const int16_t I2C_SLAVE = 0x08;

#define PIN_SR_Q 9
#define PIN_SR_CLK 10
#define PIN_SR_PSCTRL 11

// Encoders (left to right)
// 1: A3, 5 => PC5, PD5
// 2: A2, 6 => PC4, PD6
// 3: A1, 7 => PC3, PD7
// 4: A0, 4 => PC2, PD4

#define PIN_ENC_1A A3
#define PIN_ENC_2A A2
#define PIN_ENC_3A A1
#define PIN_ENC_4A A0
#define PIN_ENC_1B 5
#define PIN_ENC_2B 6
#define PIN_ENC_3B 7
#define PIN_ENC_4B 4

const int enc_pins[] = {PIN_ENC_1A, PIN_ENC_2A, PIN_ENC_3A, PIN_ENC_4A,
                        PIN_ENC_1B, PIN_ENC_2B, PIN_ENC_3B, PIN_ENC_4B};

RotaryEncoder enc1(PIN_ENC_1A, PIN_ENC_1B, RotaryEncoder::LatchMode::TWO03);
RotaryEncoder enc2(PIN_ENC_2A, PIN_ENC_2B, RotaryEncoder::LatchMode::TWO03);
RotaryEncoder enc3(PIN_ENC_3A, PIN_ENC_3B, RotaryEncoder::LatchMode::TWO03);
RotaryEncoder enc4(PIN_ENC_4A, PIN_ENC_4B, RotaryEncoder::LatchMode::TWO03);

RotaryEncoder *enc[] = {&enc1, &enc2, &enc3, &enc4};

int pos[4];
uint8_t btn; // bitmap for 8 buttons: 1 = not pressed, 0 = pressed (LOW)
int last_pos[4] = {0, 0, 0, 0};
uint8_t last_btn = 0xff;

ISR(PCINT1_vect)
{
  enc1.tick();
  enc2.tick();
  enc3.tick();
  enc4.tick();
}

ISR(PCINT2_vect)
{
  enc1.tick();
  enc2.tick();
  enc3.tick();
  enc4.tick();
}

/*
 * Reply to MIDI fighter brain requesting rotary positions
 */
void on_i2c_request() {
  for (int i = 0; i < 4; i++) {
    Wire.write((pos[i] & 0xff00) >> 8);
    Wire.write(pos[i] & 0x00ff);
  }
  Wire.write(btn ^ 0xff); // flip bits so that 0 means not pressed, 1 means pressed
}

void setup() {
  pinMode(PIN_SR_Q, INPUT);
  pinMode(PIN_SR_CLK, OUTPUT);
  pinMode(PIN_SR_PSCTRL, OUTPUT);
  digitalWrite(PIN_SR_CLK, LOW);
  digitalWrite(PIN_SR_PSCTRL, LOW);
  
  PCICR |= (1 << 1) | (1 << 2); // activate PCI on PC and PD ports
  PCMSK1 = (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2);
  PCMSK2 = (1 << 5) | (1 << 6) | (1 << 7) | (1 << 4);

  Wire.begin(I2C_SLAVE);
  Wire.onRequest(on_i2c_request);
  
  Serial.begin(9600);
  Serial.println("Setup done.");
}

uint8_t read_shift_register()
{
  // P/S set to Parallel to jam-in P1-P8
  digitalWrite(PIN_SR_PSCTRL, HIGH);
  delayMicroseconds(20);
  digitalWrite(PIN_SR_PSCTRL, LOW);
  digitalWrite(PIN_SR_CLK, LOW);

  uint8_t first = digitalRead(PIN_SR_Q);
  uint8_t r = shiftIn(PIN_SR_Q, PIN_SR_CLK, MSBFIRST);
  r >>= 1;
  r |= first << 7;
  return r;
}

int process_pos(int p)
{
  if (p % 2 == 1) p += p < 0 ? 1 : -1;
  p >>= 1;
  return p;
}

void loop() {
  int i;
  
  for (i = 0; i < 4; i++) {
    pos[i] = process_pos(enc[i]->getPosition());
  }
  btn = read_shift_register();

  // Interrupts for the encoders will still get triggered and encoder objects updated
  // delay here helps to debounce buttons
  delay(40);
}
