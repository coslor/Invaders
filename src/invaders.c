

//#pragma optimize(speed)

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
    #embed spd_sprites "invaders-2600.spd"

};


#pragma data(data)


__export int prev_raster=0;

__export int lines_used = -1;
__export int total_invs;

__export int flip_lines_used = -1;

__export signed int useless =-1;
bool first_time=true;

byte target_row = 0;

//The PETSCII values go {F1,F3,F5,f7,f2,f4,f6}. This array maps them so we can use simple code
//  to decide which fn key goes with what row.  
const byte fn_key_row[7] = {
    0,2,4,0,1,3,5
};
const byte asdf_row[6] = "asdfgh";

__export byte coll_spr_num=0xff;
__export byte coll_spr_y = 0xff;

int main() {
    //Bad things happen if these two get out of sync
    my_assert(SCANLINES_PER_ROW > SCANLINES_TO_DRAW_SPRITE, "scanlines constants are out of sync");

    vic.spr_sprcol = 0xff;

    iocharmap(IOCHM_PETSCII_2);

	vic.color_border = VCOL_WHITE;
	vic.color_back = VCOL_BLACK;

   	memset(Screen, 32, 1000);

    init_invaders();
    init_sprites();


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

    //We really don't have any need to map ROM out at this point,
    //  and not doing so has certain advantages (like being able
    //  to use getchs() ).
    //mmap_trampoline();
    //mmap_set(MMAP_NO_ROM);
    
    spr_init(Screen);

    init_sprites();

    //All sprites are multicolor
    vic.spr_multi=0b11111111;
    vic.spr_mcolor0 = VCOL_LT_GREEN;
    vic.spr_mcolor1 = VCOL_RED;

    //Instead of dealing with the CIA stuff here and in the irq routine, I should
    //  probably turn them off completely. What exactly does this code do?
    cia1.icr=0x7f;
    cia2.icr=0x7f;
    // byte b=cia1.sdr;
    // b=cia2.sdr;



    #pragma unroll(full)
    for (int c=0;c<INVADERS_PER_ROW;c++) {
        spr_move(inv_sprite_num[c], inv_x[c], inv_y[c]);
        spr_color(inv_sprite_num[c], inv_color[c]);

    }

    ship.alive = true;
    ship.x = 160;
    ship.y = 230;
    ship.sprite_num = 0;
    ship.sprite_color = 1;
    ship.image_handle = 152;
    ship.type = TYPE_SHIP;

    bullet.alive = false;
    bullet.image_handle = 153;
    bullet.sprite_num = 1; 
    bullet.sprite_color = 2;
    bullet.type = TYPE_BULLET;

    IRQ_VECTOR=raster_irq_handler;

    set_next_irq(inv_start_line[0], true);

    __asm { cli }

    int row_num = 0;

    collided_inv_index = -1;

    while(playing) {

        // char key = getchx();
        // if (key>='1' && key <= '6') {
        //     byte col=(key-'1');
        //     shoot_invader(target_row, col);
        // }
        // else if (key>=0x85 && key <= 0x8b) {
        //     target_row = fn_key_row[key - 0x85];

        // }


#ifdef USE_BORDER
        vic.color_border = VCOL_BLUE;
#endif
        //vic_waitBottom();
        wait_line_and_watch_for_collisions(255);
#ifdef USE_BORDER
        vic.color_border = VCOL_WHITE;
#endif

        byte flip_lines = vic.raster;

        if ((++(rows_frame_num)) >= rows_max_frames) {
            rows_x_shift += rows_x_frame_speed;
            rows_frame_num = 0;
            bounce_rows();
        }

        #pragma unroll(full)
        for (byte mr=0;mr<NUM_ROWS;mr++) {

#ifdef SYNC_MAIN_THREAD
            __asm {
                sei
            }
#endif

            flip_row(mr);

            
            byte raster=vic.raster;
            __asm {
                nop;
            }
            
#ifdef SYNC_MAIN_THREAD
            __asm {
                cli
            }
#endif
        }
        flip_lines_used=vic.raster - flip_lines;

        read_joy();
        move_object(&ship);
        move_object(&bullet);

        draw_object(&ship);
        draw_object(&bullet);

        //int sprcol;

        ////
        //  Collision code, work in progress
        ////
        //TODO un-comment
        // if (coll_spr_num != 0xff) {
        //     // //vic.spr_sprcol = 0b11111111;    //Is this necessary?
        //     int row_num=0xff;
        //     for (int r=0;r<NUM_ROWS;r++) {
        //         if (coll_spr_y == row_y[r]) {
        //             row_num = r; 
        //         }
        //     }
        //     if (row_num == 0xff) {
        //         vic.color_back = VCOL_ORANGE;
        //         break;
        //     }
        //     if (coll_spr_num & pow2[coll_spr_num]) {
        //         int y=0xff;
        //         for (int c=0;c<NUM_ROWS;c++) {
        //             if ()
        //         }
        //         vic.color_back = VCOL_BLUE;
        //         collided_inv_index = inv_index;
        //         inv_color[collided_inv_index] = VCOL_RED;
        //     }
        // }
        // coll_spr_num == 0xff;
        // collided_inv_index = 0xff;

       // bounce_rows();

        //for debugging
        __asm{
            nop
        }
    }
    vic.color_back=VCOL_RED;
    printf("wtf??\n");
    return 0;
};

