# op2001
Groovebox (synth, sampler, sequencer) based on a Teensy 4

Inspired by the OP-1, Elektron Digi... devices and the many different DIY/Opensource Teensy synths.

# Main features
## Synth engine
* 2 oscillators with different waveforms (sine, triangle, square)
* 1 noise output
* Low-pass, high-pass and band-pass filters with resonance
* ADSR envelope
* Mixer

## Sample playback
* Load samples from SD card or audio board flash

## MIDI
* USB MIDI input and output via Micro USB (as a USB peripheral)
** send MIDI notes and CC from the OP sequencer to a PC (e.g. to play and control a software synth engine)
** receive MIDI notes and CC from a PC (e.g. from tracks in a DAW)
* USB host: uses MAX3421E chip to 

## Hardware
* Headphone/line out ports (1/8")
* Line in port (1/8") for sampling and recording
* Mini MIDI in and out via TRS (1/8") (type can be adjusted internally)
* 220x176 2" color TFT (ILI9225 driver chipset)
* 4 rotary (endless) encoders with integrated buttons for input
* Mechanical keys for button inputs (play/stop, track selection, track mute, ...)

# Implementation
* Teensy 4.0 with audio board for synth and sample playback
* SD card and flash memory access via audio board (load and record samples)
* Rotary encoder and button inputs via ATmega328p co-processor on I2C bus (up to 8 encoders and 8 buttons)
* ESP32 for additional hardware and network I/O and for offloading TFT rendering, network handling from Teensy:
** controls the TFT via SPI
** provides HTTP API (possibly debug only) and Bluetooth connectivity (TBD later)
** drives WS2812 RGB LEDs

# Resources
* https://www.pjrc.com/store/teensy40.html
* https://www.pjrc.com/store/teensy3_audio.html
* https://www.pjrc.com/teensy/td_libs_USBHostShield.html
* https://www.pjrc.com/teensy/td_midi.html
* https://www.pjrc.com/teensy/gui/index.html
* https://minimidi.world/

## Datasheets
* https://datasheets.maximintegrated.com/en/ds/MAX3421E.pdf
