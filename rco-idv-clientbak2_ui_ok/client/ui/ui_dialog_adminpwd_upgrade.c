#include <gtk/gtk.h>
#include <string.h>
#include "ui_main.h"
#include "ui_dialog_adminpwd.h"

#define lhw_test 1

static btn_apend_t * okay_button;
static btn_apend_t * cancal_button;
static GtkWidget *dialog, *label_tips, *entry_pwd; //*hbox, *label;
static int verified = 0;

static void ui_dialog_adminpwd_upgrade_destroy(void);

static void ui_dialog_adminpwd_upgrade_hide()
{
    gtk_widget_hide(dialog);
}


static void ui_dialog_admin_verify(GtkWidget *widget,gpointer data)
{
#ifdef lhw_test

    const gchar *pwd;

    pwd = gtk_entry_get_text(GTK_ENTRY(entry_pwd));
    if (ui_extern_verify_adminpwd((char*)pwd) == 0) {
        ui_dialog_adminpwd_upgrade_destroy();
        ui_extern_upgrade_for_class();
    } else {
        gtk_widget_show(label_tips);
    }
    
#endif
}

static GtkWidget * mk_action_area()
{
    //GtkWidget * tbl;
    GtkWidget * btn_hbox;
    GtkWidget * align;
    GdkPixbuf * src_pixbuf = NULL;
    GdkPixbuf * dst_pixbuf = NULL;
    GtkWidget * image;
    GtkWidget * vbox;

    okay_button = create_button("./icon/btn_green.png", "./icon/btn_green_h.png", NULL, "确定",
            -1 ,-1, G_CALLBACK (ui_dialog_admin_verify), NULL);
    cancal_button = create_button("./icon/btn_gray.png", "./icon/btn_gray_h.png", NULL, "取消",
            -1, -1, G_CALLBACK (ui_destory_dialog_callback), dialog);

    btn_hbox = gtk_hbox_new(FALSE,15);
    gtk_box_pack_start(GTK_BOX(btn_hbox), okay_button->container, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(btn_hbox), cancal_button->container, FALSE, FALSE, 0);
    gtk_widget_show(btn_hbox);

    align = gtk_alignment_new(0.5,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), btn_hbox);
    gtk_widget_show(align);
    
    src_pixbuf = gdk_pixbuf_new_from_file("./icon/shadow.png", NULL);
    dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, 840, 10, GDK_INTERP_BILINEAR);
    image = gtk_image_new_from_pixbuf(dst_pixbuf);
    g_object_unref(src_pixbuf);
    g_object_unref(dst_pixbuf);
    gtk_widget_show(image);    

    vbox = gtk_vbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), image, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 15);
    gtk_widget_show(vbox);    
    
    return vbox;
}



static GtkWidget * mk_entry(int width, int height, char * text)
{
    GtkWidget *label;
    GtkWidget * entry_ctner;
    GtkWidget *hbox;
    GtkWidget *align;

    label = gtk_label_new(text);
    gtk_widget_show(label);

    entry_pwd = gtk_entry_new();
    entry_ctner = add_entry_style(entry_pwd, NULL,NULL, width, height, FALSE);
    gtk_widget_show(entry_ctner);
    g_signal_connect(G_OBJECT(entry_pwd),"activate",G_CALLBACK(ui_dialog_admin_verify), NULL);

    hbox=gtk_hbox_new(FALSE,3);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), entry_ctner, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

    align = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

    return align;
}

static int ui_dialog_adminpwd_upgrade_init(void)
{
    GtkWidget * action_area;
    GtkWidget * entry_ctner;
    GtkWidget *title_bar;
    GtkWidget *hide_label;
    
     /* Create the widgets */
    dialog = gtk_dialog_new();
    g_signal_connect (G_OBJECT (dialog), "realize",
                      G_CALLBACK (ui_compt_realize), &ui_dialog_adminpwd_upgrade);

    ui_dialog_white_background(dialog);
    title_bar = title_bar_init("", dialog, -1);

    action_area = mk_action_area();
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->action_area), action_area, FALSE, FALSE, 0);

    entry_ctner = mk_entry(280, 30, "输入管理密码:");

    label_tips = create_err_prompt("./icon/ico_abnormal_mm.png", "登录密码错误，请重新输入", -1, -1);

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), title_bar, FALSE, FALSE, 0);

    hide_label = gtk_label_new("");
    gtk_widget_show(hide_label);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hide_label, FALSE, FALSE, 80);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), entry_ctner, FALSE, FALSE, 15);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label_tips, FALSE, FALSE, 5);

    gtk_window_set_decorated(GTK_WINDOW(dialog),FALSE);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

    gtk_widget_set_size_request(dialog, 850, 580);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
    return 0;
}

static void ui_dialog_adminpwd_upgrade_show(void)
{
    if (verified) {
        //ui_tab[UI_TYPE_DIALOG_CONFIG]->show();
    } else {
        if (dialog == NULL) {
            ui_dialog_adminpwd_upgrade_init();
        }
        gtk_widget_show(dialog);
    }
}


static void ui_dialog_admin_upgrade_adapt(void)
{
    if (dialog) {
        if (ui_dialog_adminpwd_upgrade.status == UI_STATUS_SHOW) {
            logi("ui_dialog_admin_adapt\n");
            gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
        }
    }
}

static void ui_dialog_adminpwd_upgrade_destroy(void)
{
    if (dialog) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
}

struct ui_comp_s ui_dialog_adminpwd_upgrade = {
    .type = UI_TYPE_DIALOG_ADMINPWD_UPGRADE,
    .subtype = UI_TOPLEVEL_TYPE_DIALOG,
    .is_init = UI_NEED_INIT,
    .widget = &dialog,
    .init = ui_dialog_adminpwd_upgrade_init,
    .show = ui_dialog_adminpwd_upgrade_show,
    .hide = ui_dialog_adminpwd_upgrade_hide,
    .destroy = ui_dialog_adminpwd_upgrade_destroy,
    .adapt = ui_dialog_admin_upgrade_adapt,
};

