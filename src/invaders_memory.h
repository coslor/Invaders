#pragma region( lower, 0xa00, 0x0fff, , , {code} )


#pragma section(music_sec, 0)
#pragma region(music_reg, 0x1000, 0x1fff,,, {music_sec})

// #pragma section( spriteset_sec, 0)
// #pragma region( spriteset_reg, 0x2000, 0x2fff, , , {spriteset_sec} )

#pragma section( middle_sec, 0)
#pragma region(middle_reg, 0x2000, 0x3fff,,, {code})


// #pragma section( upper, 0)
// #pragma region(upper, 0x5000, 0x57ff,,, {code, data, bss})

#pragma section(logo_screen_sec, 0)
#pragma section(logo_color_sec, 0)
#pragma section(logo_bmp_sec, 0)

#pragma region(logo_bmp_reg, 0x4000, 0x5fff,,,{logo_bmp_sec})
#pragma region(logo_screen_reg, 0x6000, 0x63ff,,,{logo_screen_sec})
#pragma region(logo_color_reg, 0x6400, 0x6fff,,,{logo_color_sec})

#pragma section( spriteset_sec, 0)
#pragma region( spriteset_reg, 0x7000, 0x77ff,,, {spriteset_sec} )


#pragma region( main, 0x7800, 0x9fff, , , {heap, stack, code, data, bss} )


#define LOGO_FILE "resources/space_invaders_logo.kla"

#pragma data(spriteset_sec)

////
//  NOTE: anything like this, where its data that needs to be there, but the 
//      var itself isn't referenced anywhere, needs to be called out 
//      with __export or #pragma reference(name), or it will be optimized away!
////
const char const spriteset[] =  {
	#embed spd_sprites "resources/invaders-2600.spd"

};
#pragma reference(spriteset)

#pragma data(logo_bmp_sec)
__export const char const logo_bmp[] = {
	#embed 8000 2 LOGO_FILE  
};

//#section
#pragma data(logo_screen_sec)
__export static char logo_screen[1000] = {
	#embed 1000 9002 LOGO_FILE
};
//#endsection

#pragma data(logo_color_sec)
//load the text & color screens into
__export static char logo_color[1000] = {
	#embed 1000 8002 LOGO_FILE
};

// Set the data segment to be the newly created music section,
// so all initialized data gets placed there by the linker
#pragma data(music_sec)

// Make sure that the music data is not dropped by the linker for
// not being referenced with the "__export" storage qualifier.
// We also drop the first 0x7e bytes of the SID file, they contain
// only file level meta data
__export const char music[] = {
//	#embed 0x2000 0x7e "resources/gtu_a000_1x.sid" 
	#embed 0x2000 0x7e "resources/Space_Invaders_Medley.sid" 

};