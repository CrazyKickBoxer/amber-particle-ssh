#include "GraphicsSettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>

GraphicsSettingsDialog::GraphicsSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Graphics Settings");
    resize(300, 400);
    setupUi();
}

void GraphicsSettingsDialog::setValues(float glow, float opacity, float brightness, float springK, float drag, float shimmerSpeed, int density, int style, int theme, float vibrance, int font)
{
    // Block signals to avoid feedback loops during init
    bool oldState = blockSignals(true);
    
    m_glowSlider->setValue(int(glow * 100)); // 0.0 - 5.0 -> 0 - 500
    m_opacitySlider->setValue(int(opacity * 100)); // 0.0 - 1.0 -> 0 - 100
    m_brightnessSlider->setValue(int(brightness * 100)); // 0.0 - 5.0 -> 0 - 500
    m_brightnessSlider->setValue(int(brightness * 100)); // 0.0 - 5.0 -> 0 - 500
    m_springSlider->setValue(int(springK * 10)); // 0.0 - 50.0 -> 0 - 500
    m_dragSlider->setValue(int(drag * 100)); // 0.0 - 1.0 -> 0 - 100
    m_shimmerSlider->setValue(int(shimmerSpeed * 10)); // 0.0 - 10.0 -> 0 - 100
    m_densitySlider->setValue(density); 
    if (m_styleCombo) m_styleCombo->setCurrentIndex(style); 
    if (m_themeCombo) m_themeCombo->setCurrentIndex(theme); 
    
    // Update labels
    m_glowLabel->setText(QString::number(glow, 'f', 2));
    m_opacityLabel->setText(QString::number(opacity, 'f', 2));
    m_brightnessLabel->setText(QString::number(brightness, 'f', 2));
    m_springLabel->setText(QString::number(springK, 'f', 1));
    m_dragLabel->setText(QString::number(drag, 'f', 2));
    m_shimmerLabel->setText(QString::number(shimmerSpeed, 'f', 1));
    m_vibranceLabel->setText(QString::number(vibrance, 'f', 2));
    m_densityLabel->setText(QString::number(density));

    blockSignals(oldState);
}

QSlider* GraphicsSettingsDialog::createSlider(const QString& labelText, int min, int max, int val, const QString& tooltip)
{
    // Need access to layout? Layout is managed in setupUi.
    // Helper just creates the slider, caller adds to layout? 
    // Wait, I need label pointer too.
    return nullptr; 
}

void GraphicsSettingsDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    QGroupBox* visualsGroup = new QGroupBox("Visuals", this);
    QVBoxLayout* visualLayout = new QVBoxLayout(visualsGroup);
    
    // Helper lambda to create row
    auto addRow = [&](const QString& name, int min, int max, QSlider*& slider, QLabel*& valueLabel, const QString& tooltip) {
        QHBoxLayout* row = new QHBoxLayout();
        row->addWidget(new QLabel(name));
        
        slider = new QSlider(Qt::Horizontal);
        slider->setRange(min, max);
        slider->setToolTip(tooltip);
        row->addWidget(slider);
        
        valueLabel = new QLabel("0");
        valueLabel->setFixedWidth(30);
        row->addWidget(valueLabel);
        
        visualLayout->addLayout(row);
    };

    addRow("Glow Intensity", 0, 500, m_glowSlider, m_glowLabel, "Bloom strength (0.0 - 5.0)");
    addRow("Pixel Brightness", 0, 500, m_brightnessSlider, m_brightnessLabel, "Overall brightness multiplier (0.0 - 5.0)");
    addRow("Color Vibrance", 0, 300, m_vibranceSlider, m_vibranceLabel, "Color saturation boost (0.0 - 3.0), 1.0 is default");
    addRow("Background Opacity", 0, 100, m_opacitySlider, m_opacityLabel, "Transparency of the terminal background (0.0 - 1.0)");
    addRow("Particle Density", 1, 50, m_densitySlider, m_densityLabel, "Particles per pixel. Warning: >20 requires strong GPU.");

    mainLayout->addWidget(visualsGroup);

    QGroupBox* physicsGroup = new QGroupBox("Physics & Animation", this);
    QVBoxLayout* physicsLayout = new QVBoxLayout(physicsGroup);

    // Reuse helper (different layout)
    auto addRowPhys = [&](const QString& name, int min, int max, QSlider*& slider, QLabel*& valueLabel, const QString& tooltip) {
        QHBoxLayout* row = new QHBoxLayout();
        row->addWidget(new QLabel(name));
        
        slider = new QSlider(Qt::Horizontal);
        slider->setRange(min, max);
        slider->setToolTip(tooltip);
        row->addWidget(slider);
        
        valueLabel = new QLabel("0");
        valueLabel->setFixedWidth(30);
        row->addWidget(valueLabel);
        
        physicsLayout->addLayout(row);
    };
    
    // Physics Group
    // User requested "adjustment for how fast the letters draw on the screen while typing"
    // "Left to cursor speed". This is Spring Strength.
    addRowPhys("Typing Speed", 1, 2000, m_springSlider, m_springLabel, "Speed of the fly-in animation (Spring Stiffness). Higher is faster.");
    addRowPhys("Drag / Damping", 1, 99, m_dragSlider, m_dragLabel, "Air resistance (0.01 - 0.99). Higher stops faster.");
    addRowPhys("Shimmer Speed", 0, 500, m_shimmerSlider, m_shimmerLabel, "Speed of the idle particle orbit.");
    
    // Theme Selection
    QHBoxLayout* themeRow = new QHBoxLayout();
    themeRow->addWidget(new QLabel("Visual Theme"));
    m_themeCombo = new QComboBox();
    m_themeCombo->addItem("Cyberpunk Neon"); // 0
    m_themeCombo->addItem("Retro Scanlines"); // 1
    m_themeCombo->addItem("Synthwave Sunset"); // 2
    themeRow->addWidget(m_themeCombo);
    physicsLayout->addLayout(themeRow);

    // Motion Style
    QHBoxLayout* styleRow = new QHBoxLayout();
    styleRow->addWidget(new QLabel("Motion Style"));
    m_styleCombo = new QComboBox();
    m_styleCombo->addItem("Direct (Normal)");
    m_styleCombo->addItem("Nebula Twist");
    m_styleCombo->addItem("Digital Rain"); 
    m_styleCombo->addItem("Quantum Flux"); 
    m_styleCombo->addItem("Sonic Boom");
    m_styleCombo->addItem("Magnetic Assemble");
    m_styleCombo->setToolTip("Select the animation style for particle movement.");
    m_styleCombo->setToolTip("Select the animation style for particle movement.");
    styleRow->addWidget(m_styleCombo);
    physicsLayout->addLayout(styleRow);

    // Font Selection
    QHBoxLayout* fontRow = new QHBoxLayout();
    fontRow->addWidget(new QLabel("Font Style"));
    m_fontCombo = new QComboBox();
    m_fontCombo->addItem("Classic 8x8");       // 0
    m_fontCombo->addItem("High-Res 16x16");    // 1
    m_fontCombo->addItem("Segmented Vector (Abstract)", 2);
    m_fontCombo->addItem("Tech Vector (Futuristic)", 3);
    m_fontCombo->addItem("Modern Term (Readable)", 4);
    m_fontCombo->addItem("Code Pro (Serious)", 5);
    m_fontCombo->addItem("CRT Retro (Classic)", 6); // 3
    fontRow->addWidget(m_fontCombo);
    physicsLayout->addLayout(fontRow);
    
    mainLayout->addWidget(physicsGroup);
    mainLayout->addStretch();
    
    // CONNECTIONS
    connect(m_glowSlider, &QSlider::valueChanged, this, [=](int v){
        float val = v / 100.0f;
        m_glowLabel->setText(QString::number(val, 'f', 2));
        emit glowIntensityChanged(val);
    });

    connect(m_brightnessSlider, &QSlider::valueChanged, this, [=](int v){
        float val = v / 100.0f;
        m_brightnessLabel->setText(QString::number(val, 'f', 2));
        emit brightnessChanged(val);
    });
    
    connect(m_vibranceSlider, &QSlider::valueChanged, this, [=](int v){
        float val = v / 100.0f;
        m_vibranceLabel->setText(QString::number(val, 'f', 2));
        emit vibranceChanged(val);
    });
    
    connect(m_opacitySlider, &QSlider::valueChanged, this, [=](int v){
        float val = v / 100.0f;
        m_opacityLabel->setText(QString::number(val, 'f', 2));
        emit opacityChanged(val);
    });
    
    connect(m_densitySlider, &QSlider::valueChanged, this, [=](int v){
        m_densityLabel->setText(QString::number(v));
        emit densityChanged(v);
    });
    
    connect(m_springSlider, &QSlider::valueChanged, this, [=](int v){
        float val = v / 10.0f;
        m_springLabel->setText(QString::number(val, 'f', 1));
        emit springKChanged(val);
    });
    
    connect(m_dragSlider, &QSlider::valueChanged, this, [=](int v){
        float val = v / 100.0f;
        m_dragLabel->setText(QString::number(val, 'f', 2));
        emit dragChanged(val);
    });
    
    connect(m_shimmerSlider, &QSlider::valueChanged, this, [=](int v){
        float val = v / 10.0f;
        m_shimmerLabel->setText(QString::number(val, 'f', 1));
        emit shimmerSpeedChanged(val);
    });

    connect(m_styleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){
        emit animationStyleChanged(index);
    });
    
    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){
        emit themeChanged(index);
    });

    connect(m_fontCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){
        emit fontChanged(index);
    });
}
