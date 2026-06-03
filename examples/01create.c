#include "../src/ctui.h"
#include <ncurses.h>
#include <stdio.h>

int main() {
  // You can use default ncurses methods

  Application* app;
  // Start the application
  app_init();
  mvprintw(1, 1, "height: %d", LINES);
  mvprintw(2, 1, "width: %d", COLS);
  refresh();
  getch();
  
  app_end();
  return 0;
}
