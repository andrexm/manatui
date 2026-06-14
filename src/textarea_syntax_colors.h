#pragma once

#include <stdlib.h>

// all colors should be hexadecimal
typedef struct {
  char* keyword_color;
  char* function_color;
  char* number_color; // for numbers inside the text
  char* line_number_color; // the column of numbers on the textarea
  char* symbol_color;
  char* background_color;
  char* background_variant_color;
  char* comment_color;
  char* string_color; // colorize inside double quotes
  char* char_color; // colorize a single char inside single quotes
  char* text_color; // other words, like variable names
} TextareaTheme;

const TextareaTheme default_theme = { "#009f80", "#5082be", "#cdaa19", "#76797a", "#999999", "#151517", "#303030", "#514f56", "#ff5822", "#f9a337", "#b2b2b2" };

typedef struct {
  const char** keywords;
  int total;
} LanguageData;

static const char* c_keywords_list[] = {
  "auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else",
  "enum", "extern", "float", "for", "goto", "if", "int", "long", "register", "return", "short",
  "signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned", "void",
  "volatile", "while", "include", "define", "pragma", "once"
};

const LanguageData clang = {
  c_keywords_list,
  sizeof(c_keywords_list) / sizeof(char*)
};
