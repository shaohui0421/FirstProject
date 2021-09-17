#include "ui_basic_widget.h"
#include "ui_extern.h"

static GdkCursor * cursor = NULL;

void btn_normal_hide(btn_apend_t * btn_apend)
{
	if (btn_apend->image) {
		gtk_widget_hide(btn_apend->image);
	}

    if(btn_apend->label) {
    	gtk_widget_hide(btn_apend->label);
    }

    if (btn_apend->mouse_on) {
    	gtk_widget_hide(btn_apend->mouse_on);
    }

    //gtk_widget_hide(btn_apend->loading);
	if (btn_apend->loading) {
		gtk_widget_hide(btn_apend->loading);
	}
	gtk_widget_hide(btn_apend->container);
}


void btn_normal_show(btn_apend_t * btn_apend)
{
	if (btn_apend->image) {
		gtk_widget_show(btn_apend->image);
	}

    if(btn_apend->label) {
    	gtk_widget_show(btn_apend->label);
    }

    if (btn_apend->mouse_on) {
    	gtk_widget_hide(btn_apend->mouse_on);
    }

    //gtk_widget_hide(btn_apend->loading);
	if (btn_apend->loading) {
		gtk_widget_hide(btn_apend->loading);
	}
	gtk_widget_show(btn_apend->container);
}

void btn_loading_show(btn_apend_t * btn_apend)
{
	if (btn_apend->image) {
		gtk_widget_hide(btn_apend->image);
	}

    if(btn_apend->label) {
    	gtk_widget_hide(btn_apend->label);
    }

    if (btn_apend->mouse_on) {
    	gtk_widget_show(btn_apend->mouse_on);
    }

    //gtk_widget_hide(btn_apend->loading);
	if (btn_apend->loading) {
		gtk_widget_show(btn_apend->loading);
	}
	gtk_widget_show(btn_apend->container);
}

void set_btn_effect(btn_apend_t *btn_apend, gboolean enable)
{
	btn_apend->disable = !enable;
}

gboolean is_btn_able(btn_apend_t *btn_apend)
{
	return !btn_apend->disable;
}

gint button_press_handle(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    btn_apend_t * btn_apend = (btn_apend_t *)data;
    if (!is_btn_able(btn_apend)) {
    	return TRUE;
    }

    if (btn_apend->loading) {
    	gtk_widget_show(btn_apend->loading);
    	if (btn_apend->label) {
    		gtk_widget_hide(btn_apend->label);
    	}
    }

    return FALSE;
}

void reset_cursor(GtkWidget *widget)
{
    cursor = gdk_cursor_new(GDK_LEFT_PTR);
    gdk_window_set_cursor (widget->window, cursor);
    gdk_cursor_unref(cursor);
}

gint button_mouse_handle(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    btn_apend_t * btn_apend = (btn_apend_t *)data;
    static GdkPixbuf *src_pixbuf = NULL;

    if (!is_btn_able(btn_apend)) {
    	if (btn_apend->loading == NULL) {
        	btn_normal_show(btn_apend);
        	cursor = gdk_cursor_new(GDK_LEFT_PTR);
            gdk_window_set_cursor (widget->window, cursor);
            gdk_cursor_unref(cursor);
        	return TRUE;
    	}
    }

    switch(event->type)
    {
        case GDK_ENTER_NOTIFY:
    		src_pixbuf = gdk_pixbuf_new_from_file("./icon/ico_hand.png",NULL);
    		cursor = gdk_cursor_new_from_pixbuf(gdk_display_get_default(), src_pixbuf, 0,0);

    		if (btn_apend->mouse_on) {
    			gtk_widget_show(btn_apend->mouse_on);
    		}

    		if (btn_apend->image) {
    			gtk_widget_hide(btn_apend->image);
    		}

            //gdk_window_set_cursor (widget->window, gdk_cursor_new(GDK_HAND1));
            /*gdk_window_set_cursor(widget->window,\
            gdk_cursor_new_from_pixbuf(gdk_display_get_default(),\
                gdk_pixbuf_new_from_file("./icon/ico_hand.png",NULL),\
                0,0)); */
            gdk_window_set_cursor(widget->window, cursor);
            g_object_unref(src_pixbuf);
            gdk_cursor_unref(cursor);
/*
    gdk_window_set_cursor(gdk_get_default_root_window(),
            gdk_cursor_new(GDK_LEFT_PTR));
    gdk_flush();
*/
            break;
        case GDK_LEAVE_NOTIFY:
        	cursor = gdk_cursor_new(GDK_LEFT_PTR);

        	if (btn_apend->mouse_on) {
        		gtk_widget_hide(btn_apend->mouse_on);
        	}

        	if (btn_apend->image) {
        		gtk_widget_show(btn_apend->image);
        	}

            //gdk_window_set_cursor (widget->window, gdk_cursor_new(GDK_LAST_CURSOR));
            gdk_window_set_cursor (widget->window, cursor);
            gdk_cursor_unref(cursor);
            break;
        default:
            break;
    }
    return FALSE;
}

