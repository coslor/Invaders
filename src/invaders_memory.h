#pragma region( lower, 0xa00, 0x1fff, , , {code, data, bss} )

// #pragma section( spriteset_sec, 0)
// #pragma region( spriteset_reg, 0x2000, 0x2fff, , , {spriteset_sec} )

#pragma section( middle, 0)
#pragma region(middle, 0x3000, 0x4131,,, {code, data, bss})

//so...if we make the middle region above any less than $4131, the logo
//  will *not* display. I have no idea why. I want to put my text
//  screen at $4000, but no dice, I guess.

#pragma section( spriteset_sec, 0)
#pragma region( spriteset_reg, 0x4200, 0x4fff,,, {spriteset_sec} )

#pragma section( upper, 0)
#pragma region(upper, 0x5000, 0x57ff,,, {code, data, bss})

#pragma section(logo_screen_sec, 0)
#pragma section(logo_color_sec, 0)
#pragma section(logo_bmp_sec, 0)

#pragma region(logo_screen_reg, 0x5800, 0x5bff,,,{logo_screen_sec})
#pragma region(logo_color_reg, 0x5c00, 0x5fff,,,{logo_color_sec})
#pragma region(logo_bmp_reg, 0x6000, 0x7f40,,,{logo_bmp_sec})


#pragma region( main, 0x7f41, 0xa000, , , {code, data, bss, heap, stack} )
