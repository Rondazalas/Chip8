#include <gtk/gtk.h>
#include <cairo.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <sys/time.h>
#include <math.h>

#define ROM_NAME "PONG2"

#define chip8_screen_w 64 // the base internal resolution WIDTH for chip8
#define chip8_screen_h 32 // the base internal resolution HEIGHT for chip8
#define screen_res_mod 10 // the modifier for GTK to increase pixel size on screen

#define HEX00F(hexNum) (hexNum & 0xf) // grabs right-most nibble of a value (nibble s 4 bits)
#define HEX0F0(hexNum) ((hexNum >> 4) & 0xf) // grabs second right-most nibble of a value
#define HEXF00(hexNum) ((hexNum >> 8) & 0xf) // grabs third right-most nibble of a value
#define HEX0FF(hexNum) (hexNum & 0xff) // grabs the second right-most and right-most nibbles of a value
#define HEXFF0(hexNum) ((hexNum >> 4) & 0xff) // grabs the seconf and third right-most nibbles of a value
#define HEXFFF(hexNum) (hexNum & 0xfff) // grabs the three right-most nibbles of a value

#define KB0 GDK_KEY_x  // The general keybinds layout:
#define KB1 GDK_KEY_1  //  1  2  3  C
#define KB2 GDK_KEY_2  //  4  5  6  D
#define KB3 GDK_KEY_3  //  7  8  9  E
#define KB4 GDK_KEY_q  //  A  0  B  F
#define KB5 GDK_KEY_w
#define KB6 GDK_KEY_e
#define KB7 GDK_KEY_a
#define KB8 GDK_KEY_s
#define KB9 GDK_KEY_d
#define KBA GDK_KEY_z
#define KBB GDK_KEY_c
#define KBC GDK_KEY_4
#define KBD GDK_KEY_r
#define KBE GDK_KEY_f
#define KBF GDK_KEY_v

struct asmStuff
{
	unsigned char V[16];
	unsigned short PC;
	unsigned short I;
	unsigned short SP;
	unsigned char memory[4096]; // stack = 0xEA0-0xEFF | display = 0xF00-0xFFF
	unsigned char graphics[2048]; //0x800
	unsigned char key[16];
	unsigned char delayTimer;
	unsigned char soundTimer;
	unsigned char FX0A_stop; // will be 1 if FX0A asks for all assembly to pause until user input
	unsigned char FX0A_X;// Only used for asmFX0A
	unsigned char FX0A_key[16];
};

