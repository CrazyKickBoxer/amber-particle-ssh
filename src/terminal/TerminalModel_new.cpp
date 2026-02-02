#include "TerminalModel.h"
#include <QDebug>
#include <vector>

TerminalModel::TerminalModel(int cols, int rows, QObject* parent)
    : QObject(parent)
    , m_state(State::Normal)
    , m_utf8State(0)
    , m_cols(cols)
    , m_rows(rows)
    , m_cursorX(0)
    , m_cursorY(0)
{
    resize(cols, rows);
    
    // Initial text
    QString welcome = "Amber SSH v2.0 - GTX 1080 Ti Edition";
    for(QChar c : welcome) {
        putChar(c.unicode());
    }
    putChar('\n');
    QString sub = "Ready to connect...";
    for(QChar c : sub) {
        putChar(c.unicode());
    }
    putChar('\n');
}

void TerminalModel::resize(int cols, int rows)
{
    m_cols = cols;
    m_rows = rows;
    m_grid.resize(cols * rows);
    // Fill with empty
    for(auto& cell : m_grid) {
        cell.ch = ' ';
        cell.attr = m_currentAttr;
    }
    
    if (m_cursorX >= m_cols) m_cursorX = m_cols - 1;
    if (m_cursorY >= m_rows) m_cursorY = m_rows - 1;
}

const TerminalCell& TerminalModel::cell(int col, int row) const
{
    if (col < 0 || col >= m_cols || row < 0 || row >= m_rows) {
        static TerminalCell empty;
        return empty;
    }
    return m_grid[row * m_cols + col];
}

void TerminalModel::processInput(const QByteArray& data)
{
    for(uint8_t byte : data) {
        char c = (char)byte;
        
        // UTF-8 Decoder
        if (m_utf8State == 0) {
            if ((byte & 0x80) == 0) {
                // ASCII
                if (m_state == State::Normal) {
                    // Logic below
                } else {
                     handleEscapeSequence(c);
                     continue;
                }
                
                if (c == 0x1B) {
                    m_state = State::Escape;
                } else if (c < 32) {
                    handleControlChar(c);
                } else {
                    putChar(c);
                }
            } else if ((byte & 0xE0) == 0xC0) {
                m_utf8State = 1;
                m_utf8Codepoint = byte & 0x1F;
            } else if ((byte & 0xF0) == 0xE0) {
                m_utf8State = 2;
                m_utf8Codepoint = byte & 0x0F;
            } else if ((byte & 0xF8) == 0xF0) {
                m_utf8State = 3;
                m_utf8Codepoint = byte & 0x07;
            }
        } else {
            if ((byte & 0xC0) == 0x80) {
                m_utf8Codepoint = (m_utf8Codepoint << 6) | (byte & 0x3F);
                m_utf8State--;
                if (m_utf8State == 0) {
                    putChar(m_utf8Codepoint);
                }
            } else {
                // Invalid sequence
                m_utf8State = 0;
            }
        }
    }
    emit screenChanged();
}

void TerminalModel::handleControlChar(char32_t c)
{
    switch(c) {
        case '\r': m_cursorX = 0; break;
        case '\n': 
            m_cursorY++;
            if (m_cursorY >= m_rows) {
                // Scroll
                m_cursorY = m_rows - 1;
                // Move rows up
                int rowSize = m_cols;
                for (int i = 0; i < (m_rows - 1) * rowSize; ++i) {
                     m_grid[i] = m_grid[i + rowSize];
                }
                // Clear last row
                for (int i = 0; i < m_cols; ++i) {
                    m_grid[(m_rows - 1) * rowSize + i].ch = ' ';
                    m_grid[(m_rows - 1) * rowSize + i].attr = m_currentAttr;
                }
            }
            break;
        case '\b': if(m_cursorX > 0) m_cursorX--; break;
        case '\t': m_cursorX = (m_cursorX + 8) & ~7; if (m_cursorX >= m_cols) m_cursorX=m_cols-1; break;
        case 0x07: // Bell - ignore
            break;
    }
}

