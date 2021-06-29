#ifndef _MCU_COMM_H
#define _MCU_COMM_H

class McuCommUart {
    public:
    McuCommUart();
    void begin();
    void receive_uart();
    void parse_uart(int *enc_values, bool *enc_updated, bool *button_states, bool *button_updated);

    char rx_buf[32];
    int rx_buf_idx;
    bool receiving;
    bool msg_received;

    protected:
    void flush_rx_buffer();
};

#endif
