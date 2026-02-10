#ifndef MY_ASSERT_H
#define MY_ASSERT_H

#include <stddef.h>
#include <conio.h>
#include <c64/vic.h>
#include <stdio.h>


void my_assert(bool condition, void* message);
void soft_reset();
void init_my_assert();
void save_registers();
void restore_registers();
void screen_print(char *str, byte len);

#pragma compile("my_assert.c")
#endif
