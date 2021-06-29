#include <Arduino.h>
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

void McuCommUart::parse_uart(int *enc_values, bool *enc_updated, bool *button_states, bool *button_updated) {
    if (rx_buf[0] == 'e') {
        msg_received = false;
        int enc, enc_value;
        sscanf(rx_buf, "e%d:%d;", &enc, &enc_value);
        debug_print_received_msg("enc", enc, enc_value);

        if (enc >= 0 && enc <= 3) {
            enc_values[enc] = enc_value <= 127 ? enc_value : 127;
            enc_updated[enc] = true;
        }
    } else if (rx_buf[0] == 'b') {
        msg_received = false;
        int btn, btn_state;
        sscanf(rx_buf, "b%d:%d;", &btn, &btn_state);
        debug_print_received_msg("btn", btn, btn_state);

        if (btn >= 0 && btn <= 3) {
            button_states[btn] = btn_state;
            button_updated[btn] = true;
        }
    }
}

void McuCommUart::flush_rx_buffer() {
    //while (MCU_UART.available() > 0) { Serial.print("$"); Serial.print(MCU_UART.read()); delay(1); }
    while (MCU_UART.available() > 0) { MCU_UART.read(); }
}

