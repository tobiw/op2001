#ifndef _TFT_GUI_H
#define _TFT_GUI_H

#include "TFT_22_ILI9225.h"

/*
 * Wrapper for external TFT library.
 * Makes drawing common OP2001 elements easier.
 */
class TftGui {
    public:
    void begin();
    void clear();
    void draw_bar(int x, int y, int value, int color);
    void draw_text(int x, int y, const char *s);

    protected:
    int maxX, maxY;
};

#endif
