#ifndef ASSEMBLY_H_INCLUDED
#define ASSEMBLY_H_INCLUDED

#define chip8_screen_w 64 // the base internal resolution WIDTH for chip8
#define chip8_screen_h 32 // the base internal resolution HEIGHT for chip8
#define screen_res_mod 12 // the modifier for GTK to increase pixel size on screen

#define HEX00F(hexNum) (hexNum & 0xf) // grabs right-most nibble of a value (nibble s 4 bits)
#define HEX0F0(hexNum) ((hexNum >> 4) & 0xf) // grabs second right-most nibble of a value
#define HEXF00(hexNum) ((hexNum >> 8) & 0xf) // grabs third right-most nibble of a value
#define HEX0FF(hexNum) (hexNum & 0xff) // grabs the second right-most and right-most nibbles of a value
#define HEXFF0(hexNum) ((hexNum >> 4) & 0xff) // grabs the seconf and third right-most nibbles of a value
#define HEXFFF(hexNum) (hexNum & 0xfff) // grabs the three right-most nibbles of a value

#define ROM_NAME "./ROMS/PONG2"

struct asmStuff
{
	unsigned char V[16];
	unsigned short PC;
	unsigned short I;
	unsigned short SP;
	unsigned char memory[4096]; // stack = 0xEA0-0xEFF | display = 0xF00-0xFFF
	unsigned char graphics[chip8_screen_w*chip8_screen_h]; //0x800
	unsigned char bitGraphics[(chip8_screen_w*chip8_screen_h) / 8]; // bitwise graphics
	unsigned char key[16];
	unsigned char delayTimer;
	unsigned char soundTimer;
	unsigned char FX0A_stop; // will be 1 if FX0A asks for all assembly to pause until user input
	unsigned char FX0A_X;// Only used for asmFX0A
	unsigned char FX0A_key[16];
};

// Fetches ROM to insert into memory
void getROM(char *romMemoryStart);

// Sets up values for the asmStuff structure
void asmStuffSetup(struct asmStuff *given);

// 	Calls RCA 1802 program at address NNN. Not necessary for most ROMs. 
void asm0NNN(); // 0NNN

// 	Clears the screen. 
void asm00E0(struct asmStuff *regs); // 00E0

//  Returns from a subroutine. 
void asm00EE(struct asmStuff *regs); // 00EE

// 	Jumps to address NNN. 
void asm1NNN(struct asmStuff *regs, unsigned short NNN); // 1NNN

// 	Calls subroutine at NNN. 
void asm2NNN(struct asmStuff *regs, unsigned short NNN); // 2NNN

// 	3XNN - Skips the next instruction if VX equals NN. 
//  (Usually the next instruction is a jump to skip a code block) 
void asm3XNN(struct asmStuff *regs, unsigned char X, unsigned char NN); // 3XNN

//  4XNN - Skips the next instruction if VX doesn't equal NN. 
//  (Usually the next instruction is a jump to skip a code block) 
void asm4XNN(struct asmStuff *regs, unsigned char X, unsigned char NN); // 4XNN

//  5XY0 - Skips the next instruction if VX equals VY. 
//  (Usually the next instruction is a jump to skip a code block) 
void asm5XY0(struct asmStuff *regs, unsigned char X, unsigned char Y); // 5XY0

//  6XNN - Sets VX to NN. 
void asm6XNN(struct asmStuff *regs, unsigned char X, unsigned char NN); // 6XNN 

// 	7XNN - Adds NN to VX. (Carry flag is not changed) 
void asm7XNN(struct asmStuff *regs, unsigned char X, unsigned char NN); // 7XNN

//  8XY0 - Sets VX to the value of VY. 
void asm8XY0(struct asmStuff *regs, unsigned char X, unsigned char Y); // 8XY0

// 	Sets VX to VX or VY. (Bitwise OR operation) 
void asm8XY1(struct asmStuff *regs, unsigned char X, unsigned char Y); // 8XY1

// 	Sets VX to VX and VY. (Bitwise AND operation) 
void asm8XY2(struct asmStuff *regs, unsigned char X, unsigned char Y); // 8XY2

