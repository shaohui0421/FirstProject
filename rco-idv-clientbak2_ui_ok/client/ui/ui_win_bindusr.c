#include <gdk/gdkkeysyms.h>
#include "ui_main.h"


static btn_apend_t * btn_bind;
static btn_apend_t * btn_nobind;
static GtkWidget * vbox_all;
static GtkWidget * bg_image;
static GtkWidget * err_prompt = NULL;
static GtkWidget * vbox_err;
static int width;


static gint ui_win_bindusr_event_ok(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    ui_extern_bind_usr(1);
    return FALSE;
}

static gint ui_win_bindusr_event_cancal(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    ui_extern_bind_usr(0);
    return FALSE;
}

static GtkWidget * ui_win_bindusr_mk_top_title(char * text, int width, int height)
{
	GtkWidget *title;	
	
    title = gtk_label_new(text);
	gtk_label_set_justify(GTK_LABEL(title), GTK_JUSTIFY_CENTER);
	gtk_widget_set_size_request(title, width, height);
	gtk_widget_show(title);
	
	return title;	
}

static gint page_deal_key_event(GtkWidget *widget,GdkEventKey *event, gpointer data)
{
	switch(event->keyval) {
	case GDK_Return:
			if (ui_get_current_show_widget() == UI_TYPE_WIN_BINDUSR) {
				ui_extern_bind_usr(1);
			}
	        break;
	}
    return FALSE;
}

static GtkWidget * ui_win_bindusr_mk_action_area()
{
	GtkWidget *hbox;	
	GtkWidget * vbox;
	GtkWidget *align;
	
    btn_bind = create_button("./icon/btn_green.png", "./icon/btn_green_h.png", NULL, "立即绑定", -1, -1, G_CALLBACK(ui_win_bindusr_event_ok), NULL);
    set_widget_font_size(btn_bind->label, 11, "Snow");
    
    gtk_widget_grab_focus(btn_bind->event);
    g_signal_connect(G_OBJECT(g_win_manager.window),"key-press-event",G_CALLBACK(page_deal_key_event), NULL);

	btn_nobind = create_button("./icon/btn_gray.png", "./icon/btn_gray_h.png", NULL, "取消", -1, -1, G_CALLBACK(ui_win_bindusr_event_cancal), NULL);
	set_widget_font_size(btn_nobind->label, 11, NULL);
    hbox = gtk_hbox_new(FALSE, 10);
    gtk_box_pack_start(GTK_BOX(hbox), btn_bind->container, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), btn_nobind->container, FALSE, FALSE, 0);	
    
    gtk_widget_show(hbox);

	vbox_err = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox_err);
	
	vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), vbox_err, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 10);	
    
    gtk_widget_show(vbox);
    
	align = gtk_alignment_new(0.5,0.5,0,0);
    gtk_container_add(GTK_CONTAINER(align), vbox);
    gtk_widget_show(align);

        
    return align;
}


static GtkWidget * mk_prompt(char *icon, char * lab_text, int lab_font_size)
{
	GtkWidget * align;
	GtkWidget * label = NULL;
	GtkWidget * hbox;
	GtkWidget * image = NULL;
	GdkPixbuf *src_pixbuf = NULL;
	
	if (icon) {
	    src_pixbuf = gdk_pixbuf_new_from_file(icon, NULL);
	    image = gtk_image_new_from_pixbuf(src_pixbuf);
	    g_object_unref(src_pixbuf);
	    gtk_widget_show(image);		
	}

	if (lab_text) {
	    label = gtk_label_new(lab_text);
	    set_widget_font_size(label, lab_font_size, NULL);
	    //gtk_widget_set_size_request(label, set_width, set_height);
	    gtk_widget_show(label);		
	}

    hbox = gtk_hbox_new(FALSE, 5);
    if (image) {
    	gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 10);
    }
    if (label) {
    	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 8);
    }
	
	gtk_widget_show(hbox);

	align = gtk_alignment_new(0.5,0.5,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

	return align;
}



static int ui_win_bindusr_ctrl(void *data)
{
	ui_msg_t *msg = data;
	
    if (err_prompt) {
        gtk_widget_destroy(err_prompt);
        err_prompt = NULL;
    }
	switch (msg->sub_obj) {
	case UI_WIN_BINDUSR_RET_ERROR:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "绑定用户失败", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
       // g_print("xxxxxxxxxxxxxxx\n");
        break;

	}
    //able_btn_widget();
    //do_login_err();
	gtk_widget_show(err_prompt);
	return 0;
}


