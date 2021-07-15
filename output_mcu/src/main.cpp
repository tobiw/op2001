#include <Arduino.h>

#include <tft_gui.h>
#include <mcu_comm.h>

// Interface to the hardware TFT display
TftGui tft;

McuCommUart mcu_comm_dsp; // Interface to the UART communication with the Audio DSP MCU
McuCommI2c mcu_comm_encboard; // Interface to the I2C communication with the Encoder and button input board

void setup() {
  Serial.begin(9600);
  mcu_comm_dsp.begin();
  mcu_comm_encboard.begin();
  tft.begin();
  Serial.println("Setup done");
}

/*
 * These arrays are used to move user input via encoders and buttons to the currently active GUI page.
 */
#define NUM_ENCODERS 4
int enc_values[NUM_ENCODERS] = { 33, 10, 50, 75 };
bool button_states[NUM_ENCODERS] = { false, false, false, false };

// GUI constants
const int BARS_X_START = 15;
const int BARS_WIDTH = 50;
const int BARS_Y = 50;

/*
 * A GuiPage is one screen with various Gui elements such as graphics and text.
 * Each page stores all the necessary data which has to be kept in the background
 * if the user switches to a different page.
 */
class GuiPage {
    public:
    virtual void render();
    virtual void update_data(int *enc_values, bool *button_states);
    void draw_bar(int i, int value, int color, const char *text);

    protected:
    int enc_values[NUM_ENCODERS];
    int colors[NUM_ENCODERS];
    char *enc_text[NUM_ENCODERS];
    bool button_states[NUM_ENCODERS];
};

void GuiPage::draw_bar(int i, int value, int color, const char *text) {
    const int x = BARS_X_START + i * BARS_WIDTH;
    tft.draw_bar(x, BARS_Y, value, color);
    tft.draw_text(x, BARS_Y + 24, text);
}

class MixerGuiPage : public GuiPage {
    public:
    void render();
    void update_data(int *enc_values, bool *button_states);

    private:
    int volume_osc1, volume_osc2, volume_noise;
};

void MixerGuiPage::render() {
    Serial.println("Mixer render");
    draw_bar(0, volume_osc1, COLOR_RED, "Osc1");
    draw_bar(1, volume_osc2, COLOR_GREEN, "Osc2");
    draw_bar(2, volume_noise, COLOR_BLUE, "Noise");
    draw_bar(3, 0, COLOR_GREY, "...");
}

/*
 * Called when encoders/buttons manipulate the current screen.
 * This translates to updated data in the model (e.g. encoder1 -> volume osc1)
 */
    
void MixerGuiPage::update_data(int *enc_values, bool *button_states) {
    Serial.println("Mixer update_data");
    volume_osc1 = enc_values[0];
    volume_osc2 = enc_values[1];
    volume_noise = enc_values[2];
}

class OscillatorGuiPage : public GuiPage {
    public:
    void render();
    void update_data(int *enc_values, bool *button_states);

    private:
    int osc_frequency, osc_shape, pwm;
};

void OscillatorGuiPage::render() {
    draw_bar(0, osc_frequency, COLOR_RED, "Freq");
    draw_bar(1, osc_shape, COLOR_YELLOW, "Shape");
    draw_bar(2, pwm, COLOR_BLUE, "PWM");
    draw_bar(3, 0, COLOR_GREY, "...");
}

void OscillatorGuiPage::update_data(int *enc_values, bool *button_states) {
    osc_frequency = enc_values[0];
    osc_shape = enc_values[1];
    pwm = enc_values[2];
}

class FilterGuiPage : public GuiPage {
    public:
    void render();
    void update_data(int *enc_values, bool *button_states);

    private:
    int cutoff_frequency, resonance, attenuation, filter_type;
};

void FilterGuiPage::render() {
    draw_bar(0, cutoff_frequency, COLOR_RED, "Freq");
    draw_bar(1, resonance, COLOR_YELLOW, "Resonance");
    draw_bar(2, attenuation, COLOR_BLUE, "Attenuation");
    draw_bar(3, filter_type, COLOR_GREEN, "Type");
}

void FilterGuiPage::update_data(int *enc_values, bool *button_states) {
    cutoff_frequency = enc_values[0];
    resonance = enc_values[1];
    attenuation = enc_values[2];
    filter_type = enc_values[3];
}

class EnvelopeGuiPage : public GuiPage {
    public:
    void render();
    void update_data(int *enc_values, bool *button_states);

