#include "TerminalWidget.h"
#include "../particles/ParticleSystem.h"
#include <QDebug>
#include <QMatrix4x4>
#include <QClipboard>
#include <QPainter>
#include <QGuiApplication> 
#include <QRegularExpression>
#include <QDesktopServices>
#include <QUrl>
#include <QtMath> 

TerminalWidget::TerminalWidget(QWidget* parent)
    : QOpenGLWidget(parent)
    , m_frameTimer(new QTimer(this))
    , m_frameCount(0)
    , m_particleSystem(new ParticleSystem())
    , m_sshClient(new SshClient(this))
    , m_terminalModel(new TerminalModel(80, 25, this))
    , m_parent(parent)
{
    // Connect SSH -> Terminal (Incoming Data)
    connect(m_sshClient, &SshClient::dataReceived, m_terminalModel, &TerminalModel::processInput);
    connect(m_sshClient, &SshClient::errorOccurred, m_terminalModel, &TerminalModel::showMessage);
    connect(m_sshClient, &SshClient::debugMessage, m_terminalModel, &TerminalModel::showMessage);

    // Connect Terminal -> SSH (Outgoing Data)
    connect(m_terminalModel, &TerminalModel::dataOutput, m_sshClient, &SshClient::sendData);

    // Connect Terminal -> Particles (Render Update)
    connect(m_terminalModel, &TerminalModel::screenChanged, this, [this]() {
        m_screenDirty = true;
    });
    
    // Connect poll timer
    connect(m_frameTimer, &QTimer::timeout, m_sshClient, &SshClient::poll);
    
    // 120 FPS target -> ~8.33ms
    m_frameTimer->setInterval(8); 
    connect(m_frameTimer, &QTimer::timeout, this, [this]() {
        update(); // Schedules paintGL
    });
    m_frameTimer->start();
    m_elapsedTimer.start();
    
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true); 
    
    // Blink Timer
    m_blinkTimer = new QTimer(this);
    connect(m_blinkTimer, &QTimer::timeout, this, [this](){
        m_cursorBlinkState = !m_cursorBlinkState;
        
        // ASYMMETRIC BLINK:
        // If ON: Stay ON for 1000ms (Solid)
        // If OFF: Stay OFF for 200ms (Explosion duration)
        if (m_cursorBlinkState) {
            m_blinkTimer->setInterval(1000); 
        } else {
            m_blinkTimer->setInterval(200);
        }

        m_screenDirty = true;
        update();
    });
    // Start in ON state, so ensure first timeout happens after 1000ms
    m_cursorBlinkState = true; 
    m_blinkTimer->start(1000); 
}

TerminalWidget::~TerminalWidget()
{
    makeCurrent();
    delete m_particleSystem;
    doneCurrent();
}

void TerminalWidget::showConnectionDialog()
{
    ConnectionDialog* dialog = new ConnectionDialog(this);
    connect(dialog, &ConnectionDialog::connectClicked, this, [this](QString h, int p, QString u, QString pwd, QString key){
        m_sshClient->connectToHost(h, p, u, pwd, key);
        int cols = m_terminalModel->cols();
        int rows = m_terminalModel->rows();
        m_sshClient->setPtySize(cols, rows);
    });
    dialog->show();
}

void TerminalWidget::connectToHost(const QString& h, int p, const QString& u, const QString& pwd, const QString& key, const QList<SshClient::PortForwardRule>& rules)
{
     m_sshClient->connectToHost(h, p, u, pwd, key);
     int cols = m_terminalModel->cols();
     int rows = m_terminalModel->rows();
     m_sshClient->setPtySize(cols, rows);
     
     for (const auto& rule : rules) {
         if (rule.type == SshClient::PortForwardRule::Local) {
             m_sshClient->addLocalForward(rule.bindPort, rule.targetHost, rule.targetPort);
         }
     }
}

bool TerminalWidget::isConnected() const
{
    return m_sshClient && m_sshClient->isConnected();
}

