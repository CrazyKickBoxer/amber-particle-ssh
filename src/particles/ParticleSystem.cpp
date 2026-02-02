#include "ParticleSystem.h"
#include "../renderer/ShaderManager.h"
#include "../fonts/ClassicFont.h"
#include "../fonts/HighResFont.h"
#include "../fonts/SegmentedFont.h"
#include "FontData.h" // Keep for fallback if needed
#include "../terminal/TerminalModel.h"
#include <QRandomGenerator>
#include <QDebug>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


// GTX 1080 Ti Optimization: Hard cap
static const int MAX_PARTICLES = 8000000; // Increased from 2M to support Density 50

ParticleSystem::ParticleSystem()
    : m_particleCount(0)
    , m_maxParticles(MAX_PARTICLES)
    , m_vao(0)
    , m_width(100.0f)
    , m_height(100.0f)
{
}

ParticleSystem::~ParticleSystem()
{
    // Cleanup GL resources
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_posVbo);
    glDeleteBuffers(1, &m_velVbo);
    glDeleteBuffers(1, &m_targetVbo);
    glDeleteBuffers(1, &m_extraVbo);
    glDeleteBuffers(1, &m_colorVbo);
    glDeleteBuffers(1, &m_baseQuadVbo);
}

void ParticleSystem::init()
{
    initializeOpenGLFunctions();
    initShaders();
    initBuffers();
    
    // Seed some initial particles (will be overwritten by terminal update)
    // seedParticles(24000); 
    
    // Initialize Jitter Table (Optimization for high density)
    m_jitterTable.resize(8192); // 8K samples
    auto* gen = QRandomGenerator::global();
    for(size_t i=0; i<m_jitterTable.size(); ++i) {
        m_jitterTable[i] = (gen->generateDouble() - 0.5) * 2.0; // -1 to 1
    }
    
    // Force apply default theme to init color tints
    // Force apply default theme to init color tints
    setTheme(m_theme);
    
    // Default Font
    m_font = new ClassicFont(); 
    m_fontId = 0; // Classic
    qDebug() << "FONT INIT: Default to ClassicFont (8x8), ID:" << m_fontId;
}

void ParticleSystem::setFont(FontAsset* font)
{
    if (m_font && m_font != font) delete m_font;
    m_font = font;
    m_prevGrid.clear(); // Force rebuild
}

void ParticleSystem::setFontById(int id) {
    qDebug() << "FONT CHANGE REQUEST: ID" << id << "(current:" << m_fontId << ")";
    
    if (m_fontId == id && m_font) {
        qDebug() << "FONT CHANGE: Skipped (same ID)";
        return;
    }
    
    FontAsset* newFont = nullptr;
    switch(id) {
        case 1: 
            newFont = new HighResFont(); 
            qDebug() << "FONT CHANGE: Creating HighResFont (16x16)";
            break;
        case 2: 
            newFont = new SegmentedFont(); 
            qDebug() << "FONT CHANGE: Creating SegmentedFont (Vector)";
            break;
        case 3: 
            newFont = new TechVectorFont(); 
            qDebug() << "FONT CHANGE: Creating TechVectorFont (Futuristic)";
            break;
        case 4:
            newFont = new ModernTermFont();
            qDebug() << "FONT CHANGE: Creating ModernTermFont";
            break;
        case 5:
            newFont = new CodeProFont();
            qDebug() << "FONT CHANGE: Creating CodeProFont";
            break;
        case 6:
            newFont = new CrtRetroFont();
            qDebug() << "FONT CHANGE: Creating CrtRetroFont";
            break;
        case 0: 
        default: 
            newFont = new ClassicFont(); 
            qDebug() << "FONT CHANGE: Creating ClassicFont (8x8)";
            break;
    }
    setFont(newFont);
    m_fontId = id;
    qDebug() << "FONT CHANGE: Complete. New Font Type:" << (int)m_font->type() << "Size:" << m_font->width() << "x" << m_font->height();
}

void ParticleSystem::initShaders()
{
    m_renderProgram = ShaderManager::createProgram("Render", 
        "shaders/particle.vert", "shaders/particle.frag");
        
    m_computeProgram = ShaderManager::createComputeProgram("Compute", 
        "shaders/particle_compute.comp");
}

