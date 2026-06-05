#include "../include/potatui.h"
#include <ncurses.h>

int main() {
  // Start the application
  Application* app = potatui_init();

  // Create a container inside app
  Container* con = container_create(stdscr, 20, 20, 0, 1, NULL, TRUE, NULL);

  werase(con->dwin);
  container_print(con, 1, 1, "hello, welcome to my new library. It was written\ncompletely in %c and\n%s!", 'C', "ncurses");
  container_update(con, stdscr);
  getch();
  
  potatui_end();
  return 0;
}
