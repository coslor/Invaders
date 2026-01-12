#pragma optimize(maxinline)

#define VSPRITES_MAX 16
#include "invaders.h"
//
// Invaders...raping!
//

#define Screen ((char *)0x400)
#define Color ((char *)0xd800)

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

int prev_raster=0;

__export int lines_used = -1;
__export int total_invs;
__export int invs_size = sizeof(Invader);

int main() {

    // for (int i=2;i<8;i++) {
    //     //spr_image(i,128); 
    //     Screen[0x3f8 + i] = 128;   //TODO fix raw constant          
    // }

    iocharmap(IOCHM_PETSCII_2);

    // memset((char *)0x2000,0xff,128);

    // Change colors
	vic.color_border = VCOL_DARK_GREY;
	vic.color_back = VCOL_BLACK;

   	memset(Screen, 32, 1000);


    // is this really necessary? //
    // Disable interrupts while setting up
	__asm { sei };

    //Kill **all** other interrupts?
    __asm {
        lda #$7f
        sta $dc0d		 //turn off all types of cia irq/nmi.
        sta $dd0d
        lda $dc0d
        lda $dd0d
        lda #$ff
        sta $D019
        lda #$00
        sta $D01a
        sta $dc0e
        sta $dc0f
        sta $dd0e
        sta $dd0f
        lda $d01e
        lda $d01f
    }

	// Kill CIA interrupts
	//cia_init();

    //mmap_set(MMAP_NO_ROM);
    
    // enable raster interrupt via direct path
	// rirq_init(1);

	// initialize sprite multiplexer
	//vspr_init(Screen);
    spr_init(Screen);


    //Instead of dealing with the CIA stuff here and in the irq routine, I should
    //  probably turn them off completely. What exactly does this code do?
    cia1.icr=0x7f;
    cia2.icr=0x7f;
    // byte b=cia1.sdr;
    // b=cia2.sdr;




    //IRQ_VECTOR = (void *)0xfa31;
    IRQ_VECTOR=raster_irq_handler;

    set_next_irq(inv_start_line[0]);

    __asm { cli }


    while(1) {

        int a=sizeof(Invader);
        vic_waitBottom();
        //vic_waitLine(255);
        byte flip_lines = vic.raster;

        //REFACTOR: get rid of row,col stuff to eliminate multiplication used
        //          to get each inv address.
        Invader *inv=&invaders[0][0];
        total_invs = TOTAL_INVS_SIZE;
        Invader *end_inv = inv + TOTAL_INVS_SIZE;   //11b0
        //for (byte r=0;r<NUM_ROWS;r++) {
        //    for (byte c=0;c<INVADERS_PER_ROW;c++) {
        do {
                flip_image(inv++);
        } while (inv < end_inv);
        //}
        byte flip_lines_used=vic.raster - flip_lines;

        __asm{
            nop
        }
    }  
    printf("wtf??\n");
    return 0;
};