void TerminalWidget::setRenderEnabled(bool enabled)
{
    if (enabled) {
        if (!m_frameTimer->isActive()) m_frameTimer->start();
    } else {
        if (m_frameTimer->isActive()) m_frameTimer->stop();
    }
}

void TerminalWidget::initializeGL()
{
    if (!initializeOpenGLFunctions()) {
        qFatal("OpenGL 4.5 functions initialization failed");
    }

    glClearColor(0.0f, 0.0f, 0.0f, m_opacity); 
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE); 
    
    m_particleSystem->init();
    m_particleSystem->resize(width(), height());
}

void TerminalWidget::resizeGL(int w, int h)
{
    int charWidth = 16;
    int charHeight = 29;
    int cols = std::max(80, w / charWidth);
    int rows = std::max(24, h / charHeight);
    
    if (m_terminalModel && (m_terminalModel->cols() != cols || m_terminalModel->rows() != rows)) {
        m_terminalModel->resize(cols, rows);
        if (m_sshClient) {
            m_sshClient->setPtySize(cols, rows);
        }
    }
    
    if (m_particleSystem) {
        m_particleSystem->resize(w, h);
        if (m_terminalModel) {
            m_particleSystem->updateParticlesFromTerminal(*m_terminalModel);
        }
    }
}

void TerminalWidget::paintGL()
{
    float current = m_elapsedTimer.nsecsElapsed() / 1000000000.0f;
    static float last = 0;
    m_deltaTime = current - last;
    last = current;

    if (m_screenDirty) {
        if (m_particleSystem && m_terminalModel) {
             m_particleSystem->updateParticlesFromTerminal(*m_terminalModel, 
                m_terminalModel->cursorX(), 
                m_terminalModel->cursorY(), 
                m_cursorBlinkState,
                m_selStart.x(), m_selStart.y(),
                m_selEnd.x(), m_selEnd.y(),
                m_hoveredLink.row, m_hoveredLink.startCol, m_hoveredLink.endCol);
        }
        m_screenDirty = false;
    }

    m_frameCount++;
    static float timeAccum = 0;
    timeAccum += m_deltaTime;
    if (timeAccum >= 1.0f) {
        m_particleSystem->adjustQuality(m_frameCount, 120.0f);
        m_frameCount = 0;
        timeAccum = 0;
    }
    
    // Beat Sim (Removed)
    
    updatePhysics();
    renderParticles();
}

