#pragma optimize(speed)

//#define VSPRITES_MAX 16

#include "invaders.h"
//
// Invaders...raping!
//

char* text_screen = ((char *)0x400);
char* text_color = ((char *)0x1000);

#define LOGO_FILE "resources/space_invaders_logo.kla"

#pragma data(spriteset_sec)

////
//  NOTE: anything like this, where its data that needs to be there, but the 
//      var itself isn't referenced anywhere, needs to be called out 
//      with __export or #pragma reference(name), or it will be optimized away!
////
static const char spriteset[] =  {
    #embed spd_sprites "resources/invaders-2600.spd"

};
#pragma reference(spriteset)

#pragma data(logo_bmp_sec)
__export static const char logo_bmp[] = {
    #embed 8000 2 LOGO_FILE  
};

#pragma data(logo_screen_sec)
__export static char logo_screen[1000] = {
    #embed 1000 9002 LOGO_FILE
};

#pragma data(logo_color_sec)
//load the text & color screens into
__export static char logo_color[1000] = {
    #embed 1000 8002 LOGO_FILE
};

#pragma data(data)

__export int prev_raster=0;

__export int lines_used = -1;
__export int total_invs;

__export int flip_lines_used = -1;


byte target_row = 0;

//The PETSCII values go {F1,F3,F5,f7,f2,f4,f6}. This array maps them so we can use simple code
//  to decide which fn key goes with what row.  
const byte fn_key_row[7] = {
    0,2,4,0,1,3,5
};

const byte asdf_row[6] = "asdfgh";

__export byte coll_spr_num=0xff;
__export byte coll_spr_y = 0xff;

const unsigned short invaders_2600_size;

const int JOY_NUM=0;

char key;


