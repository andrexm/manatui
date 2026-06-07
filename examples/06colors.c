#include "../include/potatui.h"
#include <ncurses.h>

void on_focus(int c, void* context) {
  static int i = 0;
  Button* btn = (Button*)context;

  if (c == 10) {
    i++;
    mvwprintw(stdscr, 10, 58, "%d clicks", i);
  }
  wnoutrefresh(stdscr);
  wnoutrefresh(btn->base.dwin);
  doupdate();
}

int main() {
  Application* app = potatui_init();

  int width = 50;
  int start_x = (COLS / 2) - (width / 2);
  Button* btn = button_create(stdscr, 3, width, 5, start_x, "Click Me", on_focus);
  btn->base.user_data = btn;

  // add some colors
  btn->base.background = "#ffffff";
  btn->base.foreground = "#0099ff";

  // apply colors
  container_apply_style(btn);
  button_select(app, &btn->base); // select button and update

  app_focus_on(app, btn);
  btn->base.is_focused = TRUE;


  potatui_loop(app);

  potatui_end();
}
