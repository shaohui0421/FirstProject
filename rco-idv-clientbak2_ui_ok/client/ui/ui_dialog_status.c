#include "ui_main.h"

#define	BTN_WIDTH	(150)
#define	BTN_HEIGHT	(40)

static int mode;
static GtkWidget *  dialog = NULL;

static void ui_dialog_status_hide()
{
	if (dialog) {
		gtk_widget_hide(dialog);
	}
}

static GtkWidget * mk_prompt_newdeploy_finish()
{
    GtkWidget * vbox;
    GtkWidget * guide_area;
    GtkWidget * align;
    GtkWidget * hbox;
	GtkWidget * image = NULL;
	GtkWidget * label = NULL;
	GdkPixbuf * src_pixbuf = NULL;

    guide_area = create_newdeploy_guide_bar(5);
	
    src_pixbuf = gdk_pixbuf_new_from_file("./icon/ico_correct_40x40.png", NULL);
    image = gtk_image_new_from_pixbuf(src_pixbuf);
    g_object_unref(src_pixbuf);
    gtk_widget_show(image);	

    label = gtk_label_new("恭喜您！配置成功！");
    set_widget_font_size(label, 20, "#666666");
    gtk_widget_show(label);

    hbox = gtk_hbox_new(FALSE, 20);
	gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);   
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0); 
	gtk_widget_show(hbox);

    align = gtk_alignment_new(0.5,0.5,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

    vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), guide_area, FALSE, FALSE, 30);   
	gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 120); 
	gtk_widget_show(vbox);

    return vbox;
}

static int ui_dialog_status_init(void)
{
	GtkWidget *title_bar = NULL;
	GtkWidget *prompt = NULL;
	
     /* Create the widgets */
    dialog = gtk_dialog_new();
    g_signal_connect (G_OBJECT (dialog), "realize",
                      G_CALLBACK (ui_compt_realize), &ui_dialog_status);
    
    gtk_widget_set_size_request(dialog, 850, 580);
    ui_dialog_white_background(dialog);
    gtk_window_set_decorated(GTK_WINDOW(dialog),FALSE);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

    switch (mode) {
    case UI_DIALOG_STATUS_NEWDEPLOY_FINISH:
        title_bar = title_bar_init("配置向导", NULL, 1);
        prompt = mk_prompt_newdeploy_finish();
        break;
    }

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), title_bar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX (GTK_DIALOG (dialog)->vbox), prompt, FALSE, FALSE, 0);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
    return 0;
}

static void ui_dialog_status_show(void)
{
	gtk_widget_show(dialog);
}

static int ui_dialog_status_ctrl(void *p)
{
	ui_msg_t *msg = p;

    mode = msg->sub_obj;
    if (dialog != NULL) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
	ui_dialog_status_init();
	return 0;
}

void ui_dialog_status_adapt(void)
{
    if (dialog) {
        if (ui_dialog_status.status == UI_STATUS_SHOW) {
            logi("ui_dialog_status_adapt\n");
            gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
        }
    }
}

static void ui_dialog_status_destroy(void)
{
    if (dialog) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
}

struct ui_comp_s ui_dialog_status = {
    .type = UI_TYPE_DIALOG_STATUS,
    .subtype = UI_TOPLEVEL_TYPE_DIALOG,
    .is_init = UI_NEED_INIT,
    .widget = &dialog,
    .init = ui_dialog_status_init,
    .show = ui_dialog_status_show,
    .hide = ui_dialog_status_hide,
    .ctrl = ui_dialog_status_ctrl,
    .destroy = ui_dialog_status_destroy,
    .adapt = ui_dialog_status_adapt,
};
