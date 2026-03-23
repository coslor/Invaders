#ifndef INVADERS_H
#define INVADERS_H

//#section includes
#if 1 //includes
	#include "invaders_memory.h"
	#include "inv_assert.h" 
	#include "keyboard_reader.h"

	#include <c64/types.h>
	#include <string.h>
	#include <stdbool.h>

	#include <c64/joystick.h>
	#include <c64/keyboard.h>
	#include <c64/vic.h>
	#include <c64/sprites.h>
	#include <c64/memmap.h>
	#include <c64/rasterirq.h>
	#include <c64/cia.h>

	#include <audio/sidfx.h>

	#include <gfx/mcbitmap.h>
#endif
//#endsection

#pragma data(data)

//I used #defines here so that I could use them in the #if's later on
//  in the Invs static initializers.
#define     NUM_ROWS 			5
#define     INVADERS_PER_ROW 	6

// #ifdef USE_BORDER 
// 	static const bool DO_BORDER=true;
// #else
// 	static const bool DO_BORDER=false;
// #endif

//#ifdef USE_BORDER
    byte old_border_color=0;
//#endif

// #define     START_BORDER(color) #ifdef USE_BORDER  old_border_color=vic.color_border;vic.color_border = color;  #endif
// #define     END_BORDER #ifdef USE_BORDER  vic.color_border=old_border_color;  #endif

#define     MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define     MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define     IRQ_VECTOR *(void **)0x0314

const       byte MAX_IMAGE_HANDLES=2;

const int   MIN_Y = 50;   //MAX(SCANLINES_PER_ROW,50);
const int 	INV_MIN_Y = MIN_Y + 25;

const int   TOTAL_INVS_SIZE=NUM_ROWS * INVADERS_PER_ROW;

const int 	MIN_SPR_X = 35;
const int 	MAX_SPR_X = 320;
const int 	MIN_SPR_Y = 100;
const int 	MAX_SPR_Y = 255;

const byte  ROWS_MAX_FRAMES = 12;
const int   MAX_Y_ROW = 222;

const byte  Y_INC = 5;
const int   X_INC = 5;

const byte 	SPRITE_IMAGE_BASE = 0xc0; //0?

//26 is small ship, 24 is big one
const byte 	BIG_SHIP_OFFSET = 24;
const byte 	SMALL_SHIP_OFFSET = 26;
const byte 	SHIP_IMAGE_NUM = SPRITE_IMAGE_BASE + SMALL_SHIP_OFFSET;

const byte 	SMOOSHED_SHIP_IMAGE_NUM = SPRITE_IMAGE_BASE + 27;
const byte 	BULLET_IMAGE_NUM = SPRITE_IMAGE_BASE + 25;

const byte	BIG_INV_OFFSET = 0;
const byte 	SMALL_INV_OFFSET = 12;

const byte 	INVADER_IMAGE_BASE = SPRITE_IMAGE_BASE + SMALL_INV_OFFSET; //12;

const byte 	SHIP_OBJ_NUM = 0;
const byte 	BULLET_OBJ_NUM = 1;

const int 	SHIP_Y = 235;

const byte 	INVADER_SPRITE_HEIGHT = 10;


//TODO come up with better names for these
//const byte  MAX_FRAMES=32;      	//determines speed of invader X motion
const byte  ROW_MAX_FRAMES=64;  	//determines speed of row animations

const int 	NUM_OBJECTS = 2;

int        	current_row_num=0;

bool        inv_alive[TOTAL_INVS_SIZE];

//signed int  inv_x[TOTAL_INVS_SIZE]; // = {
//signed int  inv_y[TOTAL_INVS_SIZE]; // = {
//signed int  inv_speed_x[TOTAL_INVS_SIZE];
//signed int  inv_speed_y[TOTAL_INVS_SIZE];

byte        inv_sprite_num[TOTAL_INVS_SIZE];

// //Individual invaders' sprite x pos as drawn
// int         inv_spr_x[TOTAL_INVS_SIZE];

// //Individual invaders' sprite y pos as drawn
// byte        inv_spr_y[TOTAL_INVS_SIZE];

//byte        inv_row[TOTAL_INVS_SIZE];
//byte        inv_col[TOTAL_INVS_SIZE];

//Base (no shifts) Y value for the row
int         row_y[NUM_ROWS];

//TODO refactor so that we can make these const's

//the # of images in the animation loop for this row
byte        row_num_images[NUM_ROWS];
//the image "handles" (addr=handle*64+bank) for this row's animation frame
byte        row_image_handles[NUM_ROWS][MAX_IMAGE_HANDLES];

//byte        row_image_handle_row[NUM_ROWS];
//byte        row_image_row_index[NUM_ROWS];

//The current frame # in the row's animation loop
byte        row_image_num[NUM_ROWS];

//Each row's sprite color. Invaders don't use the sprite main color.
byte        row_color[NUM_ROWS];

//Each row's multicolor-color #1. Invaders should be drawn with mcolor0 & mcolor1.
byte        row_mcolor0[NUM_ROWS];

