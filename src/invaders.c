#define VSPRITES_MAX 12
#include "invaders.h"

//
// Invaders...raping!
//


//byte * const Screen = (byte *)0x400; //(byte *)0xc800;
//byte * const Color = (byte *)0xd800;
//byte * Sprites = (byte *)0x340; //0xd800;

#define Screen ((char *)0x400)

// make space until 0x2000 for code and data

#pragma region( lower, 0xa00, 0x2000, , , {code, data} )

// then space for our sprite data

#pragma section( spriteset, 0)

#pragma region( spriteset, 0x2000, 0x2040, , , {spriteset} )

// everything beyond will be code, data, bss and heap to the end

#pragma region( main, 0x2040, 0xa000, , , {code, data, bss, heap, stack} )


// spriteset at fixed location

#pragma data(spriteset)

/********************************
 * OK, so make really FUCKING sure that the size of the spriteset array
 *  is the actual length of the file.
 *
*********************************/
// extern volatile char spriteset[128] = {
// 	#embed "invaders-both.bin"
//     //#embed "invaders-181.bin"
//     // #embed "digitsprites.bin"
// };

__export static const char spriteset[64] =  {
    #embed spd_sprites "invaders.spd"

//     // #embed "invaders-181.bin"
//    #embed "digitsprites.bin"

//     //  sprite_image_0
//     0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xc3,0xff,0xff,0xff,
//     0xff,0xf3,0xcc,0x7e,0x33,0xcc,0x7e,0x3f,0xfc,0x7e,0x3f,0xff,0xff,0xff,0xff,0xff,
//     0xff,0x3f,0xff,0xfc,0x3f,0xff,0xfc,0x0f,0xff,0xf0,0x0f,0xff,0xf0,0x0c,0x00,0x30,
//     0x0c,0x00,0x30,0x30,0x00,0x0c,0x30,0x00,0x0c,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
//     //  sprite_image_1
//     0x0c,0x00,0x30,0x0c,0x00,0x30,0x03,0x00,0xc0,0x03,0xff,0xc0,0x03,0xff,0xc0,0x0f,
//     0xff,0xf0,0x0c,0x7e,0x30,0x0c,0x7e,0x3c,0x3c,0x7e,0x3c,0x3f,0xff,0xfc,0xff,0xff,
//     0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xcf,0xff,0xf3,0xcf,0xff,0xf3,0xcc,0x00,0x33,
//     0xcc,0x00,0x33,0xc3,0x00,0xc3,0xc3,0x00,0xc3,0x00,0x00,0x00,0x00,0x00,0x00,0x01
// };

// {
//     0x0c,0x00,0x30,0x0c,0x00,0x30,0xc3,0x00,0xc3,0xc3,0xff,0xc3,0xc3,0xff,0xc3,0xcf,
//     0xff,0xf3,0xcc,0x7e,0x33,0xcc,0x7e,0x3f,0xfc,0x7e,0x3f,0xff,0xff,0xff,0xff,0xff,
//     0xff,0x3f,0xff,0xfc,0x3f,0xff,0xfc,0x0f,0xff,0xf0,0x0f,0xff,0xf0,0x0c,0x00,0x30,
//     0x0c,0x00,0x30,0x30,0x00,0x0c,0x30,0x00,0x0c,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
//     0x0c,0x00,0x30,0x0c,0x00,0x30,0x03,0x00,0xc0,0x03,0xff,0xc0,0x03,0xff,0xc0,0x0f,
//     0xff,0xf0,0x0c,0x7e,0x30,0x0c,0x7e,0x3c,0x3c,0x7e,0x3c,0x3f,0xff,0xfc,0xff,0xff,
//     0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xcf,0xff,0xf3,0xcf,0xff,0xf3,0xcc,0x00,0x33,
//     0xcc,0x00,0x33,0xc3,0x00,0xc3,0xc3,0x00,0xc3,0x00,0x00,0x00,0x00,0x00,0x00,0x01
// };

};

//extern volatile byte value=0;

#pragma data(data)




const int NUM_INVADERS=12;

//Well, as long as we don't try to use a sprite beyond #12 in the spr_
//  functions, we can get away with just about as many sprites as we want.
//  This is time-based multiplexing, and the flickering sprites
//  look like crap.

//So, let's try out the multiplexing library.