/*
*   Returns the index of the Invader associated with the sprite
*   most containing the screen coordinate x,y
*/
//TODO use this?
// int find_inv(int screen_x, byte screen_y) {
//     //TODO Optimize this by pre-creating a reverse list
//     int r = -1;
//     #pragma unroll(full)
//     for (r=0;r<NUM_ROWS;r++) {
//         if ((screen_y >=row_y[r]) 
//             && (screen_y <= (r < NUM_ROWS-1 ? 0xff : row_y[r+1])) ) {
//             break;
//         }
//     }
//     my_assert(r != -1, "r not found in find_inv()");

//     int c = -1;

//     #pragma unroll(full)
//     for (c=0;c < INVADERS_PER_ROW; c++) {
//         byte inv_index = r * INVADERS_PER_ROW + c;
//         byte next_inv_index = inv_index + 1;
//         if (screen_x >= inv_spr_pos_x[inv_index] 
//             && screen_x < (c < INVADERS_PER_ROW - 1 ? inv_spr_pos_x[c+1] : 320)) {
//                 break;
//         }
//     }
//     my_assert(c != -1, "c not found in find_inv");

//     return r*INVADERS_PER_ROW + c;
// }

void shoot_invader(byte si_row, byte si_col) {

#ifdef USE_BORDER
    vic.color_border++;
#endif
    byte row_index = row_inv_index[si_row];
    byte inv_index = row_index + si_col;

    //my_assert(inv_alive[index],"zombie Invaders");
    if (! inv_alive[inv_index] ){
        //We've already killed this invader, so ignore it
        return;
    }

    inv_alive[inv_index]=false;
    
    row_dirty[si_row] = true;

    byte spr_mask=0;

    #pragma unroll(full)
    for (int c=0;c<INVADERS_PER_ROW;c++) {
        int off=row_inv_index[si_row]+c;
        if (inv_alive[off]) {
            spr_mask |= 1<<inv_sprite_num[off];
        }
    }
    row_sprite_enable_mask[si_row] = spr_mask;

}