void TerminalWidget::paintEvent(QPaintEvent *event)
{
    // 1. Render OpenGL (Particles)
    QOpenGLWidget::paintEvent(event);
    
    // 2. Render Minimap Overlay (QPainter)
    if (!m_terminalModel) return;
    
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);
    
    int mapWidth = 120;
    int w = width();
    int h = height();
    QRect mapRect(w - mapWidth, 0, mapWidth, h);
    
    // Background
    p.fillRect(mapRect, QColor(0, 0, 0, 150)); // Semi-transparent black
    
    // Borders
    p.setPen(QColor(60, 60, 60));
    p.drawLine(w - mapWidth, 0, w - mapWidth, h);
    
    // Calculate scaling
    int historyLines = m_terminalModel->historySize();
    int termLines = m_terminalModel->rows();
    int totalLines = historyLines + termLines;
    if (totalLines == 0) return;
    
    // Map total lines to widget height
    // If fewer lines than pixels, 1:1 or stretched? usually 1:1 or compressed.
    // If more lines than pixels, skip lines.
    
    double scaleY = (double)h / std::max(h, totalLines); 
    if (totalLines > h) scaleY = (double)h / totalLines;
    
    // Ideally we want pixel-perfect mapping for small buffers
    // For large buffers, we sample.
    
    p.setPen(Qt::NoPen);
    
    // Draw History
    // Optimization: Draw 1 rect per line if scaleY > 1, else draw blocks
    
    int step = 1;
    if (scaleY < 1.0) step = (int)(1.0 / scaleY);
    if (step < 1) step = 1;

    for (int r = 0; r < historyLines; r += step) {
        const auto& line = m_terminalModel->historyLine(r);
        int y = (int)(r * scaleY);
        
        // Sample line content
        for (int c = 0; c < line.size(); c++) {
            if (line[c].ch > 32) { // visible char
                // Map column to map width
                int x = w - mapWidth + (int)((float)c / m_terminalModel->cols() * (mapWidth - 4) + 2);
                
                // Color based on attribute or default amber
                QColor col(255, 176, 0, 120); // Amber semi-trans
                if (line[c].attr.fgColor == 1) col = QColor(255, 80, 80, 150); // Red
                else if (line[c].attr.fgColor == 2) col = QColor(80, 255, 80, 150); // Green
                else if (line[c].attr.fgColor == 4) col = QColor(80, 120, 255, 150); // Blue
                
                p.fillRect(x, y, 2, std::max(1, (int)scaleY), col);
            }
        }
    }
    
    // Draw Viewport Highlight (Visible Area)
    // viewOffset 0 = bottom (visible grid)
    // viewOffset > 0 = history
    // We need to map current view range [start, end] to Y pixels
    
    int viewOffset = m_terminalModel->viewOffset(); // 0 is active grid
    // Logic: 
    // Total lines = History + Active
    // Active grid is at index [historyLines ... historyLines+termLines-1] if offset=0
    // If offset > 0, we shift up into history.
    
    // Let's assume logical index 0 is oldest history.
    // Max index is (totalLines - 1).
    // Visible window starts at: (historyLines - viewOffset)
    // Visible window height: termLines
    
    int visibleStart = historyLines - viewOffset;
    int visibleEnd = visibleStart + termLines;
    
    // Clamp
    if (visibleStart < 0) visibleStart = 0;
    
    int yStart = (int)(visibleStart * scaleY);
    int yEnd = (int)(visibleEnd * scaleY);
    int hRect = std::max(4, yEnd - yStart);
    
    // Draw highlight rect
    p.fillRect(w - mapWidth, yStart, mapWidth, hRect, QColor(255, 255, 255, 30)); // Highlight
    p.setPen(QColor(255, 255, 255, 80));
    p.drawRect(w - mapWidth, yStart, mapWidth - 1, hRect - 1); // Border
}

void TerminalWidget::updatePhysics()
{
    m_particleSystem->update(m_deltaTime);
}

void TerminalWidget::renderParticles()
{
    glClearColor(0.0f, 0.0f, 0.0f, m_opacity);
    glClear(GL_COLOR_BUFFER_BIT);
    QMatrix4x4 projection;
    projection.ortho(0, width(), height(), 0, -1, 1);
    m_particleSystem->render(projection);
}


void TerminalWidget::sendData(const QByteArray& data)
{
    if (m_sshClient) {
        m_sshClient->sendData(data);
    }
}

bool TerminalWidget::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab) {
            keyPressEvent(keyEvent);
            return true; 
        }
    }
    return QOpenGLWidget::event(event);
}

