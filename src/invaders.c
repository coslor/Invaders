#pragma optimize(speed)

//#define VSPRITES_MAX 16

#include "invaders.h"
//
// Invaders...raping!
//

#pragma data(data)

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

static const byte  BIG_SHIPS_LINES_SPRITE=8;
static const byte  BIG_SHIPS_LINES_ROW=18 + BIG_SHIPS_LINES_SPRITE;

static const byte  SMALL_SHIPS_LINES_SPRITE=20;
static const byte  SMALL_SHIPS_LINES_ROW=12 + SMALL_SHIPS_LINES_SPRITE;

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


// char* text_screen = ((char *)0x400);
// char* text_color = ((char *)0x1000);

// #define LOGO_FILE "resources/space_invaders_logo.kla"

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
__export byte back_coll_spr_num=0xff;


const unsigned short invaders_2600_size;

const int JOY_NUM=0;

volatile bool collision = false;
volatile int coll_line = -1;

Bitmap		bitmap = {
	(static char *)logo_bmp, nullptr, 40, 25, 320
};

// Invaders...looting!

//MAIN THREAD
//#pragma optimize(0)
int main() {

	iocharmap(IOCHM_PETSCII_1);

	mmap_trampoline();
	mmap_set(MMAP_NO_BASIC);

	bool smooshed = false;

	// Disable CIA interrupts, we do not want interference
	// with our joystick interrupt
	cia_init();

	bm_init(&bitmap, (byte *)logo_bmp,40,25);
	music_init(0);

	display_logo();

	sidfx_init();
	sid.fmodevol = 15;
	
	//this is just here to play with the SIDFx stuff
	while(true) {
		#ifdef PLAY_LOGO_MUSIC		
		music_play();
		#endif
		vic_waitFrame();
		sidfx_loop();
		keyb_poll();
		joy_poll(JOY_NUM);


//        if (kr_is_key_pressed(KR_ROW_1, KR_COL_1)) {
		if (key_pressed(KSCAN_1)){
			sidfx_play(0, SIDFXFire, 1);
		}
		// if (kr_is_key_pressed(KR_ROW_2, KR_COL_2)) {
		if (key_pressed(KSCAN_2)) {
			sidfx_play(0, SIDFXExplosion, 1);
		}
		if (key_pressed(KSCAN_3)) {
		    sidfx_play(2, SIDFXBigExplosion, 3);

		}

		if (key_pressed(KSCAN_SPACE) || joyb[JOY_NUM]) {
			__asm {
				nop
				nop
			}
			//    getchx();
			break;
		}
	}

	//We're going to use the SID later for music & sfx, which will
	//  mess up any attempts to use it for PRNG so let's just
	//  use it once to seed srand(). Having waited for a keypress
	//  adds just the soupcon of true randomness we really need.

	init_sid_rand();
	//NOTE: let's go for predictability during development
	srand(1);

	vic.color_back = VCOL_LT_GREY;
	vic.color_border = VCOL_DARK_GREY;

	clear_hires_screen();

	init_screen_mc(30);

	//point the VIC to the right screen (to record sprite #s for example)
	spr_init(logo_color);

	init_invaders();
	init_sprites();

	// Disable interrupts while setting up
	 __asm { sei };

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
	

	//All sprites are multicolor EXCEPT for the missile!
	vic.spr_multi   = 0b11111101;
	vic.spr_mcolor0 = VCOL_LT_GREEN;
	vic.spr_mcolor1 = VCOL_RED;
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
	vic.intr_enable = 0b00000111;  //sprite-sprite collision interrupts enabled
	set_next_raster_irq(raster_irq_line[0], false);

	////
	// START MAIN LOOP
	////
	while(playing) {

		vic.spr_multi   = 0b11111101;
		vic.spr_expand_x= 0;
		vic.spr_expand_y= 0;
		vic.spr_mcolor0 = VCOL_LT_GREEN;
		vic.spr_mcolor1 = VCOL_RED;


		START_BORDER(VCOL_MED_GREY);
		//read inputs & move bullet
		if (playing) {
			START_BORDER(VCOL_WHITE);
			handle_inputs(JOY_NUM);
			END_BORDER();
			//move_object(SHIP_OBJ_NUM);
			if (obj_alive[BULLET_OBJ_NUM]) {
				move_object(BULLET_OBJ_NUM);
				draw_object(BULLET_OBJ_NUM);
			}
		}
		draw_object(SHIP_OBJ_NUM);
		END_BORDER();//blue

		START_BORDER(VCOL_LT_GREY);
		//TODO it seems criminal to waste this time
		vic_waitBottom();
		END_BORDER(); //light gray

		START_BORDER(VCOL_LT_GREEN);
		//play slice of SID sound FX each frame
		sidfx_loop();
		END_BORDER();//lt_green

		START_BORDER(VCOL_YELLOW);
		if (playing) {
			smooshed = ! move_invaders() ;
			move_bombs();
		}
		END_BORDER();
		
		START_BORDER(VCOL_CYAN);
		set_sprites_for_all();
		END_BORDER();

		if (smooshed) {
			game_over();

			break;
		}
		

		START_BORDER(VCOL_BROWN);
		#pragma unroll(full)
		for (byte row=0;row<NUM_ROWS;row++) {
			flip_row_image(row);
		}
		END_BORDER(); //brown

		START_BORDER(VCOL_PURPLE);
		//for (coll=0;coll<frame_collision_count;coll++) {
		if (frame_collision_count > 0) {
			//TODO assuming that it's always the bullet here; should check
			//	coll_mask
			kill_bullet(BULLET_OBJ_NUM);

			//TODO should this be reset here, or after waitBottom()? Maybe both?
			frame_collision_count = 0;

			int coll_x = obj_x[BULLET_OBJ_NUM];
			int coll_y = frame_collision_line[frame_collision_count];

			byte row_found=0xff, col_found=0xff;
			for (int r=0;r<NUM_ROWS;r++) {
				int dist = abs(coll_y - row_inv_spr_pos_y[r]);
				if (dist <= FIND_ROW_DISTANCE) {
					row_found = r;
					break;
				}
			}
			for (int c=0;c<INVADERS_PER_ROW;c++) {
				int dist=abs(coll_x - col_inv_spr_pos_x[c]);
				if (dist < FIND_COL_DISTANCE) {
					col_found = c;
					break;
				}
			}

			if (row_found!=0xff && col_found!=0xff) {
				kill_invader(row_found, col_found);
			}
			else {
				__asm {
					nop
					nop
				}
			}
		}
		END_BORDER();//purple

	} //while playing

	if (smooshed) {
		while (true) {
			vic_waitFrame();
			sidfx_loop();
			keyb_poll();
			joy_poll(JOY_NUM);	
			
			if (kr_read_key() != 0 || joyb[JOY_NUM] != 0) {
				break;
			}
		}

		// gotoxy(15,11);
		// printf("GAME OVER");
		// gotoxy(13,12);
		// printf("PRESS ANY KEY");

	}

	//while (kr_read_key() == 0) { vic_waitFrame(); };
   
	//Reset the C64
	inv_assert(false, p"Goodbyte Cruel World");

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

		bool found_live_row = false;
		for (int r=0;r<NUM_ROWS;r++) {
			if (row_alive[r]) { found_live_row = true; }
		}
		if (! found_live_row) {
			//TODO do something appropriate here if you win
			inv_assert(false, "YOU WIN!");
		}
	}

	byte spr_mask=0;

	//TODO this is UGLY, FIX IT
	spr_mask = row_sprite_enable_mask[si_row];

	//spr_mask ^= pow2[(5-si_col)+2]; 
	spr_mask &= (0xff-pow2[si_col+2]);

	if (obj_alive[0]) spr_mask |=1; else spr_mask &= 0b11111110;
	if (obj_alive[1]) spr_mask |=2; else spr_mask &= 0b11111101;
	
	row_sprite_enable_mask[si_row] = spr_mask;

	sidfx_play(0, InvaderDieFX, 1);

}

