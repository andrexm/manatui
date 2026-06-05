#include "../include/potatui.h"
#include <ncurses.h>

// You can use default ncurses methods
int main() {
  Application* app = app_init();

  mvprintw(1, 1, "height: %d", LINES);
  mvprintw(2, 1, "width: %d", COLS);
  refresh();
  getch();
  
  app_end();
  return 0;
}
