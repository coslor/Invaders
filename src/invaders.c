
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
    #embed spd_sprites "invaders-2600.spd"

};


#pragma data(data)


int prev_raster=0;

__export int lines_used = -1;
__export int total_invs;

__export int flip_lines_used = -1;

__export signed int useless =0;
bool first_time=true;

byte target_row = 0;

//The PETSCII values go {F1,F3,F5,f7,f2,f4,f6}. This array maps them so we can use simple code
//  to decide which fn key goes with what row.  
const byte fn_key_row[7] = {
    0,2,4,0,1,3,5
};
const byte asdf_row[6] = "asdfgh";

int main() {
    //Bad things happen if these two get out of sync
    my_assert(SCANLINES_PER_ROW > SCANLINES_TO_DRAW_SPRITE, "scanlines constants are out of sync");

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

    bullet.alive = false;
    bullet.image_handle = 153;

    IRQ_VECTOR=raster_irq_handler;

    set_next_irq(inv_start_line[0]);

    __asm { cli }

    int row_num = 0;


    while(playing) {
        //rows_max_spr_x = MIN_SPR_X;
        //rows_min_spr_x = MAX_SPR_X;

        char key = getchx();
        if (key>='1' && key <= '6') {
            byte col=(key-'1');
            shoot_invader(target_row, col);
        }
        else if (key>=0x85 && key <= 0x8b) {
            target_row = fn_key_row[key - 0x85];

        }


#ifdef USE_BORDER
        vic.color_border = VCOL_BLUE;
#endif
        //vic_waitBottom();
        vic_waitLine(255);
#ifdef USE_BORDER
        vic.color_border = VCOL_WHITE;
#endif

        byte flip_lines = vic.raster;

        if ((++(rows_frame_num)) >= rows_max_frames) {
            rows_x_shift += rows_x_frame_speed;
            rows_frame_num = 0;
            bounce_rows();
        }

        //#pragma unroll(full)
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

        draw_object(ship);
        draw_object(bullet);

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
    for (int c=0;c<INVADERS_PER_ROW;c++) {
        int off=row_inv_index[si_row]+c;
        if (inv_alive[off]) {
            spr_mask |= 1<<inv_sprite_num[off];
        }
    }
    row_sprite_enable_mask[si_row] = spr_mask;

}


void draw_sprite_row(byte spr_row) {

    if (spr_row == target_row) {
        vic.color_back=VCOL_BLACK;
    }

    my_assert(spr_row < NUM_ROWS,"spr_row too high");

    byte raster_dsr = vic.raster;

    //Instead of calling spr_show() 6 times, we pre-calc the spr_enable mask for the whole row
    //          in shoot_invader()
    vic.spr_enable = row_sprite_enable_mask[spr_row];

    //We have to at least disable display of the dead row before exiting
    if (!row_alive[spr_row]) {
        return;
    }

    #pragma unroll(full)
    for (byte c=0;c<INVADERS_PER_ROW; c++) {
        byte inv_index = row_inv_index[spr_row] + c; //row * INVADERS_PER_ROW + c;
        byte spr_num=c+2;

        vic.spr_pos[spr_num].y=row_y[spr_row];

        byte new_handle = row_image_handles[spr_row][row_image_num[spr_row]];

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

        //NOTE: move all this to shoot
        int spr_pos_x = col_x_index[c] + rows_x_shift; //row_x_index[spr_row];
        inv_spr_pos_x[inv_index] = spr_pos_x;
        vic.spr_pos[spr_num].x = spr_pos_x & 0xff;
        if (spr_pos_x & 0x100)
            vic.spr_msbx |= 1 << spr_num;
        else
            vic.spr_msbx &= ~(1 << spr_num);
        
    }

    //for debugging
    __asm {
        nop
    }
    vic.color_back = VCOL_BLACK;
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

        set_next_irq(inv_start_line[(current_row_num)]);

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

void set_next_irq(int rasterline) {
    //from https://codebase64.com/doku.php?id=base:introduction_to_raster_irqs

    __asm {
        sei
    }

    cia1.sdr=0x7f;
    cia2.sdr=0x7f;
    byte b=cia1.sdr;
    b=cia2.sdr;

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
    for (int r=0;r<NUM_ROWS; r++) {
        row_y[r]                = MIN_Y + SCANLINES_PER_ROW * r;
        row_num_images[r]       = 2;
        row_image_handles[r][0] = 140+(r*2);
        row_image_handles[r][1] = 141+(r*2);
        row_image_num[r]        = 0;
        row_max_frames[r]       = ROW_MAX_FRAMES;
        row_frame_num[r]        = 0;
//        rows_x_index         = 50;
//        rows_x_frame_speed    = 4;
        row_alive[r]            = true;
        //row_max_inv_alive[r]   = INVADERS_PER_ROW - 1;
        //row_min_inv_alive[r]  = 0;
        row_inv_index[r]       = r * INVADERS_PER_ROW;

        row_dirty[r] = true;
        row_sprite_enable_mask[r] = 255;

        for (int c=0;c<INVADERS_PER_ROW; c++) {
            byte index=r*INVADERS_PER_ROW+c;
            inv_alive[index]            = true;
            inv_speed_x[index]          = 1;
            inv_speed_y[index]          = 0;
            inv_sprite_num[index]       = 2 + c;
            inv_color[index]            = c + (r & 3)+1;
            inv_old_x[index]            = 0;
            inv_old_y[index]            = 0;
            col_x_index[c]              = 0 + c*35;
            inv_spr_pos_x[index]        = 0;


        }
    }
    //row_alive[3]=false;
}

void init_sprites() {
    for (int ic=0;ic<6;ic++) {
        byte spr_num=ic+2;

        Screen[0x3f8 + spr_num] = row_image_handles[0][row_image_num[0]];

        spr_move(spr_num, ic*35+24 + 50,0);          //just ignore the Y coord for now
        spr_color(spr_num,ic+1);
        spr_show(spr_num,true);

    }
}

void find_min_max_spr_x() {
    rows_min_spr_x = MAX_SPR_X;
    rows_max_spr_x = MIN_SPR_X;

    for (int row=0;row<NUM_ROWS;row++) {
        if (! row_alive[row]) continue;

        byte row_index = row_inv_index[row];

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

void bounce_rows() {
    find_min_max_spr_x();
    //TODO: we need to add a corresponding find_max_spr_y(), so that we know
    //      when the Invaders have won!

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
    for (int r8=0;r8<NUM_ROWS;r8++) {
        row_y[r8] += px_down;
        //TODO use max_spr_y (or maybe just check for alive rows here?) to 
        //      find out when the Invaders have won.
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
        useless=ship.speed_x;
    }
}

void move_object(PlayerObject *obj) {
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

    if (obj->speed_y > 0) {
        if (obj->y < 255) {
            obj->y += obj->speed_y;
        }
        else if (obj->speed_y < 0) {
            if (obj->y > 50) {
                obj->y -= obj->speed_y;
            }
        }
    }
}

void draw_object(PlayerObject obj2) {
    if (obj2.alive) {
        spr_move(obj2.sprite_num, obj2.x, obj2.y);
        spr_image(obj2.sprite_num, obj2.image_handle);
    }
    spr_show(obj2.sprite_num, obj2.alive);
}