#include <ncurses.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "../include/potatui.h"


/**
 * Colors ----------------------------------------------------------------------------
*/
static int global_color_pair_counter = 1;
static int global_color_id_counter = 16; // avoid overwritting the default colors

// Converts a hex color to the ncurses format
int _register_hex_color(int color_id, const char* hex_str) {
  if (hex_str == NULL || hex_str[0] != '#') return -1;

  unsigned int r, g, b;

  // get the hex values from hex_str
  sscanf(hex_str + 1, "%02x%02x%02x", &r, &g, &b);

  // convert to what ncurses expects - from 0-255 scale to 0-1000 scale
  short ncurses_r = (short)((r * 1000) / 255);
  short ncurses_g = (short)((g * 1000) / 255);
  short ncurses_b = (short)((b * 1000) / 255);

  init_color(color_id, ncurses_r, ncurses_g, ncurses_b);
  return 0;
}


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

  if (has_colors()) {
    start_color();
  }
  
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
void app_focus_on(Application* app, void* con) {
  if (con == NULL) return;
  app->focused_container = (Container*)con;
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
  temp->parent = parent;

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

  // start the colors
  if (con->color_pair_id > 0) {
    wattron(con->dwin, COLOR_PAIR(con->color_pair_id));
    wbkgd(con->dwin, COLOR_PAIR(con->color_pair_id));
  }

  // first draw the borders if they exist
  if (con->has_border) {
    box(con->dwin, 0, 0);
 
    // print title if it has one
    if (con->title != NULL && con->title[0] != '\0') {
      mvwprintw(con->dwin, 0, 2, "< %s >", con->title);
    }
  }

  // end the colors
  if (con->color_pair_id > 0) {
    wattroff(con->dwin, COLOR_PAIR(con->color_pair_id));
  }

  // the update
  wnoutrefresh(con->parent);
  wnoutrefresh(con->dwin);
  doupdate();
}

