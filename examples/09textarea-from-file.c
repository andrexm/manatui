#include "../manatui.h"
#include <ncurses.h>

typedef struct {
  Application* app;
  TextArea* textarea;
} Context;

// activated when the textarea has focus - in this example, the entire time
void handle_my_textarea(int c, void* context) {
  if (context == NULL) return;
  TextArea* textarea = (TextArea*)context;

  // handle ESC key
  if (c == 27) {
    // when the user press ESC, the textarea will be disabled
    // but if the user press 'i' (see line 42) the textarea will be enable again
    textarea->disabled = TRUE;
  }
}

int main() {
  Application* app = manatui_init();

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
    handle_my_textarea // on focus callback
  );

  textarea->base.has_border = TRUE;
  textarea->base.user_data = textarea; // if you want to add custom behavior to the textarea, user_data gives visibility to the object it receives
  textarea->show_line_numbers = TRUE; // show the column of numbers
  textarea->line_number_width = 4; // opitional: set up the width of the column of numbers
  textarea->enable_key = 'i'; // if you set this, when disabled, if the user press 'i' then the textarea will be enabled
  textarea->base.foreground = "#177458";
  textarea->base.background = "#090b0c";
  textarea->content_color = "#6b7a73";
  textarea->tabs_for_spaces = TRUE;
  textarea->use_theme_colors = TRUE;
  textarea->language = LANG_C;

  // add some lines to the textarea
  textarea_from_file(textarea, "src/manatui.c", false);

  app_focus_on(app, textarea); // start focus on it
  textarea_render(app, textarea); // draw the textarea

  manatui_loop(app);

  // finish the app
  textarea_destroy(textarea);
  manatui_end(app);
}