//MAIN THREAD
void set_sprites_for_all() {
	#pragma unroll(full)
	for (int c=0;c<INVADERS_PER_ROW;c++) {
		//TODO replace with inv_sprite_num?
		byte spr_num = c+2;

		/////TODO re-check this optimization
		//if (! col_invs_left_alive[c]) { continue; }

		int spr_pos_x = col_x[c] + cols_x_shift; //row_x_index[spr_row];
		col_inv_spr_pos_x[c]    = spr_pos_x;

		//Using this instead of vic.sprxy() saves us a few cycles by not setting sprite.y
		vic.spr_pos[spr_num].x = spr_pos_x; //& 0xff
		if (spr_pos_x > 0xff)
			vic.spr_msbx |= 1 << spr_num;
		else
			vic.spr_msbx &= ~(1 << spr_num);
	}

}

//IRQ THREAD
inline void draw_sprite_row(byte spr_row) {

	vic.spr_enable = row_sprite_enable_mask[spr_row];
	// if (row_sprite_enable_mask[spr_row] != 0b11111111){
	// 	// __asm {
	// 	// 	nop
	// 	// 	nop

	// 	// }
	// }

	if (!row_alive[spr_row]) {
// 		__asm {
// //            cli
// 			nop
// 			nop
// 		}
		return;
	}

	int this_row_y = row_y[spr_row];

	byte new_handle = row_latest_handle[spr_row];//row_image_handles[spr_row][row_image_num[spr_row]];
	//#pragma unroll(full)
	for (byte c=0;c<INVADERS_PER_ROW; c++) {
		//Update Y as early as possible. That way, even if we end up with a 
		//	discolored or distorted sprite, at least we'll see something.
		byte spr_num = c+2;
		
		//inv_assert(spr_num<8,"Bad spr_num:%d at draw_spr_row");

		vic.spr_pos[spr_num].y= this_row_y;  
		row_inv_spr_pos_y[spr_row] = this_row_y;
		spr_image(spr_num, new_handle);
	}

	vic.spr_mcolor0 = row_mcolor0[spr_row];
	vic.spr_mcolor1 = row_mcolor1[spr_row];

	// byte new_handle = row_image_handles[spr_row][row_image_num[spr_row]];

	// #pragma unroll(full)
	// for (byte c=0;c<INVADERS_PER_ROW; c++) {
	// 	byte spr_num = c + 2;
	// 	spr_image(spr_num, new_handle);
	// }
}