unsigned char fontset[80] = 
{ 
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void getROM(char *romMemoryStart)
{
	// open file
	FILE *fp;
	fp = fopen(ROM_NAME, "r");
	if(fp == NULL)
	{
		printf("ROM could not be found. Exiting program...\n");
		exit(0);
	}
	// get size of file
	fseek(fp, 0L, SEEK_END);
	int romBytes = ftell(fp);
	rewind(fp);

	// copy each byte into memory
	fread(romMemoryStart, romBytes, 1, fp);

	fclose(fp);
}

void asmStuffSetup(struct asmStuff *given)
{
	getROM(&(given->memory[0x200]));

	given->I = 0x000;
	given->PC = 0x200;
	given->SP = 0xEA0;
	
	given->FX0A_stop = 0;

	for (int i = 0; i < 80; i++)
	{
		given->memory[i] = fontset[i];
	}
}

// 	Calls RCA 1802 program at address NNN. Not necessary for most ROMs. 
void asm0NNN() // 0NNN
{
	//printf("0NNN ran\n");
}

// 	Clears the screen. 
void asm00E0(struct asmStuff *regs) // 00E0
{
	for (unsigned short p = 0x000; p < 0x800; p++)
	{
		regs->graphics[p] = 0x00;
	}

	//printf("00E0 ran\n");
}

//  Returns from a subroutine. 
void asm00EE(struct asmStuff *regs) // 00EE
{
	if (regs->SP - 2 < 0xEA0)
	{
		perror("CRITICAL ERROR: SP out of range of stack (lower).\n");
		exit(-1);
	}

	regs->SP -= 2; // go down one stack address
	regs->PC = (regs->memory[regs->SP] << 8) | regs->memory[regs->SP+1]; // jump
	regs->memory[regs->SP] = 0; // reset current return address in stack
	//printf("00EE ran\n");
}

// 	Jumps to address NNN. 
void asm1NNN(struct asmStuff *regs, unsigned short NNN) // 1NNN
{
	regs->PC = NNN;
	regs->PC -= 2; // offset the usual PC increase
	//printf("1NNN ran\n");
}

// 	Calls subroutine at NNN. 
void asm2NNN(struct asmStuff *regs, unsigned short NNN) // 2NNN
{
	if (regs->SP > 0xEFF)
	{
		perror("CRITICAL ERROR: SP out of range of stack (upper).\n");
		exit(-1);
	}

	regs->memory[regs->SP] = regs->PC >> 8; // store return address in stack
	regs->memory[regs->SP+1] = regs->PC & 0x00FF; // store return address in stack
	regs->SP += 2; // go up one stack level
	regs->PC = NNN; // jump to NNN
	regs->PC -= 2; // adjust PC prior to automated opcode increment
	//printf("2NNN ran\n");
}

// 	3XNN - Skips the next instruction if VX equals NN. 
//  (Usually the next instruction is a jump to skip a code block) 
void asm3XNN(struct asmStuff *regs, unsigned char X, unsigned char NN) // 3XNN
{
	if (regs->V[X] == NN)
		regs->PC += 2;
	//printf("3XNN ran\n");
}

//  4XNN - Skips the next instruction if VX doesn't equal NN. 
//  (Usually the next instruction is a jump to skip a code block) 
void asm4XNN(struct asmStuff *regs, unsigned char X, unsigned char NN) // 4XNN
{
	if (regs->V[X] != NN)
		regs->PC += 2;
	//printf("4XNN ran\n");
}

//  5XY0 - Skips the next instruction if VX equals VY. 
//  (Usually the next instruction is a jump to skip a code block) 
void asm5XY0(struct asmStuff *regs, unsigned char X, unsigned char Y) // 5XY0
{
	if (regs->V[X] == regs->V[Y])
		regs->PC += 2;
	//printf("5XY0 ran\n");
}

//  6XNN - Sets VX to NN. 
void asm6XNN(struct asmStuff *regs, unsigned char X, unsigned char NN) // 6XNN 
{
	regs->V[X] = NN;
	//printf("6XNN ran\n");
}

// 	7XNN - Adds NN to VX. (Carry flag is not changed) 
void asm7XNN(struct asmStuff *regs, unsigned char X, unsigned char NN) // 7XNN
{
	regs->V[X] += NN;
	//printf("7XNN ran\n");
}

//  8XY0 - Sets VX to the value of VY. 
void asm8XY0(struct asmStuff *regs, unsigned char X, unsigned char Y) // 8XY0
{
	regs->V[X] = regs->V[Y];
	//printf("8XY0 ran\n");
}

// 	Sets VX to VX or VY. (Bitwise OR operation) 
void asm8XY1(struct asmStuff *regs, unsigned char X, unsigned char Y) // 8XY1
{
	regs->V[X] = (regs->V[X] | regs->V[Y]);
	//printf("8XY1 ran\n");
}

// 	Sets VX to VX and VY. (Bitwise AND operation) 
void asm8XY2(struct asmStuff *regs, unsigned char X, unsigned char Y) // 8XY2
{
	regs->V[X] = (regs->V[X] & regs->V[Y]);
	//printf("8XY2 ran\n");
}

// 	Sets VX to VX xor VY. 
void asm8XY3(struct asmStuff *regs, unsigned char X, unsigned char Y) // 8XY3
{
	regs->V[X] = (regs->V[X] ^ regs->V[Y]);
	//printf("8XY3 ran\n");
}

//  8XY4 - Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't. 
void asm8XY4(struct asmStuff *regs, unsigned char X, unsigned char Y) // 8XY4
{
	if (regs->V[X] + regs->V[Y] < regs->V[X])
		regs->V[0xF] = 1;
	else
		regs->V[0xF] = 0;
	regs->V[X] += regs->V[Y];
	//printf("8XY4 ran\n");
}

// 	VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't. 
void asm8XY5(struct asmStuff *regs, unsigned char X, unsigned char Y) // 8XY5
{
	if (regs->V[X] - regs->V[Y] > regs->V[X])
		regs->V[0xF] = 0;
	else
		regs->V[0xF] = 1;
	regs->V[X] -= regs->V[Y];
	//printf("8XY5 ran\n");
}

//  Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
void asm8XY6(struct asmStuff *regs, unsigned char X) // 8XY6
{
	regs->V[0xF] = regs->V[X] & 0x01;
	regs->V[X] >>= 1;
	//printf("8XY6 ran\n");
}

//  Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't. 
void asm8XY7(struct asmStuff *regs, unsigned char X, unsigned char Y) // 8XY7
{
	if (regs->V[Y] - regs->V[X] > regs->V[Y])
		regs->V[0xF] = 0;
	else
		regs->V[0xF] = 1;
	regs->V[X] = regs->V[Y] - regs->V[X];
	//printf("8XY7 ran\n");
}

//  Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
void asm8XYE(struct asmStuff *regs, unsigned char X) // 8XYE
{
	regs->V[0xF] = regs->V[X] & 0x80;
	regs->V[X] <<= 1;
	//printf("8XYE ran\n");
}

// 	9XY0 - Skips the next instruction if VX doesn't equal VY. 
//  (Usually the next instruction is a jump to skip a code block) 
void asm9XY0(struct asmStuff *regs, unsigned char X, unsigned char Y) // 9XY0
{
	if (regs->V[X] != regs->V[Y])
		regs->PC += 2;
	//printf("9XY0 ran\n");
}

//  Sets I to the address NNN. 
void asmANNN(struct asmStuff *regs, unsigned short NNN) // ANNN
{
	regs->I = NNN;
	//printf("ANNN ran\n");
}

//  Jumps to the address NNN plus V0. 
void asmBNNN(struct asmStuff *regs, unsigned short NNN) // BNNN
{
	regs->PC = NNN + regs->V[0];
	regs->PC -= 2;
	//printf("BNNN ran\n");
}

//  Sets VX to the result of a bitwise and operation on a random number 
//  (Typically: 0 to 255) and NN. 
void asmCXNN(struct asmStuff *regs, unsigned char X, unsigned char NN) // CXNN
{
	regs->V[X] = NN & (rand() % (255 + 1 - 0) + 0);
	//printf("CXNN ran\n");
}

//  Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels 
//  and a height of N pixels. Each row of 8 pixels is read as bit-coded 
//  starting from memory location I; I value doesn’t change after the execution 
//  of this instruction. As described above, VF is set to 1 if any screen pixels 
//  are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen
void asmDXYN(struct asmStuff *regs, unsigned char X, unsigned char Y, unsigned char N) // DXYN
{
	unsigned char pixel;

	regs->V[0xF] = 0x00;
	for (int ySprite = 0; ySprite < N; ySprite++)
	{
		pixel = regs->memory[regs->I + ySprite];
		for(int xSprite = 0; xSprite < 8; xSprite++)
		{
			if((pixel & (0x80 >> xSprite)) != 0) // 0x80 = 0x1000
			{
				if(regs->graphics[regs->V[X] + xSprite + ((regs->V[Y] + ySprite) * 64)] == 1)
				{
					regs->V[0xF] = 1;                                    
				}
				regs->graphics[regs->V[X] + xSprite + ((regs->V[Y] + ySprite) * 64)] ^= 1;
			}
		}
	}
	//printf("DXYN ran\n");
}

//  Skips the next instruction if the key stored in VX is pressed. 
//  (Usually the next instruction is a jump to skip a code block) 
void asmEX9E(struct asmStuff *regs, unsigned char X) // EX9E
{
	if (regs->V[X] > 0x0F)
	{
		perror("CRITICAL ERROR: EX9E shows that key array is out of range due to VX's value.\n");
		exit(-1);
	}
	if (regs->key[regs->V[X]] == 1)
		regs->PC += 2;
	//printf("EX9E ran\n");
}

//  Skips the next instruction if the key stored in VX isn't pressed. 
//  (Usually the next instruction is a jump to skip a code block) 
void asmEXA1(struct asmStuff *regs, unsigned char X) // EXA1
{
	if (regs->V[X] > 0x0F)
	{
		perror("CRITICAL ERROR: EXA1 shows that key array is out of range due to VX's value.\n");
		exit(-1);
	}
	if (regs->key[regs->V[X]] == 0)
		regs->PC += 2;
	//printf("EXA1 ran\n");
}

//  Sets VX to the value of the delay timer. 
void asmFX07(unsigned char X, struct asmStuff *regs) // FX07
{
	regs->V[X] = regs->delayTimer;
	//printf("FX07 ran\n");
}

//  A key press is awaited, and then stored in VX. 
//  (Blocking Operation. All instruction halted until next key event) 
void asmFX0A(struct asmStuff *regs, unsigned char X) // FX0A
{
	// The remainder of this function is done in gtk_update_display
	regs->FX0A_stop = 1;
	regs->FX0A_X = X;
	for(int i = 0x0; i <= 0xF; i++) // track all keys currently pressed so we can tell what should end the pause.
		regs->FX0A_key[i] = regs->key[i];
	//printf("FX0A ran\n");
}

//  Sets the delay timer to VX. 
void asmFX15(unsigned char X, struct asmStuff *regs) // FX15
{
	regs->delayTimer = regs->V[X];
	//printf("FX15 ran\n");
}

//  Sets the sound timer to VX. 
void asmFX18(struct asmStuff *regs, unsigned char X) // FX18
{
	regs->soundTimer = regs->V[X];
	//printf("FX18 ran\n");
}

//  Adds VX to I. VF is set to 1 when there is a 
//  range overflow (I+VX>0xFFF), and to 0 when there isn't.
void asmFX1E(struct asmStuff *regs, unsigned char X) // FX1E
{
	if((regs->I + regs->V[X]) > 0xFFF)
		regs->V[0xF] = 1;
	else
		regs->V[0xF] = 0;
	regs->I += regs->V[X];
	//printf("FX1E ran\n");
}

//  Sets I to the location of the sprite for the character in VX. 
//  Characters 0-F (in hexadecimal) are represented by a 4x5 font. 
void asmFX29(struct asmStuff *regs, unsigned char X) // FX29
{
	regs->I = regs->V[X] * 5;
	//printf("FX29 ran\n");
}

//  Stores the binary-coded decimal representation of VX, with the most 
//  significant of three digits at the address in I, the middle digit at I plus 1, 
//  and the least significant digit at I plus 2. 
//  (In other words, take the decimal representation of VX, place the hundreds digit 
//  in memory at location in I, the tens digit at location I+1, and the ones digit at 
//  location I+2.) 
void asmFX33(struct asmStuff *regs, unsigned char X) // FX33
{
	regs->memory[regs->I] = regs->V[X] / 100;
	regs->memory[regs->I + 1] = (regs->V[X] / 10) % 10;
	regs->memory[regs->I + 2] = (regs->V[X] % 100) % 10;
	//printf("FX33 ran\n");
}

//  Stores V0 to VX (including VX) in memory starting at address I. The offset from I 
//  is increased by 1 for each value written, but I itself is left unmodified.
void asmFX55(struct asmStuff *regs, unsigned char X) // FX55
{
	for (int j = 0; j <= X; j++)
	{
		regs->memory[regs->I+j] = regs->V[j];
	}
	//printf("FX55 ran\n");
}

//  Fills V0 to VX (including VX) with values from memory starting at address I. The 
//  offset from I is increased by 1 for each value written, but I itself is left unmodified.
void asmFX65(struct asmStuff *regs, unsigned char X) // FX65
{
	for (int j = 0; j <= X; j++)
	{
		regs->V[j] = regs->memory[regs->I+j];
	}
	//printf("FX65 ran\n");
}

void performOpcode(unsigned short opcode, struct asmStuff* regs)
{
	//printf("DEBUG | Opcode: %x | PC: %x\n", opcode, regs->PC);
	if(opcode == 0x00E0) // 00E0
		asm00E0(regs);
	else if(opcode == 0x00EE) // 00EE
		asm00EE(regs);
	else if ((opcode & 0xF000) == 0x0000) // 0NNN
		asm0NNN();
	else if((opcode & 0xf000) == 0x1000) // 1NNN
		asm1NNN(regs, HEXFFF(opcode));
	else if((opcode & 0xf000) == 0x2000) // 2NNN
		asm2NNN(regs, HEXFFF(opcode));
	else if((opcode & 0xf000) == 0x3000) // 3XNN
		asm3XNN(regs, HEXF00(opcode), HEX0FF(opcode));
	else if((opcode & 0xf000) == 0x4000) // 4XNN
		asm4XNN(regs, HEXF00(opcode), HEX0FF(opcode));
	else if((opcode & 0xf00f) == 0x5000) // 5XY0
		asm5XY0(regs, HEXF00(opcode), HEX0F0(opcode));
	else if((opcode & 0xf000) == 0x6000) // 6XNN
		asm6XNN(regs, HEXF00(opcode), HEX0FF(opcode));
	else if((opcode & 0xf000) == 0x7000) // 7XNN
		asm7XNN(regs, HEXF00(opcode), HEX0FF(opcode));
	else if((opcode & 0xf00f) == 0x8000) // 8XY0
		asm8XY0(regs, HEXF00(opcode), HEX0F0(opcode));
	else if((opcode & 0xf00f) == 0x8001) // 8XY1
		asm8XY1(regs, HEXF00(opcode), HEX0F0(opcode));
	else if((opcode & 0xf00f) == 0x8002) // 8XY2
		asm8XY2(regs, HEXF00(opcode), HEX0F0(opcode));
	else if((opcode & 0xf00f) == 0x8003) // 8XY3
		asm8XY3(regs, HEXF00(opcode), HEX0F0(opcode));
	else if((opcode & 0xf00f) == 0x8004) // 8XY4
		asm8XY4(regs, HEXF00(opcode), HEX0F0(opcode));
	else if((opcode & 0xf00f) == 0x8005) // 8XY5
		asm8XY5(regs, HEXF00(opcode), HEX0F0(opcode));
	else if((opcode & 0xf00f) == 0x8006) // 8XY6
		asm8XY6(regs, HEXF00(opcode));
	else if((opcode & 0xf00f) == 0x8007) // 8XY7
		asm8XY7(regs, HEXF00(opcode), HEX0F0(opcode));
	else if((opcode & 0xf00f) == 0x800E) // 8XYE
		asm8XYE(regs, HEXF00(opcode));
	else if((opcode & 0xf00f) == 0x9000) // 9XY0
		asm9XY0(regs, HEXF00(opcode), HEX0F0(opcode));
	else if((opcode & 0xf000) == 0xA000) // ANNN
		asmANNN(regs, HEXFFF(opcode));
	else if((opcode & 0xf000) == 0xB000) // BNNN
		asmBNNN(regs, HEXFFF(opcode));
	else if((opcode & 0xf000) == 0xC000) // CXNN
		asmCXNN(regs, HEXF00(opcode), HEX0FF(opcode));
	else if((opcode & 0xf000) == 0xD000) // DXYN
		asmDXYN(regs, HEXF00(opcode), HEX0F0(opcode), HEX00F(opcode));
	else if((opcode & 0xf0ff) == 0xE09E) // EX9E
		asmEX9E(regs, HEXF00(opcode));
	else if((opcode & 0xf0ff) == 0xE0A1) // EXA1
		asmEXA1(regs, HEXF00(opcode));
	else if((opcode & 0xf0ff) == 0xF007) // FX07
		asmFX07(HEXF00(opcode), regs);
	else if((opcode & 0xf0ff) == 0xF00A) // FX0A
		asmFX0A(regs, HEXF00(opcode));
	else if((opcode & 0xf0ff) == 0xF015) // FX15
		asmFX15(HEXF00(opcode), regs);
	else if((opcode & 0xf0ff) == 0xF018) // FX18
		asmFX18(regs, HEXF00(opcode));
	else if((opcode & 0xf0ff) == 0xF01E) // FX1E
		asmFX1E(regs, HEXF00(opcode));
	else if((opcode & 0xf0ff) == 0xF029) // FX29
		asmFX29(regs, HEXF00(opcode));
	else if((opcode & 0xf0ff) == 0xF033) // FX33
		asmFX33(regs, HEXF00(opcode));
	else if((opcode & 0xf0ff) == 0xF055) // FX55
		asmFX55(regs, HEXF00(opcode));
	else if((opcode & 0xf0ff) == 0xF065) // FX65
		asmFX65(regs, HEXF00(opcode));
	else
		printf("instruction not found: Opcode was %x | PC was %x\n", opcode, regs->PC);
}

///////////////////
//     GTK STUFF
///////////////////

void gtk_main_quit(void)
{
  exit(0);
}

gboolean gtk_update_display(cairo_t *cr, struct asmStuff *regs)
{
	// Update timers
	if(regs->delayTimer > 0)
		--regs->delayTimer;
	if(regs->soundTimer > 0)
		--regs->soundTimer;

	//printf("%x\n", regs->delayTimer);

	cairo_set_source_rgb (cr, 0, 0, 0.5); // set color

	for(unsigned short y = 0; y < 32; y++)
	{
		for(unsigned char x = 0; x < 64; x++) 
		{
			if(regs->graphics[x + y*64] == 1)
			{
				cairo_set_line_width(cr, 0);
				cairo_rectangle(cr, (x * screen_res_mod), (y * screen_res_mod), screen_res_mod, screen_res_mod);
			}
		}
	}
	cairo_fill (cr);

	return FALSE;
}

static gboolean gtk_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
  gtk_update_display(cr, (struct asmStuff *)user_data);

  return FALSE;
}

