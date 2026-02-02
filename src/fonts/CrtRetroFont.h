#pragma once

#include "FontAsset.h"
#include <vector>

class CrtRetroFont : public FontAsset {
public:
    FontType type() const override { return FontType::Vector; }
    int width() const override { return 12; }
    int height() const override { return 18; }
    
    // CRT Retro: Classic VT220 style
    // Slightly more rounded corners where possible, wider stance
    std::vector<VectorSegment> getSegments(uint32_t c) const override {
        std::vector<VectorSegment> segs;
        
        float L = 0.15f, R = 0.85f;
        float T = 0.1f, B = 0.9f;
        float M = 0.5f;
        float CX = 0.5f;
        
        char ch = (char)(c & 0xFF);
        
        // Simulating "rounded" corners by cutting them?
        // Actually a simple vector approximation of Lear Siegler ADM-3A or VT100 font
        
        // Let's reuse the structure but tweak aspect ratios basically.
        // Or add "scanline" doubles? No, that's too much data.
        
        // Just implement standard readable font logic again but maybe slightly different style.
        // Actually, "Retro" usually means Serifs or specific weird shapes.
        // Let's do a simple stick font but distinct.
        
         switch(c) {  // Note: switch on c (uint32_t) not ch (char)
            // --- PRIORITY HANDLERS ---
            case 219: // Block Cursor (CP437)
            case 0x2588: // Unicode Full Block - Added for compatibility
                 segs.push_back({L,T,R,T}); segs.push_back({R,T,R,B}); 
                 segs.push_back({R,B,L,B}); segs.push_back({L,B,L,T});
                 segs.push_back({L,T,R,B}); segs.push_back({R,T,L,B}); 
                 break;
                 
            // --- ALL PUNCTUATION (copied from ModernTerm) ---
            case '"': segs.push_back({CX-0.1f, T, CX-0.1f, T+0.2f}); segs.push_back({CX+0.1f, T, CX+0.1f, T+0.2f}); break;
            case '#': segs.push_back({L+0.1f, T, L+0.1f, B}); segs.push_back({R-0.1f, T, R-0.1f, B}); segs.push_back({L, M-0.1f, R, M-0.1f}); segs.push_back({L, M+0.1f, R, M+0.1f}); break;
            case '$': segs.push_back({R, T+0.1f, L, T}); segs.push_back({L, T, L, M}); segs.push_back({L, M, R, M}); segs.push_back({R, M, R, B-0.1f}); segs.push_back({R, B-0.1f, L, B}); segs.push_back({CX, T-0.05f, CX, B+0.05f}); break;
            case '%': segs.push_back({L, T, L+0.15f, T}); segs.push_back({L+0.15f, T, L+0.15f, T+0.15f}); segs.push_back({L+0.15f, T+0.15f, L, T+0.15f}); segs.push_back({L, T+0.15f, L, T}); segs.push_back({R-0.15f, B-0.15f, R, B-0.15f}); segs.push_back({R, B-0.15f, R, B}); segs.push_back({R, B, R-0.15f, B}); segs.push_back({R-0.15f, B, R-0.15f, B-0.15f}); segs.push_back({R, T, L, B}); break;
            case '&': segs.push_back({R, M, L, M}); segs.push_back({L, M, L, B}); segs.push_back({L, B, R, B}); segs.push_back({R, B, L, T}); segs.push_back({L, T, R-0.1f, T}); segs.push_back({R-0.1f, T, R, M}); break;
            case '\'': segs.push_back({CX, T, CX, T+0.2f}); break;
            case '(': segs.push_back({CX+0.1f, T, CX-0.1f, M}); segs.push_back({CX-0.1f, M, CX+0.1f, B}); break;
            case ')': segs.push_back({CX-0.1f, T, CX+0.1f, M}); segs.push_back({CX+0.1f, M, CX-0.1f, B}); break;
            case '*': segs.push_back({L, M-0.2f, R, M+0.2f}); segs.push_back({R, M-0.2f, L, M+0.2f}); segs.push_back({CX, M-0.25f, CX, M+0.25f}); break;
            case '+': segs.push_back({L, M, R, M}); segs.push_back({CX, M-0.2f, CX, M+0.2f}); break;
            case ',': segs.push_back({CX, B-0.05f, CX-0.1f, B+0.1f}); break;
            case '-': segs.push_back({L, M, R, M}); break;
            case '.': segs.push_back({CX, B-0.05f, CX, B}); break;
            case '/': segs.push_back({L, B, R, T}); break;
            case ':': segs.push_back({CX, M-0.05f, CX, M+0.05f}); segs.push_back({CX, B-0.1f, CX, B}); break;
            case ';': segs.push_back({CX, M-0.05f, CX, M+0.05f}); segs.push_back({CX, B-0.05f, CX-0.1f, B+0.1f}); break;
            case '<': segs.push_back({R, T, L, M}); segs.push_back({L, M, R, B}); break;
            case '=': segs.push_back({L, M-0.1f, R, M-0.1f}); segs.push_back({L, M+0.1f, R, M+0.1f}); break;
            case '>': segs.push_back({L, T, R, M}); segs.push_back({R, M, L, B}); break;
            case '?': segs.push_back({L, T, R, T}); segs.push_back({R, T, R, M}); segs.push_back({R, M, CX, M}); segs.push_back({CX, M, CX, B-0.2f}); segs.push_back({CX, B-0.1f, CX, B}); break;
            case '@': segs.push_back({R, B, L, B}); segs.push_back({L, B, L, T}); segs.push_back({L, T, R, T}); segs.push_back({R, T, R, M}); segs.push_back({R, M, CX, M}); segs.push_back({CX, M, CX, M+0.2f}); break;
            case '[': segs.push_back({R, T, L, T}); segs.push_back({L, T, L, B}); segs.push_back({L, B, R, B}); break;
            case '\\': segs.push_back({L, T, R, B}); break;
            case ']': segs.push_back({L, T, R, T}); segs.push_back({R, T, R, B}); segs.push_back({R, B, L, B}); break;
            case '^': segs.push_back({L, M, CX, T}); segs.push_back({CX, T, R, M}); break;
            case '_': segs.push_back({L, B, R, B}); break;
            case '`': segs.push_back({CX-0.1f, T, CX+0.1f, T+0.15f}); break;
            case '!': segs.push_back({CX, T, CX, B-0.2f}); segs.push_back({CX, B-0.1f, CX, B}); break;
            case '{': segs.push_back({R, T, CX, T}); segs.push_back({CX, T, CX, M}); segs.push_back({CX, M, L, M}); segs.push_back({CX, M, CX, B}); segs.push_back({CX, B, R, B}); break;
            case '|': segs.push_back({CX, T, CX, B}); break;
            case '}': segs.push_back({L, T, CX, T}); segs.push_back({CX, T, CX, M}); segs.push_back({CX, M, R, M}); segs.push_back({CX, M, CX, B}); segs.push_back({CX, B, L, B}); break;
            case '~': segs.push_back({L, M, CX, T+0.2f}); segs.push_back({CX, T+0.2f, R, M}); break;
            
            // --- ALPHA (with custom retro styles) ---
            case 'A': segs.push_back({L, B, CX, T}); segs.push_back({CX, T, R, B}); segs.push_back({L+0.1f, M, R-0.1f, M}); break;
            case 'B': segs.push_back({L, T, L, B}); segs.push_back({L, T, R-0.1f, T+0.1f}); segs.push_back({R-0.1f, T+0.1f, R, M});
                      segs.push_back({R, M, L, M}); segs.push_back({R, M, R, B-0.1f}); segs.push_back({R, B-0.1f, L, B}); break;
            case 'O': segs.push_back({L, T+0.1f, L, B-0.1f}); segs.push_back({L, B-0.1f, R, B-0.1f}); 
                      segs.push_back({R, B-0.1f, R, T+0.1f}); segs.push_back({R, T+0.1f, L, T+0.1f}); break;
            
            default:
                // Use a "standard" set for all chars to save space, 
                // but make them look "Retro" (e.g. 0 with slash, 1 with base)
                if (ch >= 'A' && ch <= 'Z') {
                     // Standard Upper
                     if(ch=='I'){ segs.push_back({L,T,R,T}); segs.push_back({CX,T,CX,B}); segs.push_back({L,B,R,B}); }
                     else if(ch=='H'){ segs.push_back({L,T,L,B}); segs.push_back({R,T,R,B}); segs.push_back({L,M,R,M}); }
                     else {
                         // Fallback to ModernTerm logic if not defined for now, 
                         // but ideally we default to empty to allow bitmap fallback?
                         // Actually the user wants "Retro" style. Falling back to ModernTerm is probably fine for A-Z.
                         // BUT falling back to ModernTerm returns empty for *its* defaults now (which is good).
                         // So checking ModernTerm is safe.
                         return ModernTermFont().getSegments(c);
                     }
                } else if (ch >= 'a' && ch <= 'z') {
                    return ModernTermFont().getSegments(c);
                } else if (ch >= '0' && ch <= '9') {
                    if (ch == '0') { // Diagonal slash
                         segs.push_back({L, T, R, T}); segs.push_back({R, T, R, B});
                         segs.push_back({R, B, L, B}); segs.push_back({L, B, L, T});
                         segs.push_back({L, B, R, T}); 
                    } else {
                         return ModernTermFont().getSegments(c);
                    }
                } else {
                    // For symbols, fallback to ModernTerm.
                    // If ModernTerm is empty, this returns empty.
                    // If ModernTerm has segments, this returns segments.
                    // This seems correct for CrtRetro relying on ModernTerm base.
                    return ModernTermFont().getSegments(c);
                }
                break;
         }
        
        return segs;
    }
    
    const char* name() const override { return "CRT Retro"; }
};
