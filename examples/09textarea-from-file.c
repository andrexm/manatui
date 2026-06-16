#include "../manatui.h"
#include <ncurses.h>
#include <stdlib.h>

typedef struct {
  Application* app;
  TextArea* textarea;
  const char* filepath;
} Context;

// activated when the textarea has focus - in this example, the entire time
void handle_my_textarea(int c, void* con) {
  if (con== NULL) return;
  Context* context = (Context*)con;
  TextArea* textarea = context->textarea;
  const char* filepath = context->filepath;

  // handle ESC key
  if (c == 27) {
    // when the user press ESC, the textarea will be disabled
    // but if the user press 'i' (see line 42) the textarea will be enable again
    textarea->disabled = TRUE;
  }

  // save the file when pressing CTRL+S
  if (c == ctrl('s')) {
    bool success = textarea_save_to_file(textarea, filepath);
    if (!success) {
      // error while saving content to the file
    }
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

  // creating a context variable to give the textarea actions (when focused)
  // visibility over the app, filepath and the textarea itself.
  const char* filepath = "my_code.c";
  Context* context = (Context*)malloc(sizeof(Context));
  context->filepath = filepath;
  context->app = app;
  context->textarea = textarea;

  // textarea settings
  textarea->base.has_border = TRUE;
  textarea->base.user_data = context; // if you want to add custom behavior to the textarea, user_data gives visibility to the object it receives
  textarea->show_line_numbers = TRUE; // show the column of numbers
  textarea->line_number_width = 4; // opitional: set up the width of the column of numbers
  textarea->enable_key = 'i'; // if you set this, when disabled, if the user press 'i' then the textarea will be enabled
  textarea->base.foreground = "#177458";
  textarea->base.background = "#090b0c";
  textarea->content_color = "#6b7a73";
  textarea->tabs_for_spaces = TRUE;
  textarea->use_theme_colors = TRUE;
  textarea->language = LANG_C;

  // add content to the textarea from a file.
  // if the file doesn't exist, 'true' tells to create an empty one, false would prevent from this.
  // if you press "CTRL+S", the file will be saved into that file, and it will try to create it if it doesn't exist.
  // this action of saving is defined in this file, on the "handle_my_textarea" function.
  textarea_from_file(textarea, filepath, true);

  app_focus_on(app, textarea); // start focus on it
  textarea_render(app, textarea); // draw the textarea

  manatui_loop(app);

  // finish the app
  textarea_destroy(textarea);
  manatui_end(app);
}