gboolean gtk_timeout_event(GtkWidget *widget)
{
  gtk_widget_queue_draw(widget);

  return TRUE;
}

gboolean gtk_key_press (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
  struct asmStuff *regs = (struct asmStuff *)user_data;

  if(event->keyval == KB0)
  {
  	regs->key[0x0] = 1;
  	if(regs->FX0A_stop == 1 && regs->FX0A_key[0x0] == 0)
  	{
  		regs->FX0A_stop = 0;
  		regs->V[regs->FX0A_X] = 0x0;
  	}
  }
  if(event->keyval == KB1)
  {
  	regs->key[0x1] = 1;
  	if(regs->FX0A_stop == 1 && regs->FX0A_key[0x1] == 0)
  	{
  		regs->FX0A_stop = 0;
  		regs->V[regs->FX0A_X] = 0x1;
  	}
  }
  if(event->keyval == KB2)
  {
  	regs->key[0x2] = 1;
  	if(regs->FX0A_stop == 1 && regs->FX0A_key[0x2] == 0)
  	{
  		regs->FX0A_stop = 0;
  		regs->V[regs->FX0A_X] = 0x2;
  	}
  }
  if(event->keyval == KB3)
  {
  	regs->key[0x3] = 1;
  	if(regs->FX0A_stop == 1 && regs->FX0A_key[0x3] == 0)
  	{
  		regs->FX0A_stop = 0;
  		regs->V[regs->FX0A_X] = 0x3;
  	}
  }
  if(event->keyval == KB4)
  {
  	regs->key[0x4] = 1;
  	if(regs->FX0A_stop == 1 && regs->FX0A_key[0x4] == 0)
  	{
  		regs->FX0A_stop = 0;
  		regs->V[regs->FX0A_X] = 0x4;
  	}
  }
  if(event->keyval == KB5)
  {
  	regs->key[0x5] = 1;
  	if(regs->FX0A_stop == 1 && regs->FX0A_key[0x5] == 0)
  	{
  		regs->FX0A_stop = 0;
  		regs->V[regs->FX0A_X] = 0x5;
  	}
  }
  if(event->keyval == KB6)
  {
  	regs->key[0x6] = 1;
  	if(regs->FX0A_stop == 1 && regs->FX0A_key[0x6] == 0)
  	{
  		regs->FX0A_stop = 0;
  		regs->V[regs->FX0A_X] = 0x6;
  	}
  }
  if(event->keyval == KB7)
  {
  	regs->key[0x7] = 1;
  	if(regs->FX0A_stop == 1 && regs->FX0A_key[0x7] == 0)
  	{
  		regs->FX0A_stop = 0;
  		regs->V[regs->FX0A_X] = 0x7;
  	}
  }
  if(event->keyval == KB8)
  {
  	regs->key[0x8] = 1;
  	if(regs->FX0A_stop == 1 && regs->FX0A_key[0x8] == 0)
  	{
  		regs->FX0A_stop = 0;
  		regs->V[regs->FX0A_X] = 0x8;
  	}
  }
  if(event->keyval == KB9)
  {
  	regs->key[0x9] = 1;
  	if(regs->FX0A_stop == 1 && regs->FX0A_key[0x9] == 0)
  	{
  		regs->FX0A_stop = 0;
  		regs->V[regs->FX0A_X] = 0x9;
  	}
  }
  if(event->keyval == KBA)
  {
  	regs->key[0xA] = 1;
  	if(regs->FX0A_stop == 1 && regs->FX0A_key[0xA] == 0)
  	{
  		regs->FX0A_stop = 0;
  		regs->V[regs->FX0A_X] = 0xA;
  	}
  }
  if(event->keyval == KBB)
  {
  	regs->key[0xB] = 1;
  	if(regs->FX0A_stop == 1 && regs->FX0A_key[0xB] == 0)
  	{
  		regs->FX0A_stop = 0;
  		regs->V[regs->FX0A_X] = 0xB;
  	}
  }
  if(event->keyval == KBC)
  {
  	regs->key[0xC] = 1;
  	if(regs->FX0A_stop == 1 && regs->FX0A_key[0xC] == 0)
  	{
  		regs->FX0A_stop = 0;
  		regs->V[regs->FX0A_X] = 0xC;
  	}
  }
  if(event->keyval == KBD)
  {
  	regs->key[0xD] = 1;
  	if(regs->FX0A_stop == 1 && regs->FX0A_key[0xD] == 0)
  	{
  		regs->FX0A_stop = 0;
  		regs->V[regs->FX0A_X] = 0xD;
  	}
  }
  if(event->keyval == KBE)
  {
  	regs->key[0xE] = 1;
  	if(regs->FX0A_stop == 1 && regs->FX0A_key[0xE] == 0)
  	{
  		regs->FX0A_stop = 0;
  		regs->V[regs->FX0A_X] = 0xE;
  	}
  }
  if(event->keyval == KBF)
  {
  	regs->key[0xF] = 1;
  	if(regs->FX0A_stop == 1 && regs->FX0A_key[0xF] == 0)
  	{
  		regs->FX0A_stop = 0;
  		regs->V[regs->FX0A_X] = 0xF;
  	}
  }

  return FALSE; 
}