void draw_sprite_row(byte spr_row) {

    // if (spr_row == target_row) {
    //     vic.color_back=VCOL_BLACK;
    // }

    //my_assert(spr_row < NUM_ROWS,"spr_row too high");

    byte raster_dsr = vic.raster;

    //Instead of calling spr_show() 6 times, we pre-calc the spr_enable mask for the whole row
    //          in shoot_invader()
    vic.spr_enable = row_sprite_enable_mask[spr_row];

    //We have to at least disable display of the dead row before exiting
    if (!row_alive[spr_row]) {
        return;
    }

    byte new_handle = row_image_handles[spr_row][row_image_num[spr_row]];

    byte row_index = row_inv_index[spr_row];

    #pragma unroll(full)
    for (byte c=0;c<INVADERS_PER_ROW; c++) {
        byte inv_index = row_index + c; //row * INVADERS_PER_ROW + c;
        byte spr_num=c+2;



        ////
        // PROBLEM: even though we're trying to update the image for the next sprite row down,
        //          we're still writing to the image while sprites are being drawn!
        //          I think we're going to need to wait until we're past a row before we can 
        //          update the image of any sprite. Set 2 IRQ's / row? Just waitUntil(right_after_this_row)?
        //
        //          ...or have enough space between SCANLINES_TO_DRAW_SPRITE and SCANLINES_PER_ROW
        //          so that the image update happens in the space in between. Not sure how reliable 
        //          this 2nd method will be though.
        ////
        Screen[0x3f8 + spr_num] = new_handle;

        int spr_pos_x = col_x_index[c] + rows_x_shift; //row_x_index[spr_row];
        inv_spr_pos_x[inv_index] = spr_pos_x;
        vic.spr_pos[spr_num].x = spr_pos_x & 0xff;
        if (spr_pos_x & 0x100)
            vic.spr_msbx |= 1 << spr_num;
        else
            vic.spr_msbx &= ~(1 << spr_num);
        
 //       if (vic.spr_sprcol) {
 //           useless = vic.spr_sprcol;
 //       }

        vic.spr_color[spr_num] = inv_color[inv_index];

        inv_spr_pos_y[inv_index] = row_y[spr_row];
        vic.spr_pos[spr_num].y=row_y[spr_row];  //;do this last?

    }


    //for debugging
    __asm {
        nop
    }
    //vic.color_back = VCOL_BLACK;
}


 __forceinline void move_invader(byte index) {
    //Invader* inv=&invaders[inv_num];
    inv_x[index] += inv_speed_x[index];
    inv_y += inv_speed_y[index];

    if (inv_x[index] <20) {
        inv_speed_x[index] = abs(inv_speed_x[index]);
    }
    else {
        if (inv_x[index] >= 320){
            inv_speed_x[index] = -abs(inv_speed_x[index]);
        }
    }

    // spr_move(inv->sprite_num,inv->x,inv->y);
    // vspr_move(inv->sprite_num,inv->x,inv->y);
}


void raster_irq_handler() {

    int min_y=MIN_Y;

    //if (vic.intr_ctrl > 127) {          //This is a raster interrupt ONLY if bit 7 of intr_ctrl/$d019 is set

        prev_raster = vic.raster;

        vic.intr_ctrl |= 0b10000000; //0xff;           //ACK irq

        
#ifdef USE_BORDER
        vic.color_border = VCOL_GREEN;
#endif
        draw_sprite_row(current_row_num);

#ifdef USE_BORDER
        vic.color_border = VCOL_WHITE;
#endif
        

        if (++current_row_num >= NUM_ROWS) {
            current_row_num = 0;
        }

        set_next_irq(inv_start_line[current_row_num], true);
        //TODO try to fix this
        // if (! set_next_irq(inv_start_line[current_row_num])) {
        //     byte next_line = current_row_num < NUM_ROWS-1 ? (current_row_num + 1) : 0;
        //     set_next_irq(inv_start_line[next_line]);
        // }

        lines_used=vic.raster - prev_raster;

    __asm{ 
        // lsr $d019   //vic.intr_ctrl -- ACK interrupt
        jmp $ea31   //(old_irq) - 
                    // call $ea31 for original, but scans keyboard twice, not necessary
                    //      $ea81 skips keyboard scan, better
                    //      $febc skips kernal stuff altogether
                    //      $ea7e ACKs & clears any NMIs & exits
    }

}

/*
 *  Returns true if the raster hasn't already passed the requested line (plus a buffer),
 *      false otherwise.
 */
