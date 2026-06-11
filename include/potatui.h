#pragma once

#include <ncurses.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define MAX_CONTAINERS 50

// List of objects to be freed at the end (at potatui_end())
typedef struct {
  void** pointers;
  int capacity;
  int total;
} DeferFreeList;

// Container - the base for all other components
typedef struct {
  // dimensions of the Derwin
  int height;
  int width;

  // the start of the visible area at the screen (physical coordinates)
  int start_x;
  int start_y;

  // colors
  const char* foreground; // "#0099ff"
  const char* background; // "#000000"
  int color_pair_id; // the framework generates

  const char* title;
  bool has_border;
  bool is_focused;
  void (*on_focus)(int, void*);
  void (*actions)(void*, unsigned int); // this should be used to implement intrinsic behavior into specific components within the lib
  void* user_data; // this can be anything the user needs to manipulate inside the (*on_focus)(int, void*)
  WINDOW* parent;
  WINDOW* dwin;
} Container;

// Application - where everything happens
typedef struct {
  int height;
  int width;
  Container* container_list[MAX_CONTAINERS]; // a list for focusable containers
  Container* focused_container;
  int total_containers; // the amount of focused containers
  DeferFreeList* defer_list;
} Application;

// Button type
typedef struct {
  Container base;
  char label[50];
} Button;

// List type
typedef struct {
  Container base;
  char** content;
  int items; // the amount of items
  int selected;
  int scroll_top;
} List;

// Input type
typedef struct {
  Container base;
  char content[255];
  int content_size;
  int cursor_pos; // cursor position inside the text
  int text_pos; // start printing the text after this position
  bool disabled;
} TextInput;

// TextArea type
typedef struct {
  Container base;
  char** lines; // array of lines
  int total_lines;
  int lines_capacity;

  // where the cursor is
  int cursor_row;
  int cursor_col;

  // from where we are printing
  int scroll_row;
  int scroll_col;

  // other settings
  bool show_line_numbers;
  int line_number_width;
  bool disabled;
  char* content_color;
  int content_color_id;
} TextArea;


/**
 * Application ---------------------------------------------------------------
*/
Application* potatui_init();
void app_add_container(Application* app, Container* con);
void app_focus_on(Application* app, void* con);
void app_key_handle(Application* app, unsigned int c);
void potatui_loop(Application* app);
void potatui_end(Application* app);
unsigned int ctrl(unsigned int c);
void app_defer_free(Application* app, void* ptr);

/**
 * Containers ----------------------------------------------------------------
*/
void container_init(void* obj);
Container* container_create(WINDOW* parent, int height, int width, int start_y, int start_x, const char* title, bool has_border, void (*callback)(int, void*));
void vcontainer_print(Container* con, bool break_line, int y, int x, const char* format, va_list args);
void container_print(Container* con, bool break_line, int y, int x, const char* format, ...);
void container_update(void* obj);
void container_apply_style(void* con);

/**
 * Buttons -------------------------------------------------------------------
*/
Button* button_create(WINDOW* parent, int height, int width, int start_y, int start_x, const char* label, void (*callback)(int, void*));
void button_select(Application* app, Container* btn);

/**
 * Lists ---------------------------------------------------------------------
*/
void list_render(List* list); 
List* list_create(WINDOW* parent, int height, int width, int start_y, int start_x, const char* title, bool has_border, void (*callback)(int, void*));
void list_item_add(List* list, const char* line, ...);
void list_item_select(List* list, const int position);


/**
 * TextInput -----------------------------------------------------------------
*/
// on creating, renamed title to label, height = 3, and has_border is always true
TextInput* textinput_create(WINDOW* parent, int width, int start_y, int start_x, const char* label, void (*callback)(int, void*));
void textinput_render(TextInput* input);


/**
 * TextArea ------------------------------------------------------------------
*/
TextArea* textarea_create(WINDOW* parent, int height, int width, int start_y, int start_x, const char* label, void (*callback)(int, void*));
void textarea_render(TextArea* textarea);
void textarea_add_line(TextArea* textarea, const char* line);
void textarea_destroy(TextArea* textarea);
void textarea_handle_key_down(TextArea* textarea, int max_visible_lines);
void textarea_handle_key_up(TextArea* textarea);
void textarea_handle_key_enter(TextArea* textarea);
int textarea_get_usable_width(TextArea* textarea);
