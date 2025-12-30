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

typedef struct  {
    byte alive=0;

    signed int x=0;
    byte y=0;

    signed int speed_x=0; //pixels per frame
    signed int speed_y=0;

    byte num_images=0;
    byte image_handles[MAX_IMAGE_HANDLES];
    byte image_num=0;

    unsigned char sprite_num=0;

    float fps=1.0;
    unsigned short int color=7;

    ////////////auto-initialized from here down////////////////
    long frame_num=0;

    signed int old_x=0;
    signed int old_y=0;

} Invader;

//const int NUM_INVADERS=9;
const int NUM_ROWS=6;
const int INVADERS_PER_ROW=6;

void flip_images(Invader* inv);
void print_invaders();
void move_invader(Invader* inv);