bool set_next_irq(int rasterline, bool calling_from_irq) {
    //from https://codebase64.com/doku.php?id=base:introduction_to_raster_irqs

    bool ok=false;

    //NOTE: this check has GOT to be more expensive than just calling the redundant sei/cli
    if (! calling_from_irq) {
        __asm {
            sei
        }
    }

    //TODO: what do these lines of code actually do?
    cia1.sdr=0x7f;
    cia2.sdr=0x7f;
    byte b=cia1.sdr;
    b=cia2.sdr;

    //A simplified version of some code to prevent flickering, from here:
    //  https://cadaver.github.io/rants/sprite.html
    //  ...and it doesn't work, unless I give it a lot of buffer. Maybe take it out?
    //int vic_raster = vic.raster + ((vic.ctrl1 & 0b10000000) * 256);
    //byte new_ras = rasterline - vic_raster;
    
    //if ((new_ras) >= 4) {

        if (rasterline < 256) {
            vic.ctrl1 &= 0b01111111; //0x7f;                           //MSb of raster line#
        }
        else {
            vic.ctrl1 |= 0b10000000;
        }


        vic.raster = (byte)rasterline&0b11111111;                    //rest of raster line# 
        vic.intr_enable = 1;
        ok=true;
    //}
    //else {
    //    ok=false;
    //}

    //NOTE:see note above
    if (! calling_from_irq) {
        __asm{
            cli
        }
    }

    return true;
}

void flip_row(byte row) {
    if (!row_alive[row]) return;

    if ((++(row_frame_num[row])) >= row_max_frames[row]) {
        row_dirty[row]=true;

        row_image_num[row]=((row_image_num[row]+1) % row_num_images[row]);
        row_frame_num[row]=0;

        
    }

}

void init_invaders() {
    //int index=0;

    #pragma unroll(full)
    for (int r=0;r<NUM_ROWS; r++) {
        row_y[r]                = MIN_Y + SCANLINES_PER_ROW * r;
        row_num_images[r]       = 2;
        row_image_handles[r][0] = 140+(r*2);
        row_image_handles[r][1] = 141+(r*2);
        row_image_num[r]        = 0;
        row_max_frames[r]       = ROW_MAX_FRAMES;
        row_frame_num[r]        = 0;
        row_alive[r]            = true;
        row_inv_index[r]        = r * INVADERS_PER_ROW;

        row_dirty[r] = true;
        row_sprite_enable_mask[r] = 255;

        
        #pragma unroll(full)
        for (int c=0;c<INVADERS_PER_ROW; c++) {
            byte index=r*INVADERS_PER_ROW+c;
            inv_alive[index]            = true;
            inv_speed_x[index]          = 1;
            inv_speed_y[index]          = 0;
            inv_sprite_num[index]       = 2 + c;
            inv_color[index]            = r + 3; //c + (r & 3)+3;
            inv_old_x[index]            = 0;
            inv_old_y[index]            = 0;
            col_x_index[c]              = 0 + c*35;
            inv_spr_pos_x[index]        = 0;
            inv_row[index]              = r;
            inv_col[index]              = c;
        }
    }
}

void init_sprites() {
    
    #pragma unroll(full)
    for (int ic=0;ic<6;ic++) {
        byte spr_num=ic+2;

        Screen[0x3f8 + spr_num] = row_image_handles[0][row_image_num[0]];

        spr_move(spr_num, ic*35+24 + 50,0);          //just ignore the Y coord for now
        spr_color(spr_num,ic+1);
        spr_show(spr_num,true);

    }
}

//TODO:Fix this - it's way too slow!
void find_min_max_spr_x() {
    rows_min_spr_x = MAX_SPR_X;
    rows_max_spr_x = MIN_SPR_X;

    
    #pragma unroll(full)
    for (int row=0;row<NUM_ROWS;row++) {
        if (! row_alive[row]) continue;

        byte row_index = row_inv_index[row];

        
        #pragma unroll(full)
        for (int col=0;col<INVADERS_PER_ROW;col++) {
            byte inv_index = row_index + col;

            if (! inv_alive[inv_index]) continue;

            if (inv_spr_pos_x[inv_index] < rows_min_spr_x) {
                rows_min_spr_x = inv_spr_pos_x[inv_index];
            } else {
                if (inv_spr_pos_x[inv_index] > rows_max_spr_x) {
                    rows_max_spr_x = inv_spr_pos_x[inv_index];
                } //if inv_spr_pos_x >
            }//else inv_spr_pos_x <
        }//int col
    }//int row
}//find_min_max_spr_x