//MAIN THREAD
//#pragma optimize(0)
int main() {

    iocharmap(IOCHM_PETSCII_1);


    bool smooshed = false;

	// Disable CIA interrupts, we do not want interference
	// with our joystick interrupt
	cia_init();

    display_logo();

    sidfx_init();
	sid.fmodevol = 15;
    
    // // while (true) {
    // //     __asm {
    // //         nop
    // //     }
        
    // //     if (kr_is_key_pressed(KR_ROW_SPACE,KR_COL_SPACE)) {
    // //         printf("SPACE ");
    // //     }
    // //     if (kr_is_key_pressed(KR_ROW_A, KR_COL_A)) {
    // //         printf("A ");
    // //     }
    // //     if (kr_is_key_pressed(KR_ROW_D, KR_COL_D)) {
    // //         printf("D ");
    // //     }
    // //     //vic.color_back++;
    // //     vic_waitFrame();
    // // }

    // // // while (true) {
    // // //     key= kr_read_key();
    // // //     if (key != 0) {
    // // //         __asm {
    // // //             nop
    // // //         }
    // // //         break;
    // // //     }
    // // // };

    //this is just here to play with the SIDFx stuff
    while(true) {
        vic_waitFrame();
        sidfx_loop();

        if (kr_is_key_pressed(KR_ROW_1, KR_COL_1)) {
            sidfx_play(0, SIDFXFire, 1);
        }
        if (kr_is_key_pressed(KR_ROW_2, KR_COL_2)) {
        //if (key == '2') {
			sidfx_play(1, SIDFXExplosion, 1);
        }
        // if (key_pressed(KSCAN_3)) {
        //     sidfx_play(2, SIDFXBigExplosion, 3);

        // }
        //if (key_pressed(KSCAN_SPACE)) {
        //if (key == ' ') {
        if (kr_is_key_pressed(KR_ROW_SPACE, KR_COL_SPACE)) {
            //    getchx();
            break;
        }
    }

    //We're going to use the SID later for music & sfx, which will
    //  mess up any attempts to use it for PRNG so let's just
    //  use it once to seed srand(). Having waited for a keypress
    //  adds just the soupcon of true randomness we really need.

    init_sid_rand();
    srand(sid_rand());

    vic.color_back = VCOL_LT_GREY;
    vic.color_border = VCOL_DARK_GREY;

    clear_hires_screen();

    clear_text_screen();
    init_screen(50);

    //point the VIC to the right screen (to record sprite #s for example)
    //vic_setmode(VICM_HIRES_MC, logo_screen,logo_bmp); // $d018=$49 $d011=$3b $dd00=$c6
    vic_setmode(VICM_TEXT, (char *)0x0400, (char *)0x1000);
    spr_init((char *)0x400);

    init_invaders();
    init_sprites();

    // Disable interrupts while setting up
	 __asm { sei };


    //TODO BUG: if the lower-right-est Invader is killed, the Player ship is no longer displayed

    //BUG:pressed keys cause all kinds of screen flicker & distortion.
    //  Answer: bypass the kernal keyboard read code when JMPing at the end of the IRQ handler.
    //          However, when you do that, you can't use the kernal keyboard routines(duh!)

    //Kill **all** other interrupts?
    __asm {
        lda #$7f
        sta $dc0d		 //turn off all types of cia irq/nmi.
    //     sta $dd0d
    //     lda $dc0d
    //     lda $dd0d
    //     lda #$ff
    //     sta $D019
    //     lda #$00
    //     sta $D01a
    //     sta $dc0e
    //     sta $dc0f
    //     sta $dd0e
    //     sta $dd0f
    //     lda $d01e
    //     lda $d01f
    }

	// // Kill CIA interrupts
	// cia_init();


    //After loading & showing logo, so it's Ok to turn BASIC off
    // mmap_trampoline();
    // mmap_set(MMAP_NO_ROM);
    

    //init_sprites();

    //All sprites are multicolor
    vic.spr_multi   = 0b11111111;
    vic.spr_mcolor0 = VCOL_LT_GREEN;
    vic.spr_mcolor1 = VCOL_RED;

    // //Instead of dealing with the CIA stuff here and in the irq routine, I should
    // //  probably turn them off completely. What exactly does this code do?
    // cia1.icr        = 0x7f;
    // cia2.icr        = 0x7f;
    // // byte b=cia1.sdr;
    // // b=cia2.sdr;



        //mmap_trampoline();
	//mmap_set(MMAP_NO_BASIC);

    __asm { cli }

    int row_num = 0;

    collided_inv_index = 0xff;

    rows_x_shift = 50;
    rows_x_frame_speed = 4;

    rows_frame_num = 0;

    collided_inv_index=0xff;

    playing = true;

    draw_object((byte)0); //initialize ship image

    //__asm { cli }
    IRQ_VECTOR=raster_irq_handler;

    set_next_irq(inv_start_line[0], false);
    //__asm { sei }

    while(playing) {

        // //Cheat keys
        // // f1-f7    : choose the current row
        // // 1-6      : shoot the invader on the current row, in that column
        // char key = getchx();
        // if (key>='1' && key <= '6') {
        //     byte col=(key-'1');
        //     shoot_invader(target_row, col);
        //     //TODO FIXME BUG doing a printf() after changing back to text mode 
        //     // causes a rather catastrophic crash -- the emulator locks up!
        //     // // printf("pew pew\n");

        // }
        // else if (key>=0x85 && key <= 0x8b) {
        //     target_row = fn_key_row[key - 0x85];

        // }

#ifndef NO_PLAYER
        handle_inputs(JOY_NUM);
        //move_object(SHIP_OBJ_NUM);
        //move_object(BULLET_OBJ_NUM);

        draw_object(SHIP_OBJ_NUM);
        draw_object(BULLET_OBJ_NUM);
#endif

        //END_BORDER();

        //START_BORDER(VCOL_WHITE)

        //TODO it seems criminal to waste this time
        vic_waitBottom();

        //vic_waitLine(255);
        //TODO fix this & get collisions working
        //wait_line_and_watch_for_collisions(255);
        //END_BORDER();

        //play slice of special effects each frame
        sidfx_loop();

        //Actually show the sprites, and move them
        START_BORDER(VCOL_BLUE);
        
        smooshed = ! move_invaders();
        set_sprites_for_all();
        if (smooshed) {
            playing = false;
            break;
        }
        
        END_BORDER();

        //Flip the images
        //START_BORDER(VCOL_LT_GREY)
#ifdef DO_UNROLL
        #pragma unroll(full)
#endif
        for (byte row=0;row<NUM_ROWS;row++) {
            flip_row_image(row);
        }
//        flip_lines_used=vic.raster - flip_lines;

        //END_BORDER
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


        //for debugging
        __asm{
            nop
        }
    }
    if (smooshed) {
        spr_image(0, SPRITE_IMAGE_BASE + SMOOSHED_SHIP_IMAGE_NUM);

        //spr_expand(0,true,false);
    }
    //vic.color_back=VCOL_RED;

    printf("GAME OVER\n");

    while (kr_read_key() == 0) { vic_waitFrame(); };
    //return 0;
   
    inv_assert(false, "GAME OVER");
    // __asm {
    //     jmp $e37b //$fce2   //reset machine
    // }
}


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
//     inv_assert(r != -1, "r not found in find_inv()");

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
//     inv_assert(c != -1, "c not found in find_inv");