gint label_mouse_handle(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    static GdkPixbuf *src_pixbuf = NULL;

    switch(event->type)
    {
        case GDK_ENTER_NOTIFY:
            src_pixbuf = gdk_pixbuf_new_from_file("./icon/ico_hand.png", NULL);
            cursor = gdk_cursor_new_from_pixbuf(gdk_display_get_default(), src_pixbuf, 0, 0);
            gdk_window_set_cursor(widget->window, cursor);
            g_object_unref(src_pixbuf);
            gdk_cursor_unref(cursor);
            break;
        case GDK_LEAVE_NOTIFY:
            cursor = gdk_cursor_new(GDK_LEFT_PTR);
            gdk_window_set_cursor (widget->window, cursor);
            gdk_cursor_unref(cursor);
            break;
        default:
            break;
    }
    return FALSE;
}

void update_widget_bg(GtkWidget *widget, const gchar *img_file, int s_width, int s_height)  
{          
    GtkStyle *style;        
    GdkPixbuf *pixbuf;          
    GdkPixmap *pixmap;        
    gint width, height;
    GdkPixbuf *dest_pixbuf = NULL;

    pixbuf = gdk_pixbuf_new_from_file(img_file, NULL);
    width = gdk_pixbuf_get_width(pixbuf);
    height = gdk_pixbuf_get_height(pixbuf);
    if ((s_width != -1) && (s_height != -1)) {
        dest_pixbuf = gdk_pixbuf_scale_simple(pixbuf, s_width, s_height, GDK_INTERP_BILINEAR);

        if (pixbuf) {
            g_object_unref(pixbuf);
        }
        width = s_width;
        height = s_height;
        pixbuf = dest_pixbuf;
    }

    pixmap = gdk_pixmap_new(NULL, width, height, 24);
    gdk_pixbuf_render_pixmap_and_mask(pixbuf, &pixmap, NULL, 0); 
    g_object_unref(pixbuf);
    style = gtk_style_copy(GTK_WIDGET (widget)->style);       
       
    if (style->bg_pixmap[GTK_STATE_NORMAL])           
        g_object_unref(style->bg_pixmap[GTK_STATE_NORMAL]);       
       
    style->bg_pixmap[GTK_STATE_NORMAL] = g_object_ref(pixmap);       
    style->bg_pixmap[GTK_STATE_ACTIVE] = g_object_ref(pixmap);  
    style->bg_pixmap[GTK_STATE_PRELIGHT] = g_object_ref(pixmap);  
    style->bg_pixmap[GTK_STATE_SELECTED] = g_object_ref(pixmap);  
    style->bg_pixmap[GTK_STATE_INSENSITIVE] = g_object_ref(pixmap);

    gtk_widget_set_style(GTK_WIDGET (widget), style);
    g_object_unref(pixmap);
    g_object_unref(style);
}

