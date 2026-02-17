#include "inv_assert.h"
#include <c64/memmap.h>
#include <c64/vic.h>
#include <stdarg.h>

char msg_buffer[80];

//void screen_print(char *message, byte len);

/*
*   Evaluates a boolean value & if false, it resets the C64 so that, 
*       no matter what state it's currently in (hires, funky charset, etc.),
*       the message is printed legibly. It then waits for the user to press
*       a key, and does a soft reset of the machine.
*
*   condition   - if false, then print message & reset
*   message     - a standard printf() message, so all variable substitutions, etc. 
*/
#pragma optimize(0)
void inv_assert(bool condition, void* message, ...) {
#ifdef MY_ASSERT

    char buffer[80];
    va_list argptr;

    if (! condition) {
        //NOTE: this block of code CANNOT be made into a function, as the
        //      reset process *resets the stack pointer*. Therefore, the 
        //      function has nowhere to return to, and goes off into la-la land.
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
        //TODO bypass the part of $ff5b that prints the "COMMODORE" message
        JSR $FF5B	//initialise VIC and screen editor
        }

        __asm {
        cli

        }


        iocharmap(IOCHM_PETSCII_2);

        //print our message, variable substitutions included 
        va_start(argptr, message);
        vsprintf(buffer, message, argptr);
        va_end(argptr);            
        printf(buffer);
        printf("\nPress any key\n");

        do {
            keyb_poll();
            vic_waitFrame();
        } while (keyb_key == 0);

        iocharmap(IOCHM_PETSCII_1);
        __asm {
            JMP ($A000)
        }
        //         mmap_set(MMAP_ROM);
        // vic_setmode(VICM_TEXT, (char *)0x0400, (char *)0x1000);
        // //memset((char *)0x400,32,1000);
        // //memset((char *)0xc800,1,1000);

        // //restore_registers();
        
        // inv_soft_reset();

        // // __asm {
        // //     jsr $FF81         //reset the screen
        // //     jsr $fda3       //IOINIT: initialise SID, CIA and IRQ
        // //     jsr $fd15       //RESTOR: restore default I/O vectors
        // //     jsr $ff5b       //CINT: jumps to CINV/iIRQ/vector in $314 ($ea31) / initialise VIC and screen editor/ set up PAL/NTSC

        // // }
        // iocharmap(IOCHM_PETSCII_1);

        // //screen_print((char *)s"Hello World", strlen("Hello World"));

        // screen_print(message, strlen(message));
        // while(true) {
        //     vic_waitFrame();

        // //     __asm {
        // //         nop
        // //     }
        // };

    }
        
#endif
}

// void inv_reset_text_screen() {
// }



// void inv_soft_reset() {
//     __asm {
//         //from https://www.c64-wiki.com/wiki/Reset_(Process)
//         SEI             // set interrupt disable
//         LDX #$FF        // 
//         TXS             // empty the stack

//         // CLD             // clear decimal flag
//         STX $D016       // sets bit 5 (MCM) off, bit 3 (38 cols) off
//         //JSR $FDA3       // initialise I/O
//         lda #$7f
//         sta $dc0d
//         sta $dd0d
//         sta $dd00
//         lda #8
//         sta $dc0e
//         sta $dd0e
//         sta $dc0f
//         ldx #0
//         stx $dc03
//         stx $dd03
//         stx $d418
//         dex
//         stx $dc02
//         lda #7
//         sta $dd00
//         lda #$3f
//         sta $dd02

//         //set up 6510 IO lines
//         lda #$e7
//         sta 1
//         lda #$2f
//         sta 0

//         lda $02a6
//         beq ntsc_setup

//     pal_setup:
//         lda #$25
//         sta $dc04
//         lda #$40
//         jmp finish_pal_ntsc

//     ntsc_setup:
//         lda #$95
//         sta $dc04
//         lda #$42
//         sta $dc05

//     finish_pal_ntsc:
//         //JSR $FD50       // initialise memory
//         JSR $FD15       // set I/O vectors ($0314..$0333) to kernal defaults
//         JSR $FF5B       // more initialising... mostly set system IRQ to correct value and start
//         CLI             // clear interrupt flag
//     }
// }

// //from https://sta.c64.org/cbm64scrtopet.html
// void screen_print(char *str, byte len){
//     for (int i=0;i<len;i++) {
//         byte c = str[i];
//         if ((c >= 64 && c <= 95) || (c >= 160 && (c <= 191) )) {
//             c -=64;
//         }
    
//         else if ((c >= 192 && c <= 221) || (c == 223)) {
//             c -= 128;
//         }
//         else if (c <= 63) {
//             c += 128;
//         }
//         else if (c >= 128 && c <= 191) {
//             c += 128;
//         }
    
//         *(char *)(0x400 + i)=c;
//     }
// }