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


#define NUM_ROWS 4
//const int INVADERS_PER_ROW=5;
#define INVADERS_PER_ROW 6

const bool CHANGE_COLOR_BY_ROW  =true;
const bool MOVE_X_BY_ROW        =true;
const bool CHANGE_IMAGE_BY_ROW  =true;

const int SCANLINES_TO_DRAW_SPRITE =35;
const int SCANLINES_PER_ROW=45;

int current_row_num=0;

#define  IRQ_VECTOR *(void **)0x0314

const int MAX_IMAGE_HANDLES=8;

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

const int MIN_Y=MAX(SCANLINES_PER_ROW,50);

// typedef struct  {
//     bool        alive=false;
//     signed int x=0;
//     signed int y=0;
//     signed int  speed_x=0,speed_y=0; //pixels per frame

//     byte        num_images=0;
//     byte        image_handles[MAX_IMAGE_HANDLES];
//     byte        image_num=0;
//     byte        max_frames=1;

//     byte        sprite_num=0;

//     byte        color=1;
//     ////////////auto-initialized from here down////////////////
//     long        frame_num=0;

//     signed int  old_x=0;
//     signed int  old_y=0;

// } Invader;

bool        inv_alive[NUM_ROWS][INVADERS_PER_ROW]; // = {
            //     {
            //         true, true, true, true, true, true 
            //     },
            //     {
            //         true, true, true, true, true, true 
            //     },
            //     {
            //         true, true, true, true, true, true 
            //     },
            //     {
            //         true, true, true, true, true, true 
            //     }
            // };

signed int  inv_x[NUM_ROWS][INVADERS_PER_ROW]; // = {
            //     {
            //          25, 50, 75,100,125,150
            //     },
            //     {
            //          35, 60, 85,110,135,160
            //     },
            //     {
            //          45, 70, 95,120,145,170
            //     },
            //     {
            //          55, 80,105,130,125,150
            //     },
            // };
            
signed int  inv_y[NUM_ROWS][INVADERS_PER_ROW]; // = {
            //     { 
            //         MIN_Y, 
            //         MIN_Y, 
            //         MIN_Y, 
            //         MIN_Y, 
            //         MIN_Y, 
            //         MIN_Y
            //     },
            //     {
            //         MIN_Y+SCANLINES_PER_ROW*1,
            //         MIN_Y+SCANLINES_PER_ROW*1,
            //         MIN_Y+SCANLINES_PER_ROW*1,
            //         MIN_Y+SCANLINES_PER_ROW*1,
            //         MIN_Y+SCANLINES_PER_ROW*1,
            //         MIN_Y+SCANLINES_PER_ROW*1
            //     },
            //     {
            //         MIN_Y+SCANLINES_PER_ROW*2,
            //         MIN_Y+SCANLINES_PER_ROW*2,
            //         MIN_Y+SCANLINES_PER_ROW*2,
            //         MIN_Y+SCANLINES_PER_ROW*2,
            //         MIN_Y+SCANLINES_PER_ROW*2,
            //         MIN_Y+SCANLINES_PER_ROW*2,
            //     },
            //     #if (NUM_ROWS > 3)
            //     {
            //         MIN_Y+SCANLINES_PER_ROW*3,
            //         MIN_Y+SCANLINES_PER_ROW*3,
            //         MIN_Y+SCANLINES_PER_ROW*3,
            //         MIN_Y+SCANLINES_PER_ROW*3,
            //         MIN_Y+SCANLINES_PER_ROW*3,
            //         MIN_Y+SCANLINES_PER_ROW*3,
            //     },
            //     #endif
            //     #if (NUM_ROWS > 4)
            //     {
            //         MIN_Y+SCANLINES_PER_ROW*4,
            //         MIN_Y+SCANLINES_PER_ROW*4,
            //         MIN_Y+SCANLINES_PER_ROW*4,
            //         MIN_Y+SCANLINES_PER_ROW*4,
            //         MIN_Y+SCANLINES_PER_ROW*4,
            //         MIN_Y+SCANLINES_PER_ROW*4,
            //     },
            //     #endif
            //     #if (NUM_ROWS > 5)
            //     {
            //         MIN_Y+SCANLINES_PER_ROW*5,
            //         MIN_Y+SCANLINES_PER_ROW*5,
            //         MIN_Y+SCANLINES_PER_ROW*5,
            //         MIN_Y+SCANLINES_PER_ROW*5,
            //         MIN_Y+SCANLINES_PER_ROW*5,
            //         MIN_Y+SCANLINES_PER_ROW*5,
            //     }
            //     #endif
            // };
signed int  inv_speed_x[NUM_ROWS][INVADERS_PER_ROW];
signed int  inv_speed_y[NUM_ROWS][INVADERS_PER_ROW];
byte        inv_num_images[NUM_ROWS][INVADERS_PER_ROW];
byte        inv_image_handles[NUM_ROWS][INVADERS_PER_ROW][MAX_IMAGE_HANDLES];
byte        inv_image_num[NUM_ROWS][INVADERS_PER_ROW];
byte        inv_max_frames[NUM_ROWS][INVADERS_PER_ROW];
byte        inv_sprite_num[NUM_ROWS][INVADERS_PER_ROW];
byte        inv_color[NUM_ROWS][INVADERS_PER_ROW];
byte        inv_frame_num[NUM_ROWS][INVADERS_PER_ROW];
signed int  inv_old_x[NUM_ROWS][INVADERS_PER_ROW];
signed int  inv_old_y[NUM_ROWS][INVADERS_PER_ROW];



//const int COL_SIZE=sizeof(Invader);
const int ROW_SIZE=INVADERS_PER_ROW;
const int TOTAL_INVS_SIZE=NUM_ROWS * INVADERS_PER_ROW;

