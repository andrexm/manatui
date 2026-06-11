#include "../include/potatui.h"
#include <ncurses.h>

const int START_Y = 3;
const int START_X = 40;

// defining these items here simplify the example
Application* app;
Button* btn;
Button* btn2;

int clicks_btn1 = 0; // clicks on btn
int clicks_btn2 = 0; // clicks on btn2

// this function executes whenever the owner component (Button) has focus
// in this program, both buttons use this function
// you should create all your actions related to that component inside this function, and name it how you prefer
void btn_on_focus(int c, void* user_data) {
  if (c == '\t') { // when you press TAB
    Container* local_btn = // the other button (not focused)
                          app->focused_container == (Container*)btn // is btn the focused Button?
                          ? (Container*)btn2 // TRUE, then the TAB should move focus to btn2
                          : (Container*)btn; // FALSE, the btn2 is active and TAB move focus to btn
    button_select(app, local_btn); // then you focus on that other button
  }

  if (c == 10) { // when you press ENTER
    int clicks = 0;
    if (app->focused_container == (Container*)btn) {
      clicks_btn1++; // increase how many times btn was clicked
      clicks = clicks_btn1;
    } else {
      clicks_btn2++; // increase how many times btn2 was clicked
      clicks = clicks_btn2;
    }
    container_print(app->focused_container, FALSE, 1, 2, "%d clicks  ", clicks); // update label with container_print
    wmove(app->focused_container->dwin, 1, 1); // move the mouse before updating
    container_update(app->focused_container); // update the focused button
  }
}

// ------------------------

int main() {
  // Start the application
  app = potatui_init();

  // Create the buttons inside app and initialize
  btn = button_create(stdscr, 3, 12, START_Y, START_X, "Cancel", btn_on_focus);
  btn2 = button_create(stdscr, 3, 12, START_Y, START_X + 14, "Proceed", btn_on_focus);

  // Add both buttons to the focusable list
  app_add_container(app, (Container*)btn);
  app_add_container(app, (Container*)btn2);

  // You have to focus on something in order to interact with the program.
  // In this case, buttons have the 'button_select' function
  button_select(app, (Container*)btn); 
  //app->focused_container = (Container*)btn; // you could do this instead

  // Application loop
  potatui_loop(app);

  // Finish the program
  potatui_end(app);
  return 0;
}