void ParticleSystem::initBuffers()
{
    // 1. VAO Setup
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // 2. Base Quad (4 vertices)
    float quadVertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f 
    };
    glGenBuffers(1, &m_baseQuadVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_baseQuadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    // Attribute 0: Pos (vec2)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    // 3. Instance Buffers
    
    glGenBuffers(1, &m_posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_posVbo);
    glBufferData(GL_ARRAY_BUFFER, m_maxParticles * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    
    // Attribute 1: Instance Pos (vec3) - we'll just take xyz from vec4
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glVertexAttribDivisor(1, 1); // Per instance

    // Velocities
    glGenBuffers(1, &m_velVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_velVbo);
    glBufferData(GL_ARRAY_BUFFER, m_maxParticles * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    
    // Target Positions
    glGenBuffers(1, &m_targetVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_targetVbo);
    glBufferData(GL_ARRAY_BUFFER, m_maxParticles * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    // Extra Data
    glGenBuffers(1, &m_extraVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_extraVbo);
    glBufferData(GL_ARRAY_BUFFER, m_maxParticles * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    
    // Attribute 4: Extra (vec4)
    glEnableVertexAttribArray(4);
    glBindBuffer(GL_ARRAY_BUFFER, m_extraVbo);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glVertexAttribDivisor(4, 1);

    // Colors
    glGenBuffers(1, &m_colorVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_colorVbo);
    glBufferData(GL_ARRAY_BUFFER, m_maxParticles * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    
    // Attribute 3: Color (vec4)
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, m_colorVbo);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glVertexAttribDivisor(3, 1);

    // Attribute 2: Size (float) - reuse w component of Pos
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, m_posVbo); 
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3*sizeof(float))); // Offset to w
    glVertexAttribDivisor(2, 1);

    glBindVertexArray(0);
}

void ParticleSystem::seedParticles(int count)
{
    m_particleCount = std::min(count, m_maxParticles);
    
    std::vector<float> positions(m_maxParticles * 4);
    std::vector<float> velocities(m_maxParticles * 4);
    std::vector<float> colors(m_maxParticles * 4);
    
    auto* gen = QRandomGenerator::global();
    
    for(int i=0; i<m_particleCount; ++i) {
        positions[i*4 + 0] = gen->bounded(m_width);
        positions[i*4 + 1] = gen->bounded(m_height);
        positions[i*4 + 2] = 0.0f; 
        positions[i*4 + 3] = 4.0f; 
        
        velocities[i*4 + 0] = (gen->generateDouble() - 0.5) * 10.0;
        velocities[i*4 + 1] = (gen->generateDouble() - 0.5) * 10.0;
        
        colors[i*4 + 0] = 1.0f; 
        colors[i*4 + 1] = 0.7f; 
        colors[i*4 + 2] = 0.0f; 
        colors[i*4 + 3] = 1.0f; 
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, m_posVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(float), positions.data());
    
    glBindBuffer(GL_ARRAY_BUFFER, m_velVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, velocities.size() * sizeof(float), velocities.data());
    
    glBindBuffer(GL_ARRAY_BUFFER, m_targetVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(float), positions.data());
    
    glBindBuffer(GL_ARRAY_BUFFER, m_colorVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, colors.size() * sizeof(float), colors.data());
}

void ParticleSystem::updateParticlesFromTerminal(const TerminalModel& model, int cursorX, int cursorY, bool cursorVisible,
                                                  int selStartCol, int selStartRow, int selEndCol, int selEndRow,
                                                  int linkRow, int linkStart, int linkEnd)
{
    // ---------------------------------------------------------
    // OPTIMIZED GRID UPDATE (Fly-In & Partial Updates)
    // ---------------------------------------------------------

    int cols = model.cols();
    int rows = model.rows();
    
    // Safety check for empty model
    if (cols == 0 || rows == 0) return;

    // Check if we need to full-rebuild (Resize or Init)
    int particlesPerPixel = m_density;
    
    // CRITICAL: Calculate pixels per cell based on ACTUAL font size
    // Bitmap fonts use their grid, Vector fonts use 64 (standardized to 8x8 equivalent)
    // BUG FIX: Vector fonts were running out of particles. Bump default to higher value (e.g. 256 = 16x16 equiv)
    int pixelsPerCell = 256; 
    if (m_font && m_font->type() == FontType::Bitmap) {
        pixelsPerCell = m_font->width() * m_font->height();
        if (pixelsPerCell < 64) pixelsPerCell = 64; // Min cap
    }

    
    int particlesPerCell = pixelsPerCell * particlesPerPixel;
    size_t totalParticles = (size_t)cols * rows * particlesPerCell;
    
    // Check if density changed or grid changed
    // IMPORTANT: Density change alters stride (particlesPerCell), so we MUST rebuild if it changes.
    bool fullRebuild = (cols != m_gridCols || rows != m_gridRows || 
                       m_density != m_gridDensity ||
                       totalParticles > m_posData.size()/4 || 
                       m_prevGrid.size() != (size_t)(cols * rows));

    if (fullRebuild) {
        qDebug() << "GRID RESIZE/INIT: " << cols << "x" << rows << " Particles:" << totalParticles << " Density:" << m_density;
        m_gridCols = cols;
        m_gridRows = rows;
        m_gridDensity = m_density;
        m_prevGrid.assign(cols * rows, 0xFFFFFFFF); // Force update all
        m_prevChars.assign(cols * rows, 0); // Reset chars
        
        size_t floatCount = totalParticles * 4;
        
        if (totalParticles > (size_t)m_maxParticles) {
            totalParticles = m_maxParticles;
            floatCount = totalParticles * 4;
        }

        m_posData.resize(floatCount);
        m_velData.resize(floatCount);
        m_targetData.resize(floatCount);
        m_colorData.resize(floatCount);
        m_extraData.resize(floatCount);
        
        m_particleCount = totalParticles;
        
        std::fill(m_posData.begin(), m_posData.end(), 0.0f);
        std::fill(m_velData.begin(), m_velData.end(), 0.0f);
        std::fill(m_targetData.begin(), m_targetData.end(), 0.0f);
        std::fill(m_colorData.begin(), m_colorData.end(), 0.0f);
        std::fill(m_extraData.begin(), m_extraData.end(), 0.0f);
        
        // Upload ZEROED buffers
        glBindBuffer(GL_ARRAY_BUFFER, m_posVbo);
        glBufferData(GL_ARRAY_BUFFER, m_maxParticles * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW); 
        glBufferSubData(GL_ARRAY_BUFFER, 0, floatCount * sizeof(float), m_posData.data());
    }
    
    auto mapUnicodeToCP437 = [](uint32_t u) -> uint8_t {
         if (u < 128) return (uint8_t)u;
         if (u == 0x00C7) return 128; if (u == 0x00FC) return 129; if (u == 0x00E9) return 130;
         if (u == 0x00E2) return 131; if (u == 0x00E4) return 132; if (u == 0x00E0) return 133;
         if (u == 0x00E5) return 134; if (u == 0x00E7) return 135; if (u == 0x00EA) return 136;
         if (u == 0x00EB) return 137; if (u == 0x00E8) return 138; if (u == 0x00EF) return 139;
         if (u == 0x00EE) return 140; if (u == 0x00EC) return 141; if (u == 0x00C4) return 142;
         if (u == 0x00C5) return 143; if (u == 0x00C9) return 144; if (u == 0x00E6) return 145;
         if (u == 0x00C6) return 146; if (u == 0x00F4) return 147; if (u == 0x00F6) return 148;
         if (u == 0x00F2) return 149; if (u == 0x00FB) return 150; if (u == 0x00F9) return 151;
         if (u == 0x00FF) return 152; if (u == 0x00D6) return 153; if (u == 0x00DC) return 154;
         if (u == 0x2588) return 219; 
         return 63; 
    };

    float charWidth = 10.0f; 
    float charHeight = 18.0f;
    if (cols > 0) charWidth = m_width / cols;
    if (rows > 0) charHeight = m_height / rows;

    auto* gen = QRandomGenerator::global();
    
    size_t minChangeIdx = 0xFFFFFFFF;
    size_t maxChangeIdx = 0;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const TerminalCell& cell = model.cell(c, r);
            uint32_t unicode = cell.ch;
            
            // CURSOR HANDLING:
            // If this is the cursor cell AND cursor is visible (blink state):
            // We override look to BLOCK (0x2588) and COLOR.
            // We must invalidate cache for cursor moving (gridIdx check logic).
            // BUT signature is part of detection.
            // We'll mix "isCursor" into signature.
            
            bool isCursor = (c == cursorX && r == cursorY && cursorVisible);
            
            // SELECTION DETECTION: Check if cell is within selection range
            bool isSelected = false;
            if (selStartCol >= 0 && selStartRow >= 0 && selEndCol >= 0 && selEndRow >= 0) {
                // Normalize selection (handle backwards drag)
                int sR1 = selStartRow, sC1 = selStartCol;
                int sR2 = selEndRow, sC2 = selEndCol;
                if (sR2 < sR1 || (sR2 == sR1 && sC2 < sC1)) {
                    std::swap(sR1, sR2);
                    std::swap(sC1, sC2);
                }
                
                // Check if (c, r) is within selection
                if (r > sR1 && r < sR2) {
                    isSelected = true;  // Full row in middle
                } else if (r == sR1 && r == sR2) {
                    isSelected = (c >= sC1 && c <= sC2);  // Single row selection
                } else if (r == sR1) {
                    isSelected = (c >= sC1);  // First row
                } else if (r == sR2) {
                    isSelected = (c <= sC2);  // Last row
                }
            }
            
            uint32_t signature = unicode ^ (cell.attr.fgColor << 8) ^ (cell.attr.bgColor << 16) ^ (cell.attr.bold ? 0x80000000 : 0);
            if (isCursor) signature ^= 0xFFFFFFFF; // Flip bits for cursor state
            if (isSelected) signature ^= 0x55555555; // Different flip for selection
            
            int gridIdx = r * cols + c;
            
            if (!fullRebuild && m_prevGrid[gridIdx] == signature) {
                continue; 
            }
            m_prevGrid[gridIdx] = signature;
            
            size_t baseIdx = (size_t)gridIdx * particlesPerCell;
            if (baseIdx >= (size_t)m_maxParticles) continue; 
            
            if (baseIdx < minChangeIdx) minChangeIdx = baseIdx;
            
            int fgIdx = cell.attr.fgColor;
            int bgIdx = cell.attr.bgColor;
            bool fgTC = cell.attr.fgTrueColor; 
            uint8_t fgR=cell.attr.fgR, fgG=cell.attr.fgG, fgB=cell.attr.fgB;
            bool bgTC = cell.attr.bgTrueColor;
            uint8_t bgR=cell.attr.bgR, bgG=cell.attr.bgG, bgB=cell.attr.bgB;
            
            bool inverse = cell.attr.inverse;
            
            // CURSOR OVERRIDE
            if (isCursor) {
                // Force Block
                unicode = 0x2588; 
                // Force Amber Color (or Inverse of underlying?)
                // User asked for "Solid Flashing Cursor".
                // Let's make it Solid Amber (bright).
                // Or if we want text VISIBLE under cursor, we inverse.
                // Standard terminal: Inverse. 
                // But "Solid" implies filled block.
                // If I set unicode=Block, I lose text. 
                // Unless I implement "Inverse" logic where block IS text.
                // My font renderer draws PIXELS.
                // If I draw Block, it draws 8x8 pixels. Text is lost.
                // A true block cursor obscures text unless using XOR (not available) or Inverse.
                // I'll stick to Block because user said "Solid".
                // Maybe they mean "Not hollow"?
                // Let's use Inverse logic if they want text visible.
                // But "Solid Flashing Block" suggests Block.
                // I will use Block. If they complain text is hidden, I'll switch to Inverse.
                // Actually, standard is inverse.
                // I'll try INVERSE first? 
                // "Solid flashing cursor... where the cursor is".
                // If I just invert, it's a solid block WITH text cut out.
                // If I set unicode=2588, it's a solid block NO text.
                // I'll use 2588 + Bright Amber.
                fgIdx = 7; // White/Amber
                fgTC = false;
            } else {
                 if (inverse) {
                    std::swap(fgIdx, bgIdx); std::swap(fgTC, bgTC);
                    std::swap(fgR, bgR); std::swap(fgG, bgG); std::swap(fgB, bgB);
                }

            }

            // TRACKING: Check if character changed (for animation trigger)
            // 'unicode' holds the final rendered char (including cursor block 0x2588)
            uint32_t prevChar = m_prevChars[gridIdx];
            bool charChanged = (prevChar != unicode);
            m_prevChars[gridIdx] = unicode;

            uint8_t fontCharIndex = mapUnicodeToCP437(unicode);
            if (unicode == 0) fontCharIndex = 32;

            bool isBlockChar = (fontCharIndex == 0x2588 || fontCharIndex == 219);

            // GHOST FIX: If it's a SPACE (32) and NO background color/inverse, 
            // we MUST hide particles immediately.
            // Otherwise, vector renderer might skip "empty" segments and leave old particles.
            bool isVisualSpace = (fontCharIndex == 32 || fontCharIndex == 0) && (bgIdx == 0 && !bgTC && !inverse);
            if (isVisualSpace) {
                 for (int i=0; i<particlesPerCell; ++i) {
                     size_t idx = baseIdx + i;
                     if (idx < (size_t)m_maxParticles) {
                         m_posData[idx*4+3] = 0.0f; // Hide
                         m_targetData[idx*4+0] = -10000.0f; 
                         m_velData[idx*4+0] = 0.0f;
                     }
                 }
                 continue; // Skip rendering
            }

            float startX = c * charWidth;
            float startY = r * charHeight;
            
            // FONT RENDER LOGIC
            int pIdx = 0;
            
            // Debug first char of first row
            if (c == 0 && r == 0) {
                qDebug() << "FONT RENDER: Type=" << (m_font ? (int)m_font->type() : -1) 
                         << "FontID=" << m_fontId
                         << "Size=" << (m_font ? m_font->width() : 0) << "x" << (m_font ? m_font->height() : 0)
                         << "CharIdx=" << fontCharIndex;
            }
            
            // Ensure gen is available (it is defined at line 287)
            // auto* gen = QRandomGenerator::global(); 
            
            if (m_font->type() == FontType::Bitmap) {
                // ... (Bitmap logic skipped) ...
                int fw = m_font->width();
                int fh = m_font->height();
                float pixelW = charWidth / (float)fw;
                float pixelH = charHeight / (float)fh;
                
                for (int cy = 0; cy < fh; ++cy) {
                    for (int cx = 0; cx < fw; ++cx) {
                        bool isTextPixel = m_font->getPixel(fontCharIndex, cx, cy);
                        
                        // --- REUSED COLOR LOGIC ---
                        int colorToUse = -1; 
                        bool useTrueColor = false;
                        float tcR=0, tcG=0, tcB=0;
                        
                        // Extract color calc to helper or duplicate for now
                        if (isTextPixel) {
                            if (fgTC) { useTrueColor = true; tcR=fgR/255.0f; tcG=fgG/255.0f; tcB=fgB/255.0f; colorToUse=999; }
                            else { colorToUse = fgIdx; }
                        } else {
                            if (bgTC) { useTrueColor = true; tcR=bgR/255.0f; tcG=bgG/255.0f; tcB=bgB/255.0f; colorToUse=999; }
                            else if (bgIdx != 0) { colorToUse = bgIdx; }
                        }

                        // Optimization: For solid background blocks, use fewer but larger particles
                        int activeDensity = m_density;
                        float sizeScale = 1.0f;
                        if (!isTextPixel && colorToUse != -1) {
                             activeDensity = std::max(1, m_density / 4);
                             sizeScale = 2.0f; 
                        }
                        
                        for (int i = 0; i < m_density; ++i) {
                             size_t currentIdx = baseIdx + pIdx;
                             pIdx++;
                             if (currentIdx >= (size_t)m_maxParticles) break;
                             if (currentIdx > maxChangeIdx) maxChangeIdx = currentIdx;

                             if (i >= activeDensity) { // Hide optimization
                                 m_posData[currentIdx*4 + 3] = 0.0f; m_targetData[currentIdx*4+0] = -10000.0f; continue;
                             }

                             float jx = m_jitterTable[(m_jitterIndex++) % 8192] * pixelW * 0.1f * 0.5f;
                             float jy = m_jitterTable[(m_jitterIndex++) % 8192] * pixelH * 0.1f * 0.5f;
                             float tx = startX + cx * pixelW + (pixelW * 0.5f) + jx;
                             float ty = startY + cy * pixelH + (pixelH * 0.5f) + jy;
                             
                             float size = 0.0f; 
                             if (colorToUse != -1) size = std::max(1.5f, pixelW * 0.65f) * sizeScale;

                             m_targetData[currentIdx*4 + 0] = tx;
                             m_targetData[currentIdx*4 + 1] = ty;
                             m_targetData[currentIdx*4 + 2] = 0.0f;
                             m_targetData[currentIdx*4 + 3] = size;

                             float rVal=0, gVal=0, bVal=0;
                             if (colorToUse != -1) {
                                 if (useTrueColor) { rVal=tcR; gVal=tcG; bVal=tcB; }
                                 else {
                                     // Default Palette Logic
                                     if (colorToUse==0 && isTextPixel) { 
                                         if (m_theme == THEME_CYBERPUNK) { rVal=1.0f; gVal=1.0f; bVal=1.0f; } 
                                         else { rVal=0.15f; gVal=0.15f; bVal=0.15f; }
                                     }
                                     else if (colorToUse == 1) { rVal=1.0f; gVal=0.2f; bVal=0.2f; }
                                     else if (colorToUse == 2) { rVal=0.2f; gVal=1.0f; bVal=0.2f; }
                                     else if (colorToUse == 4) { rVal=0.2f; gVal=0.4f; bVal=1.0f; }
                                     else { rVal=1.0f; gVal=0.59f; bVal=0.04f; } 
                                 }
                             }
                             
                             bool isUnderCursor = (c == cursorX && r == cursorY && cursorVisible);
                             bool shouldInvert = isSelected || isUnderCursor;
                             if (shouldInvert) { rVal=1.0f-rVal; gVal=1.0f-gVal; bVal=1.0f-bVal; }
                             
                             bool isLink = (r == linkRow && c >= linkStart && c <= linkEnd);
                             if (isLink && isTextPixel) { gVal=1.0f; bVal=1.0f; rVal=0.0f; } 

                             m_colorData[currentIdx*4 + 0] = rVal;
                             m_colorData[currentIdx*4 + 1] = gVal;
                             m_colorData[currentIdx*4 + 2] = bVal;
                             m_colorData[currentIdx*4 + 3] = 1.0f; // Alpha
                             
                             // CRITICAL FIX: Set m_posData (size AND position) for visibility
                             m_posData[currentIdx*4 + 0] = tx;
                             m_posData[currentIdx*4 + 1] = ty;
                             m_posData[currentIdx*4 + 2] = 0.0f;
                             m_posData[currentIdx*4 + 3] = size;
                             
                             // Extra Data (Pulse seed)
                             // Block Chars use Bitmap path but should NOT shimmer (use 0.0f)
                             // Only actual Bitmap Text (Classic Font) should shimmer (1.0f)
                             m_extraData[currentIdx*4 + 0] = (isTextPixel && !isBlockChar) ? 1.0f : 0.0f;
                             
                             // HANDLE STARTUP ANIMATION (Simple Jitter for Update)
                             if (charChanged && size > 0.0f) {
                                  if (m_animationStyle == 2) {
                                      m_posData[currentIdx*4 + 1] -= (200.0f + gen->generateDouble() * 200.0f);
                                  } else {
                                      float ang = gen->generateDouble() * 6.28f;
                                      float dst = 100.0f;
                                      m_posData[currentIdx*4 + 0] += cos(ang)*dst;
                                      m_posData[currentIdx*4 + 1] += sin(ang)*dst;
                                  }
                             }
                        }
                    }
                }
            }

            else if (m_font->type() == FontType::Vector) {
                    std::vector<VectorSegment> segments;
                    if (m_font) segments = m_font->getSegments(unicode);
                    
                    // SPACE HANDLING: If Space (32) has a background color (or is inverse),
                    // treat it as a BLOCK CHAR (0x2588) so visual scanning fills it.
                    if (fontCharIndex == 32 && (bgIdx != 0 || bgTC || inverse)) {
                        fontCharIndex = 0x2588; 
                        // If it was just inverse space, we need to ensure the "Block" logic sees it as "Fill".
                        // Logic below checks for 0x2588.
                    }

                    // ZERO-TOLERANCE CLEANUP:
                    // If the character changed, we MUST clear all particles for this cell first.
                    // This prevents "Ghost Particles" from previous letters persisting if the new letter has fewer scan points.
                    // The "Cleanup Loop" at the end catches *unused* ones, but this is a safety nuke.
                    if (charChanged) {
                         for (int i=0; i<particlesPerCell; ++i) {
                             size_t idx = baseIdx + i;
                             if (idx < (size_t)m_maxParticles) {
                                 m_posData[idx*4+3] = 0.0f; // Hide
                                 m_targetData[idx*4+0] = -10000.0f; // Banished
                                 m_velData[idx*4+0] = 0.0f; // Stop
                                 m_extraData[idx*4+0] = 0.0f; // Reset Phase
                                 m_extraData[idx*4+1] = 0.0f;
                                 m_extraData[idx*4+2] = 0.0f;
                                 m_extraData[idx*4+3] = 0.0f;
                             }
                         }
                    }

                    // UNIFIED RASTER-SCAN APPROACH (On-the-fly Rasterization)
                    // 1. Scan virtual grid (12x18)
                    // 2. Check Point-to-Segment (FG)
                    // 3. Fallback to Background (BG)
                    
                    int gridW = 12; 
                    int gridH = 18;
                    
                    for (int gy = 0; gy < gridH; ++gy) {
                        for (int gx = 0; gx < gridW; ++gx) {
                            float nx = (gx + 0.5f) / (float)gridW; 
                            float ny = (gy + 0.5f) / (float)gridH; 
                            
                            bool isFg = false;
                            
                            // 1. Block Character Override
                            if (fontCharIndex == 0x2588 || fontCharIndex == 219) {
                                isFg = true;
                            } 
                            // 2. Vector Segment Check
                            else if (!segments.empty()) {
                                for (const auto& seg : segments) {
                                    float l2 = pow(seg.x2 - seg.x1, 2) + pow(seg.y2 - seg.y1, 2);
                                    if (l2 == 0) { 
                                        float d = pow(nx - seg.x1, 2) + pow(ny - seg.y1, 2);
                                        if (d < 0.005f) { isFg = true; break; }
                                    } else {
                                        float t = ((nx - seg.x1) * (seg.x2 - seg.x1) + (ny - seg.y1) * (seg.y2 - seg.y1)) / l2;
                                        t = std::max(0.0f, std::min(1.0f, t));
                                        float px = seg.x1 + t * (seg.x2 - seg.x1);
                                        float py = seg.y1 + t * (seg.y2 - seg.y1);
                                        float dist = pow(nx - px, 2) + pow(ny - py, 2);
                                        if (dist < 0.008f) { isFg = true; break; } 
                                    }
                                }
                            }
                            
                            // 3. COLOR SELECTION
                            // If isFg -> Use Text Color
                            // Else If hasBackground -> Use Background Color (Fill)
                            // Else -> Skip
                            
                            int colorToUse = -1;
                            bool useTrueColor = false;
                            float tcR=0, tcG=0, tcB=0;
                            
                            bool hasBackground = (bgIdx != 0) || bgTC || (inverse && !isFg); 
                             
                            if (isFg) {
                                if (fgTC) { useTrueColor = true; tcR=fgR/255.0f; tcG=fgG/255.0f; tcB=fgB/255.0f; colorToUse=999; }
                                else { colorToUse = fgIdx; }
                            } else if ((bgIdx != 0 || bgTC) && fontCharIndex != 0x2588) { 
                                 if (bgTC) { useTrueColor = true; tcR=bgR/255.0f; tcG=bgG/255.0f; tcB=bgB/255.0f; colorToUse=999; }
                                 else { colorToUse = bgIdx; }
                            }
                            
                            if (colorToUse == -1) continue; // Scanline empty

                            // Spawn Particles
                            int activeDensity = m_density;
                             // Optimize solid backgrounds (non-text pixels)
                            if (!isFg) activeDensity = std::max(1, m_density / 2);

                            for (int i = 0; i < m_density; ++i) {
                                 size_t currentIdx = baseIdx + pIdx;
                                 pIdx++;
                                 if (currentIdx >= (size_t)m_maxParticles) break;
                                 if (currentIdx > maxChangeIdx) maxChangeIdx = currentIdx;
                                 
                                 if (i >= activeDensity) {
                                     m_posData[currentIdx*4 + 3] = 0.0f; 
                                     m_targetData[currentIdx*4+0] = -10000.0f; 
                                     continue;
                                 }

                                 float jx = m_jitterTable[(m_jitterIndex++) % 8192] * 0.005f;
                                 float jy = m_jitterTable[(m_jitterIndex++) % 8192] * 0.015f;
                                 
                                 float tx = startX + nx * charWidth + jx;
                                 float ty = startY + ny * charHeight + jy;
                                 
                                 float size = std::max(1.5f, charWidth/12.0f * 0.9f);
                                 
                                 m_targetData[currentIdx*4 + 0] = tx;
                                 m_targetData[currentIdx*4 + 1] = ty;
                                 m_targetData[currentIdx*4 + 2] = 0.0f;
                                 m_targetData[currentIdx*4 + 3] = size;

                                 float rVal=0, gVal=0, bVal=0;
                                 if (useTrueColor) { rVal=tcR; gVal=tcG; bVal=tcB; }
                                 else {
                                     // Palette lookup
                                     if (colorToUse == 7) { rVal=1.0f; gVal=0.7f; bVal=0.0f; } // Amber/White
                                     else if (colorToUse == 0) { rVal=0.1f; gVal=0.1f; bVal=0.1f; } // Black
                                     else if (colorToUse == 1) { rVal=1.0f; gVal=0.2f; bVal=0.2f; } // Red
                                     else if (colorToUse == 2) { rVal=0.2f; gVal=1.0f; bVal=0.2f; } // Green
                                     else if (colorToUse == 4) { rVal=0.2f; gVal=0.4f; bVal=1.0f; } // Blue
                                     else { rVal=0.8f; gVal=0.8f; bVal=0.8f; } // Default
                                 }
                                 
                                 m_colorData[currentIdx*4 + 0] = rVal;
                                 m_colorData[currentIdx*4 + 1] = gVal;
                                 m_colorData[currentIdx*4 + 2] = bVal;
                                 m_colorData[currentIdx*4 + 3] = 1.0f;

                                 // Immediate update for visibility
                                 m_posData[currentIdx*4 + 0] = tx;
                                 m_posData[currentIdx*4 + 1] = ty;
                                 m_posData[currentIdx*4 + 2] = 0.0f;
                                 m_posData[currentIdx*4 + 3] = size;
                                 
                                 // SMART STABILITY: 
                                 // Only animate if char changed. Static text = 0.0 shimmer.
                                 m_extraData[currentIdx*4 + 0] = (isFg && charChanged) ? 1.0f : 0.0f; 
                                 
                                 if (charChanged && size > 0.0f) {
                                      m_posData[currentIdx*4 + 2] += 50.0f;
                                 }
                            }
                    }}
                    
                    // CLEANUP: Hide unused particles (CRITICAL STEP - Was missing!)
                    while (pIdx < particlesPerCell) {
                         size_t currentIdx = baseIdx + pIdx;
                         pIdx++;
                         if (currentIdx >= (size_t)m_maxParticles) break;
                         m_posData[currentIdx*4 + 3] = 0.0f; 
                         m_targetData[currentIdx*4 + 0] = -10000.0f;
                         m_velData[currentIdx*4 + 0] = 0.0f; // Reset vel
                    }
            }

            // Garbage lines removed


        }
    }

    if (minChangeIdx <= maxChangeIdx) {
        size_t startByte = minChangeIdx * 4 * sizeof(float);
        size_t count = (maxChangeIdx - minChangeIdx + 1);
        size_t sizeBytes = count * 4 * sizeof(float);
        
        size_t totalBytes = m_posData.size() * sizeof(float);
        if (startByte + sizeBytes > totalBytes) {
            sizeBytes = totalBytes - startByte;
        }

        if (sizeBytes > 0) {
            glBindBuffer(GL_ARRAY_BUFFER, m_posVbo);
            glBufferSubData(GL_ARRAY_BUFFER, startByte, sizeBytes, &m_posData[minChangeIdx*4]);
            
            glBindBuffer(GL_ARRAY_BUFFER, m_velVbo);
            glBufferSubData(GL_ARRAY_BUFFER, startByte, sizeBytes, &m_velData[minChangeIdx*4]);
            
            glBindBuffer(GL_ARRAY_BUFFER, m_targetVbo);
            glBufferSubData(GL_ARRAY_BUFFER, startByte, sizeBytes, &m_targetData[minChangeIdx*4]);
            
            glBindBuffer(GL_ARRAY_BUFFER, m_colorVbo);
            glBufferSubData(GL_ARRAY_BUFFER, startByte, sizeBytes, &m_colorData[minChangeIdx*4]);
        }
    }
}

