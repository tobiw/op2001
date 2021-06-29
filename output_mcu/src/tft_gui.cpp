#include "SPI.h"
#include <tft_gui.h>

#define TFT_RST 4
#define TFT_RS  2
#define TFT_CLK 18 // VSPI-SCK
#define TFT_SDI 23 // VSPI-MOSI
#define TFT_CS  5 // VSPI-SS0
#define TFT_LED 0 // 0 if wired to +5V directly
#define TFT_BRIGHTNESS 200

SPIClass vspi(VSPI);
TFT_22_ILI9225 spi_tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);

void TftGui::begin() {
    vspi.begin();
    spi_tft.begin(vspi);
    spi_tft.setOrientation(1);
    spi_tft.clear();

    maxX = spi_tft.maxX();
    maxY = spi_tft.maxY();

    spi_tft.setFont(Terminal6x8);
}

void TftGui::clear() {
    spi_tft.clear();
}

void TftGui::draw_bar(int x, int y, int value, int color) {
  const int area_top = maxY - y;
  const int area_bottom = maxY;
  const int text_y = maxY - y + 16;
  const int bar_width = 40;
  spi_tft.fillRectangle(x, area_top, x + bar_width, area_bottom, COLOR_BLACK); // clear
  spi_tft.drawRectangle(x, area_top, x + bar_width, area_top + 10, color);
  spi_tft.fillRectangle(x, area_top, x + map(value, 0, 127, 0, bar_width), area_top + 10, color);

  static char buf[6];
  sprintf(buf, "%02d", value);
  spi_tft.drawText(x + 8, text_y, buf);
}

void TftGui::draw_text(int x, int y, const char *s) {
    spi_tft.drawText(x, y, s);
}