gboolean gtk_key_release (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
  struct asmStuff *regs = (struct asmStuff *)user_data;
  
  if(event->keyval == KB0)
  {
  	regs->key[0x0] = 0;
  	if(regs->FX0A_stop == 1)
  		regs->FX0A_key[0x0] = 0;
  }
  if(event->keyval == KB1)
  {
  	regs->key[0x1] = 0;
  	if(regs->FX0A_stop == 1)
  		regs->FX0A_key[0x1] = 0;
  }
  if(event->keyval == KB2)
  {
  	regs->key[0x2] = 0;
  	if(regs->FX0A_stop == 1)
  		regs->FX0A_key[0x2] = 0;
  }
  if(event->keyval == KB3)
  {
  	regs->key[0x3] = 0;
  	if(regs->FX0A_stop == 1)
  		regs->FX0A_key[0x3] = 0;
  }
  if(event->keyval == KB4)
  {
  	regs->key[0x4] = 0;
  	if(regs->FX0A_stop == 1)
  		regs->FX0A_key[0x4] = 0;
  }
  if(event->keyval == KB5)
  	{
  	regs->key[0x5] = 0;
  	if(regs->FX0A_stop == 1)
  		regs->FX0A_key[0x5] = 0;
  }
  if(event->keyval == KB6)
  {
  	regs->key[0x6] = 0;
  	if(regs->FX0A_stop == 1)
  		regs->FX0A_key[0x6] = 0;
  }
  if(event->keyval == KB7)
  	{
  	regs->key[0x7] = 0;
  	if(regs->FX0A_stop == 1)
  		regs->FX0A_key[0x7] = 0;
  }
  if(event->keyval == KB8)
  {
  	regs->key[0x8] = 0;
  	if(regs->FX0A_stop == 1)
  		regs->FX0A_key[0x8] = 0;
  }
  if(event->keyval == KB9)
  {
  	regs->key[0x9] = 0;
  	if(regs->FX0A_stop == 1)
  		regs->FX0A_key[0x9] = 0;
  }
  if(event->keyval == KBA)
  {
  	regs->key[0xA] = 0;
  	if(regs->FX0A_stop == 1)
  		regs->FX0A_key[0xA] = 0;
  }
  if(event->keyval == KBB)
  {
  	regs->key[0xB] = 0;
  	if(regs->FX0A_stop == 1)
  		regs->FX0A_key[0xB] = 0;
  }
  if(event->keyval == KBC)
  {
  	regs->key[0xC] = 0;
  	if(regs->FX0A_stop == 1)
  		regs->FX0A_key[0xC] = 0;
  }
  if(event->keyval == KBD)
  {
  	regs->key[0xD] = 0;
  	if(regs->FX0A_stop == 1)
  		regs->FX0A_key[0xD] = 0;
  }
  if(event->keyval == KBE)
  {
  	regs->key[0xE] = 0;
  	if(regs->FX0A_stop == 1)
  		regs->FX0A_key[0xE] = 0;
  }
  if(event->keyval == KBF)
  {
  	regs->key[0xF] = 0;
  	if(regs->FX0A_stop == 1)
  		regs->FX0A_key[0xF] = 0;
  }

  return FALSE; 
}

