#include "../include/potatui.h"
#include <ncurses.h>

int main() {
  Application* app = potatui_init();

  int width = 60;
  int height = 20;
  int start_x = (COLS / 2) - (width / 2); // center the textarea
  int start_y = (LINES / 2) - (height / 2); // center the textarea
  TextArea* textarea = textarea_create(
    stdscr, // parent WINDOW
    height,
    width,
    start_y, // start Y
    start_x, // start X
    "TextArea", // Title
    NULL // on focus callback
  );

  textarea->base.user_data = textarea;
  textarea->show_line_numbers = TRUE;

  // add some lines to the textarea
  for (int i = 0; i < 20; i++) {  
    textarea_add_line(textarea, "This is the first line! This text continues until where I want it to stop. Maybe here, or here.");
  }

  app_add_container(app, (Container*)textarea);
  app_focus_on(app, textarea);

  textarea_render(textarea);

  potatui_loop(app);

  // finish the app
  textarea_destroy(textarea);
  potatui_end();
}
