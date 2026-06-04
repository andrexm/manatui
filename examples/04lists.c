#include "../src/ctui.h"
#include <ncurses.h>

Application* app = NULL;

void list_focus(int c) {
  Container* my_list = app->focused_container;
  container_print(my_list, 1, 1, "content");
  container_update(my_list, stdscr);
}

int main() {
  //start application
  app = app_init();

  List* list = list_create(stdscr, 8, 20, 2, 2, "Languages", TRUE, list_focus);
  list_render(list);

  // set up list focus
  app_add_container(app, (Container*)list);
  app->focused_container = (Container*)list;

  int c;
  while ((c = getch())) {
    // here magic happens!
    app_key_handle(app, c);
  }

  app_end();
  return 0;
}
