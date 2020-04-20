#ifndef GTK_H_INCLUDED
#define GTK_H_INCLUDED

#include <gtk/gtk.h>
#include <cairo.h>

void gtk_main_quit(void);

gboolean gtk_update_display(cairo_t *cr, struct asmStuff *regs);

static gboolean gtk_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data);

gboolean gtk_timeout_event(GtkWidget *widget);

gboolean gtk_key_press (GtkWidget *widget, GdkEventKey *event, gpointer user_data);

gboolean gtk_key_release (GtkWidget *widget, GdkEventKey *event, gpointer user_data);

#endif