#pragma region( lower, 0xa00, 0x1fff, , , {code, data, bss} )

#pragma section( spriteset_sec, 0)
#pragma region( spriteset_reg, 0x2000, 0x2fff, , , {spriteset_sec} )

#pragma section( middle, 0)
#pragma region(middle, 0x3000, 0x4fff,,, {code, data, bss})

// #pragma section(hires_screen, 0)
// #pragma region(hires_screen, 0x4000,0x43ff,,, {hires_screen})

// #pragma section(hires_color, 0)
// #pragma region(hires_color, 0x4400, 0x4fff,,, {hires_color})


#pragma section( upper, 0)
#pragma region(upper, 0x5000, 0x57ff,,, {code, data, bss})

#pragma section(logo_screen_sec, 0)
#pragma section(logo_color_sec, 0)
#pragma section(logo_bmp_sec, 0)

#pragma region(logo_screen_reg, 0x5800, 0x5bff,,,{logo_screen_sec})
#pragma region(logo_color_reg, 0x5c00, 0x5fff,,,{logo_color_sec})
#pragma region(logo_bmp_reg, 0x6000, 0x7f40,,,{logo_bmp_sec})


#pragma region( main, 0x7f41, 0xa000, , , {code, data, bss, heap, stack} )
