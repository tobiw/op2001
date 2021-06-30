#ifndef _MCU_COMM_H
#define _MCU_COMM_H

class McuCommUart {
    public:
    McuCommUart();
    void begin();
    void receive_uart();
    void parse_uart(int *enc_values, bool *enc_updated, bool *button_states, bool *button_updated, int num_inputs);

    private:
    void flush_rx_buffer();

    char rx_buf[32];
    int rx_buf_idx;
    bool receiving;
    bool msg_received;
};

class McuCommI2c {
    public:
    McuCommI2c();
    void begin();
    void request_encoders_buttons(int *enc_values, bool *enc_updated, bool *button_states, bool *button_updated, int num_inputs);

    static const int I2C_SLAVE_ENCODERS = 0x8;

    private:
    int prev_enc_values[8];
    bool prev_button_states[8];
};

#endif
