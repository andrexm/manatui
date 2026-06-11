#include "../include/potatui.h"
#include <ncurses.h>

int main() {
  Application* app = potatui_init();

  // app_alloc substitutes malloc and realloc, of course, by working with reallo under the hood.
  // the reason to have this function is to automatically free the object when potatui_end() is called.
  // also, if the allocation fails, the program will close and you will see a message on stderr.
  char* test_str = (char*)app_alloc(app, NULL, sizeof(char) * 8);
  test_str = "text 01";
  wprintw(stdscr, "test_str: '%s'\n", test_str);

  wprintw(stdscr, "Press any key to close...");
  getch();
  potatui_end(app);
}
