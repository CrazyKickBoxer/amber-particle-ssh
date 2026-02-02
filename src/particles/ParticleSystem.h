#pragma once

#include <vector>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLShaderProgram>
#include <memory>
#include "../fonts/FontAsset.h"
#include "../fonts/ClassicFont.h"
#include "../fonts/HighResFont.h"
#include "../fonts/TechVectorFont.h"
#include "../fonts/ModernTermFont.h"
#include "../fonts/CodeProFont.h"
#include "../fonts/CrtRetroFont.h"
#include "../fonts/TechVectorFont.h" // NEW

// SoA Layout for strict cache coherency on CPU (if needed) and direct mapping to GPU buffers
class ParticleSystem : protected QOpenGLFunctions_4_5_Core
{
public:
    ParticleSystem();
    ~ParticleSystem();

    enum AnimationStyle {
        STYLE_NORMAL = 0,
        STYLE_TWIST = 1,
        STYLE_RAIN = 2,
        STYLE_QUANTUM = 3,
        STYLE_SONIC = 4,
        STYLE_MAGNETIC = 5
    };
    
    enum Theme {
        THEME_CYBERPUNK = 0,
        THEME_RETRO = 1,
        THEME_SYNTHWAVE = 2,
        THEME_CUSTOM = 99
    };
    
    void init();
    void resize(int width, int height);
    void update(float dt);
    void render(const QMatrix4x4& viewProjection);
    
    // Optimization Systems
    void setZoomLevel(float zoom);
    void adjustQuality(float currentFps, float targetFps);

    // Initial setup
    void seedParticles(int count);
    
    // Text Rendering
    void updateParticlesFromTerminal(const class TerminalModel& model, int cursorX = -1, int cursorY = -1, bool cursorVisible = false,
                                     int selStartCol = -1, int selStartRow = -1, int selEndCol = -1, int selEndRow = -1,
                                     int linkRow = -1, int linkStart = -1, int linkEnd = -1);
                                     
    void triggerShockwave(float x, float y);
    
    void setAudioLevel(float level) { m_audioLevel = level; } 
    float getAudioLevel() const { return m_audioLevel; }
                                     
    void setTheme(int theme); // 0=Cyberpunk, 1=Retro, 2=Synthwave
    void setScanlineIntensity(float val) { m_scanlineIntensity = val; }
    void setColorTint(QVector3D tint) { m_colorTint = tint; }

    void setGlowIntensity(float val) { m_glowIntensity = val; }
    void setBrightness(float val) { m_brightness = val; }
    void setVibrance(float val) { m_vibrance = val; } // NEW
    void setSpringK(float val) { m_springK = val; }
    void setDrag(float val) { m_drag = val; }
    void setShimmerSpeed(float val) { m_shimmerSpeed = val; }
    void setDensity(int val) { 
        if (m_density != val) {
            m_density = val; 
            m_prevGrid.clear(); // Force rebuild
            m_prevGrid.clear(); // Force rebuild
        }
    }
    
    void setFont(FontAsset* font); 
    void setFontById(int id); 
    int getFontId() const { return m_fontId; }
    
    // Getters for UI init
    float getGlowIntensity() const { return m_glowIntensity; }
    float getBrightness() const { return m_brightness; }
    float getVibrance() const { return m_vibrance; } // NEW
    float getSpringK() const { return m_springK; }
    float getDrag() const { return m_drag; }
    float getShimmerSpeed() const { return m_shimmerSpeed; }
    int getDensity() const { return m_density; }
    int getAnimationStyle() const { return m_animationStyle; }
    float getZoomLevel() const { return m_zoomLevel; }
    
    int getTheme() const { return m_theme; }
    float getScanlineIntensity() const { return m_scanlineIntensity; }
    QVector3D getColorTint() const { return m_colorTint; }

    void setAnimationStyle(int style) { m_animationStyle = style; }

private:
    void initBuffers();
    void initShaders();

    // GPU Buffers
    GLuint m_vao;
    GLuint m_posVbo;      // Current Position (vec4: x, y, z, w)
    GLuint m_velVbo;      // Velocity (vec4: vx, vy, vz, vw)
    GLuint m_targetVbo;   // Target Position (vec4: tx, ty, tz, padding)
    GLuint m_extraVbo;    // Extra: x=pulse, y=flicker, z=radius, w=unused - NEW
    GLuint m_colorVbo;    // Color (vec4: r, g, b, a)
    GLuint m_baseQuadVbo; // The single quad geometry

    // Shader Programs
    std::unique_ptr<QOpenGLShaderProgram> m_renderProgram;
    std::unique_ptr<QOpenGLShaderProgram> m_computeProgram;

    // Data
    int m_particleCount;
    int m_maxParticles;
    
    // Member vectors to avoid re-allocation
    std::vector<float> m_posData;
    std::vector<float> m_velData;
    std::vector<float> m_targetData;
    std::vector<float> m_extraData;
    std::vector<float> m_colorData;
    
    // Bounds
    float m_width;
    float m_height;
    
    // Optimization Settings
    float m_zoomLevel = 1.0f;
    int m_density = 8; // Increased default density for brightness
    int m_gridCols = 0;
    int m_gridRows = 0;
    int m_gridDensity = 0; // Track density used for allocation
    std::vector<uint32_t> m_prevGrid; // Store char codes to detect changes
    std::vector<uint32_t> m_prevChars; // Store actual character codes for animation triggers
    std::vector<float> m_jitterTable; // Optimization: Pre-computed random noise
    int m_jitterIndex = 0;
    
    // Visual Parameters
    float m_glowIntensity = 1.0f;
    float m_brightness = 1.0f; // New global multiplier
    float m_vibrance = 1.0f; // New saturation
    float m_springK = 80.0f; // Much faster (was 15.0)
    float m_drag = 0.90f;   // Higher damping (was 0.85)
    float m_shimmerSpeed = 4.0f; // Base speed
    
    float m_elapsedTime = 0.0f; // For wave animation timing
    int m_animationStyle = 0; // 0 = Normal
    
    // Shockwave Data
    float m_shockX = -1000.0f;
    float m_shockY = -1000.0f;
    float m_shockTime = -10.0f;
    
    // Theme Data
    int m_theme = 0;
    float m_scanlineIntensity = 0.05f; // Default subtle
    QVector3D m_colorTint = QVector3D(1.0f, 1.0f, 1.0f);
    
    // Audio
    float m_audioLevel = 0.0f; 
    
    // Font
    FontAsset* m_font = nullptr;
    int m_fontId = 0;
};
