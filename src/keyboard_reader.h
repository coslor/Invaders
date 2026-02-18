#ifndef KEYBOARD_READER_H

#include <c64/cia.h>
#include <math.h>

//From (among other places):
//  https://sta.c64.org/cbm64kbdlay.html 
//  https://paulnotebook.net/wp-content/uploads/2019/01/keyboard-matrix.png

static const byte KR_COL_SPACE = 4;
static const byte KR_ROW_SPACE = 7;

static const byte KR_ROW_A = 1;
static const byte KR_COL_A = 2;

static const byte KR_ROW_D = 2;
static const byte KR_COL_D = 2;

static const byte KR_ROW_1 = 7;
static const byte KR_COL_1 = 0;

static const byte KR_ROW_2 = 7;
static const byte KR_COL_2 = 3;

static const byte KR_ROW_RETURN = 0;
static const byte KR_COL_RETURN = 1;

static const byte KR_ROW_COMMA = 5;
static const byte KR_COL_COMMA = 7;

static const byte KR_ROW_DOT = 5;
static const byte KR_COL_DOT = 4;

static const byte KR_ROW_COLON = 5;
static const byte KR_COL_COLON = 5;

static const byte KR_ROW_SEMI = 6;
static const byte KR_COL_SEMI = 2;


byte kr_log2(byte i);
bool kr_is_key_pressed(byte row, byte col);
bool kr_is_char_pressed(char c);
#define KEYBOARD_READER_H;
#endif