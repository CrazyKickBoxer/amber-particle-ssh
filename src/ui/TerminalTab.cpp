#include "TerminalTab.h"
#include <QDebug>
#include <QApplication>

TerminalTab::TerminalTab(QWidget* parent)
    : QWidget(parent)
    , m_layout(new QVBoxLayout(this))
{
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    
    setupInitialTerminal();
}

TerminalTab::~TerminalTab()
{
}

void TerminalTab::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    for(auto* t : m_terminals) t->setRenderEnabled(true);
}

void TerminalTab::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    for(auto* t : m_terminals) t->setRenderEnabled(false);
}

void TerminalTab::setupInitialTerminal()
{
    // Start with a single TerminalWidget
    TerminalWidget* term = new TerminalWidget(this);
    m_layout->addWidget(term);
    setActiveTerminal(term);
    m_terminals.append(term);
    
    connect(term, &TerminalWidget::focused, this, [this, term](){
        setActiveTerminal(term);
    });
    connect(term, &TerminalWidget::navigateRequest, this, &TerminalTab::navigateRequest);
    connect(term, &TerminalWidget::inputData, this, &TerminalTab::handleInputBroadcast);
    connect(term, &TerminalWidget::splitRequest, this, &TerminalTab::split);
    connect(term, &TerminalWidget::zoomRequest, this, [this](float delta){
        float current = getZoomLevel();
        setZoomLevel(current + delta);
    });
}

TerminalWidget* TerminalTab::activeTerminal() const
{
    // If we have an active tracking, return it. 
    // Otherwise return the first one or nullptr.
    if (m_activeTerminal) return m_activeTerminal;
    if (!m_terminals.isEmpty()) return m_terminals.first();
    return nullptr;
}

void TerminalTab::setActiveTerminal(TerminalWidget* terminal)
{
    m_activeTerminal = terminal;
    if (terminal) {
        setFocusProxy(terminal);
        terminal->setFocus();
    }
}

void TerminalTab::connectToHost(const QString& host, int port, const QString& user, const QString& password, const QString& keyPath, const QList<SshClient::PortForwardRule>& rules)
{
    if (activeTerminal()) {
        activeTerminal()->connectToHost(host, port, user, password, keyPath, rules);
    }
}

bool TerminalTab::isConnected() const {
    return activeTerminal() && activeTerminal()->isConnected();
}

void TerminalTab::setBroadcastInput(bool enabled)
{
    m_broadcastInput = enabled;
}

void TerminalTab::handleInputBroadcast(const QByteArray& data)
{
    if (!m_broadcastInput) return;
    
    TerminalWidget* sender = qobject_cast<TerminalWidget*>(QObject::sender());
    if (!sender) return;
    
    for (TerminalWidget* term : m_terminals) {
        if (term != sender) {
            term->sendData(data);
        }
    }
}

void TerminalTab::navigateRequest(int dx, int dy)
{
    TerminalWidget* current = activeTerminal();
    if (!current) return;
    
    // Heuristic: Find a widget in the visual direction
    // Map current center to global, reset offset, check widget
    QPoint center = current->mapToGlobal(current->rect().center());
    
    int jumpX = (current->width() / 2) + 20;
    int jumpY = (current->height() / 2) + 20;
    
    QPoint target = center + QPoint(dx * jumpX, dy * jumpY);
    QWidget* found = QApplication::widgetAt(target);
    
    TerminalWidget* nextTerm = qobject_cast<TerminalWidget*>(found);
    if (nextTerm && nextTerm != current) {
        setActiveTerminal(nextTerm);
    }
}

void TerminalTab::splitHorizontal()
{
    split(Qt::Horizontal);
}

void TerminalTab::splitVertical()
{
    split(Qt::Vertical);
}

