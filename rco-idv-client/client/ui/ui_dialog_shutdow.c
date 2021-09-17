#include <gtk/gtk.h>
#include "ui_main.h"

static GtkWidget *dialog;
static GtkWidget *combo;

static void ui_dialog_shutdown_ok(GtkWidget *widget,gpointer data)
{
    ui_extern_shutdown(gtk_combo_box_get_active(GTK_COMBO_BOX(combo)));

    if (dialog) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
}

static GtkWidget *mk_title_bar(char *title, void *data)
{
	GtkWidget *title_bar;
    GtkWidget *label1;
	btn_apend_t *button_quit;

	title_bar = gtk_hbox_new(FALSE,0);

    label1 = gtk_label_new(title);
    set_widget_font_size(label1, 12, NULL);
    gtk_misc_set_alignment(GTK_MISC(label1), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(title_bar), label1, FALSE, FALSE, 10);

    button_quit = create_button("./icon/btn_close.png", "./icon/btn_close_h.png", NULL, NULL, -1, -1, G_CALLBACK(ui_destory_dialog_start_timer), data);
    gtk_box_pack_end(GTK_BOX(title_bar), button_quit->container, FALSE, FALSE, 5);

    gtk_widget_show(label1);
	gtk_widget_show(title_bar);

	return title_bar;
}

static GtkWidget * mk_logo()
{
	GtkWidget * align;
	GtkWidget *image;
    GdkPixbuf *src_pixbuf = NULL;

	src_pixbuf = gdk_pixbuf_new_from_file("./icon/about_logo.png", NULL);

    image = gtk_image_new_from_pixbuf(src_pixbuf);
    g_object_unref(src_pixbuf);
    gtk_widget_show(image);

	align = gtk_alignment_new(0.5,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), image);
    gtk_widget_show(align);
	return align;
}

static GtkWidget * mk_action_area()
{
	GtkWidget * align;
	GtkWidget * btn_hbox;
	btn_apend_t * okay_button;
	btn_apend_t * cancal_button;

    okay_button = create_button("./icon/btn_green.png", "./icon/btn_green_h.png", NULL, "确定",
    		-1 ,-1, G_CALLBACK(ui_dialog_shutdown_ok), NULL);
	set_widget_font_size(okay_button->label, 13, "Snow");
	
    cancal_button = create_button("./icon/btn_gray.png", "./icon/btn_gray_h.png", NULL, "取消",
    		-1, -1, G_CALLBACK(ui_destory_dialog_start_timer), dialog);
    set_widget_font_size(cancal_button->label, 13, NULL);

    btn_hbox = gtk_hbox_new(FALSE,15);
    gtk_box_pack_start(GTK_BOX(btn_hbox), okay_button->container, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(btn_hbox), cancal_button->container, FALSE, FALSE, 0);
    gtk_widget_show(btn_hbox);

	align = gtk_alignment_new(0.5,0.5,0,0);
    gtk_container_add(GTK_CONTAINER(align), btn_hbox);
    gtk_widget_show(align);
        
    return align;
}

static GtkWidget * mk_shutdow_prompt()
{
	GtkWidget * align;
	GtkWidget * hbox;
	GtkWidget * label;
	
	label = gtk_label_new("您希望：");
	set_widget_font_size(label, 13, NULL);
	gtk_widget_show(label);
	
    combo = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo),"关机");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo),"重启");
    gtk_combo_box_set_active (GTK_COMBO_BOX(combo), 0);
    gtk_widget_set_size_request(combo, 220, 30);
    gtk_widget_show(combo);
    	
    hbox = gtk_hbox_new(FALSE,2);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), combo, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

	align = gtk_alignment_new(0.5,0.5,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);
        
    return align;
}


static int ui_dialog_shutdown_init(void)
{
	GtkWidget *title_bar;
	GtkWidget *logo;
	GtkWidget * hide_label;
	GtkWidget * prompt;
	GtkWidget * action_area;
    int space;

    dialog = gtk_dialog_new();
    
    g_signal_connect (G_OBJECT (dialog), "realize",
                      G_CALLBACK (ui_compt_realize), &ui_dialog_shutdown);

    gtk_widget_set_size_request(dialog, 460 ,310);

    if((ui_tab[UI_TYPE_DIALOG_NEWDEPLOY_CONNECT]->status != UI_STATUS_SHOW)
            && (ui_tab[UI_TYPE_DIALOG_SETTYPE]->status != UI_STATUS_SHOW)) {
        logi("l2u_show_wifi_status: show\n");
        ui_dialog_white_background(dialog);
        title_bar = title_bar_init("电源选项", dialog, 2);
        space = 0;
    } else {
        logi("l2u_show_wifi_status: show\n");
        update_widget_bg(dialog, "./icon/wireless/frame_wifi_verify.png", -1, -1);
        title_bar = mk_title_bar("电源选项", dialog);
        space = 5;
    }

    logo = mk_logo();

    prompt = mk_shutdow_prompt();

	action_area = mk_action_area();

	hide_label = gtk_label_new("");
	gtk_widget_show(hide_label);

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), title_bar, FALSE, FALSE, space);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), logo, FALSE, FALSE, 15);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), prompt, FALSE, FALSE, 15);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hide_label, FALSE, FALSE, 15);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), action_area, FALSE, FALSE, 15);

    gtk_window_set_decorated(GTK_WINDOW(dialog),FALSE);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
    return 0;
}

static void ui_dialog_shutdown_show(void)
{
    if (dialog != NULL) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }     
    ui_dialog_shutdown_init();
    gtk_widget_show(dialog);
}

static void ui_dialog_shutdown_hide(void)
{
    gtk_combo_box_popdown(GTK_COMBO_BOX(combo));
    if (dialog != NULL) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
}

static void ui_dialog_shutdown_adapt(void)
{
    if (dialog) {
        if(ui_dialog_shutdown.status == UI_STATUS_SHOW) { 
            logi("ui_dialog_shutdown_adapt\n");
            gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
        }
    }
}

static void ui_dialog_shutdown_destroy(void)
{
     if (dialog != NULL) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
}

struct ui_comp_s ui_dialog_shutdown = {
    .type = UI_TYPE_DIALOG_SHUTDOWN,
    .subtype = UI_TOPLEVEL_TYPE_DIALOG,
    .is_init = UI_NEED_INIT,
    .widget = &dialog,
    .init = ui_dialog_shutdown_init,
    .show = ui_dialog_shutdown_show,
    .hide = ui_dialog_shutdown_hide,
    .adapt = ui_dialog_shutdown_adapt,
    .destroy = ui_dialog_shutdown_destroy,
};

