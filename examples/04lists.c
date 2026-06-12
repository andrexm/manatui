#include "../manatui.h"
#include <ncurses.h>

void list_focus(int c, void* con) {
  List* list = (List*)con;

  // handle 'j' key - go down
  if (c == 'j') {
    list_item_select(list, list->selected + 1);
  }

  // handle 'k' key - go up
  if (c == 'k') {
    list_item_select(list, list->selected - 1);
  }

  // handle the ENTER key
  if (c == 10) {
    mvprintw(list->base.height + list->base.start_y + 1, 1, "Selected item: %-*s", 40, list->content[list->selected]);
  }
}

int main() {
  //start application
  Application* app = manatui_init();

  List* list = list_create(stdscr, 8, 50, 2, 2, "Languages", TRUE, list_focus);
  list->base.user_data = list; // add the list itself to the list_focus context

  // OPTIONAL - adding some colors
  list->base.foreground = "#ff7823";
  list->base.background = "#222226";

  // adding some content to the list
  for (int i = 0; i < 10; i++) {
    list_item_add(list, "This is the line N%d!", i+1);
  }
  list_render(app, list);
  app_focus_on(app, list); // start focusing on the list

  // we can read the list content whenever we want
  for (int i = 0; i < list->items; i++) {
    mvprintw(i + 1, 60, "%s", list->content[i]);
  }

  manatui_loop(app);

  manatui_end(app);
  return 0;
}
