#include <ncurses.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "../include/potatui.h"

/**
 * Starting and Ending ---------------------------------------------------------------
 */

// Start app
Application* potatui_init() {
  initscr();
  noecho();
  raw();
  cbreak();
  keypad(stdscr, TRUE);

  Application* app = (Application*)malloc(sizeof(Application));
  if (app == NULL) exit(1);

  app->focused_container = NULL;
  app->total_containers = 0;

  return app;
}

// Adds a new container to the focusable list
void app_add_container(Application* app, Container* con) {
  if (app->total_containers < MAX_CONTAINERS) {
    app->container_list[app->total_containers] = con;
    app->total_containers++;
  }
}

// Move the focus to the specified container
void app_focus_on(Application* app, Container* con) {
  if (con == NULL) return;
  app->focused_container = con;
}

// End ncurses
void potatui_end() {
  endwin();
}

// Handle all key inputs
void app_key_handle(Application* app, int c) {
  if (app == NULL) return;

  if (c == ctrl('q')) {
    potatui_end();
    exit(0);
  }

  // global actions
  switch (c) {
    case '\t':
      break;
  }

  // handle container focus
  if (app->focused_container != NULL && app->focused_container->on_focus != NULL) {
    app->focused_container->on_focus(c, app->focused_container->user_data);
  }

  // handle container default actions (a list handles arrow keys, for example)
  if (app->focused_container != NULL && app->focused_container->actions != NULL) {
    app->focused_container->actions(app->focused_container, c);
  }
}

// Application loop
void potatui_loop(Application* app) {
  if (app == NULL) return;

  // make sure to initialize the focused container when the loop starts
  if (app->focused_container != NULL) {
    container_update(app->focused_container);
  }

  int c;
  while ((c = getch())) {
    app_key_handle(app, c);
  }
}

// CTRL + key - doing this as a function helps on portability to other languages
int ctrl(int c) {
  return ((c) & (0x1f));
}


/**
 * Containers ------------------------------------------------------------------------
 */

// Initialize a container inside a parent
// Obj is a component based on Container
void container_init(void* obj) {
  if (obj == NULL) exit(1);

  Container* con = (Container*)obj;

  con->dwin = derwin(con->parent, con->height, con->width, con->start_y, con->start_x);

  // so we have to activate keypad for every window
  keypad(con->dwin, TRUE);
}

// Working with base containers
Container* container_create(WINDOW* parent, int height, int width, int start_y, int start_x, const char* title, bool has_border, void (*callback)(int, void*)) {
  Container* temp = (Container*)malloc(sizeof(Container));
  if (temp == NULL || parent == NULL) exit(1);

  temp->height = height;
  temp->width = width;
  temp->start_y = start_y;
  temp->start_x = start_x;
  temp->on_focus = callback;
  temp->actions = NULL;
  temp->title = title;
  temp->has_border = has_border;
  temp->user_data = NULL;

  // Initialize before printing any content inside it
  container_init(temp);
  return temp;
}

// print text on the container using va_list
void vcontainer_print(Container* con, bool break_line, int y, int x, const char* format, va_list args) {
  if (!con || !con->dwin) return;

  // 1. Finding the size for the buffer
  va_list args_copy;
  va_copy(args_copy, args);
  // when we give NULL and 0, the function return the necessary size for our buffer! (without considering the '\0')
  int necessary_size = vsnprintf(NULL, 0, format, args_copy);
  va_end(args_copy);

  if (necessary_size < 0) return; // formatation error

  // 2. Alocating memory (considering the '\0')
  char* buffer = (char*)malloc(necessary_size + 1);
  if (buffer == NULL) return; // system memory error

  // 3. Finally, truly formating the text
  int size = break_line ? (necessary_size + 1) : (con->width - 1); // this prevents the text from breaking line if we don't want it to break
  vsnprintf(buffer, size, format, args);

  // 4. Calculate the container internal limits
  int max_y = con->height - 1;
  int max_x = con->width - 1;

  // put the cursor on the desired position
  int cur_y = y;
  int cur_x = x;
  wmove(con->dwin, cur_y, cur_x);

  // 5. Print each character considering the borders
  for (int i = 0; buffer[i] != '\0'; i++) {
    // if we have a \n
    if (buffer[i] == '\n') {
      cur_y++;
      cur_x = 1; // go after the border
      if (cur_y >= max_y) break; // avoid going after the container view ending
      wmove(con->dwin, cur_y, cur_x);
      continue;
    }

    // jump to next line if we find a right border
    if (cur_x == max_x) {
      cur_y++;
      cur_x = 1;
      if (cur_y >= max_y) break;
      wmove(con->dwin, cur_y, cur_x);
    }

    // Draw the character and advance to the next column
    waddch(con->dwin, buffer[i]);
    cur_x++;
  }

  // 6. Freeing the memory
  free(buffer);
}

