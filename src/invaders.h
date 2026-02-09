#ifndef INVADERS_H
#define INVADERS_H

#include "invaders_memory.h"

#include "c64/types.h"
//#include <conio.h>
#include <stdlib.h>
#include <string.h>
//#include <assert.h>
//#include <stdio.h>
#include <stdbool.h>
#include <c64/joystick.h>
#include <c64/keyboard.h>
#include <c64/vic.h>
#include <c64/sprites.h>
#include <c64/memmap.h>
#include <c64/rasterirq.h>
#include <c64/cia.h>
#include <math.h>
//#include "invaders.h"
#include "my_assert.h" 


//I used #defines here so that I could use them in the #if's later on
//  in the Invs static initializers.
#define     NUM_ROWS 6
#define     INVADERS_PER_ROW 6

#ifdef USE_BORDER
    byte old_border_color;
#endif
#define     START_BORDER(color) //nothing #endif
// #ifdef USE_BORDER { old_border_color=vic.color_border;vic.color_border = color; } #else //nothing #endif
#define     END_BORDER //nothing #endif 
// #ifdef USE_BORDER { vic.color_border=old_border_color; } #else //nothing #endif

const byte  SCANLINES_TO_DRAW_SPRITE=16;
const byte  SCANLINES_PER_ROW=10 + SCANLINES_TO_DRAW_SPRITE;

byte        current_row_num=0;

#define     IRQ_VECTOR *(void **)0x0314

const       byte MAX_IMAGE_HANDLES=2;

#define     MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define     MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

const int   MIN_Y=MAX(SCANLINES_PER_ROW,50);

const int   TOTAL_INVS_SIZE=NUM_ROWS * INVADERS_PER_ROW;

const signed int MIN_SPR_X = 35;
const signed int MAX_SPR_X = 320;


bool        inv_alive[TOTAL_INVS_SIZE]; // = {
//signed int  inv_x[TOTAL_INVS_SIZE]; // = {
//signed int  inv_y[TOTAL_INVS_SIZE]; // = {
//signed int  inv_speed_x[TOTAL_INVS_SIZE];
//signed int  inv_speed_y[TOTAL_INVS_SIZE];
byte        inv_sprite_num[TOTAL_INVS_SIZE];
//int         inv_spr_pos_x[TOTAL_INVS_SIZE];
//byte        inv_spr_pos_y[TOTAL_INVS_SIZE];
//byte        inv_row[TOTAL_INVS_SIZE];
//byte        inv_col[TOTAL_INVS_SIZE];

int         row_y[NUM_ROWS];

//TODO refactor so that we can make these const's
byte        row_num_images[NUM_ROWS];
byte        row_image_handles[NUM_ROWS][MAX_IMAGE_HANDLES];
byte        row_image_handle_row[NUM_ROWS];
byte        row_image_row_index[NUM_ROWS];
byte        row_image_num[NUM_ROWS];

byte        row_color[NUM_ROWS];
byte        row_mcolor0[NUM_ROWS];              //Invaders should be drawn with mcolor0 & mcolor1
byte        row_mcolor1[NUM_ROWS];

bool        row_alive[NUM_ROWS];

//left & right-most borders for all rows
signed int  rows_max_spr_x = MIN_SPR_X;
signed int  rows_min_spr_x = MAX_SPR_X;

signed int  rows_x_shift;

//TODO: this is more movement speed, not frame speed
signed int  rows_x_frame_speed;             //X motion speed


//TODO Fix this--it's halfway all rows (for movement) and halfway by row (for images)

//# of frames for Invaders between moving/flipping images
//This is for Invader rows movement
byte        rows_frame_num = 0;
const byte  ROWS_MAX_FRAMES = 32;
//This is for invader row image flipping
byte        row_max_frames[NUM_ROWS];
byte        row_frame_num[NUM_ROWS];

byte        row_inv_index[NUM_ROWS];
byte        row_sprite_enable_mask[NUM_ROWS];

int         rows_inv_spr_pos_x[INVADERS_PER_ROW];

byte        col_invs_left_alive[INVADERS_PER_ROW];
int         col_x[INVADERS_PER_ROW];

//TODO come up with better names for these
//const byte  MAX_FRAMES=32;      //determines speed of invader X motion
const byte  ROW_MAX_FRAMES=32;  //determines speed of row animations

bool        playing;
const int   MAX_Y_ROW = 222;

const byte  Y_INC = 5;
const int   X_INC = 5;

const byte SPRITE_IMAGE_BASE = 0;
//const byte SPRITE_IMAGE_BASE = 128;

//26 is small ship, 24 is big one
const byte SHIP_IMAGE_NUM = 26;
const byte SMOOSHED_SHIP_IMAGE_NUM = 27;
const byte BULLET_IMAGE_NUM = 25;
//0 is the big Invaders, 12 is the small ones
const byte INVADER_IMAGE_BASE = 12;

