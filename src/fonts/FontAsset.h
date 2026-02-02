#pragma once

#include <vector>
#include <cstdint>
#include <cmath>

enum class FontType {
    Bitmap,
    Vector
};

struct VectorSegment {
    float x1, y1;
    float x2, y2;
};

class FontAsset {
public:
    virtual ~FontAsset() = default;
    
    virtual FontType type() const = 0;
    virtual const char* name() const = 0;

    // Bitmap Interface
    virtual int width() const { return 0; }
    virtual int height() const { return 0; }
    virtual bool getPixel(uint32_t cp437_index, int x, int y) const { return false; }

    // Vector Interface
    // Returns segments in normalized [0,1] coordinate space
    virtual std::vector<VectorSegment> getSegments(uint32_t cp437_index) const { return {}; }
};
