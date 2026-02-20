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
    {   0x14,   0x0d,   0x1d,   0x88,   0x85,   0x86,   0X87,   0X11 },
    {   '3',    'W',    'A',    '4',    'Z',    'S',    'E',    0xff }, //0xff indicates a non-char key
    {   '5',    'R',    'D',    '6',    'C',    'F',    'T',    'X'  },
    {   '7',    'Y',    'G',    '8',    'B',    'H',    'U',    'V'  },
    {   '9',    'I',    'J',    '0',    'M',    'K',    'O',    'N'  },
    {   '+',    'p',    'l',    '-',    '.',    ':',    '@',    ','  },
    {   0x5c,   '*',    ';',    0x13,   0xff,   '=',    '^',    '/'  },
    {   '1',    0x5f,   0xff,   '2',    ' ',    0xff,   'q',    0x03 }
};

byte get_kr_keyb_matrix(byte row, byte col) {
    if (row<8 && col < 8) {
        return kr_keyb_matrix[row][col];
    }
    return 0;
}

int kr_log2(byte val) {
    for (int i=0;i<8;i++)  {
        if (val == pow2[i]) {
            return i;
        }
    }
    return 0;
    //return log(i)/log(2);
}

/* Returns character currently being pressed, or 0 if none*/
#pragma optimize(0)
char kr_read_key() {
    
    byte a,x,y;
    byte row2, key2;
    byte col2 = 0xff;

    cia1.ddrb = 0;
    cia1.ddra = 0xff;

    cia1.pra = 0;
    byte col_bit = ~cia1.prb;
    if (col_bit == 0x00) {
        return 0;       //no key
    }

    //col_bit = ~col_bit;
    //we have a column

    col2 = kr_log2(col_bit);

    if (col2 == 0xff) {
        //printf("\nERROR!\n");
        return 0;
    }
    a = 0x7f;

    for (row2=8;row2>0;row2--) {
        cia1.pra = a;
        a = a >> 1 + 0b10000000;
        byte prb = cia1.prb;
        if ( prb != 0xff) {
            break;
        }
    }
    
    if (row2 == 0) {
        return 0;
    }
    //We want 0-7. not 1-8
    //row2--;

    if (row2>8 || col2>7 ) {
        return 0;
    }
    int key4 = (get_kr_keyb_matrix(row2-1,col2)); // & 0x7f;
    if (key4 > 0xf0) {
        return 0;
    }
    return key4;

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

