#include <gtk/gtk.h>
#include <string.h>
#include "ui_main.h"

#define	BTN_WIDTH	(150)
#define	BTN_HEIGHT	(40)

static GtkWidget *  dialog = NULL;

static btn_apend_t * single_user = NULL;
static btn_apend_t * mult_user = NULL;
static btn_apend_t * public_user = NULL;
static GtkWidget * binduser_area;
//static GtkWidget * hide_area;
static int time_sec;
static GtkWidget * times;
static guint choice_timer;
static GtkWidget * time_hbox = NULL;
static GtkWidget * usr_entry = NULL;
static int mode;
static int timer_forbiden = 0;

GtkWidget * err_prompt;
GtkWidget * err_box;

static gboolean bind_usr_empty()
{
    return (strcmp(gtk_entry_get_text(GTK_ENTRY(usr_entry)), "") == 0);
}

static void ui_dialog_settype_hide()
{
/*    gtk_widget_destroy(dialog);
    dialog = NULL;
*/
    if (time_hbox) {
    	gtk_widget_hide(time_hbox);
    }
    if (err_prompt) {
        gtk_widget_hide(err_prompt);
    }
	if (dialog) {
		gtk_widget_hide(dialog);
	}

}

static gboolean bindusr_press_event(GtkEditable *editable, gpointer user_data)
{
    //printf("bindusr_press_event\n");
    if (time_hbox) {
    	gtk_widget_hide(time_hbox);
    }
    timer_forbiden = 1;
    return FALSE;
}

static void bindusr_start_download(GtkEntry *entry, gpointer  user_data)
{
    ui_extern_new_deploy(mode, gtk_entry_get_text(GTK_ENTRY(usr_entry)));
    timer_forbiden = 1;
    return;
}


static GtkWidget *  align_center(GtkWidget * widget)
{
	GtkWidget * align;

	align = gtk_alignment_new(0.5,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), widget);
    gtk_widget_show(align);
    return align;
}


static GtkWidget * mk_binduser_area(void)
{
	GtkWidget * hbox;
	GtkWidget * label;
	GtkWidget * cntner;
	
	GdkPixbuf *src_pixbuf = NULL;
	GtkWidget *image;
	GtkWidget * fixed;
	GtkWidget * vbox;
	int width;
	int height;
	
	label = gtk_label_new("绑定用户:");
	gtk_widget_show(label);
	
	usr_entry = gtk_entry_new_with_max_length(20);
	g_signal_connect (G_OBJECT (usr_entry), "changed", G_CALLBACK (bindusr_press_event), NULL);
    g_signal_connect(G_OBJECT(usr_entry), "activate", G_CALLBACK(bindusr_start_download), NULL);
    g_signal_connect(G_OBJECT(usr_entry),"focus",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    g_signal_connect(G_OBJECT(usr_entry),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
	cntner = add_entry_style(usr_entry,  NULL,  NULL, 227, 30, TRUE);
	
	hbox = gtk_hbox_new(FALSE,10);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), cntner, FALSE, FALSE, 0);	
	
	//update_widget_bg(hbox, "./icon/user_box.png", -1, -1);
	gtk_widget_show(hbox);
	
	src_pixbuf = gdk_pixbuf_new_from_file("./icon/user_box.png", NULL);
	width = gdk_pixbuf_get_width(src_pixbuf);
	height = gdk_pixbuf_get_height(src_pixbuf);
	
	image = gtk_image_new_from_pixbuf(src_pixbuf);
	g_object_unref(src_pixbuf);	
	gtk_widget_show(image);
	
    fixed = gtk_fixed_new();
	gtk_fixed_put(GTK_FIXED(fixed), image, 0, 0);
	gtk_fixed_put(GTK_FIXED(fixed), hbox, (1*width)/5, (3*height)/8);
	gtk_widget_show(fixed);
	

	err_box = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(err_box);

	vbox = gtk_vbox_new(FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vbox), fixed, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), align_center(err_box), FALSE, FALSE, 0);
	gtk_widget_show(vbox);
	return 	vbox;
}

