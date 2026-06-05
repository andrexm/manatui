#include "../include/potatui.h"
#include <ncurses.h>

void list_focus(int c, void* con) {
  List* list = (List*)con;

  // handle 'j' key - go down
  if (c == 'j' && list->selected < list->items - 1) {
    list->selected++;
  }

  // handle 'k' key - go up
  if (c == 'k' && list->selected > 0) {
    list->selected--;
  }
}

int main() {
  //start application
  Application* app = potatui_init();

  List* list = list_create(stdscr, 8, 50, 2, 2, "Languages", TRUE, list_focus);
  list->base.user_data = list; // add the list itself to the list_focus context

  list_item_add(list, "%d this is the 1 line!", 1);
  list_item_add(list, "%d this is the 2 line!", 2);
  list_item_add(list, "%d this is the 3 line!", 3);
  list_item_add(list, "%d this is the 4 line!", 4);
  list_item_add(list, "%d this is the 5 line!", 5);
  list_item_add(list, "%d this is the 6 line!", 6);
  list_item_add(list, "%d this is the 7 line!", 7);
  list_item_add(list, "%d this is the 8 line!", 8);
  list_item_add(list, "%d this is the 9 line!", 9);
  list_item_add(list, "%d this is the 10 line!", 10);
  list_item_add(list, "%d this is the 11 line!", 11);
  list_render(list);

  // we can read the list content whenever we want
  for (int i = 0; i < list->items; i++) {
    mvprintw(i + 1, 60, list->content[i]);
  }

  // set up list focus
  app_add_container(app, (Container*)list);
  app->focused_container = (Container*)list;

  potatui_loop(app);

  potatui_end();
  return 0;
}
