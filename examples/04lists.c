#include "../include/ctui.h"
#include <ncurses.h>

Application* app = NULL;

void list_focus(int c, void* con) {
  Container* my_list = con;
  container_print(my_list, 1, 1, "content");
  container_update(my_list, stdscr);
}

int main() {
  //start application
  app = app_init();

  List* list = list_create(stdscr, 8, 20, 2, 2, "Languages", TRUE, list_focus);
  list->base.user_data = list;
  list_render(list);

  // set up list focus
  app_add_container(app, (Container*)list);
  app->focused_container = (Container*)list;

  app_loop(app);

  app_end();
  return 0;
}
