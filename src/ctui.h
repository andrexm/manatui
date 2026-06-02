#pragma once

#include "ncurses.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// Application - where everything happens
typedef struct {
  int height;
  int width;
} Application;

// Container - the base for all other components
typedef struct {
  // dimensions of the Pad
  int height;
  int width;

  // the start of the visible area at the screen (physical coordinates)
  int start_x;
  int start_y;

  bool has_border;
  WINDOW* dwin;
} Container;


/**
 * Starting and Ending ---------------------------------------------------------------
 */

// Start ncurses
void init_app() {
  initscr();
  noecho();
  raw();
  cbreak();
  keypad(stdscr, TRUE);
}

// End ncurses
void end_app() {
  endwin();
}

/**
 * Containers ------------------------------------------------------------------------
 */

// Working with base containers
Container* container_create() {
  Container* temp = (Container*)malloc(sizeof(Container));
  if (temp == NULL) {
    printf("Error while creating container.\n");
    exit(1);
  }
  return temp;
}

// initialize a container inside a parent
void container_init(Container* con, WINDOW* parent) {
  con->dwin = derwin(parent, con->height, con->width, con->start_y, con->start_x);
}

// update the container after some change
void container_update(Container* con, WINDOW* parent) {
  // first draw the borders if they exist
  if (con->has_border) box(con->dwin, 0, 0);

  // the update
  wnoutrefresh(parent);
  wnoutrefresh(con->dwin);
  doupdate();
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


/**
 * Buttons -----------------------------------------------------------------------------------------------------------
 */
typedef struct {
  Container base;
  char label[50];
  void (*on_click)();
} Button;

// Creates a new button instance
Button* button_create(WINDOW* parent, int height, int width, int start_y, int start_x, const char* label, void (*callback)()) {
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
  btn->on_click = callback;
  return btn;
}

Button* button_select(WINDOW* parent, Button* btn) {
  box(btn->base.dwin, 0, 0);
  move(btn->base.start_y + 1, btn->base.start_x + 1);
}

//

// TODO: container_sprint() -> prints a Text<String>, you can set up the starting line to start from it to the end of the view!
// NOTE: maybe this only make sense for text components.
