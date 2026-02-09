

//#pragma optimize(speed)

//#define VSPRITES_MAX 16

#include "invaders.h"
//
// Invaders...raping!
//

#define DO_UNROLL true

//#define Screen ((char *)0x400)
//#define Color ((char *)0xd800)

// char* hires_screen = ((char *)0x4000);
// char* hires_color = ((char *)0x4800);

char* text_screen = ((char *)0x400);
char* text_color = ((char *)0x1000);

//byte* Color = ((byte *)0xd800);
//byte* hires_color = ((byte *)0x4800);

#define LOGO_FILE "space_invaders_logo.kla" 


// spriteset at fixed location

#pragma data(spriteset_sec)

////
//  NOTE: anything like this, where its data that needs to be there, but the 
//      var itself isn't referenced anywhere, needs to be called out 
//      with __export or #pragma reference(name), or it will be optimized away!
////
__export static const char spriteset[] =  {
    #embed spd_sprites "invaders-2600.spd"

};

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


// #pragma data(hires_color)
// __export static const char logo_color[1000] = {
//     #embed 1000 9002 LOGO_FILE //0x03e8 (0x1f40+0x3e8)
// };

#pragma data(data)

__export int prev_raster=0;

__export int lines_used = -1;
__export int total_invs;

__export int flip_lines_used = -1;

//__export signed int useless =-1;
//bool first_time=true;

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

//#define TEST_KEYBOARD

//MAIN THREAD
#pragma optimize(0)
int main() {

    bool smooshed = false;


	// Disable CIA interrupts, we do not want interference
	// with our joystick interrupt
	//cia_init();


        // Activate trampoline
	mmap_trampoline();

	mmap_set(MMAP_NO_ROM);

    display_logo();


    //vic_waitBottom();
//    getch_with_keybounce();

    do {
        //vic_waitFrame();
        keyb_poll();
        //bool space_pressed = key_pressed(KSCAN_SPACE);
    } while (keyb_key == 0);

    //vic_setmode(VICM_TEXT, text_screen,text_color);
    
    //vic_setmode(VICM_TEXT, (char *)0x0400, (char *)0x2000);
    // *(char *)0xd018 = 0x15;

	//vic_setmode(VICM_HIRES_MC, hires_screen, hires_color);

   	//memset(text_screen, 32, 1000);
    // memset(hires_screen, 0, 8000);
    // memset(hires_color, 0, 1000);

    memset(logo_bmp, 0, 8000);
    memset(logo_screen, 0, 1000);
    memset(logo_color, 0, 1000);

        //point the VIC to the right screen (to record sprite #s for example)
    vic_setmode(VICM_HIRES_MC, logo_screen,logo_bmp); // $d018=$49 $d011=$3b $dd00=$c6

    //(logo_screen);
    //don't do this--it overwrites a chunk of your program
    //memset(text_color,2,1000);


    init_invaders();
    init_sprites();

    // Disable interrupts while setting up
	 __asm { sei };


    //TODO BUG: if the lower-right-est Invader is killed, the Player ship is no longer displayed

    //BUG:pressed keys cause all kinds of screen flicker & distortion.
    //  Answer: bypass the kernal keyboard read code when JMPing at the end of the IRQ handler.
    //          However, when you do that, you can't use the keyboard (duh!)

    //Kill **all** other interrupts?
    // __asm {
    //     lda #$7f
    //     sta $dc0d		 //turn off all types of cia irq/nmi.
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
    // }

	// // Kill CIA interrupts
	// cia_init();

    //We really don't have any need to map ROM out at this point,
    //  and not doing so has certain advantages (like being able
    //  to use getchs() ).
    //mmap_trampoline();
    //mmap_set(MMAP_NO_ROM);
    

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


    IRQ_VECTOR=raster_irq_handler;

    set_next_irq(inv_start_line[0], false);

     __asm { cli }

    int row_num = 0;

    collided_inv_index = 0xff;

    rows_x_shift = 50;
    rows_x_frame_speed = 4;

    rows_frame_num = 0;

    collided_inv_index=0xff;

    playing = true;

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
        move_object(SHIP_OBJ_NUM);
        //move_object(SHIP_OBJ_NUM);

        draw_object(SHIP_OBJ_NUM);
        draw_object(BULLET_OBJ_NUM);
#endif

        END_BORDER

        START_BORDER(VCOL_WHITE)

        //TODO it seems criminal to waste this time
        vic_waitBottom();
        //vic_waitLine(255);
        //TODO fix this & get collisions working
        //wait_line_and_watch_for_collisions(255);
        END_BORDER

        //Actually show the sprites, and move them
        START_BORDER(VCOL_BLUE)
        
        smooshed = ! move_invaders();
        set_sprites_for_all();
        if (smooshed) {
            playing = false;
            break;
        }
        
        END_BORDER

        //Flip the images
        START_BORDER(VCOL_BLACK)
#ifdef DO_UNROLL
        #pragma unroll(full)
#endif
        for (byte row=0;row<NUM_ROWS;row++) {
            flip_row_image(row);
        }
//        flip_lines_used=vic.raster - flip_lines;

        END_BORDER
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
        //logo_screen[0x3f8 + 0] = SPRITE_IMAGE_BASE + SMOOSHED_SHIP_IMAGE_NUM;

        //spr_expand(0,true,false);
    }
    vic.color_back=VCOL_RED;
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