void TerminalWidget::keyPressEvent(QKeyEvent *event)
{
    if (!m_sshClient) return;
    
    // Clipboard & Split Shortcuts
    if (event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) {
        if (event->key() == Qt::Key_C) { copySelection(); return; }
        if (event->key() == Qt::Key_V) { pasteClipboard(); return; }
        if (event->key() == Qt::Key_E) { emit splitRequest(Qt::Horizontal); return; }
        if (event->key() == Qt::Key_O) { emit splitRequest(Qt::Vertical); return; }
    }
    
    // Zoom Shortcuts (Ctrl +/-)
    if (event->modifiers() & Qt::ControlModifier) {
         if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) {
             emit zoomRequest(0.1f);
             return; 
         }
         if (event->key() == Qt::Key_Minus) {
             emit zoomRequest(-0.1f);
             return;
         }
    }
    
    // Navigation (Ctrl+Arrows)
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->key() == Qt::Key_Up) { emit navigateRequest(0, -1); return; }
        if (event->key() == Qt::Key_Down) { emit navigateRequest(0, 1); return; }
        if (event->key() == Qt::Key_Left) { emit navigateRequest(-1, 0); return; }
        if (event->key() == Qt::Key_Right) { emit navigateRequest(1, 0); return; }
    }
    
    VTermModifier mod = VTERM_MOD_NONE;
    if (event->modifiers() & Qt::ShiftModifier) mod = (VTermModifier)(mod | VTERM_MOD_SHIFT);
    if (event->modifiers() & Qt::AltModifier)   mod = (VTermModifier)(mod | VTERM_MOD_ALT);
    if (event->modifiers() & Qt::ControlModifier) mod = (VTermModifier)(mod | VTERM_MOD_CTRL);

    VTermKey vtKey = VTERM_KEY_NONE;
    switch(event->key()) {
        case Qt::Key_Enter: 
        case Qt::Key_Return: 
            vtKey = VTERM_KEY_ENTER; 
            // SONIC BOOM TRIGGER
            if (m_particleSystem && getAnimationStyle() == 4) { // 4 = Sonic
                // Get cursor position in pixels
                 if (m_terminalModel) {
                     float cw = (float)width() / std::max(1, m_terminalModel->cols());
                     float ch = (float)height() / std::max(1, m_terminalModel->rows());
                     float x = m_terminalModel->cursorX() * cw + (cw/2);
                     float y = m_terminalModel->cursorY() * ch + (ch/2);
                     m_particleSystem->triggerShockwave(x, y);
                 }
            }
            break;
        case Qt::Key_Tab: vtKey = VTERM_KEY_TAB; break;
        case Qt::Key_Backspace: vtKey = VTERM_KEY_BACKSPACE; break;
        case Qt::Key_Escape: vtKey = VTERM_KEY_ESCAPE; break;
        case Qt::Key_Up: vtKey = VTERM_KEY_UP; break;
        case Qt::Key_Down: vtKey = VTERM_KEY_DOWN; break;
        case Qt::Key_Left: vtKey = VTERM_KEY_LEFT; break;
        case Qt::Key_Right: vtKey = VTERM_KEY_RIGHT; break;
        case Qt::Key_Insert: vtKey = VTERM_KEY_INS; break;
        case Qt::Key_Delete: vtKey = VTERM_KEY_DEL; break;
        case Qt::Key_Home: vtKey = VTERM_KEY_HOME; break;
        case Qt::Key_End: vtKey = VTERM_KEY_END; break;
        case Qt::Key_PageUp: vtKey = VTERM_KEY_PAGEUP; break;
        case Qt::Key_PageDown: vtKey = VTERM_KEY_PAGEDOWN; break;
        default: 
            if (event->key() >= Qt::Key_F1 && event->key() <= Qt::Key_F12) 
                vtKey = (VTermKey)(VTERM_KEY_FUNCTION(event->key() - Qt::Key_F1 + 1));
            break;
    }
    
    if (vtKey != VTERM_KEY_NONE) {
        m_terminalModel->sendKey(vtKey, mod);
    } else {
        QString text = event->text();
        if (!text.isEmpty()) {
            m_terminalModel->sendText(text);
        }
    }
}

void TerminalWidget::focusInEvent(QFocusEvent *event)
{
    QOpenGLWidget::focusInEvent(event);
    emit focused();
}

