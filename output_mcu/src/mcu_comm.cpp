#include <Arduino.h>
#include <Wire.h>
#include <mcu_comm.h>

#define MCU_UART Serial2

McuCommUart::McuCommUart() {
    rx_buf_idx = 0;
    receiving = false;
    msg_received = false;
}

void McuCommUart::begin() {
    Serial2.begin(9600);
}

void McuCommUart::receive_uart() {
  if (rx_buf_idx > 30) {
    //Serial.println("rx_buf overflow!");
    // Clear UART buffer, lose current message
    flush_rx_buffer();
    rx_buf_idx = 0;
    receiving = false;
  }
  
  if (MCU_UART.available() > 0) {
    char c = MCU_UART.read();
    if (c != '\n' && c != '\r' && c != '\0') {
      rx_buf[rx_buf_idx] = c;
      //Serial.print(rx_buf_idx); Serial.print(">");
      //Serial.println(rx_buf[rx_buf_idx]);
    } else {
      return;
    }
    
    if (!receiving) {
      receiving = true;
    } else {
      if (rx_buf[rx_buf_idx] == ';') { // stop receiving when newline hit
        delay(1);
        flush_rx_buffer(); // might require delay in while loop
        receiving = false;
        msg_received = true;
        rx_buf[rx_buf_idx+1] = '\0';
      }
    }

    rx_buf_idx = receiving ? (rx_buf_idx + 1) : 0;
  }
}


void debug_print_received_msg(const char *type, int a, int b) {
    Serial.print("Received: ");
    Serial.print(type);
    Serial.print(a);
    Serial.print(" value ");
    Serial.println(b);
}

/*
 * non-blocking, i.e. will process one character per call and only parse the message if a full message has been buffered,
 * rather than looping and receiving the full message in one call.
 */
void McuCommUart::parse_uart(int *enc_values, bool *enc_updated, bool *button_states, bool *button_updated, int num_inputs) {
    receive_uart();
    if (!msg_received) return;

    if (rx_buf[0] == 'e') {
        msg_received = false;
        int enc, enc_value;
        sscanf(rx_buf, "e%d:%d;", &enc, &enc_value);
        debug_print_received_msg("enc", enc, enc_value);

        if (enc >= 0 && enc < num_inputs) {
            enc_values[enc] = enc_value <= 127 ? enc_value : 127;
            enc_updated[enc] = true;
            Serial.print("enc updated: ");
            Serial.println(enc);
        }
    } else if (rx_buf[0] == 'b') {
        msg_received = false;
        int btn, btn_state;
        sscanf(rx_buf, "b%d:%d;", &btn, &btn_state);
        debug_print_received_msg("btn", btn, btn_state);

        if (btn >= 0 && btn <= num_inputs) {
            button_states[btn] = btn_state;
            button_updated[btn] = true;
        }
    }
}

void McuCommUart::flush_rx_buffer() {
    //while (MCU_UART.available() > 0) { Serial.print("$"); Serial.print(MCU_UART.read()); delay(1); }
    while (MCU_UART.available() > 0) { MCU_UART.read(); }
}



McuCommI2c::McuCommI2c() {
    memset(prev_enc_values, 0, sizeof (int) * 8);
    memset(prev_button_states, 0, sizeof (bool) * 8);
}

void McuCommI2c::begin() {
    Wire.begin(); // default I2C pins: 21, 22
}

void McuCommI2c::request_encoders_buttons(int *enc_values, bool *enc_updated, bool *button_states, bool *button_updated, int num_inputs) {
    Wire.requestFrom(McuCommI2c::I2C_SLAVE_ENCODERS, 9);

    byte b[9] = {};
    int i = 0;

    while (Wire.available()) {
        b[i++] = Wire.read();
        Serial.println(b[i-1]);
    }

    enc_values[0] = (b[0] << 8) | b[1];
    enc_values[1] = (b[2] << 8) | b[3];
    enc_values[2] = (b[4] << 8) | b[5];
    enc_values[3] = (b[6] << 8) | b[7];
    enc_values[4] = enc_values[5] = enc_values[6] = enc_values[7] = 0;

    for (int i = 0; i < num_inputs; i++) {
        if (enc_values[i] != prev_enc_values[i]) {
            enc_updated[i] = true;
            prev_enc_values[i] = enc_values[i];
        }
        if (button_states[i] != prev_button_states[i]) {
            button_updated[i] = true;
            prev_button_states[i] = button_states[i];
        }
    }

    button_states[0] = (b[8] & 8) >> 3;
    button_states[1] = (b[8] & 4) >> 2;
    button_states[2] = (b[8] & 2) >> 1;
    button_states[3] = b[8] & 1;
    button_states[4] = button_states[5] = button_states[6] = button_states[7] = 1; // HIGH = not pressed
}