// 	Sets VX to VX xor VY. 
void asm8XY3(struct asmStuff *regs, unsigned char X, unsigned char Y); // 8XY3

//  8XY4 - Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't. 
void asm8XY4(struct asmStuff *regs, unsigned char X, unsigned char Y); // 8XY4

// 	VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't. 
void asm8XY5(struct asmStuff *regs, unsigned char X, unsigned char Y); // 8XY5

//  Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
void asm8XY6(struct asmStuff *regs, unsigned char X); // 8XY6

//  Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't. 
void asm8XY7(struct asmStuff *regs, unsigned char X, unsigned char Y); // 8XY7

//  Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
void asm8XYE(struct asmStuff *regs, unsigned char X); // 8XYE

// 	9XY0 - Skips the next instruction if VX doesn't equal VY. 
//  (Usually the next instruction is a jump to skip a code block) 
void asm9XY0(struct asmStuff *regs, unsigned char X, unsigned char Y); // 9XY0

//  Sets I to the address NNN. 
void asmANNN(struct asmStuff *regs, unsigned short NNN); // ANNN

//  Jumps to the address NNN plus V0. 
void asmBNNN(struct asmStuff *regs, unsigned short NNN); // BNNN

//  Sets VX to the result of a bitwise and operation on a random number 
//  (Typically: 0 to 255) and NN. 
void asmCXNN(struct asmStuff *regs, unsigned char X, unsigned char NN); // CXNN

//  Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels 
//  and a height of N pixels. Each row of 8 pixels is read as bit-coded 
//  starting from memory location I; I value doesn’t change after the execution 
//  of this instruction. As described above, VF is set to 1 if any screen pixels 
//  are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen
void asmDXYN(struct asmStuff *regs, unsigned char X, unsigned char Y, unsigned char N); // DXYN

//  Skips the next instruction if the key stored in VX is pressed. 
//  (Usually the next instruction is a jump to skip a code block) 
void asmEX9E(struct asmStuff *regs, unsigned char X); // EX9E

//  Skips the next instruction if the key stored in VX isn't pressed. 
//  (Usually the next instruction is a jump to skip a code block) 
void asmEXA1(struct asmStuff *regs, unsigned char X); // EXA1

//  Sets VX to the value of the delay timer. 
void asmFX07(unsigned char X, struct asmStuff *regs); // FX07

//  A key press is awaited, and then stored in VX. 
//  (Blocking Operation. All instruction halted until next key event) 
void asmFX0A(struct asmStuff *regs, unsigned char X); // FX0A

//  Sets the delay timer to VX. 
void asmFX15(unsigned char X, struct asmStuff *regs); // FX15

//  Sets the sound timer to VX. 
void asmFX18(struct asmStuff *regs, unsigned char X); // FX18

//  Adds VX to I. VF is set to 1 when there is a 
//  range overflow (I+VX>0xFFF), and to 0 when there isn't.
void asmFX1E(struct asmStuff *regs, unsigned char X); // FX1E

//  Sets I to the location of the sprite for the character in VX. 
//  Characters 0-F (in hexadecimal) are represented by a 4x5 font. 
void asmFX29(struct asmStuff *regs, unsigned char X); // FX29

//  Stores the binary-coded decimal representation of VX, with the most 
//  significant of three digits at the address in I, the middle digit at I plus 1, 
//  and the least significant digit at I plus 2. 
//  (In other words, take the decimal representation of VX, place the hundreds digit 
//  in memory at location in I, the tens digit at location I+1, and the ones digit at 
//  location I+2.) 
void asmFX33(struct asmStuff *regs, unsigned char X); // FX33

//  Stores V0 to VX (including VX) in memory starting at address I. The offset from I 
//  is increased by 1 for each value written, but I itself is left unmodified.
void asmFX55(struct asmStuff *regs, unsigned char X); // FX55

//  Fills V0 to VX (including VX) with values from memory starting at address I. The 
//  offset from I is increased by 1 for each value written, but I itself is left unmodified.
void asmFX65(struct asmStuff *regs, unsigned char X); // FX65

void performOpcode(unsigned short opcode, struct asmStuff* regs);

#endif