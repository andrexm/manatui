#include "../include/manatui.h"
#include <ncurses.h>

// You can use default ncurses methods
int main() {
  Application* app = manatui_init();

  mvprintw(1, 1, "height: %d", LINES);
  mvprintw(2, 1, "width: %d", COLS);
  refresh();
  getch();
  
  manatui_end(app);
  return 0;
}
