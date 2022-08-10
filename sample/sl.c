/*========================================
 *    sl.h: SL version 5.02
 *	Copyright 1993,2002,2014
 *                Toyoda Masashi
 *		  (mtoyoda@acm.org)
 *	Last Modified: 2014/06/03
 *========================================
 */

/* -lncursesを加えてコンパイルする */

#define D51HEIGHT 10
#define D51FUNNEL 7
#define D51LENGTH 83
#define D51PATTERNS 6

#define D51STR1 "      ====        ________                ___________ "
#define D51STR2 "  _D _|  |_______/        \\__I_I_____===__|_________| "
#define D51STR3 "   |(_)---  |   H\\________/ |   |        =|___ ___|   "
#define D51STR4 "   /     |  |   H  |  |     |   |         ||_| |_||   "
#define D51STR5 "  |      |  |   H  |__--------------------| [___] |   "
#define D51STR6 "  | ________|___H__/__|_____/[][]~\\_______|       |   "
#define D51STR7 "  |/ |   |-----------I_____I [][] []  D   |=======|__ "

#define D51WHL11 "__/ =| o |=-~~\\  /~~\\  /~~\\  /~~\\ ____Y___________|__ "
#define D51WHL12 " |/-=|___|=    ||    ||    ||    |_____/~\\___/        "
#define D51WHL13 "  \\_/      \\O=====O=====O=====O_/      \\_/            "

#define D51WHL21 "__/ =| o |=-~~\\  /~~\\  /~~\\  /~~\\ ____Y___________|__ "
#define D51WHL22 " |/-=|___|=O=====O=====O=====O   |_____/~\\___/        "
#define D51WHL23 "  \\_/      \\__/  \\__/  \\__/  \\__/      \\_/            "

#define D51WHL31 "__/ =| o |=-O=====O=====O=====O \\ ____Y___________|__ "
#define D51WHL32 " |/-=|___|=    ||    ||    ||    |_____/~\\___/        "
#define D51WHL33 "  \\_/      \\__/  \\__/  \\__/  \\__/      \\_/            "

#define D51WHL41 "__/ =| o |=-~O=====O=====O=====O\\ ____Y___________|__ "
#define D51WHL42 " |/-=|___|=    ||    ||    ||    |_____/~\\___/        "
#define D51WHL43 "  \\_/      \\__/  \\__/  \\__/  \\__/      \\_/            "

#define D51WHL51 "__/ =| o |=-~~\\  /~~\\  /~~\\  /~~\\ ____Y___________|__ "
#define D51WHL52 " |/-=|___|=   O=====O=====O=====O|_____/~\\___/        "
#define D51WHL53 "  \\_/      \\__/  \\__/  \\__/  \\__/      \\_/            "

#define D51WHL61 "__/ =| o |=-~~\\  /~~\\  /~~\\  /~~\\ ____Y___________|__ "
#define D51WHL62 " |/-=|___|=    ||    ||    ||    |_____/~\\___/        "
#define D51WHL63 "  \\_/      \\_O=====O=====O=====O/      \\_/            "

#define D51DEL "                                                      "

#define COAL01 "                              "
#define COAL02 "                              "
#define COAL03 "    _________________         "
#define COAL04 "   _|                \\_____A  "
#define COAL05 " =|                        |  "
#define COAL06 " -|                        |  "
#define COAL07 "__|________________________|_ "
#define COAL08 "|__________________________|_ "
#define COAL09 "   |_D__D__D_|  |_D__D__D_|   "
#define COAL10 "    \\_/   \\_/    \\_/   \\_/    "

#define COALDEL "                              "

#define LOGOHEIGHT 6
#define LOGOFUNNEL 4
#define LOGOLENGTH 84
#define LOGOPATTERNS 6

#define LOGO1 "     ++      +------ "
#define LOGO2 "     ||      |+-+ |  "
#define LOGO3 "   /---------|| | |  "
#define LOGO4 "  + ========  +-+ |  "

#define LWHL11 " _|--O========O~\\-+  "
#define LWHL12 "//// \\_/      \\_/    "

#define LWHL21 " _|--/O========O\\-+  "
#define LWHL22 "//// \\_/      \\_/    "