//MAIN THREAD
void shoot_invader(byte si_row, byte si_col) {

    byte row_index = row_inv_index[si_row];
    byte inv_index = row_index + si_col;

    //my_assert(inv_alive[index],"zombie Invaders");
    if (! inv_alive[inv_index] ){
        //We've already killed this invader, so ignore it
        return;
    }

    inv_alive[inv_index]=false;
    // my_assert(col_invs_left_alive[si_col] > 0, "mismatch in col-invs--left-alive");
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

    // my_assert(spr_row<NUM_ROWS,"too many spr rows");

    //Instead of calling spr_show() 6 times, we pre-calc the spr_enable mask for the whole row
    //          in shoot_invader()
    //TODO why doesn't this turn off the player & bullet?
    vic.spr_enable = row_sprite_enable_mask[spr_row];

    if (!row_alive[spr_row]) {
        return;
    }

    byte new_handle = row_image_handles[spr_row][row_image_num[spr_row]];

    byte row_index = row_inv_index[spr_row];

    //byte this_row_color = row_color[spr_row];
    int this_row_y = row_y[spr_row];

    vic.spr_mcolor0 = row_mcolor0[spr_row];
    vic.spr_mcolor1 = row_mcolor1[spr_row];

#ifdef DO_UNROLL
    #pragma unroll(full)
#endif
    for (byte c=0;c<INVADERS_PER_ROW; c++) {
        byte inv_index = row_index + c; //row * INVADERS_PER_ROW + c;
        if (! inv_alive[inv_index]) {
            continue;
        }

        byte spr_num = c + 2;
        //logo_screen[0x3f8 + spr_num] = new_handle;
        spr_image(spr_num, new_handle);
        vic.spr_pos[spr_num].y= this_row_y;  //;do this last?
    }

    //take_vic_snapshot();
    //for debugging
    __asm {
        nop
    }
    //vic.color_back = VCOL_BLACK;
}

//TODO either use this or remove it

//  __forceinline void move_invader(byte index) {
//     //Invader* inv=&invaders[inv_num];
//     inv_x[index] += inv_speed_x[index];
//     inv_y += inv_speed_y[index];

//     if (inv_x[index] <20) {
//         inv_speed_x[index] = abs(inv_speed_x[index]);
//     }
//     else {
//         if (inv_x[index] >= 320){
//             inv_speed_x[index] = -abs(inv_speed_x[index]);
//         }
//     }

//     // spr_move(inv->sprite_num,inv->x,inv->y);
//     // vspr_move(inv->sprite_num,inv->x,inv->y);
// }