void TerminalWidget::mousePressEvent(QMouseEvent *event)
{
    // If app is tracking mouse, forward to terminal
    if (m_terminalModel->mouseTracking()) {
        int button = 1;
        if (event->button() == Qt::MiddleButton) button = 2;
        else if (event->button() == Qt::RightButton) button = 3;
        
        VTermModifier mod = VTERM_MOD_NONE;
        if (event->modifiers() & Qt::ShiftModifier) mod = (VTermModifier)(mod | VTERM_MOD_SHIFT);
        if (event->modifiers() & Qt::AltModifier)   mod = (VTermModifier)(mod | VTERM_MOD_ALT);
        if (event->modifiers() & Qt::ControlModifier) mod = (VTermModifier)(mod | VTERM_MOD_CTRL);
        
        m_terminalModel->sendMouse(button, true, mod);
        return;
    }
    
    // Middle-click: Paste
    if (event->button() == Qt::MiddleButton) {
        pasteClipboard();
        return;
    }
    
    // Left-click: Link or Selection
    if (event->button() == Qt::LeftButton) {
        if (m_hoveredLink.isValid()) {
            QDesktopServices::openUrl(QUrl(m_hoveredLink.url));
            return;
        }

        clearSelection();
        m_selecting = true;
        m_selStart = pixelToCell(event->position());
        m_selEnd = m_selStart;
        m_screenDirty = true;
        update();
    }
}

void TerminalWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_terminalModel->mouseTracking()) {
        int button = 1;
        if (event->button() == Qt::MiddleButton) button = 2;
        else if (event->button() == Qt::RightButton) button = 3;
        
        VTermModifier mod = VTERM_MOD_NONE;
        if (event->modifiers() & Qt::ShiftModifier) mod = (VTermModifier)(mod | VTERM_MOD_SHIFT);
        if (event->modifiers() & Qt::AltModifier)   mod = (VTermModifier)(mod | VTERM_MOD_ALT);
        if (event->modifiers() & Qt::ControlModifier) mod = (VTermModifier)(mod | VTERM_MOD_CTRL);
        
        m_terminalModel->sendMouse(button, false, mod);
        return;
    }
    
    // Left-click release: Finalize selection and auto-copy
    if (event->button() == Qt::LeftButton && m_selecting) {
        m_selecting = false;
        m_selEnd = pixelToCell(event->position());
        
        // Auto-copy on selection (PuTTY behavior)
        if (hasSelection()) {
            copySelection();
        }
        m_screenDirty = true;
        update();
    }
}

void TerminalWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_terminalModel->mouseTracking()) {
        float cellW = (float)width() / std::max(1, m_terminalModel->cols());
        float cellH = (float)height() / std::max(1, m_terminalModel->rows());
        int col = (int)(event->position().x() / cellW);
        int row = (int)(event->position().y() / cellH);
        
        VTermModifier mod = VTERM_MOD_NONE;
        if (event->modifiers() & Qt::ShiftModifier) mod = (VTermModifier)(mod | VTERM_MOD_SHIFT);
        if (event->modifiers() & Qt::AltModifier)   mod = (VTermModifier)(mod | VTERM_MOD_ALT);
        if (event->modifiers() & Qt::ControlModifier) mod = (VTermModifier)(mod | VTERM_MOD_CTRL);
        
        m_terminalModel->sendMouseMove(col, row, mod);
        return;
    }
    
    // Detect Links
    detectLinkAt(event->position().toPoint());
    
    // Update selection during drag
    if (m_selecting) {
        m_selEnd = pixelToCell(event->position());
        m_screenDirty = true;
        update();
    }
}

void TerminalWidget::wheelEvent(QWheelEvent *event)
{
    if (m_terminalModel->mouseTracking()) {
        // ... (existing mouse code) ... 
        // VTerm doesn't have wheel buttons directly in API typically, 
        // but often encoded as buttons 4/5
        int button = (event->angleDelta().y() > 0) ? 4 : 5; 
        
        VTermModifier mod = VTERM_MOD_NONE;
        if (event->modifiers() & Qt::ShiftModifier) mod = (VTermModifier)(mod | VTERM_MOD_SHIFT);
        if (event->modifiers() & Qt::AltModifier)   mod = (VTermModifier)(mod | VTERM_MOD_ALT);
        if (event->modifiers() & Qt::ControlModifier) mod = (VTermModifier)(mod | VTERM_MOD_CTRL);
        
        m_terminalModel->sendMouse(button, true, mod);
        return;
    }
    
    int steps = event->angleDelta().y() / 40; 
    // Invert direction for natural scrolling
    m_terminalModel->scrollView(-steps); 
}

