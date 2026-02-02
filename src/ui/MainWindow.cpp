#include "MainWindow.h"
#include "TerminalTab.h"
#include "../renderer/TerminalWidget.h"
#include "../renderer/TerminalWidget.h"
#include "ConnectionDialog.h"
#include "GraphicsSettingsDialog.h"
#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QTabBar> 
#include <QToolButton> 

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_tabWidget(new QTabWidget(this))
{
    setupUi();
    setupMenu();
    setupShortcuts();

    setupShortcuts();

    // Start with one empty tab (shows the terminal background)
    createNewTab();

    // Prompt for connection immediately
    QTimer::singleShot(100, this, &MainWindow::onNewConnection);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    setWindowTitle("Amber SSH v2.0");
    resize(1280, 800);
    
    // Transparency Support
    setAttribute(Qt::WA_TranslucentBackground);
    
    // Setup Tab Widget
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    m_tabWidget->setDocumentMode(true); 
    setCentralWidget(m_tabWidget);

    // New Tab Button
    QToolButton* newTabButton = new QToolButton(this);
    newTabButton->setText("+");
    newTabButton->setToolTip("New Connection");
    newTabButton->setCursor(Qt::PointingHandCursor);
    // Adjust size/style via stylesheet or manually
    m_tabWidget->setCornerWidget(newTabButton, Qt::TopRightCorner);
    
    connect(newTabButton, &QToolButton::clicked, this, &MainWindow::onNewConnection);

    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    // Styling
    setStyleSheet(R"(
        QMainWindow { background-color: transparent; } /* Transparent hole to desktop */
        QMenuBar { background-color: #222; color: #ccc; border-bottom: 1px solid #333; }
        QMenuBar::item { padding: 4px 10px; background: transparent; }
        QMenuBar::item:selected { background: #444; }
        
        QTabWidget::pane { border: 1px solid #333; background-color: transparent; } 
        QToolButton { 
            background: #222; color: #dca000; border: none; font-weight: bold; padding: 2px 8px; border-radius: 2px;
        }
        QToolButton:hover { background: #333; }
        QToolButton:pressed { background: #111; }
        
        QTabBar { background-color: #222; } /* Solid strip behind tabs */
        QTabBar::tab {
            background-color: #222;
            color: #888;
            padding: 8px 16px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
            margin-right: 2px;
        }
        QTabBar::tab:selected {
            background-color: #333; /* Darker opaque for selected */
            color: #dca000; /* Amber */
            border-bottom: 2px solid #dca000;
        }
        QTabBar::tab:hover {
            background-color: #444;
        }
    )");
}

void MainWindow::setupMenu()
{
    // File Menu
    m_fileMenu = menuBar()->addMenu("&File");
    
    QAction* newConnAction = m_fileMenu->addAction("&New Connection...");
    newConnAction->setShortcut(QKeySequence("Ctrl+N"));
    connect(newConnAction, &QAction::triggered, this, &MainWindow::onNewConnection);

    QAction* newTabAction = m_fileMenu->addAction("New &Tab");
    newTabAction->setShortcut(QKeySequence("Ctrl+T"));
    connect(newTabAction, &QAction::triggered, this, &MainWindow::createNewTab);

    // Split Actions
    m_fileMenu->addSeparator();
    QAction* splitHAction = m_fileMenu->addAction("Split &Horizontal");
    splitHAction->setShortcut(QKeySequence("Ctrl+Shift+H"));
    connect(splitHAction, &QAction::triggered, this, [this](){
        TerminalTab* tab = qobject_cast<TerminalTab*>(m_tabWidget->currentWidget());
        if (tab) tab->splitHorizontal();
    });

    QAction* splitVAction = m_fileMenu->addAction("Split &Vertical");
    splitVAction->setShortcut(QKeySequence("Ctrl+Shift+V"));
    connect(splitVAction, &QAction::triggered, this, [this](){
        TerminalTab* tab = qobject_cast<TerminalTab*>(m_tabWidget->currentWidget());
        if (tab) tab->splitVertical();
    });

    m_fileMenu->addSeparator();

    QAction* exitAction = m_fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

    // View Menu
    m_viewMenu = menuBar()->addMenu("&View");
    // Options
    QAction* graphicsAction = m_viewMenu->addAction("Graphics &Settings...");
    graphicsAction->setShortcut(QKeySequence("Ctrl+G"));
    connect(graphicsAction, &QAction::triggered, this, &MainWindow::onGraphicsSettings);

    // Tools Menu
    QMenu* toolsMenu = menuBar()->addMenu("&Tools");
    QAction* broadcastAction = toolsMenu->addAction("Broadcast &Input");
    broadcastAction->setCheckable(true);
    broadcastAction->setShortcut(QKeySequence("Alt+B"));
    connect(broadcastAction, &QAction::toggled, this, [this](bool checked){
        TerminalTab* tab = qobject_cast<TerminalTab*>(m_tabWidget->currentWidget());
        if (tab) tab->setBroadcastInput(checked);
    });

    // Help Menu
    m_helpMenu = menuBar()->addMenu("&Help");
    m_helpMenu->addAction("About Qt", qApp, &QApplication::aboutQt);
}

void MainWindow::setupShortcuts()
{
    // Close Tab Shortcut
    QAction* closeTabAction = new QAction(this);
    closeTabAction->setShortcut(QKeySequence("Ctrl+W"));
    connect(closeTabAction, &QAction::triggered, this, [this](){
        if (m_tabWidget->count() > 0) {
            onTabCloseRequested(m_tabWidget->currentIndex());
        }
    });
    addAction(closeTabAction);
}

TerminalTab* MainWindow::createNewTab()
{
    TerminalTab* tab = new TerminalTab(this);
    int index = m_tabWidget->addTab(tab, "Terminal");
    m_tabWidget->setCurrentIndex(index);
    return tab;
}

void MainWindow::onNewConnection()
{
    ConnectionDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        TerminalTab* tab = qobject_cast<TerminalTab*>(m_tabWidget->currentWidget());
        
        // Reuse current tab if exists and NOT connected
        if (!tab || tab->isConnected()) {
             tab = createNewTab();
        }
        
        if (tab) {
            tab->connectToHost(
                dialog.host(),
                dialog.port(),
                dialog.user(),
                dialog.password(),
                dialog.keyPath(),
                dialog.forwards()
            );
            
            int idx = m_tabWidget->indexOf(tab);
            if (idx >= 0) {
                m_tabWidget->setTabText(idx, dialog.host());
            }
            tab->setFocus();
        }
    }
}

void MainWindow::onTabCloseRequested(int index)
{
    QWidget* widget = m_tabWidget->widget(index);
    m_tabWidget->removeTab(index);
    delete widget; 
}

void MainWindow::onTabChanged(int index)
{
    if (index >= 0) {
         QWidget* widget = m_tabWidget->widget(index);
         if (widget) widget->setFocus();
    }
}

void MainWindow::updateTabTitle(int index, const QString& title)
{
    m_tabWidget->setTabText(index, title);
}

void MainWindow::onGraphicsSettings()
{
    if (!m_graphicsDialog) {
        m_graphicsDialog = new GraphicsSettingsDialog(this);
        
        // Connect Dialog -> Current Tab -> Terminal
        // Note: This connects GLOBAL settings. 
        // Ideally we update ALL tabs or just the current?
        // Let's stick to Current Tab for now, OR iterate all tabs.
        // Or simpler: Dialog emits, MainWindow catches and iterates.
        
        connect(m_graphicsDialog, &GraphicsSettingsDialog::glowIntensityChanged, this, [this](float v){
            for(int i=0; i<m_tabWidget->count(); ++i) {
                TerminalTab* tab = qobject_cast<TerminalTab*>(m_tabWidget->widget(i));
                if(tab) tab->setGlowIntensity(v);
            }
        });

        connect(m_graphicsDialog, &GraphicsSettingsDialog::opacityChanged, this, [this](float v){
            for(int i=0; i<m_tabWidget->count(); ++i) {
                TerminalTab* tab = qobject_cast<TerminalTab*>(m_tabWidget->widget(i));
                if(tab) tab->setOpacity(v);
            }
        });
        
        connect(m_graphicsDialog, &GraphicsSettingsDialog::brightnessChanged, this, [this](float v){
            for(int i=0; i<m_tabWidget->count(); ++i) {
                TerminalTab* tab = qobject_cast<TerminalTab*>(m_tabWidget->widget(i));
                if(tab) tab->setBrightness(v);
            }
        });
        
        connect(m_graphicsDialog, &GraphicsSettingsDialog::vibranceChanged, this, [this](float v){
            for(int i=0; i<m_tabWidget->count(); ++i) {
                TerminalTab* tab = qobject_cast<TerminalTab*>(m_tabWidget->widget(i));
                if(tab) tab->setVibrance(v);
            }
        });


        connect(m_graphicsDialog, &GraphicsSettingsDialog::springKChanged, this, [this](float v){
             for(int i=0; i<m_tabWidget->count(); ++i) {
                TerminalTab* tab = qobject_cast<TerminalTab*>(m_tabWidget->widget(i));
                 if(tab) tab->setSpringK(v);
             }
        });
        
        connect(m_graphicsDialog, &GraphicsSettingsDialog::dragChanged, this, [this](float v){
             for(int i=0; i<m_tabWidget->count(); ++i) {
                TerminalTab* tab = qobject_cast<TerminalTab*>(m_tabWidget->widget(i));
                 if(tab) tab->setDrag(v);
             }
        });
        
        connect(m_graphicsDialog, &GraphicsSettingsDialog::shimmerSpeedChanged, this, [this](float v){
             for(int i=0; i<m_tabWidget->count(); ++i) {
                TerminalTab* tab = qobject_cast<TerminalTab*>(m_tabWidget->widget(i));
                 if(tab) tab->setShimmerSpeed(v);
             }
        });
        
        connect(m_graphicsDialog, &GraphicsSettingsDialog::densityChanged, this, [this](int v){
             for(int i=0; i<m_tabWidget->count(); ++i) {
                TerminalTab* tab = qobject_cast<TerminalTab*>(m_tabWidget->widget(i));
                 if(tab) tab->setDensity(v);
             }
        });

        connect(m_graphicsDialog, &GraphicsSettingsDialog::animationStyleChanged, this, [this](int style){
             for(int i=0; i<m_tabWidget->count(); ++i) {
                TerminalTab* tab = qobject_cast<TerminalTab*>(m_tabWidget->widget(i));
                 if(tab) tab->setAnimationStyle(style);
             }
        });
        
        connect(m_graphicsDialog, &GraphicsSettingsDialog::themeChanged, this, [this](int theme){
             for(int i=0; i<m_tabWidget->count(); ++i) {
                TerminalTab* tab = qobject_cast<TerminalTab*>(m_tabWidget->widget(i));
                 if(tab) tab->setTheme(theme);
             }
        });
        
        connect(m_graphicsDialog, &GraphicsSettingsDialog::fontChanged, this, [this](int font){
             for(int i=0; i<m_tabWidget->count(); ++i) {
                TerminalTab* tab = qobject_cast<TerminalTab*>(m_tabWidget->widget(i));
                 if(tab) tab->setFont(font);
             }
        });
    }
    
    // Sync UI with current tab (if exists)
    TerminalTab* tab = qobject_cast<TerminalTab*>(m_tabWidget->currentWidget());
    if (tab) {
        m_graphicsDialog->setValues(
            tab->getGlowIntensity(),
            tab->getOpacity(),
            tab->getBrightness(),
            tab->getSpringK(),
            tab->getDrag(),
            tab->getShimmerSpeed(),
            tab->getDensity(),
            tab->getAnimationStyle(),
            tab->getTheme(),
            tab->getVibrance(),
            tab->getFont()
        );
    }
    
    m_graphicsDialog->show();
    m_graphicsDialog->raise();
    m_graphicsDialog->activateWindow();
}
