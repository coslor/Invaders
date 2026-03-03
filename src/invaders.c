#pragma optimize(speed)

//#define VSPRITES_MAX 16

#include "invaders.h"
//
// Invaders...raping!
//

//////////////////////
// SCANLINES
//	These constants determine the timing of the raster interrupts. 
//		Make them too small, and the Invader sprites become
//		flashing garbage. Too large, and you can't fit all of the Invaders 
//		onscreen. Check "lines_used" to see a snapshot of the # of lines
//		that you've been taking to set up the next sprite.
//
//	Note that it takes more time to draw big sprites than small ones.
//		Interesting, no? The times also vary based on how many sprites
//		in a row, and how many rows.
///////////////////////

static const byte  BIG_SHIPS_LINES_SPRITE=16;
static const byte  BIG_SHIPS_LINES_ROW=12 + BIG_SHIPS_LINES_SPRITE;

static const byte  SMALL_SHIPS_LINES_SPRITE=13;
static const byte  SMALL_SHIPS_LINES_ROW=18 + SMALL_SHIPS_LINES_SPRITE;

/* How many scanlines BEFORE the sprite is to be shown, do we need for setup (color, image, etc)?*/
static const byte  SCANLINES_TO_BUILD_SPRITE=    SMALL_SHIPS_LINES_SPRITE;

/* How many total scanlines, including SCANLINES_TO_BUILD_SPRITE and the time
 *	it actually takes to draw the sprite, do we need for each row?
*/
static const byte  SCANLINES_PER_ROW=           SMALL_SHIPS_LINES_ROW;
/////////////////////
//	NOTE:when you tune these ^^^^ be sure to check for sprite collisions
//		afterwards. I have no goddamn idea why, but cutting the timing too thin
//		results in collisions being ignored *sometimes*. It may not fail
//		until the 3rd or 4th or later collision.
/////////////////////


char* text_screen = ((char *)0x400);
char* text_color = ((char *)0x1000);


#pragma data(data)

__export int prev_raster=0;

// Keep this debugging variable. 
//	Tells you the # of lines your code is taking to set up the next sprite.
//	Very useful when trying to determine timing. 
// @see SCANLINES_TO_BUILD_SPRITE 
__export int lines_used = -1;
__export int total_invs;

__export int flip_lines_used = -1;


byte target_row = NUM_ROWS-1;

//The PETSCII values go {F1,F3,F5,f7,f2,f4,f6}. This array maps them so we can use simple code
//  to decide which fn key goes with what row.  
const byte fn_key_row[7] = {
	0,2,4,0,1,3,5
};

const byte asdf_row[6] = "asdfgh";

//TODO refactor this
__export byte coll_spr_num=0xff;
__export byte coll_spr_y = 0xff;

const unsigned short invaders_2600_size;

const int JOY_NUM=0;

volatile bool collision = false;
volatile int coll_line = -1;