// Graphics Settings Delegation
void TerminalWidget::setGlowIntensity(float val) { if (m_particleSystem) m_particleSystem->setGlowIntensity(val); }
void TerminalWidget::setOpacity(float val) { m_opacity = val; update(); }
void TerminalWidget::setBrightness(float val) { if (m_particleSystem) m_particleSystem->setBrightness(val); }
void TerminalWidget::setVibrance(float val) { if (m_particleSystem) m_particleSystem->setVibrance(val); } // NEW
void TerminalWidget::setSpringK(float val) { if (m_particleSystem) m_particleSystem->setSpringK(val); }
void TerminalWidget::setDrag(float val) { if (m_particleSystem) m_particleSystem->setDrag(val); }
void TerminalWidget::setShimmerSpeed(float val) { if (m_particleSystem) m_particleSystem->setShimmerSpeed(val); }
void TerminalWidget::setFont(int val) { if (m_particleSystem) m_particleSystem->setFontById(val); }
void TerminalWidget::setDensity(int val) { 
    if (m_particleSystem) {
        m_particleSystem->setDensity(val);
        m_screenDirty = true; 
        update();
    }
}
void TerminalWidget::setZoomLevel(float zoom) { 
    if (m_particleSystem) {
        m_particleSystem->setZoomLevel(zoom);
        m_screenDirty = true;
        update();
    }
}
void TerminalWidget::setAnimationStyle(int style) { if (m_particleSystem) m_particleSystem->setAnimationStyle(style); }
void TerminalWidget::setTheme(int theme) { if (m_particleSystem) m_particleSystem->setTheme(theme); }

// Getters 
float TerminalWidget::getGlowIntensity() const { return m_particleSystem ? m_particleSystem->getGlowIntensity() : 1.0f; }
float TerminalWidget::getOpacity() const { return m_opacity; }
float TerminalWidget::getBrightness() const { return m_particleSystem ? m_particleSystem->getBrightness() : 1.0f; }
float TerminalWidget::getVibrance() const { return m_particleSystem ? m_particleSystem->getVibrance() : 1.0f; } // NEW
float TerminalWidget::getSpringK() const { return m_particleSystem ? m_particleSystem->getSpringK() : 15.0f; }
float TerminalWidget::getDrag() const { return m_particleSystem ? m_particleSystem->getDrag() : 0.85f; }
float TerminalWidget::getShimmerSpeed() const { return m_particleSystem ? m_particleSystem->getShimmerSpeed() : 4.0f; }
int TerminalWidget::getDensity() const { return m_particleSystem ? m_particleSystem->getDensity() : 8; }
int TerminalWidget::getFont() const { return m_particleSystem ? m_particleSystem->getFontId() : 0; }
float TerminalWidget::getZoomLevel() const { return m_particleSystem ? m_particleSystem->getZoomLevel() : 1.0f; }
int TerminalWidget::getTheme() const { return m_particleSystem ? m_particleSystem->getTheme() : 0; }
int TerminalWidget::getAnimationStyle() const { return m_particleSystem ? m_particleSystem->getAnimationStyle() : 0; }

// ==== Text Selection Methods ====

QPoint TerminalWidget::pixelToCell(const QPointF& pos) const
{
    if (!m_terminalModel) return QPoint(-1, -1);
    
    float cellW = (float)width() / std::max(1, m_terminalModel->cols());
    float cellH = (float)height() / std::max(1, m_terminalModel->rows());
    
    int col = qBound(0, (int)(pos.x() / cellW), m_terminalModel->cols() - 1);
    int row = qBound(0, (int)(pos.y() / cellH), m_terminalModel->rows() - 1);
    
    return QPoint(col, row);
}

void TerminalWidget::clearSelection()
{
    m_selStart = QPoint(-1, -1);
    m_selEnd = QPoint(-1, -1);
    m_selecting = false;
    m_screenDirty = true;
    update();
}

bool TerminalWidget::hasSelection() const
{
    return m_selStart != QPoint(-1, -1) && m_selEnd != QPoint(-1, -1) && m_selStart != m_selEnd;
}

