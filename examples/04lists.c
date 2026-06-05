#include "../include/potatui.h"

void list_focus(int c, void* con) {
  Container* my_list = con;
  //container_print(my_list, TRUE, 1, 1, "content");
  //container_update(my_list, stdscr);
}

int main() {
  //start application
  Application* app = potatui_init();

  List* list = list_create(stdscr, 8, 50, 2, 2, "Languages", TRUE, list_focus);
  list->base.user_data = list; // add the list itself to the list_focus context

  list_item_add(list, "this is the first line!");
  list_item_add(list, "this is the second line!");
  list_item_add(list, "this is the third line!");
  list_render(list);

  // set up list focus
  app_add_container(app, (Container*)list);
  app->focused_container = (Container*)list;

  potatui_loop(app);

  potatui_end();
  return 0;
}