// Apply color system
void container_apply_style(void* con) {
  if (!has_colors()) return;

  Container* container = (Container*)con;

  // init common color system
  if (container->foreground != NULL && container->background != NULL) {
    int fg_id = global_color_id_counter++;
    int bg_id = global_color_id_counter++;
    int pair_id = global_color_pair_counter++;

    // init colors
    _register_hex_color(fg_id, container->foreground);
    _register_hex_color(bg_id, container->background);

    // init color pair
    init_pair(pair_id, fg_id, bg_id);
    container->color_pair_id = pair_id;
  }
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


/**
 * TextArea ------------------------------------------------------------------
*/

// calculates and sets the number of digits for the lines numbers
void _textarea_set_line_width(TextArea* textarea) {
  int temp_lines = textarea->total_lines;
  int digits = 1;

  while (temp_lines >= 10) {
    digits++;
    temp_lines /= 10;
  }

  textarea->line_number_width = digits;
}

int _textarea_get_max_visible_lines(TextArea* textarea) {
  int max_visible_lines = textarea->base.height - 2;
  if (max_visible_lines < 1) max_visible_lines = 1;
  return max_visible_lines;
}

int _textarea_get_usable_width(TextArea* textarea) {
  int usable_width = textarea->base.width - textarea->line_number_width - 3;
  if (usable_width < 1) usable_width = 1;
  return usable_width;
}

// Handles cursor position and text scroll after KEY_DOWN is pressed
void textarea_handle_key_down(TextArea* textarea, int max_visible_lines) { 
  if (textarea->total_lines - 1 > textarea->cursor_row) {
    textarea->cursor_row++;

    // if the bottom line is smaller than the current
    int next_line_len = strlen(textarea->lines[textarea->cursor_row]);
    if (textarea->cursor_col > next_line_len) {
      textarea->cursor_col = next_line_len;
    }

    // fix the cursor visibility if the new line is very short
    if (textarea->cursor_col < textarea->scroll_col) {
      textarea->scroll_col = textarea->cursor_col > 3 ? textarea->cursor_col - 3 : textarea->cursor_col;
    }

    // scroll the text if necessary
    if (textarea->cursor_row >= textarea->scroll_col + max_visible_lines) {
      textarea->scroll_row = textarea->cursor_row - max_visible_lines + 1;
    }
  }
}

// Handles cursor position and text scroll after KEY_UP is pressed
void textarea_handle_key_up(TextArea* textarea) { 
  if (textarea->cursor_row > 0) {
    textarea->cursor_row--;

    // adjust cursor position if the above line is shorter
    int prev_line_len = strlen(textarea->lines[textarea->cursor_row]);
    if (textarea->cursor_col > prev_line_len) {
      textarea->cursor_col = prev_line_len;
    }

    // fix the cursor visibility if the new line is very short
    if (textarea->cursor_col < textarea->scroll_col) {
      textarea->scroll_col = textarea->cursor_col > 3 ? textarea->cursor_col - 3 : textarea->cursor_col;
    }

    // returns the text scroll back if necessary
    if (textarea->cursor_row < textarea->scroll_row) {
      textarea->scroll_row = textarea->cursor_row;
    }
  }
}

// Handles cursor position and text scroll after KEY_RIGHT is pressed
void textarea_handle_key_right(TextArea* textarea, int usable_width) {
  int current_line_len = strlen(textarea->lines[textarea->cursor_row]);
    
  if (textarea->cursor_col < current_line_len) {
    textarea->cursor_col++;

    // scroll to the right
    if (textarea->cursor_col >= textarea->scroll_col + usable_width) {
      textarea->scroll_col = textarea->cursor_col - usable_width + 1;
    }
  }
  // cursor jumps to the start of the next line
  else if (textarea->cursor_row < textarea->total_lines - 1) {
    textarea->cursor_col = 0;
    textarea->scroll_col = 0;
    textarea->cursor_row++;

    // scroll down if necessary
    int max_visible_lines = _textarea_get_max_visible_lines(textarea);
    if (textarea->cursor_row >= textarea->scroll_row + max_visible_lines) {
      textarea->scroll_row++;
    }
  }
}

// Handles cursor position and text scroll after KEY_LEFT is pressed
void textarea_handle_key_left(TextArea *textarea) {
  if (textarea->cursor_col > 0) {
    textarea->cursor_col--;

    // scroll to the left
    if (textarea->cursor_col < textarea->scroll_col) {
      textarea->scroll_col = textarea->cursor_col;
    }
  } else {
    // move the cursor to the end of the line above
    if (textarea->cursor_row > 0) {
      textarea->cursor_row--;
      textarea->cursor_col = strlen(textarea->lines[textarea->cursor_row]) - 1;

      int visible_area = textarea->base.width - 3 - textarea->line_number_width;
      if (strlen(textarea->lines[textarea->cursor_row]) > visible_area) {
        textarea->scroll_col = strlen(textarea->lines[textarea->cursor_row]) - (visible_area);
      }
    }
  }
}

// Handles the textarea content after pressing the ENTER key
void textarea_handle_key_enter(TextArea* textarea) {
  if (textarea == NULL) return;

  // the original size of the line
  int current_len = strlen(textarea->lines[textarea->cursor_row]);
  
  // the size of the line created
  int new_line_size = current_len - textarea->cursor_col;

  // the state of the original line after pressing ENTER (breaking the line)
  char* new_line = (char*)malloc((new_line_size + 1) * sizeof(char));
  if (new_line == NULL) exit(1);

  // copy the content for the new line
  strcpy(new_line, textarea->lines[textarea->cursor_row] + textarea->cursor_col);

  // updates the current line
  textarea->lines[textarea->cursor_row][textarea->cursor_col] = '\0';
  
  // free the unused memory from the original line
  char* shrunk_line = (char*)realloc(textarea->lines[textarea->cursor_row], (textarea->cursor_col + 1) * sizeof(char));
  if (shrunk_line != NULL) {
    textarea->lines[textarea->cursor_row] = shrunk_line;
  }

  // make sure there is enough space for a new line
  // NOTE: this conditional is block possibly capable of being separated into a dedicated function,
  // since it is used also in textarea_add_line.
  if (textarea->total_lines >= textarea->lines_capacity) {
    // start with capacity for 8 lines
    int new_capacity = (textarea->lines_capacity == 0) ? 8 : textarea->lines_capacity * 2;

    char** temp_lines = (char**)realloc(textarea->lines, new_capacity * sizeof(char*));
    if (temp_lines == NULL) exit(1);

    textarea->lines = temp_lines;
    textarea->lines_capacity = new_capacity;
  }

  // copy each line until we find the new line position
  for (int i = textarea->total_lines; i > textarea->cursor_row + 1; i--) {
    textarea->lines[i] = textarea->lines[i - 1];
  }

  // insert the new line
  textarea->lines[textarea->cursor_row + 1] = new_line;

  // update textarea state
  textarea->cursor_row++;
  textarea->total_lines++;
  textarea->cursor_col = 0; // start of the line
  textarea->scroll_col = 0; // reset scroll

  // update vertical scroll if necessary
  int max_visible_lines = _textarea_get_max_visible_lines(textarea);
  if (textarea->cursor_row >= textarea->scroll_row + max_visible_lines) {
    textarea->scroll_row++;
  }
}

//
// When scrolling to the right, if the scroll_col for bigger than this line, the printer will print from
// memory addressess after the string, this prevents that from happening, by giving " " (space) to the printer
// 
char* _textarea_get_printable_content(TextArea* textarea, int file_line_index) {
  char* content = (textarea->scroll_col <= strlen(textarea->lines[file_line_index]))
                  ? &textarea->lines[file_line_index][textarea->scroll_col]
                  : " ";
  return content;
}

// Handle adding a char to the text
void _textarea_add_char(TextArea* textarea, int c) {
  if (textarea == NULL) return;

  // calculate the current lien length
  int current_len = strlen(textarea->lines[textarea->cursor_row]);

  // get space to allocate a new char and the null terminator
  char* temp_line = (char*)realloc(textarea->lines[textarea->cursor_row], (current_len + 2) * sizeof(char));
  if (temp_line == NULL) exit(1);

  textarea->lines[textarea->cursor_row] = temp_line;

  // move each character if necessary
  for (int i = current_len; i >= textarea->cursor_col; i--) {
    textarea->lines[textarea->cursor_row][i + 1] = textarea->lines[textarea->cursor_row][i];
  }

  // add the new character and update the cursor position
  textarea->lines[textarea->cursor_row][textarea->cursor_col] = (char)c;
  textarea->cursor_col++;

  // adjust scroll
  int usable_width = _textarea_get_usable_width(textarea);
  if (textarea->cursor_col >= textarea->scroll_col + usable_width) {
    textarea->scroll_col = textarea->cursor_col - usable_width + 1;
  }
}

// remove a character just at the left from the current cursor position
void _textarea_remove_left_char(TextArea* textarea) {
  if (textarea == NULL) return;

  // NOTE case 01: line 0, column 0
  if (textarea->cursor_col == 0 && textarea->cursor_row == 0) {
    return;
  }

  // NOTE case 02: default behavior - take characters from right to left
  if (textarea->cursor_col > 0) {
    // length of the current line
    int line_len = strlen(textarea->lines[textarea->cursor_row]);
    char* current_line = textarea->lines[textarea->cursor_row];

    // move characters from right to left, overwitting the target
    for (int i = textarea->cursor_col; i < line_len; i++) {
      current_line[i - 1] = current_line[i];
    }
    current_line[line_len - 1] = '\0';
    textarea->cursor_col--; // update cursor

    // update scroll if the cursor is at a position before the visible area
    if (textarea->cursor_col < textarea->scroll_col) {
      textarea->scroll_col = textarea->cursor_col;
    }
    return;
  }

  // NOTE case 03: when the cursor is at the begining of a line, merge this line with the end of the line above
  int prev_row = textarea->cursor_row - 1;
  int prev_len = strlen(textarea->lines[prev_row]);
  int current_len = strlen(textarea->lines[textarea->cursor_row]);

  // 1 - get enough space for the previous line
  char* new_prev_line = (char*)realloc(textarea->lines[prev_row], (prev_len + current_len + 1) * sizeof(char));
  if (new_prev_line == NULL) return;
  textarea->lines[prev_row] = new_prev_line;

  // merge these two lines
  strcpy(textarea->lines[prev_row] + prev_len, textarea->lines[textarea->cursor_row]);
  free(textarea->lines[textarea->cursor_row]); // free the moved line

  // move all the lines, after the recently moved one, one position up
  for (int i = textarea->cursor_row; i < textarea->total_lines - 1; i++) {
    textarea->lines[i] = textarea->lines[i + 1];
  } 
  textarea->lines[textarea->total_lines - 1] = NULL; // clear the last index
  textarea->total_lines--;

  // move the cursor to where the lines were merged
  textarea->cursor_row--;
  textarea->cursor_col = prev_len;

  // update the visible area
  if (textarea->cursor_row < textarea->scroll_row) {
    textarea->scroll_row = textarea->cursor_row;
  }

  int usable_width = _textarea_get_usable_width(textarea);
  if (textarea->cursor_col >= textarea->scroll_col + usable_width) {
    textarea->scroll_col = textarea->cursor_col - usable_width + 1;
  }
  else if (textarea->cursor_col < textarea->scroll_col) {
    textarea->scroll_col = textarea->cursor_col;
  }
}

// Default behavior of the TextArea
void _textarea_actions(void* context, int c) {
  TextArea* textarea = (TextArea*)context;
  if (textarea == NULL) return;

  curs_set(0);

  int max_visible_lines = _textarea_get_max_visible_lines(textarea);
  int usable_width = _textarea_get_usable_width(textarea);

  // fix line number width
  if (textarea->line_number_width == 0) {
    _textarea_set_line_width(textarea);
  }

  switch (c) {
    case KEY_DOWN: textarea_handle_key_down(textarea, max_visible_lines); break; // move cursor DOWN
    case KEY_UP: textarea_handle_key_up(textarea); break; // move cursor UP
    case KEY_RIGHT: textarea_handle_key_right(textarea, usable_width); break; // move cursor RIGHT
    case KEY_LEFT: textarea_handle_key_left(textarea); break; // move cursor LEFT
    case 10: case KEY_ENTER: textarea_handle_key_enter(textarea); break; // when pressing ENTER

    case KEY_BACKSPACE:
      if (textarea->disabled == FALSE) {
        _textarea_remove_left_char(textarea);
      }
      break;

    // print the character
    default:
      if (c >= 32 && c <= 126 && textarea->disabled == FALSE) {
        _textarea_add_char(textarea, c);
      }
      break;
  }
  container_update(textarea);

  // render the visible text and line numbers
  for (int i = 0; i < max_visible_lines; i++) {
    int file_line_index = textarea->scroll_row + i;
    int screen_row = i + 1;

    // if the line exists, draw the line numebr and the text
    if (file_line_index < textarea->total_lines) {
      // print with specific content color if it is set
      if (textarea->content_color != NULL) {
        wattron(
          textarea->base.dwin,
          COLOR_PAIR(textarea->content_color_id)
        );
        wbkgd(
          textarea->base.dwin,
          COLOR_PAIR(textarea->base.color_pair_id)
        );
      }
      
      container_print(
        (Container*)textarea,
        FALSE,
        screen_row,
        textarea->line_number_width + 2,
        "%-*.*s",
        usable_width,
        usable_width,
        _textarea_get_printable_content(textarea, file_line_index)
      );

      // if we have the content specific color active
      if (textarea->content_color != NULL) {
        wattroff(textarea->base.dwin, COLOR_PAIR(textarea->content_color_id));
        wattroff(textarea->base.dwin, COLOR_PAIR(textarea->base.color_pair_id));
      }

      if (textarea->show_line_numbers) {
        // print the line number
        mvwprintw(textarea->base.dwin, screen_row, 1, "%*d", textarea->line_number_width, file_line_index + 1);
      }
    }
    // if the line was removed, overwrite the remaining region with empty chars
    else {
      container_print(
        (Container*)textarea,
        FALSE,
        screen_row,
        textarea->line_number_width + 2,
        "%-*s",
        usable_width,
        ""
      );

      // clear the line number
      if (textarea->show_line_numbers) {
        mvwprintw(textarea->base.dwin, screen_row, 1, "%*s", textarea->line_number_width, "");
      }
    }


  }

  // update cursor
  int visual_y = textarea->cursor_row - textarea->scroll_row + 1;
  int visual_x = textarea->line_number_width + textarea->cursor_col - textarea->scroll_col + 2; 
  wmove(textarea->base.dwin, visual_y, visual_x);

  // update screen buffers
  wnoutrefresh(textarea->base.dwin);
  doupdate();

  curs_set(1);
}

// create a new TextArea
TextArea* textarea_create(WINDOW* parent, int height, int width, int start_y, int start_x, const char* label, void (*callback)(int, void*)) {
  if (parent == NULL) return NULL;
  
  TextArea* textarea = (TextArea*)malloc(sizeof(TextArea));
  if (textarea == NULL) exit(1);

  textarea->lines = NULL;
  textarea->total_lines = 0;
  textarea->lines_capacity = 0;
  textarea->cursor_row = 0;
  textarea->cursor_col = FALSE;
  textarea->scroll_row = 0;
  textarea->scroll_col = FALSE;
  textarea->disabled = FALSE;
  textarea->show_line_numbers = FALSE;
  textarea->line_number_width = 2;

  // base container properties
  textarea->base.actions = _textarea_actions;
  textarea->base.has_border = TRUE;
  textarea->base.height = height; // top and bottom borders plus content in one line
  textarea->base.on_focus = callback;
  textarea->base.parent = parent;
  textarea->base.start_x = start_x;
  textarea->base.start_y = start_y;
  textarea->base.width = width;
  textarea->base.user_data = NULL;
  textarea->base.title = label;

  // extra style default value
  textarea->content_color = NULL;

  container_init(textarea);
  container_update(textarea);

  wmove(textarea->base.dwin, 1, 1);
  wnoutrefresh(textarea->base.dwin);
  doupdate();

  int initial_y = textarea->cursor_row + 1;
  int initial_x = textarea->line_number_width + 2 + textarea->cursor_col;
  wmove(textarea->base.dwin, initial_y, initial_x);

  return textarea;
}

// Init colors specific to the textarea.
// For now, the only extra color is for the content inside the textarea.
void  _textarea_init_content_style(TextArea* textarea) {
  if (textarea == NULL || !has_colors()) return;

  int fg_id = global_color_id_counter++;
  int bg_id = global_color_id_counter++;
  int pair_id = global_color_pair_counter++;

  // init colors
  _register_hex_color(fg_id, textarea->content_color);
  _register_hex_color(bg_id, textarea->base.background);

  // init color pair
  init_pair(pair_id, fg_id, bg_id);
  textarea->content_color_id = pair_id;
}

// Draw the textarea content on screen
void textarea_render(TextArea *textarea) {
  if (textarea == NULL) return;

  // init specific colors
  // the content can a different color from the borders
  _textarea_init_content_style(textarea);

  _textarea_actions(textarea, 0);
}

// Adds a new line under the cursor position (after the current active line)
// and move the cursor to the new line.
// On the first load, the cursor should appear at the top of the textarea.
void textarea_add_line(TextArea* textarea, const char* line) {
  if (textarea == NULL) exit(1);

  // make sure there is enough space for a new line
  if (textarea->total_lines >= textarea->lines_capacity) {
    // start with capacity for 8 lines
    int new_capacity = (textarea->lines_capacity == 0) ? 8 : textarea->lines_capacity * 2;

    char** temp_lines = (char**)realloc(textarea->lines, new_capacity * sizeof(char*));
    if (temp_lines == NULL) exit(1);

    textarea->lines = temp_lines;
    textarea->lines_capacity = new_capacity;
  }

  // allocate enough space for the new string
  int text_length = line ? strlen(line) : 0;
  textarea->lines[textarea->total_lines] = (char*)malloc((text_length + 1) * sizeof(char));
  if (textarea->lines[textarea->total_lines] == NULL) exit(1);

  // copy line to the textarea
  if (line != NULL) {
    strcpy(textarea->lines[textarea->total_lines], line);
  } else {
    // create a valid empty string on NULL
    textarea->lines[textarea->total_lines][0] = '\0';
  }

  // update lines total
  textarea->total_lines++;
}

// free memory requested by the textarea
void textarea_destroy(TextArea* textarea) {
  if (textarea == NULL) return;

  if (textarea->lines != NULL) {
    // clear each line
    for (int i = 0; i < textarea->total_lines ; i++) {
      if (textarea->lines[i] != NULL) {
        free(textarea->lines[i]);
      }
    }
    // free the lines table
    free(textarea->lines);
  }
  // free the structure
  free(textarea);
}
