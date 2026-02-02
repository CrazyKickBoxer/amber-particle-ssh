#pragma once

#include "FontAsset.h"
#include "../particles/FontData.h" // Assumes existing 8x8 data is here

class ClassicFont : public FontAsset {
public:
    FontType type() const override { return FontType::Bitmap; }
    const char* name() const override { return "Classic 8x8"; }

    int width() const override { return 8; }
    int height() const override { return 8; }

    bool getPixel(uint32_t index, int x, int y) const override {
        if (index > 255) return false;
        if (x < 0 || x >= 8 || y < 0 || y >= 8) return false;
        
        uint8_t row = PROGMEM_FONT8x8[index * 8 + y];
        return (row >> (7 - x)) & 1;
    }
};
