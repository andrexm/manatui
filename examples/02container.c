#include "../src/ctui.h"
#include <ncurses.h>
#include <stdio.h>

int main() {
  // Start the application
  Application* app;
  app_init();

  // Create a container inside app
  Container* con = container_create(stdscr, 20, 20, 0, 1, TRUE, NULL);

  werase(con->dwin);
  //box(con->dwin, 0, 0);
  container_print(con, 1, 1, "hello, welcome to my new library. It was written\ncompletely in %c and\n%s!", 'C', "ncurses");
  container_update(con, stdscr);
  getch();
  
  app_end();
  return 0;
}