    private:
    int attack, decay, sustain, release;
};

void EnvelopeGuiPage::render() {
    draw_bar(0, attack, COLOR_RED, "Attack");
    draw_bar(1, decay, COLOR_YELLOW, "Decay");
    draw_bar(2, sustain, COLOR_BLUE, "Sustain");
    draw_bar(3, release, COLOR_GREEN, "Release");
}

void EnvelopeGuiPage::update_data(int *enc_values, bool *button_states) {
    attack = enc_values[0];
    decay = enc_values[1];
    sustain = enc_values[2];
    release = enc_values[3];
}

/*
 * The Gui object manages the various pages. Each page can render itself.
 */
class Gui {
    public:
        Gui();
        void switch_page(int i);
        void previous_page();
        void next_page();
        void render(); // render the whole GUI
        void update_encoder(int i); // redraw only a specific encoder area (bar, text, maybe graphics if affected)
        void update_button(int i); // redraw only a specific button area (bar, text, maybe graphics if affected)
        void update_data(int *enc_values, bool *button_states);

        enum gui_pages_enum {
            PAGE_MIXER = 0,
            PAGE_SYNTH,
            PAGE_FILTER,
            PAGE_ENVELOPE,
            PAGE_SAMPLER,
            PAGE_SEQUENCER
        };

    private:
        static const int max_page = 4;
        GuiPage *pages[max_page];
        int current_page_idx;
        GuiPage *current_page;
} gui;

Gui::Gui() {
    pages[PAGE_MIXER] = new MixerGuiPage();
    pages[PAGE_SYNTH] = new OscillatorGuiPage();
    pages[PAGE_FILTER] = new FilterGuiPage();
    pages[PAGE_ENVELOPE] = new EnvelopeGuiPage();
    //pages[4] = new SamplerGuiPage();
    //pages[5] = new SequencerGuiPage();
    current_page_idx = PAGE_MIXER;
    current_page = pages[current_page_idx];
}

void Gui::switch_page(int i) {
    if (i < 0) i = Gui::max_page;
    else if (i > Gui::max_page) i = 0;
    current_page_idx = i;
    current_page = pages[current_page_idx];
}

void Gui::previous_page() {
    switch_page(current_page_idx - 1);
}

void Gui::next_page() {
    switch_page(current_page_idx + 1);
}

void Gui::render() {
    pages[current_page_idx]->render();
}

void Gui::update_encoder(int i) {
    //tft.draw_bar(BARS_X_START + i * BARS_WIDTH, BARS_Y, enc_values[i], bar_colors[i]); // TODO: model
    pages[current_page_idx]->render();
}

void Gui::update_button(int i) {
    // TODO
    pages[current_page_idx]->render();
}

void Gui::update_data(int *enc_values, bool *button_states) {
    // Do GUI-wide stuff first like switching pages or something else "global"
    // ...
    if (button_states[3]) {
        next_page();
    }

    // Now pass updated data to the current page
    current_page->update_data(enc_values, button_states);
}

bool data_update_required(bool *enc_updated, bool *button_updated) {
    for (int i = 0; i < NUM_ENCODERS; i++) {
        if (enc_updated[i] || button_updated[i]) {
            return true;
        }
    }
    return false;
}

// This is basically the controller: get updates, move it into the data mode, and update presentation.
void loop() {
    bool enc_updated[NUM_ENCODERS] = { false, false, false, false };
    bool button_updated[NUM_ENCODERS] = { false, false, false, false };
    //mcu_comm_dsp.parse_uart(enc_values, enc_updated, button_states, button_updated, NUM_ENCODERS); // get data from Teensy via UART
    mcu_comm_encboard.request_encoders_buttons(enc_values, enc_updated, button_states, button_updated, NUM_ENCODERS); // get data from encoder board via I2C

    if (data_update_required(enc_updated, button_updated)) {
        //Serial.println("Calling GUI update_data");
        //gui.update_data(enc_values, button_states);
        //gui.render();
    }

    // TODO: optimisation to only redraw part that requires an update (e.g. one bar or button)
    /*for (int i = 0; i < NUM_ENCODERS; i++) {
        if (enc_updated[i]) {
            enc_updated[i] = false;
            gui.update_encoder(i);
        }
        if (button_updated[i]) {
            button_updated[i] = false;
            gui.update_button(i);
        }
    }*/
}