static void btn_selected(btn_apend_t * btn)
{
	//btn_loading_show(btn);
	if (btn->loading) {
		gtk_widget_show(btn->loading);
	}
	gtk_widget_show(btn->label);
	set_widget_font_size(btn->label, -1, "Snow");
	gtk_widget_show(btn->container);
}

static void btn_no_selected(btn_apend_t * btn)
{
	//btn_normal_show(btn);
	if (btn->loading) {
		gtk_widget_hide(btn->loading);
	}
	gtk_widget_show(btn->label);
	set_widget_font_size(btn->label, -1, "Black");
	gtk_widget_show(btn->container);
}

static void single_user_btn_handle()
{
	btn_selected(single_user);
	btn_no_selected(mult_user);
	btn_no_selected(public_user);
	
	gtk_widget_show(binduser_area);
	//gtk_widget_hide(hide_area);
    gtk_window_set_focus(GTK_WINDOW(g_win_manager.window), usr_entry);
    mode = 0;
}

static void mult_user_btn_handle()
{
	btn_selected(mult_user);
	btn_no_selected(single_user);
	btn_no_selected(public_user);
	gtk_widget_hide(binduser_area);
	//gtk_widget_show(hide_area);
	mode = 1;
}

static void public_user_btn_handle()
{
	btn_selected(public_user);
	btn_no_selected(mult_user);
	btn_no_selected(single_user);
	gtk_widget_hide(binduser_area);
	//gtk_widget_show(hide_area);
	mode = 2;
}

static gint single_user_btn_handle_cb(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    //gtk_widget_hide(time_hbox);
    if (time_hbox) {
    	gtk_widget_hide(time_hbox);
    }    
    if (err_prompt && bind_usr_empty()) {
        gtk_widget_hide(err_prompt);
    }
    timer_forbiden = 1;
    single_user_btn_handle();
    return FALSE;
}

static gint mult_user_btn_handle_cb(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    //gtk_widget_hide(time_hbox);
	if (time_hbox) {
    	gtk_widget_hide(time_hbox);
    }
    if (err_prompt && bind_usr_empty()) {
        gtk_widget_hide(err_prompt);
    }
	timer_forbiden = 1;
    mult_user_btn_handle();
    return FALSE;
}

static gint public_user_btn_handle_cb(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    //gtk_widget_hide(time_hbox);
    if (time_hbox) {
    	gtk_widget_hide(time_hbox);
    }    
    if (err_prompt && bind_usr_empty()) {
        gtk_widget_hide(err_prompt);
    }
	timer_forbiden = 1;
    public_user_btn_handle();
    return FALSE;
}

static GtkWidget * mk_btn_item(void)
{
	GtkWidget * hbox;
	
	single_user = create_button("./icon/btn_190x36.png", "./icon/btn_190x36_h.png", "./icon/btn_190x36_green.png", "单用户终端",
    		-1, -1, G_CALLBACK(single_user_btn_handle_cb), NULL);
    //btn_selected(single_user);
    
    
    mult_user = create_button("./icon/btn_190x36.png", "./icon/btn_190x36_h.png", "./icon/btn_190x36_green.png", "多用户终端",
    		-1, -1, G_CALLBACK(mult_user_btn_handle_cb), NULL);

    		
    public_user = create_button("./icon/btn_190x36.png", "./icon/btn_190x36_h.png", "./icon/btn_190x36_green.png", "公用终端",
    		-1, -1, G_CALLBACK(public_user_btn_handle_cb), NULL);		

	hbox = gtk_hbox_new(FALSE,10);
    gtk_box_pack_start(GTK_BOX(hbox), single_user->container, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), mult_user->container, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), public_user->container, FALSE, FALSE, 0);	
	gtk_widget_show(hbox);
	
	return hbox;   		
}