/*
*   Build a list of all sprite-sprite collisions that occur in one frame. 
*   NOTES:  1) frame_irq_collision_count should be reset each frame (thus the name)
*           2) if the # of collisions exceeds MAX_COLLISIONS, then the last detected collision mask
*               will be in frame_irq_collision_mask[collision_count] & the rest will be silently
*               discarded.
*           3) for each collision, the (approximate-but-pretty-close) rasterline it occurred on 
				in frame_will also be stored
*/
void register_collision(byte coll_mask, byte raster) {
	byte new_coll_count = frame_collision_count;
	frame_collision_mask[frame_collision_count]=coll_mask;
	frame_collision_line[frame_collision_count] = raster;	//passed raster along in case we get interrupted
	if (new_coll_count<MAX_FRAME_COLLISIONS) new_coll_count++;
	frame_collision_count = new_coll_count;
}

/** 
 * Registers only collisions between bombs & the ship
*/
void register_bomb_collision(byte coll_mask, byte raster) {
	__asm {
		nop
	}
}

//IRQ THREAD
//#pragma optimize(0)
void handle_irq() {
	//intr_ctrl  =$d019
	//intr_enable=$d01a

	byte intr_ctrl=vic.intr_ctrl;
	//Check to see if this is a VIC interrupt. If not, ignore it
	if (intr_ctrl & 0b10000000) {
		if (intr_ctrl & 0b00000100) {   //spr-spr collision IRQ

			// //We shouldn't(?) have to do this, since we're running in an interrupt anyway. 
			// //	But better safe than sorry
			// __asm {
			// 	sei
			// }
			vic.intr_ctrl = 00000100;   //ACK spr-spr interrupt
			//vic.intr_ctrl = intr_ctrl;

			//NOTE: apparently we need to read spr_sprcol AFTER we ack the interrupt.
			register_collision(vic.spr_sprcol, vic.raster);

			coll_spr_num = vic.spr_sprcol;

			// __asm {
			// 	cli
			// }
		}
		else if (intr_ctrl & 0b00000010) {	//is it spr-back collision?
			vic.intr_ctrl = 0b00000010;		//Yes, so ACK spr-back IRQ
			back_coll_spr_num = vic.spr_backcol;
			if ((back_coll_spr_num & 0x00000011) != 0) {	//are the ship or bullet involved? 
				register_bomb_collision(back_coll_spr_num, vic.raster);//if so, do something
			}
		}
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

	vic.intr_enable = 0b00000111;
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
	// if (raster >= 230) {

	// 	// if (obj_sprite_mcolor0[SHIP_OBJ_NUM] < 0xff) {
	// 	// 	vic.spr_mcolor0 = obj_sprite_mcolor0[SHIP_OBJ_NUM];
	// 	// }
	// 	// if (obj_sprite_mcolor1[SHIP_OBJ_NUM] < 0xff) {
	// 	// 	vic.spr_mcolor1 = obj_sprite_mcolor1[SHIP_OBJ_NUM];
	// 	// }
	// }
	// else 
	{
		vic.color_back = VCOL_BLACK;

		START_BORDER(VCOL_GREEN);
		draw_sprite_row(current_row_num);
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

	//NOTE: this check is important!
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
	vic.intr_enable = 0b00000111;
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
	#ifdef MY_ASSERT
		inv_assert(row < NUM_ROWS, "row=%d in flip-row-image", row);
	#endif

	// if (row_alive[row] == false) {
	//     __asm {
	//         nop
	//         nop
	//     }
	//     return;
	// }
	// else {
	//     __asm {
	//         nop
	//         nop
	//     }
	// }

 	if ((++(row_frame_num[row])) > row_max_frames[row]) {

		byte new_image_num=((row_image_num[row]+1) % row_num_images[row]);
		row_image_num[row]=new_image_num;
		row_latest_handle[row]=row_image_handles[row][new_image_num];

		//inv_assert(new_image_num>0, "row=%d new-image-num=%d in flip-row-image", row, new_image_num);
		

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
			int spr_col_x = col_inv_spr_pos_x[c];
			
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
		for (int c=0;c<INVADERS_PER_ROW;c++) {
			if (inv_alive[inv_index+c]) {
				rows_max_spr_y=row_inv_spr_pos_y[r];
				return;
			}//if
		}//for c
	}//for r
	inv_assert(rows_max_spr_y!=0, "bad y:0 at find-rows-max-spr-y");
	// if (rows_max_spr_y == 0 ) {
	// 	vic.color_back=VCOL_RED;
	// }
}


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

	START_BORDER(VCOL_YELLOW);

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
		#ifdef MY_ASSERT
			inv_assert((rows_x_frame_speed = -4), "rows_x_frame_speed=%d bounce_rows",rows_x_frame_speed);
		#endif

		ok = move_rows_down(Y_INC);
		cols_x_shift -= rows_x_frame_speed-1; //X_INC*2;
		rows_x_frame_speed *= -1;
	}
	return ok;
	END_BORDER();
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
	for (int r=0;r<NUM_ROWS;r++) {       

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
 * 		With more than one (2-3, depending) keys down at once.
 * 
 * 		Right now, the keyboard controls are immutable, and are:
 * 		A  D	= left,right
 * 		RETURN	= fire
 **/
void handle_inputs(char joy_num) {

	static bool b_pressed = false;

	keyb_poll();
	joy_poll(joy_num);

			// printf("%c", keyb_codes[keyb_key & 0b01111111]);

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
	
	if ((keyb_key == 0) && (joyb[joy_num]==0) && (joyx[joy_num]==0) && (joyy[joy_num]==0) ){
	    return;
	}

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
		return;
	}

	if (key_pressed(KSCAN_B) && (b_pressed == false) && (num_bombs < MAX_BOMBS)) {
		b_pressed = true;
		//Find a column with some live Invaders in it
		for (byte col=0;col<INVADERS_PER_ROW;col++) {
			if (col_invs_left_alive[col]>0) {
				//Then find a row where the bottom-lost live Invader isn't so low that we can't draw the bomb.
				//	Note that this might mean that we won't drop a bomb at all.
				for (int row=NUM_ROWS-1;row>-1;row++) {
					int inv_index=row_inv_index[row] + col;
					if ((inv_alive[inv_index]) && (row_inv_spr_pos_y[row]<245)) {
						//TODO replace these magic numbers (4)
						add_bomb(col_inv_spr_pos_x[col] + 4,
							row_inv_spr_pos_y[row] + 4);
						goto break_both_loops;
					}//if inv_alive
				}//for r
			} //if col_invs_left_alive
		}//for col

//It's not always harmful when carefully considered. So there.
break_both_loops:
		__asm {
			nop
		}
		// do {
		// 	col=rand() % INVADERS_PER_ROW;
		// } while ((col_invs_left_alive[col]==0);

	} else {			//if B
		b_pressed = false;
	}
}

//MAIN thread
void fire_bullet(byte obj_num) {
	if (obj_type[obj_num] == TYPE_BULLET) {
		sidfx_play(0, PlayerFireFX, 1);

		obj_x[BULLET_OBJ_NUM] = obj_x[SHIP_OBJ_NUM];

		obj_y[BULLET_OBJ_NUM] = obj_y[SHIP_OBJ_NUM];
		obj_speed_x[BULLET_OBJ_NUM] = 0;

		obj_speed_y[BULLET_OBJ_NUM] = -1;

		obj_sprite_num[BULLET_OBJ_NUM] = 1;
		obj_image_handle[BULLET_OBJ_NUM] = BULLET_IMAGE_NUM;

		obj_alive[BULLET_OBJ_NUM] = true;

		//enable showing the bullet in all rows
		for (int r=0;r<NUM_ROWS;r++) {
			row_sprite_enable_mask[r] |=2;
		}		
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
			else {
				__asm {
					nop
				}
			}
		} else {
			if (new_y > MIN_SPR_Y) {
				new_y += obj_speed_y[obj_num];
			}
			else {
				__asm {
					nop
				}
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
			vic.color_back = VCOL_LT_BLUE;
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

	inv_assert(sprite_num<8,"bad spr_num:%d in draw-object", sprite_num);

	spr_show(sprite_num, obj_alive[obj_num]);

	if (obj_alive[obj_num]) {
		
		inv_assert(obj_num<2 && obj_x[obj_num]<321 && obj_y[obj_num]<255, 
			"bad obj_num:%d x:%d y:%d in draw-object",obj_num,obj_x[obj_num], obj_y[obj_num]);
		spr_move(sprite_num, obj_x[obj_num], obj_y[obj_num]);

		spr_color(sprite_num, obj_sprite_color[obj_num]);
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
		spr_show(sprite_num,0);
		//vic.spr_enable &= (255 - pow2[obj_num]);
	}

	//__asm{ sei }
}

//MAIN thread
void kill_bullet(byte obj_num) {
	obj_alive[obj_num] = false;
	int spr_num=obj_sprite_num[obj_num];

	inv_assert(spr_num==1,"bad spr_num:%d in kill-bullet",spr_num);
	// if (spr_num!=1) {
	// 	vic.color_back=VCOL_RED;
	// 	while(true);
	// }
	//disable showing the bullet in all rows
	for (int r=0;r<NUM_ROWS;r++) {
		//TODO fix this for the generic case
		//row_sprite_enable_mask[r] &= (0b11111111 - 1 << spr_num);
		row_sprite_enable_mask[r] &= (0b11111101);
	}
	spr_show(spr_num, false);
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

	//TODO implement raster splits at 0 and 8 to switch in/out of text mode
	for (int i=0;i<NUM_ROWS;i++) {
		raster_irq_line[i] = INV_MIN_Y+SCANLINES_PER_ROW*i-SCANLINES_TO_BUILD_SPRITE;
		// raster_irq_line[i] = MIN_Y+SCANLINES_PER_ROW*i-SCANLINES_TO_BUILD_SPRITE;
	}
	//TODO cheating
	raster_irq_line[NUM_ROWS] = 230;

	for (byte r=0;r<NUM_ROWS; r++) {
		// inv_assert(r < NUM_ROWS, "r is broken");
		row_y[r]                = INV_MIN_Y + SCANLINES_PER_ROW * r; //SCANLINES_PER_ROW * r;
		row_num_images[r]       = 2;
		row_image_handles[r][0] = INVADER_IMAGE_BASE +(r*2);
		row_image_handles[r][1] = INVADER_IMAGE_BASE +(r*2) + 1;
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
		
		for (int c=0;c<INVADERS_PER_ROW; c++) {
			byte inv_index			= row_inv_index[r]+c;
			inv_alive[inv_index]	= true;
			inv_sprite_num[inv_index] = 2 + c;
			// inv_spr_x[inv_index]	= 0;
			// inv_spr_x[inv_index]	= 0;
		}
	}

	/** NEW VERSION
	//TODO implement raster splits at 0 and 8 to switch in/out of text mode
	for (int i=0;i<NUM_ROWS;i++) {
		raster_irq_line[i] = row_y[i] - SCANLINES_TO_BUILD_SPRITE;
			//MIN_Y+(SCANLINES_PER_ROW*i)-SCANLINES_TO_BUILD_SPRITE;
		// raster_irq_line[i] = MIN_Y+SCANLINES_PER_ROW*i-SCANLINES_TO_BUILD_SPRITE;
	}
	//TODO cheating
	raster_irq_line[NUM_ROWS] = 230;
	*/

	for (int c=0;c<INVADERS_PER_ROW;c++) {
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
	

	//Init bombs
	for (byte i=0;i<MAX_BOMBS;i++) {
		bomb_alive[i]=false;
		bomb_x[i]=0;
		bomb_y[i]=0;
		bomb_color[i]=VCOL_BLACK;
		bomb_y_speed[i]=BOMB_Y_SPEED;		
	}
	num_bombs=0;

	__asm {
		nop
	}
}

void init_sprites() {
	//spr_init((char *)logo_screen);

	vic.spr_mcolor0 = 1;    //TODO change this raw #
	vic.spr_mcolor0 = 2;    //TODO change this raw #

	#pragma unroll(full)
	for (int ic=0;ic<NUM_ROWS;ic++) {
		byte spr_num=ic+2;

		inv_assert(spr_num<8, "spr_num=%d at init-sprites()",spr_num);

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
	sidfx_play(0, PlayerDieFX, 3);
	spr_image(0, SMOOSHED_SHIP_IMAGE_NUM);

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

	//NOTE: can't unroll this loop, since the bounds are not constant
	//#pragma unroll(full)
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

/*
	CLears the multicolor screen, and draws num_stars 
*/
void init_screen_mc(byte num_stars) {

	/*
	%00 for background color 0 ($d021)
	%01 for the upper nibble of the screen matrix
		NOTE: %01 is transparent for sprite collisions
	%10 for lower nibble of the screen matrix
	%11 for the lower nibble of color RAM
	*/
    memset(logo_color, 0x12, 1000);
	memset((byte *)0xd800, 0x03, 1000);
	memset(logo_bmp, 0x00, 8000);

	for (byte i=0;i<num_stars;i++) {
		unsigned int x=rand() % 320;
		unsigned int y=rand() % 200;

		bmmc_put(&bitmap, x,y,1);

		int char_num=((y-50)/8)*40+((x-24)/8);
		logo_color[char_num]=(rand() % 16) <<4 ;	//color in upper nibble 
	}
}
void clear_hires_screen() {
		//clear out the hires stuff
	memset(logo_bmp, 0, 8000);
	memset(logo_screen, 0, 1000);
	memset(logo_color, 0, 1000);
}

void clear_text_screen() {
	//clear out the text stuff
	//FIXME remove magic
	memset((void *)0x400, 0x20, 1000);
	memset((void *)0xd800, 0, 1000);    
}

/**
 * Move the active bombs. We want to erase all bombs here, since one of them 
 * 	might have been killed since move_bombs() last ran. Then we can move
 * 	only the living ones. Note that we want to use color %10 or %11 to draw the 
 * 	bullet, since %01 is transparent as far as sprite-background collisions
 * 	go. If we want to change the color of the bombs, we need to change the
 * 	contents of the logo_color cells.
 * @see init_screen_mc
 */
void move_bombs() {
	if (num_bombs == 0) {
		return;
	}

	//#pragma unroll(full)
	for (byte i=0;i<MAX_BOMBS; i++) {
		// if (bomb_alive[i]) {
		// 	bomb_countdown[i]--;
		// }
		// if (bomb_alive[i] && 
		// 	(bomb_countdown[i]-- != 0)) {
		// 	continue;
		// }

		if (! bomb_alive[i]) {
			continue;
		}
		int *x_ptr=&bomb_x[i];
		int *y_ptr=&bomb_y[i];
		int x=*x_ptr;
		int y=*y_ptr;

		if ((x<0) || (x>320) || (y<0) || (y>200)) {
			// inv_assert(x>0&&x<320&&y>0&&y<200, "bad x:%d y:%d in move-bombs",x,y);
			kill_bomb(i);
			continue;
		}
		bmmc_put(&bitmap, x, y, 0); //erase
		if ((bomb_countdown[i]--) == 0) {
			bomb_countdown[i]=MAX_BOMB_COUNTDOWN;
			*y_ptr += bomb_y_speed[i];
			// if (*y_ptr > 200) {	//TODO replace with constant
			// 	kill_bomb(i);
			// 	continue;
			// }
		}
		bmmc_put(&bitmap, x, *y_ptr, 1); //draw?
	}

}
/**	Find the next bomb slot that isn't already alive, and bring it to life. 
 * @param x	x-coordinate in SPRITE format
 * @param y	y-coordinate in SPRITE format
 * @returns true if a new bomb was dropped, false if we're already maxxed out
 */
bool add_bomb(int x, int y) {
	if (num_bombs < (MAX_BOMBS-1)) {
		for (byte i=0;i<MAX_BOMBS; i++) {
			if (! bomb_alive[i]) {
				bomb_x[i]		=x-24;	//convert sprite-hires
				bomb_y[i]		=y-50;	//convert sprite-hires
				bomb_alive[i]	=true;
				bomb_y_speed[i]	= BOMB_Y_SPEED;
				bomb_countdown[i]= MAX_BOMB_COUNTDOWN;
				num_bombs++;
				return true;
			} //if alive
		}//for i
	}//if < MAX_BOMBS
	return false;
}

void kill_bomb(byte bomb_num) {
	bomb_alive[bomb_num]=false;
	//Just in case we're trying to go below 0 somwehere
	inv_assert(num_bombs<MAX_BOMBS, "bad num_bombs:%d in kill-bomb", num_bombs);
	num_bombs--;
	byte y=bomb_y[bomb_num];
	//The bombs only move vertically & down, so the only way one could
	//	be offscreen is if it fell off of the bottom
	if (y<200) {
		bmmc_put(&bitmap, bomb_x[bomb_num],y, 0); //erase
	}
}

// Initialize the music playback for the given sub tune
void music_init(char tune) {
	// Call the function entry using inline assembler
	__asm {
		lda		tune
		jsr		$a000
	}
}

// Iterate the music playback routine, must be called onece
// per video frame
void music_play(void) {
	// Call the function entry using inline assembler
	__asm {
		jsr		$a003
	}
}