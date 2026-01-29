#include "my_assert.h"
#include <c64/memmap.h>

void my_assert(bool condition, void* message) {
#ifdef MY_ASSERT
    if (! condition) {
        //TODO reset c64 so that message is visible
        mmap_set(MMAP_ROM);
        __asm {
            jsr $FF81         //reset the screen
            jsr $fda3       //IOINIT: initialise SID, CIA and IRQ
            jsr $fd15       //RESTOR: restore default I/O vectors
            jsr $ff5b       //CINT: jumps to CINV/iIRQ/vector in $314 ($ea31) / initialise VIC and screen editor/ set up PAL/NTSC
        }
        iocharmap(IOCHM_PETSCII_2);

        printf("ASS FAILED:%s\n",(message == NULL ? "" : message));
    exit(-1);
    }
#endif
}