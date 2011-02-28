#include "emacswindow.h"

#include <iostream>
#include <algorithm>

EmacsWindow emacs_window;

#include <io.h>

#undef min
#undef max

EmacsWindowWin32::EmacsWindowWin32()
    : _overwriting(false), _ovy(0), _ovx(0),
      _curr_colour_pair(0),
      _fore(15), _back(0),
      _wsp_fore(15), _show_wsp(false),
      _ctrl_fore(11),
      _old_fore(-1), _old_back(-1),
      _botScrollPercent(0), _topScrollPercent(0) {
    hStdIn = ::GetStdHandle(STD_INPUT_HANDLE);
    if (hStdIn != INVALID_HANDLE_VALUE)
        ::SetConsoleMode(hStdIn, ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT);
    hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdOut != INVALID_HANDLE_VALUE) {
        ::SetConsoleMode(hStdOut, 0);
        _has_colours = true;
    } else {
        _has_colours = false;
    }
    clearConsole();

    // Fill out the defaults
    _decode_key[0x00] = MintString("C-@");
    for (mintchar_t i = 1; i < 32; ++i) {
        MintString key("C-");
        key.append(1, static_cast<mintchar_t>(i+'a'-1));
        _decode_key[i] = key;
    } // for
    for (mintchar_t i = 32; i < 127; ++i) {
        _decode_key[i] = MintString(1, i);
    } // for
    // Now fill in the specials
    _decode_key['\b'] = MintString("Back Space");
    _decode_key['\t'] = MintString("Tab");
    _decode_key['\n'] = MintString("Return");
    _decode_key['\r'] = MintString("Return");
    _decode_key[0x1B] = MintString("Escape");
    // ASCII keys that are inconvenient to do in ASCII
    _decode_key[','] = MintString("Comma");
    _decode_key['('] = MintString("LPar");
    _decode_key[')'] = MintString("RPar");
    // NCURSES decodes
    _decode_key[VK_DOWN     ] = MintString("Down Arrow");
    _decode_key[KEY_UP       ] = MintString("Up Arrow");
    _decode_key[KEY_LEFT     ] = MintString("Left Arrow");
    _decode_key[KEY_RIGHT    ] = MintString("Right Arrow");
    _decode_key[KEY_HOME     ] = MintString("Home");
    _decode_key[KEY_BACKSPACE] = MintString("Back Space");
    _decode_key[KEY_F(1)     ] = MintString("F1");
    _decode_key[KEY_F(2)     ] = MintString("F2");
    _decode_key[KEY_F(3)     ] = MintString("F3");
    _decode_key[KEY_F(4)     ] = MintString("F4");
    _decode_key[KEY_F(5)     ] = MintString("F5");
    _decode_key[KEY_F(6)     ] = MintString("F6");
    _decode_key[KEY_F(7)     ] = MintString("F7");
    _decode_key[KEY_F(8)     ] = MintString("F8");
    _decode_key[KEY_F(9)     ] = MintString("F9");
    _decode_key[KEY_F(10)    ] = MintString("F10");
    _decode_key[KEY_F(11)    ] = MintString("F11");
    _decode_key[KEY_F(12)    ] = MintString("F12");
    _decode_key[KEY_F(13)    ] = MintString("S-F1");
    _decode_key[KEY_F(14)    ] = MintString("S-F2");
    _decode_key[KEY_F(15)    ] = MintString("S-F3");
    _decode_key[KEY_F(16)    ] = MintString("S-F4");
    _decode_key[KEY_F(17)    ] = MintString("S-F5");
    _decode_key[KEY_F(18)    ] = MintString("S-F6");
    _decode_key[KEY_F(19)    ] = MintString("S-F7");
    _decode_key[KEY_F(20)    ] = MintString("S-F8");
    _decode_key[KEY_F(21)    ] = MintString("S-F9");
    _decode_key[KEY_F(22)    ] = MintString("S-F10");
    _decode_key[KEY_F(23)    ] = MintString("S-F11");
    _decode_key[KEY_F(24)    ] = MintString("S-F12");
    _decode_key[KEY_DC       ] = MintString("Del");
    _decode_key[KEY_IC       ] = MintString("Ins");
    _decode_key[KEY_NPAGE    ] = MintString("Pg Dn");
    _decode_key[KEY_PPAGE    ] = MintString("Pg Up");
    _decode_key[KEY_END      ] = MintString("End");
} // EmacsWindow

EmacsWindowWin32::~EmacsWindowWin32() {
} // ~EmacsWindow

mintcount_t EmacsWindowWin32::getColumns() const {
    return getConsoleColumns();
} // getColumns

mintcount_t EmacsWindowWin32::getConsoleColumns() const {
    if (hStdOut != INVALID_HANDLE_VALUE) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (::GetConsoleScreenBufferInfo(hStdOut, &csbi))
            return csbi.dwSize.X;
    } // if
    return 80;
} // getConsoleColumns

