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
        //
        //This code is from the C64 hard start code in ROM
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
    }
        
#endif
}

