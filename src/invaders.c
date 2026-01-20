//A test comment #6

#pragma optimize(maxinline)

//#define VSPRITES_MAX 16
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
#pragma region( spriteset, 0x2000, 0x2800, , , {spriteset} )

// everything beyond will be code, data, bss and heap to the end
#pragma region( main, 0x2800, 0xa000, , , {code, data, bss, heap, stack} )


// spriteset at fixed location

#pragma data(spriteset)

////
//  NOTE: anything like this, where its data that needs to be there, but the 
//      var itself isn't referenced anywhere, needs to be called out 
//      with __export or #pragma reference(name), or it will be optimized away!
////
__export static const char spriteset[] =  {
//    #embed spd_sprites "invaders-both.spd"
    #embed spd_sprites "invaders-2600.spd"

};


#pragma data(data)

int prev_raster=0;

__export int lines_used = -1;
__export int total_invs;

bool first_time=true;

int main() {

    init_invaders();

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

    init_sprites();

    vic.spr_multi=0b11111100;
    //vic.spr_expand_x=0b11111100;
    //vic.spr_expand_y=0b11111100;

    //Instead of dealing with the CIA stuff here and in the irq routine, I should
    //  probably turn them off completely. What exactly does this code do?
    cia1.icr=0x7f;
    cia2.icr=0x7f;
    // byte b=cia1.sdr;
    // b=cia2.sdr;



    for (int c=0;c<INVADERS_PER_ROW;c++) {
        spr_move(inv_sprite_num[c], inv_x[c], inv_y[c]);
        spr_color(inv_sprite_num[c], inv_color[c]);

    } 

    //IRQ_VECTOR = (void *)0xfa31;
    IRQ_VECTOR=raster_irq_handler;

    set_next_irq(inv_start_line[0]);

    __asm { cli }


    int rn = 0;

    while(1) {


        //int a=sizeof(Invader);
        //vic_waitBottom();
        vic_waitLine(255);
        byte flip_lines = vic.raster;

        //REFACTOR: get rid of row,col stuff to eliminate multiplication used
        //          to get each inv address.
        //Invader *inv=&invaders[0][0];
        //total_invs = TOTAL_INVS_SIZE;
        //Invader *end_inv = inv + TOTAL_INVS_SIZE;   //11b0

        int main_offset=0;
        //#pragma unroll(full)
        for (byte mr=0;mr<NUM_ROWS;mr++) {
        //#pragma unroll(full)
            for (byte mc=0;mc<INVADERS_PER_ROW;mc++) {
                __asm {
                    sei
                }
                flip_image(main_offset++);
                __asm {
                    cli
                }
                
            }
        }
        byte flip_lines_used=vic.raster - flip_lines;

        __asm{
            nop
        }
    }  
    printf("wtf??\n");
    return 0;
};

