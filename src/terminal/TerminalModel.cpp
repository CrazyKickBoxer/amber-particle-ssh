#include "TerminalModel.h"
#include <QDebug>
#include <vector>
#include <algorithm>

// Static wrappers
int TerminalModel::cb_damage(VTermRect rect, void *user) {
    TerminalModel *self = static_cast<TerminalModel*>(user);
    if(self) {
        int cellsCopied = 0;
        // Copy cells from vterm to m_grid for this rect
        for(int row = rect.start_row; row < rect.end_row; ++row) {
            for(int col = rect.start_col; col < rect.end_col; ++col) {
                VTermScreenCell vcell;
                VTermPos pos = {row, col};
                vterm_screen_get_cell(self->m_vts, pos, &vcell);
                
                int gridIdx = row * self->m_cols + col;
                if(gridIdx >= 0 && gridIdx < self->m_grid.size()) {
                    TerminalCell& myCell = self->m_grid[gridIdx];
                    
                    // Char
                    if(vcell.chars[0]) {
                        myCell.ch = vcell.chars[0];
                        cellsCopied++;
                    }
                    else myCell.ch = ' ';
                    
                    // Attrs
                    myCell.attr.bold = vcell.attrs.bold;
                    myCell.attr.underline = vcell.attrs.underline;
                    myCell.attr.italic = vcell.attrs.italic;
                    myCell.attr.blink = vcell.attrs.blink;
                    myCell.attr.blink = vcell.attrs.blink;
                    // XOR with global screen reverse
                    myCell.attr.inverse = vcell.attrs.reverse ^ self->m_screenReverse;
                    
                    // Colors
                    // FG
                    if (VTERM_COLOR_IS_RGB(&vcell.fg)) {
                        myCell.attr.fgTrueColor = true;
                        myCell.attr.fgR = vcell.fg.rgb.red;
                        myCell.attr.fgG = vcell.fg.rgb.green;
                        myCell.attr.fgB = vcell.fg.rgb.blue;
                        myCell.attr.fgColor = 7; 
                    } else if (VTERM_COLOR_IS_INDEXED(&vcell.fg)) {
                        myCell.attr.fgTrueColor = false;
                        myCell.attr.fgColor = vcell.fg.indexed.idx;
                    } else {
                        myCell.attr.fgTrueColor = false;
                        myCell.attr.fgColor = 7;
                    }
                    
                    // BG
                    if (VTERM_COLOR_IS_RGB(&vcell.bg)) {
                        myCell.attr.bgTrueColor = true;
                        myCell.attr.bgR = vcell.bg.rgb.red;
                        myCell.attr.bgG = vcell.bg.rgb.green;
                        myCell.attr.bgB = vcell.bg.rgb.blue;
                        myCell.attr.bgColor = 0;
                    } else if (VTERM_COLOR_IS_INDEXED(&vcell.bg)) {
                        myCell.attr.bgTrueColor = false;
                        myCell.attr.bgColor = vcell.bg.indexed.idx;
                    } else {
                        myCell.attr.bgTrueColor = false;
                        myCell.attr.bgColor = 0;
                    }
                }
            }
        }
        
        emit self->screenChanged();
    }
    return 1;
}

int TerminalModel::cb_movecursor(VTermPos pos, VTermPos oldpos, int visible, void *user) {
    TerminalModel *self = static_cast<TerminalModel*>(user);
    if(self) {
        self->m_cursorX = pos.col;
        self->m_cursorY = pos.row;
        self->m_cursorVisible = visible;
        // emit self->screenChanged(); // Cursor move might just need paint update
    }
    return 1;
}

int TerminalModel::cb_sb_pushline(int cols, const VTermScreenCell *cells, void *user) {
    TerminalModel *self = static_cast<TerminalModel*>(user);
    if(self) {
        QVector<TerminalCell> savedRow;
        savedRow.resize(cols);
        for(int i=0; i<cols; ++i) {
            // Copy minimal data for scrollback
            if(cells[i].chars[0]) savedRow[i].ch = cells[i].chars[0];
            else savedRow[i].ch = ' ';
            
            // Attrs logic (Simplified copy)
            if (VTERM_COLOR_IS_INDEXED(&cells[i].fg)) savedRow[i].attr.fgColor = cells[i].fg.indexed.idx;
            if (VTERM_COLOR_IS_INDEXED(&cells[i].bg)) savedRow[i].attr.bgColor = cells[i].bg.indexed.idx;
            savedRow[i].attr.bold = cells[i].attrs.bold;
        }
        
        self->m_scrollback.push_back(savedRow);
        if(self->m_scrollback.size() > self->m_maxHistoryLines) {
            self->m_scrollback.pop_front();
        }
    }
    return 1;
}

