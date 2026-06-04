#include "../src/ctui.h"
#include <ncurses.h>

void list_focus(int c) {
  //
}

int main() {
  //start application
  Application* app = app_init();

  List* list = list_create(stdscr, 8, 20, 2, 2, "Languages", TRUE, list_focus);
  list_render(list);

  // set up list focus
  app_add_container(app, (Container*)list);
  app->focused_container = (Container*)list;

  int c;
  while ((c = getch())) {
    if (c == 'q') break;
    container_print((Container*)list, 1, 1, "content");
    container_update((Container*)list, stdscr);

    // here magic happens!
    app_key_handle(app, c);
  }

  app_end();
  return 0;
}
