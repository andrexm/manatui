#pragma once

#include "ncurses.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define MAX_CONTAINERS 50

// Container - the base for all other components
typedef struct {
  // dimensions of the Derwin
  int height;
  int width;

  // the start of the visible area at the screen (physical coordinates)
  int start_x;
  int start_y;

  const char* title;
  bool has_border;
  void (*on_focus)(int);
  void (*actions)(void*, int); // this should be used to implement intrinsic behavior into specific components within the lib
  WINDOW* dwin;
} Container;

// Application - where everything happens
typedef struct {
  int height;
  int width;
  Container* container_list[MAX_CONTAINERS]; // a list for focusable containers
  Container* focused_container;
  int total_containers; // the amount of focused containers
} Application;

/**
 * Starting and Ending ---------------------------------------------------------------
 */

// Start ncurses
Application* app_init() {
  initscr();
  noecho();
  raw();
  cbreak();
  keypad(stdscr, TRUE);

  Application* app = (Application*)malloc(sizeof(Application));
  if (app == NULL) exit(1);

  app->focused_container = NULL;
  app->total_containers = 0;

  return app;
}

// Adds a new container to the focusable list
void app_add_container(Application* app, Container* con) {
  if (app->total_containers < MAX_CONTAINERS) {
    app->container_list[app->total_containers] = con;
    app->total_containers++;
  }
}

// Move the focus to the specified container
void app_focus_on(Application* app, Container* con) {
  if (con == NULL) return;
  app->focused_container = con;
}

// End ncurses
void app_end() {
  endwin();
}

// Handle all key inputs
void app_key_handle(Application* app, int c) {
  if (app == NULL) return;

  // global actions
  switch (c) {
    case 'q':
      app_end();
      exit(0);

    case '\t':
      return;
  }

  // handle container default actions (a list handles arrow keys, for example)
  if (app->focused_container != NULL && app->focused_container->actions != NULL) {
    app->focused_container->actions(app, c);
  }

  // handle container focus
  if (app->focused_container != NULL && app->focused_container->on_focus != NULL) {
    app->focused_container->on_focus(c);
  }
}


/**
 * Containers ------------------------------------------------------------------------
 */

// initialize a container inside a parent
void container_init(Container* con, WINDOW* parent) {
  con->dwin = derwin(parent, con->height, con->width, con->start_y, con->start_x);
}

// Working with base containers
Container* container_create(WINDOW* parent, int height, int width, int start_y, int start_x, const char* title, bool has_border, void (*callback)(int)) {
  Container* temp = (Container*)malloc(sizeof(Container));
  if (temp == NULL) exit(1);

  temp->height = height;
  temp->width = width;
  temp->start_y = start_y;
  temp->start_x = start_x;
  temp->on_focus = callback;
  temp->actions = NULL;
  temp->title = title;
  temp->has_border = has_border;

  // Initialize before printing any content inside it
  container_init(temp, stdscr);
  return temp;
}

void container_print(Container* con, int y, int x, const char* format, ...) {
  if (!con || !con->dwin) return;

  va_list args;

  // 1. Finding the size for the buffer
  va_start(args, format);
  // when we give NULL and 0, the function return the necessary size for our buffer! (without considering the '\0')
  int necessary_size = vsnprintf(NULL, 0, format, args);
  va_end(args);

  if (necessary_size < 0) return; // formatation error

  // 2. Alocating memory (considering the '\0')
  char* buffer = (char*)malloc(necessary_size + 1);
  if (buffer == NULL) return; // system memory error

  // 3. Finally, truly formating the text
  va_start(args, format);
  vsnprintf(buffer, necessary_size + 1, format, args);
  va_end(args);

  // 4. Calculate the container internal limits
  int max_y = con->height - 1;
  int max_x = con->width - 1;

  // put the cursor on the desired position
  int cur_y = y;
  int cur_x = x;
  wmove(con->dwin, cur_y, cur_x);

  // 5. Print each character considering the borders
  for (int i = 0; buffer[i] != '\0'; i++) {
    // if we have a \n
    if (buffer[i] == '\n') {
      cur_y++;
      cur_x = 1; // go after the border
      if (cur_y >= max_y) break; // avoid going after the container view ending
      wmove(con->dwin, cur_y, cur_x);
      continue;
    }

    // jump to next line if we find a right border
    if (cur_x == max_x) {
      cur_y++;
      cur_x = 1;
      if (cur_y >= max_y) break;
      wmove(con->dwin, cur_y, cur_x);
    }

    // Draw the character and advance to the next column
    waddch(con->dwin, buffer[i]);
    cur_x++;
  }

  // 6. Freeing the memory
  free(buffer);
}

