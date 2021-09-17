#include <gtk/gtk.h>
#include "ui_main.h"
#include "ui_dialog_adminpwd.h"

static GtkWidget *hbox_button;
//static GtkWidget *button_config;
//static GtkWidget *button_about;
btn_apend_t *button_config;
btn_apend_t *button_about;
btn_apend_t *button_shutdown;
//static GtkWidget *button_shutdown;
//static GtkWidget *button_reboot;


void ui_win_btn_all_disable(void)
{
	set_btn_effect(button_config, FALSE);
	set_btn_effect(button_about, FALSE);
	set_btn_effect(button_shutdown, FALSE);	
}


void ui_win_btn_all_enable(void)
{
	set_btn_effect(button_config, TRUE);
	set_btn_effect(button_about, TRUE);
	set_btn_effect(button_shutdown, TRUE);
	ui_win_close_keyboard(); 
}

void ui_win_btn_disable_config(void)
{
	set_btn_effect(button_config, FALSE);
	set_btn_effect(button_about, TRUE);
	set_btn_effect(button_shutdown, TRUE);
}

static gint ui_win_button_event_shutdown(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    ui_manager_publogin_timeout_disable();
    ui_manager_offline_autologin_timer_disable();
    ui_settype_timer_disable();
    ui_settype_err_prompt_hide();
	ui_tab[UI_TYPE_DIALOG_SHUTDOWN]->show();
    return FALSE;
}

gint ui_win_button_click_config(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    if (g_win_manager.config_disable) {
        return FALSE;
    }

        ui_extern_enter_settings();
        ui_manager_publogin_timeout_disable();
        ui_manager_offline_autologin_timer_disable();
        ui_settype_timer_disable();
        ui_settype_err_prompt_hide();

    ui_tab[UI_TYPE_DIALOG_ADMINPWD]->show();
    // XXX: send redirect mesg to adminpwd

    return FALSE;
}

static gint ui_win_button_click_about(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    ui_manager_publogin_timeout_disable();
    ui_manager_offline_autologin_timer_disable();
    ui_settype_timer_disable();
    ui_settype_err_prompt_hide();
    ui_tab[UI_TYPE_DIALOG_ABOUT]->show();
    
    return FALSE;
}

static int ui_win_button_box_init(void)
{
    hbox_button = gtk_hbox_new(TRUE, 20);
    
    g_signal_connect (G_OBJECT (hbox_button), "realize",
                      G_CALLBACK (ui_compt_realize), &ui_win_button_box);

    button_config = create_button_above_label("./icon/ico_c.png", "./icon/ico_c_h.png", NULL, "设置",
    						GTK_SIGNAL_FUNC(ui_win_button_click_config), NULL);
	set_widget_font_size(button_config->label, 13, "Snow");
	
    button_about = create_button_above_label("./icon/ico_d.png", "./icon/ico_d_h.png", NULL, "关于",
    						GTK_SIGNAL_FUNC(ui_win_button_click_about), NULL);
	set_widget_font_size(button_about->label, 13, "Snow");
	
    button_shutdown = create_button_above_label("./icon/ico_b.png", "./icon/ico_b_h.png", NULL, "电源", G_CALLBACK(ui_win_button_event_shutdown), NULL);
	set_widget_font_size(button_shutdown->label, 13, "Snow");
	
    gtk_box_pack_start(GTK_BOX(hbox_button), button_config->container, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(hbox_button), button_about->container, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(hbox_button), button_shutdown->container, TRUE, FALSE, 10);

    ui_win_set_pos(hbox_button, 0, 0);

    return 0;
}

static void ui_win_button_box_show(void)
{
	btn_normal_show(button_config);
	btn_normal_show(button_about);
	btn_normal_show(button_shutdown);
    gtk_widget_show(hbox_button);
}



static int ui_win_button_box_ctrl(void *data)
{
	ui_msg_t *msg = data;
	switch (msg->sub_obj) {
	case UI_WIN_BTN_ENABLE:
		ui_win_btn_all_enable();
        break;
	case UI_WIN_BTN_DISABLE:
		ui_win_btn_all_disable();
		break;
	}
	return 0;
}

static void ui_win_botton_box_adapt(void)
{

    if (hbox_button) {
        ui_win_button_box.connect_id = g_signal_connect(G_OBJECT (hbox_button), "size-allocate", 
                                            G_CALLBACK(ui_win_put_nice_position_cb), &ui_win_button_box);

        logi("ui_win_botton_box_adapt status %d\n", ui_win_button_box.status);

        if(ui_win_button_box.status == UI_STATUS_SHOW) {
            gtk_widget_hide(hbox_button);
            gtk_widget_show(hbox_button);
        }
    }

}

static void ui_win_botton_box_destroy(void)
{
    if (hbox_button) {
        gtk_widget_destroy(hbox_button);
        hbox_button = NULL;
    }
}

struct ui_comp_s ui_win_button_box = {
    .type = UI_TYPE_WIN_BUTTON_BOX,
    .subtype = UI_TOPLEVEL_TYPE_WIN,
    .is_init = UI_NEED_INIT,
    .widget = &hbox_button,
    .init = ui_win_button_box_init,
    .show = ui_win_button_box_show,
    .ctrl = ui_win_button_box_ctrl,
    .adapt = ui_win_botton_box_adapt,
    .destroy = ui_win_botton_box_destroy,
    .height = 88.67,
};