void draw_sprite_row(int row) {

    //assert(row < NUM_ROWS);

/*  REMOVE COLOR CODE FOR NOW
    //set colors before anything else
    // #pragma unroll(page)
    for (int c=0; c<INVADERS_PER_ROW; c++) {
    //#assign c1 0
    //#repeat
    //    assert(c1 < INVADERS_PER_ROW);
        Invader *inv=&invaders[row][c];
        vic.spr_color[inv->sprite_num]=inv->color;
        //Invader *inv_##c1 = &invaders[row][c1];
        // vic.spr_color[inv_##c1->sprite_num]=inv_##c1->color;
    //#assign c1 c1+1    
    //#until c1 == INVADERS_PER_ROW
    }

    lines_used=vic.raster - prev_raster;
*/

    // #pragma unroll(page)
    // for (int c=0; c<INVADERS_PER_ROW; c++) {
        Invader *inv;

    #assign c 0
    #repeat
        //pass "-NDEBUG" to compiler to remove assertion code
        //  ...or not. Maybe -DNDEBUG?
        //assert(c < INVADERS_PER_ROW);

        Invader *inv2##c=&invaders[row][c];
        if (inv2##c->alive > 0) {
            //vic_sprxy(inv->sprite_num,inv->x, inv->y);
            spr_move(inv2##c->sprite_num,inv2##c->x, inv2##c->y);
            spr_color(inv2##c->sprite_num,inv2##c->color);
            spr_image(inv2##c->sprite_num,inv2##c->image_handles[inv2##c->image_num]);
            // vic.spr_pos[inv->sprite_num].y = inv->y;
            // vic.spr_pos[inv->sprite_num].x = inv->x & 0xff;
            // if (inv->x & 0x100)
            //     vic.spr_msbx |= 1 << inv->sprite_num;
            // else
            //     vic.spr_msbx &= ~(1 << inv->sprite_num);

            //Screen[0x3f8 + inv2##c->sprite_num] = inv2##c->image_handles[inv2##c->image_num];   //TODO fix raw constant
            vic.spr_enable |= (1<<inv2##c->sprite_num);

        }
        else {
            vic.spr_enable &= (0xff - (1<<inv2##c->sprite_num));        //turn bit off
        }
    #assign c c+1

    #until c==INVADERS_PER_ROW
    #undef c
    // }   //for c
    __asm {
        nop
    }

}


/** PROBLEM: flip_image() is just too damn slow. Like by an order of magnitude.
*       As in it takes like a full frame just to flip the images (and no, not counting the waitBottom).
*       Maybe instead of keeping track of frames for each Invader, we do it once & check each 
*       Invader to see if it's ready to flip? Or something? 'Cause this ain't gonna work.
*       Maybe...we don't really need to redraw every sprite every frame? That might mean that 
*       movement gets choppier (like move 4px every 4th frame or whatever), but then that's hypothetical right now anyway.
*       Hell, maybe we only work on 1 Invader per frame...
*/

//__forceinline 
void flip_image(Invader *inv2) {
    //__assume(inv2->frame_num<256);
    //__assume(inv2->max_frames<256);
    //__assume(inv2->image_num<256);
    //__assume(inv2->num_images<256);

    if ((++inv2->frame_num) >= inv2->max_frames) {
        inv2->image_num++;
        inv2->image_num=(inv2->image_num & 1);
        //inv2->image_num = (inv2->image_num + 1) & 1;//num_images-1
        inv2->frame_num = 0;
    //     if (++(inv2->image_num) >= inv2->num_images) {
    //         inv2->image_num=0;
    //     }
    //     inv2->frame_num=0;
    // }
    // // else {
    // //     (inv->frame_num)++;
    }
    // //inv2->frame_num ++;
    // //vic.color_back=VCOL_BLACK;
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


void raster_irq_handler() {

    //if (vic.intr_ctrl > 127) {          //This is a raster interrupt ONLY if bit 7 of intr_ctrl/$d019 is set

        prev_raster = vic.raster;

        vic.intr_ctrl = 0xff;           //ACK irq

        draw_sprite_row(current_row_num);


        vic.intr_enable = 1;                                                                //$d01a

        current_row_num++;
        if (current_row_num >= NUM_ROWS) {
            current_row_num = 0;
        }
        // IRQ_VECTOR = raster_irq_handler;
        set_next_irq(inv_start_line[(current_row_num)]);

        lines_used=vic.raster - prev_raster;

        //vic.color_back=0;

    //}

    __asm{ 
        // lsr $d019   //vic.intr_ctrl -- ACK interrupt
        jmp $ea31   //(old_irq) - 
                    // call $ea31 for original, but scans keyboard twice, not necessary
                    //      $ea81 skips keyboard scan, better
                    //      $febc skips kernal stuff altogether
                    //      $ea7e ACKs & clears any NMIs & exits
    }

}

void set_next_irq(byte rasterline) {
    //from https://codebase64.com/doku.php?id=base:introduction_to_raster_irqs

    __asm {
        sei
    }

    cia1.sdr=0x7f;
    cia2.sdr=0x7f;
    byte b=cia1.sdr;
    b=cia2.sdr;

      //IRQ_VECTOR=irq_handler;                     //shouldn't have to set this every time
    //IRQ_VECTOR = (void *)0xfa31;
//     __asm{
//         lda #<irq_handler
//         sta $314
//         lda #>irq_handler
//         sta $315
// }


    vic.intr_enable = 1;
	vic.ctrl1 &= 0x7f;                           //MSb of raster line#   //TODO fix this to use MSb
	vic.raster = rasterline;                    //rest of raster line#  //TODO fix for MSB


    __asm{
        cli
    }
}