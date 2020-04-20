#include "assembly.h"
#include "gtk_tools.h"
#include <gtk/gtk.h>
#include <cairo.h>

#define chip8_screen_w 64 // the base internal resolution WIDTH for chip8
#define chip8_screen_h 32 // the base internal resolution HEIGHT for chip8
#define screen_res_mod 12 // the modifier for GTK to increase pixel size on screen

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