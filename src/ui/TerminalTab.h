#pragma once

#include <QWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <QVBoxLayout>
#include <QList>
#include <QShowEvent>
#include <QHideEvent>
#include "../renderer/TerminalWidget.h"

class TerminalTab : public QWidget
{
    Q_OBJECT

public:
    explicit TerminalTab(QWidget* parent = nullptr);
    ~TerminalTab();

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

public:
    void splitHorizontal(); // Side-by-side
    void splitVertical();   // Top-bottom
    
    // Connection helpers
    void connectToHost(const QString& host, int port, const QString& user, const QString& password, const QString& keyPath, const QList<SshClient::PortForwardRule>& rules = {});
    bool isConnected() const;
    
    TerminalWidget* activeTerminal() const;

    // Advanced features
    void navigateRequest(int dx, int dy);
    void setBroadcastInput(bool enabled);
    bool isBroadcastInput() const { return m_broadcastInput; }
    
    void handleInputBroadcast(const QByteArray& data);

    // Graphics Settings Delegation
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
    
    // Getters (from active)
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

private:
    void setupInitialTerminal();
    void split(Qt::Orientation orientation);
    void setActiveTerminal(TerminalWidget* terminal);
    
    // Recursively find the splitter containing the widget
    QSplitter* findParentSplitter(QWidget* widget);

    QVBoxLayout* m_layout;
    TerminalWidget* m_activeTerminal = nullptr;
    QList<TerminalWidget*> m_terminals;
    bool m_broadcastInput = false;
};