//     return r*INVADERS_PER_ROW + c;
// }

//MAIN THREAD
void shoot_invader(byte si_row, byte si_col) {

    byte row_index = row_inv_index[si_row];
    byte inv_index = row_index + si_col;

    //inv_assert(inv_alive[index],"zombie Invaders");
    if (! inv_alive[inv_index] ){
        //We've already killed this invader, so ignore it
        return;
    }

    inv_alive[inv_index]=false;
    // inv_assert(col_invs_left_alive[si_col] > 0, "mismatch in col-invs--left-alive");
    col_invs_left_alive[si_col]--;
    
    //row_dirty[si_row] = true;

    byte spr_mask=0;

#ifdef DO_UNROLL
    #pragma unroll(full)
#endif
    for (byte c=0;c<INVADERS_PER_ROW;c++) {
        byte off=row_inv_index[si_row]+c;
        if (inv_alive[off]) {
            spr_mask |= 1<<inv_sprite_num[off];
        }
    }
    row_sprite_enable_mask[si_row] = spr_mask;

}

//MAIN THREAD
void set_sprites_for_all() {
#ifdef DO_UNROLL    
    #pragma unroll(full)
#endif
    for (int c=0;c<INVADERS_PER_ROW;c++) {
        //TODO replace with inv_sprite_num?
        byte spr_num = c+2;

        if (! col_invs_left_alive[c]) { continue; }

        int spr_pos_x = col_x[c] + rows_x_shift; //row_x_index[spr_row];
        rows_inv_spr_pos_x[c]    = spr_pos_x;

        #ifdef MY_ASSERT
            inv_assert(spr_num<8,"spr-num=%d at set-sprites-for-all", spr_num);
        #endif

        //Using this instead of vic.sprxy() saves us a few cycles by not setting sprite.y
        vic.spr_pos[spr_num].x = spr_pos_x; //& 0xff
        if (spr_pos_x > 0xff)
            vic.spr_msbx |= 1 << spr_num;
        else
            vic.spr_msbx &= ~(1 << spr_num);
    }
    //take_vic_snapshot();

}

//IRQ THREAD
void draw_sprite_row(byte spr_row) {

    #ifdef MY_ASSERT                        //0123456789012345678901234567890123456789
        inv_assert(vic.spr_multi==0xff,     "multi=0 for row %d in draw-sprite-row", spr_row);
        inv_assert(vic.spr_expand_x == 0,   "xpandx=1 for row %d in draw-sprite-row", spr_row);
        inv_assert(spr_row < NUM_ROWS,      "spr-row=%d in draw-sprite-row", spr_row);
    #endif
    //Instead of calling spr_show() 6 times, we pre-calc the spr_enable mask for the whole row
    //          in shoot_invader()
    //TODO why doesn't this turn off the player & bullet?
    vic.spr_enable = row_sprite_enable_mask[spr_row];

    if (!row_alive[spr_row]) {
        // __asm {
        //     cli
        // }
        return;
    }

    int this_row_y = row_y[spr_row];

    #pragma unroll(full)
    for (byte c=0;c<INVADERS_PER_ROW; c++) {
        #ifdef MY_ASSERT
            inv_assert(c+2<8,"Bad c+2(%d) in draw-sprite-row", c+2);
        #endif

        vic.spr_pos[c+2].y= this_row_y;  //do this last? Nope.
    }

    vic.spr_mcolor0 = row_mcolor0[spr_row];
    vic.spr_mcolor1 = row_mcolor1[spr_row];

    byte new_handle = row_image_handles[spr_row][row_image_num[spr_row]];

    #ifdef MY_ASSERT
        inv_assert(new_handle > 0, "new-handle=0 in draw-sprite-row(%d)", spr_row);
    #endif

    #pragma unroll(full)
    for (byte c=0;c<INVADERS_PER_ROW; c++) {
        // byte inv_index = row_index + c; //row * INVADERS_PER_ROW + c;
        // if (! inv_alive[inv_index]) {
        //     continue;
        // }

        byte spr_num = c + 2;

        #ifdef MY_ASSERT
            inv_assert(spr_num<8, "spr-num=%d at draw-sprite-row", spr_num);
        #endif

        spr_image(spr_num, new_handle);
        // vic.spr_pos[spr_num].y= this_row_y;  //;do this last?
    }
}