void TerminalModel::handleEscapeSequence(char c)
{
    if (m_state == State::Escape) {
        if (c == '[') {
            m_state = State::CSI;
            m_params.clear();
        } else if (c == ']') {
            m_state = State::OSC;
            m_oscBuffer.clear();
        } else if (c == '(' || c == ')') {
            // G0/G1 charset - Valid immediate end of escape for charset
            m_state = State::Normal; 
        } else {
            m_state = State::Normal; // Valid 2-char escape?
        }
    } else if (m_state == State::CSI) {
        if (c >= '0' && c <= '9') {
            if (m_params.isEmpty()) m_params.push_back(0);
            m_params.last() = m_params.last() * 10 + (c - '0');
        } else if (c == ';') {
            m_params.push_back(0);
        } else if (c >= 0x40 && c <= 0x7E) {
            handleCSI(c);
            m_state = State::Normal;
        }
    } else if (m_state == State::OSC) {
        if (c == 0x07 || c == 0x00) { // BEL or NUL terminates OSC
             m_state = State::Normal;
        } else {
            m_oscBuffer.append(c);
        }
    }
}

void TerminalModel::handleCSI(char c)
{
    int p1 = m_params.value(0, 1); // Default to 1 for most
    int p2 = m_params.value(1, 1);
    
    switch(c) {
        case 'A': // Cursor Up
            m_cursorY = std::max(0, m_cursorY - p1);
            break;
        case 'B': // Cursor Down
            m_cursorY = std::min(m_rows - 1, m_cursorY + p1);
            break;
        case 'C': // Cursor Forward
            m_cursorX = std::min(m_cols - 1, m_cursorX + p1);
            break;
        case 'D': // Cursor Back
            m_cursorX = std::max(0, m_cursorX - p1);
            break;
        case 'H': // Cursor Position
        case 'f':
            if (m_params.size() < 2) p2 = 1; // Default to 1,1
             // ANSI is 1-based
            m_cursorY = std::max(0, std::min(m_rows - 1, p1 - 1));
            m_cursorX = std::max(0, std::min(m_cols - 1, p2 - 1));
            break;
        case 'J': // Erase in Display
            {
                int mode = m_params.value(0, 0);
                if (mode == 2) { // Clear all
                    for(auto& cell : m_grid) { cell.ch = ' '; cell.attr = m_currentAttr; }
                    m_cursorX = 0; m_cursorY = 0;
                } else if (mode == 0) { // Clear cursor to end
                     // Logic omitted for brevity, good enough for now
                }
            }
            break;
        case 'K': // Erase in Line
             // 0: cursor to end
            if (m_params.value(0, 0) == 0) {
                 for(int x = m_cursorX; x < m_cols; ++x) {
                     m_grid[m_cursorY * m_cols + x].ch = ' ';
                     m_grid[m_cursorY * m_cols + x].attr = m_currentAttr;
                 }
            }
            break;
        case 'm': // SGR - Select Graphic Rendition
            if (m_params.isEmpty()) {
                m_currentAttr = TerminalAttribute(); // Reset
            } else {
                for(int p : m_params) {
                    if (p == 0) m_currentAttr = TerminalAttribute();
                    else if (p == 1) m_currentAttr.bold = true;
                    else if (p == 5) m_currentAttr.blink = true;
                    else if (p >= 30 && p <= 37) m_currentAttr.fgColor = p - 30;
                    else if (p >= 40 && p <= 47) m_currentAttr.bgColor = p - 40;
                    else if (p >= 90 && p <= 97) m_currentAttr.fgColor = p - 90 + 8; // Bright
                }
            }
            break;
    }
}

void TerminalModel::putChar(char32_t c)
{
    if (m_cursorX >= m_cols) {
        m_cursorX = 0;
        handleControlChar('\n');
    }
    
    m_grid[m_cursorY * m_cols + m_cursorX].ch = c;
    m_grid[m_cursorY * m_cols + m_cursorX].attr = m_currentAttr;
    m_cursorX++;
}

void TerminalModel::showMessage(const QString& msg)
{
    // Write message at current cursor
    handleControlChar('\n');
    // Convert to Utf8 to reuse processInput if needed, or iterate
    for(QChar c : msg) {
        if(c == '\n') handleControlChar('\n');
        else putChar(c.unicode());
    }
    emit screenChanged();
}