////
// NOTE: C's static array initializers will allow *fewer* elements than
//          specified in the array definition, but will choke on *more*
////          
// Invader invaders[NUM_ROWS][INVADERS_PER_ROW] = {
//     {

// //NOTE: one Invader per row is disabled on purpose to show that that works
// //alive,x,y,speed_x,speed_y,num_images,image_handles,image_num (to start),max_frames,sprite_num,color
//         {true, 25, MIN_Y, 1,0,2,{128,129},0, 32, 2, 1},
//         {true, 50, MIN_Y, 1,0,2,{128,129},1, 32, 3, 2},
//         {true, 75, MIN_Y, 1,0,2,{128,129},0, 32, 4, 3},
//         {true,100, MIN_Y, 1,0,2,{128,129},1, 32, 5, 4},
//         {true,125, MIN_Y, 1,0,2,{128,129},0, 32, 6, 5},
//         {true,150, MIN_Y, 1,0,2,{128,129},1, 32, 7, 6},
//     },
//     #if (NUM_ROWS > 1)
//     {
//         {true, 50, MIN_Y+SCANLINES_PER_ROW*1, 1,0,2,{128,129},1,32, 2, 1},
//         {true,100, MIN_Y+SCANLINES_PER_ROW*1, 1,0,2,{128,129},0,32, 3, 2},
//         {true,150, MIN_Y+SCANLINES_PER_ROW*1, 1,0,2,{128,129},1,32, 4, 3},
//         {true,200, MIN_Y+SCANLINES_PER_ROW*1, 1,0,2,{128,129},0,32, 5, 4},
//         {true,250, MIN_Y+SCANLINES_PER_ROW*1, 1,0,2,{128,129},1,32, 6, 5},
//         {true,300, MIN_Y+SCANLINES_PER_ROW*1, 1,0,2,{128,129},0,32, 7, 6}
//     },
//     #endif
//     #if (NUM_ROWS>2)
//     {
//         {true, 50, MIN_Y+SCANLINES_PER_ROW*2, 1,0,2,{128,129},1,32, 2, 2},
//         {true,100, MIN_Y+SCANLINES_PER_ROW*2, 1,0,2,{128,129},0,32, 3, 3},
//         {true,150, MIN_Y+SCANLINES_PER_ROW*2, 1,0,2,{128,129},1,32, 4, 4},
//         {true,200, MIN_Y+SCANLINES_PER_ROW*2, 1,0,2,{128,129},0,32, 5, 5},
//         {true,250, MIN_Y+SCANLINES_PER_ROW*2, 1,0,2,{128,129},1,32, 6, 6},
//         {true,300, MIN_Y+SCANLINES_PER_ROW*2, 1,0,2,{128,129},0,32, 7, 8}
//     },
//     #endif
//     #if (NUM_ROWS > 3)
//     {
//         {true, 40, MIN_Y+SCANLINES_PER_ROW*3, 1,0,2,{128,129},0,32, 2, 2},
//         {true, 80, MIN_Y+SCANLINES_PER_ROW*3, 1,0,2,{128,129},1,32, 3, 4},
//         {true,120, MIN_Y+SCANLINES_PER_ROW*3, 1,0,2,{128,129},0,32, 4, 6},
//         {true,160, MIN_Y+SCANLINES_PER_ROW*3, 1,0,2,{128,129},1,32, 5, 8},
//         {true,200, MIN_Y+SCANLINES_PER_ROW*3, 1,0,2,{128,129},0,32, 6,10},
//         {true,240, MIN_Y+SCANLINES_PER_ROW*3, 1,0,2,{128,129},1,32, 7,12}
//     },
//     #endif
//     #if (NUM_ROWS > 4)
//     {
//         {true, 40, MIN_Y+SCANLINES_PER_ROW*4, 1,0,2,{128,129},0,32, 2, 7},
//         {true, 80, MIN_Y+SCANLINES_PER_ROW*4, 1,0,2,{128,129},0,32, 3, 6},
//         {true,120, MIN_Y+SCANLINES_PER_ROW*4, 1,0,2,{128,129},0,32, 4, 5},
//         {true,160, MIN_Y+SCANLINES_PER_ROW*4, 1,0,2,{128,129},0,32, 5, 4},
//         {true,200, MIN_Y+SCANLINES_PER_ROW*4, 1,0,2,{128,129},0,32, 6, 3},
//         {true,240, MIN_Y+SCANLINES_PER_ROW*4, 1,0,2,{128,129},0,32, 7, 2}
//     },
//     #endif
//     #if (NUM_ROWS > 5)
//     {
//         {true, 50, MIN_Y+SCANLINES_PER_ROW*5, 1,0,2,{128,129},0,32, 2, 1},
//         {true,100, MIN_Y+SCANLINES_PER_ROW*5, 1,0,2,{128,129},0,32, 3, 3},
//         {true,150, MIN_Y+SCANLINES_PER_ROW*5, 1,0,2,{128,129},0,32, 4, 5},
//         {true,200, MIN_Y+SCANLINES_PER_ROW*5, 1,0,2,{128,129},0,32, 5, 7},
//         {true,250, MIN_Y+SCANLINES_PER_ROW*5, 1,0,2,{128,129},0,32, 6, 9},
//         {true,300, MIN_Y+SCANLINES_PER_ROW*5, 1,0,2,{128,129},0,32, 7, 11}
//     }
//     #endif
// };

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

void flip_image(byte r, byte c);
void print_invaders();
__forceinline void move_invader(byte r, byte c);

void raster_irq_handler();

void set_next_irq(int rasterline);

void draw_sprite_row(int row, bool change_color_by_row, bool move_x_by_row, bool change_image_by_row);

void init_invaders();

#pragma compile("invaders.c")
#endif
