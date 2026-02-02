#pragma once

#include "FontAsset.h"
#include <vector>
#include <cmath>

class ModernTermFont : public FontAsset {
public:
    FontType type() const override { return FontType::Vector; }
    int width() const override { return 10; } // Slightly narrower than Tech
    int height() const override { return 18; }
    
    // Modern Term: Clean, minimal, sans-serif, high readability
    // Similar to Roboto Mono / SF Mono
    std::vector<VectorSegment> getSegments(uint32_t c) const override {
        std::vector<VectorSegment> segs;
        
        // Bounds
        float L = 0.2f, R = 0.8f;
        float T = 0.15f, B = 0.85f;
        float M = 0.5f;
        float CX = 0.5f;
        
        char ch = (char)(c & 0xFF);
        
        // Basic mapping for common ASCII
        // Basic mapping for common ASCII
        switch(c) {
            case 0x2588: // Full Block
            case 219:
                segs.push_back({L,T,R,T}); segs.push_back({R,T,R,B}); 
                segs.push_back({R,B,L,B}); segs.push_back({L,B,L,T});
                segs.push_back({L,T,R,B}); segs.push_back({R,T,L,B}); // X-Box
                break;

            // --- Uppercase ---
            case 'A': 
                segs.push_back({L, B, CX, T}); segs.push_back({CX, T, R, B}); 
                segs.push_back({L+0.1f, M+0.1f, R-0.1f, M+0.1f}); break;
            case 'B':
                segs.push_back({L, T, L, B}); 
                segs.push_back({L, T, R-0.1f, T}); segs.push_back({R-0.1f, T, R, T+0.1f});
                segs.push_back({R, T+0.1f, R, M-0.05f}); segs.push_back({R, M-0.05f, R-0.1f, M});
                segs.push_back({R-0.1f, M, L, M});
                segs.push_back({L, M, R-0.1f, M}); segs.push_back({R-0.1f, M, R, M+0.05f});
                segs.push_back({R, M+0.05f, R, B-0.1f}); segs.push_back({R, B-0.1f, R-0.1f, B});
                segs.push_back({R-0.1f, B, L, B}); break;
            case 'C':
                segs.push_back({R, T+0.1f, R-0.1f, T}); segs.push_back({R-0.1f, T, L+0.1f, T});
                segs.push_back({L+0.1f, T, L, T+0.1f}); segs.push_back({L, T+0.1f, L, B-0.1f});
                segs.push_back({L, B-0.1f, L+0.1f, B}); segs.push_back({L+0.1f, B, R-0.1f, B});
                segs.push_back({R-0.1f, B, R, B-0.1f}); break;
            case 'D':
                segs.push_back({L, T, L, B}); segs.push_back({L, T, R-0.1f, T});
                segs.push_back({R-0.1f, T, R, T+0.15f}); segs.push_back({R, T+0.15f, R, B-0.15f});
                segs.push_back({R, B-0.15f, R-0.1f, B}); segs.push_back({R-0.1f, B, L, B}); break;
            case 'E':
                segs.push_back({R, T, L, T}); segs.push_back({L, T, L, B}); segs.push_back({L, B, R, B});
                segs.push_back({L, M, R-0.1f, M}); break;
            case 'F':
                segs.push_back({R, T, L, T}); segs.push_back({L, T, L, B});
                segs.push_back({L, M, R-0.1f, M}); break;
            case 'G': // Like C but with hook
                segs.push_back({R, T+0.1f, R-0.1f, T}); segs.push_back({R-0.1f, T, L+0.1f, T});
                segs.push_back({L+0.1f, T, L, T+0.1f}); segs.push_back({L, T+0.1f, L, B-0.1f});
                segs.push_back({L, B-0.1f, L+0.1f, B}); segs.push_back({L+0.1f, B, R-0.1f, B});
                segs.push_back({R-0.1f, B, R, B-0.1f}); segs.push_back({R, B-0.1f, R, M});
                segs.push_back({R, M, CX+0.1f, M}); break;
            case 'H':
                segs.push_back({L, T, L, B}); segs.push_back({R, T, R, B}); segs.push_back({L, M, R, M}); break;
            case 'I':
                segs.push_back({CX, T, CX, B}); segs.push_back({L+0.1f, T, R-0.1f, T}); segs.push_back({L+0.1f, B, R-0.1f, B}); break;
            case 'J':
                segs.push_back({R, T, R, B-0.1f}); segs.push_back({R, B-0.1f, R-0.1f, B});
                segs.push_back({R-0.1f, B, CX, B}); segs.push_back({CX, B, L, B-0.1f}); break;
            case 'K':
                segs.push_back({L, T, L, B}); segs.push_back({R, T, L, M}); segs.push_back({L, M, R, B}); break;
            case 'L':
                segs.push_back({L, T, L, B}); segs.push_back({L, B, R, B}); break;
            case 'M':
                segs.push_back({L, B, L, T}); segs.push_back({L, T, CX, M+0.1f}); 
                segs.push_back({CX, M+0.1f, R, T}); segs.push_back({R, T, R, B}); break;
            case 'N':
                segs.push_back({L, B, L, T}); segs.push_back({L, T, R, B}); segs.push_back({R, B, R, T}); break;
            case 'O':
                segs.push_back({L, T+0.1f, L, B-0.1f}); segs.push_back({L, B-0.1f, L+0.1f, B});
                segs.push_back({L+0.1f, B, R-0.1f, B}); segs.push_back({R-0.1f, B, R, B-0.1f});
                segs.push_back({R, B-0.1f, R, T+0.1f}); segs.push_back({R, T+0.1f, R-0.1f, T});
                segs.push_back({R-0.1f, T, L+0.1f, T}); segs.push_back({L+0.1f, T, L, T+0.1f}); break;
            case 'P':
                segs.push_back({L, B, L, T}); segs.push_back({L, T, R-0.1f, T});
                segs.push_back({R-0.1f, T, R, T+0.1f}); segs.push_back({R, T+0.1f, R, M-0.1f});
                segs.push_back({R, M-0.1f, R-0.1f, M}); segs.push_back({R-0.1f, M, L, M}); break;
            case 'Q':
                // O loop
                segs.push_back({L, T+0.1f, L, B-0.1f}); segs.push_back({L, B-0.1f, L+0.1f, B});
                segs.push_back({L+0.1f, B, R-0.1f, B}); segs.push_back({R-0.1f, B, R, B-0.1f});
                segs.push_back({R, B-0.1f, R, T+0.1f}); segs.push_back({R, T+0.1f, R-0.1f, T});
                segs.push_back({R-0.1f, T, L+0.1f, T}); segs.push_back({L+0.1f, T, L, T+0.1f});
                // Tail
                segs.push_back({CX+0.1f, M+0.2f, R, B}); break;
            case 'R':
                segs.push_back({L, B, L, T}); segs.push_back({L, T, R-0.1f, T});
                segs.push_back({R-0.1f, T, R, T+0.1f}); segs.push_back({R, T+0.1f, R, M-0.1f});
                segs.push_back({R, M-0.1f, R-0.1f, M}); segs.push_back({R-0.1f, M, L, M});
                segs.push_back({L+0.2f, M, R, B}); break;
            case 'S':
                segs.push_back({R, T+0.1f, L+0.1f, T+0.05f}); segs.push_back({L+0.1f, T+0.05f, L, M-0.05f});
                segs.push_back({L, M-0.05f, R, M+0.05f}); segs.push_back({R, M+0.05f, R, B-0.05f});
                segs.push_back({R, B-0.05f, L, B-0.1f}); break;
            case 'T':
                segs.push_back({L, T, R, T}); segs.push_back({CX, T, CX, B}); break;
            case 'U':
                segs.push_back({L, T, L, B-0.1f}); segs.push_back({L, B-0.1f, L+0.1f, B});
                segs.push_back({L+0.1f, B, R-0.1f, B}); segs.push_back({R-0.1f, B, R, B-0.1f});
                segs.push_back({R, B-0.1f, R, T}); break;
            case 'V':
                segs.push_back({L, T, CX, B}); segs.push_back({CX, B, R, T}); break;
            case 'W':
                segs.push_back({L, T, L+0.1f, B}); segs.push_back({L+0.1f, B, CX, M});
                segs.push_back({CX, M, R-0.1f, B}); segs.push_back({R-0.1f, B, R, T}); break;
            case 'X':
                segs.push_back({L, T, R, B}); segs.push_back({R, T, L, B}); break;
            case 'Y':
                segs.push_back({L, T, CX, M}); segs.push_back({R, T, CX, M});
                segs.push_back({CX, M, CX, B}); break;
            case 'Z':
                segs.push_back({L, T, R, T}); segs.push_back({R, T, L, B}); segs.push_back({L, B, R, B}); break;
                
             // --- Lowercase (Simplified scaling of Upper for now, or proper lowercase if time permits) ---
             // Let's do proper lowercase for "readable" fonts
             case 'a': segs.push_back({R, M, R, B}); segs.push_back({L, M+0.1f, R, M+0.1f}); 
                       segs.push_back({L, M+0.1f, L, B-0.1f}); segs.push_back({L, B-0.1f, R, B}); break;
             case 'b': segs.push_back({L, T, L, B}); segs.push_back({L, M+0.1f, R-0.1f, M+0.1f});
                       segs.push_back({R-0.1f, M+0.1f, R, M+0.2f}); segs.push_back({R, M+0.2f, R, B-0.1f});
                       segs.push_back({R, B-0.1f, R-0.1f, B}); segs.push_back({R-0.1f, B, L, B}); break;
             case 'c': segs.push_back({R, M+0.2f, R-0.1f, M+0.1f}); segs.push_back({R-0.1f, M+0.1f, L+0.1f, M+0.1f});
                       segs.push_back({L+0.1f, M+0.1f, L, M+0.2f}); segs.push_back({L, M+0.2f, L, B-0.1f});
                       segs.push_back({L, B-0.1f, L+0.1f, B}); segs.push_back({L+0.1f, B, R-0.1f, B}); break;
             case 'd': segs.push_back({R, T, R, B}); segs.push_back({R, M+0.1f, L+0.1f, M+0.1f});
                       segs.push_back({L+0.1f, M+0.1f, L, M+0.2f}); segs.push_back({L, M+0.2f, L, B-0.1f});
                       segs.push_back({L, B-0.1f, L+0.1f, B}); segs.push_back({L+0.1f, B, R, B}); break;
             case 'e': segs.push_back({L, M+0.2f, R, M+0.2f}); segs.push_back({R, M+0.2f, R, M+0.1f});
                       segs.push_back({R, M+0.1f, L, M+0.1f}); segs.push_back({L, M+0.1f, L, B});
                       segs.push_back({L, B, R, B}); break;
             case 'f': segs.push_back({CX, B, CX, T+0.1f}); segs.push_back({CX, T+0.1f, R, T});
                       segs.push_back({L, M, R, M}); break;
             case 'g': segs.push_back({R, M, R, B+0.2f}); segs.push_back({R, B+0.2f, L, B+0.2f});
                       // Circle part
                       segs.push_back({L, M, R, M}); segs.push_back({L, M, L, B}); segs.push_back({L, B, R, B}); break;
             case 'h': segs.push_back({L, T, L, B}); segs.push_back({L, M+0.1f, R, M+0.1f}); segs.push_back({R, M+0.1f, R, B}); break;
             case 'i': segs.push_back({CX, M, CX, B}); segs.push_back({CX, M-0.2f, CX, M-0.15f}); break; // Dot
             case 'j': segs.push_back({CX, M, CX, B+0.2f}); segs.push_back({CX, B+0.2f, L, B+0.2f}); 
                       segs.push_back({CX, M-0.2f, CX, M-0.15f}); break;
             case 'k': segs.push_back({L, T, L, B}); segs.push_back({R, M, L, M+0.2f}); segs.push_back({L, M+0.2f, R, B}); break;
             case 'l': segs.push_back({CX, T, CX, B}); break; // Simple l
             case 'm': segs.push_back({L, M, L, B}); segs.push_back({L, M, CX, M}); segs.push_back({CX, M, CX, B});
                       segs.push_back({CX, M, R, M}); segs.push_back({R, M, R, B}); break;
             case 'n': segs.push_back({L, M, L, B}); segs.push_back({L, M, R, M}); segs.push_back({R, M, R, B}); break;
             case 'o': segs.push_back({L, M, L, B}); segs.push_back({L, B, R, B}); 
                       segs.push_back({R, B, R, M}); segs.push_back({R, M, L, M}); break;
             case 'p': segs.push_back({L, M, L, B+0.2f}); segs.push_back({L, M, R, M}); 
                       segs.push_back({R, M, R, B}); segs.push_back({R, B, L, B}); break;
             case 'q': segs.push_back({R, M, R, B+0.2f}); segs.push_back({R, M, L, M});
                       segs.push_back({L, M, L, B}); segs.push_back({L, B, R, B}); break;
             case 'r': segs.push_back({L, M, L, B}); segs.push_back({L, M+0.05f, R, M}); break;
             case 's': segs.push_back({R, M, L, M}); segs.push_back({L, M, L, M+0.2f}); 
                       segs.push_back({L, M+0.2f, R, M+0.2f}); segs.push_back({R, M+0.2f, R, B});
                       segs.push_back({R, B, L, B}); break;
             case 't': segs.push_back({CX, T, CX, B}); segs.push_back({L, M, R, M}); break;
             case 'u': segs.push_back({L, M, L, B}); segs.push_back({L, B, R, B}); segs.push_back({R, B, R, M}); break;
             case 'v': segs.push_back({L, M, CX, B}); segs.push_back({CX, B, R, M}); break;
             case 'w': segs.push_back({L, M, CX-0.1f, B}); segs.push_back({CX-0.1f, B, CX, M+0.2f});
                       segs.push_back({CX, M+0.2f, CX+0.1f, B}); segs.push_back({CX+0.1f, B, R, M}); break;
             case 'x': segs.push_back({L, M, R, B}); segs.push_back({R, M, L, B}); break; 
             case 'y': segs.push_back({L, M, R, B+0.2f}); segs.push_back({R, M, L, B}); break;
             case 'z': segs.push_back({L, M, R, M}); segs.push_back({R, M, L, B}); segs.push_back({L, B, R, B}); break;
                
            // --- Numbers ---
            case '0': segs.push_back({L, T, R, T}); segs.push_back({R, T, R, B}); 
                      segs.push_back({R, B, L, B}); segs.push_back({L, B, L, T}); break;
            case '1': segs.push_back({CX-0.1f, T+0.1f, CX, T}); segs.push_back({CX, T, CX, B}); break;
            case '2': segs.push_back({L, T, R, T}); segs.push_back({R, T, R, M}); 
                      segs.push_back({R, M, L, B}); segs.push_back({L, B, R, B}); break;
            case '3': segs.push_back({L, T, R, T}); segs.push_back({R, T, R, B});
                      segs.push_back({R, B, L, B}); segs.push_back({L, M, R, M}); break;
            case '4': segs.push_back({R, M, R, B}); segs.push_back({R, B, R, T}); // stem
                      segs.push_back({R, T, L, M}); segs.push_back({L, M, R, M}); break; // this is messy, let's fix
                      // 4 logic: L-Shape then cross
                      segs.clear();
                      segs.push_back({R, T, R, B}); segs.push_back({R, M, L, M}); segs.push_back({L, M, L, T}); break;
            case '5': segs.push_back({R, T, L, T}); segs.push_back({L, T, L, M});
                      segs.push_back({L, M, R, M}); segs.push_back({R, M, R, B});
                      segs.push_back({R, B, L, B}); break;
            case '6': segs.push_back({R, T, L, T}); segs.push_back({L, T, L, B});
                      segs.push_back({L, B, R, B}); segs.push_back({R, B, R, M});
                      segs.push_back({R, M, L, M}); break;
            case '7': segs.push_back({L, T, R, T}); segs.push_back({R, T, L+0.2f, B}); break;
            case '8': segs.push_back({L, T, R, T}); segs.push_back({R, T, R, B});
                      segs.push_back({R, B, L, B}); segs.push_back({L, B, L, T});
                      segs.push_back({L, M, R, M}); break;
            case '9': segs.push_back({L, B, R, B}); segs.push_back({R, B, R, T});
                      segs.push_back({R, T, L, T}); segs.push_back({L, T, L, M});
                      segs.push_back({L, M, R, M}); break;
            
            // --- Symbols ---
            case '-': segs.push_back({L, M, R, M}); break;
            case '_': segs.push_back({L, B, R, B}); break;
            case '=': segs.push_back({L, M-0.1f, R, M-0.1f}); segs.push_back({L, M+0.1f, R, M+0.1f}); break;
            case '+': segs.push_back({L, M, R, M}); segs.push_back({CX, M-0.2f, CX, M+0.2f}); break;
            case '!': segs.push_back({CX, T, CX, B-0.2f}); segs.push_back({CX, B-0.1f, CX, B}); break;
            case '.': segs.push_back({CX, B-0.05f, CX, B}); break;
            case ',': segs.push_back({CX, B-0.05f, CX-0.05f, B+0.1f}); break;
            case ':': segs.push_back({CX, M-0.1f, CX, M}); segs.push_back({CX, B-0.1f, CX, B}); break;
            case ';': segs.push_back({CX, M-0.1f, CX, M}); segs.push_back({CX, B-0.1f, CX-0.05f, B+0.1f}); break;
            case '(': segs.push_back({R, T, L, M}); segs.push_back({L, M, R, B}); break;
            case ')': segs.push_back({L, T, R, M}); segs.push_back({R, M, L, B}); break;
            case '[': segs.push_back({R, T, L, T}); segs.push_back({L, T, L, B}); segs.push_back({L, B, R, B}); break;
            case ']': segs.push_back({L, T, R, T}); segs.push_back({R, T, R, B}); segs.push_back({R, B, L, B}); break;
            case '/': segs.push_back({L, B, R, T}); break;
            case '\\': segs.push_back({L, T, R, B}); break;
            case '<': segs.push_back({R, T, L, M}); segs.push_back({L, M, R, B}); break;
            case '>': segs.push_back({L, T, R, M}); segs.push_back({R, M, L, B}); break;
            case '|': segs.push_back({CX, T, CX, B}); break;
            case '@': // Simplified swirl
                      segs.push_back({R, B, L, B}); segs.push_back({L, B, L, T});
                      segs.push_back({L, T, R, T}); segs.push_back({R, T, R, M});
                      segs.push_back({R, M, CX, M}); segs.push_back({CX, M, CX, M+0.2f}); break;
            case '#': 
                     segs.push_back({L+0.1f, T, L-0.1f, B}); segs.push_back({R+0.1f, T, R-0.1f, B});
                     segs.push_back({L, M-0.1f, R, M-0.1f}); segs.push_back({L, M+0.1f, R, M+0.1f}); break;

            // Missing ASCII Ennchancements
            case '"': segs.push_back({CX-0.1f, T, CX-0.1f, T+0.2f}); segs.push_back({CX+0.1f, T, CX+0.1f, T+0.2f}); break;
            case '$': // S with line
                     segs.push_back({R, T+0.1f, L+0.1f, T+0.05f}); segs.push_back({L+0.1f, T+0.05f, L, M-0.05f});
                     segs.push_back({L, M-0.05f, R, M+0.05f}); segs.push_back({R, M+0.05f, R, B-0.05f});
                     segs.push_back({R, B-0.05f, L, B-0.1f}); 
                     segs.push_back({CX, T-0.05f, CX, B+0.05f}); break;
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
            case '{': segs.push_back({R, T, CX, T}); segs.push_back({CX, T, CX, M}); segs.push_back({CX, M, L, M}); segs.push_back({CX, M, CX, B}); segs.push_back({CX, B, R, B}); break;
            case '}': segs.push_back({L, T, CX, T}); segs.push_back({CX, T, CX, M}); segs.push_back({CX, M, R, M}); segs.push_back({CX, M, CX, B}); segs.push_back({CX, B, L, B}); break;
            case '~': segs.push_back({L, M, CX, T+0.2f}); segs.push_back({CX, T+0.2f, R, M}); break;

            case '?': 
                     segs.push_back({L, T+0.1f, L+0.1f, T}); segs.push_back({L+0.1f, T, R-0.1f, T});
                     segs.push_back({R-0.1f, T, R, T+0.1f}); segs.push_back({R, T+0.1f, R, M});
                     segs.push_back({R, M, CX, M}); segs.push_back({CX, M, CX, B-0.2f}); 
                     segs.push_back({CX, B-0.1f, CX, B}); break;
            
            // BLOCK CURSOR (0x2588) - Simulated with Hatching/Scribble
            // BLOCK CURSOR (0x2588 maps to 219 in CP437)
            case 219: // Full Block (CP437)
            case 0x258B: // Left Heavy
            case 0x258C: // Left Half
                     // Draw a tight hatch/spiral to look "solid"
                     segs.push_back({L, T, R, T}); segs.push_back({R, T, R, B});
                     segs.push_back({R, B, L, B}); segs.push_back({L, B, L, T});
                     // Hatching
                     segs.push_back({L, T, R, M}); segs.push_back({L, M, R, B});
                     segs.push_back({R, T, L, M}); segs.push_back({R, M, L, B});
                     segs.push_back({L, T, R, B}); segs.push_back({R, T, L, B});
                     break;

            default:
                break;
        }
        return segs;
    }
    
    const char* name() const override { return "Modern Term"; }
};
