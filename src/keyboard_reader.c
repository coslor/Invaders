#include <c64/cia.h>

/*
    from https://codebase.c64.org/doku.php?id=base:reading_the_keyboard
*/

static byte columntab[256];
static bool columntab_is_initialized = false;

//SEE https://retrocomputing.stackexchange.com/questions/6421, https://sta.c64.org/cbm64pet.html
//row,col
char kr_keyb_matrix[8][8] = {
    {   0x14,   0x0d,   0x1d,   0x88,   0x85,   0x86,   0X87,   0X99 },
    {   '3',    'w',    'a',    '4',    'z',    's',    'e',    0xff }, //0xff indicates a non-char key
    {   '5',    'r',    'd',    '6',    'c',    'f',    't',    'x'  },
    {   '7',    'y',    'g',    '8',    'b',    'h',    'u',    'v'  },
    {   '9',    'i',    'j',    '0',    'm',    'k',    'o',    'n'  },
    {   '+',    'p',    'l',    '-',    '.',    ':',    '@',    ','  },
    {   0x5c,   '*',    ';',    0x13,   0xff,   '=',    '^',    '/'  },
    {   '1',    0x5f,   0xff,   '2',    ' ',    0xff,   'q',    0x03 }
};

//void init_columntab();

byte kr_read_key() {
    byte a,x,y, row, key;
    int col = -1;

    cia1.ddrb = 0;
    cia1.ddra = 0xff;

    cia1.pra = 0;
    byte col_bit = cia1.prb;
    if (col_bit == 0xff) {
        return 0;       //no key
    }

    col_bit = ~col_bit;
    //we have a column

    for (int i=0;i<8;i++) {
        if (col_bit == pow2[i]) {
            col = i;
            break;
        }
    }
    if (col == -1) {
        //printf("\nERROR!\n");
        return 0;
    }
    a = 0x7f;

    for (row=7;row>=0;row--) {
        cia1.pra = a;
        a = a >> 1 + 0b10000000;
        byte prb = cia1.prb;
        if ( prb != 0xff) {
            break;
        }
    }
    
    if (row == -1) {
        return 0;
    }

    key = kr_keyb_matrix[row][col];
    return key;

}



// void init_columntab() {
//     for (int i=0;i<256;i++) {
//         if (i == (0xff - 0x80)) {
//             columntab[i] = 0x70;
//         } else if (i == (0xff - 0x40)) {
//             columntab[i] = 0x60;
//         } else if (i == (0xff - 0x20)) {
//             columntab[i] = 0x50;
//         } else if (i == (0xff - 0x10)) {
//             columntab[i] = 0x40;
//         } else if (i == (0xff - 0x08)) {
//             columntab[i] = 0x30;
//         } else if (i == (0xff - 0x04)) {
//             columntab[i] = 0x20;
//         } else if (i == (0xff - 0x02)) {
//             columntab[i] = 0x10;
//         } else if (i == (0xff - 0x01)) {
//             columntab[i] = 0;
//         } else {
//             columntab[i] = 0xff;
//         }
//     }
// }