void update_widget_irregular_bg(GtkWidget *widget, const gchar *img_file, int s_width, int s_height)  
{          
    GtkStyle *style;        
    GdkPixbuf *pixbuf;          
    GdkBitmap *bitmap;          
    GdkPixmap *pixmap;        
    gint width, height;
    GdkPixbuf *dest_pixbuf;    
       
    pixbuf = gdk_pixbuf_new_from_file(img_file, NULL);
    width = gdk_pixbuf_get_width(pixbuf);
    height = gdk_pixbuf_get_height(pixbuf);
    if ((s_width != -1) && (s_height != -1)) {
        dest_pixbuf = gdk_pixbuf_scale_simple(pixbuf, s_width, s_height, GDK_INTERP_BILINEAR);

        if (pixbuf) {
            g_object_unref(pixbuf);
        }
        width = s_width;
        height = s_height;
        pixbuf = dest_pixbuf;
    }
    pixmap = gdk_pixmap_new(NULL, width, height, 24);      
    gdk_pixbuf_render_pixmap_and_mask(pixbuf, &pixmap, &bitmap, 128); 
    gtk_widget_shape_combine_mask(widget, bitmap, 0, 0);
    gdk_window_set_back_pixmap(GTK_WIDGET (widget)->window, pixmap, FALSE);
    gtk_window_set_transient_for (GTK_WINDOW (GTK_WIDGET (widget)->window), NULL);
    g_object_unref(pixbuf);
    g_object_unref(bitmap);
    style = gtk_style_copy(GTK_WIDGET (widget)->style);       
       
    if (style->bg_pixmap[GTK_STATE_NORMAL])           
        g_object_unref(style->bg_pixmap[GTK_STATE_NORMAL]);       
       
    style->bg_pixmap[GTK_STATE_NORMAL] = g_object_ref(pixmap);       
    style->bg_pixmap[GTK_STATE_ACTIVE] = g_object_ref(pixmap);  
    style->bg_pixmap[GTK_STATE_PRELIGHT] = g_object_ref(pixmap);  
    style->bg_pixmap[GTK_STATE_SELECTED] = g_object_ref(pixmap);  
    style->bg_pixmap[GTK_STATE_INSENSITIVE] = g_object_ref(pixmap);  
    gtk_widget_set_style(GTK_WIDGET (widget), style);
    g_object_unref(pixmap);
    g_object_unref(style);
}

void btn_free_event(GtkWidget *object, gpointer user_data)
{
    btn_apend_t *btn = user_data;
    g_free(btn);
}