void ParticleSystem::resize(int width, int height)
{
    m_width = width;
    m_height = height;
}

void ParticleSystem::update(float dt)
{
    if (!m_computeProgram) {
        static bool warned = false;
        if (!warned) { qDebug() << "ERROR: No compute program!"; warned = true; }
        return;
    }
    
    m_elapsedTime += dt;
    
    // Debug: print every second
    static float debugTimer = 0;
    debugTimer += dt;
    if (debugTimer > 1.0f) {
        // qDebug() << "Compute running, elapsedTime:" << m_elapsedTime << "particles:" << m_particleCount;
        debugTimer = 0;
    }

    m_computeProgram->bind();
    m_computeProgram->setUniformValue("deltaTime", dt);
    m_computeProgram->setUniformValue("elapsedTime", m_elapsedTime);
    m_computeProgram->setUniformValue("bounds", QVector2D(m_width, m_height));
    
    // Pass adjustable physics params
    m_computeProgram->setUniformValue("uSpringK", m_springK);
    m_computeProgram->setUniformValue("uDrag", m_drag);
    m_computeProgram->setUniformValue("uShimmerBase", m_shimmerSpeed);
    m_computeProgram->setUniformValue("uStyle", m_animationStyle);
    m_computeProgram->setUniformValue("uShockwave", QVector3D(m_shockX, m_shockY, m_shockTime));
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_posVbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_velVbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_targetVbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_extraVbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_colorVbo);
    
    int groups = (m_particleCount + 255) / 256;
    glDispatchCompute(groups, 1, 1);
    
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