static GtkWidget * mk_prompt_line(char * lab_text, char * explain_text)
{

	GtkWidget * label = NULL;
	GtkWidget * explain = NULL;
	GtkWidget * hbox;
	GtkWidget * image = NULL;
	GdkPixbuf *src_pixbuf = NULL;
	

	src_pixbuf = gdk_pixbuf_new_from_file("./icon/dot.png", NULL);
	image = gtk_image_new_from_pixbuf(src_pixbuf);
	g_object_unref(src_pixbuf);
	gtk_widget_show(image);		


	label = gtk_label_new(lab_text);
	set_widget_font_size(label, -1, NULL);
	    //gtk_widget_set_size_request(label, set_width, set_height);
	gtk_widget_show(label);		

	explain = gtk_label_new(explain_text);
	set_widget_font_size(explain, -1, "Grey");
	    //gtk_widget_set_size_request(label, set_width, set_height);
	gtk_widget_show(explain);	

    hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
 	gtk_box_pack_start(GTK_BOX(hbox), explain, FALSE, FALSE, 0);
	gtk_widget_show(hbox);
	return hbox;
}
/*
static GtkWidget * hbox_cnter(GtkWidget * widget)
{
	GtkWidget * hbox;
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
	gtk_widget_show(hbox);	
	return 	hbox;
}
*/
static GtkWidget * mk_set_area(void)
{
	GtkWidget * vbox;
	GtkWidget * tip_vbox;
	GtkWidget * align;
	//GtkWidget * label;
	GtkWidget * btn_area;
	GtkWidget * single_tip;
	GtkWidget * mult_tip;
	GtkWidget * public_tip;
	GtkWidget * time_tip;
	GtkWidget * hbox;
	
	//label = gtk_label_new("选择终端类型");
	//gtk_label_set_justify(GTK_LABEL(label),  GTK_JUSTIFY_LEFT);
	//gtk_widget_show(label);
	
	btn_area = mk_btn_item();
	
	binduser_area = mk_binduser_area();

	single_tip = mk_prompt_line("单用户终端","建议使用绑定账号登录");
	mult_tip  = mk_prompt_line("多用户终端","支持多账号登录");
	public_tip = mk_prompt_line("公用终端","   无需登录,直接使用");
    time_tip = mk_prompt_line("改变终端类型，可终止倒计时","");
	
	tip_vbox = gtk_vbox_new(FALSE, 7);
	gtk_box_pack_start(GTK_BOX(tip_vbox), single_tip, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(tip_vbox), mult_tip, FALSE, FALSE, 0);	
	gtk_box_pack_start(GTK_BOX(tip_vbox), public_tip, FALSE, FALSE, 0);	
	gtk_box_pack_start(GTK_BOX(tip_vbox), time_tip, FALSE, FALSE, 0);	
	gtk_widget_show(tip_vbox);	
	
	
    vbox = gtk_vbox_new(FALSE, 0);
	//gtk_box_pack_start(GTK_BOX(vbox), hbox_cnter(label), FALSE, FALSE, 10);
	gtk_box_pack_start(GTK_BOX(vbox), btn_area, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(vbox), binduser_area, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), tip_vbox, FALSE, FALSE, 15);
	gtk_widget_show(vbox);	
	

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);	
	
	align = gtk_alignment_new(0.5,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);		
	
	return align;
}

// get type, call api,


static gboolean choice_timer_timeout(gpointer user_data)
{
    char buf[20];
    
    if (--time_sec >= 0) {
        g_snprintf (buf, 20, "%d秒", time_sec);
        gtk_label_set_text(GTK_LABEL(times), buf);
        return TRUE;
    } else {
        choice_timer = 0;
        logi("choice start!!\n");
        ui_extern_new_deploy(mode, gtk_entry_get_text(GTK_ENTRY(usr_entry)));
        //timer_forbiden = 1;
        return FALSE;
    }
}

static void choice_timer_start(GtkWidget *widget, gpointer data)
{
    if (choice_timer == 0) {
        logi("choice_timer_start\n");
        choice_timer = g_timeout_add_seconds(1, choice_timer_timeout, NULL);
    }
}

static void choice_timer_end(GtkWidget *widget, gpointer data)
{
    if (choice_timer) {
        logi("choice_timer_end\n");
        g_source_remove(choice_timer);
        choice_timer = 0;
    }
}

void ui_settype_timer_disable(void)
{
    if (time_hbox) {
    	gtk_widget_hide(time_hbox);
    }
    timer_forbiden = 1;
	choice_timer_end(NULL, NULL);
}

void ui_settype_err_prompt_hide(void)
{
    if (err_prompt && bind_usr_empty()) {
        gtk_widget_hide(err_prompt);
    }
}

