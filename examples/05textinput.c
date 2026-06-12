#include "../include/manatui.h"
#include <ncurses.h>
#include <string.h>

// This type will help on giving visibility of our components to the input_on_focus - the function we created here to handle each input focus state
typedef struct {
  Application* app;
  TextInput* input;
  TextInput* disabled_input;
} InputContext;

// Handles the focus state of each input - of course, you could define a specific function for each input
void input_on_focus(int c, void* user_data) {
  if (user_data == NULL) return;

  InputContext* context = (InputContext*)user_data;

  // change focus on pressing TAB
  if (c == '\t') {
    if (context->app->focused_container == &context->input->base) {
      app_focus_on(context->app, (Container*)context->disabled_input); // change focus to the disabled input
    } else {
      app_focus_on(context->app, (Container*)context->input); // change focus to the enabled input
    }
  }
}

int main() {
  Application* app = manatui_init();

  int width = 40;
  int start_x = (COLS / 2) - (width / 2);

  // create an input
  TextInput* input = textinput_create(stdscr, width, 1, start_x, "Your Name", input_on_focus);

  // create an input and disable it
  TextInput* disabled_input = textinput_create(stdscr, width, 6, start_x, "Disabled Input", input_on_focus);
  disabled_input->disabled = TRUE;
  char* str = "this is an example to show the behavior of disabled inputs!";
  textinput_set(disabled_input, str); // add str as the disabled input content
  textinput_render(app, disabled_input);

  InputContext* context = (InputContext*)app_alloc(app, NULL, sizeof(InputContext));
  context->disabled_input = disabled_input;
  context->input = input;
  context->app = app;

  disabled_input->base.user_data = context;
  input->base.user_data = context;

  textinput_render(app, input);
  textinput_render(app, disabled_input);

  app_focus_on(app, input); // start focusing on the enabled input

  manatui_loop(app);

  manatui_end(app);
  return 0;
}
