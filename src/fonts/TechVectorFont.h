#pragma once

#include "FontAsset.h"
#include <vector>

class TechVectorFont : public FontAsset {
public:
    FontType type() const override { return FontType::Vector; }
    int width() const override { return 12; }
    int height() const override { return 18; }
    
    // "Tech" Font: Angular, Sci-Fi, Readable
    // A simplified vector font that looks like CAD/HUD text
    std::vector<VectorSegment> getSegments(uint32_t c) const override {
        std::vector<VectorSegment> segs;
        
        // Helper constraints (0.0 - 1.0)
        float L = 0.2f, R = 0.8f;
        float T = 0.1f, B = 0.9f;
        float M = 0.5f; // Middle Y
        float CX = 0.5f; // Center X
        
        // Uppercase Logic (Simplified for brevity, expands real usage)
        char ch = (char)(c & 0xFF);
        bool upper = (ch >= 'A' && ch <= 'Z');
        bool lower = (ch >= 'a' && ch <= 'z');
        
        // If lowercase, squish vertically for "Small Caps" look
        float top = T;
        float bot = B;
        if (lower) {
            top = M - 0.1f; // Start lower
             // Expand B slightly?
             ch = toupper(ch); // Use uppercase logic
        } else {
             // Keep original
        }
        
        if (upper || lower) {
            switch(ch) {
                case 'A': 
                    segs.push_back({L, bot, CX, top}); segs.push_back({CX, top, R, bot}); 
                    segs.push_back({L+0.1f, M, R-0.1f, M}); break;
                case 'B':
                    segs.push_back({L, bot, L, top}); segs.push_back({L, top, R-0.1f, top});
                    segs.push_back({R-0.1f, top, R, top+0.1f}); segs.push_back({R, top+0.1f, R, M-0.1f});
                    segs.push_back({R, M-0.1f, R-0.1f, M}); segs.push_back({R-0.1f, M, L, M});
                    segs.push_back({L, M, R-0.1f, M}); segs.push_back({R-0.1f, M, R, M+0.1f});
                    segs.push_back({R, M+0.1f, R, bot-0.1f}); segs.push_back({R, bot-0.1f, R-0.1f, bot});
                    segs.push_back({R-0.1f, bot, L, bot}); break;
                case 'C':
                    segs.push_back({R, top+0.2f, R, top}); segs.push_back({R, top, L, top});
                    segs.push_back({L, top, L, bot}); segs.push_back({L, bot, R, bot});
                    segs.push_back({R, bot, R, bot-0.2f}); break;
                case 'D':
                    segs.push_back({L, bot, L, top}); segs.push_back({L, top, R-0.2f, top});
                    segs.push_back({R-0.2f, top, R, top+0.2f}); segs.push_back({R, top+0.2f, R, bot-0.2f});
                    segs.push_back({R, bot-0.2f, R-0.2f, bot}); segs.push_back({R-0.2f, bot, L, bot}); break;
                case 'E':
                    segs.push_back({R, top, L, top}); segs.push_back({L, top, L, bot});
                    segs.push_back({L, bot, R, bot}); segs.push_back({L, M, R-0.2f, M}); break;
                case 'F':
                    segs.push_back({R, top, L, top}); segs.push_back({L, top, L, bot});
                    segs.push_back({L, M, R-0.2f, M}); break;
                case 'G':
                    segs.push_back({R, top+0.2f, R, top}); segs.push_back({R, top, L, top});
                    segs.push_back({L, top, L, bot}); segs.push_back({L, bot, R, bot});
                    segs.push_back({R, bot, R, M}); segs.push_back({R, M, CX, M}); break;
                case 'H':
                    segs.push_back({L, top, L, bot}); segs.push_back({R, top, R, bot});
                    segs.push_back({L, M, R, M}); break;
                case 'I':
                    segs.push_back({L, top, R, top}); segs.push_back({CX, top, CX, bot});
                    segs.push_back({L, bot, R, bot}); break;
                case 'J':
                    segs.push_back({R, top, R, bot}); segs.push_back({R, bot, L+0.2f, bot});
                    segs.push_back({L+0.2f, bot, L, bot-0.2f}); segs.push_back({L, bot-0.2f, L, M+0.2f}); break;
                case 'K':
                    segs.push_back({L, top, L, bot}); segs.push_back({R, top, L, M});
                    segs.push_back({L, M, R, bot}); break;
                case 'L':
                    segs.push_back({L, top, L, bot}); segs.push_back({L, bot, R, bot}); break;
                case 'M':
                    segs.push_back({L, bot, L, top}); segs.push_back({L, top, CX, M});
                    segs.push_back({CX, M, R, top}); segs.push_back({R, top, R, bot}); break;
                case 'N':
                    segs.push_back({L, bot, L, top}); segs.push_back({L, top, R, bot});
                    segs.push_back({R, bot, R, top}); break;
                case 'O':
                    segs.push_back({L, top, R, top}); segs.push_back({R, top, R, bot});
                    segs.push_back({R, bot, L, bot}); segs.push_back({L, bot, L, top}); break;
                case 'P':
                    segs.push_back({L, bot, L, top}); segs.push_back({L, top, R, top});
                    segs.push_back({R, top, R, M}); segs.push_back({R, M, L, M}); break;
                case 'Q':
                    segs.push_back({L, top, R, top}); segs.push_back({R, top, R, bot});
                    segs.push_back({R, bot, L, bot}); segs.push_back({L, bot, L, top});
                    segs.push_back({CX, M, R, bot}); break;
                case 'R':
                    segs.push_back({L, bot, L, top}); segs.push_back({L, top, R, top});
                    segs.push_back({R, top, R, M}); segs.push_back({R, M, L, M});
                    segs.push_back({L, M, R, bot}); break;
                case 'S':
                    segs.push_back({R, top, L, top}); segs.push_back({L, top, L, M});
                    segs.push_back({L, M, R, M}); segs.push_back({R, M, R, bot});
                    segs.push_back({R, bot, L, bot}); break;
                case 'T':
                    segs.push_back({L, top, R, top}); segs.push_back({CX, top, CX, bot}); break;
                case 'U':
                    segs.push_back({L, top, L, bot}); segs.push_back({L, bot, R, bot});
                    segs.push_back({R, bot, R, top}); break;
                case 'V':
                    segs.push_back({L, top, CX, bot}); segs.push_back({CX, bot, R, top}); break;
                case 'W':
                    segs.push_back({L, top, L, bot}); segs.push_back({L, bot, CX, M});
                    segs.push_back({CX, M, R, bot}); segs.push_back({R, bot, R, top}); break;
                case 'X':
                    segs.push_back({L, top, R, bot}); segs.push_back({R, top, L, bot}); break;
                case 'Y':
                    segs.push_back({L, top, CX, M}); segs.push_back({R, top, CX, M});
                    segs.push_back({CX, M, CX, bot}); break;
                case 'Z':
                    segs.push_back({L, top, R, top}); segs.push_back({R, top, L, bot});
                    segs.push_back({L, bot, R, bot}); break;
            }
            return segs;
        }

        // Use c directly
        switch(c) {
            case 'A':
                segs.push_back({L, B, CX, T});
                segs.push_back({CX, T, R, B});
                segs.push_back({L, M, L+0.2f, M}); // Sci-fi: partial crossbar
                segs.push_back({R, M, R-0.2f, M}); // Split crossbar
                break;
            case '0': 
                segs.push_back({L, T, R, T}); segs.push_back({R, T, R, B});
                segs.push_back({R, B, L, B}); segs.push_back({L, B, L, T}); 
                segs.push_back({L, T, R, B}); break; 
            case '1': segs.push_back({CX, T, CX, B}); segs.push_back({L, T+0.2f, CX, T}); break;
            case '2':
                segs.push_back({L, T, R, T}); segs.push_back({R, T, R, M});
                segs.push_back({R, M, L, M}); segs.push_back({L, M, L, B});
                segs.push_back({L, B, R, B}); break;
            case '3':
                segs.push_back({L, T, R, T}); segs.push_back({R, T, R, B});
                segs.push_back({R, B, L, B}); segs.push_back({L, M, R, M}); break;
            case '4':
                segs.push_back({L, T, L, M}); segs.push_back({L, M, R, M});
                segs.push_back({R, T, R, B}); break;
            case '5':
                segs.push_back({R, T, L, T}); segs.push_back({L, T, L, M});
                segs.push_back({L, M, R, M}); segs.push_back({R, M, R, B});
                segs.push_back({R, B, L, B}); break;
            case '6':
                segs.push_back({R, T, L, T}); segs.push_back({L, T, L, B});
                segs.push_back({L, B, R, B}); segs.push_back({R, B, R, M});
                segs.push_back({R, M, L, M}); break;
            case '7':
                segs.push_back({L, T, R, T}); segs.push_back({R, T, L+0.2f, B}); break;
            case '8':
                segs.push_back({L, T, R, T}); segs.push_back({R, T, R, B});
                segs.push_back({R, B, L, B}); segs.push_back({L, B, L, T});
                segs.push_back({L, M, R, M}); break;
            case '9':
                segs.push_back({L, B, R, B}); segs.push_back({R, B, R, T});
                segs.push_back({R, T, L, T}); segs.push_back({L, T, L, M});
                segs.push_back({L, M, R, M}); break;
            
            // Basic Symbols
            case '-': segs.push_back({L, M, R, M}); break;
            case '_': segs.push_back({L, B, R, B}); break;
            case '=': segs.push_back({L, M-0.1f, R, M-0.1f}); segs.push_back({L, M+0.1f, R, M+0.1f}); break;
            case '+': segs.push_back({L, M, R, M}); segs.push_back({CX, T+0.2f, CX, B-0.2f}); break;
            case '<': segs.push_back({R, T, L, M}); segs.push_back({L, M, R, B}); break;
            case '>': segs.push_back({L, T, R, M}); segs.push_back({R, M, L, B}); break;
            case '/': segs.push_back({R, T, L, B}); break;
            case '\\': segs.push_back({L, T, R, B}); break;
            case '|': segs.push_back({CX, T, CX, B}); break;
            case '[': segs.push_back({R-0.2f, T, L, T}); segs.push_back({L, T, L, B}); segs.push_back({L, B, R-0.2f, B}); break;
            case ']': segs.push_back({L+0.2f, T, R, T}); segs.push_back({R, T, R, B}); segs.push_back({R, B, L+0.2f, B}); break;
            case '(': segs.push_back({CX+0.1f, T, CX-0.1f, M}); segs.push_back({CX-0.1f, M, CX+0.1f, B}); break;
            case ')': segs.push_back({CX-0.1f, T, CX+0.1f, M}); segs.push_back({CX+0.1f, M, CX-0.1f, B}); break;
            case '{': segs.push_back({R-0.1f, T, L, T+0.1f}); segs.push_back({L, T+0.1f, L, M}); segs.push_back({L, M, L-0.1f, M}); segs.push_back({L, M, L, B-0.1f}); segs.push_back({L, B-0.1f, R-0.1f, B}); break;
            case '}': segs.push_back({L+0.1f, T, R, T+0.1f}); segs.push_back({R, T+0.1f, R, M}); segs.push_back({R, M, R+0.1f, M}); segs.push_back({R, M, R, B-0.1f}); segs.push_back({R, B-0.1f, L+0.1f, B}); break;
            
            case '.': segs.push_back({CX, B-0.1f, CX, B}); break;
            case ',': segs.push_back({CX, B-0.1f, CX-0.2f, B+0.2f}); break; 
            case ':': segs.push_back({CX, T+0.3f, CX, T+0.4f}); segs.push_back({CX, B-0.4f, CX, B-0.3f}); break;
            case ';': segs.push_back({CX, T+0.3f, CX, T+0.4f}); segs.push_back({CX, B-0.3f, CX-0.2f, B}); break;
            case '!': segs.push_back({CX, T, CX, B-0.2f}); segs.push_back({CX, B-0.1f, CX, B}); break;
            case '?': 
                segs.push_back({L, T+0.1f, L+0.1f, T}); segs.push_back({L+0.1f, T, R, T});
                segs.push_back({R, T, R, M}); segs.push_back({R, M, CX, M});
                segs.push_back({CX, M, CX, B-0.2f}); segs.push_back({CX, B-0.1f, CX, B}); break;
            
            case '~': segs.push_back({L, M, CX, T}); segs.push_back({CX, T, R, M}); break;
            case '@': segs.push_back({R, B, R, T}); segs.push_back({R, T, L, T}); segs.push_back({L, T, L, B}); segs.push_back({L, B, CX, B}); segs.push_back({CX, B, CX, M}); segs.push_back({CX, M, R-0.2f, M}); break;
            case '#': segs.push_back({L+0.2f, T, L+0.2f, B}); segs.push_back({R-0.2f, T, R-0.2f, B}); segs.push_back({L, T+0.2f, R, T+0.2f}); segs.push_back({L, B-0.2f, R, B-0.2f}); break;
            
            // Missing ASCII Ennchancements
            case '"': segs.push_back({CX-0.1f, T, CX-0.1f, T+0.2f}); segs.push_back({CX+0.1f, T, CX+0.1f, T+0.2f}); break;
            case '$': // S with line
                     segs.push_back({R, T+0.1f, L+0.1f, T+0.05f}); segs.push_back({L+0.1f, T+0.05f, L, M-0.05f});
                     segs.push_back({L, M-0.05f, R, M+0.05f}); segs.push_back({R, M+0.05f, R, B-0.05f});
                     segs.push_back({R, B-0.05f, L, B-0.1f}); 
                     segs.push_back({CX, T-0.1f, CX, B+0.1f}); break;
            case '%':
                     segs.push_back({L, T, L+0.2f, T}); segs.push_back({L+0.2f,T,L+0.2f,T+0.2f}); segs.push_back({L+0.2f,T+0.2f,L,T+0.2f}); segs.push_back({L,T+0.2f,L,T}); // Top circle
                     segs.push_back({R-0.2f, B-0.2f, R, B-0.2f}); segs.push_back({R,B-0.2f,R,B}); segs.push_back({R,B,R-0.2f,B}); segs.push_back({R-0.2f,B,R-0.2f,B-0.2f}); // Bot circle
                     segs.push_back({R, T, L, B}); break; // Slash
            case '&':
                     segs.push_back({R, M, L, M}); segs.push_back({L, M, L, B}); segs.push_back({L, B, R, B}); // Loop bot
                     segs.push_back({R, B, L, T}); segs.push_back({L, T, R-0.1f, T}); segs.push_back({R-0.1f, T, R, M}); break; // Loop top
            case '\'': segs.push_back({CX, T, CX, T+0.2f}); break;
            case '*': 
                     segs.push_back({L, M-0.2f, R, M+0.2f}); segs.push_back({R, M-0.2f, L, M+0.2f}); // X
                     segs.push_back({CX, M-0.3f, CX, M+0.3f}); break; // Vertical
            case '^': segs.push_back({L, M, CX, T}); segs.push_back({CX, T, R, M}); break;
            case '`': segs.push_back({CX-0.1f, T, CX+0.1f, T+0.15f}); break;

            case ' ': break; // Space

            // BLOCK CURSOR (Simulated)
            case 219: // CP437 Block (Mapped from 0x2588)
                     segs.push_back({L, T, R, T}); segs.push_back({R, T, R, B});
                     segs.push_back({R, B, L, B}); segs.push_back({L, B, L, T});
                     // X-Cross fill
                     segs.push_back({L, T, R, B}); segs.push_back({R, T, L, B});
                     segs.push_back({L, M, R, M}); segs.push_back({CX, T, CX, B});
                     break;

            default: 
                 // Simple small box for missing chars
                 float sz = 0.2f;
                 segs.push_back({CX-sz, M-sz, CX+sz, M-sz});
                 segs.push_back({CX+sz, M-sz, CX+sz, M+sz});
                 segs.push_back({CX+sz, M+sz, CX-sz, M+sz});
                 segs.push_back({CX-sz, M+sz, CX-sz, M-sz});
                 break;
        }
        return segs;
    }
    
    const char* name() const override { return "Tech Vector 12x18"; }
    bool getPixel(uint32_t c, int x, int y) const override { return false; } // uint32_t signature
};