#define LWHL31 " _|--/~O========O-+  "
#define LWHL32 "//// \\_/      \\_/    "

#define LWHL41 " _|--/~\\------/~\\-+  "
#define LWHL42 "//// \\_O========O    "

#define LWHL51 " _|--/~\\------/~\\-+  "
#define LWHL52 "//// \\O========O/    "

#define LWHL61 " _|--/~\\------/~\\-+  "
#define LWHL62 "//// O========O_/    "

#define LCOAL1 "____                 "
#define LCOAL2 "|   \\@@@@@@@@@@@     "
#define LCOAL3 "|    \\@@@@@@@@@@@@@_ "
#define LCOAL4 "|                  | "
#define LCOAL5 "|__________________| "
#define LCOAL6 "   (O)       (O)     "

#define LCAR1 "____________________ "
#define LCAR2 "|  ___ ___ ___ ___ | "
#define LCAR3 "|  |_| |_| |_| |_| | "
#define LCAR4 "|__________________| "
#define LCAR5 "|__________________| "
#define LCAR6 "   (O)        (O)    "

#define DELLN "                     "

#define C51HEIGHT 11
#define C51FUNNEL 7
#define C51LENGTH 87
#define C51PATTERNS 6

#define C51DEL "                                                       "

#define C51STR1 "        ___                                            "
#define C51STR2 "       _|_|_  _     __       __             ___________"
#define C51STR3 "    D__/   \\_(_)___|  |__H__|  |_____I_Ii_()|_________|"
#define C51STR4 "     | `---'   |:: `--'  H  `--'         |  |___ ___|  "
#define C51STR5 "    +|~~~~~~~~++::~~~~~~~H~~+=====+~~~~~~|~~||_| |_||  "
#define C51STR6 "    ||        | ::       H  +=====+      |  |::  ...|  "
#define C51STR7 "|    | _______|_::-----------------[][]-----|       |  "

#define C51WH61 "| /~~ ||   |-----/~~~~\\  /[I_____I][][] --|||_______|__"
#define C51WH62 "------'|oOo|==[]=-     ||      ||      |  ||=======_|__"
#define C51WH63 "/~\\____|___|/~\\_|   O=======O=======O  |__|+-/~\\_|     "
#define C51WH64 "\\_/         \\_/  \\____/  \\____/  \\____/      \\_/       "

#define C51WH51 "| /~~ ||   |-----/~~~~\\  /[I_____I][][] --|||_______|__"
#define C51WH52 "------'|oOo|===[]=-    ||      ||      |  ||=======_|__"
#define C51WH53 "/~\\____|___|/~\\_|    O=======O=======O |__|+-/~\\_|     "
#define C51WH54 "\\_/         \\_/  \\____/  \\____/  \\____/      \\_/       "

#define C51WH41 "| /~~ ||   |-----/~~~~\\  /[I_____I][][] --|||_______|__"
#define C51WH42 "------'|oOo|===[]=- O=======O=======O  |  ||=======_|__"
#define C51WH43 "/~\\____|___|/~\\_|      ||      ||      |__|+-/~\\_|     "
#define C51WH44 "\\_/         \\_/  \\____/  \\____/  \\____/      \\_/       "

#define C51WH31 "| /~~ ||   |-----/~~~~\\  /[I_____I][][] --|||_______|__"
#define C51WH32 "------'|oOo|==[]=- O=======O=======O   |  ||=======_|__"
#define C51WH33 "/~\\____|___|/~\\_|      ||      ||      |__|+-/~\\_|     "
#define C51WH34 "\\_/         \\_/  \\____/  \\____/  \\____/      \\_/       "

#define C51WH21 "| /~~ ||   |-----/~~~~\\  /[I_____I][][] --|||_______|__"
#define C51WH22 "------'|oOo|=[]=- O=======O=======O    |  ||=======_|__"
#define C51WH23 "/~\\____|___|/~\\_|      ||      ||      |__|+-/~\\_|     "
#define C51WH24 "\\_/         \\_/  \\____/  \\____/  \\____/      \\_/       "

#define C51WH11 "| /~~ ||   |-----/~~~~\\  /[I_____I][][] --|||_______|__"
#define C51WH12 "------'|oOo|=[]=-      ||      ||      |  ||=======_|__"
#define C51WH13 "/~\\____|___|/~\\_|  O=======O=======O   |__|+-/~\\_|     "
#define C51WH14 "\\_/         \\_/  \\____/  \\____/  \\____/      \\_/       "

