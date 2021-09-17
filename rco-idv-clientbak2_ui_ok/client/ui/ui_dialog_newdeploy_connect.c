#include "ui_main.h"
#include "ui_dialog_newdeploy_connect.h"

#define	BTN_WIDTH	(150)
#define	BTN_HEIGHT	(40)

static int mode = 0;
static GtkWidget *  dialog = NULL;

static void ui_dialog_newdeploy_connect_hide()
{
	if (dialog) {
		gtk_widget_hide(dialog);
	}
}

static GtkWidget * mk_connect_area(const char *filename)
{
	GtkWidget *connect_area;	
    GdkPixbuf *src_pixbuf = NULL;
        
    src_pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
	connect_area = gtk_image_new_from_pixbuf(src_pixbuf);
	g_object_unref(src_pixbuf);
    gtk_widget_show(connect_area);

	return connect_area;

}

static gint ui_newdeploy_show_config(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    //ui_tab[UI_TYPE_DIALOG_CONFIG]->show();
    ui_tab[UI_TYPE_DIALOG_ADMINPWD]->show();
	return FALSE;
}

static gint ui_newdeploy_show_wifi_config(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    ui_tab[UI_TYPE_DIALOG_WIFI]->show();
	return FALSE;
}

static GtkWidget * mk_tip_connect_fail()
{
	GtkWidget *label = NULL;

    label = gtk_label_new("请检查终端有线网络连接是否正常");
    set_widget_font_size(label, 13, "#666666");
	gtk_widget_show(label);

	return label;
}

static GtkWidget * mk_tip_connect_fail_wifi_terminal()
{
	GtkWidget *align = NULL;
	GtkWidget *hbox = NULL;
	GtkWidget *label = NULL;
	GtkWidget *label_wifi = NULL;
    GtkWidget * eventbox;

    label = gtk_label_new("请检查终端有线网络连接是否正常或者");
    set_widget_font_size(label, 13, "#666666");
	gtk_widget_show(label);
        
    label_wifi = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_wifi), "<span underline='single'>连接无线网络</span>");
    set_widget_font_size(label_wifi, 13, "#3ECACB");
    
	eventbox = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(eventbox),FALSE);
	gtk_container_add(GTK_CONTAINER(eventbox), label_wifi);
    g_signal_connect(GTK_OBJECT(eventbox), "button_release_event", G_CALLBACK(ui_newdeploy_show_wifi_config), NULL);
    g_signal_connect(G_OBJECT(eventbox),"enter_notify_event",G_CALLBACK(label_mouse_handle), NULL);
    g_signal_connect(G_OBJECT(eventbox),"leave_notify_event",G_CALLBACK(label_mouse_handle), NULL);
    
	gtk_widget_show(label_wifi);
	gtk_widget_show(eventbox);

    hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), eventbox, FALSE, FALSE, 0); 
	gtk_widget_show(hbox);
    
    align = gtk_alignment_new(0.5,0.5,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

	return align;
}
/*
static GtkWidget * mk_tip_connect_eth()
{
	GtkWidget *label = NULL;

    label = gtk_label_new("网络连接成功，请配置云主机和网络IP地址");
    set_widget_font_size(label, 13, "#666666");
	gtk_widget_show(label);

	return label;
}
*/
static GtkWidget * mk_tip_connect_config_server()
{
    GtkWidget *align = NULL;
    GtkWidget *hbox = NULL;
    GtkWidget *label = NULL;
    GtkWidget *label_config = NULL;
    GtkWidget * eventbox;

    label = gtk_label_new("云主机连接异常，");
    set_widget_font_size(label, 13, "#666666");
    gtk_widget_show(label);
        
    label_config = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_config), "<span underline='single'>请配置云主机</span>");
    set_widget_font_size(label_config, 13, "#3ECACB");
    
    eventbox = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(eventbox),FALSE);
    gtk_container_add(GTK_CONTAINER(eventbox), label_config);
    g_signal_connect(GTK_OBJECT(eventbox), "button_release_event", G_CALLBACK(ui_newdeploy_show_config), NULL);
    g_signal_connect(G_OBJECT(eventbox),"enter_notify_event",G_CALLBACK(label_mouse_handle), NULL);
    g_signal_connect(G_OBJECT(eventbox),"leave_notify_event",G_CALLBACK(label_mouse_handle), NULL);
    
    gtk_widget_show(label_config);
    gtk_widget_show(eventbox);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), eventbox, FALSE, FALSE, 0); 
    gtk_widget_show(hbox);
    
    align = gtk_alignment_new(0.5,0.5,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

    return align;
}