//Bitmap	bitmap;
Bitmap		bitmap = {
	(static char *)logo_bmp, nullptr, 40, 25, 320
};
//MAIN THREAD
//#pragma optimize(0)
int main() {

	iocharmap(IOCHM_PETSCII_1);

	// int x = 320;
	// fxp_12_4 fxp_x = to_fxp_12_4(x);
	// fxp
	bool smooshed = false;

	// Disable CIA interrupts, we do not want interference
	// with our joystick interrupt
	cia_init();

	bm_init(&bitmap, (char *)logo_bmp,40,25);

	display_logo();

	sidfx_init();
	sid.fmodevol = 15;
	
	//this is just here to play with the SIDFx stuff
	while(true) {
		vic_waitFrame();
		sidfx_loop();
		keyb_poll();
		joy_poll(JOY_NUM);


		if (key_pressed(KSCAN_1)){
			sidfx_play(0, SIDFXFire, 1);
		}
		if (key_pressed(KSCAN_2)) {
			sidfx_play(0, SIDFXExplosion, 1);
		}
		if (key_pressed(KSCAN_SPACE) || joyb[JOY_NUM]) {
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
	vic.color_border = VCOL_LT_BLUE;

	clear_hires_screen();

	//clear_text_screen();
	init_screen_hires(25);

	wait_for_fire();

	vic.color_back=VCOL_BLACK;
	//All sprites are multicolor EXCEPT for the bullet!
	vic.spr_multi   = 0b11111101;
	vic.spr_mcolor0 = VCOL_LT_GREEN;
	vic.spr_mcolor1 = VCOL_RED;

	//point the VIC to the right screen (to record sprite #s for example)
	//TODO BANK vic_setmode(VICM_TEXT, (char *)0x0400, (char *)0x1000);
	//spr_init((char *)0x400);
	spr_init(logo_color);

	init_invaders();
	init_sprites();

	// Disable interrupts while setting up
	 __asm { sei };


	//TODO BUG: if the lower-right-est Invader is killed, the Player ship is no longer displayed STILL HERE??

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
		lda $d01e       //make sure sprite collisions cleared
		lda $d01f
	}

	//After loading & showing logo, it's Ok to turn BASIC off
	// mmap_trampoline();
	// mmap_set(MMAP_NO_ROM);
	

	//spr_color(obj_sprite_num[BULLET_OBJ_NUM], VCOL_RED);

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

	//collided_inv_index = 0xff;

	//How far all cols are shifted to the right as Invaders move back & forth
	cols_x_shift = 50;

	rows_x_frame_speed = 4;

	rows_frame_num = 0;

	//collided_inv_index=0xff;

	playing = true;
	draw_object((byte)0); //initialize ship image

	//ACK any flags already here
	vic.intr_ctrl = 0xff;
	IRQ_VECTOR=handle_irq;
	vic.intr_enable = 0b00001011;  //sprite-sprite collision interrupts enabled
	set_next_raster_irq(raster_irq_line[0], false);

	////
	// START MAIN LOOP
	////
	while(playing) {

		//read inputs & move bullet
		if (playing) {
			handle_inputs(JOY_NUM);
			//move_object(SHIP_OBJ_NUM);
			if (obj_alive[BULLET_OBJ_NUM]) {
				move_object(BULLET_OBJ_NUM);
				draw_object(BULLET_OBJ_NUM);
			}
		}

		draw_object(SHIP_OBJ_NUM);
		
		//TODO it seems criminal to waste this time
		vic_waitBottom();

		for (byte r=0;r<NUM_ROWS;r++) {
			draw_sprite_row(&row_vic[r], &(row_sprite_handles[r*8]), r);
		}

		START_BORDER(VCOL_PURPLE);
		//play slice of SID sound FX each frame
		sidfx_loop();
		END_BORDER();


		//MOVE_INVADERS
		//Actually show the sprites, and move them
		START_BORDER(VCOL_LT_GREEN);

		if (playing) {
			smooshed = ! move_invaders();
		}

		set_sprites_for_all();
		END_BORDER();

		if (smooshed) {
			game_over();

			break;
		}
		

		if (playing) {
			START_BORDER(VCOL_ORANGE);

			#pragma unroll(full)
			for (byte row=0;row<NUM_ROWS;row++) {
				flip_row_image(row);
			}
			END_BORDER();
		}

		//COLLISION HANDLING
		START_BORDER(VCOL_LT_GREY);
		if (frame_collision_count > 0) {
			kill_bullet(BULLET_OBJ_NUM);

			//TODO should this be reset here, or after waitBottom()? Maybe both?
			frame_collision_count = 0;

			int coll_x = obj_x[BULLET_OBJ_NUM];
			int coll_y = frame_collision_line[frame_collision_count];

			byte row_found=0xff, col_found=0xff;
			for (byte r=0;r<NUM_ROWS;r++) {
				int dist = abs(coll_y - row_inv_spr_pos_y[r]);
				if (dist <= 30) {
					row_found = r;
					break;
				}
			}
			for (byte c=0;c<INVADERS_PER_ROW;c++) {
				int dist=abs(coll_x - cols_inv_spr_pos_x[c]);
				if (dist < 30) {
					col_found = c;
					break;
				}
			}

			if (row_found!=0xff && col_found!=0xff) {
				kill_invader(row_found, col_found);
			}
			else {
				DEBUG_POINT();
			}
			END_BORDER();
		}
	}; //while playing

	if (smooshed) {
		for (byte i=0;i<60;i++) {
			vic_waitFrame();
			sidfx_loop();
		}

		gotoxy(15,11);
		printf("GAME OVER");
		gotoxy(13,12);
		printf("PRESS ANY KEY");

	};

	while (kr_read_key() == 0) { vic_waitFrame(); };
   
	inv_assert(false, NULL);

}


//MAIN THREAD
void kill_invader(byte si_row, byte si_col) {

	byte row_index = row_inv_index[si_row];
	byte inv_index = row_index + si_col;

	if (! inv_alive[inv_index] ){
		//We've already killed this invader, so ignore it
		return;
	}

	inv_alive[inv_index]=false;

	col_invs_left_alive[si_col]--;

	//There's no col_alive yet...should there be?
	// if (col_invs_left_alive[si_col]==0) {
	// 	col_alive[si_row]=false;
	// }

	byte invs_alive = (--row_invs_left_alive[si_row]);
	if (invs_alive==0) {
		row_alive[si_row]=false;
	}

	byte spr_mask=0;

	//TODO this is UGLY, FIX IT
	spr_mask = row_sprite_enable_mask[si_row];

	//spr_mask ^= pow2[(5-si_col)+2]; 
	spr_mask &= (0xff-pow2[si_col+2]);

	if (obj_alive[0]) spr_mask |=1; else spr_mask &= 0b11111110;
	if (obj_alive[1]) spr_mask |=2; else spr_mask &= 0b11111101;
	
	row_sprite_enable_mask[si_row] = spr_mask;

	sidfx_play(0, SIDFXExplosion, 1);

}

//MAIN THREAD
void set_sprites_for_all() {
#ifdef DO_UNROLL    
	#pragma unroll(full)
#endif
	for (byte c=0;c<INVADERS_PER_ROW;c++) {
		//TODO replace with inv_sprite_num?
		byte spr_num = c+2;

		/////TODO re-check this optimization
		//if (! col_invs_left_alive[c]) { continue; }

		int spr_pos_x = col_x[c] + cols_x_shift; //row_x_index[spr_row];
		cols_inv_spr_pos_x[c]    = spr_pos_x;

		//Using this instead of vic.sprxy() saves us a few cycles by not setting sprite.y
		vic.spr_pos[spr_num].x = spr_pos_x; //& 0xff
		if (spr_pos_x > 0xff)
			vic.spr_msbx |= 1 << spr_num;
		else
			vic.spr_msbx &= ~(1 << spr_num);
	}
}

//IRQ THREAD
inline void draw_sprite_row(VIC* this_vic, byte* sprite_handle_bytes, byte inv_row) {

	inv_assert(inv_row < 6 && sprite_handle_bytes > 0, "OOB at draw_sprite_row:%d %u\n",
		inv_row, sprite_handle_bytes);

	//	Note the "&=" instead of "=", since the Invaders have to co-exist with the Objects
	//@see row_sprite_enable_mask
	this_vic->spr_enable = row_sprite_enable_mask[inv_row];

	if (!row_alive[inv_row]) {
		return;
	}

	int this_row_y = row_y[inv_row];

	byte handle_num=row_image_handle_num[inv_row];
	byte new_handle = row_image_handle[handle_num];

	#pragma unroll(full)
	for (byte c=0;c<INVADERS_PER_ROW; c++) {
		//Update Y as early as possible. That way, even if we end up with a 
		//	discolored or distorted sprite, at least we'll see something.
		byte spr_num = c+2;
		
		int spr_pos_y_int = (int)&(this_vic->spr_pos[spr_num].y);
		*(byte *)spr_pos_y_int= this_row_y;  
		row_inv_spr_pos_y[inv_row] = this_row_y;
		
		//spr_image(spr_num, new_handle);
		//row_sprite_handles[inv_row*8+spr_num] = new_handle;
		sprite_handle_bytes[spr_num] = new_handle;
	}

	vic.spr_mcolor0 = row_mcolor0[inv_row];
	vic.spr_mcolor1 = row_mcolor1[inv_row];
}

void draw_sprite_row_asm(byte spr_row) {
	byte c;
	byte new_handle;
	byte spr_num;
	int this_row_y;

	__asm {
		//line 426
		LDY spr_row //(spr_row + 0) //P0 instead??
		LDA row_sprite_enable_mask,y // (row_sprite_enable_mask[0] + 0)
		STA $d019
		//line 428 
		LDA row_alive,y // if ! row_alive exit
		BNE s6
	s5:
		NOP
		NOP
		RTS
	s6:
		//line 444
		LDA #$02
		//448
		STA P0
		//444
		STA spr_num // (spr_num + 0)
		//441
		LDA #$00
		STA c // (c + 0)
		//437
		TYA
		ASL
		STA this_row_y 
		TAX
		LDA $38ae,x // (row_y[0] + 0)
		STA T3 + 0 
		STA $39a5,x // (row_inv_spr_pos_y[0] + 0)
		STA $d005 
		STA $9ff4 // (this_row_y + 0)
		LDA $38af,x // (row_y[0] + 1)
		STA T3 + 1 
		STA $39a6,x // (row_inv_spr_pos_y[0] + 1)
		STA $9ff5 // (this_row_y + 1)
		LDA $38cc,y // (row_image_num[0] + 0)
		CLC
		ADC T0 + 0 
		TAX
		LDA $38c0,x // (row_image_handles[0][0] + 0)
		STA P1 
		STA $9ff3 // (new_handle + 0)
		JSR $16a4 // (spr_image.s4 + 0)
		LDA T3 + 0 
		LDX T0 + 0 
		STA $39a5,x // (row_inv_spr_pos_y[0] + 0)
		STA $d007 
		LDA T3 + 1 
		STA $39a6,x // (row_inv_spr_pos_y[0] + 1)
		LDA #$03
		INC P0 
		STA $9ff1 // (spr_num + 0)
		LDA #$01
		STA $9ff2 // (c + 0)
		JSR $16a4 // (spr_image.s4 + 0)
		LDA T3 + 0 
		LDX T0 + 0 
		STA $39a5,x // (row_inv_spr_pos_y[0] + 0)
		STA $d009 
		LDA T3 + 1 
		STA $39a6,x // (row_inv_spr_pos_y[0] + 1)
		LDA #$04
		INC P0 
		STA $9ff1 // (spr_num + 0)
		LDA #$02
		STA $9ff2 // (c + 0)
		JSR $16a4 // (spr_image.s4 + 0)
		LDA T3 + 0 
		LDX T0 + 0 
		STA $39a5,x // (row_inv_spr_pos_y[0] + 0)
		STA $d00b 
		LDA T3 + 1 
		STA $39a6,x // (row_inv_spr_pos_y[0] + 1)
		LDA #$05
		INC P0 
		STA $9ff1 // (spr_num + 0)
		LDA #$03
		STA $9ff2 // (c + 0)
		JSR $16a4 // (spr_image.s4 + 0)
		LDA T3 + 0 
		LDX T0 + 0 
		STA $39a5,x // (row_inv_spr_pos_y[0] + 0)
		STA $d00d 
		LDA T3 + 1 
		STA $39a6,x // (row_inv_spr_pos_y[0] + 1)
		LDA #$06
		INC P0 
		STA $9ff1 // (spr_num + 0)
		LDA #$04
		STA $9ff2 // (c + 0)
		JSR $16a4 // (spr_image.s4 + 0)
		LDA T3 + 0 
		LDX T0 + 0 
		STA $39a5,x // (row_inv_spr_pos_y[0] + 0)
		STA $d00f 
		LDA T3 + 1 
		STA $39a6,x // (row_inv_spr_pos_y[0] + 1)
		LDA #$07
		INC P0 
		STA $9ff1 // (spr_num + 0)
		LDA #$05
		STA $9ff2 // (c + 0)
		JSR $16a4 // (spr_image.s4 + 0)
		LDX P2 // (spr_row + 0)
		LDA $38f6,x // (row_mcolor0[0] + 0)
		STA $d025 
		LDA $3900,x // (row_mcolor1[0] + 0)
		STA $d026 
	s3:
		RTS		
	}
}

/**
*   Build a list of all sprite-sprite collisions that occur in one frame. 
*   NOTES:  1) frame_irq_collision_count should be reset each frame (thus the name)
*           2) if the # of collisions exceeds MAX_COLLISIONS, then the last detected collision mask
*               will be in frame_irq_collision_mask[collision_count] & the rest will be silently
*               discarded.
*           3) for each collision, the (approximate-but-pretty-close) rasterline it occurred on 
				in frame_will also be stored
			4) OK, this is pretty much overkill, since we are only using the first collision anyway,
				but it's pretty cheap & it could come in handy someday
*/
void register_collision(byte coll_mask, byte raster) {
	byte new_coll_count = frame_collision_count;
	frame_collision_mask[frame_collision_count]=coll_mask;
	frame_collision_line[frame_collision_count] = raster;	//passed raster along in case we get interrupted
	if (new_coll_count<MAX_FRAME_COLLISIONS) new_coll_count++;
	frame_collision_count = new_coll_count;
}

void handle_irq() {
//IRQ THREAD

	//intr_ctrl  =$d019
	//intr_enable=$d01a

	byte intr_ctrl=vic.intr_ctrl;
	//Check to see if this is a VIC interrupt. If not, ignore it
	if (intr_ctrl & 0b10000000) {
		if (intr_ctrl & 0b00000100) {   //collision IRQ
			vic.intr_ctrl = 00000100;    //ACK spr-spr interrupt
			//vic.intr_ctrl = intr_ctrl;

			//NOTE: apparently we need to read spr_sprcol AFTER we ack the interrupt.
			//      This is safe (sort-of-atomic) because we're in an IRQ handler
			register_collision(vic.spr_sprcol, vic.raster);

			// //TODO do we need to read spr_sprcol here?
			coll_spr_num = vic.spr_sprcol;
		}
		//TODO this assumes no sprite-background collisions, may need to be extended
		else {  
			prev_raster = vic.raster;
			handle_raster_irq(prev_raster);
			lines_used=vic.raster - prev_raster;
			vic.intr_ctrl = 0b00000001;
		}
	}

	//ACK all currently active interrupt events. Only do this when all enabled sources (raster, 
	//  sprite-back collision, and/or sprite-sprite collision) have been dealt with.
	//TODO: What if we don't do this? Re-enable if necessary
	//'cause here's the deal: what if we have multiple events (raster irq, collision irq) on
	//  the same rasterline? I'm assuming that we would get multiple IRQs, but would
	//  the line below mask out any additional ones that haven't been dealt with yet?
	vic.intr_ctrl |= 0b10000000; 

	vic.intr_enable = 0b00000101;
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

void handle_raster_irq(byte raster) {
//IRQ THREAD
	if (raster >= 230) {

		// if (obj_sprite_mcolor0[SHIP_OBJ_NUM] < 0xff) {
		// 	vic.spr_mcolor0 = obj_sprite_mcolor0[SHIP_OBJ_NUM];
		// }
		// if (obj_sprite_mcolor1[SHIP_OBJ_NUM] < 0xff) {
		// 	vic.spr_mcolor1 = obj_sprite_mcolor1[SHIP_OBJ_NUM];
		// }
	}
	else {
		//vic.color_back = VCOL_BLACK;

		START_BORDER(VCOL_GREEN);
		//draw_sprite_row(current_row_num);

		copy_vic(&vic, &row_vic[current_row_num]);//vic=row_vic[...]
		memcpy((byte *)logo_color+1000, (byte *)(row_sprite_handles + current_row_num), 8);
		END_BORDER();
	}

	//FIXME infinite loop if no rows are alive
	while (true) {
		if ((++current_row_num) >= NUM_ROWS) {
			current_row_num = 0;
		}
		if (row_alive[current_row_num]) {
			set_next_raster_irq(raster_irq_line[current_row_num], true);
			break;
		}
	}

	vic.intr_ctrl = 0b10000011;    //ACK raster interrupt
}

/*
 *  Called from the raster IRQ handler -- calculates the line# for the next raster IRQ and gives
 *		it to the VIC to set up. Returns true if the raster hasn't already passed the requested line (plus a buffer),
 *      false otherwise.
 */
bool set_next_raster_irq(unsigned int rasterline, bool calling_from_irq) {
//IRQ THREAD
	//from https://codebase64.com/doku.php?id=base:introduction_to_raster_irqs

	bool ok=false;

	//NOTE: t his ch eck is important!
	if (! calling_from_irq) { //SEI
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
	vic.intr_enable = 0b00000101;
	ok = true;

	//NOTE:see note above
	if (! calling_from_irq) { //CLI
		__asm{
			cli
		}
	}

	return ok;
}

//MAIN THREAD
/**
 * 	Increment the Invader row flip counter. If it reaches the max, change the current 
 * 		image to the next one in the row's animation loop.
 */
void flip_row_image(byte row) {
	//__asm { cli }
	if ((++(row_frame_num[row])) > row_max_frames[row]) {

		byte new_image_num=((row_image_num[row]+1) % row_num_images[row]);

		//inv_assert(new_image_num>0, "row=%d new-image-num=%d in flip-row-image", row, new_image_num);
		
		row_image_num[row]=new_image_num;
		row_frame_num[row]=0;        
	}
	//__asm { sei }
}



//This is about as fast as it's going to get with this approach.
//  Not a huge deal as it's running in the main thread anyway.
void find_min_max_spr_x() {
//MAIN thread
	rows_min_spr_x = MAX_SPR_X;
	rows_max_spr_x = MIN_SPR_X;

	
	for (byte c=0;c<INVADERS_PER_ROW;c++) {
		if (col_invs_left_alive[c] > 0) {
			int spr_col_x = cols_inv_spr_pos_x[c];
			
			if (spr_col_x < rows_min_spr_x) {
				rows_min_spr_x = spr_col_x;
			}
			if (spr_col_x > rows_max_spr_x) {
				rows_max_spr_x = spr_col_x;
			}
		}
		else {
			__asm {
				nop
				nop
			}
		}
	}
	__asm {
		nop
	}
	return;
}//find_min_max_spr_x

/*
*	Find the bottom-most row with any Invaders in it, then find the first living Invader in that row
*		and use that inv_spr_y[] value as our rows_max_spr_y
*/
void find_rows_max_spr_y() {
	rows_max_spr_y=0;
	for (byte r=NUM_ROWS-1;r>=0;r--) {
		if (! row_alive[r]) {
			continue;
		}
		byte inv_index = row_inv_index[r];
		for (byte c=0;c<INVADERS_PER_ROW;c++) {
			if (inv_alive[inv_index+c]) {
				rows_max_spr_y=row_inv_spr_pos_y[r];
				return;
			}//if
		}//for c
	}//for r
	if (rows_max_spr_y == 0 ) {
		vic.color_back=VCOL_RED;
	}
}


/*
 * Check to see if the max/min X position has been crossed. If so,
 *  move all of the rows down and reverse their direction.
 *
 *  Uses rows_max_spr_x, rows_min_spr_x, calculated from previous draw_sprite_row() calls.
 *
 *  Returns true if OK, false if this bounce pushed the Invaders past the max Y allowed
*/
bool bounce_rows() {
//MAIN thread

	//START_BORDER(VCOL_YELLOW);

	bool ok = true;

	find_min_max_spr_x();
	find_rows_max_spr_y();

	//TODO combine these 2 if's?
	if ((rows_x_frame_speed > 0) && (rows_max_spr_x >= MAX_SPR_X)) {
		ok= move_rows_down(Y_INC);
		cols_x_shift -= rows_x_frame_speed-1; //X_INC*2;
		rows_x_frame_speed *= -1;
	}
	else if ((rows_x_frame_speed < 0) && (rows_min_spr_x <= MIN_SPR_X)) {
		// #ifdef MY_ASSERT
		// 	inv_assert((rows_x_frame_speed = -4), "rows_x_frame_speed=%d bounce_rows",rows_x_frame_speed);
		// #endif

		ok = move_rows_down(Y_INC);
		cols_x_shift -= rows_x_frame_speed-1; //X_INC*2;
		rows_x_frame_speed *= -1;
	}
	//END_BORDER();
	return ok;
}

//MAIN THREAD
//Returns true if OK, false if OOB
bool move_invaders() {
	bool ok = true;
	if ((++(rows_frame_num)) >= ROWS_MAX_FRAMES) {

		cols_x_shift += rows_x_frame_speed;
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

	#pragma unroll(full)
	for (byte r=0;r<NUM_ROWS;r++) {       

		row_y[r] += px_down;
		raster_irq_line[r] += px_down;

		//TODO make a proper GAME OVER
		//if ((row_y[r] + INVADER_SPRITE_HEIGHT) >= (SHIP_Y+4)) { //MAX_Y_ROW) {
		if (rows_max_spr_y >= (SHIP_Y-10)) {
			return false;
		}
	}
	return true;
}



//MAIN thread
/**
 * 	Process inputs from the keyboard & mouse. We should be able to deal
 * 		With more than one (2-3, depending) key down at once.
 * 
 * 		Right now, the keyboard controls are immutable, and are:
 * 		A  D	= left,right
 * 		RETURN	= fire
 **/
bool handle_inputs(char joy_num) {

	keyb_poll();
	joy_poll(joy_num);

			// printf("%c", keyb_codes[keyb_key & 0b01111111]);

	START_BORDER(VCOL_WHITE);

	// if (key == 0) {
	//      return false;
	// }

	   // //Cheat keys
		// // f1-f7    : choose the current row
		// // 1-6      : shoot the invader on the current row, in that column
		// char key = getchx();
		// if (key>='1' && key <= '6') {
		//     byte col=(key-'1');
		//     kill_invader(target_row, col);
		//     //TODO FIXME BUG doing a printf() after changing back to text mode 
		//     // causes a rather catastrophic crash -- the emulator locks up!
		//     // // printf("pew pew\n");
		// }
		// else if (key>=0x85 && key <= 0x8b) {
		//     target_row = fn_key_row[key - 0x85];
		// }

	byte key = keyb_codes[keyb_key & 0b01111111];
	
	// if ((keyb_key == 0) && (joyb[joy_num]==0)){
	//     return false;
	// }

	bool go_left    = 
		key_pressed(KSCAN_A) || (joyx[joy_num] == -1);  
	  
	bool go_right   = 
		key_pressed(KSCAN_D) || (joyx[joy_num] == 1);
	bool fire       = 
		key_pressed(KSCAN_RETURN) || (joyb[joy_num] > 0);

	signed int new_x = obj_x[SHIP_OBJ_NUM];

	//NOTE: we don't want to use a switch() here, since more than 1 key can be
	//      pressed at once (up to 3, depending on the keys involved).
	if (go_left == true) {
		new_x -= SHIP_SPEED;
		if (new_x >= MIN_SPR_X) {
			obj_x[SHIP_OBJ_NUM] = new_x;
		}
	}
	if (go_right) {
		new_x += SHIP_SPEED;
		if (new_x <= MAX_SPR_X) {
			obj_x[SHIP_OBJ_NUM] = new_x;
		}
	} 
	if (fire) {
		fire_bullet(BULLET_OBJ_NUM);
		END_BORDER();
		return true;
	}
	END_BORDER();
	return false;
}

//MAIN thread
void fire_bullet(byte obj_num) {
	if (obj_type[obj_num] == TYPE_BULLET) {
		sidfx_play(0, SIDFXFire, 1);

		obj_x[BULLET_OBJ_NUM] = obj_x[SHIP_OBJ_NUM];

		obj_y[BULLET_OBJ_NUM] = obj_y[SHIP_OBJ_NUM];
		//obj_y_12_4[BULLET_OBJ_NUM] = obj_y_12_4[SHIP_OBJ_NUM];
		obj_speed_x[BULLET_OBJ_NUM] = 0;

		//Convert BULLET_SPEED to fix12.4 & put it in y
		//obj_speed_y_12_4[BULLET_OBJ_NUM] = BULLET_SPEED_12_4;
		obj_speed_y[BULLET_OBJ_NUM] = -1;

		obj_sprite_num[BULLET_OBJ_NUM] = 1;
		obj_image_handle[BULLET_OBJ_NUM] = BULLET_IMAGE_NUM;

		obj_alive[BULLET_OBJ_NUM] = true;

		//enable showing the bullet in all rows
		for (byte r=0;r<NUM_ROWS;r++) {
			row_sprite_enable_mask[r] |=2;
		}
		
		//vic.color_back = VCOL_LT_GREEN;
	}
}

//MAIN thread
/**
 * 	Move a PlayerObject
 */
void move_object(byte obj_num) {
	signed int this_x = obj_x[obj_num];

	if (obj_speed_x[obj_num] != 0) { 
		signed int new_x = obj_x[obj_num];
		if (obj_speed_x[obj_num] > 0) {
			if (new_x < MAX_SPR_X) {
				new_x += obj_speed_x[obj_num];
			}
		} else{
			if (new_x > MIN_SPR_X) {
				new_x += obj_speed_x[obj_num];
			}
		}
		//If it didn't move, don't do anything
		if (new_x != obj_x[obj_num]) {
			if ((new_x>=MIN_SPR_X)  && (new_x<=MAX_SPR_X)) {
				obj_x[obj_num]=new_x;
			}
			else if (obj_kill_on_border[obj_num]) {
				kill_object(obj_num);
			}//if obj_kill
		}//if new_x!=obj_x
	}//if obj_speed
	
	//TODO refactor this to eliminate copypasta & endless braces

	if (obj_speed_y[obj_num] != 0) { 
		signed int new_y = obj_y[obj_num];
		if (obj_speed_y[obj_num] > 0) {
			if (new_y < MAX_SPR_Y) {
				new_y += obj_speed_y[obj_num];
			}
		} else {
			if (new_y > MIN_SPR_Y) {
				new_y += obj_speed_y[obj_num];
			}
		} 
		//if (new_y != obj_y[obj_num]) {      //has anything changed?
			if ((new_y>MIN_SPR_Y)  && (new_y<MAX_SPR_Y)) {    //new_y is in bounds
				obj_y[obj_num]=new_y;            
			} else {
				if (obj_kill_on_border[obj_num]) {   //we have hit a border. Should we kill the object?
					kill_object(obj_num);
					return;
				}//if (obj_kill_on_border)
			}//if new_y in bounds
		//}//if (new_y != obj_y)
	} //if obj_y_speed != 0
}   //move_object()

/*
 *	"Kill" a PlayerObject
 */
void kill_object(byte obj_num) {
	switch (obj_type[obj_num]) {
		case TYPE_BULLET: {
			kill_bullet(BULLET_OBJ_NUM);
			break;
		}

//         case TYPE_SHIP: {
//             game_over();
//             break;
//         }

		default: {
			vic.color_back = VCOL_LT_RED;
			printf("kill-object() got obj type %d\n", obj_type[obj_num]);
			while(true);
		}
	}
}

//MAIN thread
/*
 *	"Draw" (set up the sprite for) the PlayerObjects, like the ship & bullet.
*/
void draw_object(byte obj_num) {	
	byte sprite_num = obj_sprite_num[obj_num];
	spr_show(sprite_num, obj_alive[obj_num]);

	if (obj_alive[obj_num]) {
		
		spr_move(sprite_num, obj_x[obj_num], obj_y[obj_num]);

		spr_color(sprite_num, obj_sprite_color[obj_num]);
		//TODO always <255?
		if (obj_sprite_mcolor0[obj_num] < 0xff) {
			vic.spr_mcolor0 = obj_sprite_mcolor0[obj_num];
		}
		if (obj_sprite_mcolor1[obj_num] < 0xff) {
			vic.spr_mcolor1 = obj_sprite_mcolor1[obj_num];
		}
		//TODO? assumes only 2 objects
		spr_image(obj_sprite_num[obj_num], obj_image_handle[obj_num]);
		vic.spr_enable |= obj_num;
	}
	else  {
		vic.spr_enable &= (255 - pow2[obj_num]);
	}

	//__asm{ sei }
}

//MAIN thread
void kill_bullet(byte obj_num) {
	obj_alive[obj_num] = false;
	spr_show(obj_sprite_num[obj_num], false);
}

/**
*   coll_spr_y 
 */
byte wait_line_and_watch_for_collisions(byte line)
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
	
	for (byte c=0;c<INVADERS_PER_ROW;c++) {
		col_invs_left_alive[c]  = NUM_ROWS;
	}

	//TODO implement raster splits at 0 and 8 to switch in/out of text mode
	for (byte i=0;i<NUM_ROWS;i++) {
		raster_irq_line[i] = INV_MIN_Y+SCANLINES_PER_ROW*i-SCANLINES_TO_BUILD_SPRITE;
		// raster_irq_line[i] = MIN_Y+SCANLINES_PER_ROW*i-SCANLINES_TO_BUILD_SPRITE;
	}
	//TODO cheating
	raster_irq_line[NUM_ROWS] = 230;

	for (byte r=0;r<NUM_ROWS; r++) {
		// inv_assert(r < NUM_ROWS, "r is broken");
		row_y[r]                = INV_MIN_Y + SCANLINES_PER_ROW * r; //SCANLINES_PER_ROW * r;

		row_num_images[r]       = 2;
		row_image_handle_num[r]	= r*2;
		row_image_handle[r*2]	= INVADER_IMAGE_BASE +(r*2);
		row_image_handle[r*2+1]	= INVADER_IMAGE_BASE +(r*2) + 1;
		// row_image_handles[r][0] = INVADER_IMAGE_BASE +(r*2);
		// row_image_handles[r][1] = INVADER_IMAGE_BASE +(r*2) + 1;
		row_image_num[r]        = 0;

		row_max_frames[r]       = ROW_MAX_FRAMES;
		row_frame_num[r]        = 0;
		row_invs_left_alive[r]	= INVADERS_PER_ROW;
		row_alive[r]            = true;
		row_inv_index[r]        = r * INVADERS_PER_ROW;
		row_color[r]            = 0;    //Invaders don't use sprite main color
		row_mcolor0[r]          = (r + 2) % 16;
		row_mcolor1[r]          = (row_mcolor0[r] == VCOL_RED ? VCOL_GREEN : VCOL_RED);

		row_sprite_enable_mask[r] = 0b11111111;
		
		for (byte c=0;c<INVADERS_PER_ROW; c++) {
			byte inv_index			= row_inv_index[r]+c;
			inv_alive[inv_index]	= true;
			inv_sprite_num[inv_index] = 2 + c;
			// inv_spr_x[inv_index]	= 0;
			// inv_spr_x[inv_index]	= 0;
		}
	}

	for (byte c=0;c<INVADERS_PER_ROW;c++) {
		col_x[c] = 0 + c*35;
	}

	obj_x               = (signed int[]){160,           0};
	obj_speed_x         = (signed int[]){0,             0};

	//Convert y into 12.4
	obj_y          =   //(fxp12_4[])   { int_to_fxp12_4(230),  int_to_fxp12_4(230) };
							//(signed int[]){(230<<4),      230<<4};
						(signed int[])	{SHIP_Y,		SHIP_Y};
	obj_speed_y    =    //(fxp12_4[])   { int_to_fxp12_4(0),    frac_to_fxp12_4(8)  };
						(signed int[])	{0,             0};

	obj_alive           = (bool[])      {true,          false};
	obj_sprite_num      = (byte[])      {0,             1};
	obj_sprite_color    = (byte[])      {VCOL_WHITE,    VCOL_RED};
	obj_sprite_mcolor0  = (byte[])      {VCOL_GREEN,    VCOL_GREEN};
	obj_sprite_mcolor1  = (byte[])      {VCOL_RED,      VCOL_RED};
	obj_kill_on_border  = (bool[])      {false,         true};
	obj_type            = (PlayerObjectType[])
										{TYPE_SHIP,		TYPE_BULLET};
	obj_image_handle    = (byte[])  	{SHIP_IMAGE_NUM, BULLET_IMAGE_NUM};
	
	rows_frame_num      = 0;
	
	rows_max_spr_x      = MIN_SPR_X;
	rows_min_spr_x      = MAX_SPR_X;
	
	for (byte i=0;i<NUM_ROWS;i++) {
		//row_vic[i]=vic;
		copy_vic(&(row_vic[i]), &vic);
	}
	__asm {
		nop
	}
}

void init_sprites() {
	//spr_init((char *)logo_screen);

	vic.spr_mcolor0 = 1;    //TODO change this raw #
	vic.spr_mcolor0 = 2;    //TODO change this raw #

	//#pragma unroll(full)
	for (byte ic=0;ic<NUM_ROWS;ic++) {
		byte spr_num=ic+2;

		#ifdef MY_ASSERT
			inv_assert(spr_num<8, "spr_num=%d at init-sprites()",spr_num);
		#endif
		spr_set(spr_num, true, ic*35+24 + 50, 0, row_image_handle[0], 0, true, false, false);
		// spr_image(spr_num, row_image_handle[0]);

		// spr_move(spr_num, ic*35+24 + 50,0);          //just ignore the Y coord for now

		// spr_color(spr_num,ic+1);
		// spr_show(spr_num,true);

	}

	//Make copies of this sprite setup for each row
	for (byte r=0;r<NUM_ROWS;r++) {
		//memcpy((byte *)(&(row_vic[r]), vic, sizeof(VIC));
		//row_vic[r]=vic;
		copy_vic(&row_vic[r], &vic);
		memcpy(&(row_sprite_handles[8*r]), (byte*)(logo_color + 1000), 8);
	}
}

void display_logo(){
	vic.color_back=VCOL_BLACK;

	vic_setmode(VICM_HIRES_MC, logo_color,logo_bmp); // $d018=$49 $d011=$3b $dd00=$c6
}

void game_over() {
	playing = false;
	sidfx_play(0, SIDFXExplosion, 1);
	spr_image(0, SMOOSHED_SHIP_IMAGE_NUM);

}


inline const void START_BORDER(byte new_color) {
	if (DO_BORDER) {
		old_border_color = vic.color_border;
		vic.color_border = new_color;
	}
}

inline const void END_BORDER() {
	if (DO_BORDER) {
		vic.color_border = old_border_color;
	}
}

/* Must be called before sid_rand() or sid_int_rand() */
void init_sid_rand() {
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
 * TODO: use a different charset for the text
 */
void init_screen(byte num_stars) {
	clear_text_screen();

	for(byte i=0;i<num_stars;i++) {
		unsigned int pos;
		byte* text=(byte*)0x4000;	//FIXME remove magic number
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

__noinline void init_screen_hires(byte num_stars) {
	for (byte i=0;i<num_stars;i++) {
		int x=rand() % 320;
		int y=rand() % 200;
		//int c=rand() % 16;

		bm_set(&bitmap, x,y);
		int logo_offset = (40 *(y>>3))+x>>3;//40 * (y & ~7) + (x & ~7) + (y & 7);
		//logo_color[logo_offset] = 1;
		logo_screen[logo_offset] = 2;
		//logo_color[(y/40)+(x/8)] = 1;

		// unsigned int pos;
		// byte* text=(byte*)0x4000;	//FIXME remove magic number
		// byte* color=(byte*)(0xd800);

		// while ( text[pos=40+(rand() % 960)] != 32);
		// text[pos]=46; //screencode for "."
		// color[pos]=rand() % 16;
		vic_waitBottom();
		
	}
}
void clear_hires_screen() {
		//clear out the hires stuff
	memset(logo_bmp, 0x00, 8000);
	memset(logo_screen, 1, 1000);
	memset(logo_color, 2, 1000);
}

void clear_text_screen() {
	//clear out the text stuff
	//FIXME remove magic
	memset((void *)0x400, 0x20, 1000);
	memset((void *)0xd800, 0, 1000);    
}

void wait_for_fire() {
	while(true) {
		vic_waitFrame();
		keyb_poll();
		joy_poll(JOY_NUM);

		if (keyb_key != 0) {
			break;
		}
	}
}

void copy_vic(VIC *vic_dest, VIC *vic_src) {
	//TODO optimize this so it only copies the bytes we really need
	//This copy skips the first 2 x,y pairs, so that we're leaving sprites 0 & 1 alone
	//memcpy((struct VIC *)0xd000 + 3,&(row_vic[current_row_num])+3, sizeof(VIC)-4);//dest,source,len
	//	vic=row_vic[current_row_num];
	//*(vic_dest)=*(vic_src);

	if ((int)vic_dest == 0xd000) {
		DEBUG_POINT();
	} else {
		DEBUG_POINT();
	};

	#pragma unroll(full)
	for (byte i=2;i<8;i++) {
		vic_dest->spr_pos[i].x	= vic_src->spr_pos[i].x;
		vic_dest->spr_pos[i].y	= vic_src->spr_pos[i].y;
		vic_dest->spr_color[i]	= vic_src->spr_color[i];
	}
	vic_dest->spr_msbx 		= (vic_dest->spr_msbx & 0b00000011) | (vic_src->spr_msbx & 0b11111100);
	//no raster
	//no light pen
	vic_dest->spr_enable 	= (vic_dest->spr_enable & 0b00000011) | (vic_src->spr_enable & 0b11111100);
	vic_dest->ctrl2 		= vic_src->ctrl2;
	//vic_dest->spr_expand_y = vic_src->spr_expand_y;
	//vic_dest->memptr		= vic_src->memptr;
	//vic_dest->intr_ctrl1	= vic_src->intr_ctrl;
	//intr_enable
	//spr_priority

	vic_dest->spr_multi		= (vic_dest->spr_multi & 0b00000011) | (vic_src->spr_multi & 0b11111100);
	//spr_expand_x
	//spr_sprcol
	//spr_backcol
	//color_border
	//color_back2
	//color_back3
	//color_back
	vic_dest->spr_mcolor0	= vic_src->spr_mcolor0;
	vic_dest->spr_mcolor1	= vic_src->spr_mcolor1;
}