#include <Arduino.h>

#include <tft_gui.h>
#include <mcu_comm.h>

TftGui tft;
McuCommUart mcu_comm;

void setup() {
  Serial.begin(9600);
  mcu_comm.begin();
  tft.begin();
  Serial.println("Setup done");
}


/*
 * These arrays are used to move user input via encoders and buttons to the currently active GUI page.
 */
#define NUM_ENCODERS 4
int enc_values[NUM_ENCODERS] = { 33, 10, 50, 75 };
bool enc_updated[NUM_ENCODERS] = { true, true, true, true };
bool button_states[NUM_ENCODERS] = { false, false, false, false };
bool button_updated[NUM_ENCODERS] = { true, true, true, true };

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
    void set_data(int*, int*, int*, char**); // TODO: store in model
    void draw_bar(int i, int value, int color, const char *text);

    protected:
    int enc_values[NUM_ENCODERS];
    int colors[NUM_ENCODERS];
    char *enc_text[NUM_ENCODERS];
    bool button_states[NUM_ENCODERS];
};

void GuiPage::set_data(int *arg_enc_values, int *arg_colors, int *arg_button_states, char **arg_enc_text) {
    for (int i = 0; i < NUM_ENCODERS; i++) {
        enc_values[i] = arg_enc_values[i];
        colors[i] = arg_colors[i];
        button_states[i] = arg_button_states[i];
        strcpy(enc_text[i], arg_enc_text[i]);
    }
}

void GuiPage::draw_bar(int i, int value, int color, const char *text) {
    const int x = BARS_X_START + i * BARS_WIDTH;
    tft.draw_bar(x, BARS_Y, value, color);
    tft.draw_text(x, BARS_Y + 24, text);
}

class MixerGuiPage : public GuiPage {
    public:
    void render();

    private:
    int volume_osc1, volume_osc2, volume_noise;
};

void MixerGuiPage::render() {
    draw_bar(0, volume_osc1, COLOR_RED, "Osc1");
    draw_bar(1, volume_osc2, COLOR_RED, "Osc2");
    draw_bar(2, volume_noise, COLOR_BLUE, "Noise");
    draw_bar(3, 0, COLOR_GREY, "...");
}

class OscillatorGuiPage : public GuiPage {
    public:
    void render();

    private:
    int osc_frequency, osc_shape, pwm;
};

void OscillatorGuiPage::render() {
    draw_bar(0, osc_frequency, COLOR_RED, "Freq");
    draw_bar(1, osc_shape, COLOR_YELLOW, "Shape");
    draw_bar(2, pwm, COLOR_BLUE, "PWM");
    draw_bar(3, 0, COLOR_GREY, "...");
}

class FilterGuiPage : public GuiPage {
    public:
    void render();

    private:
    int cutoff_frequency, resonance, attenuation, filter_type;
};

void FilterGuiPage::render() {
    draw_bar(0, cutoff_frequency, COLOR_RED, "Freq");
    draw_bar(1, resonance, COLOR_YELLOW, "Resonance");
    draw_bar(2, attenuation, COLOR_BLUE, "Attenuation");
    draw_bar(3, filter_type, COLOR_GREEN, "Type");
}

class EnvelopeGuiPage : public GuiPage {
    public:
    void render();

    private:
    int attack, decay, sustain, release;
};

void EnvelopeGuiPage::render() {
    draw_bar(0, attack, COLOR_RED, "Attack");
    draw_bar(1, decay, COLOR_YELLOW, "Decay");
    draw_bar(2, sustain, COLOR_BLUE, "Sustain");
    draw_bar(3, release, COLOR_GREEN, "Release");
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

    private:
        static const int max_page = 4;
        GuiPage *pages[max_page];
        int current_page_idx;
} gui;

Gui::Gui() {
    pages[0] = new MixerGuiPage();
    pages[1] = new OscillatorGuiPage();
    pages[2] = new FilterGuiPage();
    pages[3] = new EnvelopeGuiPage();
    //pages[4] = new SamplerGuiPage();
    //pages[5] = new SequencerGuiPage();
    current_page_idx = 0;
}

void Gui::switch_page(int i) {
    if (i < Gui::max_page && i >= 0) current_page_idx = i;
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


void loop() {
  for (int i = 0; i < NUM_ENCODERS; i++) {
    if (enc_updated[i]) {
      enc_updated[i] = false;
      gui.update_encoder(i);
    }
    if (button_updated[i]) {
        button_updated[i] = false;
        gui.update_button(i);
    }
  }

  mcu_comm.receive_uart();
  if (mcu_comm.msg_received) mcu_comm.parse_uart(enc_values, enc_updated, button_states, button_updated);
}