//IRQ THREAD
void raster_irq_handler() {

    if (playing) {
        // //TODO needed? Useful?    
        // if (vic.intr_ctrl < 128) {          //This is a raster interrupt ONLY if bit 7 of intr_ctrl/$d019 is set
        //     vic.color_back=VCOL_YELLOW;
        //     __asm {
        //         rti;
        //     }
        // }

        prev_raster = vic.raster;


        if (prev_raster >= 230) {
            //START_BORDER(VCOL_YELLOW);

            // inv_assert(ship.sprite_num < 8, "bad ship sprite in rirq");
            //vic.spr_color[ship.sprite_num] = ship.sprite_color;

            //TODO always <0xff?
            
            if (obj_sprite_mcolor0[SHIP_OBJ_NUM] < 0xff) {
                vic.spr_mcolor0 = obj_sprite_mcolor0[SHIP_OBJ_NUM];
            }
            if (obj_sprite_mcolor1[SHIP_OBJ_NUM] < 0xff) {
                vic.spr_mcolor1 = obj_sprite_mcolor1[SHIP_OBJ_NUM];
            }
            //END_BORDER();
        }
        else {

            vic.color_back = VCOL_BLACK;

            START_BORDER(VCOL_GREEN);
            #ifdef MY_ASSERT
                inv_assert(current_row_num < NUM_ROWS, 
                    "current-row-num=%d in raster..handler", current_row_num);
            #endif

            draw_sprite_row(current_row_num);
            END_BORDER();
        }

        if ((++current_row_num) >= NUM_ROWS) {
            current_row_num = 0;
        }

        set_next_irq(inv_start_line[current_row_num], true);

        lines_used=vic.raster - prev_raster;
    }

    vic.intr_ctrl |= 0b10000000; //0xff;           //ACK irq

    //take_vic_snapshot();
    __asm{ 
        // lsr $d019   //vic.intr_ctrl -- ACK interrupt

        //NOTE: if you JMP to $EA81, (instead of $ea31) the keyboard will be disabled, which gets rid
        //      of annoying screen flicker when a key is pressed. Of course, you also cannot then
        //      use getch() to read the keyboard.

        //rti
        jmp $febc //$ea81   //(old_irq) - 
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
#pragma optimize(noinline)
 bool set_next_irq(unsigned int rasterline, bool calling_from_irq) {
//IRQ THREAD
    //from https://codebase64.com/doku.php?id=base:introduction_to_raster_irqs

    bool ok=false;

    //TODO: this check has GOT to be more expensive than just calling the redundant sei/cli
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



    vic.raster = (byte)rasterline; //&0b11111111;                    //rest of raster line# 
    if (rasterline < 256) {
        vic.ctrl1 &= 0b01111111; //0x7f;                           //MSb of raster line#
    }
    else {
        vic.ctrl1 |= 0b10000000;
    }
    //$d01a: 
    // Bit #0: 1 = Raster interrupt enabled.
    // Bit #1: 1 = Sprite-background collision interrupt enabled.
    // Bit #2: 1 = Sprite-sprite collision interrupt enabled.
    // Bit #3: 1 = Light pen interrupt enabled.
    vic.intr_enable = 1;
    ok = true;

    //NOTE:see note above
    if (! calling_from_irq) {
        __asm{
            cli
        }
    }

    //take_vic_snapshot();
    return ok;
}

//MAIN THREAD
//#pragma optimize(noinline)
void flip_row_image(byte row) {
    //__asm { cli }
    #ifdef MY_ASSERT
        inv_assert(row < NUM_ROWS, "row=%d in flip-row-image", row);
    #endif

    if (!row_alive[row]) return;

    if ((++(row_frame_num[row])) > row_max_frames[row]) {

        byte new_image_num=((row_image_num[row]+1) % row_num_images[row]);

        //inv_assert(new_image_num>0, "row=%d new-image-num=%d in flip-row-image", row, new_image_num);
        
        row_image_num[row]=new_image_num;
        row_frame_num[row]=0;        
    }
    //__asm { sei }
}



//MAIN thread
//This is about as fast as it's going to get with this approach.
//  Not a huge deal as it's running in the main thread anyway.
void find_min_max_spr_x() {
    rows_min_spr_x = MAX_SPR_X;
    rows_max_spr_x = MIN_SPR_X;

    
    for (byte c=0;c<INVADERS_PER_ROW;c++) {
        if (col_invs_left_alive[c] > 0) {
            int spr_col_x = col_x[c] + rows_x_shift;
            
            if (spr_col_x < rows_min_spr_x) {
                rows_min_spr_x = spr_col_x;
            }
            if (spr_col_x > rows_max_spr_x) {
                rows_max_spr_x = spr_col_x;
            }
        }
    }
    __asm {
        nop
    }
    return;
}//find_min_max_spr_x


/*
 * Check to see if the max/min X position has been crossed. If so,
 *  move all of the rows down and reverse their direction.
 *
 *  Uses rows_max_spr_x, rows_min_spr_x, calculated from previous draw_sprite_row() calls.
 *
 *  Returns true if OK, false if this bounce pushed the Invaders past the max Y allowed
*/
//MAIN thread
bool bounce_rows() {
    bool ok = true;

    find_min_max_spr_x();

    //TODO combine these 2 if's?
    if ((rows_x_frame_speed > 0) && (rows_max_spr_x >= MAX_SPR_X)) {
        #ifdef MY_ASSERT
            inv_assert((rows_x_frame_speed = 4), "rows_x_frame_speed=%d bounce_rows",rows_x_frame_speed);
        #endif

        ok= move_rows_down(Y_INC);
        rows_x_shift -= rows_x_frame_speed-1; //X_INC*2;
        rows_x_frame_speed *= -1;
    }
    else if ((rows_x_frame_speed < 0) && (rows_min_spr_x <= MIN_SPR_X)) {
        #ifdef MY_ASSERT
            inv_assert((rows_x_frame_speed = -4), "rows_x_frame_speed=%d bounce_rows",rows_x_frame_speed);
        #endif

        ok = move_rows_down(Y_INC);
        rows_x_shift -= rows_x_frame_speed-1; //X_INC*2;
        rows_x_frame_speed *= -1;
    }
    return ok;
}

//MAIN THREAD
//Returns true if OK, false if OOB
bool move_invaders() {
    bool ok = true;
    if ((++(rows_frame_num)) >= ROWS_MAX_FRAMES) {

        #ifdef MY_ASSERT
            inv_assert(rows_x_frame_speed == -4 || rows_x_frame_speed == 4,
                "rows_x_frame_speed=%d in main", rows_x_frame_speed);
        #endif

        rows_x_shift += rows_x_frame_speed;
        rows_frame_num = 0;
        //if past Y bounds, quit
        ok = bounce_rows();
    }
    return ok;
}


/*
 * Returns true if OK, false if the move causes the Invs to push down past their allowed Y bounds
*/
//MAIN thread
bool move_rows_down(byte px_down) {
    
    __asm {
        nop
    }

#ifdef DO_UNROLL
    #pragma unroll(full)
#endif
    for (int r=0;r<NUM_ROWS;r++) {       

        row_y[r] += px_down;
        inv_start_line[r] += px_down;

        //TODO make a proper GAME OVER
        if ((row_y[r] + INVADER_SPRITE_HEIGHT) >= (SHIP_Y+4)) { //MAX_Y_ROW) {
            return false;
        }
        // out_of_bounds != (row_y[r] > MAX_Y_ROW);

        // if (row_y[r] > MAX_Y_ROW) {
            
        //     vic.color_back = VCOL_RED;
        //     playing = false;
        //     printf("exit in move-rows-down()\n");
        //     return;
        // }

    }
    return true;
}

// void poll_inputs(char joy_num) {

// }

//MAIN thread
#pragma optimize(0)
bool handle_inputs(char joy_num) {

    START_BORDER(VCOL_WHITE);

    // signed int key_x_speed=0, key_y_speed=0;
    // bool key_fire_pressed = false;
    
    
    // // joy_poll(joy_num);
    // //keyb_poll();
    // char key = kr_read_key();

    // //vic.color_back=(keyb_key);

    // // while (c:\Users\chris\Downloads\spaxce invaders c64 multi.klakeyb_key != 0) {
    // //     joy_num++;
    // //     // __asm {
    // //     //     nop
    // //     // }
    // // };

    // //keyboard combos:
    // //  W
    // // ASD  fire=RETURN
    // //or
    // //  UP == SHIFT-DN
    // //LT  RT            fire = SPACE
    // // (LT is shift-RT arrow)
    // //
    // // OR joystick#2
    // //

    // if (key == 0) {
    //      return false;
    // }

    
    bool key_a_pressed = false;
    bool key_d_pressed = false;
    bool key_rtn_pressed = false;

    key_a_pressed      = kr_is_key_pressed(KR_ROW_A,       KR_COL_A); //(key == 'a') ; //(keyb_key  == (KSCAN_A | KSCAN_QUAL_DOWN)); //key_pressed(KSCAN_A);
    // if (key_a_pressed) {
    //     vic.color_back=VCOL_LT_BLUE;
    //     __asm {
    //         nop
    //         nop
    //         nop
            
    //     }
    // }
   key_d_pressed      = kr_is_key_pressed(KR_ROW_D,       KR_COL_D); //(key == 'd'); //keyb_key  == (KSCAN_D | KSCAN_QUAL_DOWN)); //key_pressed(KSCAN_D);
   // key_rtn_pressed = kr_is_key_pressed(KR_ROW_RETURN, KR_COL_RETURN); //(key == 13); // (keyb_key == (KSCAN_RETURN | KSCAN_QUAL_DOWN)); //key_pressed(KSCAN_RETURN);

    signed int new_x = obj_x[SHIP_OBJ_NUM];

    if (key_a_pressed) {
        new_x -=5;
        if (new_x >= MIN_SPR_X) {
            obj_x[SHIP_OBJ_NUM] = new_x;
            //return false;
        }
    }
    if (key_d_pressed) {
        new_x += 5;
        if (new_x <= MAX_SPR_X) {
            obj_x[SHIP_OBJ_NUM] = new_x;
            //return false;
        }
    } 
    if (key_rtn_pressed) {
        fire_bullet(BULLET_OBJ_NUM);
        END_BORDER();
        return true;
    }
    END_BORDER();
    return false;
}

//MAIN thread
void fire_bullet(byte obj_num) {
    //inv_assert(obj_type[obj_num] == TYPE_BULLET, "wrong playerobject type");
    if (obj_type[obj_num] == TYPE_BULLET) {
        sidfx_play(0, SIDFXFire, 1);

        obj_x[BULLET_OBJ_NUM] = obj_x[SHIP_OBJ_NUM];
        obj_y[BULLET_OBJ_NUM] = obj_y[SHIP_OBJ_NUM];
        obj_speed_x[BULLET_OBJ_NUM] = 0;
        obj_speed_y[BULLET_OBJ_NUM] = -1;
        obj_alive[BULLET_OBJ_NUM] = true;
        // b->x = ship.x;
        // b->y = ship.y-3;
        // b->speed_x = 0;
        // b->speed_y = -1;
        // b->alive = true;
        
        vic.color_back = VCOL_LT_GREEN;
    }
}

//byte obj_num2;
//MAIN thread
// #pragma  optimize(0)
void move_object(byte obj_num) {
    inv_assert(1 == 0, "Should be the other way round");
    if (obj_num == 0) {
        __asm {
            nop
            nop
            brk
        }
    }
    byte this_obj_num = obj_num;

    if (obj_num>NUM_OBJECTS) {
        vic.color_back=VCOL_ORANGE;

        __asm {
            nop
            nop
            nop
            brk
        }
    }
    // obj_num2 = obj_num;
    signed int this_x = obj_x[obj_num];

    bool too_low = (this_x < MIN_SPR_X);
    bool too_high = (this_x > MAX_SPR_X);

    if ( too_low || too_high ) {
        __asm { 
            nop 
            nop
            brk
        }
    }

    signed int new_x = obj_x[obj_num];

    if (obj_speed_x[obj_num] != 0) { 
        if (obj_speed_x[obj_num] > 0) {
            if (new_x < MAX_SPR_X) {
                new_x += obj_speed_x[obj_num];
            }
        } else{
            if (new_x > MIN_SPR_X) {
                new_x += obj_speed_x[obj_num];
            }
        }
        obj_speed_x[obj_num] = 0;
    }
    //signed int new_x = obj_x[this_obj_num];

    if ((new_x != obj_x[obj_num]) && (new_x>=MIN_SPR_X)  && (new_x<=MAX_SPR_X)) {
        obj_x[obj_num]=new_x;
    }
    else {
        __asm { 
            nop 
            nop
        }
    }

}

// void kill_object(byte obj_num) {
//     switch (obj_type[obj_num]) {
//         case TYPE_BULLET: {
//             kill_bullet(BULLET_OBJ_NUM);
//             obj_alive[obj_num] = false;
//             break;
//         }

//         case TYPE_SHIP: {
//             game_over();
//             break;
//         }

//         default: {
//             vic.color_back = VCOL_RED;
//             //printf("kill-object() got obj type %d\n", obj_type[obj_num]);
//             while(true);
//         }
//     }

// }
//MAIN thread
void draw_object(int obj_num) {
    //__asm { cli }

    byte draw_obj_obj_num = obj_num;

    #ifdef MY_ASSERT
                            //1234567890123456789012345678901234567890
        inv_assert(obj_num<2, "obj-num==%d in draw-object", obj_num);
    #endif
    

    if (obj_alive[obj_num]) {
        byte sprite_num = obj_sprite_num[obj_num];

        #ifdef MY_ASSERT
            inv_assert(sprite_num < 8, "sprite-num=%d in draw-object(%d)", sprite_num, obj_num);
        #endif
        spr_move(sprite_num, obj_x[obj_num], obj_y[obj_num]);

        spr_color(sprite_num, obj_sprite_color[obj_num]);
        //TODO always <255?
        if (obj_sprite_mcolor0[obj_num] < 0xff) {
            vic.spr_mcolor0 = obj_sprite_mcolor0[obj_num];
        }
        if (obj_sprite_mcolor1[obj_num] < 0xff) {
            vic.spr_mcolor1 = obj_sprite_mcolor1[obj_num];
        }
        
        #ifdef MY_ASSERT
                                            //  1234567890123456789012345678901234567890
            inv_assert(obj_sprite_num[obj_num] < 3, "obj-sprite-num[%d]=%d at draw-object", 
                obj_num, obj_sprite_num[obj_num]);
        #endif

                                                // 1234567890123456789012345678901234567890
        //inv_assert((obj_image_handle[obj_num]) > 0, "bad obj-image-handle[%d]:%d in draw-object", 
        //    (byte)obj_num, (byte)obj_image_handle[obj_num]);
            
        spr_image(obj_sprite_num[obj_num], obj_image_handle[obj_num]);

        #ifdef MY_ASSERT
                                                        //1234567890123456789012345678901234567890
            inv_assert(*((char *)0x0400+0x03f8+0) > 0, "sprite=0 in draw-object(%d)", draw_obj_obj_num);
        #endif

    }
    #ifdef MY_ASSERT
                                                //1234567890123456789012345678901234567890
        inv_assert(obj_sprite_num[obj_num] < 3,  "obj-sprite-num[%d]=%d at draw-object()", 
            obj_num, obj_sprite_num[obj_num]);
    #endif

    spr_show(obj_sprite_num[obj_num], obj_alive[obj_num]);

    //__asm{ sei }
}

//MAIN thread
void kill_bullet(byte obj_num) {
    obj_alive[obj_num] = false;
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
		//while (((raster=vic.raster) != lower) ) {
        while (true) {
            volatile int raster=vic.raster;
            if (vic.raster == lower) {
                break;
            }
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

//MAIN thread
void init_invaders() {
    //int index=0;

    current_row_num=0;
    
    for (int c=0;c<INVADERS_PER_ROW;c++) {
        col_invs_left_alive[c]  = NUM_ROWS;
    }

    for (int i=0;i<NUM_ROWS;i++) {
        inv_start_line[i] = INV_MIN_Y+SCANLINES_PER_ROW*i-SCANLINES_TO_DRAW_SPRITE;
        // inv_start_line[i] = MIN_Y+SCANLINES_PER_ROW*i-SCANLINES_TO_DRAW_SPRITE;
    }
    //TODO cheating
    inv_start_line[NUM_ROWS] = 230;

#ifdef DO_UNROLL
    //#pragma unroll(full)
#endif
    for (byte r=0;r<NUM_ROWS; r++) {
        // inv_assert(r < NUM_ROWS, "r is broken");
        row_y[r]                = INV_MIN_Y + SCANLINES_PER_ROW * r; //SCANLINES_PER_ROW * r;
        row_num_images[r]       = 2;
        row_image_handles[r][0] = INVADER_IMAGE_BASE + SPRITE_IMAGE_BASE +(r*2);
        row_image_handles[r][1] = INVADER_IMAGE_BASE + SPRITE_IMAGE_BASE +(r*2) + 1;
        row_image_num[r]        = 0;
        row_max_frames[r]       = ROW_MAX_FRAMES;
        row_frame_num[r]        = 0;
        row_alive[r]            = true;
        row_inv_index[r]        = r * INVADERS_PER_ROW;
        row_color[r]            = 0;    //Invaders don't use sprite main color
        row_mcolor0[r]          = (r + 2) % 16;
        row_mcolor1[r]          = (row_mcolor0[r] == VCOL_RED ? VCOL_GREEN : VCOL_RED);

        row_sprite_enable_mask[r] = 255;
        

// #ifdef DO_UNROLL
//         #pragma unroll(full)
// #endif
        
        for (int c=0;c<INVADERS_PER_ROW; c++) {
            byte index=row_inv_index[r]+c;
            inv_alive[index]            = true;
            inv_sprite_num[index]       = 2 + c;
            col_x[c]              = 0 + c*35;
        }
    }

    obj_x               = (signed int[]){160,           160};
    obj_speed_x         = (signed int[]){0,             0};
    obj_y               = (signed int[]){230,           230};
    obj_speed_y         = (signed int[]){0,             0};
    obj_alive           = (bool[])      {true,          false};
    obj_sprite_num      = (byte[])      {0,             1};
    obj_sprite_color    = (byte[])      {VCOL_WHITE,    VCOL_WHITE};
    obj_sprite_mcolor0  = (byte[])      {VCOL_GREEN,    VCOL_GREEN};
    obj_sprite_mcolor1  = (byte[])      {VCOL_RED,      VCOL_RED};
    obj_kill_on_border  = (bool[])      {false,         true};

    obj_type            = (PlayerObjectType[]){TYPE_SHIP,TYPE_BULLET};

    obj_image_handle    = (byte[]) {SPRITE_IMAGE_BASE + SHIP_IMAGE_NUM,
                                    SPRITE_IMAGE_BASE + SHIP_IMAGE_NUM};

    rows_frame_num = 0;
    
    rows_max_spr_x = MIN_SPR_X;
    rows_min_spr_x = MAX_SPR_X;
    
    __asm {
        nop
    }
}

void init_sprites() {
    //spr_init((char *)logo_screen);

    vic.spr_mcolor0 = 1;    //TODO change this raw #
    vic.spr_mcolor0 = 2;    //TODO change this raw #

#ifdef DO_UNROLL
    //#pragma unroll(full)
#endif
    for (int ic=0;ic<NUM_ROWS;ic++) {
        byte spr_num=ic+2;

        #ifdef MY_ASSERT
            inv_assert(spr_num<8, "spr_num=%d at init-sprites()",spr_num);
        #endif

        spr_image(spr_num, row_image_handles[0][row_image_num[0]]);

        spr_move(spr_num, ic*35+24 + 50,0);          //just ignore the Y coord for now

        spr_color(spr_num,ic+1);
        spr_show(spr_num,true);

    }
}

void display_logo(){
    vic.color_back=VCOL_BLACK;

    vic_setmode(VICM_HIRES_MC, logo_color,logo_bmp); // $d018=$49 $d011=$3b $dd00=$c6
}

void game_over() {
    playing = false;
}

// char getch_with_keybounce() {
//     char c=getch();
//     char old_c = c;

//     while (c==old_c) {
//         c = getchx();
//     }

//     return c;
// }

__forceinline const void START_BORDER(byte new_color) {
    if (DO_BORDER) {
        old_border_color = vic.color_border;
        vic.color_border = new_color;
    }
}

__forceinline const void END_BORDER() {
    if (DO_BORDER) {
        vic.color_border = old_border_color;
    }
}

/* Must be called before sid_rand() or sid_int_rand() */
byte init_sid_rand() {
    sid.voices[2].freq=0xffff;
    sid.voices[2].ctrl=0b10000000;   //noise waveform, gate off
}

/* 
*   Generate an 8-bit random value from SID. 
*   WARNING: Attempting to call this too quickly will just result in 
*   duplicate values. Also, using SID for music will
*   probably screw these values up.
*/
byte sid_rand() {
    return sid.random;
}

/** Generate a 16-bit random value from SID. */
unsigned int sid_int_rand() {
    return (unsigned int)(sid.random * 256 + sid.random);
}

/*
 *  Draw the text screen, including the top status line and 
 *  some (num_stars) random stars.
 * 
 * TODO: use a different charset
 */
void init_screen(byte num_stars) {
    clear_text_screen();

    for(byte i=0;i<num_stars;i++) {
        unsigned int pos;
        byte* text=(byte*)0x400;
        byte* color=(byte*)(0xd800);

        while ( text[pos=40+(rand() % 960)] != 32);
        text[pos]=46; //screencode for "."
        color[pos]=rand() % 16;
    }

    gotoxy(0,0);
    printf("LIVES:XXXXX");

    gotoxy(15,0);
    printf("INVADERS");

    gotoxy(27,0);
    printf("SCORE:000000");

}

void clear_hires_screen() {
        //clear out the hires stuff
    memset(logo_bmp, 0, 8000);
    memset(logo_screen, 0, 1000);
    memset(logo_color, 0, 1000);
}

void clear_text_screen() {
    //clear out the text stuff
    memset((void *)0x400, 0x20, 1000);
    memset((void *)0xd800, 0, 1000);    
}

