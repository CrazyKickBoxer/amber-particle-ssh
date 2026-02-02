#pragma once

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QSlider>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QComboBox>

class GraphicsSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GraphicsSettingsDialog(QWidget *parent = nullptr);

    // Initial values to sync UI
    void setValues(float glow, float opacity, float brightness, float springK, float drag, float shimmerSpeed, int density, int style, int theme, float vibrance, int font);

signals:
    void glowIntensityChanged(float val);
    void opacityChanged(float val);
    void brightnessChanged(float val);
    void springKChanged(float val);
    void dragChanged(float val);
    void shimmerSpeedChanged(float val);
    void densityChanged(int val);
    void animationStyleChanged(int style);
    void themeChanged(int theme);
    void fontChanged(int fontIndex); // NEW
    void vibranceChanged(float val); // NEW

private:
    void setupUi();

    QSlider* createSlider(const QString& label, int min, int max, int val, const QString& tooltip = "");
    
    QSlider* m_glowSlider;
    QSlider* m_opacitySlider;
    QSlider* m_brightnessSlider;
    QSlider* m_springSlider;
    QSlider* m_dragSlider;
    QSlider* m_shimmerSlider;
    QSlider* m_densitySlider;
    QSlider* m_vibranceSlider; // NEW
    QComboBox* m_styleCombo;
    QComboBox* m_fontCombo; // NEW
    QComboBox* m_themeCombo;
    
    QLabel* m_glowLabel;
    QLabel* m_opacityLabel;
    QLabel* m_brightnessLabel;
    QLabel* m_springLabel;
    QLabel* m_dragLabel;
    QLabel* m_shimmerLabel;
    QLabel* m_densityLabel;
    QLabel* m_vibranceLabel; // NEW
};
