#include "../src/ctui.h"
#include <ncurses.h>
#include <stdio.h>

const int START_Y = 3;
const int START_X = 40;

Application* app;
Button* btn;
Button* btn2;

// this function check each input when the button is focused (the name 'onclick' is because we only handle the enter key)
void btn_onclick(int c) {
  if (c == 10) container_print(app->focused_container, 1, 1, " Clicked!!!");
  container_update(app->focused_container, stdscr);
}

// move the cursor between each button after pressing TAB
void tab_buttons() {
  int y, x;
  getyx(stdscr, y, x);
  if (x == START_X || x == START_X + 1) {
    wmove(btn->base.dwin, START_Y + 1, START_X + 14 + 1);
  } else {
    wmove(btn2->base.dwin, START_Y + 1, START_X + 1);
  }
}

int main() {
  // Start the application
  app = init_app();

  // Create a container inside app
  Container* con = container_create();

  // Create a button inside app and initialize it
  btn = button_create(stdscr, 3, 12, START_Y, START_X, "click me", btn_onclick);
  container_update((Container*)btn, stdscr);

  // Create another button
  btn2 = button_create(stdscr, 3, 14, START_Y, START_X + 14, "click me 2", btn_onclick);
  container_update((Container*)btn2, stdscr);

  // Add both buttons to the focusable list
  app_add_container(app, (Container*)btn);
  app_add_container(app, (Container*)btn2);

  int c;
  while (c = getch()) {
    // handle keys
    switch (c) {
      case 'q':
        goto finish;
        break;

      case '\t':
        if (app->focused_container == (Container*)btn) {
          button_select(app, stdscr, (Container*)btn2);
        } else {
          button_select(app, stdscr, (Container*)btn);
        }
        continue;
        break;

      default:
        break;
    }
    // executes the on_focus method of the active container
    if (app->focused_container != NULL) {

      app->focused_container->on_focus(c);
      continue;
    }

  }

  finish:
  end_app();
  return 0;
}