void ParticleSystem::setZoomLevel(float zoom)
{
    m_zoomLevel = zoom;
    
    // Increased Density for Brightness (was 8/6/4/3/2)
    if (zoom > 1.8) m_density = 16;       
    else if (zoom > 1.3) m_density = 12;  
    else if (zoom > 0.8) m_density = 8;   // Standard (was 4)
    else if (zoom > 0.5) m_density = 6;   
    else m_density = 4;                   
}

void ParticleSystem::adjustQuality(float currentFps, float targetFps)
{
    if (currentFps < targetFps * 0.9f) { 
        m_glowIntensity *= 0.9f;
        if (m_glowIntensity < 0.5f) m_glowIntensity = 0.5f;
    } else if (currentFps > targetFps * 1.1f) { 
        m_glowIntensity = std::min(2.0f, m_glowIntensity * 1.05f);
    }
}

void ParticleSystem::triggerShockwave(float x, float y) {
    m_shockX = x;
    m_shockY = y;
    m_shockTime = m_elapsedTime;
}

void ParticleSystem::setTheme(int theme) {
    m_theme = theme;
    
    // Apply Presets
    if (theme == THEME_CYBERPUNK) {
        m_scanlineIntensity = 0.05f;
        m_colorTint = QVector3D(1.5f, 0.45f, 0.05f); // DEEP Orange
    }
    else if (theme == THEME_RETRO) {
        m_scanlineIntensity = 0.50f; // Strong scanlines
        m_colorTint = QVector3D(0.8f, 1.0f, 0.8f); // Greenish tint
    }
    else if (theme == THEME_SYNTHWAVE) {
        m_scanlineIntensity = 0.15f; 
        m_colorTint = QVector3D(1.0f, 1.0f, 1.0f); // Gradient handled in shader
    }
}

