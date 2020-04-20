== README ==
This current build is only testing with the PONG2 ROM.

I will be allowing the user to choose their own ROM in the future once I get each of these tested and working.

This can be compiled with... 

gcc `pkg-config --cflags gtk+-3.0` asmInstruction.c `pkg-config --libs gtk+-3.0`

You currently NEED the ROM to be in the same directory as the program.

You will need GTK installed.
