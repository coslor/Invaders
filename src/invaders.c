#include "invaders.h"

//
// Invaders...raping!
//


byte * const Screen = (byte *)0x400; //(byte *)0xc800;
byte * const Color = (byte *)0xd800;
byte * Sprites = (byte *)0x340; //0xd800;


unsigned char invaders_181[128];

//OK, now what happens when we use more sprites than we have?
const int NUM_INVADERS=18;

//Well, as long as we don't try to use a sprite beyond #7 in the spr_
//  functions, we can get away with just about as many sprites as we want.
//  This is time-based multiplexing, and the flickering sprites
//  look like crap.


Invader invaders[NUM_INVADERS] = {
//active,x,y,speed_x,speed_y,num_images,image_handles,image_num (to start),sprite_num,fps
    {true, 50,50, 1,0,2,{13,14},0,2,4.0},
    {true,100,50, 1,0,2,{13,14},1,3,4.0},
    {true,150,50, 1,0,2,{13,14},0,4,4.0},
    {true,200,50, 1,0,2,{13,14},1,5,4.0},
    {true,250,50, 1,0,2,{13,14},0,6,4.0},
    {true,300,50, 1,0,2,{13,14},1,7,4.0},

    {true, 50,75, 1,0,2,{13,14},0,2,4.0},
    {true,100,75, 1,0,2,{13,14},1,3,4.0},
    {true,150,75, 1,0,2,{13,14},0,4,4.0},
    {true,200,75, 1,0,2,{13,14},1,5,4.0},
    {true,250,75, 1,0,2,{13,14},0,6,4.0},
    {true,300,75, 1,0,2,{13,14},1,7,4.0},

    {true, 50,100, 1,0,2,{13,14},0,2,4.0},
    {true,100,100, 1,0,2,{13,14},1,3,4.0},
    {true,150,100, 1,0,2,{13,14},0,4,4.0},
    {true,200,100, 1,0,2,{13,14},1,5,4.0},
    {true,250,100, 1,0,2,{13,14},0,6,4.0},
    {true,300,100, 1,0,2,{13,14},1,7,4.0},
};



int main() {
    iocharmap(IOCHM_PETSCII_2);

    // Change colors
	vic.color_border = VCOL_GREEN;
	vic.color_back = VCOL_BLACK;
	vic.color_back1 = VCOL_WHITE;
	vic.color_back2 = VCOL_DARK_GREY;

	vic.spr_mcolor0 = VCOL_DARK_GREY;
	vic.spr_mcolor1 = VCOL_WHITE;

   	memset(Screen, 32, 1000);
	memset(Color, 1, 1000);

	//memcpy(Font, charset, 2048);
	memcpy(Sprites, invaders_181, 128);

    spr_init(Screen);

    for (int i=0;i<8; i++) {
        vic.spr_color[i]=VCOL_WHITE;
    }

    while(true) {

        vic_waitBottom();

        for (int i=0;i<NUM_INVADERS; i++){
            //vic.color_border=i;
            Invader *inv=&invaders[i];

            if (inv->alive) {
                int frames_to_switch=(int)(60.0 / inv->fps);

                move_invader(inv);
                if (inv->x <20) {
                    inv->speed_x = abs(inv->speed_x);
                }
                else if (inv->x >= 320){
                    inv->speed_x = -abs(inv->speed_x);

                }
                
                flip_images(
                    inv);
                spr_image(inv->sprite_num, 
                    inv->image_handles[inv->image_num]);
            }
        spr_show(inv->sprite_num, inv->alive);

        }
        vic.color_border=9;
    }  
    printf("wtf??\n");
    return 0;
}

void flip_images(Invader *inv) {
    //byte spr_num, byte **image_handles, byte num_images, float fps) {

    assert(inv->fps>0);


    int max_frames=(int)(60.0/inv->fps);
    //static int frames=0;
    //static int image_num=0;

    int frame_num = inv->frame_num;
    int image_num=inv->image_num;

    //inv->frames++;
    if (((inv->frame_num+1) % max_frames)==0) {
        inv->image_num=(inv->image_num+1) % inv->num_images;
        inv->frame_num=0;
        inv=inv;
        int a=5;
        inv=(Invader*)a;
    }
    else {
        (inv->frame_num)++;
    }

}

void move_invader(Invader* inv) {
    //Invader* inv=&invaders[inv_num];
    inv->x += inv->speed_x;
    inv->y += inv->speed_y;
    spr_move(inv->sprite_num,inv->x,inv->y);
}

/**
void print_invaders() {
    printf("# invaders=%d\n",NUM_INVADERS);

    for (byte i=0;i<NUM_INVADERS;i++) {
        printf("Invader %d:\n");
        printf("\tactive=%d x=%d y=%d speed-x=%d speed-y=%d\n", 
            invaders[i].alive,
            invaders[i].x,          invaders[i].y,
            invaders[i].speed_x,    invaders[i].speed_y);

        printf("\tnum-images=%d image-handles=[");
        for (byte j=0;j<invaders[i].num_images;j++) {
            printf("%d,", invaders[i].image_handles[j]);
        }
        printf("]\n");

        printf("\tfps=%f\n", invaders[i].fps);
    }
}

**/