static int ui_win_bindusr_init(void)
{
    GtkWidget * prompt;
    GtkWidget * tilte;
    GtkWidget * action_area;
    GtkWidget * vbox_up;
    GdkPixbuf * pixbuf = NULL;
    GtkWidget * vbox_line;
    GtkWidget * line1;
    GtkWidget * line2;
//    GtkWidget * err_prompt;
    
    tilte = ui_win_bindusr_mk_top_title("绑定终端", -1, -1);
    set_widget_font_size(tilte, 12, NULL);
    
    //set_widget_font_size();
    
    //prompt = create_err_prompt("./icon/ico_prompt_y.png", "需要将账号绑定终端", -1, -1);
    prompt = mk_prompt("./icon/ico_prompt_y.png", "需要将账号绑定终端", 14);
    
    line1 = mk_prompt(NULL, "绑定后将变成该账号的专用终端", 10);
    //line2 = mk_prompt(NULL, "其他账号无法登陆", 10);
    line2 = mk_prompt(NULL, "", 10);
    
    vbox_line = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_line), line1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_line), line2, FALSE, FALSE, 0);	
	gtk_widget_show(vbox_line);
    
    //gtk_widget_show(prompt);
	
	action_area = ui_win_bindusr_mk_action_area();
	
    vbox_up = gtk_vbox_new(FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbox_up), tilte, FALSE, FALSE, 20);
    gtk_box_pack_start(GTK_BOX(vbox_up), prompt, FALSE, FALSE, 10);	
    
    gtk_box_pack_start(GTK_BOX(vbox_up), vbox_line, FALSE, FALSE, 10);	
	gtk_widget_show(vbox_up);

	
    vbox_all = gtk_vbox_new(FALSE, 80);

    gtk_box_pack_start(GTK_BOX(vbox_all), vbox_up, FALSE, FALSE, 25);	
    
    //gtk_box_pack_start(GTK_BOX(vbox_all), vbox_err, FALSE, FALSE, 5);	
	gtk_box_pack_start(GTK_BOX(vbox_all), action_area, FALSE, FALSE, 0);	
	
    g_signal_connect (G_OBJECT (vbox_all), "realize",
                      G_CALLBACK (ui_compt_realize), &ui_win_bindusr);	
    if (bg_image == NULL) {
        pixbuf = gdk_pixbuf_new_from_file("./icon/login_box.png", NULL);

        width = gdk_pixbuf_get_width(pixbuf);
        bg_image = gtk_image_new_from_pixbuf(pixbuf);
        g_object_unref(pixbuf);
        gtk_widget_hide(bg_image);
        ui_win_put_nice_position(bg_image, 0, width, 25);
    }

    gtk_widget_hide(vbox_all);
    ui_win_set_pos(vbox_all, 0, 0);

	return 0;
}


static void ui_win_bindusr_show(void)
{
    gtk_widget_show(bg_image);
    gtk_widget_show(vbox_all);
    if (err_prompt) {
    	gtk_widget_destroy(err_prompt);
    	err_prompt = NULL;
    }

}

static void ui_win_bindusr_hide(void)
{
    gtk_widget_hide(bg_image);
	gtk_widget_hide(vbox_all);
}

static void ui_win_binduser_adapt(void)
{
    if (bg_image) {
        ui_win_put_nice_position(bg_image, 1, width, 25);
    }

    if (vbox_all) {
        ui_win_bindusr.connect_id = g_signal_connect(G_OBJECT (vbox_all), "size-allocate", 
                                                    G_CALLBACK(ui_win_put_nice_position_cb), &ui_win_bindusr);
    }

    if(ui_win_bindusr.status == UI_STATUS_SHOW) {
        if (bg_image && vbox_all) {
            gtk_widget_hide(bg_image);
            gtk_widget_hide(vbox_all);
            gtk_widget_show(bg_image);
            gtk_widget_show(vbox_all);
        }
    }
}

static void ui_win_binduser_destroy(void)
{
     if (bg_image) {
        gtk_widget_destroy(bg_image);
        bg_image = NULL;
     }

     if (vbox_all) {
        gtk_widget_destroy(vbox_all);
        vbox_all = NULL;
     }
}

struct ui_comp_s ui_win_bindusr = {
    .type = UI_TYPE_WIN_BINDUSR,
    .subtype = UI_TOPLEVEL_TYPE_WIN,
    .is_init = UI_NEED_INIT,
    .widget = &vbox_all,
    .init = ui_win_bindusr_init,
    .show = ui_win_bindusr_show,
    .hide = ui_win_bindusr_hide,
    .ctrl = ui_win_bindusr_ctrl,
    .adapt = ui_win_binduser_adapt,
     .destroy = ui_win_binduser_destroy,
    .height = 25,
};



