#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLShaderProgram>
#include <QTimer>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QPoint>
#include "../terminal/SshClient.h"
#include "../terminal/TerminalModel.h"
#include "../ui/ConnectionDialog.h"

class ParticleSystem;

class TerminalWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core
{
    Q_OBJECT

public:
    TerminalWidget(QWidget* parent = nullptr);
    ~TerminalWidget();

    void showConnectionDialog();
    void connectToHost(const QString& host, int port, const QString& user, const QString& password, const QString& keyPath, const QList<SshClient::PortForwardRule>& rules = {});
    void sendData(const QByteArray& data);
    bool isConnected() const;
    void setRenderEnabled(bool enabled);
    
    // Graphics Settings
    void setGlowIntensity(float val);
    void setOpacity(float val);
    void setBrightness(float val);
    void setVibrance(float val); // NEW
    void setSpringK(float val);
    void setDrag(float val);
    void setShimmerSpeed(float val);
    void setDensity(int val);
    void setFont(int fontIndex); // NEW
    void setZoomLevel(float val);
    void setTheme(int theme);
    void setAnimationStyle(int style);
    
    float getGlowIntensity() const;
    float getOpacity() const;
    float getBrightness() const;
    float getVibrance() const; // NEW
    float getSpringK() const;
    float getDrag() const;
    float getShimmerSpeed() const;
    int getDensity() const;
    int getFont() const; // NEW
    float getZoomLevel() const;
    int getTheme() const;
    int getAnimationStyle() const;

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void paintEvent(QPaintEvent *event) override; // Overlay Minimap
    bool event(QEvent *event) override; // Capture Tab before focus navigation
    
    // Input
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

public:
signals:
    void focused();
    void inputData(const QByteArray& data);
    void navigateRequest(int dx, int dy); // -1, 0, 1 direction
    void splitRequest(Qt::Orientation orientation);
    void zoomRequest(float delta);
    
private:
    void updatePhysics();
    void renderParticles();

    QTimer* m_frameTimer;
    QElapsedTimer m_elapsedTimer;
    float m_deltaTime;
    int m_frameCount;
    
    ParticleSystem* m_particleSystem;
    class SshClient* m_sshClient;
    class TerminalModel* m_terminalModel;
    QWidget* m_parent;
    bool m_screenDirty = false;
    
    // Cursor Blinking
    QTimer* m_blinkTimer;
    bool m_cursorBlinkState = true;
    
    // Text Selection (PuTTY-like)
    bool m_selecting = false;
    QPoint m_selStart{-1, -1};  // Start cell (col, row)
    QPoint m_selEnd{-1, -1};    // End cell (col, row)
    
    // Smart Links
    struct LinkInfo {
        QString url;
        int row = -1;
        int startCol = -1;
        int endCol = -1;
        bool isValid() const { return row != -1; }
        void clear() { row = -1; url.clear(); }
    } m_hoveredLink;
    
    void detectLinkAt(QPoint pos);
    
    // Transparency
    float m_opacity = 0.85f; // Default semi-transparent
    
    QPoint pixelToCell(const QPointF& pos) const;
    void clearSelection();
    void copySelection();
    void pasteClipboard();
    QString getSelectedText() const;
    bool hasSelection() const;
};