static gint start_download_cb(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    ui_extern_new_deploy(mode, gtk_entry_get_text(GTK_ENTRY(usr_entry)));
    timer_forbiden = 1;
    return FALSE;
}

static GtkWidget * mk_prompt_area(void)
{
	GtkWidget * tip;
	GtkWidget * vbox;
	GtkWidget * vbox_cntner;
	GtkWidget * hbox;
	GtkWidget * align;
	GtkWidget * hide_label;

	btn_apend_t * dnw_btn = NULL;

	vbox = gtk_vbox_new(FALSE, 10);
	
	time_hbox = NULL;
//	if (timer_forbiden == 0) {
	    time_sec = 30;
		times = gtk_label_new("30秒");
		set_widget_font_size(times, 20, "LightSeaGreen");
		gtk_widget_show(times);

		tip = gtk_label_new("后自动下载镜像");
		set_widget_font_size(tip, 20, NULL);
		gtk_widget_show(tip);	
		
		time_hbox = gtk_hbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX(time_hbox), times, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(time_hbox), tip, FALSE, FALSE, 0);
	    g_signal_connect (G_OBJECT (time_hbox), "show", G_CALLBACK (choice_timer_start), NULL);
	    g_signal_connect (G_OBJECT (time_hbox), "hide", G_CALLBACK (choice_timer_end), NULL);
	    g_signal_connect (G_OBJECT (time_hbox), "destroy", G_CALLBACK (choice_timer_end), NULL);
	    //gtk_widget_show(time_hbox);
	    gtk_widget_hide(time_hbox);
		gtk_box_pack_start(GTK_BOX(vbox), time_hbox, FALSE, FALSE, 10);		
//	}

	dnw_btn = create_button("./icon/btn_220x50.png", "./icon/btn_220x50_h.png", NULL, "立即下载",
    		-1, -1, G_CALLBACK(start_download_cb), NULL);
	set_widget_font_size(dnw_btn->label, 13, NULL);
	

	gtk_box_pack_start(GTK_BOX(vbox), dnw_btn->container, FALSE, FALSE, 10);
	//gtk_box_pack_start(GTK_BOX(vbox), hide_label, FALSE, FALSE, 20);
	gtk_widget_show(vbox);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	align = gtk_alignment_new(0.5,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);		
	

	hide_label = gtk_label_new("");
	gtk_widget_set_size_request(hide_label, 850, 10);
	gtk_widget_show(hide_label);

	vbox_cntner = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_cntner), align, FALSE, FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox_cntner), hide_label, FALSE, FALSE,20);
	gtk_widget_show(vbox_cntner);		
		
    return vbox_cntner;
}


static void ui_dialog_settype_reload()
{
    int mode;
    int readonly;
    ui_extern_get_terminal_mode(&mode, &readonly);

    switch(mode) {
    case 0:
        single_user_btn_handle(NULL, NULL, NULL);
        break;
    case 1:
        mult_user_btn_handle(NULL, NULL, NULL);
        break;
    case 2:
       public_user_btn_handle(NULL, NULL, NULL);
       break;
    }

}

static int ui_dialog_settype_init(void)
{
	GtkWidget * guide_area;
	GtkWidget * set_area;
	GtkWidget * prompt_area;
	GtkWidget *title_bar;
	//GtkWidget *hide_label;
	
     /* Create the widgets */
    dialog = gtk_dialog_new();
    g_signal_connect (G_OBJECT (dialog), "realize",
                      G_CALLBACK (ui_compt_realize), &ui_dialog_settype);

    ui_dialog_white_background(dialog);

    title_bar = title_bar_init("配置向导", NULL, 1);
    guide_area = create_newdeploy_guide_bar(3);
    set_area = mk_set_area();
	prompt_area = mk_prompt_area();
	//hide_area = gtk_label_new("");
	//gtk_widget_hide(hide_area);
	
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), title_bar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), guide_area, FALSE, FALSE, 30);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), set_area, FALSE, FALSE, 0);
	
	//gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hide_area, FALSE, FALSE, 25);
	//gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), prompt_area, FALSE, FALSE, 30);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->action_area), prompt_area, FALSE, FALSE, 0);

    //hide_label = gtk_label_new("");
    //gtk_widget_show(hide_label);
    //gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hide_label, FALSE, FALSE, 10);

    gtk_window_set_decorated(GTK_WINDOW(dialog),FALSE);
    //gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_widget_set_size_request(dialog, 850, 580);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);

    //ui_dialog_settype_reload();