const byte SHIP_OBJ_NUM = 0;
const byte BULLET_OBJ_NUM = 1;

const int SHIP_Y = 230;

const byte INVADER_SPRITE_HEIGHT = 10;

//__export byte vic_copy[0x2f];

enum PlayerObjectType {TYPE_SHIP, TYPE_BULLET};

const int NUM_OBJECTS = 2;

signed int  obj_x[NUM_OBJECTS]              = {160,         0};
signed int  obj_speed_x[NUM_OBJECTS]        = {0,           0};
signed int  obj_y[NUM_OBJECTS]              = {230,         230};
signed int  obj_speed_y[NUM_OBJECTS]        = {0,           0};
bool        obj_alive[NUM_OBJECTS]          = {true,        false};
byte        obj_sprite_num[NUM_OBJECTS]     = {0,           1};
byte        obj_sprite_color[NUM_OBJECTS]   = {VCOL_WHITE,  VCOL_WHITE};
byte        obj_sprite_mcolor0[NUM_OBJECTS] = {VCOL_GREEN,  VCOL_GREEN};
byte        obj_sprite_mcolor1[NUM_OBJECTS] = {VCOL_RED,    VCOL_RED};
bool        obj_kill_on_border[NUM_OBJECTS] = {false,       true};

PlayerObjectType obj_type[NUM_OBJECTS]      = {TYPE_SHIP,   TYPE_BULLET};

byte        obj_image_handle[NUM_OBJECTS]   = {SPRITE_IMAGE_BASE + SHIP_IMAGE_NUM, 
                                               SPRITE_IMAGE_BASE + SHIP_IMAGE_NUM};

// typedef struct {
//     signed int          x               = 0;
//     signed int          speed_x         = 0;
//     byte                y               = 0;
//     signed int          speed_y         = 0;
//     bool                alive           = false;
//     byte                sprite_num      = 0xff;
//     byte                sprite_color    = VCOL_WHITE;
//     byte                sprite_mcolor0  = VCOL_GREEN;
//     byte                sprite_mcolor1  = VCOL_RED;
//     byte                image_handle    = 0xff;
//     bool                kill_on_border  = false;
//     PlayerObjectType    type;
// } PlayerObject;


// PlayerObject    ship,bullet;

//byte collision_reg[NUM_ROWS];
int collided_inv_index=0xff;


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

// 
//byte            ship_color    = VCOL_WHITE;
//byte            ship_mcolor0  = VCOL_GREEN;
//byte            ship_mcolor1  = VCOL_RED;

unsigned int inv_start_line[NUM_ROWS + 2]; // = {
//     //0,
//     //MIN_Y-SCANLINES_PER_ROW-1,
//     MIN_Y-SCANLINES_TO_DRAW_SPRITE, 
//     #if (NUM_ROWS>1)
//     //MIN_Y-6,
//     MIN_Y+SCANLINES_PER_ROW*1-SCANLINES_TO_DRAW_SPRITE, 
//     #endif
//     #if (NUM_ROWS > 2)
//     //MIN_Y+SCANLINES_PER_ROW-6,
//     MIN_Y+SCANLINES_PER_ROW*2-SCANLINES_TO_DRAW_SPRITE, 
//     #endif
//     #if (NUM_ROWS > 3)
//     //MIN_Y+SCANLINES_PER_ROW*2-6,
//     MIN_Y+SCANLINES_PER_ROW*3-SCANLINES_TO_DRAW_SPRITE, 
//     #endif
//     #if (NUM_ROWS > 4)
//     MIN_Y+SCANLINES_PER_ROW*4-SCANLINES_TO_DRAW_SPRITE,
//     #endif
//     #if (NUM_ROWS > 5)
//     //MIN_Y+SCANLINES_PER_ROW*5,
//     MIN_Y+SCANLINES_PER_ROW*5-SCANLINES_TO_DRAW_SPRITE,
//     #endif
//     230
// };

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
void flip_row_image(byte row);
void shoot_invader(byte row, byte col);
void poll_inputs(char joy_num);
void move_object(byte obj_num);
void draw_object(byte obj_num);
void fire_bullet(byte obj_num);
void kill_bullet(byte obj_num);
byte wait_line_and_watch_for_collisions(int line);
void set_sprites_for_all();
//void take_vic_snapshot();

//All these return true if OK and false if out of bounds
bool move_invaders();
bool bounce_rows();
bool move_rows_down(byte px_down);

void display_logo();

void kill_object(byte obj_num);
void game_over();

char getch_with_keybounce();
bool handle_inputs(byte joy_num);

#pragma compile("invaders.c")
#endif