// print text on the container
void container_print(Container *con, bool break_line, int y, int x, const char *format, ...) {
  va_list args;
  va_start(args, format);
  vcontainer_print(con, break_line, y, x, format, args);
  va_end(args);
}

// update the container after some change
// obj is a component based on Container
void container_update(void* obj) {
  if (obj == NULL) return;

  Container* con = (Container*)obj;
  if (con->dwin == NULL) return;

  // first draw the borders if they exist
  if (con->has_border) {
    box(con->dwin, 0, 0);
 
    // print title if it has one
    if (con->title != NULL && con->title[0] != '\0') {
      mvwprintw(con->dwin, 0, 2, "< %s >", con->title);
    }
  }

  // the update
  wnoutrefresh(con->parent);
  wnoutrefresh(con->dwin);
  doupdate();
}


/**
 * Buttons -----------------------------------------------------------------------------------------------------------
 */

// Creates a new button instance
Button* button_create(WINDOW* parent, int height, int width, int start_y, int start_x, const char* label, void (*callback)(int, void*)) {
  Button* btn = (Button*)malloc(sizeof(Button));
  if (btn == NULL || parent == NULL) exit(1);

  // set up base container
  btn->base.height = height;
  btn->base.width = width;
  btn->base.start_y = start_y;
  btn->base.start_x = start_x;
  btn->base.parent = parent;
  btn->base.on_focus = callback;
  btn->base.is_focused = FALSE;
  btn->base.has_border = TRUE;
  btn->base.actions = NULL;
  btn->base.user_data = NULL;

  container_init(btn);

  snprintf(btn->label, sizeof(btn->label), "%s", label);
  container_update(btn);

  int center_x = (width / 2) - (strlen(btn->label) / 2);  
  container_print(&btn->base, FALSE, 1, 1, "%s", btn->label);

  wnoutrefresh(btn->base.dwin);
  doupdate();
  
  return btn;
}

// position the cursor inside the specified button and add it to the app->focused_container
void button_select(Application* app, Container* btn) {
  if (app == NULL || btn == NULL) return;

  if (app->focused_container != NULL) {
    Button* old_btn = (Button*)app->focused_container;
    int old_x = (old_btn->base.width / 2) - (strlen(old_btn->label) / 2);

    container_print(app->focused_container, FALSE, 1, 1, "%-*s", old_btn->base.width - 2, " ");
    container_print(app->focused_container, FALSE, 1, old_x, "%s", old_btn->label);
    wnoutrefresh(app->focused_container->dwin);
  }

  // change focus to the new button
  app->focused_container = btn;
  Button* new_btn = (Button*)btn;
  int new_x = (new_btn->base.width / 2) - (strlen(new_btn->label) / 2);

  container_print(btn, FALSE, 1, 1, "%-*s", new_btn->base.width - 2, " ");
  
  // print label in BOLD
  wattron(btn->dwin, A_BOLD);
  container_print(btn, FALSE, 1, new_x, "%s", new_btn->label);
  wattroff(btn->dwin, A_BOLD); // turn off BOLD
  
  wnoutrefresh(btn->dwin);
  doupdate();
}


/**
 * Lists ------------------------------------------------------------------------------------------------------------
 */

