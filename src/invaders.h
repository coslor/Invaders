#include "c64/types.h"
#include <conio.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>


#include <c64/joystick.h>
#include <c64/vic.h>
#include <c64/sprites.h>
#include <c64/memmap.h>
#include <c64/rasterirq.h>

//#include "invaders.h"

const int MAX_IMAGE_HANDLES=8;

typedef struct  {
    unsigned char alive;
    int x=0;
    unsigned char y=0;

    char speed_x=0; //pixels per frame
    char speed_y=0;

    unsigned char num_images;
    unsigned char image_handles[MAX_IMAGE_HANDLES];

    unsigned char sprite_num;

    float fps;
} Invader;

void flip_images(byte, byte**, byte, float);
void print_invaders();
void move_invader(byte inv_num);
