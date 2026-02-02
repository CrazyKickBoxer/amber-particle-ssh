#pragma once

#include "FontAsset.h"
#include <map>
#include <vector>
#include <cmath>

// 14-Segment Flag Bits
//      A
//    F   B
//    G1 G2
//    E   C
//      D
//
//    H I J
//     K L M
//
// MAPPING:
// A=0, B=1, C=2, D=3, E=4, F=5, G1=6, G2=7 
// H=8 (TL Diag), I=9 (Top Vert), J=10 (TR Diag)
// K=11 (BL Diag), L=12 (Bot Vert), M=13 (BR Diag)

class SegmentedFont : public FontAsset {
public:
    FontType type() const override { return FontType::Vector; }
    const char* name() const override { return "14-Seg Vector"; }

    int width() const override { return 12; }
    int height() const override { return 18; }

    std::vector<VectorSegment> getSegments(uint32_t c) const override {
        uint16_t mask = getMask(c);
        std::vector<VectorSegment> segs;
        
        // Geometry Constants (Normalized 0.0 - 1.0)
        float l = 0.2f, r = 0.8f;   // Left, Right columns
        float t = 0.1f, b = 0.9f;   // Top, Bottom rows
        float my = 0.5f, mx = 0.5f; // Midpoints
        
        // Outer Ring
        if (mask&(1<<0)) segs.push_back({l, t, r, t});    // A (Top)
        if (mask&(1<<1)) segs.push_back({r, t, r, my});   // B (Top Right)
        if (mask&(1<<2)) segs.push_back({r, my, r, b});   // C (Bot Right)
        if (mask&(1<<3)) segs.push_back({l, b, r, b});    // D (Bot)
        if (mask&(1<<4)) segs.push_back({l, my, l, b});   // E (Bot Left)
        if (mask&(1<<5)) segs.push_back({l, t, l, my});   // F (Top Left)
        
        // Horizontal Mids
        if (mask&(1<<6)) segs.push_back({l, my, mx, my}); // G1 (Mid Left)
        if (mask&(1<<7)) segs.push_back({mx, my, r, my}); // G2 (Mid Right)
        
        // Inner Diagonals / Verts
        if (mask&(1<<8)) segs.push_back({l, t, mx, my});  // H (TL -> Center)
        if (mask&(1<<9)) segs.push_back({mx, t, mx, my}); // I (Top Vert)
        if (mask&(1<<10)) segs.push_back({r, t, mx, my}); // J (TR -> Center)
        
        if (mask&(1<<11)) segs.push_back({l, b, mx, my}); // K (BL -> Center)
        if (mask&(1<<12)) segs.push_back({mx, b, mx, my});// L (Bot Vert)
        if (mask&(1<<13)) segs.push_back({r, b, mx, my}); // M (BR -> Center)

        return segs;
    }

private:
     uint16_t getMask(uint32_t c) const {
        // Standard ASCII Mapping for 14-Segment
        switch (toupper(c)) {
            case ' ': return 0;
            
            // Numbers
            case '0': return 0x003F | (1<<10) | (1<<11); // 0 (with slash J+K)
            case '1': return (1<<1)|(1<<2);              // B, C (Right side)
            case '2': return (1<<0)|(1<<1)|(1<<6)|(1<<7)|(1<<4)|(1<<3); // A,B,G,E,D
            case '3': return (1<<0)|(1<<1)|(1<<6)|(1<<7)|(1<<2)|(1<<3); // A,B,G,C,D
            case '4': return (1<<5)|(1<<6)|(1<<7)|(1<<1)|(1<<2);        // F,G,B,C
            case '5': return (1<<0)|(1<<5)|(1<<6)|(1<<7)|(1<<2)|(1<<3); // A,F,G,C,D
            case '6': return 0x00FC;                     // A,F,E,D,C,G
            case '7': return (1<<0)|(1<<10)|(1<<11);     // A, J(Slant), K(Slant)? Or just A,B,C?
                      // Standard 7 is often A,B,C. Or A, J, L?
                      // Let's use A, B, C.
                      return (1<<0)|(1<<1)|(1<<2);
            case '8': return 0x00FF;                     // All outer + G
            case '9': return (1<<0)|(1<<1)|(1<<2)|(1<<5)|(1<<6)|(1<<7); // A,B,C,F,G

            // Letters
            case 'A': return 0x00F7; // No D, No I
            case 'B': return (1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<9)|(1<<12); // A,B,C,D,I,L (Vert B)
            case 'C': return 0x0039; // A,D,E,F
            case 'D': return (1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<9)|(1<<12); // Same as B?
                      // Usually D is A,B,C,D,I,L logic but rounded?
                      // Let's use Right-Curve? No curve.
                      // Let's use Standard D: A,B,C,D, I, L. Same as B visually in 14seg without diagonals.
                      // Or use A, D, I, L + B, C.
                      return 0x000F | (1<<9) | (1<<12);
            case 'E': return 0x00F9; // A,D,E,F,G
            case 'F': return 0x00F1; // A,E,F,G
            case 'G': return 0x00BD; // A,C,D,E,F,G2
            case 'H': return 0x00F6; // B,C,E,F,G
            case 'I': return (1<<0)|(1<<3)|(1<<9)|(1<<12); // A,D, I, L
            case 'J': return (1<<1)|(1<<2)|(1<<3)|(1<<4); // B,C,D,E
            case 'K': return (1<<5)|(1<<4)|(1<<6)|(1<<10)|(1<<13); // F,E,G1, J(TR), M(BR)
            case 'L': return 0x0038; // D,E,F
            case 'M': return (1<<4)|(1<<5)|(1<<1)|(1<<2)|(1<<8)|(1<<10); // F,E,B,C, H,J (Top M)
            case 'N': return (1<<4)|(1<<5)|(1<<1)|(1<<2)|(1<<8)|(1<<13); // F,E,B,C, H,M (Iso N)
            case 'O': return 0x003F; // Box
            case 'P': return 0x00F3; // A,B,E,F,G
            case 'Q': return 0x003F | (1<<13); // Box + M
            case 'R': return 0x00F3 | (1<<13); // P + M
            case 'S': return 0x00ED; // A,F,G,C,D
            case 'T': return (1<<0)|(1<<9)|(1<<12); // A,I,L
            case 'U': return 0x003E; // B,C,D,E,F
            case 'V': return (1<<4)|(1<<5)|(1<<10)|(1<<11); 
                      // F, E (Left Wall) + J, K (Right Slant?) No.
                      // Use New Logic: H(TL->C), J(TR->C), L(Bot)? NO that's Y.
                      // V is: H(TL->C), J(TR->C), but wait, they meet at center.
                      // We need center to bottom.
                      // K(BL->C) and M(BR->C) go UP to center.
                      // So H+K is Left Slash. J+M is Right Slash.
                      // V is H + K + J + M ... Wait that's X.
                      // Actually V is just Left Slant (H+K) + Right Slant (J+M) meeting at bottom?
                      // No, K/M meet at center.
                      // So 14-seg V is hard.
                      // Let's use L (Bot Vert) + H (Top Left) + J (Top Right)? = Y.
                      // Let's use F, E (Left) + K (Bot Left Diag)? No.
                      // Let's use H (TL->C) + J (TR->C) + L (Bot Vert). It looks like Y but reads as V/Y.
                      return (1<<8)|(1<<10)|(1<<12); // Y-shape
            case 'W': return (1<<4)|(1<<5)|(1<<1)|(1<<2)|(1<<11)|(1<<13); // F,E,B,C, K,M (Bot W)
            case 'X': return (1<<8)|(1<<10)|(1<<11)|(1<<13); // H,J,K,M
            case 'Y': return (1<<8)|(1<<10)|(1<<12); // Same as V
            case 'Z': return (1<<0)|(1<<3)|(1<<10)|(1<<11); // A,D, J, K

            // Punctuation
            case '-': return (1<<6)|(1<<7);
            case '+': return (1<<9)|(1<<12)|(1<<6)|(1<<7);
            case '=': return (1<<6)|(1<<7)|(1<<3); // Mid + Bot? Or separate?
                      // Equal sign usually G + D? To low.
                      // Should be G and ... ?
                      // Let's use G + D (Low equals).
                      return (1<<6)|(1<<7)|(1<<3);
            case '*': return 0x3F00; // All inner
            case '/': return (1<<10)|(1<<11); // J, K
            case '\\': return (1<<8)|(1<<13); // H, M
            case '<': return (1<<10)|(1<<13); // J, M (Right pointing?) No.
                      // < Points Left.
                      // Top Right (J) -> Center. Bot Right (M) -> Center.
                      return (1<<10)|(1<<13);
            case '>': return (1<<8)|(1<<11); // H, K
            case '[': return (1<<0)|(1<<3)|(1<<9)|(1<<12); // A,D, I,L (Bracket) -> actually Box Left.
                      return (1<<0)|(1<<3)|(1<<5)|(1<<4);
            case ']': return (1<<0)|(1<<3)|(1<<1)|(1<<2);
            case '{': return (1<<8)|(1<<11)|(1<<9)|(1<<12); // Slant?
                      return (1<<5)|(1<<4)|(1<<6); // Left + G1
            case '}': return (1<<1)|(1<<2)|(1<<7); // Right + G2
            case '(': return (1<<10)|(1<<13); // same as <
            case ')': return (1<<8)|(1<<11); // same as >
            
            case '^': return (1<<8)|(1<<10); // H, J
            case ',': return (1<<11); // K (Tail)
            case '.': return 0; // Empty? No, maybe L (Bot Vert) as dot.
                      return (1<<12); 
            case '!': return (1<<1)|(1<<2)|(1<<12); // B,C + dot?
            case '?': return (1<<0)|(1<<1)|(1<<6)|(1<<12); // A,B,G1, L
            case '"': return (1<<9)|(1<<1); // I, B (Two ticks)
            case '\'': return (1<<9); // I
            case '`': return (1<<8); // H
            case '~': return (1<<0)|(1<<3); // Top/Bot?
            case '|': return (1<<9)|(1<<12);
            case '_': return (1<<3);

            case 0x2588: // Block
            case 219:
                 return 0x3FFF;

            default: return 0;
        }
    }
};
