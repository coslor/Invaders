#ifndef KEYBOARD_READER_H

#include <c64/cia.h>
#include <math.h>

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

byte kr_log2(byte i);
bool kr_is_key_pressed(byte row, byte col);
bool kr_is_char_pressed(char c);
#define KEYBOARD_READER_H;
#endif