btn_apend_t * create_button(char* icon, char* icon_mouse_on, char * icon_loading,
		char *text, int width, int height, GCallback callback, void* data)
{
    GdkPixbuf *src_pixbuf = NULL;
    GdkPixbuf *dst_pixbuf = NULL;
	btn_apend_t * btn_apend;
  	//GtkWidget * label;
  	GtkWidget * eventbox;
  	GtkWidget * fixed;
  	GtkWidget * align;

	GdkPixbufAnimation* anim_pixbuf;

    int	set_width;
    int	set_height;
    int img_width;
    int img_height;
	
	img_width = 0;
	img_height = 0;
    btn_apend = g_malloc(sizeof(btn_apend_t));

    memset(btn_apend, 0, sizeof(btn_apend_t));
    set_width = -1;
    set_height = -1;

    if(icon) {
        if (strstr(icon, ".gif") != NULL) {
            anim_pixbuf = gdk_pixbuf_animation_new_from_file(icon, NULL);
            img_width = gdk_pixbuf_animation_get_width (anim_pixbuf);
            img_height = gdk_pixbuf_animation_get_height (anim_pixbuf);
            btn_apend->image = gtk_image_new_from_animation(anim_pixbuf);
            g_object_unref(anim_pixbuf);
        } else {
                src_pixbuf = gdk_pixbuf_new_from_file(icon, NULL);

                set_width = gdk_pixbuf_get_width(src_pixbuf);
                set_height = gdk_pixbuf_get_height(src_pixbuf);
                if (width > 0) {
                    set_width = width;
                }

                if (height > 0) {
                    set_height = height;
                }

                dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, set_width, set_height, GDK_INTERP_BILINEAR);
                btn_apend->image = gtk_image_new_from_pixbuf(dst_pixbuf);
                g_object_unref(src_pixbuf);
                g_object_unref(dst_pixbuf);
            }
    }

    if (icon_mouse_on) {
        if (strstr(icon_mouse_on, ".gif") != NULL) {
        anim_pixbuf = gdk_pixbuf_animation_new_from_file(icon_mouse_on, NULL);
        img_width = gdk_pixbuf_animation_get_width (anim_pixbuf);
        img_height = gdk_pixbuf_animation_get_height (anim_pixbuf);
        btn_apend->mouse_on = gtk_image_new_from_animation(anim_pixbuf);
        g_object_unref(anim_pixbuf);
    } else {
        src_pixbuf = gdk_pixbuf_new_from_file(icon_mouse_on, NULL);
        if (set_width > 0 && set_height > 0) {
        	dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, set_width, set_height, GDK_INTERP_BILINEAR);
            btn_apend->mouse_on = gtk_image_new_from_pixbuf(dst_pixbuf);
            g_object_unref(src_pixbuf);
            g_object_unref(dst_pixbuf);
        } else {
            btn_apend->mouse_on = gtk_image_new_from_pixbuf(src_pixbuf);
            g_object_unref(src_pixbuf);
        }
    }

    }

	if (icon_loading) {
		if (strstr(icon_loading, ".gif") != NULL) {
			anim_pixbuf = gdk_pixbuf_animation_new_from_file(icon_loading, NULL);
			img_width = gdk_pixbuf_animation_get_width (anim_pixbuf);
			img_height = gdk_pixbuf_animation_get_height (anim_pixbuf);
			btn_apend->loading = gtk_image_new_from_animation(anim_pixbuf);
			g_object_unref(anim_pixbuf);
		} else {
	        src_pixbuf = gdk_pixbuf_new_from_file(icon_loading, NULL);
	        img_width = gdk_pixbuf_get_width(src_pixbuf);
	        img_height = gdk_pixbuf_get_height(src_pixbuf);
	        btn_apend->loading = gtk_image_new_from_pixbuf(src_pixbuf);
	        g_object_unref(src_pixbuf);
		}
	}

	if (text) {
		btn_apend->label = gtk_label_new(text);
	    gtk_widget_set_size_request(btn_apend->label, set_width, set_height);
	    gtk_label_set_justify(GTK_LABEL(btn_apend->label), GTK_JUSTIFY_CENTER);
	}

    fixed = gtk_fixed_new();
    if (btn_apend->image) {
    	 gtk_fixed_put(GTK_FIXED(fixed), btn_apend->image, 0, 0);
    }

    if (btn_apend->mouse_on) {
    	gtk_fixed_put(GTK_FIXED(fixed), btn_apend->mouse_on, 0, 0);
    }

	if (btn_apend->loading) {
		if (set_width > 0 && set_height > 0)	{
			gtk_fixed_put(GTK_FIXED(fixed), btn_apend->loading, (set_width-img_width)/2, (set_height-img_height)/2);
		} else {
			gtk_fixed_put(GTK_FIXED(fixed), btn_apend->loading, 0, 0);
		}
	}
    if(btn_apend->label) {
    	gtk_fixed_put(GTK_FIXED(fixed), btn_apend->label, 0, 0);
    }	
	gtk_widget_set_size_request(fixed, set_width, set_height);

    eventbox = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(eventbox),FALSE);
    gtk_widget_set_size_request(eventbox, set_width, set_height);
	gtk_container_add(GTK_CONTAINER(eventbox), fixed);
	//gtk_container_add(GTK_CONTAINER(eventbox), btn_apend->image);

	align = gtk_alignment_new(0.5,0.5,0,0);
    gtk_container_add(GTK_CONTAINER(align), eventbox);
    btn_apend->container = align;

	gtk_widget_show(fixed);
    gtk_widget_show(eventbox);
    btn_normal_show(btn_apend);

    btn_apend->event = eventbox;
    g_signal_connect(G_OBJECT(eventbox),"button_release_event",G_CALLBACK(button_press_handle), btn_apend);
    if (callback) {
    	g_signal_connect(G_OBJECT(eventbox),"button_release_event",G_CALLBACK(callback), (void*) data);
    }
    g_signal_connect(G_OBJECT(eventbox),"enter_notify_event",G_CALLBACK(button_mouse_handle), btn_apend);
    g_signal_connect(G_OBJECT(eventbox),"leave_notify_event",G_CALLBACK(button_mouse_handle), btn_apend);

    g_signal_connect (G_OBJECT(btn_apend->container), "destroy", G_CALLBACK(btn_free_event), btn_apend);
	return btn_apend;
}


