#ifndef __UI_BASIC_WIDGET_H__
#define __UI_BASIC_WIDGET_H__

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include "ui_main.h"

typedef struct btn_apend {
	GtkWidget *container;
	GtkWidget *image;
	GtkWidget * mouse_on;
	GtkWidget * loading;
	GtkWidget * label;
	GtkWidget * event;
	GtkWidget * under_img;
	GtkWidget * under_img_h;
	gboolean	disable;
} btn_apend_t;

extern btn_apend_t * create_button(char* icon, char* icon_mouse_on, char * icon_loading, char *text, int width, int height, GCallback callback, void* data);
extern GtkWidget * add_entry_style(GtkWidget *entry,  char* icon,  char *text, int width, int height, gboolean visible);
extern void set_btn_effect(btn_apend_t *btn_apend, gboolean enable);
extern void update_widget_bg(GtkWidget *widget, const gchar *img_file, int s_width, int s_height);
extern void update_widget_irregular_bg(GtkWidget *widget, const gchar *img_file, int s_width, int s_height);
extern void btn_normal_show(btn_apend_t * btn_apend);
extern GtkWidget * create_err_prompt(char *icon, char * text, int width, int height);
extern void set_widget_font_size(GtkWidget *widget, int size, char * color_str);
extern void set_widget_font_type_size(GtkWidget *widget, char *font_name, char * color_str);
extern void reset_cursor(GtkWidget *widget);
GtkWidget *title_bar_init(char *title, void *data, int type);
void ui_dialog_white_background(GtkWidget *dialog);
btn_apend_t * create_button_above_label(char* icon, char* icon_mouse_on, char * icon_loading, char *text, GCallback callback, void* data);
gint ui_destory_dialog_callback(GtkWidget *widget,GdkEventButton *event, gpointer data);
gint ui_destory_dialog_start_timer(GtkWidget *widget,GdkEventButton *event, gpointer data);
gint ui_destory_dialog_callback_notify(GtkWidget *widget,GdkEventButton *event, gpointer data);
gint ui_destory_wifi_config_callback_notify(GtkWidget *widget,GdkEventButton *event, gpointer data);
GtkWidget * create_prompt(char *icon, char * lab_text, int lab_font_size, char * btn_text, GCallback callback, void* data);
void btn_loading_show(btn_apend_t * btn_apend);
GtkWidget * create_newdeploy_guide_bar(int type);
gint label_mouse_handle(GtkWidget *widget,GdkEventButton *event, gpointer data);
void btn_normal_hide(btn_apend_t * btn_apend);

#endif
