#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <deque>
#include "vterm.h"

struct TerminalAttribute {
    uint8_t fgColor = 7; // Default white/amber
    uint8_t bgColor = 0; // Default black
    
    bool fgTrueColor = false;
    uint8_t fgR=0, fgG=0, fgB=0;
    bool bgTrueColor = false;
    uint8_t bgR=0, bgG=0, bgB=0;

    bool bold = false;
    bool blink = false;
    bool inverse = false;
    bool underline = false;
    bool italic = false;
    
    bool operator==(const TerminalAttribute& other) const {
        return fgColor == other.fgColor && bgColor == other.bgColor && 
               bold == other.bold && blink == other.blink && inverse == other.inverse;
    }
};

struct TerminalCell {
    char32_t ch = ' ';
    TerminalAttribute attr;
};

class TerminalModel : public QObject
{
    Q_OBJECT

public:
    explicit TerminalModel(int cols, int rows, QObject* parent = nullptr);
    ~TerminalModel();

    void resize(int cols, int rows);
    void processInput(const QByteArray& data);
    
    int cols() const;
    int rows() const;
    
    // For renderer to read
    const TerminalCell& cell(int col, int row) const;
    
    // Access full history for Minimap
    int historySize() const { return m_scrollback.size(); }
    const QVector<TerminalCell>& historyLine(int index) const { return m_scrollback[index]; }
    
    void showMessage(const QString& msg);
    
    bool isAlternateScreen() const { return m_isAlternateScreen; }
    
    // View/Scroll Logic
    void scrollView(int lines);
    void resetScroll();
    int viewOffset() const { return m_viewOffset; }
    
    // Cursor Access
    int cursorX() const { return m_cursorX; }
    int cursorY() const { return m_cursorY; }
    bool isCursorVisible() const { return m_cursorVisible; }
    
    // State Accessors for Renderer/Input
    bool appCursorKeys() const { return m_appCursorKeys; }
    bool mouseTracking() const { return m_mouseTracking; } // ANY mouse mode
    bool sgrMouse() const { return m_sgrMouse; }
    bool bracketedPaste() const { return m_bracketedPaste; }
    
    // Input Generation (Qt -> VTerm)
    void sendKey(int key, int modifier); // key is VTermKey
    void sendText(const QString& text);
    void sendMouse(int button, bool pressed, int modifier);
    void sendMouseMove(int col, int row, int modifier);
    
    // VTerm Callbacks (Static wrappers)
    static int cb_damage(VTermRect rect, void *user);
    static int cb_movecursor(VTermPos pos, VTermPos oldpos, int visible, void *user);
    static int cb_sb_pushline(int cols, const VTermScreenCell *cells, void *user);
    static int cb_sb_popline(int cols, VTermScreenCell *cells, void *user);
    static int cb_bell(void *user);
    static int cb_settermprop(VTermProp prop, VTermValue *val, void *user);
    static void cb_output(const char *s, size_t len, void *user);

signals:
    void screenChanged();
    void modeChanged(bool alternate);
    void bellRing();
    void titleChanged(QString title);
    void dataOutput(QByteArray data); // Outgoing to SSH

private:
    void flushDamage(); // Sync vterm grid to our grid
    
    VTerm* m_vt = nullptr;
    VTermScreen* m_vts = nullptr;
    
    // We maintain a display grid that mirrors VTerm's screen state
    // This allows the renderer to be fast and lock-free relative to VTerm logic
    QVector<TerminalCell> m_grid; 
    
    std::deque<QVector<TerminalCell>> m_scrollback;
    int m_maxHistoryLines = 2000;
    
    int m_cols;
    int m_rows;
    int m_cursorX = 0;
    int m_cursorY = 0;
    int m_viewOffset = 0; // >0 is looking into history
    
    // State Flags (Synced from VTerm Props)
    bool m_isAlternateScreen = false;
    bool m_screenReverse = false; // Global screen reverse
    bool m_appCursorKeys = false;
    bool m_appKeypad = false;
    bool m_mouseTracking = false; 
    bool m_sgrMouse = false;
    bool m_bracketedPaste = false;
    bool m_cursorVisible = true;
};