/*
 * Check to see if the max/min X position has been crossed. If so,
 *  move all of the rows down and reverse their direction.
*/
void bounce_rows() {
    find_min_max_spr_x();
    if (rows_max_spr_x > MAX_SPR_X) {
        move_rows_down(Y_INC);
        rows_x_frame_speed *= -1;

        rows_x_shift -= X_INC;
    }
     if (rows_min_spr_x < MIN_SPR_X) {
        move_rows_down(Y_INC);
        rows_x_frame_speed *= -1;

        rows_x_shift += X_INC;
    }
}

void move_rows_down(byte px_down) {
    
    #pragma unroll(full)
    for (int r8=0;r8<NUM_ROWS;r8++) {       //"r8" for debugging
        row_y[r8] += px_down;

        //TODO make a proper GAME OVER
        if (row_y[r8] > MAX_Y_ROW) {
            vic.color_back = VCOL_RED;
            playing = false;
        }
        inv_start_line[r8] += px_down;

    }
}

void read_joy() {
    joy_poll(0);
    ship.speed_x = joyx[0];
    if (ship.speed_x < 0) {
        // useless=ship.speed_x;   //for debugging
    }
    if (joyb[0]) {
        fire_bullet(&bullet);
    }
}

void fire_bullet(PlayerObject* b) {
    if (b->type == TYPE_BULLET) {
        b->x = ship.x;
        b->y = ship.y-3;
        b->speed_x = 0;
        b->speed_y = -1;
        b->alive = true;
        
        vic.color_back = 15;
    }
    else {
        printf("Wrong PlayerObject type:%d\n", b->type);
    }
}

void move_object(PlayerObject *obj) {
    // if (obj == &bullet && obj->alive) {
    //     useless = (int)obj;          //debugging
    // }   
    if (obj->speed_x > 0) {
        if (obj->x < 320) {
            obj->x += obj->speed_x;
        }
    }
    else if (obj->speed_x < 0) {
        if (obj->x > 25) {
            obj->x += obj->speed_x;
        }
    }

    if (obj->speed_y != 0) {
        if (obj->y < 255) {
            obj->y += obj->speed_y;

            if (obj->y > 255) {
                kill_bullet(&bullet);
            }
        }
        else if (obj->speed_y < 50) {
            if (obj->y > 50) {
                obj->y += obj->speed_y;
            }
            if (obj == &bullet) {
                kill_bullet(&bullet);
                obj->alive = false;
            }
        }
    }
}

void draw_object(PlayerObject *obj2) {
    if (obj2->alive) {
        spr_move(obj2->sprite_num, obj2->x, obj2->y);
        spr_image(obj2->sprite_num, obj2->image_handle);
    }
    spr_show(obj2->sprite_num, obj2->alive);
}

void kill_bullet(PlayerObject *b) {
    b->alive = false;
    vic.color_back = VCOL_BLACK;
}

/**
*   coll_spr_y 
 */
byte wait_line_and_watch_for_collisions(int line)
{
	char	upper = (char)(line >> 1) & VIC_CTRL1_RST8;
	char	lower = (char)line;

	do
	{
        int raster;
		while (((raster=vic.raster) != lower) ) {
            //TODO get collision  handling working
            // int coll = vic.spr_sprcol;
            // if (coll != 0) {
            //     coll_spr_num=0xff;
            //     if (coll & 0b00000010 == 0) {
            //         continue;
            //     }
            //     //Find out which byte is set
            //     for (byte b=2;b<8;b++) {
            //         if ((coll & pow2[b]) != 0) {
            //         coll_spr_num = coll + 2;

            //         }
            //     }
            //     coll_spr_y = raster;
            // }	
        }
	} while ((vic.ctrl1 & VIC_CTRL1_RST8) != upper);
    return vic.raster;
}