// update the container after some change
void container_update(Container* con, WINDOW* parent) {
  if (!con || !con->dwin) return;

  // first draw the borders if they exist
  if (con->has_border) {
    box(con->dwin, 0, 0);

    // print title if it has one
    if (con->title != NULL && con->title[0] != '\0') {
      container_print(con, 0, 2, "< %s >", con->title);
    }
  }

  // the update
  wnoutrefresh(parent);
  wnoutrefresh(con->dwin);
  doupdate();
}


/**
 * Buttons -----------------------------------------------------------------------------------------------------------
 */
typedef struct {
  Container base;
  char label[50];
} Button;

// Creates a new button instance
Button* button_create(WINDOW* parent, int height, int width, int start_y, int start_x, const char* label, void (*callback)(int)) {
  Button* btn = (Button*)malloc(sizeof(Button));
  if (btn == NULL) exit(1);

  // set up base container
  btn->base.height = height;
  btn->base.width = width;
  btn->base.start_y = start_y;
  btn->base.start_x = start_x;

  btn->base.dwin = derwin(parent, height, width, start_y, start_x);

  box(btn->base.dwin, 0, 0);
  container_print(&btn->base, 1, 1, " %s ", label);
  btn->base.on_focus = callback;
  btn->base.actions = NULL;
  return btn;
}

// position the cursor inside the specified button and add it to the app->focused_container
void button_select(Application* app, WINDOW* parent, Container* btn) {
  if (btn == NULL || btn->dwin == NULL) return;

  box(btn->dwin, 0, 0);
  wmove(btn->dwin, 1, 1);
  container_update(btn, parent);
  app_focus_on(app, btn);
}


/**
 * Lists ------------------------------------------------------------------------------------------------------------
 */

typedef struct {
  Container base;
  char** content;
  int items;
  int selected;
} List;

// render the list component
void list_render(List* list) {
  container_update((Container*)list, stdscr);
}
 
// This executes when the list is focused, with the purpose of managing default actions of each list
void _list_actions(void* app, int c) {
  // if the list is focused, we know that it is exaclty the ACTIVE_CONTAINER
  Application* _app = (Application*)app;
  Container* _list = _app->focused_container;

  // handle arrow keys
  werase((WINDOW*)_list->dwin);
  if (c == KEY_DOWN) container_print(_list, 1, 1, "down");
  container_update(_list, stdscr);
}

// Creates a new list and returns its pointer
List* list_create(WINDOW* parent, int height, int width, int start_y, int start_x, const char* title, bool has_border, void (*callback)(int)) {
  List* temp = (List*)malloc(sizeof(List));
  if (temp == NULL) exit(1);

  // init Container members
  temp->base.height = height;
  temp->base.width = width;
  temp->base.start_y = start_y;
  temp->base.start_x = start_x;
  temp->base.has_border = has_border;
  temp->base.on_focus = callback;
  temp->base.title = title;

  // init the List's derwin
  container_init(&temp->base, parent);

  // List properties
  temp->content = NULL;
  temp->selected = 0;
  temp->items = 0;
  temp->base.actions = _list_actions;

  return temp;
}


// TODO: container_sprint() -> prints a Text<String>, you can set up the starting line to start from it to the end of the view!
// NOTE: maybe this only make sense for text components.

