#pragma once

#include "ClassicFont.h"

class HighResFont : public FontAsset {
public:
    FontType type() const override { return FontType::Bitmap; }
    const char* name() const override { return "High-Res 16x16"; }

    int width() const override { return 16; }
    int height() const override { return 16; }

    bool getPixel(uint32_t c, int x, int y) const override {
        // Map 16x16 (x,y) to 8x8 (srcX, srcY)
        int srcX = x / 2;
        int srcY = y / 2;
        int subX = x % 2;
        int subY = y % 2;
        
        // P = current pixel
        bool P = getRaw(c, srcX, srcY);
        
        // Neighbors (Unused for Nearest Neighbor)
        // bool A = getRaw(c, srcX, srcY - 1); // Top
        // bool B = getRaw(c, srcX + 1, srcY); // Right
        // bool C = getRaw(c, srcX - 1, srcY); // Left
        // bool D = getRaw(c, srcX, srcY + 1); // Bottom
        
        // Scale2x Algorithm - REMOVED for Sharpness
        // The user requested "Sharpened" font.
        // Scale2x smooths corners. Nearest Neighbor preserves hard edges.
        return P;
    }

private:
    bool getRaw(uint32_t c, int x, int y) const {
        if (c == 0x2588) c = 219; // Map Block Cursor to CP437
        if (c > 255) return false;
        if (x < 0 || x >= 8 || y < 0 || y >= 8) return false;
        uint8_t row = PROGMEM_FONT8x8[c * 8 + y];
        return (row >> (7 - x)) & 1;
    }
};