void ParticleSystem::render(const QMatrix4x4& projection)
{
    if (!m_renderProgram) return;

    m_renderProgram->bind();
    
    QMatrix4x4 finalProj = projection;
    finalProj.translate(m_width/2, m_height/2);
    finalProj.scale(m_zoomLevel);
    finalProj.translate(-m_width/2, -m_height/2);
    
    m_renderProgram->setUniformValue("projection", finalProj);
    m_renderProgram->setUniformValue("glowIntensity", m_glowIntensity);
    m_renderProgram->setUniformValue("uBrightness", m_brightness);
    m_renderProgram->setUniformValue("uVibrance", m_vibrance); // FIXED
    m_renderProgram->setUniformValue("uShimmerSpeed", m_shimmerSpeed);
    m_renderProgram->setUniformValue("elapsedTime", m_elapsedTime); 
    
    // Theme Uniforms
    m_renderProgram->setUniformValue("uScanlineIntensity", m_scanlineIntensity);
    m_renderProgram->setUniformValue("uColorTint", m_colorTint); // vec3
    m_renderProgram->setUniformValue("uTheme", m_theme);
    m_renderProgram->setUniformValue("uResolution", QVector2D(m_width, m_height)); 
    
    glBindVertexArray(m_vao);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, m_particleCount);
    glBindVertexArray(0);
}
