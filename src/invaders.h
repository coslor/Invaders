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
    byte alive=0;
    int x=0;
    byte y=0;

    char speed_x=0; //pixels per frame
    char speed_y=0;

    byte num_images;
    byte image_handles[MAX_IMAGE_HANDLES];
    byte image_num=0;

    unsigned char sprite_num;

    float fps=1.0;

    long frame_num=0;
} Invader;

void flip_images(Invader* inv);
void print_invaders();
void move_invader(Invader* inv);
