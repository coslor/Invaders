#define VSPRITES_MAX 16
#include "invaders.h"
//
// Invaders...raping!
//

__export int nums[4]={1};

//byte * const Screen = (byte *)0x400; //(byte *)0xc800;
//byte * const Color = (byte *)0xd800;
//byte * Sprites = (byte *)0x340; //0xd800;

#define Screen ((char *)0x400)

// make space until 0x2000 for code and data

#pragma region( lower, 0xa00, 0x2000, , , {code, data} )

// then space for our sprite data

#pragma section( spriteset, 0)

#pragma region( spriteset, 0x2000, 0x2080, , , {spriteset} )

// everything beyond will be code, data, bss and heap to the end

#pragma region( main, 0x2080, 0xa000, , , {code, data, bss, heap, stack} )


// spriteset at fixed location

#pragma data(spriteset)

////
//  NOTE: anything like this, where its data that needs to be there, but the 
//      var itself isn't referenced anywhere, needs to be called out 
//      with __export or #pragma reference(name), or it will be optimized away!
////
__export static const char spriteset[128] =  {
    #embed spd_sprites "invaders-both.spd"
};

#pragma data(data)

__export int lines_used = -1;

int main() {

    iocharmap(IOCHM_PETSCII_2);

        // memset((char *)0x2000,0xff,128);


    // Change colors
	vic.color_border = VCOL_DARK_GREY;
	vic.color_back = VCOL_BLACK;
	//vic.color_back1 = VCOL_WHITE;
	//vic.color_back2 = VCOL_DARK_GREY;

	//vic.spr_mcolor0 = VCOL_DARK_GREY;
	//vic.spr_mcolor1 = VCOL_WHITE;

   	memset(Screen, 32, 1000);

    // is this really necessary? //
    // Disable interrupts while setting up
	//__asm { sei };

	// Kill CIA interrupts
	cia_init();

    //mmap_set(MMAP_NO_ROM);
    
    // enable raster interrupt via direct path
	// rirq_init(1);

	// initialize sprite multiplexer
	//vspr_init(Screen);
    spr_init(Screen);


    
    // for (int r=0;r<NUM_ROWS;r++) {
    //     for (int c=0;c<INVADERS_PER_ROW;c++) {

    //         Invader *inv1=&invaders[r][c];
    //         if (inv1->alive) {
    //             vspr_set(inv1->sprite_num,
    //                 inv1->x, inv1->y,
    //                 inv1->image_handles[inv1->image_num],
    //                 inv1->color
    //             );
    //         }
    //     }
    // }
 
    // // initial sort and update
	// vspr_sort();
	// vspr_update();
	// rirq_sort();

    // // start raster IRQ processing
	// rirq_start();

    vic_waitFrame();
    vic_waitTop();

    IRQ_VECTOR = raster_irq_handler;
    set_next_irq(inv_start_line[0],raster_irq_handler);

    while(1) {

       // vic.color_back=VCOL_LT_GREY;

        //vic_waitTop();
        //vic.color_back=VCOL_BLACK;

        /** NOTHING TO DO IN THE MAIN THREAD (yet)

        
        #pragma unroll(full)
        for (int r=0;r<NUM_ROWS; r++){
            //vic.color_back=10;
            //vic.color_back=VCOL_BROWN;
            //vic_waitBelow(inv_start_line[r]);
            //vic.color_back=VCOL_RED;

            // this should be in the irq handler
            // draw_sprite_row(current_row_num++);

            // if (current_row_num >= NUM_ROWS) {
            //     current_row_num = 0;
            // }

            //vic.color_back=0;
            vic.color_back=VCOL_BLACK;

        }   //for r
        //vic.color_back=VCOL_BLUE;
        //vic_waitBottom();
        //vic.color_back=VCOL_BLACK;
        **/

        /*
		// wait for raster IRQ to reach and of frame
		rirq_wait();

    	// sort virtual sprites by y position
		vspr_sort();

		// update sprites back to normal and set up raster IRQ for sprites 8 to 31
		vspr_update();

		// sort raster IRQs
		rirq_sort();

        */
        
        //vic.color_border++; 
        //vic.color_border=9;
        __asm{
            nop
        }
    }  
    printf("wtf??\n");
    return 0;
};

