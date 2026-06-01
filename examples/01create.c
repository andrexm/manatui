#include "../src/ctui.h"
#include <ncurses.h>
#include <stdio.h>

int main() {
  // Create a new application
  Application* app = create_app();
  WINDOW* win = newwin(LINES, COLS, 20, 10);

  init_app(app);

  wmove(app->default_win, 1, 1);
  wprintw(app->default_win, "height: %d", app->height);
  wmove(app->default_win, 2, 1);
  wprintw(app->default_win, "width: %d", app->width);
  wrefresh(app->default_win);
  getch();
  
  end_app(app);
  return 0;
}
