== README ==
This current build is only testing with the PONG2 ROM.

I will be allowing the user to choose their own ROM in the future once I get each of these tested and working.

You currently NEED the PONG2 ROM to be in the ROMS folder.

The working executable for the current version is the file "myChip8". You can run it with 

./myChip8

== COMPILATION ==

This can be compiled with... 

gcc `pkg-config --cflags gtk+-3.0` asmInstruction.c `pkg-config --libs gtk+-3.0`

You will need GTK installed to compile this program.

This is only tested to work on Ubuntu as of now.
