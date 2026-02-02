#pragma once

#include "FontAsset.h"
#include <vector>

class CodeProFont : public FontAsset {
public:
    FontType type() const override { return FontType::Vector; }
    int width() const override { return 12; } // Wider for "Pro" feel
    int height() const override { return 18; }
    
    // Code Pro: "Serious Coder" font, Operator Mono / Fira Code vibes
    // Distinct features: Slashed zero, serif-I, hooked-l
    std::vector<VectorSegment> getSegments(uint32_t c) const override {
        std::vector<VectorSegment> segs;
        
        float L = 0.2f, R = 0.8f;
        float T = 0.15f, B = 0.85f;
        float M = 0.5f;
        float CX = 0.5f;
        
        char ch = (char)(c & 0xFF); // Keep ch for fallback logic ranges
        
        switch(c) {
            // --- TOP LEVEL HANDLERS (Priority) ---
            case 219: // Block Cursor (CP437 0xDB) - MOVED HERE TO ENSURE HIT
            case 0x2588: // Unicode Full Block
                 segs.push_back({L,T,R,T}); segs.push_back({R,T,R,B}); 
                 segs.push_back({R,B,L,B}); segs.push_back({L,B,L,T});
                 segs.push_back({L,T,R,B}); segs.push_back({R,T,L,B}); // Simple Cross
                 break;

            // --- Uppercase (Serif touches) ---
            case 'I': 
                segs.push_back({L, T, R, T}); // Top Serif
                segs.push_back({CX, T, CX, B}); 
                segs.push_back({L, B, R, B}); // Bot Serif
                break;
            case 'A':
                segs.push_back({L, B, CX, T}); segs.push_back({CX, T, R, B});
                segs.push_back({L+0.1f, M+0.1f, R-0.1f, M+0.1f}); 
                break; 
            // Reuse generic for most, augment key chars
            case '0': // Slashed Zero
                segs.push_back({L, T, R, T}); segs.push_back({R, T, R, B});
                segs.push_back({R, B, L, B}); segs.push_back({L, B, L, T});
                segs.push_back({R, T, L, B}); // Slash
                break;
            case 'O':
                segs.push_back({L, T, R, T}); segs.push_back({R, T, R, B});
                segs.push_back({R, B, L, B}); segs.push_back({L, B, L, T});
                break;
            case '7':
                segs.push_back({L, T, R, T}); segs.push_back({R, T, L, B});
                segs.push_back({0.4f, 0.5f, 0.6f, 0.5f}); // Crossbar on 7 for "Code" feel
                break;
            case 'Z':
                segs.push_back({L, T, R, T}); segs.push_back({R, T, L, B}); segs.push_back({L, B, R, B});
                segs.push_back({0.4f, 0.5f, 0.6f, 0.5f}); // Crossbar on Z
                break;
            case '1':
                segs.push_back({L, T+0.1f, CX, T}); segs.push_back({CX, T, CX, B});
                segs.push_back({L, B, R, B}); // Base on 1
                break;
            case 'l': // Hooked l
                segs.push_back({CX, T, CX, B});
                segs.push_back({CX, B, CX+0.2f, B}); // Hook right
                break;
            case 'i': // Serif i
                 segs.push_back({L+0.1f, M, R-0.1f, M}); // Top serif of body
                 segs.push_back({CX, M, CX, B});
                 segs.push_back({L+0.1f, B, R-0.1f, B}); // Bot serif
                 segs.push_back({CX, M-0.2f, CX, M-0.15f}); // Dot
                 break;
            case 'g': // Double story g
                 segs.push_back({CX+0.1f, M, CX-0.1f, M}); // Top loop top
                 segs.push_back({CX-0.1f, M, CX-0.1f, M+0.2f});
                 segs.push_back({CX-0.1f, M+0.2f, CX+0.1f, M+0.2f}); // Mid
                 segs.push_back({CX+0.1f, M+0.2f, CX+0.1f, M});
                 segs.push_back({CX+0.1f, M+0.1f, CX+0.1f, B+0.1f}); // Descender
                 segs.push_back({CX+0.1f, B+0.1f, CX-0.1f, B+0.1f}); // Loop
                 break;
            
            // Ligature hints (Visual only)
            case '=':
                 segs.push_back({L, M-0.1f, R, M-0.1f}); segs.push_back({L, M+0.1f, R, M+0.1f}); break;
            case '!':  segs.push_back({CX, T, CX, B-0.2f}); segs.push_back({CX, B-0.1f, CX, B}); break;
            case '>':  segs.push_back({L, T, R, M}); segs.push_back({R, M, L, B}); break;
            
            // --- ALL PUNCTUATION (TOP-LEVEL for guaranteed hit) ---
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
            case '?': segs.push_back({L, T, R, T}); segs.push_back({R, T, R, M}); segs.push_back({R, M, CX, M}); segs.push_back({CX, M, CX, B-0.2f}); segs.push_back({CX, B-0.1f, CX, B}); break;
            case '@': segs.push_back({R, B, L, B}); segs.push_back({L, B, L, T}); segs.push_back({L, T, R, T}); segs.push_back({R, T, R, M}); segs.push_back({R, M, CX, M}); segs.push_back({CX, M, CX, M+0.2f}); break;
            case '[': segs.push_back({R, T, L, T}); segs.push_back({L, T, L, B}); segs.push_back({L, B, R, B}); break;
            case '\\': segs.push_back({L, T, R, B}); break;
            case ']': segs.push_back({L, T, R, T}); segs.push_back({R, T, R, B}); segs.push_back({R, B, L, B}); break;
            case '^': segs.push_back({L, M, CX, T}); segs.push_back({CX, T, R, M}); break;
            case '_': segs.push_back({L, B, R, B}); break;
            case '`': segs.push_back({CX-0.1f, T, CX+0.1f, T+0.15f}); break;
            case '{': segs.push_back({R, T, CX, T}); segs.push_back({CX, T, CX, M}); segs.push_back({CX, M, L, M}); segs.push_back({CX, M, CX, B}); segs.push_back({CX, B, R, B}); break;
            case '|': segs.push_back({CX, T, CX, B}); break;
            case '}': segs.push_back({L, T, CX, T}); segs.push_back({CX, T, CX, M}); segs.push_back({CX, M, R, M}); segs.push_back({CX, M, CX, B}); segs.push_back({CX, B, L, B}); break;
            case '~': segs.push_back({L, M, CX, T+0.2f}); segs.push_back({CX, T+0.2f, R, M}); break;
            
            // Defaults for rest (Copy ModernTerm logic mostly, but widen)
            default:
            {
               // Fallback to minimal blocky logic for brevity in this task, 
               // but ensure readability.
               // Actually, for "Code Pro" to be usable, it shouldn't have missing chars.
               // I will duplicate the switch from ModernTerm but tweak style.
               // ... Or, to save token space/time, I can just implement the same switch
               // but with wider coords?
               
               // Let's implement the rest:
               if (ch >= 'A' && ch <= 'Z' && ch!='I' && ch!='A' && ch!='0' && ch!='O' && ch!='7' && ch!='Z') {
                   // Generic blocky caps
                    if(ch=='B'){ segs.push_back({L,T,L,B}); segs.push_back({L,T,R,T}); segs.push_back({R,T,R,M}); segs.push_back({R,M,L,M}); segs.push_back({R,M,R,B}); segs.push_back({R,B,L,B}); }
                    else if(ch=='C'){ segs.push_back({R,T,L,T}); segs.push_back({L,T,L,B}); segs.push_back({L,B,R,B}); }
                    else if(ch=='D'){ segs.push_back({L,T,L,B}); segs.push_back({L,T,R,M}); segs.push_back({R,M,L,B}); }
                    else if(ch=='E'){ segs.push_back({R,T,L,T}); segs.push_back({L,T,L,B}); segs.push_back({L,B,R,B}); segs.push_back({L,M,R,M}); }
                    else if(ch=='F'){ segs.push_back({R,T,L,T}); segs.push_back({L,T,L,B}); segs.push_back({L,M,R,M}); }
                    else if(ch=='G'){ segs.push_back({R,T,L,T}); segs.push_back({L,T,L,B}); segs.push_back({L,B,R,B}); segs.push_back({R,B,R,M}); segs.push_back({R,M,CX,M}); }
                    else if(ch=='H'){ segs.push_back({L,T,L,B}); segs.push_back({R,T,R,B}); segs.push_back({L,M,R,M}); }
                    else if(ch=='J'){ segs.push_back({R,T,R,B}); segs.push_back({R,B,L,B}); segs.push_back({L,B,L,M+0.2f}); }
                    else if(ch=='K'){ segs.push_back({L,T,L,B}); segs.push_back({R,T,L,M}); segs.push_back({L,M,R,B}); }
                    else if(ch=='L'){ segs.push_back({L,T,L,B}); segs.push_back({L,B,R,B}); }
                    else if(ch=='M'){ segs.push_back({L,B,L,T}); segs.push_back({L,T,CX,M}); segs.push_back({CX,M,R,T}); segs.push_back({R,T,R,B}); }
                    else if(ch=='N'){ segs.push_back({L,B,L,T}); segs.push_back({L,T,R,B}); segs.push_back({R,B,R,T}); }
                    else if(ch=='P'){ segs.push_back({L,B,L,T}); segs.push_back({L,T,R,T}); segs.push_back({R,T,R,M}); segs.push_back({R,M,L,M}); }
                    else if(ch=='Q'){ segs.push_back({L,T,R,T}); segs.push_back({R,T,R,B}); segs.push_back({R,B,L,B}); segs.push_back({L,B,L,T}); segs.push_back({CX,M,R,B}); }
                    else if(ch=='R'){ segs.push_back({L,B,L,T}); segs.push_back({L,T,R,T}); segs.push_back({R,T,R,M}); segs.push_back({R,M,L,M}); segs.push_back({L,M,R,B}); }
                    else if(ch=='S'){ segs.push_back({R,T,L,T}); segs.push_back({L,T,L,M}); segs.push_back({L,M,R,M}); segs.push_back({R,M,R,B}); segs.push_back({R,B,L,B}); }
                    else if(ch=='T'){ segs.push_back({L,T,R,T}); segs.push_back({CX,T,CX,B}); }
                    else if(ch=='U'){ segs.push_back({L,T,L,B}); segs.push_back({L,B,R,B}); segs.push_back({R,B,R,T}); }
                    else if(ch=='V'){ segs.push_back({L,T,CX,B}); segs.push_back({CX,B,R,T}); }
                    else if(ch=='W'){ segs.push_back({L,T,L,B}); segs.push_back({L,B,CX,M}); segs.push_back({CX,M,R,B}); segs.push_back({R,B,R,T}); }
                    else if(ch=='X'){ segs.push_back({L,T,R,B}); segs.push_back({R,T,L,B}); }
                    else if(ch=='Y'){ segs.push_back({L,T,CX,M}); segs.push_back({R,T,CX,M}); segs.push_back({CX,M,CX,B}); }
               }
                else if (ch >= 'a' && ch <= 'z' && ch!='l' && ch!='i' && ch != 'g') {
                   // Lowercase (Simple)
                    float lT = M;
                    if(ch=='a'){ segs.push_back({L,lT,R,lT}); segs.push_back({R,lT,R,B}); segs.push_back({R,B,L,B}); segs.push_back({L,B,L,M+0.1f}); segs.push_back({L,M+0.1f,R,M+0.1f}); }
                    else if(ch=='b'){ segs.push_back({L,T,L,B}); segs.push_back({L,lT,R,lT}); segs.push_back({R,lT,R,B}); segs.push_back({R,B,L,B}); }
                    else if(ch=='c'){ segs.push_back({R,lT,L,lT}); segs.push_back({L,lT,L,B}); segs.push_back({L,B,R,B}); }
                    else if(ch=='d'){ segs.push_back({R,T,R,B}); segs.push_back({R,lT,L,lT}); segs.push_back({L,lT,L,B}); segs.push_back({L,B,R,B}); }
                    else if(ch=='e'){ segs.push_back({L,M+0.2f,R,M+0.2f}); segs.push_back({R,M+0.2f,R,lT}); segs.push_back({R,lT,L,lT}); segs.push_back({L,lT,L,B}); segs.push_back({L,B,R,B}); }
                    else if(ch=='f'){ segs.push_back({CX,T,CX,B}); segs.push_back({L,lT,R,lT}); segs.push_back({CX,T,R,T}); }
                    else if(ch=='h'){ segs.push_back({L,T,L,B}); segs.push_back({L,lT,R,lT}); segs.push_back({R,lT,R,B}); }
                    else if(ch=='j'){ segs.push_back({CX,lT,CX,B+0.2f}); segs.push_back({CX,B+0.2f,L,B+0.2f}); segs.push_back({CX,lT-0.2f,CX,lT-0.1f}); }
                    else if(ch=='k'){ segs.push_back({L,T,L,B}); segs.push_back({R,lT,L,M+0.2f}); segs.push_back({L,M+0.2f,R,B}); }
                    else if(ch=='m'){ segs.push_back({L,lT,L,B}); segs.push_back({L,lT,CX,lT}); segs.push_back({CX,lT,CX,B}); segs.push_back({CX,lT,R,lT}); segs.push_back({R,lT,R,B}); }
                    else if(ch=='n'){ segs.push_back({L,lT,L,B}); segs.push_back({L,lT,R,lT}); segs.push_back({R,lT,R,B}); }
                    else if(ch=='o'){ segs.push_back({L,lT,R,lT}); segs.push_back({R,lT,R,B}); segs.push_back({R,B,L,B}); segs.push_back({L,B,L,lT}); }
                    else if(ch=='p'){ segs.push_back({L,lT,L,B+0.2f}); segs.push_back({L,lT,R,lT}); segs.push_back({R,lT,R,B}); segs.push_back({R,B,L,B}); }
                    else if(ch=='q'){ segs.push_back({R,lT,R,B+0.2f}); segs.push_back({R,lT,L,lT}); segs.push_back({L,lT,L,B}); segs.push_back({L,B,R,B}); }
                    else if(ch=='r'){ segs.push_back({L,lT,L,B}); segs.push_back({L,lT,R,lT}); }
                    else if(ch=='s'){ segs.push_back({R,lT,L,lT}); segs.push_back({L,lT,L,M+0.1f}); segs.push_back({L,M+0.1f,R,M+0.1f}); segs.push_back({R,M+0.1f,R,B}); segs.push_back({R,B,L,B}); }
                    else if(ch=='t'){ segs.push_back({L,lT,R,lT}); segs.push_back({CX,T,CX,B}); }
                    else if(ch=='u'){ segs.push_back({L,lT,L,B}); segs.push_back({L,B,R,B}); segs.push_back({R,B,R,lT}); }
                    else if(ch=='v'){ segs.push_back({L,lT,CX,B}); segs.push_back({CX,B,R,lT}); }
                    else if(ch=='w'){ segs.push_back({L,lT,L,B}); segs.push_back({L,B,CX,lT}); segs.push_back({CX,lT,R,B}); segs.push_back({R,B,R,lT}); }
                    else if(ch=='x'){ segs.push_back({L,lT,R,B}); segs.push_back({R,lT,L,B}); }
                    else if(ch=='y'){ segs.push_back({L,lT,R,B}); segs.push_back({R,lT,L,B+0.2f}); }
                    else if(ch=='z'){ segs.push_back({L,lT,R,lT}); segs.push_back({R,lT,L,B}); segs.push_back({L,B,R,B}); }
               }
               // Numbers (except 0, 1, 7)
               else if (ch >= '0' && ch <= '9' && ch!='0' && ch!='1' && ch!='7') {
                   // Copy from Modern Term largely
                   if(ch=='2'){ segs.push_back({L,T,R,T}); segs.push_back({R,T,R,M}); segs.push_back({R,M,L,B}); segs.push_back({L,B,R,B}); }
                   else if(ch=='3'){ segs.push_back({L,T,R,T}); segs.push_back({R,T,R,B}); segs.push_back({R,B,L,B}); segs.push_back({L,M,R,M}); }
                   else if(ch=='4'){ segs.push_back({R,T,R,B}); segs.push_back({R,M,L,M}); segs.push_back({L,M,L,T}); }
                   else if(ch=='5'){ segs.push_back({R,T,L,T}); segs.push_back({L,T,L,M}); segs.push_back({L,M,R,M}); segs.push_back({R,M,R,B}); segs.push_back({R,B,L,B}); }
                   else if(ch=='6'){ segs.push_back({R,T,L,T}); segs.push_back({L,T,L,B}); segs.push_back({L,B,R,B}); segs.push_back({R,B,R,M}); segs.push_back({R,M,L,M}); }
                   else if(ch=='8'){ segs.push_back({L,T,R,T}); segs.push_back({R,T,R,B}); segs.push_back({R,B,L,B}); segs.push_back({L,B,L,T}); segs.push_back({L,M,R,M}); }
                   else if(ch=='9'){ segs.push_back({L,B,R,B}); segs.push_back({R,B,R,T}); segs.push_back({R,T,L,T}); segs.push_back({L,T,L,M}); segs.push_back({L,M,R,M}); }
               }
               // Basic Symbols
                else {
                    switch(c) {
                       case '-': segs.push_back({L, M, R, M}); break;
                       case '_': segs.push_back({L, B, R, B}); break;
                       case ':': segs.push_back({CX, M-0.05f, CX, M+0.05f}); segs.push_back({CX, B-0.1f, CX, B}); break;
                       case '.': segs.push_back({CX, B-0.05f, CX, B}); break;
                       case ',': segs.push_back({CX, B-0.05f, CX-0.1f, B+0.1f}); break;
                       case '/': segs.push_back({L, B, R, T}); break;
                       case '\\': segs.push_back({L, T, R, B}); break;
                       case '[': segs.push_back({R, T, L, T}); segs.push_back({L, T, L, B}); segs.push_back({L, B, R, B}); break;
                       case ']': segs.push_back({L, T, R, T}); segs.push_back({R, T, R, B}); segs.push_back({R, B, L, B}); break;
                       case '{': segs.push_back({R, T, CX, T}); segs.push_back({CX, T, CX, M}); segs.push_back({CX, M, L, M}); segs.push_back({CX, M, CX, B}); segs.push_back({CX, B, R, B}); break;
                        case '}': segs.push_back({L, T, CX, T}); segs.push_back({CX, T, CX, M}); segs.push_back({CX, M, R, M}); segs.push_back({CX, M, CX, B}); segs.push_back({CX, B, L, B}); break;
                        case '?': 
                                 segs.push_back({L, T, R, T}); segs.push_back({R, T, R, M}); 
                                 segs.push_back({R, M, CX, M}); segs.push_back({CX, M, CX, B-0.2f}); 
                                 segs.push_back({CX, B-0.1f, CX, B}); break;
                              break;
                   }
               }
               
            }
            break;
        }
        return segs;
    }
    
    const char* name() const override { return "Code Pro"; }
};
