#ifndef INVADERS_H
#define INVADERS_H

#include "invaders_memory.h"

#include <c64/types.h>
//#include <conio.h>
//#include <stdlib.h>
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
//#include <math.h>
//#include "invaders.h"
#include "inv_assert.h" 
#include "keyboard_reader.h"

#include <audio/sidfx.h>



//I used #defines here so that I could use them in the #if's later on
//  in the Invs static initializers.
#define     NUM_ROWS 5
#define     INVADERS_PER_ROW 6

#define 	DO_UNROLL true

#ifdef USE_BORDER 
	static const bool DO_BORDER=true;
#else
	static const bool DO_BORDER=false;
#endif

#ifdef USE_BORDER
    byte old_border_color=0;
#endif

// #define     START_BORDER(color) #ifdef USE_BORDER  old_border_color=vic.color_border;vic.color_border = color;  #endif
// #define     END_BORDER #ifdef USE_BORDER  vic.color_border=old_border_color;  #endif

#define     MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define     MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define     IRQ_VECTOR *(void **)0x0314

//////////////////////
// SCANLINES
//////////////////////
static const byte  SCANLINES_TO_DRAW_SPRITE=15;

static const byte  SCANLINES_PER_ROW=10 + SCANLINES_TO_DRAW_SPRITE;
/////////////////////


const       byte MAX_IMAGE_HANDLES=2;

const int   MIN_Y = 50;   //MAX(SCANLINES_PER_ROW,50);
const int 	INV_MIN_Y = MIN_Y + 25;

const int   TOTAL_INVS_SIZE=NUM_ROWS * INVADERS_PER_ROW;

const int 	MIN_SPR_X = 35;
const int 	MAX_SPR_X = 320;
const byte  ROWS_MAX_FRAMES = 32;
const int   MAX_Y_ROW = 222;

const byte  Y_INC = 5;
const int   X_INC = 5;

const byte 	SPRITE_IMAGE_BASE = 0x80;

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


//TODO come up with better names for these
//const byte  MAX_FRAMES=32;      //determines speed of invader X motion
const byte  ROW_MAX_FRAMES=32;  //determines speed of row animations

const int NUM_OBJECTS = 2;


int        current_row_num=0;

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
signed int  rows_max_spr_x;// = MIN_SPR_X;
signed int  rows_min_spr_x;// = MAX_SPR_X;

signed int  rows_x_shift;

//TODO: this is more movement speed, not frame speed
signed int  rows_x_frame_speed;             //X motion speed


//TODO Fix this--it's halfway all rows (for movement) and halfway by row (for images)

//# of frames for Invaders between moving/flipping images
//This is for Invader rows movement
byte        rows_frame_num;// = 0;
//This is for invader row image flipping
byte        row_max_frames[NUM_ROWS];
byte        row_frame_num[NUM_ROWS];

byte        row_inv_index[NUM_ROWS];
byte        row_sprite_enable_mask[NUM_ROWS];

int         rows_inv_spr_pos_x[INVADERS_PER_ROW];

byte        col_invs_left_alive[INVADERS_PER_ROW];
int         col_x[INVADERS_PER_ROW];


bool        playing;


//__export byte vic_copy[0x2f];

//We can actually leave off the typedef for Oscar, but it pisses off the C checker
typedef enum PlayerObjectType {TYPE_SHIP, TYPE_BULLET} PlayerObjectType;

signed int  obj_x[NUM_OBJECTS]; 
signed int  obj_speed_x[NUM_OBJECTS];
signed int  obj_y[NUM_OBJECTS];
signed int  obj_speed_y[NUM_OBJECTS];
bool        obj_alive[NUM_OBJECTS];
byte        obj_sprite_num[NUM_OBJECTS];
byte        obj_sprite_color[NUM_OBJECTS];
byte        obj_sprite_mcolor0[NUM_OBJECTS];
byte        obj_sprite_mcolor1[NUM_OBJECTS];
bool        obj_kill_on_border[NUM_OBJECTS];

PlayerObjectType obj_type[NUM_OBJECTS];

byte        obj_image_handle[NUM_OBJECTS];

int collided_inv_index=0xff;

//
//inv_start_line contains 2 extra elements:
//- #0 is at the top of the screen, and it sets the screen to text mode
//- From 1-NUM_ROWS are for the Invader rows
//- #NUM_ROWS+1 is at the bottom of the screen, and it's for the ship
//
unsigned int inv_start_line[NUM_ROWS + 3];


// Powers of 2, for quick lookups
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

//from DrMortalWombat's hscrollshmup game sample
// Sound effect for a player shot
const SIDFX	SIDFXFire[1] = {{
	8000, 1000, 
	SID_CTRL_GATE | SID_CTRL_SAW,
	SID_ATK_16 | SID_DKY_114,
	0x40  | SID_DKY_750,
	-80, 0,
	4, 30
}};

// Sound effect for enemy explosion
const SIDFX	SIDFXExplosion[1] = {{
	1000, 1000, 
	SID_CTRL_GATE | SID_CTRL_NOISE,
	SID_ATK_2 | SID_DKY_6,
	0xf0  | SID_DKY_1500,
	-20, 0,
	8, 40
}};



void flip_image(byte index);
void print_invaders();
__forceinline void move_invader(byte index);
void raster_irq_handler();
bool set_next_irq(unsigned int rasterline, bool calling_from_irq);
void draw_sprite_row(byte current_row_num);
void init_invaders();
void init_sprites();
void flip_row_image(byte row);
void shoot_invader(byte row, byte col);
void poll_inputs(char joy_num);
void move_object(byte obj_num);
void draw_object(int obj_num);
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

byte kr_read_key();

__forceinline const void START_BORDER(byte new_color);
__forceinline const void END_BORDER();


byte sid_rand();
byte init_sid_rand();
unsigned int sid_int_rand();

void init_screen(byte num_stars);
void clear_hires_screen();
void clear_text_screen();


#pragma compile("invaders.c")
#endif
