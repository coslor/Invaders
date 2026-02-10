#include "my_assert.h"
#include <c64/memmap.h>
#include <c64/vic.h>

char vic_data[64];
char cia1_data[16];
char cia2_data[16];
char data_6510[2];

char string[64]=s"ASSERTION:";
int string_len=10;

// void init_my_assert() {
//     save_registers();
// }

// #pragma optimize(0)
// void save_registers() {
//    memcpy(vic_data,  (char *) 0xd000, 64);
//    memcpy(cia1_data, (char *) 0xdc00, 16);
//    memcpy(cia2_data, (char *) 0xdd00, 16);
//    data_6510[0]=*(byte *)0;
//    data_6510[1]=*(byte *)1;
// }

// void restore_registers() {
//    __asm { sei };

//    memcpy((char *) 0xd000,    vic_data,  64);
//    memcpy((char *) 0xdc00,    cia1_data, 16);
//    memcpy((char *) 0xdd00,    cia2_data, 16);
//    *(char *)0 = data_6510[0];
//    *(char *)1 = data_6510[1];

//    __asm { cli };
// }

char test_msg[]="Test message";

void my_assert(bool condition, void* message) {
#ifdef MY_ASSERT
    if (! condition) {
        //TODO reset c64 so that message is visible

        __asm {
            sei
            ldx #$ff
            txs
            cld
            jsr $fd02   //scan for autostart ROM at $8000
            //BNE START1
            //JMP ($8000) //run cart code
            //START1:
            stx $d016
            JSR $FDA3	//initialise SID, CIA and IRQ
            JSR $FD50	//RAM test and find RAM end
            JSR $FD15	//restore default I/O vectors
            JSR $FF5B	//initialise VIC and screen editor
            cli
        }
        iocharmap(IOCHM_PETSCII_2);

        printf(message);
        printf("Press any key\n");

        do {
            keyb_poll();
        } while (keyb_key == 0);

        // // vic_waitFrames(255);
        __asm {
        //     lda #<print_msg
        //     pha
        //     lda #>print_msg
        //     pha
            JMP ($A000)
        //     print_msg:
        //     lda #<test_msg
        //     ldy #>test_msg
        //     jsr $ab1e
        //     rts
        }
        // //while(1);
        //exit(-1);
        
        mmap_set(MMAP_ROM);
        vic_setmode(VICM_TEXT, (char *)0x0400, (char *)0x1000);
        //memset((char *)0x400,32,1000);
        //memset((char *)0xc800,1,1000);

        //restore_registers();
        
        soft_reset();

        // __asm {
        //     jsr $FF81         //reset the screen
        //     jsr $fda3       //IOINIT: initialise SID, CIA and IRQ
        //     jsr $fd15       //RESTOR: restore default I/O vectors
        //     jsr $ff5b       //CINT: jumps to CINV/iIRQ/vector in $314 ($ea31) / initialise VIC and screen editor/ set up PAL/NTSC

        // }
        iocharmap(IOCHM_PETSCII_1);

        //screen_print((char *)s"Hello World", strlen("Hello World"));

        screen_print(message, strlen(message));
        while(true) {
            vic_waitFrame();

        //     __asm {
        //         nop
        //     }
        };
    }
#endif
}

void print_msg() {
    // printf("Hello world");
    exit(-1);
}

void soft_reset() {
    __asm {
        //from https://www.c64-wiki.com/wiki/Reset_(Process)
        SEI             // set interrupt disable
        LDX #$FF        // 
        TXS             // empty the stack

        // CLD             // clear decimal flag
        STX $D016       // sets bit 5 (MCM) off, bit 3 (38 cols) off
        //JSR $FDA3       // initialise I/O
        lda #$7f
        sta $dc0d
        sta $dd0d
        sta $dd00
        lda #8
        sta $dc0e
        sta $dd0e
        sta $dc0f
        ldx #0
        stx $dc03
        stx $dd03
        stx $d418
        dex
        stx $dc02
        lda #7
        sta $dd00
        lda #$3f
        sta $dd02

        //set up 6510 IO lines
        lda #$e7
        sta 1
        lda #$2f
        sta 0

        lda $02a6
        beq ntsc_setup

    pal_setup:
        lda #$25
        sta $dc04
        lda #$40
        jmp finish_pal_ntsc

    ntsc_setup:
        lda #$95
        sta $dc04
        lda #$42
        sta $dc05

    finish_pal_ntsc:
        //JSR $FD50       // initialise memory
        JSR $FD15       // set I/O vectors ($0314..$0333) to kernal defaults
        JSR $FF5B       // more initialising... mostly set system IRQ to correct value and start
        CLI             // clear interrupt flag
    }
}

//from https://sta.c64.org/cbm64scrtopet.html
void screen_print(char *str, byte len){
    for (int i=0;i<len;i++) {
        byte c = str[i];
        if ((c >= 64 && c <= 95) || (c >= 160 && (c <= 191) )) {
            c -=64;
        }
    
        else if ((c >= 192 && c <= 221) || (c == 223)) {
            c -= 128;
        }
        else if (c <= 63) {
            c += 128;
        }
        else if (c >= 128 && c <= 191) {
            c += 128;
        }
    
        *(char *)(0x400 + i)=c;
    }
}