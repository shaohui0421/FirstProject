#include <gtk/gtk.h>
#include "ui_main.h"
#include "ui_dialog_adminpwd.h"

#define TIP_CONNECTED  "已连接"
#define TIP_DISCONNECT "未连接"

GtkWidget *wifi_btnbox = NULL;
btn_apend_t *wifi_button = NULL;

void ui_win_wifibtn_disable(void)
{
    if(wifi_button) {
        set_btn_effect(wifi_button, FALSE);
    }
}

void ui_win_wifibtn_enable(void)
{
    if(wifi_button) {
        set_btn_effect(wifi_button, TRUE);
    }
}

static void reflash_wifi_button(btn_apend_t *object, char *text, char *icon, char *icon_mouse_on)
{
    GdkPixbuf *src_pixbuf = NULL;

    if(icon) {
        src_pixbuf = gdk_pixbuf_new_from_file(icon, NULL);
        gtk_image_set_from_pixbuf(GTK_IMAGE(object->image), src_pixbuf);
        g_object_unref(src_pixbuf);
    }

    if (icon_mouse_on) {
        src_pixbuf = gdk_pixbuf_new_from_file(icon_mouse_on, NULL);
        gtk_image_set_from_pixbuf(GTK_IMAGE(object->mouse_on), src_pixbuf);
        g_object_unref(src_pixbuf);
    }

    if (text) {
        gtk_label_set_text(GTK_LABEL(object->label), text);
    }
}

static void set_wifibtn_signal_icon(int intensity)
{
    if(intensity >= -100 && intensity < -85) {
        reflash_wifi_button(wifi_button, TIP_CONNECTED, "./icon/wireless/wifi_signal_d1.png", "./icon/wireless/wifi_signal_d1_h.png");
    } else if(intensity >= -85 && intensity < -70) {
        reflash_wifi_button(wifi_button, TIP_CONNECTED, "./icon/wireless/wifi_signal_d2.png", "./icon/wireless/wifi_signal_d2_h.png");
    } else if(intensity >= -70 && intensity < -55) {
        reflash_wifi_button(wifi_button, TIP_CONNECTED, "./icon/wireless/wifi_signal_d3.png", "./icon/wireless/wifi_signal_d3_h.png");
    } else if(intensity >= -55 && intensity <= 0) {
        reflash_wifi_button(wifi_button, TIP_CONNECTED, "./icon/wireless/wifi_signal_d4.png", "./icon/wireless/wifi_signal_d4_h.png");
    } else {
        logi("Invalid signal strength!\n");
    }
}

static gint ui_win_button_click_wifi(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    ui_extern_enter_wifi_config();
    ui_manager_publogin_timeout_disable();
    ui_manager_offline_autologin_timer_disable();
    ui_settype_timer_disable();

    ui_tab[UI_TYPE_DIALOG_WIFI]->show();

    return FALSE;
}

static int ui_win_wifibtn_reflash(void *args)
{
    ui_wifi_btn_t *result = args;

    switch (result->type) {
        case UI_BTNBOX_WIRE_CONNECT:
            reflash_wifi_button(wifi_button, TIP_CONNECTED, "./icon/wireless/lan_net_up.png", "./icon/wireless/lan_net_up_h.png");
            break;
        case UI_BTNBOX_WIRE_DISCONNECT:
            reflash_wifi_button(wifi_button, TIP_DISCONNECT, "./icon/wireless/lan_net_down.png", "./icon/wireless/lan_net_down_h.png");
            break;
        case UI_BTNBOX_WIRELESS_CONNECT:
            set_wifibtn_signal_icon(result->intensity);
            break;
        case UI_BTNBOX_WIRELESS_DISCONNECT:
            reflash_wifi_button(wifi_button, TIP_DISCONNECT, "./icon/wireless/lan_net_down.png", "./icon/wireless/lan_net_down_h.png");
            break;
        default:
            break;
    }

    return 0;
}

