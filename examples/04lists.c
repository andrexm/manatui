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

  // handle the ENTER key
  if (c == 10) {
    mvprintw(list->base.height + list->base.start_y + 1, 1, "Selected item: %-*s", 40, list->content[list->selected]);
  }
}

int main() {
  //start application
  Application* app = potatui_init();

  List* list = list_create(stdscr, 8, 50, 2, 2, "Languages", TRUE, list_focus);
  list->base.user_data = list; // add the list itself to the list_focus context

  for (int i = 0; i < 10; i++) {
    list_item_add(list, "This is the line N%d!", i+1);
  }
  list_render(list);

  // we can read the list content whenever we want
  for (int i = 0; i < list->items; i++) {
    mvprintw(i + 1, 60, "%s", list->content[i]);
  }

  // set up list focus
  app_add_container(app, (Container*)list);
  app->focused_container = (Container*)list;

  potatui_loop(app);

  potatui_end();
  return 0;
}
