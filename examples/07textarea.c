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
  textarea->base.foreground = "#177458";
  textarea->base.background = "#000000";
  textarea->content_color = "#6b7a73";
  container_apply_style(textarea);

  // add some lines to the textarea
  textarea_add_line(textarea, "This is the first line! This text continues until where I want it to stop. Maybe here, or here.");

  textarea_add_line(textarea, "Line 01.");
  textarea_add_line(textarea, "Line 02.");
  textarea_add_line(textarea, "Line 03.");
  textarea_add_line(textarea, "Line 04.");
  textarea_add_line(textarea, "Line 05.");
  textarea_add_line(textarea, "Line 06.");
  textarea_add_line(textarea, "Line 07.");
  textarea_add_line(textarea, "Line 08.");
  textarea_add_line(textarea, "Line 09.");
  textarea_add_line(textarea, "Line 10.");
  textarea_add_line(textarea, "Line 11.");
  textarea_add_line(textarea, "Line 12.");
  textarea_add_line(textarea, "Line 13.");
  textarea_add_line(textarea, "Line 14.");
  textarea_add_line(textarea, "Line 15.");
  textarea_add_line(textarea, "Line 16.");
  textarea_add_line(textarea, "Line 17.");
  textarea_add_line(textarea, "Line 18.");
  textarea_add_line(textarea, "Line 19.");
  textarea_add_line(textarea, "Line 20.");
  textarea_add_line(textarea, "Line 21.");
  textarea_add_line(textarea, "Line 22.");
  textarea_add_line(textarea, "Line 23.");

  app_add_container(app, (Container*)textarea);
  app_focus_on(app, textarea);

  textarea_render(textarea);

  potatui_loop(app);

  // finish the app
  textarea_destroy(textarea);
  potatui_end();
}