static void ui_win_wifibtn_load(void)
{
    int status;

    status = ui_extern_net_status_query_result();

    switch (status) {
    case 1:
        wifi_button = create_button_above_label("./icon/wireless/lan_net_up.png", "./icon/wireless/lan_net_up_h.png",
                    NULL, TIP_CONNECTED, GTK_SIGNAL_FUNC(ui_win_button_click_wifi), NULL);
        break;
    case 2:
        wifi_button = create_button_above_label("./icon/wireless/wifi_signal_d4.png", "./icon/wireless/wifi_signal_d4_h.png",
                    NULL, TIP_CONNECTED, GTK_SIGNAL_FUNC(ui_win_button_click_wifi), NULL);
        break;
    case 3:
        wifi_button = create_button_above_label("./icon/wireless/lan_net_down.png", "./icon/wireless/lan_net_down_h.png",
                    NULL, TIP_DISCONNECT, GTK_SIGNAL_FUNC(ui_win_button_click_wifi), NULL);
        break;
    default:
        wifi_button = create_button_above_label("./icon/wireless/lan_net_down.png", "./icon/wireless/lan_net_down_h.png",
                    NULL, TIP_DISCONNECT, GTK_SIGNAL_FUNC(ui_win_button_click_wifi), NULL);
        break;
    }

    set_widget_font_size(wifi_button->label, 13, "Snow");

    return;
}

static int ui_win_wifi_btnbox_init(void)
{
    logi("%s\n", __func__);
    wifi_btnbox = gtk_hbox_new(FALSE, 20);
    g_signal_connect (G_OBJECT(wifi_btnbox), "realize",
                        G_CALLBACK(ui_compt_realize), &ui_win_wifi_btnbox);

    ui_win_wifibtn_load();
    gtk_box_pack_end(GTK_BOX(wifi_btnbox), wifi_button->container, FALSE, FALSE, 0);
    ui_win_set_pos(wifi_btnbox, 94, 90.33);

    return 0;
}

static void ui_win_wifi_btnbox_show(void)
{
	btn_normal_show(wifi_button);
    gtk_widget_show(wifi_btnbox);
}

static int ui_win_wifi_btnbox_ctrl(void *data)
{
	ui_msg_t *msg = data;

	switch (msg->sub_obj) {
	case UI_WIN_WIFI_BTNBOX_ENABLE:
		ui_win_wifibtn_enable();
        break;
	case UI_WIN_WIFI_BTNBOX_DISABLE:
		ui_win_wifibtn_disable();
		break;
    case UI_WIN_WIFI_BTNBOX_REFLASH:
		ui_win_wifibtn_reflash(msg->args);
		break;
	}

	return 0;
}

static void ui_win_wifi_btnbox_adapt(void)
{
    if (wifi_btnbox) {
        if(ui_win_wifi_btnbox.status == UI_STATUS_SHOW) {
            gtk_widget_hide(wifi_btnbox);
            ui_win_move_pos(wifi_btnbox, 94, 90.33);
            gtk_widget_show(wifi_btnbox);
        }
    }
}

static void ui_win_wifi_btnbox_destroy(void)
{
    if (wifi_btnbox) {
        gtk_widget_destroy(wifi_btnbox);
        wifi_btnbox = NULL;
    }
}


struct ui_comp_s ui_win_wifi_btnbox = {
    .type = UI_TYPE_WIN_WIFI_BTNBOX,
    .subtype = UI_TOPLEVEL_TYPE_DIALOG,
    .is_init = UI_NEED_INIT,
    .widget = &wifi_btnbox,
    .init = ui_win_wifi_btnbox_init,
    .show = ui_win_wifi_btnbox_show,
    .adapt = ui_win_wifi_btnbox_adapt,
    .destroy = ui_win_wifi_btnbox_destroy,
    .ctrl = ui_win_wifi_btnbox_ctrl,
    .height = 88.67,
};