//IRQ THREAD
void raster_irq_handler() {

    if (playing) {
        int min_y=MIN_Y;

    //TODO needed? Useful?    
    //if (vic.intr_ctrl > 127) {          //This is a raster interrupt ONLY if bit 7 of intr_ctrl/$d019 is set

        prev_raster = vic.raster;


        if (prev_raster >= 230) {
            START_BORDER(VCOL_ORANGE)

            // my_assert(ship.sprite_num < 8, "bad ship sprite in rirq");
            //vic.spr_color[ship.sprite_num] = ship.sprite_color;

            //TODO always <0xff?
            
            if (obj_sprite_mcolor0[SHIP_OBJ_NUM] < 0xff) {
                vic.spr_mcolor0 = obj_sprite_mcolor0[SHIP_OBJ_NUM];
            }
            if (obj_sprite_mcolor1[SHIP_OBJ_NUM] < 0xff) {
                vic.spr_mcolor1 = obj_sprite_mcolor1[SHIP_OBJ_NUM];
            }
            END_BORDER
        }
        else {

            START_BORDER(VCOL_GREEN)
            draw_sprite_row(current_row_num);
            END_BORDER
        }

        if (++current_row_num > NUM_ROWS) {
            current_row_num = 0;
        }

        set_next_irq(inv_start_line[current_row_num], true);

        lines_used=vic.raster - prev_raster;
    }

    vic.intr_ctrl |= 0b10000000; //0xff;           //ACK irq

    //take_vic_snapshot();
    __asm{ 
        // lsr $d019   //vic.intr_ctrl -- ACK interrupt

        //NOTE: if you JMP to anything but $EA31, they keyboard will be disabled, which gets rid
        //      of annoying screen flicker when a key is pressed. Of course, you also cannot then
        //      use getch() to read the keyboard.

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
 //IRQ THREAD
bool set_next_irq(int rasterline, bool calling_from_irq) {
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
//TODO is this noinline necessary?
#pragma optimize(noinline)
void flip_row_image(byte row) {

    //TODO Another Oscar64 bug? If I leave the assert out, row 0 never gets flipped.
    //  I think it's getting inlined incorrectly
    //  ...or not, since adding noinline isn't helping
    // my_assert(row<NUM_ROWS, "too many rows to flip");

    if (!row_alive[row]) return;

    if ((++(row_frame_num[row])) >= row_max_frames[row]) {

        row_image_num[row]=((row_image_num[row]+1) % row_num_images[row]);
        row_frame_num[row]=0;        
    }
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
    if ((rows_x_frame_speed > 0) && rows_max_spr_x >= MAX_SPR_X) {
        my_assert((rows_x_frame_speed = 4), "rows_x_frame_speed bad in bounce_rows()");
        ok= move_rows_down(Y_INC);
        rows_x_shift -= rows_x_frame_speed-1; //X_INC*2;
        rows_x_frame_speed *= -1;
    }
    else if ((rows_x_frame_speed < 0) && rows_min_spr_x <= MIN_SPR_X) {
        my_assert((rows_x_frame_speed = -4), "rows_x_frame_speed bad in bounce_rows()");
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

        my_assert(rows_x_frame_speed == -4 || rows_x_frame_speed == 4,
            "rows_x_frame_speed bad value in main\n");

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
        if (row_y[r] + INVADER_SPRITE_HEIGHT >= (SHIP_Y+4)) { //MAX_Y_ROW) {
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

void poll_inputs(char joy_num) {

}
//MAIN thread
#pragma optimize(0)
bool handle_inputs(char joy_num) {

    signed int key_x_speed=0, key_y_speed=0;
    bool key_fire_pressed = false;

    // joy_poll(joy_num);
    keyb_poll();

    // while (keyb_key != 0) {
    //     joy_num++;
    //     // __asm {
    //     //     nop
    //     // }
    // };

    //keyboard combos:
    //  W
    // ASD  fire=RETURN
    //or
    //  UP == SHIFT-DN
    //LT  RT            fire = SPACE
    // (LT is shift-RT arrow)
    //
    // OR joystick#2
    //
    bool key_a_pressed = key_pressed(KSCAN_A);
    bool key_d_pressed = key_pressed(KSCAN_D);

    // bool key_pressed_csr_left = key_pressed(KSCAN_CSR_RIGHT && key_shift());
    // bool key_spc_pressed = key_pressed(KSCAN_SPACE);
    bool key_rtn_pressed = key_pressed(KSCAN_RETURN);

    if (key_a_pressed) {
        obj_speed_x[SHIP_OBJ_NUM] = -2;
    } else if (key_d_pressed) {
        obj_speed_x[SHIP_OBJ_NUM] = 2;
    } else if (key_rtn_pressed) {
        fire_bullet(BULLET_OBJ_NUM);
        return true;
    }
    return false;
}

//MAIN thread
void fire_bullet(byte obj_num) {
    //my_assert(obj_type[obj_num] == TYPE_BULLET, "wrong playerobject type");
    if (obj_type[obj_num] == TYPE_BULLET) {
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
        
        vic.color_back = 15;
    }
}

byte obj_num2;
//MAIN thread
#pragma  optimize(0)
void move_object(byte obj_num) {
    obj_num2 = obj_num;
    signed int this_x = obj_x[obj_num];

    if (obj_speed_x[obj_num] != 0) { 
        if (obj_speed_x[obj_num] > 0) {
            if (obj_x[obj_num] < MAX_SPR_X) {
                obj_x[obj_num] += obj_speed_x[obj_num];
            }
        } else{
            if (obj_x[obj_num] > MIN_SPR_X) {
                obj_x[obj_num] += obj_speed_x[obj_num];
            }
        }
        obj_speed_x[obj_num] = 0;
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
void draw_object(byte obj_num) {
    if (obj_alive[obj_num]) {
        byte sprite_num = obj_sprite_num[obj_num];
        spr_move(sprite_num, obj_x[obj_num], obj_y[obj_num]);

        spr_color(sprite_num, obj_sprite_color[obj_num]);
        //TODO always <255?
        if (obj_sprite_mcolor0[obj_num] < 0xff) {
            vic.spr_mcolor0 = obj_sprite_mcolor0[obj_num];
        }
        if (obj_sprite_mcolor1[obj_num] < 0xff) {
            vic.spr_mcolor1 = obj_sprite_mcolor1[obj_num];
        }
        
        spr_image(obj_sprite_num[obj_num], obj_image_handle[obj_num]);
    }
    spr_show(obj_sprite_num[obj_num], obj_alive[obj_num]);
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
    
    //my_assert(NUM_ROWS == 6, "**GURU MEDITATION ERROR**");
    for (int c=0;c<INVADERS_PER_ROW;c++) {
        col_invs_left_alive[c]  = NUM_ROWS;
    }

    for (int i=0;i<NUM_ROWS;i++) {
        inv_start_line[i] = MIN_Y+SCANLINES_PER_ROW*i-SCANLINES_TO_DRAW_SPRITE;
    }
    //TODO cheating
    inv_start_line[NUM_ROWS] = 230;

    //TODO: oscar64 bug? If I do #pragma(unroll) on this loop, row_y[0] gets corrupted
#ifdef DO_UNROLL
    #pragma unroll(full)
#endif
    for (byte r=0;r<NUM_ROWS; r++) {
        // my_assert(r < NUM_ROWS, "r is broken");
        row_y[r]                = MIN_Y + SCANLINES_PER_ROW * r;
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

#ifdef DO_UNROLL
        #pragma unroll(full)
#endif
        for (int c=0;c<INVADERS_PER_ROW; c++) {
            byte index=r*INVADERS_PER_ROW+c;
            inv_alive[index]            = true;
            //inv_speed_x[index]          = 1;
            //inv_speed_y[index]          = 0;
            inv_sprite_num[index]       = 2 + c;
            col_x[c]              = 0 + c*35;
            //inv_spr_pos_x[index]        = 0;
            //inv_row[index]              = r;
            //inv_col[index]              = c;
        }
    }
    __asm {
        nop
    }
}

void init_sprites() {
    spr_init((char *)logo_screen);

    vic.spr_mcolor0 = 1;
    vic.spr_mcolor0 = 2;

#ifdef DO_UNROLL
    //#pragma unroll(full)
#endif
    for (int ic=0;ic<NUM_ROWS;ic++) {
        byte spr_num=ic+2;

        // //int img_loc = (int)(&Screen[0x3f8 + spr_num]);
        // int row_image_handle_loc = (int)&row_image_handles[0][row_image_num[0]];
        // //*((char *)img_loc) = *((char *)row_image_handle_loc); //Screen[0x3f8 + spr_num]=  row_image_handles[0][row_image_num[0]];
        // byte handle = row_image_handles[0][row_image_num[0]];
        // spr_image(spr_num, handle );

        spr_image(spr_num, row_image_handles[0][row_image_num[0]]);
        //logo_screen[0x3f8 + spr_num] = row_image_handles[0][row_image_num[0]];

        spr_move(spr_num, ic*35+24 + 50,0);          //just ignore the Y coord for now
        spr_color(spr_num,ic+1);
        spr_show(spr_num,true);

    }
}

void display_logo(){

    //memcpy(hires_screen, logo_screen, 0x400);
    //memcpy(hires_color, logo_color, 0x400);

    vic.color_back=0;

    vic_setmode(VICM_HIRES_MC, logo_color,logo_bmp); // $d018=$49 $d011=$3b $dd00=$c6
    //vic_setbank(1);

    /**
    //from https://www.codebase64.net/doku.php?id=base:vicii_memory_organizing
    *(char*)0xdd00 = 2; //0x32; //*((char*)0xdd00) | 0b00000010 & 0b11111110;
    //from https://groups.google.com/g/comp.sys.cbm/c/p7owkelfs4s/m/QzAZTv9twZcJ
    //logo_screen = $5000 - $4000 = $1000 / $1000 = 1
    //bitmap = $6000 - $4000 = $2000 / $2000 = 1
    *(char*)0xd018 = 0x78; //0b01001000; //0x4f;
    //*(char *)0xd011 = (*(char *)0xd011) & 0b10111111 | 00100000;
    *(char*)0xd011 = 0x3b;
    *(char*)0xd016 = 0xd8; //0x18;
    //*(char *)0xd016 = (*(char *)0xd016) | 0b00010000;
    **/
    //memcpy((char *)0xd800, logo_color, 0x400);
}

void game_over() {
    playing = false;
}

char getch_with_keybounce() {
    char c=getch();
    char old_c = c;

    while (c==old_c) {
        c = getchx();
    }

    return c;
}

