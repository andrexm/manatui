#pragma once

#include "ncurses.h"
#include <stdio.h>
#include <stdlib.h>

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

// Start ncurses and the Application instance
//void init_app(Application* app) {
void init_app() {
  initscr();
  noecho();
  raw();
  cbreak();
}

// End ncurses and the Application instance
void end_app() {
  endwin();
}

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
