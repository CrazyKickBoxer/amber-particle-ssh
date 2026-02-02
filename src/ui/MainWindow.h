#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStatusBar> // Added for status feedback

class TerminalWidget;
class TerminalTab;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    TerminalTab* createNewTab();

private slots:
    void onNewConnection();
    void onTabCloseRequested(int index);
    void onTabChanged(int index);
    void updateTabTitle(int index, const QString& title);
    void onGraphicsSettings();

private:
    void setupUi();
    void setupMenu();
    void setupShortcuts();

    QTabWidget* m_tabWidget;
    
    // Menus
    QMenu* m_fileMenu;
    QMenu* m_viewMenu;
    QMenu* m_helpMenu;
    
    // Dialogs
    class GraphicsSettingsDialog* m_graphicsDialog = nullptr;
};