Invader invaders[NUM_INVADERS] = {
//active,x,y,speed_x,speed_y,num_images,image_handles,image_num (to start),sprite_num,fps
    {true, 50,50, 1,0,2,{128,129},0, 2,4.0},
    {true,100,50, 1,0,2,{128,129},1, 3,4.0},
    {true,150,50, 1,0,2,{128,129},0, 4,4.0},
    {true,200,50, 1,0,2,{128,129},1, 5,4.0},
    {true,250,50, 1,0,2,{128,129},0, 6,4.0},
    {true,300,50, 1,0,2,{128,129},1, 7,4.0},

    {true, 50,100, 1,0,2,{128,129},0, 8,4.0},
    {true,100,100, 1,0,2,{128,129},1, 9,4.0},
    {true,150,100, 1,0,2,{128,129},0,10,4.0},
    {true,200,100, 1,0,2,{128,129},1,11,4.0},
    {true,250,100, 1,0,2,{128,129},0,12,4.0},
    {true,300,100, 1,0,2,{128,129},1,13,4.0},

    // {true, 50,100, 1,0,2,{13,14},0,14,4.0},
    // {true,100,100, 1,0,2,{13,14},1,15,4.0},
    // {true,150,100, 1,0,2,{13,14},0,16,4.0},
    // {true,200,100, 1,0,2,{13,14},1,17,4.0},
    // {true,250,100, 1,0,2,{13,14},0,18,4.0},
    // {true,300,100, 1,0,2,{13,14},1,19,4.0},
};



int main() {

        iocharmap(IOCHM_PETSCII_2);

        // memset((char *)0x2000,0xff,128);


    // Change colors
	vic.color_border = VCOL_GREEN;
	vic.color_back = VCOL_BLACK;
	vic.color_back1 = VCOL_WHITE;
	vic.color_back2 = VCOL_DARK_GREY;

	vic.spr_mcolor0 = VCOL_DARK_GREY;
	vic.spr_mcolor1 = VCOL_WHITE;

   	memset(Screen, 32, 1000);
	//memset(Color, 1, 1000);

	//memcpy(Font, charset, 2048);
	//memcpy(Sprites, invaders_181, 128);
    // memset((char *)0x2000,0xff,128);

    //spr_init(Screen);

    // is this really necessary? //
    // Disable interrupts while setting up
	__asm { sei };

	// Kill CIA interrupts
	cia_init();

    mmap_set(MMAP_NO_ROM);
    
    // enable raster interrupt via direct path
	rirq_init(false);

	// initialize sprite multiplexer
	vspr_init(Screen);


    for (int i=0;i<NUM_INVADERS;i++) {
        Invader *inv=&invaders[i];
        if (inv->alive) {
            byte color = (i & 15);
            if (color == 0) {
                color = 1;
            }
            // value =     
            //         //(((unsigned)&spriteset[0]) / 64);
            //         (byte)(inv->image_handles[inv->image_num]);
            //         //(byte)0x80;

            vspr_set(inv->sprite_num,
                inv->x, inv->y,
                //inv->image_handles[inv->image_num],
                //(unsigned)(0xc000 / 64),
                //value,
                 128,

                //////////////////////////
                //      IMPORTANT       //
                //////////////////////////
                //  You **MUST** reference spriteset in the code somewhere, or 
                //      oscar will optimize it away. There's probably a better
                //      way to do this with #pragma or something.
                //
                // ((unsigned)&spriteset[0]) / 64,

                color
            );
        }
    }
    // for (int i=0;i<8; i++) {
    //     vic.spr_color[i]=VCOL_WHITE;
    // }

    // initial sort and update
	vspr_sort();
	vspr_update();
	rirq_sort();

    // start raster IRQ processing
	rirq_start();

    while(true) {

        //vic_waitBottom();

        for (int i=0;i<NUM_INVADERS; i++){
            //vic.color_border=i;
            Invader *inv=&invaders[i];

            if (inv->alive) {
                //int frames_to_switch=(int)(60.0 / inv->fps);

                move_invader(inv);
                if (inv->x <20) {
                    inv->speed_x = abs(inv->speed_x);
                }
                else {
                    if (inv->x >= 320){
                        inv->speed_x = -abs(inv->speed_x);
                    }
                }
                
                // flip_images(inv);

                // // spr_image(inv->sprite_num, 
                // //     inv->image_handles[inv->image_num]);
                // vspr_image(inv->sprite_num, 
                //     inv->image_handles[inv->image_num]);
            }
            else {
                vspr_hide(inv->sprite_num);
        // spr_show(inv->sprite_num, inv->alive);        
            }
        //vic.color_border++; 

        }   //for

    	// sort virtual sprites by y position
		vspr_sort();

		// wait for raster IRQ to reach and of frame
		rirq_wait();

		// update sprites back to normal and set up raster IRQ for sprites 8 to 31
		vspr_update();

		// sort raster IRQs
		rirq_sort();

        //vic.color_border++; 
        //vic.color_border=9;
    }  
    printf("wtf??\n");
    return 0;
};


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
    //spr_move(inv->sprite_num,inv->x,inv->y);
    vspr_move(inv->sprite_num,inv->x,inv->y);
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