// render the list component
void list_render(List* list) {
  if (list == NULL || list->base.dwin == NULL) return;

  // hide the cursor
  curs_set(0);

  // redraw the border and the title
  container_update(list);

  int visible_height = list->base.height - 2;
  int usable_width = list->base.width - 2;

  // draw only visible items of the list
  for (int i = 0; i < visible_height; i++) {
    int real_index = list->scroll_top + i;
    int screen_row = i + 1;

    if (real_index < list->items) {
      if (real_index == list->selected) {
        // print selected item with inverted colors
        wattron(list->base.dwin, A_REVERSE);
        container_print((Container*)list, FALSE, screen_row, 1, "%-*s", usable_width, list->content[real_index]);
        wattroff(list->base.dwin, A_REVERSE);
      } else {
        container_print((Container*) list, FALSE, screen_row, 1, "%-*s", usable_width, list->content[real_index]);
      }
    } else { // there is no item for this visible line
      // remove trash from the screen
      container_print((Container*)list, FALSE, screen_row, 1, "%-*s", usable_width, "");
    }
  }

  // now positining the cursor over the selected item
  int visual_selected_row = (list->selected - list->scroll_top) + 1;
  wmove(list->base.dwin, visual_selected_row, 1);
  
  // force updating the visual buffer
  wnoutrefresh(list->base.dwin);
  doupdate();

  // show the cursor again
  curs_set(1);
}
 
// This executes when the list is focused, with the purpose of managing default actions of each list
void _list_actions(void* context, int c) {
  if (context == NULL) return;

  // if the list is focused, we know that it is exaclty the ACTIVE_CONTAINER
  List* list = (List*)context;

  if (list == NULL || list->items == 0) return;

  int visible_height = list->base.height - 2;

  // handle arrow keys
  switch (c) {
    case KEY_DOWN:
      if (list->selected < list->items - 1) {
        list->selected++;
      }
      // list_item_select(list, position);
      break;

    case KEY_UP:
      if (list->selected > 0) {
        list->selected--;
      }
      break;
  }

  // update scroll_top if the selected item is over the list view
  if (list->selected < list->scroll_top) {
    list->scroll_top = list->selected;
  }

  // update scroll_top if the selected item is under the list view
  if (list->selected >= list->scroll_top + visible_height) {
    list->scroll_top = list->selected - visible_height + 1;
  }
  
  list_render(list);
}

// Creates a new list and returns its pointer
List* list_create(WINDOW* parent, int height, int width, int start_y, int start_x, const char* title, bool has_border, void (*callback)(int, void*)) {
  List* temp = (List*)malloc(sizeof(List));
  if (temp == NULL || parent == NULL) exit(1);

  // init Container members
  temp->base.height = height;
  temp->base.width = width;
  temp->base.start_y = start_y;
  temp->base.start_x = start_x;
  temp->base.has_border = has_border;
  temp->base.on_focus = callback;
  temp->base.title = title;
  temp->base.parent = parent;

  // List properties
  temp->content = NULL;
  temp->selected = 0;
  temp->items = 0;
  temp->base.actions = _list_actions;
  temp->base.user_data = NULL;
  temp->scroll_top = 0;

  // init the List's derwin
  container_init(temp);

  return temp;
}

// Saves the recently added text line in the list->content string array
void _list_content_add(List* list, const char* line) {
  if (list == NULL || line == NULL) return;

  char** new_content = (char**)realloc(list->content, list->items * sizeof(char*));
  if (new_content == NULL) exit(1);

  list->content = new_content;
  list->content[list->items - 1] = (char*)line;
}

// add a new item to the list
void list_item_add(List* list, const char* line, ...) {
  if (list == NULL || line == NULL) return;

  // process the string on a temporary, clean, buffer
  va_list args;
  va_start(args, line);

  va_list args_copy;
  va_copy(args_copy, args);
  int needed = vsnprintf(NULL, 0, line, args_copy);
  va_end(args_copy);

  if (needed < 0) { va_end(args); return; }

  char* formatted_line = (char*)malloc(needed + 1);
  if (formatted_line == NULL) { va_end(args); exit(1); }

  vsnprintf(formatted_line, needed + 1, line, args);
  va_end(args);

  list->items += 1; // increase amount of lines - it also acts a way to know in which line of the list to print the next line
  list->selected = 0;// select item 0
  _list_content_add(list, formatted_line); // register the line in the list->content

  // force update
  int target_row = list->items;
  container_print((Container*)list, FALSE, target_row, 1, "%s", formatted_line);
}