/*========================================
 *    sl.c: SL version 5.03
 *        Copyright 1993,1998,2014-2015
 *                  Toyoda Masashi
 *                  (mtoyoda@acm.org)
 *        Last Modified: 2014/06/03
 *========================================
 */
/* sl version 5.03 : Fix some more compiler warnings.                        */
/*                                              by Ryan Jacobs    2015/01/19 */
/* sl version 5.02 : Fix compiler warnings.                                  */
/*                                              by Jeff Schwab    2014/06/03 */
/* sl version 5.01 : removed cursor and handling of IO                       */
/*                                              by Chris Seymour  2014/01/03 */
/* sl version 5.00 : add -c option                                           */
/*                                              by Toyoda Masashi 2013/05/05 */
/* sl version 4.00 : add C51, usleep(40000)                                  */
/*                                              by Toyoda Masashi 2002/12/31 */
/* sl version 3.03 : add usleep(20000)                                       */
/*                                              by Toyoda Masashi 1998/07/22 */
/* sl version 3.02 : D51 flies! Change options.                              */
/*                                              by Toyoda Masashi 1993/01/19 */
/* sl version 3.01 : Wheel turns smoother                                    */
/*                                              by Toyoda Masashi 1992/12/25 */
/* sl version 3.00 : Add d(D51) option                                       */
/*                                              by Toyoda Masashi 1992/12/24 */
/* sl version 2.02 : Bug fixed.(dust remains in screen)                      */
/*                                              by Toyoda Masashi 1992/12/17 */
/* sl version 2.01 : Smoke run and disappear.                                */
/*                   Change '-a' to accident option.                         */
/*                                              by Toyoda Masashi 1992/12/16 */
/* sl version 2.00 : Add a(all),l(long),F(Fly!) options.                     */
/*                                              by Toyoda Masashi 1992/12/15 */
/* sl version 1.02 : Add turning wheel.                                      */
/*                                              by Toyoda Masashi 1992/12/14 */
/* sl version 1.01 : Add more complex smoke.                                 */
/*                                              by Toyoda Masashi 1992/12/14 */
/* sl version 1.00 : SL runs vomiting out smoke.                             */
/*                                              by Toyoda Masashi 1992/12/11 */

#include <ctype.h>
#include <curses.h>
#include <errno.h>
#include <ncurses.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

void add_smoke(int y, int x);
void add_man(int y, int x);
int add_C51(int x);
int add_D51(int x);
int add_sl(int x);
void option(char *str);
int my_mvaddstr(int y, int x, char *str);

int ACCIDENT = 0;
int LOGO = 0;
int FLY = 0;
int C51 = 0;

int my_mvaddstr(int y, int x, char *str) {
    for (; x < 0; ++x, ++str)
        if (*str == '\0') return ERR;
    for (; *str != '\0'; ++str, ++x)
        if (mvaddch(y, x, *str) == ERR) return ERR;
    return OK;
}

