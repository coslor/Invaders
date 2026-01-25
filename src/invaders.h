#ifndef INVADERS_H
#define INVADERS_H

#include "c64/types.h"
//#include <conio.h>
#include <stdlib.h>
#include <string.h>
//#include <assert.h>
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
#include "my_assert.h" 


#define NUM_ROWS 6
#define INVADERS_PER_ROW 6

// __export const bool CHANGE_COLOR_BY_ROW  =false;
// __export const bool MOVE_X_BY_ROW        =false;
// __export const bool CHANGE_IMAGE_BY_ROW  =true;

const byte SCANLINES_TO_DRAW_SPRITE=19;
const byte SCANLINES_PER_ROW=26;


byte current_row_num=0;

#define  IRQ_VECTOR *(void **)0x0314

//not sure this does anything
//#define SYNC_MAIN_THREAD    true

const byte MAX_IMAGE_HANDLES=2;

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
int         inv_spr_pos_x[TOTAL_INVS_SIZE];
byte        inv_spr_pos_y[TOTAL_INVS_SIZE];
byte        inv_row[TOTAL_INVS_SIZE];
byte        inv_col[TOTAL_INVS_SIZE];

signed int  row_y[NUM_ROWS];
byte        row_num_images[NUM_ROWS];
byte        row_image_handles[NUM_ROWS][MAX_IMAGE_HANDLES];
byte        row_image_num[NUM_ROWS];
byte        row_max_frames[NUM_ROWS];
byte        row_frame_num[NUM_ROWS];

bool        row_alive[NUM_ROWS];
//byte        row_max_inv_alive[NUM_ROWS];
//byte        row_min_inv_alive[NUM_ROWS];
//const byte   NO_INVS_ALIVE = -1;
//int         row_max_x[NUM_ROWS];
//int         row_min_x[NUM_ROWS];

const int   MIN_SPR_X = 25;
const int   MAX_SPR_X = 320;


//left & right-most borders for all rows
int         rows_max_spr_x = MIN_SPR_X;
int         rows_min_spr_x = MAX_SPR_X;


//byte        row_x_index[NUM_ROWS];
//byte        row_x_frame_speed[NUM_ROWS];                    //pixels moved / frame

int         rows_x_shift = 50;
//X motion speed
int         rows_x_frame_speed = 4;

byte        rows_frame_num = 0;
byte        rows_max_frames = 32;


bool        row_dirty[NUM_ROWS];  
byte        row_inv_index[NUM_ROWS];
byte        row_sprite_enable_mask[NUM_ROWS];

// const int   MIN_ROW_X_OFFSET=50;
// const int   MIN_ROW_X_OFFSET_PLUS_1 = MIN_ROW_X_OFFSET + 1;
// const int   MAX_ROW_X_OFFSET=100;
// const int   MAX_ROW_X_OFFSET_MINUS_1 = MAX_ROW_X_OFFSET - 1;


byte        col_x_index[INVADERS_PER_ROW];

//TODO come up with better names for these
const byte  MAX_FRAMES=32;      //determines speed of invader X motion
const byte  ROW_MAX_FRAMES=32;  //determines speed of row animations

bool        playing = true;
int         MAX_Y_ROW = 220;

const byte  Y_INC = 5;
const int   X_INC = 5;

enum PlayerObjectType {TYPE_SHIP, TYPE_BULLET};

typedef struct {
    int                 x = 0;
    signed int          speed_x = 0;
    byte                y = 0;
    signed int          speed_y = 0;
    bool                alive = false;
    byte                sprite_num = 0xff;
    byte                sprite_color = 1;
    byte                image_handle = 0xff;
    PlayerObjectType    type;
} PlayerObject;


PlayerObject    ship,bullet;

//byte collision_reg[NUM_ROWS];
byte collided_inv_index=-1;

// int         ship_x = 160;
// int         ship_speed_x = 0;
// byte        ship_y = 250;
// int         ship_speed_y = 0;
// bool        ship_alive        

// int         bullet_x = 160;
// int         bullet_speed_x = 0;
// byte        bullet_y=140;
// int         bullet_speed_y = -1;
// bool        bullet_alive = false;

//const 
unsigned int inv_start_line[NUM_ROWS] = {
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

const byte pow2[8] = {
    0b00000001,
    0b00000010,
    0b00000100,
    0b00001000,
    0b00010000,
    0b00100000,
    0b01000000,
    0b10000000,
};

void flip_image(byte index);
void print_invaders();
__forceinline void move_invader(byte index);
void raster_irq_handler();
bool set_next_irq(int rasterline, bool calling_from_irq);
void draw_sprite_row(byte current_row_num);
void init_invaders();
void init_sprites();
void flip_row(byte row);
void shoot_invader(byte row, byte col);
void bounce_rows();
void move_rows_down(byte px_down);
void read_joy();
void move_object(PlayerObject* obj);
void draw_object(PlayerObject* obj);
void fire_bullet(PlayerObject *obj);
void kill_bullet(PlayerObject *b);
byte wait_line_and_watch_for_collisions(int line);

#pragma compile("invaders.c")
#endif