int main(int argc, char **argv)
{
	// Setting up asmStuff structure
	struct asmStuff *regs;
	regs = malloc(sizeof(struct asmStuff));
	asmStuffSetup(regs);

	// GTK Widget Pointers
	GtkWidget *window;
	GtkWidget *darea;

	// Initialize GTK
	gtk_init(&argc, &argv);

	// GTK Window properties
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	darea = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(window), darea);

	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), (chip8_screen_w*screen_res_mod), (chip8_screen_h*screen_res_mod)); 
	gtk_window_set_title(GTK_WINDOW(window), "Chip8 Emulator or interpreter or whatever.");

	// Connecting g_signals for GTK functions
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(gtk_draw_event), regs);
	g_signal_connect (G_OBJECT (window), "key_press_event", G_CALLBACK (gtk_key_press), regs);
	g_signal_connect (G_OBJECT (window), "key_release_event", G_CALLBACK (gtk_key_release), regs);

	// I'd be lying if I told you that I remembered what this did
	gtk_widget_show_all(window);

	// Run the gtk_timer_handler function every (first parameter) miliseconds
	g_timeout_add(16, (GSourceFunc)gtk_timeout_event, darea);

	// Current opcode
	unsigned short opcode;

	while(1)
	{
		opcode = (regs->memory[regs->PC] << 8) | regs->memory[regs->PC + 1];
		if(regs->FX0A_stop != 1)
		{
			performOpcode(opcode, regs);
			regs->PC += 2;
		}
		while (gtk_events_pending())
    	{
      		gtk_main_iteration();
   		}
	}

	return 0;
}