mintcount_t EmacsWindowWin32::getLines() const {
    return getConsoleLines() - 3;
} // getLines

mintcount_t EmacsWindowWin32::getConsoleLines() const {
    if (hStdOut != INVALID_HANDLE_VALUE) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (::GetConsoleScreenBufferInfo(hStdOut, &csbi))
            return csbi.dwSize.Y - 3;
    } // if
    return 24;
} // getConsoleLines

void EmacsWindow::writeLine(const EmacsBuffer& buf, mintcount_t bol, mintcount_t eol) {
    EmacsBuffer::const_iterator bolp = buf.begin() + bol;
    EmacsBuffer::const_iterator eolp = buf.begin() + eol;
    // Find the last non-space character
    EmacsBuffer::const_iterator nwsp = eolp;
    while (bolp != nwsp) {
        mintchar_t ch = *--nwsp;
        if (ch != '\t' && ch != ' ') {
            break;
        } // if
    } // while
    mintcount_t cur_col = 0;
    mintcount_t leftcol = buf.getLeftColumn();
    while ((cur_col < leftcol) && (bolp != eolp)) {
        cur_col += buf.charWidth(cur_col, *bolp++);
    } // while
    const mintcount_t COLS = getConsoleColumns();
    while ((cur_col < leftcol+COLS) && (bolp != eolp)) {
        umintchar_t ch = *bolp++;
        if (ch == 0x09) {
            mintcount_t tabw = buf.charWidth(cur_col, ch);
            tabw = std::min(tabw, leftcol + COLS - cur_col);
            setConsoleAttributes(_fore, _back);
            chtype ch = (_show_wsp && (bolp > nwsp)) ? ACS_BULLET : ' ';
            for (mintcount_t i = 0; i < tabw; ++i) {
                writeConsoleChar(ch);
            } // for
            cur_col += tabw;
        } else if (ch < 0x20) {
            setConsoleAttributes(_ctrl_fore, _back);
            writeConsoleChar(ch + '@'));
            ++cur_col;
        } else if (ch == 0x20) {
            setConsoleAttributes(_fore, _back);
            writeConsoleChar((_show_wsp && (bolp > nwsp)) ? ACS_BULLET : ' ');
            ++cur_col;
        } else {
            setConsoleAttributes(_fore, _back);
            writeConsoleChar(ch);
            ++cur_col;
        } // else
    } // while
    setConsoleAttributes(_fore, _back);
    while (cur_col++ < leftcol+COLS) {
        writeConsoleChar(' ');
    } // if
} // writeLine

void EmacsWindow::redisplay(EmacsBuffer& buf, bool force) {
    if (hStdOut != INVALID_HANDLE_VALUE) {
        _overwriting = false;
        if (force) {
            // Does nothing here
        } // if
        const mintsize_t COLS = getConsoleColumns();
        const mintsize_t LINES = getConsoleLines();
        buf.forcePointInWindow(LINES - 2, COLS, _topScrollPercent, _botScrollPercent);
        mintcount_t curline = buf.getMarkPosition(EmacsBuffer::MARK_TOPLINE);
        mintcount_t screen_line = buf.countNewlines(curline, buf.getMarkPosition(EmacsBuffer::MARK_POINT));
        mintcount_t screen_col = buf.getColumn() - buf.getLeftColumn();
        for (int i = 0; i < LINES - 2; ++i) {
            moveConsoleCursor(i, 0);
            mintcount_t eol = buf.getMarkPosition(EmacsBuffer::MARK_EOL, curline);
            writeLine(buf, curline, eol);
            curline = buf.getMarkPosition(EmacsBuffer::MARK_NEXT_CHAR, eol);
        } // for
        moveConsoleCursor(screen_line, screen_col);
    } // if
} // EmacsWindow::redisplay

void EmacsWindow::overwrite(const MintString& str) {
    if (hStdOut != INVALID_HANDLE_VALUE) {
        if (!_overwriting) {
            _overwriting = true;
            _ovy = _ovx = 0;
        } // if
        setConsoleAttributes(_fore, _back);
        moveConsoleCursor(_ovy, _ovx);
        // FIXME: Control codes and high chars are written incorrectly
        std::for_each(str.begin(), str.end(), std::mem_fun(writeConsoleChar));
        getConsoleCursorPos(&_ovy, &_ovx);
    } else {
        std::cout << str;
    } // else
} // overwrite

void EmacsWindow::gotoxy(int x, int y) {
    if (hStdOut != INVALID_HANDLE_VALUE) {
        if (!_overwriting) {
            _overwriting = true;
        } // if
        _ovy = std::max(0, std::min(y, getConsoleLines()));
        _ovx = std::max(0, std::min(x, getConsoleColumns()));
        moveConsoleCursor(_ovy, _ovx);
    } // if
} // overwrite

bool EmacsWindow::keyWaiting() {
    if (_w(_win)) {
#ifndef XCURSES
        nodelay(_w(_win), true);
        int ch = getch();
        if (ch != ERR) {
            ungetch(ch);
            return true;
        } // if
#endif
    } // if
    return false;
} // keyWaiting