void draw_sprite_row(int offset, bool change_color_by_row, bool move_x_by_row, bool change_image_by_row) {

    //assert(row < NUM_ROWS);

    //#pragma unroll(full)


    for (int c=0; c<INVADERS_PER_ROW; c++) {
        //Invader *inv=&invaders[row][c];

        //pass "-NDEBUG" to compiler to remove assertion code
        //  ...or not. Maybe -DNDEBUG?
        assert(c < INVADERS_PER_ROW);

        //byte c_offset=offset+c;

        if (first_time || move_x_by_row || change_color_by_row || change_image_by_row) {
            byte spr_num=inv_sprite_num[offset];

            if (inv_alive[offset]) {

                //We always have to move Y, otherwise no rows
                vic.spr_pos[spr_num].y = inv_y[offset];

                //But maybe Invaders don't need unique X positions between rows?
                if (move_x_by_row || first_time) {
                    //spr_move(inv_sprite_num[offset], inv_x[offset], inv_y[offset]);
                    vic.spr_pos[spr_num].x = inv_x[offset] & 0xff;
                    if (inv_x[offset] & 0x100)
                        vic.spr_msbx |= 1 << spr_num;
                    else
                        vic.spr_msbx &= ~(1 << spr_num);
                }

                if (change_color_by_row || first_time) {
                    spr_color(spr_num, inv_color[offset]);
                }
                if (change_image_by_row || first_time) {
                    spr_image(spr_num,inv_image_handles[offset][inv_image_num[offset]]);
                }

                spr_show(spr_num,true);

            }
            else {
                spr_show(spr_num,false);
            }

        } else {
            if (inv_alive[offset]) {
                byte sprite_num=inv_sprite_num[offset];
                vic.spr_pos[sprite_num].y = inv_y[offset];
                spr_show(sprite_num,true);
                //spr_image(inv_sprite_num[offset],inv_image_handles[offset][inv_image_num[offset]]);
                Screen[0x3f8 + sprite_num] = inv_image_handles[offset][inv_image_num[offset]];   //TODO fix raw constant
            }
        }

        offset++;
    }   //for c
   
    first_time=false;

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
*
        BASICALLY SOLVED: We just run flip_image() in the main thread.
*/

//__forceinline 
void flip_image(int fi_offset) {
    //__assume(inv2->frame_num<256);
    //__assume(inv2->max_frames<256);
    //__assume(inv2->image_num<256);
    //__assume(inv2->num_images<256);

    byte image_num=inv_image_num[fi_offset];
    if ((++(inv_frame_num[fi_offset])) >= inv_max_frames[fi_offset]) {
        inv_image_num[fi_offset]=((inv_image_num[fi_offset]+1) % inv_num_images[fi_offset]);
        inv_frame_num[fi_offset]=0;
        //inv2->image_num = (inv2->image_num + 1) & 1;//num_images-1
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

 __forceinline void move_invader(int offset) {
    //Invader* inv=&invaders[inv_num];
    inv_x[offset] += inv_speed_x[offset];
    inv_y += inv_speed_y[offset];

    if (inv_x[offset] <20) {
        inv_speed_x[offset] = abs(inv_speed_x[offset]);
    }
    else {
        if (inv_x[offset] >= 320){
            inv_speed_x[offset] = -abs(inv_speed_x[offset]);
        }
    }

    // spr_move(inv->sprite_num,inv->x,inv->y);
    // vspr_move(inv->sprite_num,inv->x,inv->y);
}


void raster_irq_handler() {

    int min_y=MIN_Y;

    //if (vic.intr_ctrl > 127) {          //This is a raster interrupt ONLY if bit 7 of intr_ctrl/$d019 is set

        prev_raster = vic.raster;

        vic.intr_ctrl = 0xff;           //ACK irq


        //todo take this calc out
        int offset = (current_row_num)*INVADERS_PER_ROW;

        draw_sprite_row(offset, CHANGE_COLOR_BY_ROW, MOVE_X_BY_ROW, CHANGE_IMAGE_BY_ROW);

        if (++current_row_num >= NUM_ROWS) {
            current_row_num = 0;
        }

        //vic.intr_enable = 1;                                                                //$d01a

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

void set_next_irq(int rasterline) {
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


    if (rasterline < 256) {
	    vic.ctrl1 &= 0b01111111; //0x7f;                           //MSb of raster line#
    }
    else {
        vic.ctrl1 |= 0b10000000;
    }

	vic.raster = (byte)rasterline&0b11111111;                    //rest of raster line# 
    vic.intr_enable = 1;


    __asm{
        cli
    }
}

void init_invaders() {
    int offset=0;
    for (int r=0;r<NUM_ROWS; r++) {
            row_y[offset]               = MIN_Y + SCANLINES_PER_ROW * r;
            row_num_images[offset]      =2;
            row_image_handles[offset][0] = 140+(r*2);
            row_image_handles[offset][1] = 141+(r*2);
            row_image_num[offset]       = 0;
            row_max_frames[offset]      = 32;
            row_frame_num[offset]       =0;


        for (int c=0;c<INVADERS_PER_ROW; c++) {
            inv_alive[offset]           = true;
            inv_x[offset]               = c*35+24 + 50;
            //inv_y[offset]             = MIN_Y + SCANLINES_PER_ROW * r;
            inv_speed_x[offset]         =1;
            inv_speed_y[offset]         =0;
            // inv_num_images[offset]    =2;
            // inv_image_handles[offset][0] = 140+(r*2);
            // inv_image_handles[offset][1] = 141+(r*2);
            // inv_image_num[offset]     = 0;
            // inv_max_frames[offset]    = 32;
            inv_sprite_num[offset]      = 2 + c;
            inv_color[offset]           = c + (r & 3)+1;
            //inv_frame_num[offset]     =0;
            inv_old_x[offset]           =0;
            inv_old_y[offset]           =0;
            offset++;
        }
    }
}

void init_sprites() {
    for (int c=0;c<6;c++) {
        spr_move(c+2, c*35+24 + 50,0);          //just ignore the Y coord for now
        vic.spr_color[c+2]              =c+1;

    }
}