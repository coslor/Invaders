#include <stdio.h>
#include <c64/types.h>
#include <stdbool.h>
#include <conio.h>
#include <c64/vic.h>

/**
 *  We want to be able to show the # of raster lines that parts of the program use. The simplest way to
 *      do this on the C64 is to change the border color while something is running. However, with
 *      interrupts and tasks within tasks, it's not quite as simple. stacking_borders uses a simple stack
 *      to allow for up to 16 levels of nested borders. 
 *      
 */
static const byte MAX_STACK_SIZE=16;

static VICColors border_stack[MAX_STACK_SIZE];
static byte stack_ptr=0;

/**
 * @returns false if the stack was full
 */
bool push(VICColors color) {
    if (stack_ptr<MAX_STACK_SIZE) {
        border_stack[stack_ptr++]=color;
        return true;
    }
    return false;
}

/**
 * @returns true if the stack wasn't empty, and sets value to the color at the top; false if the stack is empty
 */
bool pop(VICColors *value) {
    if (stack_ptr>0) {
        *value=border_stack[--stack_ptr];
        return true;
    }
    return false;
}

bool is_stack_empty() {
    return (stack_ptr==0);
}

void empty_stack() {
    stack_ptr==0;
}

/**
 *  @param color    the color you want to change the border to; don't use VCOL_RED as that indicates a stack error
 * */
inline void START_BORDER(VICColors color) {
#ifdef USE_BORDERS
    if (push(vic.color_border)) {
        vic.color_border=color;
    }
    else {
        vic.color_border=VCOL_RED;
    }
#endif
};

/**
 * @returns true if the color can be popped. It returns false if not and additionally sets the border to VCOL_RED
 * */
inline void END_BORDER(){
#ifdef USE_BORDERS
    VICColors color;
    if (pop(&color)) {
        vic.color_border=color;
    }
    else {
        vic.color_border=VCOL_RED;
    }
#endif
};

inline void wait_lines(byte min_lines) {
    byte old_raster=vic.raster;
    while ((vic.raster-old_raster)<50) {
        __asm {
            nop //to keep from being optimized away
        }
    }
}

#ifdef RUN_STACKING_BORDERS
int main() {
    iocharmap(IOCHM_PETSCII_1);

    printf("Pushing:");
    for (int i=0;i<6;i++) {
        byte r = rand() % 255 + 1;

        printf("%d ",r );
        if (! push(r)) {
            printf("ERR pushing %d ", r);
        }
    }
    printf("\n");

    VICColors ret_val;
    byte  rtn_count=0;

    printf("Popping:");
    while (pop(&ret_val)) {
        printf("%d ", ret_val);
        rtn_count++;
    }
    printf("\n");
    if (rtn_count != 6) {
        printf("ERROR! rtn_count==%d\n", rtn_count);
        return 1;
    } 
    if (pop(&ret_val)) {    //should fail
        printf("ERROR! popped an empty stack!\n");
        return 1;
    }
    if (push(VCOL_GREEN)) {
        if (pop(&ret_val)) {
            if (ret_val==VCOL_GREEN) {
                printf("SUCCESS!\n");
            }
        } else { 
            printf("ERROR popping!\n");
            return 1;
        }
    } else {
        printf("ERROR pushing!\n");
        return 1;
    }

    while (true) {
        vic_waitTop();
        wait_lines(20);

        START_BORDER(VCOL_GREEN);
        wait_lines(20);

        START_BORDER(VCOL_BLACK);
        wait_lines(20);

        END_BORDER();   //black

        wait_lines(20);
        END_BORDER();   //green

        wait_lines(30);
    }

}
#endif
