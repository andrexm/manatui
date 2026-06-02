#include "../src/ctui.h"
#include <ncurses.h>
#include <stdio.h>

const int START_Y = 3;
const int START_X = 40;


void btn_onclick() {
  exit(0);
}

// move the cursor between each button after pressing TAB
void tab_buttons() {
  int y, x;
  getyx(stdscr, y, x);
  if (x == START_X || x == START_X + 1) {
    move(START_Y + 1, START_X + 14 + 1);
  } else {
    move(START_Y + 1, START_X + 1);
  }
}

int main() {
  // Start the application
  init_app();

  // Create a container inside app
  Container* con = container_create();

  // Create a button inside app and initialize it
  Button* btn = button_create(stdscr, 3, 12, START_Y, START_X, "click me", btn_onclick);
  container_update((Container*)btn, stdscr);

  // Create another button
  Button* btn2 = button_create(stdscr, 3, 14, START_Y, START_X + 14, "click me 2", btn_onclick);
  container_update((Container*)btn2, stdscr);

  int c;
  while (c = getch()) {
    switch (c) {
      case 'q':
        goto finish;
        break;

      case '\t':
        tab_buttons();
        break;

      default:
        break;
    }
  }

  finish:
  end_app();
  return 0;
}
