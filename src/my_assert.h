#ifndef MY_ASSERT_H
#define MY_ASSERT_H

#include <stddef.h>
#include <conio.h>


void my_assert(bool condition, void* message);

#pragma compile("my_assert.c")
#endif