//Each row's multicolor-color #2
byte        row_mcolor1[NUM_ROWS];

//Does this row have any living Invaders in it?
bool        row_alive[NUM_ROWS];

//left-most x border for all rows;calculated in find_min_max_spr_x()
signed int  rows_min_spr_x;

//right-most x border for all rows;calculated in find_min_max_spr_x()
signed int  rows_max_spr_x;

//bottom-most border for all rows; calculated in find_rows_max_spr_y()
byte 		rows_max_spr_y;

//How far from col[x] each col should be drawn (for shifts back & forth horizontally)
signed int  cols_x_shift;

//TODO: this is more movement speed, not frame speed

//How far (in pixels) the rows move horizontally each frame
signed int  rows_x_frame_speed;             //X motion speed


//TODO Fix this--it's halfway all rows (for movement) and halfway by row (for images)

//The # of frames for Invaders between moving/flipping images

//This is for Invader rows movement
byte        rows_frame_num;// = 0;
//This is for invader row image flipping
byte        row_max_frames[NUM_ROWS];
byte        row_frame_num[NUM_ROWS];

//The index (from 0 to (NUM_ROWS*INVADERS_PER_ROW-1)) of the Invader
//	at col#0 for each row
byte        row_inv_index[NUM_ROWS];

	/*	Precalculate vic.spr_enable. The bits should be in this order (low to hi):
		0=ship
		1=bullet
		2=Invader #0 (leftmost)
		...
		7=Invader 5 (rightmost)
	*/
byte        row_sprite_enable_mask[NUM_ROWS];

/* Actual sprite y position for Invaders in each row (all the same for that row) */
int 		row_inv_spr_pos_y[NUM_ROWS];

byte 		row_invs_left_alive[NUM_ROWS];

/* handle = row_image_handles[spr_row][row_image_num[spr_row]] */
byte 		row_latest_handle[NUM_ROWS];

//The X coordinate, as drawn, for each column. All Invaders in a column have the same x.
int         col_inv_spr_pos_x[INVADERS_PER_ROW];

//the # of Invaders left alive in a column
byte        col_invs_left_alive[INVADERS_PER_ROW];

//The "raw" x coord for each column, not including shift. For the exact value as drawn in sprite coords, 
//	use col_inv_spr_pos_x
int         col_x[INVADERS_PER_ROW];

//Are we still playing? If not, stop moving stuff!
bool        playing;

////////////////////////
// Non-Invaders objects, like the ship & bullet
////////////////////////
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

//Should we kill this Object if it hits a border?
bool        obj_kill_on_border[NUM_OBJECTS];

PlayerObjectType obj_type[NUM_OBJECTS];

//Note that atm there is no provision for Object animation
byte        obj_image_handle[NUM_OBJECTS];

////
//Hardware collision detection stuff
////

//Needs to be >NUM_ROWS at least
static const byte MAX_FRAME_COLLISIONS=16;

//A list of all frame collisions registered during a frame
volatile byte frame_collision_mask[MAX_FRAME_COLLISIONS];

//A list of the rasterline that each collision took place on
volatile byte frame_collision_line[MAX_FRAME_COLLISIONS];

//The number of collisions so far this frame
volatile byte frame_collision_count;

// When searching for an Invader to mmatch a spr/spr collision, 
//	this is how close a row has to be, to count as a match
static const byte	FIND_ROW_DISTANCE=20;

// When searching for an Invader to mmatch a spr/spr collision, 
//	this is how close a column has to be, to count as a match
static const byte	FIND_COL_DISTANCE=20;

//
//raster_irq_line contains:
//- #0 is at the top of the screen, and it sets the screen to text mode (TODO implement this)
//- From 1-NUM_ROWS are for the Invader rows
//- #NUM_ROWS+1 is at the bottom of the screen, and it's for the ship
//
unsigned int raster_irq_line[NUM_ROWS + 3];//why +3 and not +2???


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

//How fast does the ship move horizontally?
static const int SHIP_SPEED = 2;

//How fast does the bullet move vertically?
static const int BULLET_SPEED = 3;


//////////////////////////////////
//	Sound effects
//	from DrMortalWombat's hscrollshmup game sample
//////////////////////////////////

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

// Sound effect for player explosion
const SIDFX	SIDFXBigExplosion[3] = {
	{
	1000, 1000, 
	SID_CTRL_GATE | SID_CTRL_SAW,
	SID_ATK_2 | SID_DKY_6,
	0xf0  | SID_DKY_168,
	-20, 0,
	4, 0
	},
	{
	1000, 1000, 
	SID_CTRL_GATE | SID_CTRL_NOISE,
	SID_ATK_2 | SID_DKY_6,
	0xf0  | SID_DKY_168,
	-20, 0,
	10, 0
	},
	{
	1000, 1000, 
	SID_CTRL_GATE | SID_CTRL_NOISE,
	SID_ATK_2 | SID_DKY_6,
	0xf0  | SID_DKY_1500,
	-10, 0,
	8, 80
	},	
};