static gint entry_clean(GtkWidget *widget, GdkEventButton *event, gpointer buf)
{	
	//g_print("ok  !!!!!!!!!!%p  (%s) (%s)\n", buf, gtk_entry_get_text(GTK_ENTRY(widget)), (char *)buf);
	//strcpy((char *)buf, gtk_entry_get_text(GTK_ENTRY(widget)));	
	if (g_strcmp0(gtk_entry_get_text(GTK_ENTRY(widget)), (char *)buf) == 0 )	{
		gtk_entry_set_text (GTK_ENTRY(widget), "");
	}
	
	return FALSE;
}

GtkWidget * add_entry_style(GtkWidget *entry,  char* icon,  char *text, int width, int height, gboolean visible)
{
    GdkPixbuf *src_pixbuf = NULL;
	//GtkWidget * align;
	//char * text_buf;
    int	set_width;
    int	set_height;

    set_width = width;
    set_height = height;
    if (icon != NULL) {
        src_pixbuf = gdk_pixbuf_new_from_file(icon, NULL);

    	set_height = gdk_pixbuf_get_height(src_pixbuf);
        if (height > 0) {
    		set_height = height;
        }

    	gtk_entry_set_icon_from_pixbuf (GTK_ENTRY(entry), GTK_ENTRY_ICON_PRIMARY, src_pixbuf);
        g_object_unref(src_pixbuf);
    }


	gtk_widget_set_size_request(entry, set_width, set_height);

	if (text != NULL) {
		//gtk_entry_prepend_text(GTK_ENTRY(entry), text);
		gtk_entry_set_text(GTK_ENTRY(entry), text);
	}

	gtk_entry_set_visibility(GTK_ENTRY(entry), visible);
	gtk_entry_set_invisible_char(GTK_ENTRY(entry),'*');
	
	if (text) {
		g_signal_connect(G_OBJECT(entry),"focus",G_CALLBACK(entry_clean), text);
		g_signal_connect(G_OBJECT(entry),"button_press_event",G_CALLBACK(entry_clean), text);
	}
	
	//gtk_entry_set_inner_border(GTK_ENTRY(entry), gtk_border_new());

	//align = gtk_alignment_new(0,0,0,0);
    //gtk_container_add(GTK_CONTAINER(align), entry);

	//g_signal_connect(G_OBJECT(entry),"changed",G_CALLBACK(callback), data);
    gtk_widget_show(entry);
    return entry;
}