int TerminalModel::cb_sb_popline(int cols, VTermScreenCell *cells, void *user) {
    TerminalModel *self = static_cast<TerminalModel*>(user);
    if(!self || self->m_scrollback.empty()) {
        return 0; // No lines to pop
    }
    
    const QVector<TerminalCell>& savedRow = self->m_scrollback.back();
    
    for(int i = 0; i < cols; ++i) {
        memset(&cells[i], 0, sizeof(VTermScreenCell));
        
        if(i < savedRow.size()) {
            cells[i].chars[0] = savedRow[i].ch;
            cells[i].width = 1;
            
            // Restore color as indexed
            cells[i].fg.type = VTERM_COLOR_INDEXED;
            cells[i].fg.indexed.idx = savedRow[i].attr.fgColor;
            cells[i].bg.type = VTERM_COLOR_INDEXED;
            cells[i].bg.indexed.idx = savedRow[i].attr.bgColor;
            cells[i].attrs.bold = savedRow[i].attr.bold;
        } else {
            cells[i].chars[0] = ' ';
            cells[i].width = 1;
            cells[i].fg.type = VTERM_COLOR_INDEXED;
            cells[i].fg.indexed.idx = 7;
            cells[i].bg.type = VTERM_COLOR_INDEXED;
            cells[i].bg.indexed.idx = 0;
        }
    }
    
    self->m_scrollback.pop_back();
    return 1;
}

int TerminalModel::cb_bell(void *user) {
    TerminalModel *self = static_cast<TerminalModel*>(user);
    if(self) emit self->bellRing();
    return 1;
}

int TerminalModel::cb_settermprop(VTermProp prop, VTermValue *val, void *user) {
    TerminalModel *self = static_cast<TerminalModel*>(user);
    if(!self) return 0;
    
    switch(prop) {
        case VTERM_PROP_CURSORVISIBLE:
            self->m_cursorVisible = val->boolean;
            break;
        case VTERM_PROP_TITLE:
            emit self->titleChanged(QString::fromUtf8(val->string.str, val->string.len));
            break;
        case VTERM_PROP_ICONNAME:
            break;
        case VTERM_PROP_REVERSE:
            self->m_screenReverse = val->boolean;
            {
               // Force full redraw (damage all)
               VTermRect rect = {0, self->m_rows, 0, self->m_cols};
               cb_damage(rect, self);
            }
            break;
        case VTERM_PROP_CURSORSHAPE:
            break;
        case VTERM_PROP_MOUSE:
            self->m_mouseTracking = (val->number != 0); // VTERM_PROP_MOUSE_NONE = 0
            if (val->number == VTERM_PROP_MOUSE_CLICK) {
               // self->m_mouseMotion = false;
            } else if (val->number == VTERM_PROP_MOUSE_DRAG) {
               // self->m_mouseMotion = true;
            } else if (val->number == VTERM_PROP_MOUSE_MOVE) {
               // motion
            }
            break;
        case VTERM_PROP_ALTSCREEN:
            self->m_isAlternateScreen = val->boolean;
            emit self->modeChanged(self->m_isAlternateScreen);
            break;
        default: break;
    }
    return 1;
}

// ... (existing callbacks above)

void TerminalModel::cb_output(const char *s, size_t len, void *user) {
    TerminalModel *self = static_cast<TerminalModel*>(user);
    if(self) {
        emit self->dataOutput(QByteArray(s, len));
    }
}

static VTermScreenCallbacks vterm_callbacks = {
    .damage = TerminalModel::cb_damage,
    .moverect = nullptr,
    .movecursor = TerminalModel::cb_movecursor,
    .settermprop = TerminalModel::cb_settermprop,
    .bell = TerminalModel::cb_bell,
    .resize = nullptr, 
    .sb_pushline = TerminalModel::cb_sb_pushline,
    .sb_popline = TerminalModel::cb_sb_popline,
};