// Smart Link Detection
void TerminalWidget::detectLinkAt(QPoint pos)
{
    if (!m_terminalModel) return;
    QPoint cell = pixelToCell(pos);
    if (cell.x() < 0) {
        if (m_hoveredLink.isValid()) {
             m_hoveredLink.clear();
             m_screenDirty = true;
             setCursor(Qt::ArrowCursor);
             update();
        }
        return;
    }
    
    // If mouse moved but still traversing same link, verify range
    if (m_hoveredLink.isValid() && m_hoveredLink.row == cell.y() && 
        cell.x() >= m_hoveredLink.startCol && cell.x() <= m_hoveredLink.endCol) {
        return; // Still over same link
    }
    
    // New potential link
    m_hoveredLink.clear();
    setCursor(Qt::ArrowCursor);
    
    // Extract line text
    QString lineText;
    int cols = m_terminalModel->cols();
    for (int c = 0; c < cols; ++c) {
        const TerminalCell& tc = m_terminalModel->cell(c, cell.y());
        lineText += QChar(tc.ch > 0 ? (char16_t)tc.ch : ' ');
    }
    
    // Regex for URLs
    static QRegularExpression urlRegex(R"((https?://\S+))");
    QRegularExpressionMatchIterator i = urlRegex.globalMatch(lineText);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        if (cell.x() >= match.capturedStart() && cell.x() < match.capturedEnd()) {
            m_hoveredLink.url = match.captured();
            m_hoveredLink.row = cell.y();
            m_hoveredLink.startCol = match.capturedStart();
            m_hoveredLink.endCol = match.capturedEnd() - 1;
            
            setCursor(Qt::PointingHandCursor);
            m_screenDirty = true;
            update();
            return;
        }
    }
}

QString TerminalWidget::getSelectedText() const
{
    if (!hasSelection() || !m_terminalModel) return QString();
    
    // Normalize start/end (start is top-left, end is bottom-right)
    QPoint start = m_selStart;
    QPoint end = m_selEnd;
    
    // Swap if end is before start (dragged backwards)
    if (end.y() < start.y() || (end.y() == start.y() && end.x() < start.x())) {
        std::swap(start, end);
    }
    
    QString result;
    int cols = m_terminalModel->cols();
    
    for (int row = start.y(); row <= end.y(); ++row) {
        int colStart = (row == start.y()) ? start.x() : 0;
        int colEnd = (row == end.y()) ? end.x() : cols - 1;
        
        for (int col = colStart; col <= colEnd; ++col) {
            const TerminalCell& cell = m_terminalModel->cell(col, row);
            if (cell.ch != 0) {
                // Handle Unicode chars including those > 0xFFFF (needs surrogate pairs)
                if (cell.ch <= 0xFFFF) {
                    result += QChar(static_cast<char16_t>(cell.ch));
                } else {
                    // Use fromUcs4 for characters outside BMP
                    char32_t ch32 = cell.ch;
                    result += QString::fromUcs4(&ch32, 1);
                }
            } else {
                result += ' ';
            }
        }
        
        // Add newline between rows (but not after last row)
        if (row < end.y()) {
            result += '\n';
        }
    }
    
    // Trim trailing spaces from each line
    QStringList lines = result.split('\n');
    for (QString& line : lines) {
        while (line.endsWith(' ')) line.chop(1);
    }
    result = lines.join('\n');
    
    return result;
}

void TerminalWidget::copySelection()
{
    QString text = getSelectedText();
    if (!text.isEmpty()) {
        QClipboard* clipboard = QGuiApplication::clipboard();
        clipboard->setText(text);
        qDebug() << "Copied to clipboard:" << text.length() << "chars";
    }
}

void TerminalWidget::pasteClipboard()
{
    QClipboard* clipboard = QGuiApplication::clipboard();
    QString text = clipboard->text();
    if (!text.isEmpty() && m_terminalModel) {
        m_terminalModel->sendText(text);
    }
}
