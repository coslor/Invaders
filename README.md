A Space Invaders clone for the C64. It uses sprites for everything except maybe Invader bombs, which will have to be characters or hires dots or something. 

The point of writing this is to:
1) Learn the [**Oscar64**](https://github.com/drmortalwombat/oscar64) optimizing 6502 compiler better. Dr Mortal Wombat, the author, has created a really good, fast C compiler for the 6502, which is generally pretty unsuited to C.
2) Learn sprite multiplexing on the C64 using IRQs. Oscar64 includes sprite-muxing and irq libraries, but I wanted to write my own. I've ended up with 36 stable, flicker-free sprites onscreen at once, which is pretty cool.
3) See if I can actually finish a video game on the C64. For most of my life, I have writen little games on all kinds of platforms to try out some technology or technique. Then, I always, *always* abandon them as soon as the cool part is over. This time, I want to **finish** a game, even a simple & boring one. Maybe, if I can finish this game, I'll be motivated to actually finish some more!
4) Sharpen up my code skills a bit. I've had some health & family issues for a while, and I want to try to get the gears in my head rotating again.