MintString EmacsWindow::getInput(mintcount_t millisec) {
    if (_w(_win)) {
        if (millisec < 10) {
            nodelay(_w(_win), true);
        } else {
            nodelay(_w(_win), false);
            wtimeout(_w(_win), millisec);
        } // else
        // FIXME: wrefresh(_w(_win));
        int ch = getch();
        if (ch == ERR) {
            return MintString("Timeout");
        } else {
            std::map<int,MintString>::const_iterator i = _decode_key.find(ch);
            if (i == _decode_key.end()) {
                return MintString("Unknown");
            } else {
                return i->second;
            } // else
        } // if
    } else {
        if (millisec > 0) {
            char ch;

            std::cin >> ch;
            return MintString(1, ch);
        } else {
            return MintString("Timeout");
        } // else
    } // else
} // getInput

void EmacsWindow::announce(const MintString& left, const MintString& right) {
    if (_w(_win)) {
        int n = std::min(left.size(), static_cast<size_t>(COLS - 1));
        setCursesAttributes(_fore, _back);
        wmove(_w(_win), LINES - 1, 0);
        std::for_each(left.begin(), left.begin() + n, 
                      std::bind1st(std::ptr_fun(waddch), _w(_win)));
        int y, x;
        getyx(_w(_win), y, x);
        int m = std::min(right.size(), static_cast<size_t>(COLS - (n + 1)));
        std::for_each(right.begin(), right.begin() + m,
                      std::bind1st(std::ptr_fun(waddch), _w(_win)));
        if ((n + m) < COLS) {
            wclrtoeol(_w(_win));
        } // if
        wmove(_w(_win), y, x);
        refresh();
    } else {
        std::cout << left << right << std::endl;
    } // else
} // EmacsWindow::announce

void EmacsWindow::announceWin(const MintString& left, const MintString& right) {
    if (_w(_win)) {
        int n = std::min(left.size(), static_cast<size_t>(COLS - 1));
        setCursesAttributes(_fore, _back);
        int y, x;
        getyx(_w(_win), y, x);
        wmove(_w(_win), LINES - 2, 0);
        std::for_each(left.begin(), left.begin() + n, 
                      std::bind1st(std::ptr_fun(waddch), _w(_win)));
        int m = std::min(right.size(), static_cast<size_t>(COLS - n));
        std::for_each(right.begin(), right.begin() + m,
                      std::bind1st(std::ptr_fun(waddch), _w(_win)));
        if ((n + m) < COLS) {
            wclrtoeol(_w(_win));
        } // if
        wmove(_w(_win), y, x);
        refresh();
    } // if
} // EmacsWindow::announceWin

void EmacsWindow::audibleBell(mintcount_t /*freq*/, mintcount_t /*millisec*/) {
    if (_w(_win)) {
        beep();
    } else {
        std::cout << '\007';
    } // else
} // EmacsWindow::audibleBell

void EmacsWindow::visualBell(mintcount_t /*millisec*/) {
    if (_w(_win)) {
        flash();
    } // if
} // EmacsWindow::visualBell

namespace {
    const int colourXlat[8] = {
        COLOR_BLACK,  COLOR_BLUE,
        COLOR_GREEN,  COLOR_CYAN,
        COLOR_RED,    COLOR_MAGENTA,
        COLOR_YELLOW, COLOR_WHITE
    };
    inline int cursesColour(int colour) {
        return colourXlat[colour & 0x07];
    } // cursesColour
    inline int cursesBold(int colour) {
        return (colour & 0x08) ? A_BOLD : A_NORMAL;
    } // cursesColour
}; // namespace

void EmacsWindow::setCursesAttributes(int fo, int ba) {
    if (_has_colours) {
        if ((fo != _old_fore) || (ba != _old_back)) {
            _old_fore = fo;
            _old_back = ba;
            int forecolour = cursesColour(fo);
            int forebold = cursesBold(fo);
            int backcolour = cursesColour(ba);
            short use_pair = COLOR_PAIRS;
            for (short i = 0; i < COLOR_PAIRS; ++i) {
                short f, b;
                if ((ERR != pair_content(i, &f, &b)) && (f == forecolour) && (b == backcolour)) {
                    use_pair = i;
                    break;
                } // if
            } // for
            if (use_pair >= COLOR_PAIRS) {
                if (++_curr_colour_pair >= COLOR_PAIRS) {
                    _curr_colour_pair = 1;
                } // if
                use_pair = _curr_colour_pair;
                init_pair(use_pair, forecolour, backcolour);
            } // if
            wattrset(_w(_win), COLOR_PAIR(use_pair) | forebold);
            // FIXME: I know this is not the preferred way, but how else?
            wbkgdset(_w(_win), COLOR_PAIR(use_pair) | forebold | ' ');
        } // if
    } // if
} // EmacsWindow::setCursesAttributes

// EOF
