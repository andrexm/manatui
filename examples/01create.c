#include "../src/ctui.h"
#include <ncurses.h>
#include <stdio.h>

int main() {
  // You can use default ncurses methods

  // Start the application
  init_app();
  mvprintw(1, 1, "height: %d", LINES);
  mvprintw(2, 1, "width: %d", COLS);
  refresh();
  getch();

  WINDOW* win = newwin(10, 10, 20, 10);
  box(win, 0, 0);
  wrefresh(win);

  getch();
  
  end_app();
  return 0;
}