void TerminalTab::split(Qt::Orientation orientation)
{
    TerminalWidget* current = activeTerminal();
    if (!current) return;

    // Create new terminal
    TerminalWidget* newTerm = new TerminalWidget(this);
    m_terminals.append(newTerm);
    
    // Find parent of current terminal
    QWidget* parent = current->parentWidget();
    QSplitter* parentSplitter = qobject_cast<QSplitter*>(parent);
    
    if (parentSplitter && parentSplitter->orientation() == orientation) {
        // Add to existing splitter
        int index = parentSplitter->indexOf(current);
        parentSplitter->insertWidget(index + 1, newTerm);
    } else {
        // Need to replace 'current' with a new splitter containing [current, newTerm]
        QSplitter* newSplitter = new QSplitter(orientation, this);
        newSplitter->setChildrenCollapsible(false); // standard terminal behavior
        
        // If current was in a layout (root)
        if (m_layout->indexOf(current) != -1) {
            m_layout->replaceWidget(current, newSplitter);
        } 
        // If current was in another splitter
        else if (parentSplitter) {
            int index = parentSplitter->indexOf(current);
            parentSplitter->replaceWidget(index, newSplitter);
        }
        
        // Add widgets to new splitter
        newSplitter->addWidget(current);
        newSplitter->addWidget(newTerm);
        
        // QSplitter takes ownership, but check parenting
        current->setParent(newSplitter);
        current->show(); // ensure visible
    }
    
    connect(newTerm, &TerminalWidget::focused, this, [this, newTerm](){
        setActiveTerminal(newTerm);
    });
    connect(newTerm, &TerminalWidget::navigateRequest, this, &TerminalTab::navigateRequest);
    connect(newTerm, &TerminalWidget::inputData, this, &TerminalTab::handleInputBroadcast);
    connect(newTerm, &TerminalWidget::splitRequest, this, &TerminalTab::split);
    connect(newTerm, &TerminalWidget::zoomRequest, this, [this](float delta){
        float current = getZoomLevel();
        setZoomLevel(current + delta);
    });
    
    setActiveTerminal(newTerm);
}

// Graphics Delegation
void TerminalTab::setGlowIntensity(float val) { for(auto* t : m_terminals) t->setGlowIntensity(val); }
void TerminalTab::setOpacity(float val) { for(auto* t : m_terminals) t->setOpacity(val); }
void TerminalTab::setBrightness(float val) { for(auto* t : m_terminals) t->setBrightness(val); }
void TerminalTab::setVibrance(float val) { for(auto* t : m_terminals) t->setVibrance(val); } // NEW
void TerminalTab::setSpringK(float val) { for(auto* t : m_terminals) t->setSpringK(val); }
void TerminalTab::setDrag(float val) { for(auto* t : m_terminals) t->setDrag(val); }
void TerminalTab::setShimmerSpeed(float val) { for(auto* t : m_terminals) t->setShimmerSpeed(val); }
void TerminalTab::setDensity(int val) { for(auto* t : m_terminals) t->setDensity(val); }
void TerminalTab::setFont(int val) { for(auto* t : m_terminals) t->setFont(val); } // NEW
void TerminalTab::setZoomLevel(float val) { for(auto* t : m_terminals) t->setZoomLevel(val); }
void TerminalTab::setTheme(int theme) { for(auto* t : m_terminals) t->setTheme(theme); }
void TerminalTab::setAnimationStyle(int style) { for(auto* t : m_terminals) t->setAnimationStyle(style); }

float TerminalTab::getGlowIntensity() const { return m_activeTerminal ? m_activeTerminal->getGlowIntensity() : 1.0f; }
float TerminalTab::getOpacity() const { return m_activeTerminal ? m_activeTerminal->getOpacity() : 0.85f; } 
float TerminalTab::getBrightness() const { return m_activeTerminal ? m_activeTerminal->getBrightness() : 1.0f; }
float TerminalTab::getVibrance() const { return m_activeTerminal ? m_activeTerminal->getVibrance() : 1.0f; } // NEW
float TerminalTab::getSpringK() const { return m_activeTerminal ? m_activeTerminal->getSpringK() : 15.0f; }
float TerminalTab::getDrag() const { return m_activeTerminal ? m_activeTerminal->getDrag() : 0.85f; }
float TerminalTab::getShimmerSpeed() const { return m_activeTerminal ? m_activeTerminal->getShimmerSpeed() : 4.0f; }
int TerminalTab::getDensity() const { return m_activeTerminal ? m_activeTerminal->getDensity() : 8; }
float TerminalTab::getZoomLevel() const { return m_activeTerminal ? m_activeTerminal->getZoomLevel() : 1.0f; }
int TerminalTab::getTheme() const { return m_activeTerminal ? m_activeTerminal->getTheme() : 0; }
int TerminalTab::getFont() const { return m_activeTerminal ? m_activeTerminal->getFont() : 0; }
int TerminalTab::getAnimationStyle() const { return m_activeTerminal ? m_activeTerminal->getAnimationStyle() : 0; }