void draw_sprite_row(int row) {
    int c;
    //#pragma unroll(full)
    for (c=0;c<INVADERS_PER_ROW; c++) {
        Invader *inv=&invaders[row][c];
        if (inv->alive) {
            //vic_sprxy(inv->sprite_num,inv->x, inv->y);
            vic.spr_pos[inv->sprite_num].y = inv->y;
            vic.spr_pos[inv->sprite_num].x = inv->x & 0xff;
            if (inv->x & 0x100)
                vic.spr_msbx |= 1 << inv->sprite_num;
            else
                vic.spr_msbx &= ~(1 << inv->sprite_num);


            //vic.spr_color[inv->sprite_num]= inv->color;
            Screen[0x3f8 + inv->sprite_num] = inv->image_handles[inv->image_num];   //TODO fix raw constant
            vic.spr_enable |= (1<<inv->sprite_num);

            //I'm pretty sure this belongs in the main thread
            //flip_image(inv);
        }
        else {
            vic.spr_enable &= (0xff - (1<<inv->sprite_num));        //turn bit off
        }

    }   //for c
    __asm {
        nop
    }

}


//__forceinline 
void flip_image(Invader *inv2) {
    __assume(inv2->frame_num<256);
    __assume(inv2->max_frames<256);
    __assume(inv2->image_num<256);
    __assume(inv2->num_images<256);

    //byte spr_num, byte **image_handles, byte num_images, float fps) {

    //assert(inv->fps>0);


    //int max_frames=(int)(60.0/inv->fps);
    //static int frames=0;
    //static int image_num=0;

    //vic.color_back=VCOL_PURPLE;
    //int frame_num = inv->frame_num;
    //int image_num=inv->image_num;

    //inv->frames++;
    if ((++inv2->frame_num) >= inv2->max_frames) {
        if (++(inv2->image_num) >= inv2->num_images) {
            inv2->image_num=0;
        }
        inv2->frame_num=0;
    }
    // else {
    //     (inv->frame_num)++;
    // }
    //inv2->frame_num ++;
    //vic.color_back=VCOL_BLACK;
}

 __forceinline void move_invader(Invader* inv) {
    //Invader* inv=&invaders[inv_num];
    inv->x += inv->speed_x;
    inv->y += inv->speed_y;

    if (inv->x <20) {
        inv->speed_x = abs(inv->speed_x);
    }
    else {
        if (inv->x >= 320){
            inv->speed_x = -abs(inv->speed_x);
        }
    }

    // spr_move(inv->sprite_num,inv->x,inv->y);
    // vspr_move(inv->sprite_num,inv->x,inv->y);
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

void raster_irq_handler() {
    int prev_raster=0;

    if (vic.intr_ctrl > 127) {          //This is a raster interrupt ONLY if bit 7 of intr_ctrl/$d019 is set

        prev_raster = vic.raster;
        current_row_num = 0;
        for (int i=0;i<NUM_ROWS;i++) {
            if (prev_raster >= inv_start_line[i]) {
                current_row_num = i;
            }
        }
        //vic.spr_enable = 0x00;

        //vic.color_back=VCOL_GREEN;

        draw_sprite_row(current_row_num);
        lines_used=vic.raster - prev_raster;

        //vic.color_back = VCOL_BLACK;

        current_row_num++;
        if (current_row_num >= NUM_ROWS) {
            current_row_num = 0;
        }
        // IRQ_VECTOR = raster_irq_handler;
        set_next_irq(inv_start_line[(current_row_num)], raster_irq_handler);

    }
    __asm{ 
        asl $d019   //vic.intr_ctrl -- ACK interrupt
        jmp $ea31   //(old_irq) 
    }

}

__export void set_next_irq(byte rasterline, void (*irq_handler)()) {
    //from https://codebase64.com/doku.php?id=base:introduction_to_raster_irqs

    __asm {
        sei
    }

    // cia1.sdr=0x7f;
    // cia2.sdr=0x7f;
    // byte b=cia1.sdr;
    // b=cia2.sdr;

      IRQ_VECTOR=irq_handler;                     //shouldn't have to set this every time
    //IRQ_VECTOR = (void *)0xfa31;
//     __asm{
//         lda #<irq_handler
//         sta $314
//         lda #>irq_handler
//         sta $315
// }


    vic.intr_enable = 1;
	vic.ctrl1 &= 0x7f;                           //MSb of raster line#   //TODO fix this to use MSb
	vic.raster = rasterline & 0b11111111;        //rest of raster line#


    __asm{
        cli
    }
}