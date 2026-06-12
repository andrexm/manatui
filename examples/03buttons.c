#include "../manatui.h"
#include <ncurses.h>

const int START_Y = 3;
const int START_X = 40;

// defining these items here simplify the example
Application* app;
Button* btn;
Button* btn2;

// this function executes whenever the owner component (btn) has focus
void handle_btn1(int c, void* user_data) {
  if (c == '\t') {
    app_focus_on(app, btn2);
    wmove(btn2->base.dwin, 1, btn2->base.width / 2);
  }
  container_update(btn2);
}

// this function executes whenever the owner component (btn2) has focus
void handle_btn2(int c, void* user_data) {
  if (c == '\t') {
    app_focus_on(app, btn);
    wmove(btn->base.dwin, 1, btn->base.width / 2);
  }
  container_update(btn);
}

// ------------------------

int main() {
  // Start the application
  app = manatui_init();
  mvwprintw(stdscr, 1, COLS / 2 - 12, "Press Ctrl+Q to exit.");

  // Create the buttons inside app and initialize
  btn = button_create(stdscr, 3, 18, START_Y, START_X, "Cancel", handle_btn1);
  btn->base.foreground = "#ffffff";
  btn->base.background = "#ec003f";

  btn2 = button_create(stdscr, 3, 18, START_Y, START_X + 20, "Proceed", handle_btn2);
  btn2->base.foreground = "#ffffff";
  btn2->base.background = "#0069a8";

  button_render(app, btn);
  button_render(app, btn2);

  // You have to focus on something in order to interact with the program.
  app_focus_on(app, btn2); 

  // Application loop
  manatui_loop(app);

  // Finish the program
  manatui_end(app);
  return 0;
}