GtkWidget * create_err_prompt(char *icon, char * text, int width, int height)
{
	GtkWidget * align;
	GtkWidget * label;
	GtkWidget * hbox;
	GtkWidget * image;
	int	set_width;
	int set_height;

	GdkPixbuf *src_pixbuf = NULL;

    src_pixbuf = gdk_pixbuf_new_from_file(icon, NULL);

	set_height = gdk_pixbuf_get_height(src_pixbuf);
    if (height > 0) {
		set_height = height;
    }
	set_width = width;

    image = gtk_image_new_from_pixbuf(src_pixbuf);
    g_object_unref(src_pixbuf);
    gtk_widget_show(image);

    label = gtk_label_new(text);
    gtk_widget_set_size_request(label, set_width, set_height);
    gtk_widget_show(label);

    hbox = gtk_hbox_new(FALSE, 3);
    gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	gtk_widget_show(hbox);

	align = gtk_alignment_new(0.5,0.5,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    //gtk_widget_show(align);

	return align;
}

void set_widget_font_size(GtkWidget *widget, int size, char * color_str)
{

    GdkColor color;

    PangoFontDescription *font;
    gchar* font_name = NULL;
    GtkSettings* settings;
    gint fontSize = size;

    if (size > 0) {
        settings = gtk_settings_get_default();
        g_object_get(settings, "gtk-font-name", &font_name, NULL);
        //g_print("font is:%s", font_name);

        font = pango_font_description_from_string(font_name);
        pango_font_description_set_size(font, fontSize*PANGO_SCALE);

        gtk_widget_modify_font(GTK_WIDGET(widget), font);
        pango_font_description_free(font);
        g_free(font_name);
    }

    //color = (0, FFFF, A5A5, 0000);
    if (color_str) {
        gdk_color_parse (color_str, &color);
        gtk_widget_modify_fg(GTK_WIDGET(widget),  GTK_STATE_NORMAL, &color);
    }

    //gtk_widget_modify_text (widget, GTK_STATE_NORMAL, &color);

    return;
}

void set_widget_font_type_size(GtkWidget *widget, char *font_name, char * color_str)
{
    GdkColor color;
    PangoFontDescription *font = NULL;

    if (font_name) {
        font = pango_font_description_from_string(font_name);
        gtk_widget_modify_font(GTK_WIDGET(widget), font);
        pango_font_description_free(font);
    }

    if (color_str) {
        gdk_color_parse (color_str, &color);
        gtk_widget_modify_fg(GTK_WIDGET(widget),  GTK_STATE_NORMAL, &color);
    }
    return;

}

gint ui_destory_dialog_callback(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    gtk_widget_destroy((GtkWidget *)data);
    return FALSE;
}

gint ui_destory_dialog_start_timer(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
	if(g_win_manager.timer_hold == 1) {
		ui_manager_publogin_timeout_enable();
		ui_manager_offline_autologin_timer_enable();
	}

    gtk_widget_destroy((GtkWidget *)data);

    return FALSE;
}


gint ui_destory_dialog_callback_notify(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    ui_extern_cancal_setting();
    return ui_destory_dialog_callback(widget, event, data);
}

gint ui_destory_wifi_config_callback_notify(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    ui_extern_cancel_wifi_config();
    return ui_destory_dialog_callback(widget, event, data);
}

GtkWidget *title_bar_init(char *title, void *data, int notify_type)
{
	GtkWidget *title_bar;
    GtkWidget *label1;
    GtkWidget *align_label;
	GtkWidget *align_image;
	//GtkWidget *toolbar;
	//GtkWidget *image;
	btn_apend_t *button_quit;

	title_bar = gtk_hbox_new(FALSE,0);
    label1 = gtk_label_new(title);
	set_widget_font_size(label1, 12, NULL);
    align_label = gtk_alignment_new(0, 1, 0, 0);
    gtk_container_add(GTK_CONTAINER(align_label), label1);
	//gtk_container_add(GTK_CONTAINER(title_bar), align_label);
	gtk_box_pack_start(GTK_BOX(title_bar), align_label, FALSE, FALSE, 10);

    if (data) {
        align_image = gtk_alignment_new(1, 0, 0, 0);
        if (notify_type == 1) {
        	button_quit = create_button("./icon/btn_close.png", "./icon/btn_close_h.png", NULL, NULL, -1, -1, G_CALLBACK(ui_destory_dialog_callback_notify), data);
        } else if (notify_type == 2) {
        	button_quit = create_button("./icon/btn_close.png", "./icon/btn_close_h.png", NULL, NULL, -1, -1, G_CALLBACK(ui_destory_dialog_start_timer), data);
        } else if (notify_type == 3) {
        	button_quit = create_button("./icon/btn_close.png", "./icon/btn_close_h.png", NULL, NULL, -1, -1, G_CALLBACK(ui_destory_wifi_config_callback_notify), data);
        } else {
           button_quit = create_button("./icon/btn_close.png", "./icon/btn_close_h.png", NULL, NULL, -1, -1, G_CALLBACK(ui_destory_dialog_callback), data);
        }
    	gtk_container_add(GTK_CONTAINER(align_image), button_quit->container);
    	gtk_container_add(GTK_CONTAINER(title_bar), align_image);
  	    gtk_widget_show(align_image);
	}

    gtk_widget_show(label1);
	gtk_widget_show(align_label);
	gtk_widget_show(title_bar);
	
	return title_bar;
}

void ui_dialog_white_background(GtkWidget *dialog)
{
    GdkColor color;
    color.red = 0xffff;
    color.green = 0xffff;
    color.blue = 0xffff;
    
    gtk_widget_modify_bg(dialog, GTK_STATE_NORMAL, &color);
}


btn_apend_t * create_button_above_label(char* icon, char* icon_mouse_on, char * icon_loading,
		char *text, GCallback callback, void* data)
{
	btn_apend_t * btn_apend;
	GtkWidget * vbox;
	GtkWidget * label;

	label = gtk_label_new(text);

	btn_apend = create_button(icon, icon_mouse_on, icon_loading, NULL, -1, -1, callback, data);

	btn_apend->label = label;
	gtk_widget_show(btn_apend->label);

    vbox = gtk_vbox_new(FALSE, 3);
    gtk_box_pack_start(GTK_BOX(vbox), btn_apend->container, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn_apend->label, FALSE, FALSE, 0);

    btn_apend->container = vbox;
	return btn_apend;
}


GtkWidget * create_prompt(char *icon, char * lab_text, int lab_font_size, char * btn_text, GCallback callback, void* data)
{
	GtkWidget * align;
	GtkWidget * label = NULL;
	GtkWidget * hbox;
	btn_apend_t * btn_cnter = NULL;
	GtkWidget * image = NULL;
	GdkPixbuf *src_pixbuf = NULL;
    GdkPixbufAnimation* anim_pixbuf = NULL;

	if (icon) {
        if (strstr(icon, ".gif") != NULL) {
			anim_pixbuf = gdk_pixbuf_animation_new_from_file(icon, NULL);
			image = gtk_image_new_from_animation(anim_pixbuf);
			g_object_unref(anim_pixbuf);
		} else {
    	    src_pixbuf = gdk_pixbuf_new_from_file(icon, NULL);
    	    image = gtk_image_new_from_pixbuf(src_pixbuf);
    	    g_object_unref(src_pixbuf);
        }
	    gtk_widget_show(image);
	}

	if (lab_text) {
	    label = gtk_label_new(lab_text);
	    set_widget_font_size(label, lab_font_size, "Snow");
	    //gtk_widget_set_size_request(label, set_width, set_height);
	    gtk_widget_show(label);
	}

	if (btn_text) {
		btn_cnter = create_button("./icon/btn_setup.png", "./icon/btn_setup_h.png", NULL, btn_text, -1, -1, (GCallback)callback, data);
		set_widget_font_size(btn_cnter->label, (lab_font_size * 2) / 3, "Snow");
	}


    hbox = gtk_hbox_new(FALSE, 5);
    if (image) {
    	gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 10);
    }
    if (label) {
    	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 8);
    }
    
    if (btn_cnter) {
    	gtk_box_pack_start(GTK_BOX(hbox), btn_cnter->container, FALSE, FALSE, 7);
    }
	
	gtk_widget_show(hbox);

	align = gtk_alignment_new(0.5,0.5,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

	return align;
}