const SIDFX Thump_F[1] = {
	{
		NOTE_F(4),						//freq
		0x0800,							//pwm
		//1000,
		SID_CTRL_RECT | SID_CTRL_GATE,	//ctrl, 
//		0x88,
		// 0x00,						//attdec, susrel,
		// SID_ATK_16 | SID_DKY_114,		//attdec
		//0xf0  | SID_DKY_1500,			//susrel
		0x0f,0x00,
		// -10,								//??dfreq,
		0,								//??dfreq,
		0,								//??dwpm,
		100,20,							//?time
		10								//?Priority
	}
};
const SIDFX Thump_E[1] = {
	{
		NOTE_E(4),						//freq
		0x0800,							//pwm
		//1000,
		SID_CTRL_RECT | SID_CTRL_GATE,	//ctrl, 
		0x0f,0x00,						//attdec, susrel,
		//SID_ATK_16 | SID_DKY_114,		//attdec
		//0xf0  | SID_DKY_1500,			//susrel
		// -10,								//??dfreq,
		0,								//??dfreq,
		0,								//??dwpm,
		100,20,							//?time
		10								//?Priority
	}
};
const SIDFX Thump_D[1] = {
	{
		NOTE_D(4),						//freq
		0x0800,							//pwm
		//1000,
		SID_CTRL_RECT | SID_CTRL_GATE,	//ctrl, 
		0x0f,0x00,						//attdec, susrel,
		//SID_ATK_16 | SID_DKY_114,		//attdec
		//0xf0  | SID_DKY_1500,			//susrel
		// -10,								//??dfreq,
		0,								//??dfreq,
		0,								//??dwpm,
		100,20,							//?time
		10								//?Priority
	}
};
const SIDFX Thump_C[1] = {
	{
		NOTE_C(4),						//freq
		0x0800,							//pwm
		//1000,
		SID_CTRL_RECT | SID_CTRL_GATE,	//ctrl, 
		0x0f,0x00,						//attdec, susrel,
		//SID_ATK_16 | SID_DKY_114,		//attdec
		//0xf0  | SID_DKY_1500,			//susrel
		// -10,								//??dfreq,
		0,								//??dfreq,
		0,								//??dwpm,
		100,20,							//?time
		10								//?Priority
	}
};

// const SIDFX const AllThumps[4][1] = {
// 	Thump_F,Thump_E,Thump_D,Thump_C
// };

const SIDFX	*InvaderDieFX	= SIDFXExplosion;
const SIDFX *PlayerFireFX	= SIDFXFire;
const SIDFX *PlayerDieFX	= SIDFXBigExplosion;
//const SIDFX *MusicFX		= ThumpOfDoom;


/////////////////////////////////////
//	BOMB VARIABLES
/////////////////////////////////////


static const byte MAX_BOMBS=2;
//How fast to the bombs fall?
static const byte BOMB_Y_SPEED=2;


bool 	bomb_alive[MAX_BOMBS];
//Each bomb's X coord, in *hires* coordinates, not sprite ones
int 	bomb_x[MAX_BOMBS];
//Each bomb's Y coord, in *hires* coordinates, not sprite ones
int 	bomb_y[MAX_BOMBS];
byte	bomb_y_speed[MAX_BOMBS];
byte	bomb_color[MAX_BOMBS];
byte	num_bombs;
// Each bomb only moves every so many frames
byte 	bomb_move_countdown[MAX_BOMBS];
const byte	MAX_BOMB_MOVE_COUNTDOWN=1;
//Drop a new bomb every so many frames

const byte MAX_BOMB_DROP_COUNTDOWN=30;
byte bomb_drop_countdown=MAX_BOMB_DROP_COUNTDOWN;



void flip_image(byte index);
void print_invaders();
__forceinline void move_invader(byte index);
void handle_irq();
void handle_raster_irq(byte raster);

bool set_next_raster_irq(unsigned int rasterline, bool calling_from_irq);
inline void draw_sprite_row(byte current_row_num);
void init_invaders();
void init_sprites();
void flip_row_image(byte row);
void kill_invader(byte row, byte col);
void poll_inputs(byte joy_num);
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
void handle_inputs(byte joy_num);

//byte kr_read_key();

// __forceinline const void START_BORDER(byte new_color);
// __forceinline const void END_BORDER();
inline void START_BORDER(VICColors);
inline void END_BORDER();

byte sid_rand();
void init_sid_rand();
unsigned int sid_int_rand();

void init_screen(byte num_stars);
void init_screen_mc(byte num_stars);
void clear_hires_screen();
void clear_text_screen();

void move_bombs();
bool add_bomb(int x, int y);
void kill_bomb(byte bomb_num);
void register_bomb_collision(byte coll_mask, byte raster);
void drop_bomb();

void music_init(char tune);
void music_play(void);

inline byte find_live_row();

#pragma compile("invaders.c")
#endif
