#include "assembly.c"
#include "gtk_tools.c"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define chip8_screen_w 64 // the base internal resolution WIDTH for chip8
#define chip8_screen_h 32 // the base internal resolution HEIGHT for chip8
#define screen_res_mod 12 // the modifier for GTK to increase pixel size on screen

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

