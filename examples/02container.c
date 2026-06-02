#include "../src/ctui.h"
#include <ncurses.h>
#include <stdio.h>

int main() {
  // Start the application
  init_app();

  // Create a container inside app
  Container* con = container_create();

  // Set the container properties
  con->height = 20;
  con->width = 20;
  con->start_x = 0;
  con->start_y = 1;
  con->has_border = TRUE;

  // Initialize before printing any content inside it
  container_init(con, stdscr);

  werase(con->dwin);
  mvwprintw(con->dwin, 1, 1, "hello, container! welcome to another amazing library!");
  container_update(con, stdscr);
  getch();
  
  end_app();
  return 0;
}
