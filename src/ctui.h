#pragma once

#include "ncurses.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int height;
  int width;
  WINDOW* default_win;
} Application;

// Create a new application
Application* create_app() {
  Application* temp = (Application*)malloc(sizeof(Application));
  if (temp == NULL) {
    printf("Error while creating Application.\n");
    exit(1);
  }
  return temp;
}

// Start ncurses and the Application instance
void init_app(Application* app) {
  initscr();
  noecho();
  raw();
  cbreak();
  refresh();

  app->width = COLS;
  app->height = LINES;
  app->default_win = newwin(LINES, COLS, 0, 0);
  box(app->default_win, 0, 0);
  wrefresh(app->default_win);

  refresh();
}

// End ncurses and the Application instance
void end_app(Application* app) {
  free(app);
  endwin();
}
