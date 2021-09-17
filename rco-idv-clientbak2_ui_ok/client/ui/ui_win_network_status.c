#include <gtk/gtk.h>
#include "ui_main.h"

static GtkWidget *label;

static int ui_win_network_status_ctrl(void *str)
{
    //FIXME: define new struct, set pic and string
    gtk_label_set_text(GTK_LABEL(label), str);

    return 0;
}

static void ui_win_network_status_hide()
{
    gtk_widget_hide(label);
}

static int ui_win_network_status_init()
{
    label = gtk_label_new("Offline");
    ui_win_set_pos(label, 85, 85);
    
    g_signal_connect (G_OBJECT (label), "realize",
                      G_CALLBACK (ui_compt_realize), &ui_win_network_status);

    return 0;
}

static void ui_win_network_status_show()
{
    gtk_widget_show(label);
}

struct ui_comp_s ui_win_network_status = {
    .type = UI_TYPE_WIN_NETWORK_STATUS,
    .subtype = UI_TOPLEVEL_TYPE_WIN,
    .is_init = UI_NEED_INIT,
    .widget = &label,
    .init = ui_win_network_status_init,
    .show = ui_win_network_status_show,
    .hide = ui_win_network_status_hide,
    .ctrl = ui_win_network_status_ctrl,
};



