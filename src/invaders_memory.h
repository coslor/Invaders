#pragma region( lower, 0xa00, 0x2fff, , , {code, data, bss} )

// #pragma section( spriteset_sec, 0)
// #pragma region( spriteset_reg, 0x2000, 0x2fff, , , {spriteset_sec} )

#pragma section( middle_sec, 0)
#pragma region(middle_reg, 0x3000, 0x3fff,,, {code, data, bss})


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


#pragma region( main, 0x7800, 0xa000, , , {code, data, bss, heap, stack} )

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
