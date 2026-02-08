#pragma region( lower, 0xa00, 0x1fff, , , {code, data, bss} )

#pragma section( spriteset_sec, 0)
#pragma region( spriteset_reg, 0x4000, 0x4800, , , {spriteset_sec} )

#pragma section( middle, 0)
#pragma region(middle, 0x3000, 0x3fff,,, {code, data, bss})

// #pragma section(hires_screen, 0)
// #pragma region(hires_screen, 0x4000,0x43ff,,, {hires_screen})

// #pragma section(hires_color, 0)
// #pragma region(hires_color, 0x4400, 0x4fff,,, {hires_color})

#pragma section( upper, 0)
#pragma region(upper, 0x4400, 0x4fff,,, {code, data, bss})

#pragma section(logo_sec, 0)
//hires:$2000 + bank 1($4000) = $6000
#pragma region(logo_screen, 0x5000, 0x5400,,,{logo_sec})
#pragma region(logo_color, 0x5400, 0x5800,,,{logo_sec})
#pragma region(logo_bmp, 0x6000, 0x7f40,,,{logo_sec})


#pragma region( main, 0x7f41, 0xa000, , , {code, data, bss, heap, stack} )
