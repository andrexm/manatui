#include "../include/potatui.h"
#include <ncurses.h>

int main() {
  // Start the application
  Application* app = potatui_init();

  int width = 30;
  int height = 20;
  int pos_x = (COLS / 2) - (width / 2); // we'll use this to draw the container at the center of the screen

  // Create a container inside app
  Container* con = // Container is the base for all components
    container_create(
      stdscr, // parent WINDOW*
      height,
      width,
      1, // start at y row
      pos_x, // start at x column
      "Welcome", // title - you can pass NULL to avoid rendering the title
      TRUE, // has border
      NULL // function to handle focus
    );

  container_print(// this is how you put text inside a container - every other component uses it too
                  con, // Container
                  TRUE, // break line if the text is larger than the container width
                  1, // start at y column of the container - 1 is to write below the top border
                  1, // start at x row of the container - 1 writes after the left border
                  "Hello, welcome to Potatui. It was written completely in %c and %s!", // we can pass a formatted string, like with printf
                  'C', "ncurses" // then you pass as many parameters as you need in your string
                );
  container_update(con);

  getch(); // wait for a keypress
  
  potatui_end(); // finish the app
  return 0;
}