/**
 * TextInput -----------------------------------------------------------------
*/

// add a char at the given position
void _textinput_add_char(TextInput* input, int position, int c) {
  if (input == NULL) return;
  if (position > input->content_size || position >= sizeof(input->content) - 1 || position < 0) return;

  for (int i = input->content_size; i > position; i--) {
    input->content[i] = input->content[i - 1];
  }

  input->content[position] = (char)c;
  input->content_size++;
  input->content[input->content_size] = '\0';
}

// remove a char from the given position
void _textinput_remove_char(TextInput* input, int position) {
  if (input == NULL) return;
  if (position < 0 || position > input->content_size) return;

  for (int i = position; i < input->content_size - 1; i++) {
    input->content[i] = input->content[i + 1];
  }

  input->content_size--;
  input->content[input->content_size] = '\0';
}

// the default behavior of the text input
void _textinput_default_actions(void* context, int c) {
  if (context == NULL) return;

  TextInput* input = (TextInput*)context;

  curs_set(0);

  // navigating through the text
  if (c == KEY_LEFT) {
    if (input->cursor_pos > 0) input->cursor_pos--;
  }
  else if (c == KEY_RIGHT) {
    if (input->cursor_pos < input->content_size) input->cursor_pos++;
  }

  // removing characters
  else if (c == KEY_BACKSPACE || c == 127 || c == 8) {
    if (input->cursor_pos > 0 && input->disabled == FALSE) {      
      _textinput_remove_char(input, input->cursor_pos - 1);
      input->cursor_pos--; // the cursor walks 1 character to the left after removing a char
    }
  }
  
  // filter usable characters
  else if (c >= 32 && c <= 126 && input->disabled == FALSE) {
    _textinput_add_char(input, input->cursor_pos, c);
    input->cursor_pos++; // the cursor advances after typing
  }

  // update boders and title
  container_update(input);

  // calculate which portion of the text to show and where to position the cursor
  int usable_width = input->base.width - 2;
  if (input->content_size < usable_width) {
    input->text_pos = 0;
  }
  else {
    // the cursor moves beyond the left visible area
    if (input->cursor_pos < input->text_pos) {
      input->text_pos = input->cursor_pos;
    }
    // the cursor moves beyond the right visible area
    if (input->cursor_pos >= input->text_pos + usable_width) {
      input->text_pos = input->cursor_pos - usable_width + 1;
    }
  }

  // where the cursor visually appears in the input
  int curs_pos_at_container = input->cursor_pos - input->text_pos;

  // print that portion of text and position the cursor
  container_print((Container*)input, FALSE, 1, 1, "%-*s", usable_width, &input->content[input->text_pos]);
  wmove(input->base.dwin, 1, curs_pos_at_container + 1);

  // update
  wnoutrefresh(input->base.dwin);
  doupdate();

  curs_set(1);
}

// create a new TextInput
TextInput* textinput_create(WINDOW* parent, int width, int start_y, int start_x, const char* label, void (*callback)(int, void*)) {
  if (parent == NULL) return NULL;
  
  TextInput* input = (TextInput*)malloc(sizeof(TextInput));
  if (input == NULL) exit(1);

  input->content[0] = '\0';
  input->content_size = 0;
  input->cursor_pos = 0;
  input->text_pos = 0;
  input->disabled = FALSE;

  // base container properties
  input->base.actions = _textinput_default_actions;
  input->base.has_border = TRUE;
  input->base.height = 3; // top and bottom borders plus content in one line
  input->base.on_focus = callback;
  input->base.parent = parent;
  input->base.start_x = start_x;
  input->base.start_y = start_y;
  input->base.width = width;
  input->base.user_data = NULL;
  input->base.title = label;

  container_init(input);
  container_update(input);
  wnoutrefresh(input->base.dwin);
  doupdate();

  return input;
}

// Force re-drawing the input when needed.
// For example, when you create a disabled input and then add text to it - you should re-render the input.
void textinput_render(TextInput* input) {
  _textinput_default_actions(input, 0);
}
