#include "../include/potatui.h"
#include <ncurses.h>
#include <stdlib.h>
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
  Application* app = potatui_init();

  int width = 40;
  int start_x = (COLS / 2) - (width / 2);

  // create an input
  TextInput* input = textinput_create(stdscr, width, 1, start_x, "Your Name", input_on_focus);

  // create an input and disable it
  TextInput* disabled_input = textinput_create(stdscr, width, 6, start_x, "Disabled Input", input_on_focus);
  disabled_input->disabled = TRUE;

  char* str = "this is an example to show the behavior of disabled inputs!";
  memcpy(disabled_input->content, str, strlen(str));
  disabled_input->content_size = strlen(str);
  textinput_render(disabled_input);

  InputContext* context = (InputContext*)malloc(sizeof(InputContext));
  if (context == NULL) exit(1);
  context->disabled_input = disabled_input;
  context->input = input;
  context->app = app;

  disabled_input->base.user_data = context;
  input->base.user_data = context;

  app_add_container(app, (Container*)input);
  app_add_container(app, (Container*)disabled_input);

  app_focus_on(app, input); // start focusing on the enabled input

  potatui_loop(app);

  potatui_end();
  return 0;
}
