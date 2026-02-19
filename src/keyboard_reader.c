#include "keyboard_reader.h"
#include <c64/vic.h>

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

byte kr_log2(byte i) {
    return log(i)/log(2);
}

/* Returns character currently being pressed, or 0 if none*/
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

    col = kr_log2(col_bit);

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

#pragma optimize(0)
/* Is the key at the given keyboard matrix address being pressed right now?*/
bool kr_is_key_pressed(byte row, byte col) {
    bool value = false;
    byte col_mask = ~pow2[col];
    byte row_mask = ~pow2[row];

    //__asm { sei}

    cia1.ddrb = 0;
    cia1.ddra = 0xff;

    cia1.pra = row_mask;

    byte cols_found_mask = cia1.prb;

    if (cols_found_mask != 0xff) {
        //byte cols_found=kr_log2(~cols_found_mask);
        //if (cols_found & col) {     
        if(~cols_found_mask & pow2[col]) {  //has to be & and not ==, otherwise
                                            //  it doesn't catch multiple keys at once
            value = true;
        }
    }

    //__asm { cli}

    return value;
}

/* Is the given key being pressed right now? */
bool kr_is_char_pressed(char c) {
    byte row=0xff,col=0xff;

    bool keep_running = true;

    for (int r=0;r<8 && keep_running;r++) {
        for (int c=0;c<8 && keep_running;c++) {
            if (kr_keyb_matrix[r][c] == c) {
                row = r;
                col = c;
                keep_running = false;
            }
        }
    }

    if (row == 0xff) {
        return false;
    }
    else {
        return kr_is_key_pressed(row,col);
    }
}

