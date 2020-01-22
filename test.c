#include <curses.h>

int main (void) {
    initscr();           //curses
	noecho();            //curses
	cbreak();            //curses
	keypad(stdscr,TRUE); //curses

    int ch;
    while(1) addstr("run...");

    endwin();
}