#include "../include/potatui.h"
#include <ncurses.h>

int main() {
  Application* app = potatui_init();

  char* test_str = (char*)app_alloc(app, NULL, sizeof(char) * 8);
  test_str = "text 01";
  wprintw(stdscr, "test_str: '%s'", test_str);

  getch();
  potatui_end(app);
}
