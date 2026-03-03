NOTES

- Let's say we want to convert our home-grown IRQ system to DMW's. 
    Each row could be 1 IRQ, plus a couple for split-screen and 1 for ship/bullet
    For each row, we do the following (in draw_sprite_row):
    1. vic.spr_enable &= row_sprite_enable_mask[spr_row];
        * We could pre-calculate (vic.spr_enable & row_sprite_enable_mask[spr_row]) in main thread, and just write it during IRQ
            `row_irq.rirq_write(row_num, vic.spr_enable, row_enable_value[row_num])`
    2. If ! row-alive exit
        * Probably not worth the cost
    3. Y positions:
        * `vic.spr_pos[2].y=row_y[2]`
        * `vic.spr_pos[3].y=row_y[3]`
        * `vic.spr_pos[4].y=row_y[4]`
        * `vic.spr_pos[5].y=row_y[5]`
        * `vic.spr_pos[6].y=row_y[6]`
        * `vic.spr_pos[7].y=row_y[7]`
        * Now here and for the `spr_image` ones, we have a problem: not enough slots. Each rirq can only "hold" 5 operations, and we need like 15-20 writes per IRQ. I guess we'll have to start even further up onscreen, so that each sprite gets its own IRQ. But then we quickly run over the 16-IRQ limit. I think we could override it, but that's going to be expensive in CPU time.
    4. vic.spr_mcolor0 = row_mcolor0[spr_row];
    5. vic.spr_mcolor1 = row_mcolor1[spr_row];
    6. Update sprite images.
        (We could calculate new_handle for each sprite in the main thread)
        `spr_image(2, row_image_handles[2][row_image_num[2]])`
        `spr_image(3, row_image_handles[3][row_image_num[3]])`
        `spr_image(4, row_image_handles[4][row_image_num[4]])`
        `spr_image(5, row_image_handles[5][row_image_num[5]])`
        `spr_image(6, row_image_handles[6][row_image_num[6]])`
    7. We can totally override the max # of IRQ's. The good Dr does it, so we can too! Not sure how far it can go, though.


-----
COLLISION DETECTION
-----

$d01e cannot be written to, and is reset upon reading

1. sei, to temporarily disable interrupts

1. X store the address of my own interrupt routine in $0314 and $0315

1. X Disable timer interrupts on the CIA

1. store the value 3 in d01a (vic.intr_enable), to enable raster and sprite to data collision interrupts.

1. store the value $FF in D012 to get interrupts on raster line FF.

1. store the value 0 in D01F, as a test, to make sure that that sprite to background register is cleared. I read somewhere that this is needed, but I didn't see a difference.
1. TO ADD: read from $d01e

1. cli to re-enable interrupts

Then in my own interrupt routine:

1.test D019 for bit 2 set (by ANDing with 00000010). if that results in a 0, jump to the raster routine, which updates the sprites (ball and paddle)

1.if that doesn't result in a 0, I call the routine that handles sprite to datacollision. For now this increments the border colour and saves the value 0 in the $D01F (again, that last one as a test)

1.In both cases I store the $FF in $d019 (vic.intr_ctrl) to acknowledge the interrupt and be able to receive the next.

"For the MBC and MMC interrupts, only the first collision will trigger an
interrupt (i.e. if the collision registers $d01e resp. $d01f contained the
value zero before the collision). To trigger further interrupts after a
collision, the concerning register has to be cleared first by reading from
it."

"It's a double whammy. You need to write to $d019 (vic.intr_ctrl) to reset the interrupt condition, AND you read from $D01E/$D01F to reset the sprite collision."

To clear a latched bit in $d01a (vic.intr_enable), write a 1 to it

Only the first collision will trigger an
interrupt (i.e. if the collision registers $d01e or $d01f contained the
value zero before the collision). To trigger further interrupts after a
collision, the respective register has to be cleared first by reading from
it.

The bit 7 in the latch $d019 (vic.intr_ctrl) reflects the inverted state of the IRQ output
of the VIC.

----
MISC
----
- 1 clock cycles is 8 pixels
- 64 cycles/line
- 

----
----
vic.intr_ctrl
There are four interrupt sources in the VIC. Each source has a corresponding
bit in the interrupt latch (register $d019 (vic.intr_ctrl)) and a bit in the interrupt
enable register ($d01a (vic.intr_enable)). When an interrupts occurs, the corresponding bit in
the latch is set. To clear it, the processor has to write a "1" there "by
hand". The VIC doesn't clear the latch on its own.

If at least one latch bit and the corresponding bit in the enable register
is set, the VIC holds the IRQ line low and thereby triggers an interrupt in
the processor. So the four interrupt sources can be independently enabled
and disabled using the enable bits. Since the VIC, as mentioned, doesn't
clear the interrupt latch by itself, the processor has to do this before it
resets the I flag or returns from the interrupt routine. Otherwise the
interrupt will be re-triggered immediately (the IRQ input of the 6510 is
state-sensitive).

The following table describes the four interrupt sources and their bits in
the latch and enable registers:

 Bit|Name| Trigger condition
 ---+----+-----------------------------------------------------------------
  0 | RST| Reaching a certain raster line. The line is specified by writing
    |    | to register $d012 and bit 7 of $d011, and internally stored by
    |    | the VIC for the raster compare. The test for reaching the
    |    | interrupt raster line is done in cycle 1 of every line (for line
    |    | 0, in cycle 2). It is possible to trigger an interrupt
    |    | immediately by writing to $d011/$d012, but the interrupt can
    |    | never occur more than once per raster line.
  1 | MBC| Collision of at least one sprite with the text/bitmap graphics
    |    | (one sprite data sequencer outputs non-transparent pixel at the
    |    | same time at which the graphics data sequencer outputs a
    |    | foreground pixel)
  2 | MMC| Collision of two or more sprites (two sprite data sequencers
    |    | output a non-transparent pixel at the same time)
  3 | LP | Negative edge on the LP input (lightpen)

For the MBC and MMC interrupts, only the first collision will trigger an
interrupt (i.e. if the collision registers $d01e or $d01f contained the
value zero before the collision). To trigger further interrupts after a
collision, the respective register has to be cleared first by reading from
it.

The bit 7 in the latch $d019 (vic.intr_ctrl) reflects the inverted state of the IRQ output
of the VIC.


__BORDER BANDS__
| COLOR      |  SECTION            |
|---         |---                  |
| LT_GREY    | bullet handling     |
| GREEN      | handle raster irq   |
| WHITE      | handle inputs       |
| PURPLE     | SID handling        |
| LT_GREEN   | move Invaders       |
| ORANGE     | flip_row_image()    |
| LT_BLUE    | default             |

