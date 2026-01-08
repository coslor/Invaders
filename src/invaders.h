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

const int MAX_IMAGE_HANDLES=8;

const int MIN_Y=50;

typedef struct  {
    byte        alive=0;
    signed int  x=0;
    byte        y=0;
    signed int  speed_x=0,speed_y=0; //pixels per frame

    byte        num_images=0;
    byte        image_handles[MAX_IMAGE_HANDLES];
    byte        image_num=0;

    byte        sprite_num=0;

    byte        max_frames=1;

    unsigned short int color=1;
    ////////////auto-initialized from here down////////////////
    long        frame_num=0;

    signed int  old_x=0;
    signed int  old_y=0;

} Invader;

const int NUM_ROWS=2;
const int INVADERS_PER_ROW=6;
const int SCANLINES_PER_ROW=60;

int current_row_num=0;

#define  IRQ_VECTOR *(void **)0x0314



Invader invaders[NUM_ROWS][INVADERS_PER_ROW] = {
    {
//alive,x,y,speed_x,speed_y,num_images,image_handles,image_num (to start),sprite_num,max_frames,color
        {1, 50, MIN_Y+SCANLINES_PER_ROW*0, 1,0,2,{128,129},0, 2,16, 2},
        {1,100, MIN_Y+SCANLINES_PER_ROW*0, 1,0,2,{128,129},1, 3,16, 3},
        {0,150, MIN_Y+SCANLINES_PER_ROW*0, 1,0,2,{128,129},0, 4,16, 4},
        {1,200, MIN_Y+SCANLINES_PER_ROW*0, 1,0,2,{128,129},1, 5,16, 5},
        {1,250, MIN_Y+SCANLINES_PER_ROW*0, 1,0,2,{128,129},0, 6,16, 6},
        {1,300, MIN_Y+SCANLINES_PER_ROW*0, 1,0,2,{128,129},1, 7,16, 7},
    },
    {
        {1, 50, MIN_Y+SCANLINES_PER_ROW*1, 1,0,2,{128,129},0, 2,16, 2},
        {1,100, MIN_Y+SCANLINES_PER_ROW*1, 1,0,2,{128,129},1, 3,16, 3},
        {1,150, MIN_Y+SCANLINES_PER_ROW*1, 1,0,2,{128,129},0, 4,16, 4},
        {1,200, MIN_Y+SCANLINES_PER_ROW*1, 1,0,2,{128,129},1, 5,16, 5},
        {0,250, MIN_Y+SCANLINES_PER_ROW*1, 1,0,2,{128,129},0, 6,16, 6},
       // {1,300, MIN_Y+SCANLINES_PER_ROW*1, 1,0,2,{128,129},1, 7,16, 7}
    },
    // {
    //     {1, 50, MIN_Y+SCANLINES_PER_ROW*3, 1,0,2,{128,129},0, 2,16, 1},
    //     {1,100, MIN_Y+SCANLINES_PER_ROW*3, 1,0,2,{128,129},1, 3,16, 3},
    //     {1,150, MIN_Y+SCANLINES_PER_ROW*3, 1,0,2,{128,129},0, 4,16, 4},
    //     {1,200, MIN_Y+SCANLINES_PER_ROW*3, 1,0,2,{128,129},1, 5,16, 5},
    //     {1,250, MIN_Y+SCANLINES_PER_ROW*3, 1,0,2,{128,129},0, 6,16, 6},
    //     {1,300, MIN_Y+SCANLINES_PER_ROW*3, 1,0,2,{128,129},1, 7,16, 7}
    // },
    // {
    //     {1, 50, MIN_Y+SCANLINES_PER_ROW*4, 1,0,2,{128,129},0, 2,16, 1},
    //     {1,100, MIN_Y+SCANLINES_PER_ROW*4, 1,0,2,{128,129},1, 3,16, 3},
    //     {1,150, MIN_Y+SCANLINES_PER_ROW*4, 1,0,2,{128,129},0, 4,16, 4},
    //     {1,200, MIN_Y+SCANLINES_PER_ROW*4, 1,0,2,{128,129},1, 5,16, 5},
    //     {1,250, MIN_Y+SCANLINES_PER_ROW*4, 1,0,2,{128,129},0, 6,16, 6},
    //     {1,300, MIN_Y+SCANLINES_PER_ROW*4, 1,0,2,{128,129},1, 7,16, 7}
    // },
    // {
    //     {1, 50, MIN_Y+SCANLINES_PER_ROW*5, 1,0,2,{128,129},0, 2,16, 1},
    //     {1,100, MIN_Y+SCANLINES_PER_ROW*5, 1,0,2,{128,129},1, 3,16, 3},
    //     {1,150, MIN_Y+SCANLINES_PER_ROW*5, 1,0,2,{128,129},0, 4,16, 4},
    //     {1,200, MIN_Y+SCANLINES_PER_ROW*5, 1,0,2,{128,129},1, 5,16, 5},
    //     {1,250, MIN_Y+SCANLINES_PER_ROW*5, 1,0,2,{128,129},0, 6,16, 6},
    //     {1,300, MIN_Y+SCANLINES_PER_ROW*5, 1,0,2,{128,129},1, 7,16, 7}
    // },
    // {
    //     {1, 50,MIN_Y0, 1,0,2,{128,129},0, 2,16, 1},
    //     {1,100,MIN_Y0, 1,0,2,{128,129},1, 3,16, 3},
    //     {1,150,MIN_Y0, 1,0,2,{128,129},0, 4,16, 4},
    //     {1,200,MIN_Y0, 1,0,2,{128,129},1, 5,16, 5},
    //     {1,250,MIN_Y0, 1,0,2,{128,129},0, 6,16, 6},
    //     {1,300,MIN_Y0, 1,0,2,{128,129},1, 7,16, 7}
    // }
};

int inv_start_line[NUM_ROWS] = {
    0+5,//MIN_Y-SCANLINES_PER_ROW*0+5,
    SCANLINES_PER_ROW+5, //0+SCANLINES_PER_ROW*1+5,
    //  MIN_Y+SCANLINES_PER_ROW*2,
    //  MIN_Y+SCANLINES_PER_ROW*3,
    //  MIN_Y+SCANLINES_PER_ROW*4,
//     MIN_Y+SCANLINES_PER_ROW*5,
};

void flip_image(Invader* inv);
void print_invaders();
void move_invader(Invader* inv);

void raster_irq_handler();

void set_next_irq(byte, void (*)());

void draw_sprite_row(int);
