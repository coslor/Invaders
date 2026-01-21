#ifndef INVADERS_H
#define INVADERS_H

#include "c64/types.h"
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <c64/joystick.h>
#include <c64/vic.h>
#include <c64/sprites.h>
#include <c64/memmap.h>
#include <c64/rasterirq.h>
#include <c64/cia.h>
#include <math.h>
//#include "invaders.h"


#define NUM_ROWS 6
//const int INVADERS_PER_ROW=5;
#define INVADERS_PER_ROW 6

__export const bool CHANGE_COLOR_BY_ROW  =false;
__export const bool MOVE_X_BY_ROW        =false;
__export const bool CHANGE_IMAGE_BY_ROW  =true;

const int SCANLINES_TO_DRAW_SPRITE=21;
const int SCANLINES_PER_ROW=22;

int current_row_num=0;

#define  IRQ_VECTOR *(void **)0x0314

//not sure this does anything
//#define SYNC_MAIN_THREAD    true

const int MAX_IMAGE_HANDLES=2;

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

const int MIN_Y=MAX(SCANLINES_PER_ROW,50);

const int TOTAL_INVS_SIZE=NUM_ROWS * INVADERS_PER_ROW;

bool        inv_alive[TOTAL_INVS_SIZE]; // = {
signed int  inv_x[TOTAL_INVS_SIZE]; // = {
signed int  inv_y[TOTAL_INVS_SIZE]; // = {
signed int  inv_speed_x[TOTAL_INVS_SIZE];
signed int  inv_speed_y[TOTAL_INVS_SIZE];
byte        inv_num_images[TOTAL_INVS_SIZE];
byte        inv_image_handles[TOTAL_INVS_SIZE][MAX_IMAGE_HANDLES];
byte        inv_image_num[TOTAL_INVS_SIZE];
byte        inv_max_frames[TOTAL_INVS_SIZE];
byte        inv_sprite_num[TOTAL_INVS_SIZE];
byte        inv_color[TOTAL_INVS_SIZE];
byte        inv_frame_num[TOTAL_INVS_SIZE];
signed int  inv_old_x[TOTAL_INVS_SIZE];
signed int  inv_old_y[TOTAL_INVS_SIZE];

signed int  row_y[NUM_ROWS];
byte        row_num_images[NUM_ROWS];
byte        row_image_handles[NUM_ROWS][MAX_IMAGE_HANDLES];
byte        row_image_num[NUM_ROWS];
byte        row_max_frames[NUM_ROWS];
byte        row_frame_num[NUM_ROWS];
byte        row_x_offset[NUM_ROWS];

const int   MIN_ROW_X_OFFSET=50;
const int   MIN_ROW_X_OFFSET_PLUS_1 = MIN_ROW_X_OFFSET + 1;
const int   MAX_ROW_X_OFFSET=130;
const int   MAX_ROW_X_OFFSET_MINUS_1 = MAX_ROW_X_OFFSET - 1;


byte        row_x_frame_speed[NUM_ROWS];

byte        col_x_offset[INVADERS_PER_ROW];

const byte  MAX_FLIP_FRAMES=32;

const unsigned int inv_start_line[NUM_ROWS] = {
    //0,
    //MIN_Y-SCANLINES_PER_ROW-1,
    MIN_Y-SCANLINES_TO_DRAW_SPRITE, 
    #if (NUM_ROWS>1)
    //MIN_Y-6,
    MIN_Y+SCANLINES_PER_ROW*1-SCANLINES_TO_DRAW_SPRITE, 
    #endif
    #if (NUM_ROWS > 2)
    //MIN_Y+SCANLINES_PER_ROW-6,
    MIN_Y+SCANLINES_PER_ROW*2-SCANLINES_TO_DRAW_SPRITE, 
    #endif
    #if (NUM_ROWS > 3)
    //MIN_Y+SCANLINES_PER_ROW*2-6,
    MIN_Y+SCANLINES_PER_ROW*3-SCANLINES_TO_DRAW_SPRITE, 
    #endif
    #if (NUM_ROWS > 4)
    MIN_Y+SCANLINES_PER_ROW*4-SCANLINES_TO_DRAW_SPRITE,
    #endif
    #if (NUM_ROWS > 5)
    //MIN_Y+SCANLINES_PER_ROW*5,
    MIN_Y+SCANLINES_PER_ROW*5-SCANLINES_TO_DRAW_SPRITE
    #endif
};

void flip_image(int offset);
void print_invaders();
__forceinline void move_invader(int offset);

void raster_irq_handler();

void set_next_irq(int rasterline);

void draw_sprite_row(byte offset, byte row, bool change_color_by_row, bool move_x_by_row, bool change_image_by_row);

void init_invaders();

void init_sprites();

byte flip_row(byte row);

#pragma compile("invaders.c")
#endif
