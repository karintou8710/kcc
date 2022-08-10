#ifndef CURSES_H
#define CURSES_H

typedef unsigned int chtype;
typedef unsigned int mmask_t;
typedef struct screen SCREEN;
typedef struct _win_st WINDOW;
typedef chtype attr_t;

struct _win_st {
    short _cury;
    short _curx;

    short _maxy;
    short _maxx;
    short _begy;
    short _begx;

    short _flags;

    attr_t _attrs;
    chtype _bkgd;

    _Bool _notimeout;
    _Bool _clear;
    _Bool _leaveok;
    _Bool _scroll;
    _Bool _idlok;
    _Bool _idcok;
    _Bool _immed;
    _Bool _sync;
    _Bool _use_keypad;
    int _delay;

    struct ldat *_line;

    short _regtop;
    short _regbottom;

    int _parx;
    int _pary;
    WINDOW *_parent;

    struct pdat {
        short _pad_y;
        short _pad_x;
        short _pad_top;
        short _pad_left;
        short _pad_bottom;
        short _pad_right;
    } _pad;

    short _yoffset;
};

extern WINDOW *curscr;
extern WINDOW *newscr;
extern WINDOW *stdscr;

int TRUE = 1;
int FALSE = 0;
extern int COLORS;
extern int COLOR_PAIRS;
extern int COLS;
extern int ESCDELAY;
extern int LINES;
extern int TABSIZE;

extern int mvaddch(int, int, const chtype);
extern WINDOW *initscr(void);
extern int noecho(void);
extern int curs_set_sp(SCREEN *, int);
extern int curs_set(int);
extern int nodelay(WINDOW *, _Bool);
extern int leaveok(WINDOW *, _Bool);
extern int scrollok(WINDOW *, _Bool);
extern int getch(void);
extern int refresh(void);
extern int mvcur(int, int, int, int);
extern int endwin(void);

#endif