void option(char *str) {
    extern int ACCIDENT, LOGO, FLY, C51;

    while (*str != '\0') {
        switch (*str++) {
            case 'a':
                ACCIDENT = 1;
                break;
            case 'F':
                FLY = 1;
                break;
            case 'l':
                LOGO = 1;
                break;
            case 'c':
                C51 = 1;
                break;
            default:
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    int x, i;

    for (i = 1; i < argc; ++i) {
        if (*argv[i] == '-') {
            option(argv[i] + 1);
        }
    }
    initscr();
    signal(SIGINT, SIG_IGN);
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    leaveok(stdscr, TRUE);
    scrollok(stdscr, FALSE);

    for (x = COLS - 1;; --x) {
        if (LOGO == 1) {
            if (add_sl(x) == ERR) break;
        } else if (C51 == 1) {
            if (add_C51(x) == ERR) break;
        } else {
            if (add_D51(x) == ERR) break;
        }
        getch();
        refresh();
        usleep(40000);
    }
    mvcur(0, COLS - 1, LINES - 1, 0);
    endwin();

    return 0;
}

int add_sl(int x) {
    static char *sl[LOGOPATTERNS][LOGOHEIGHT + 1] = {{LOGO1, LOGO2, LOGO3, LOGO4, LWHL11, LWHL12, DELLN},
                                                     {LOGO1, LOGO2, LOGO3, LOGO4, LWHL21, LWHL22, DELLN},
                                                     {LOGO1, LOGO2, LOGO3, LOGO4, LWHL31, LWHL32, DELLN},
                                                     {LOGO1, LOGO2, LOGO3, LOGO4, LWHL41, LWHL42, DELLN},
                                                     {LOGO1, LOGO2, LOGO3, LOGO4, LWHL51, LWHL52, DELLN},
                                                     {LOGO1, LOGO2, LOGO3, LOGO4, LWHL61, LWHL62, DELLN}};

    static char *coal[LOGOHEIGHT + 1] = {LCOAL1, LCOAL2, LCOAL3, LCOAL4, LCOAL5, LCOAL6, DELLN};

    static char *car[LOGOHEIGHT + 1] = {LCAR1, LCAR2, LCAR3, LCAR4, LCAR5, LCAR6, DELLN};

    int i, y, py1 = 0, py2 = 0, py3 = 0;

    if (x < -LOGOLENGTH) return ERR;
    y = LINES / 2 - 3;

    if (FLY == 1) {
        y = (x / 6) + LINES - (COLS / 6) - LOGOHEIGHT;
        py1 = 2;
        py2 = 4;
        py3 = 6;
    }
    for (i = 0; i <= LOGOHEIGHT; ++i) {
        my_mvaddstr(y + i, x, sl[(LOGOLENGTH + x) / 3 % LOGOPATTERNS][i]);
        my_mvaddstr(y + i + py1, x + 21, coal[i]);
        my_mvaddstr(y + i + py2, x + 42, car[i]);
        my_mvaddstr(y + i + py3, x + 63, car[i]);
    }
    if (ACCIDENT == 1) {
        add_man(y + 1, x + 14);
        add_man(y + 1 + py2, x + 45);
        add_man(y + 1 + py2, x + 53);
        add_man(y + 1 + py3, x + 66);
        add_man(y + 1 + py3, x + 74);
    }
    add_smoke(y - 1, x + LOGOFUNNEL);
    return OK;
}

int add_D51(int x) {
    static char *d51[D51PATTERNS][D51HEIGHT + 1] = {{D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
                                                     D51WHL11, D51WHL12, D51WHL13, D51DEL},
                                                    {D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
                                                     D51WHL21, D51WHL22, D51WHL23, D51DEL},
                                                    {D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
                                                     D51WHL31, D51WHL32, D51WHL33, D51DEL},
                                                    {D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
                                                     D51WHL41, D51WHL42, D51WHL43, D51DEL},
                                                    {D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
                                                     D51WHL51, D51WHL52, D51WHL53, D51DEL},
                                                    {D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
                                                     D51WHL61, D51WHL62, D51WHL63, D51DEL}};
    static char *coal[D51HEIGHT + 1] = {COAL01, COAL02, COAL03, COAL04, COAL05,
                                        COAL06, COAL07, COAL08, COAL09, COAL10, COALDEL};

    int y, i, dy = 0;

    if (x < -D51LENGTH) return ERR;
    y = LINES / 2 - 5;

    if (FLY == 1) {
        y = (x / 7) + LINES - (COLS / 7) - D51HEIGHT;
        dy = 1;
    }
    for (i = 0; i <= D51HEIGHT; ++i) {
        my_mvaddstr(y + i, x, d51[(D51LENGTH + x) % D51PATTERNS][i]);
        my_mvaddstr(y + i + dy, x + 53, coal[i]);
    }
    if (ACCIDENT == 1) {
        add_man(y + 2, x + 43);
        add_man(y + 2, x + 47);
    }
    add_smoke(y - 1, x + D51FUNNEL);
    return OK;
}

int add_C51(int x) {
    static char *c51[C51PATTERNS][C51HEIGHT + 1] = {{C51STR1, C51STR2, C51STR3, C51STR4, C51STR5, C51STR6, C51STR7,
                                                     C51WH11, C51WH12, C51WH13, C51WH14, C51DEL},
                                                    {C51STR1, C51STR2, C51STR3, C51STR4, C51STR5, C51STR6, C51STR7,
                                                     C51WH21, C51WH22, C51WH23, C51WH24, C51DEL},
                                                    {C51STR1, C51STR2, C51STR3, C51STR4, C51STR5, C51STR6, C51STR7,
                                                     C51WH31, C51WH32, C51WH33, C51WH34, C51DEL},
                                                    {C51STR1, C51STR2, C51STR3, C51STR4, C51STR5, C51STR6, C51STR7,
                                                     C51WH41, C51WH42, C51WH43, C51WH44, C51DEL},
                                                    {C51STR1, C51STR2, C51STR3, C51STR4, C51STR5, C51STR6, C51STR7,
                                                     C51WH51, C51WH52, C51WH53, C51WH54, C51DEL},
                                                    {C51STR1, C51STR2, C51STR3, C51STR4, C51STR5, C51STR6, C51STR7,
                                                     C51WH61, C51WH62, C51WH63, C51WH64, C51DEL}};
    static char *coal[C51HEIGHT + 1] = {COALDEL, COAL01, COAL02, COAL03, COAL04, COAL05,
                                        COAL06, COAL07, COAL08, COAL09, COAL10, COALDEL};

    int y, i, dy = 0;

    if (x < -C51LENGTH) return ERR;
    y = LINES / 2 - 5;

    if (FLY == 1) {
        y = (x / 7) + LINES - (COLS / 7) - C51HEIGHT;
        dy = 1;
    }
    for (i = 0; i <= C51HEIGHT; ++i) {
        my_mvaddstr(y + i, x, c51[(C51LENGTH + x) % C51PATTERNS][i]);
        my_mvaddstr(y + i + dy, x + 55, coal[i]);
    }
    if (ACCIDENT == 1) {
        add_man(y + 3, x + 45);
        add_man(y + 3, x + 49);
    }
    add_smoke(y - 1, x + C51FUNNEL);
    return OK;
}

void add_man(int y, int x) {
    static char *man[2][2] = {{"", "(O)"}, {"Help!", "\\O/"}};
    int i;

    for (i = 0; i < 2; ++i) {
        my_mvaddstr(y + i, x, man[(LOGOLENGTH + x) / 12 % 2][i]);
    }
}

void add_smoke(int y, int x)
#define SMOKEPTNS 16
{
    static struct smokes {
        int y;
        int x;
        int ptrn;
        int kind;
    } S[1000];
    static int sum = 0;
    static char *Smoke[2][SMOKEPTNS] = {{"(   )", "(    )", "(    )", "(   )", "(  )",
                                         "(  )", "( )", "( )", "()", "()",
                                         "O", "O", "O", "O", "O",
                                         " "},
                                        {"(@@@)", "(@@@@)", "(@@@@)", "(@@@)", "(@@)",
                                         "(@@)", "(@)", "(@)", "@@", "@@",
                                         "@", "@", "@", "@", "@",
                                         " "}};
    static char *Eraser[SMOKEPTNS] = {"     ", "      ", "      ", "     ", "    ",
                                      "    ", "   ", "   ", "  ", "  ",
                                      " ", " ", " ", " ", " ",
                                      " "};
    static int dy[SMOKEPTNS] = {2, 1, 1, 1, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0};
    static int dx[SMOKEPTNS] = {-2, -1, 0, 1, 1, 1, 1, 1, 2, 2,
                                2, 2, 2, 3, 3, 3};
    int i;

    if (x % 4 == 0) {
        for (i = 0; i < sum; ++i) {
            my_mvaddstr(S[i].y, S[i].x, Eraser[S[i].ptrn]);
            S[i].y -= dy[S[i].ptrn];
            S[i].x += dx[S[i].ptrn];
            S[i].ptrn += (S[i].ptrn < SMOKEPTNS - 1) ? 1 : 0;
            my_mvaddstr(S[i].y, S[i].x, Smoke[S[i].kind][S[i].ptrn]);
        }
        my_mvaddstr(y, x, Smoke[sum % 2][0]);
        S[sum].y = y;
        S[sum].x = x;
        S[sum].ptrn = 0;
        S[sum].kind = sum % 2;
        sum++;
    }
}