#pragma once

#include <ncurses.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define MAX_CONTAINERS 50

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
  void (*actions)(void*, int); // this should be used to implement intrinsic behavior into specific components within the lib
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


/**
 * Application ---------------------------------------------------------------
*/
Application* potatui_init();
void app_add_container(Application* app, Container* con);
void app_focus_on(Application* app, void* con);
void app_key_handle(Application* app, int c);
void potatui_loop(Application* app);
void potatui_end();
int ctrl(int c);

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
void _list_actions(void* app, int c);
List* list_create(WINDOW* parent, int height, int width, int start_y, int start_x, const char* title, bool has_border, void (*callback)(int, void*));
void list_item_add(List* list, const char* line, ...);
void list_item_select(List* list, const int position);


/**
 * TextInput -----------------------------------------------------------------
*/
// on creating, renamed title to label, height = 3, and has_border is always true
TextInput* textinput_create(WINDOW* parent, int width, int start_y, int start_x, const char* label, void (*callback)(int, void*));
void textinput_actions(void* app, int c);
void textinput_render(TextInput* input);

