#ifndef MY_ASSERT_H
#define MY_ASSERT_H

#include <stddef.h>
#include <conio.h>
#include <c64/vic.h>
#include <stdio.h>


void inv_assert(bool condition, void* message, ...);

#pragma compile("inv_assert.c")
#endif