TerminalModel::TerminalModel(int cols, int rows, QObject* parent)
    : QObject(parent)
    , m_cols(cols)
    , m_rows(rows)
{
    m_vt = vterm_new(rows, cols);
    vterm_set_utf8(m_vt, 1);
    
    // Register Output Callback (for keyboard -> ssh)
    vterm_output_set_callback(m_vt, TerminalModel::cb_output, this);
    
    m_vts = vterm_obtain_screen(m_vt);
    vterm_screen_enable_altscreen(m_vts, 1);
    vterm_screen_set_callbacks(m_vts, &vterm_callbacks, this);
    
    // Reset state
    vterm_screen_reset(m_vts, 1); // 1=hard reset
    
    m_grid.resize(cols * rows);
}

TerminalModel::~TerminalModel() {
    if(m_vt) vterm_free(m_vt);
}

void TerminalModel::resize(int cols, int rows) {
    if(cols != m_cols || rows != m_rows) {
        m_cols = cols;
        m_rows = rows;
        m_grid.resize(cols * rows);
        vterm_set_size(m_vt, rows, cols);
        
        // Force full sync because resize might not trigger damage for everything
        // or we reshaped the grid and need to refill it.
        VTermRect rect = {0, rows, 0, cols};
        cb_damage(rect, this); // Reuse damage callback to pull all cells
        
        // Also flush? vterm doesn't enforce damage on resize logic strictly for all cells sometimes
        // Manual refresh:
        emit screenChanged();
    }
}

void TerminalModel::processInput(const QByteArray& data) {
    qDebug() << "📥 processInput:" << data.size() << "bytes";
    vterm_input_write(m_vt, data.constData(), data.size());
    // Flush damage to ensure all screen changes fire callbacks
    vterm_screen_flush_damage(m_vts);
}

void TerminalModel::sendKey(int key, int modifier) {
    vterm_keyboard_key(m_vt, static_cast<VTermKey>(key), static_cast<VTermModifier>(modifier));
}

void TerminalModel::sendText(const QString& text) {
    for(QChar c : text) {
        char32_t u = c.unicode(); 
        vterm_keyboard_unichar(m_vt, u, VTERM_MOD_NONE);
    }
}

void TerminalModel::sendMouse(int button, bool pressed, int modifier) {
    vterm_mouse_button(m_vt, button, pressed, static_cast<VTermModifier>(modifier));
}

void TerminalModel::sendMouseMove(int col, int row, int modifier) {
    vterm_mouse_move(m_vt, row, col, static_cast<VTermModifier>(modifier)); 
}

int TerminalModel::cols() const { return m_cols; }
int TerminalModel::rows() const { return m_rows; }

const TerminalCell& TerminalModel::cell(int col, int row) const {
     if (col < 0 || col >= m_cols || row < 0 || row >= m_rows) {
        static TerminalCell empty;
        return empty;
    }
    
    // If not scrolling, fast path
    if (m_viewOffset == 0) {
        return m_grid[row * m_cols + col];
    }
    
    // Virtual Row Logic for Scrollback
    int logicalRow = row - m_viewOffset;
    if (logicalRow >= 0) {
        return m_grid[logicalRow * m_cols + col];
    } else {
        int histIndex = (int)m_scrollback.size() + logicalRow;
        if (histIndex >= 0 && histIndex < (int)m_scrollback.size()) {
            const auto& line = m_scrollback[histIndex];
            if (col < line.size()) {
                return line[col];
            }
        }
        static TerminalCell empty;
        return empty;
    }
}

void TerminalModel::showMessage(const QString& msg) {
    QByteArray u8 = msg.toUtf8();
    vterm_input_write(m_vt, u8.constData(), u8.size());
    
    // Force newline?
    char nl = '\n';
    vterm_input_write(m_vt, &nl, 1);
}

void TerminalModel::scrollView(int lines) {
    if (m_isAlternateScreen) return;
    
    m_viewOffset += lines;
    if (m_viewOffset < 0) m_viewOffset = 0;
    int maxScroll = (int)m_scrollback.size();
    if (m_viewOffset > maxScroll) m_viewOffset = maxScroll;
    
    emit screenChanged();
}

void TerminalModel::resetScroll() {
    if (m_viewOffset != 0) {
        m_viewOffset = 0;
        emit screenChanged();
    }
}