/*    if (timer_forbiden == 0) {
    	timer_forbiden = 1;
    }
*/
    
    return 0;
}

static void ui_dialog_settype_show(void)
{
/*
    if (dialog != NULL) {
        gtk_widget_destroy(dialog);
    }

	ui_dialog_settype_init();
	gtk_widget_show(dialog);
*/
	if (dialog == NULL) {
		ui_dialog_settype_init();
	}
	if (timer_forbiden != 1) {
		time_sec = 30;
		gtk_label_set_text(GTK_LABEL(times), "30秒");
		gtk_widget_show(time_hbox);
	}

	if (usr_entry) {
		gtk_entry_set_text(GTK_ENTRY(usr_entry), "");
		//gtk_editable_set_editable(GTK_EDITABLE(GTK_ENTRY(usr_entry)), TRUE);
        gtk_widget_set_sensitive(usr_entry, TRUE);
	}
	ui_dialog_settype_reload();

	gtk_widget_show(dialog);
}

static int ui_dialog_settype_ctrl(void *data)
{

	ui_msg_t *msg = data;

    if (err_prompt) {
        gtk_widget_destroy(err_prompt);
        err_prompt = NULL;
    }
	switch (msg->sub_obj) {
	case UI_WIN_SETTYPE_USER_IVALID:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "用户名不存在或用户无IDV权限，绑定失败", -1, -1);
        gtk_box_pack_start(GTK_BOX (err_box), err_prompt, FALSE, FALSE, 0);
        break;
	case UI_WIN_SETTYPE_USER_BINDED:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "用户已在其它终端绑定,无法再绑定", -1, -1);
        gtk_box_pack_start(GTK_BOX (err_box), err_prompt, FALSE, FALSE, 0);
        break;
	case UI_WIN_SETTYPE_TERM_BINDED:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "终端已被其他用户绑定，无法绑定，请尝试重启终端", -1, -1);
        gtk_box_pack_start(GTK_BOX (err_box), err_prompt, FALSE, FALSE, 0);
        break;
	case UI_WIN_SETTYPE_FIX_USER:
		if (msg->args) {
			gtk_entry_set_text(GTK_ENTRY(usr_entry), (gchar *)(((ui_string_arg_t *)(msg->args))->str));
			//gtk_editable_set_editable(GTK_EDITABLE(GTK_ENTRY(usr_entry)), FALSE);
            gtk_widget_set_sensitive(usr_entry, FALSE);
			//err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "用户已经绑定终端,要解绑请和管理员联系", -1, -1);
	        //gtk_box_pack_start(GTK_BOX (err_box), err_prompt, FALSE, FALSE, 0);
		}
        break;

	}

	if (err_prompt) {
		gtk_widget_show(err_prompt);
	}

	return 0;
}

static void ui_dialog_settype_adapt(void)
{
    if (dialog) {
        if (ui_dialog_settype.status == UI_STATUS_SHOW) {
            logi("ui_dialog_settype_adapt\n");
             gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
            if (ui_dialog_config.status == UI_STATUS_SHOW) {
                // if newdeploy and show config dialog then should hide settype
                gtk_widget_hide(dialog);
            }
        }
    }
}

static void ui_dialog_settype_destroy(void)
{
    if (dialog) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
}

struct ui_comp_s ui_dialog_settype = {
    .type = UI_TYPE_DIALOG_SETTYPE,
    .subtype = UI_TOPLEVEL_TYPE_DIALOG,
    .is_init = UI_NOT_NEED_INIT,
    .widget = &dialog,
    .init = ui_dialog_settype_init,
    .show = ui_dialog_settype_show,
    .hide = ui_dialog_settype_hide,
    .ctrl = ui_dialog_settype_ctrl,
    .adapt = ui_dialog_settype_adapt,
    .destroy = ui_dialog_settype_destroy,
};