static GtkWidget * mk_prompt_area(void)
{
	GtkWidget * vbox;
	GtkWidget * vbox_net;
	GtkWidget * guide_area = NULL;
	GtkWidget * connect_area = NULL;
	GtkWidget * tip_area = NULL;
    int wifi_terminal = 0;
    
    switch (mode)
    {
    case UI_DIALOG_NEWDEPLOY_CONNECT_FAIL:
        guide_area = create_newdeploy_guide_bar(1);
        connect_area = mk_connect_area("./icon/connect_fail.png");

        ui_extern_is_wifi_terminal(&wifi_terminal);
        if (wifi_terminal) {
	        tip_area = mk_tip_connect_fail_wifi_terminal(); 
        } else {            
	        tip_area = mk_tip_connect_fail(); 
        }
        break;   
    //case UI_DIALOG_NEWDEPLOY_CONNECT_ETH:
    //    guide_area = create_newdeploy_guide_bar(2);
    //    connect_area = mk_connect_area("./icon/connect_eth.png");
	//    tip_area = mk_tip_connect_wifi(); 
    //    break;
    case UI_DIALOG_NEWDEPLOY_CONNECT_CONFIG_SERVER:
        guide_area = create_newdeploy_guide_bar(2);
        connect_area = mk_connect_area("./icon/connect_server_fail.png");
	    tip_area = mk_tip_connect_config_server(); 
        break;
    }   

    vbox = gtk_vbox_new(FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), guide_area, FALSE, FALSE, 30);

    vbox_net = gtk_vbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(vbox_net), connect_area, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_net), tip_area, FALSE, FALSE, 15);
    gtk_widget_show(vbox_net);
	gtk_box_pack_start(GTK_BOX(vbox), vbox_net, FALSE, FALSE, 100);    

    gtk_widget_show(vbox);
    return vbox;
}

static int ui_dialog_newdeploy_connect_init(void)
{
	GtkWidget *title_bar;
	GtkWidget *prompt_area;
	
     /* Create the widgets */
    dialog = gtk_dialog_new();
    g_signal_connect (G_OBJECT (dialog), "realize",
                      G_CALLBACK (ui_compt_realize), &ui_dialog_newdeploy_connect);
    
    gtk_widget_set_size_request(dialog, 850, 580);

    ui_dialog_white_background(dialog);
    title_bar = title_bar_init("配置向导", NULL, 1);
	prompt_area = mk_prompt_area();
	
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), title_bar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (GTK_DIALOG (dialog)->vbox), prompt_area, FALSE, FALSE, 0);

    gtk_window_set_decorated(GTK_WINDOW(dialog),FALSE);
    //gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
    return 0;
}

static void ui_dialog_newdeploy_connect_show(void)
{
    if (dialog) {
        gtk_widget_show(dialog);
    }
}

static int ui_dialog_newdeploy_connect_ctrl(void *p)
{
	ui_msg_t *msg = p;
    int old_mode = mode;

    mode = msg->sub_obj;

    if (dialog && mode != old_mode) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
    if (dialog == NULL) {
        ui_dialog_newdeploy_connect_init();
    }
	return 0;
}


void ui_dialog_newdeploy_connect_adapt(void)
{
    if (dialog) {
        if (ui_dialog_newdeploy_connect.status == UI_STATUS_SHOW) {
            logi("ui_dialog_newdeploy_connect_adapt\n");
            gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
        }
    }
}

static void ui_dialog_newdeploy_connect_destroy(void)
{
    if (dialog) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
}

struct ui_comp_s ui_dialog_newdeploy_connect = {
    .type = UI_TYPE_DIALOG_NEWDEPLOY_CONNECT,
    .subtype = UI_TOPLEVEL_TYPE_DIALOG,
    .is_init = UI_NEED_INIT,
    .widget = &dialog,
    .init = ui_dialog_newdeploy_connect_init,
    .show = ui_dialog_newdeploy_connect_show,
    .hide = ui_dialog_newdeploy_connect_hide,
    .ctrl = ui_dialog_newdeploy_connect_ctrl,
    .adapt = ui_dialog_newdeploy_connect_adapt,
    .destroy = ui_dialog_newdeploy_connect_destroy,
};
