#include "../include/potatui.h"
#include <ncurses.h>

int main() {
  Application* app = potatui_init();

  int width = 40;
  int start_x = (COLS / 2) - (width / 2);
  TextInput* input = textinput_create(stdscr, width, 1, start_x, "Your Name", NULL);

  app_add_container(app, (Container*)input);
  app->focused_container = (Container*)input;

  potatui_loop(app);

  potatui_end();
  return 0;
}