GtkWidget * create_newdeploy_guide_bar(int type)
{
	GtkWidget * vbox;
    //GtkWidget *hide_label;
	GtkWidget *guide_bar;	
    GdkPixbuf *src_pixbuf = NULL;
    char pix_filename[64];
    
    //hide_label = gtk_label_new("");
    //gtk_widget_show(hide_label);

    switch(type)
    {
    case 1:
        strcpy(pix_filename, "./icon/newdeploy_bar1.png");
        break;
    case 2:
        strcpy(pix_filename, "./icon/newdeploy_bar2.png");
        break;
    case 3:
        strcpy(pix_filename, "./icon/newdeploy_bar3.png");
        break;
    case 4:
        strcpy(pix_filename, "./icon/newdeploy_bar4.png");
        break;
    case 5:
        strcpy(pix_filename, "./icon/newdeploy_bar5.png");
        break;
    default:
        logi("unknown guide type: %d\n", type);
        return NULL;
    }
        
    src_pixbuf = gdk_pixbuf_new_from_file(pix_filename, NULL);
	guide_bar = gtk_image_new_from_pixbuf(src_pixbuf);
	g_object_unref(src_pixbuf);
    gtk_widget_show(guide_bar);
    
	vbox = gtk_vbox_new(FALSE, 0);
	//gtk_box_pack_start(GTK_BOX(vbox), hide_label, FALSE, FALSE, 10);
	gtk_box_pack_start(GTK_BOX(vbox), guide_bar, FALSE, FALSE, 0);
	gtk_widget_show(vbox);

	return vbox;
}

