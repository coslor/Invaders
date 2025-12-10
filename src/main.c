#include "invaders.h"

//
// Invaders...raping!
//


//const char* hello_text = "HELLO, WORLD!";
//char* screen = (char*) (0x4 0e);
//unsigned char* color = (unsigned char*) (0xd80e);

byte * const Screen = (byte *)0x400; //(byte *)0xc800;
//byte * const Font = (byte *)0xd000;
byte * const Color = (byte *)0xd800;
byte * const Sprites = (byte *)0x340; //0xd800;

float f = (float)1.1;

//byte invader1_0[];
unsigned char invaders_181[128];

const float fps = 1.0;

const int NUM_INVADERS=1;
    //{.alive=true,.x=25,.y=50,.speed_x=5,.speed_y=0,.num_images=2,.image_handles={13,14},.fps=1.0},


Invader invaders[NUM_INVADERS] = {
        
        
        // {
        //     (byte)true,
        //     (int)25,
        //     (char)50,
        //     (char)5,
        //     (char)0,
        //     (unsigned char)2,
        //     {
        //         (unsigned char)13,
        //         (unsigned char)14
        //     },
        //     (char)0,
        //     (float)1.0
        // }
    {
        true,
        75,
        50,
        5,
        0,
        2,{13,14},
        0,
        2.0
    },
    // {true,150,50,5,0,2,{13,14},1.0},
    // {true,225,50,5,0,2,{13,14},1.0},
    // {true,300,50,5,0,2,{13,14},1.0},
    // {true,375,50,5,0,2,{13,14},1.0}
};


byte image_num=13;

int main() {
    iocharmap(IOCHM_PETSCII_2);


    byte frames=0;
 	// Switch screen
	//vic_setmode(VICM_TEXT, Screen, Font);

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

    // spr_move(0,100,200);
    // //spr_image(0,Sprites/16);
    // //*((char *)(2040)) = 13;
    // spr_image(0,14);
    // spr_show(0,true);
    // // unsigned char i = 0;
    // // char c;

    // *((unsigned char*) 0xd018) = 21;

    // for (;;) {
    //     c = (char) (hello_text[i]&0b00111111);
    //     if (c == '\0') break;
    //     screen[i] = c;
    //     color[i] = (unsigned char) 1;
    //     i++;
    // }

    for (int i=0;i<NUM_INVADERS;i++){ 
        spr_image(invaders[i].sprite_num,invaders[i].image_handles[0]);
        spr_show(invaders[i].sprite_num, true);
    }

    //byte invader_images[2]={13,14};

    // printf("FPS=%f\n",fps);
    // printf("fps=%f2.2\n",fps);
    

    // print_invaders();

    while(true) {

        vic_waitBottom();

        for (int i=0;i<NUM_INVADERS; i++){
            if (invaders[i].alive) {
                int frames_to_switch=(int)(60.0 / invaders[i].fps);
                move_invader(i);
                flip_images(invaders[i].sprite_num, &invaders[i].image_handles,
                    invaders[i].num_images, invaders[i].fps );
            }
        }
        // for (int x=24;x<320;x++) {	
        //     //vic.color_border = VCOL_RED;

        //     vic_waitBottom();

        //     //vic.color_border = VCOL_WHITE;
        //     vic_waitBottom();

        //     spr_move(0,x,100);
        //     flip_images(0,invader_images, 2,2.0);
        //     //vic.color_border = VCOL_BLACK;
        //     //vic_waitFrame();
        // }
        // for (int x=320;x>24;x--) {	
        //     vic.color_border = VCOL_BLUE;

        //     vic_waitBottom();

        //     vic.color_border ++;//= VCOL_WHITE;

        //     spr_move(0,x,100);
        //     flip_images(0,invader_images, 2,2.0);
        //     //vic_waitFrame();
        //}
    }    
    return 0;
}

void flip_images(Invader inv) {
    //byte spr_num, byte **image_handles, byte num_images, float fps) {
    assert(fps>0);


    int max_frames=(int)(60.0/inv.fps);
    static int frames=0;
    static int image_num=0;

    frames++;
    if ((frames % max_frames)==0) {
        image_num=(image_num+1) % num_images;
        frames=0;
    }

}

void move_invader(byte inv_num) {
    Invader* inv=&invaders[inv_num];
    inv->x += inv->speed_x;
    inv->y += inv->speed_y;
    spr_move(inv->sprite_num,inv->x,inv->y);
}

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

//    {.x=25,.y=50,.speed_x=5,.speed_y=0,.num_images=2,.image_handles={13,14},.fps=1.0},
