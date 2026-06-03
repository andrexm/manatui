#include "../src/ctui.h"
#include <ncurses.h>
#include <stdio.h>

int main() {
  // You can use default ncurses methods

  Application* app;
  // Start the application
  init_app(app);
  mvprintw(1, 1, "height: %d", LINES);
  mvprintw(2, 1, "width: %d", COLS);
  refresh();
  getch();
  
  end_app();
  return 0;
}
