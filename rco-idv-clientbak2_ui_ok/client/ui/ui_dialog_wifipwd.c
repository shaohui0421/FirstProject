#include "ui_main.h"
#include "ui_dialog_wifipwd.h"

#define WIFIPWD_LEAST_LENTH 8

static GtkWidget *dialog = NULL;
static GtkWidget *wifiusrname;
static GtkWidget *wifipwd;
static GtkWidget *wificheckbox;
static GtkWidget *wificombo;
static GtkWidget *wifissid;
static GtkWidget *hide_usrname;
static GtkWidget *hide_passwd;
static btn_apend_t *okay_button;
static btn_apend_t *pwd_visible_button;

static int mode;
static gchar* ssid_t = NULL;
static gboolean pwd_visibility = FALSE;

static int ui_check_wifipwd_legality(int type)
{
    int pwd_len = 0;
    ui_msg_t msg = {0};
    int ret = 0;

    if (wifipwd) {
        pwd_len = strlen(gtk_entry_get_text(GTK_ENTRY(wifipwd)));
        if(pwd_len == 0) {
            //TODO : tip wifipwd can not be null
            logi("tip wifipwd can not be null\n");
            msg.sub_obj = UI_DIALOG_TIPS_AUTH_PWD_EMPTY;
            msg.args = NULL;
            ret = -1;
        } else if (pwd_len > 0 && pwd_len < WIFIPWD_LEAST_LENTH) {
            //TODO : tip wifipwd is too short
            if (type == UI_DIALOG_WIFIPWD_PSK) {
                logi("type %d tip wifipwd can not less than 8\n", type);
                msg.sub_obj = UI_DIALOG_TIPS_AUTH_PWD_TOO_SHORT;
                msg.args = NULL;
                ret = -1;
            }
          }
    }

    if (ret == -1) {
        ui_tab[UI_TYPE_DIALOG_TIPS]->ctrl(&msg);
        ui_tab[UI_TYPE_DIALOG_TIPS]->show();
    }
    return ret;
}

static void ui_combo_select_result(void)
{
    int key_mgmt;

    key_mgmt = gtk_combo_box_get_active(GTK_COMBO_BOX(wificombo));

    if(key_mgmt == 0) {
        gtk_widget_hide(hide_usrname);
        gtk_widget_hide(hide_passwd);
    } else if(key_mgmt == 1) {
        gtk_widget_hide(hide_usrname);
        gtk_widget_show(hide_passwd);
    } else if(key_mgmt == 2) {
        gtk_widget_hide(hide_usrname);
        gtk_widget_show(hide_passwd);
    } else {
        gtk_widget_show(hide_usrname);
        gtk_widget_show(hide_passwd);
    }
}

gint ui_wifi_pwd_destroy_callback(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    gtk_widget_destroy((GtkWidget *)data);
    ui_extern_wifi_cancel_button_handle();
    ui_win_close_keyboard();
    return FALSE;
}

static void set_button_icon(btn_apend_t *object, char *icon, char *icon_mouse_on)
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
}

static gint ui_pwd_visibility_btn_callback(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    if (pwd_visibility == FALSE) {
        pwd_visibility = TRUE;
        set_button_icon(pwd_visible_button, "./icon/wireless/wifipwd_visible.png", "./icon/wireless/wifipwd_visible.png");        
    } else {
        pwd_visibility = FALSE;
        set_button_icon(pwd_visible_button, "./icon/wireless/wifipwd_invisible.png", "./icon/wireless/wifipwd_invisible.png");
    }
    gtk_entry_set_visibility(GTK_ENTRY(wifipwd), pwd_visibility);
    gtk_entry_set_visibility(GTK_ENTRY(hide_passwd), pwd_visibility);
    return FALSE;
}

static void ui_dialog_hide_net_handle(GtkWidget *widget, gpointer data)
{
    upload_config_s config;
    int type = mode;
    int key_mgmt;
    ui_msg_t msg;

    logi("type is %d\n", type);

    if(strlen(gtk_entry_get_text(GTK_ENTRY(wifissid))) == 0) {
        //TODO : tip ssid can not be null
        logi("tip ssid can not be null\n");
        msg.sub_obj = UI_DIALOG_TIPS_AUTH_SSID_EMPTY;
        msg.args = NULL;
        ui_tab[UI_TYPE_DIALOG_TIPS]->ctrl(&msg);
        ui_tab[UI_TYPE_DIALOG_TIPS]->show();
        return;
    }
/*
    if(strlen(gtk_entry_get_text(GTK_ENTRY(wifissid))) > 32) {
        //TODO : tip ssid can not over 32 bytes
        logi("tip ssid can not over 32 bytes\n");
        return;
    }
*/
    key_mgmt = gtk_combo_box_get_active(GTK_COMBO_BOX(wificombo));

    if((key_mgmt == 1) || (key_mgmt == 2)) {
        if (ui_check_wifipwd_legality(key_mgmt) < 0) {
            return;
        }
        strcpy(config.passwd, gtk_entry_get_text(GTK_ENTRY(wifipwd)));
    }

    if(key_mgmt == 3) {
        if(strlen(gtk_entry_get_text(GTK_ENTRY(wifiusrname))) == 0) {
            //TODO : tip wifiusrname can not be null
            logi("tip wifiusrname can not be null\n");
            msg.sub_obj = UI_DIALOG_TIPS_AUTH_USER_EMPTY;
            msg.args = NULL;
            ui_tab[UI_TYPE_DIALOG_TIPS]->ctrl(&msg);
            ui_tab[UI_TYPE_DIALOG_TIPS]->show();

            return;
        }

        if (ui_check_wifipwd_legality(key_mgmt) < 0) {
            return;
        }

        strcpy(config.passwd, gtk_entry_get_text(GTK_ENTRY(wifipwd)));
        strcpy(config.usrname, gtk_entry_get_text(GTK_ENTRY(wifiusrname)));
    }

    config.key_mgmt = key_mgmt;
    strcpy(config.ssid, gtk_entry_get_text(GTK_ENTRY(wifissid)));

    /*调用接口：认证*/
    set_btn_effect(okay_button, FALSE);
    ui_extern_wifi_authenticate_enter(type, config);
    set_btn_effect(okay_button, TRUE);
}

static void ui_dialog_wp_handle(GtkWidget *widget, gpointer data)
{
    upload_config_s config;
    int type = mode;

    logi("type is %d\n", type);
    if (ui_check_wifipwd_legality(type) < 0) {
        return;
    }

    strcpy(config.passwd, gtk_entry_get_text(GTK_ENTRY(wifipwd)));

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wificheckbox))) {
        config.save = TRUE;
    } else {
        config.save = FALSE;
    }

    /*调用接口：认证*/
    set_btn_effect(okay_button, FALSE);
    ui_extern_wifi_authenticate_enter(type, config);
    set_btn_effect(okay_button, TRUE);
}

static void ui_dialog_eap_handle(GtkWidget *widget, gpointer data)
{
    upload_config_s config;
    int type = mode;
    ui_msg_t msg;

    logi("type is %d\n", type);
    if(strlen(gtk_entry_get_text(GTK_ENTRY(wifiusrname))) == 0)  {
        //TODO : tip wifiusrname can not be null
        logi("tip wifiusrname can not be null\n");
        msg.sub_obj = UI_DIALOG_TIPS_AUTH_USER_EMPTY;
        msg.args = NULL;
        ui_tab[UI_TYPE_DIALOG_TIPS]->ctrl(&msg);
        ui_tab[UI_TYPE_DIALOG_TIPS]->show();
        return;
    }

    if (ui_check_wifipwd_legality(type) < 0) {
        return;
    }

    strcpy(config.usrname, gtk_entry_get_text(GTK_ENTRY(wifiusrname)));
    strcpy(config.passwd, gtk_entry_get_text(GTK_ENTRY(wifipwd)));

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wificheckbox))) {
        config.save = TRUE;
    } else {
        config.save = FALSE;
    }

    /*调用接口：认证*/
    set_btn_effect(okay_button, FALSE);
    ui_extern_wifi_authenticate_enter(type, config);
    set_btn_effect(okay_button, TRUE);
}


static void okay_button_enter_handle(GtkWidget *widget, gpointer data)
{
    g_signal_emit_by_name(G_OBJECT(okay_button->event), "button_release_event");
}


static GtkWidget *mk_action_area(GCallback callback)
{
	GtkWidget *btn_hbox;
    GtkWidget *vbox;
    GtkWidget *align;
    GtkWidget *label;

    btn_apend_t * cancal_button;

    okay_button = create_button("./icon/btn_green.png", "./icon/btn_green_h.png", NULL, "确定",
    		100, 38, callback, NULL);
    set_widget_font_size(okay_button->label, -1, "Snow");
    cancal_button = create_button("./icon/btn_gray.png", "./icon/btn_gray_h.png", NULL, "取消",
    		100, 38, G_CALLBACK(ui_wifi_pwd_destroy_callback), dialog);
    set_widget_font_size(cancal_button->label, -1, "#666666");

    btn_hbox = gtk_hbox_new(FALSE, 20);

    gtk_box_pack_start(GTK_BOX(btn_hbox), okay_button->container, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(btn_hbox), cancal_button->container, FALSE, FALSE, 0);
    gtk_widget_show(btn_hbox);

    label = gtk_label_new("");
	gtk_widget_show(label);

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	gtk_widget_show(vbox);

    align = gtk_alignment_new(0.5, 1, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), vbox);
    gtk_widget_show(align);

    return align;
}

static GtkWidget * mk_verify_entry(int type, int width, int height, char * text)
{
	GtkWidget *label;
	GtkWidget *entry;
	GtkWidget *hbox;
	GtkWidget *align;

	label = gtk_label_new(text);
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_widget_set_size_request(label, 64, 30);
	gtk_widget_show(label);

    hbox=gtk_hbox_new(FALSE, 3);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    if(type == UI_PASSWORD_ENTRY) {    
    	entry = gtk_entry_new();
    	wifipwd = add_entry_style(entry, NULL, NULL, width, height, FALSE);
        gtk_entry_set_max_length(GTK_ENTRY(wifipwd), 32);
    	gtk_widget_show(wifipwd);
        gtk_box_pack_start(GTK_BOX(hbox), wifipwd, FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(wifipwd), "activate", G_CALLBACK(okay_button_enter_handle), NULL);

        pwd_visible_button = create_button("./icon/wireless/wifipwd_invisible.png", "./icon/wireless/wifipwd_invisible.png", NULL, NULL,
    		-1 ,-1, G_CALLBACK(ui_pwd_visibility_btn_callback), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), pwd_visible_button->container, FALSE, FALSE, 5);        
    } else if(type == UI_USERNAME_ENTRY) {
        entry = gtk_entry_new();
        wifiusrname = add_entry_style(entry, NULL, NULL, width, height, FALSE);
        gtk_entry_set_visibility(GTK_ENTRY(wifiusrname), TRUE);
        gtk_entry_set_max_length(GTK_ENTRY(wifiusrname), 32);
        gtk_widget_show(wifiusrname);
        gtk_box_pack_start(GTK_BOX(hbox), wifiusrname, FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(wifiusrname), "activate", G_CALLBACK(okay_button_enter_handle), NULL);
    } else if (type == UI_SSID_ENTRY) {
        entry = gtk_entry_new();
        wifissid = add_entry_style(entry, NULL, NULL, width, height, FALSE);
        gtk_entry_set_visibility(GTK_ENTRY(wifissid), TRUE);
        gtk_entry_set_max_length(GTK_ENTRY(wifissid), 32);
        gtk_widget_show(wifissid);
        gtk_box_pack_start(GTK_BOX(hbox), wifissid, FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(wifissid), "activate", G_CALLBACK(okay_button_enter_handle), NULL);
    }
    g_signal_connect(G_OBJECT(entry),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    gtk_widget_show(hbox);

	align = gtk_alignment_new(0, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

	return align;
}

static GtkWidget * mk_check_button(void)
{
	GtkWidget *text;
	GtkWidget *hbox;
	GtkWidget *align;
    GtkWidget *label;

    label = gtk_label_new("");
    gtk_widget_set_size_request(label, 61, 10);
    gtk_widget_show(label);

	text = gtk_label_new("自动连接");
    gtk_misc_set_alignment(GTK_MISC(text), 0, 0.5);
    gtk_widget_set_size_request(text, 61, 10);
	gtk_widget_show(text);

    wificheckbox = gtk_check_button_new();
    gtk_widget_show(wificheckbox);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wificheckbox), TRUE);

	hbox=gtk_hbox_new(FALSE, 3);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), wificheckbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), text, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	align = gtk_alignment_new(0, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

	return align;
}

static GtkWidget * mk_verify_type(void)
{
	GtkWidget *label;
	GtkWidget *hbox;
    GtkWidget *align;

	label = gtk_label_new("安全类型：");
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_widget_set_size_request(label, 66, 30);
	gtk_widget_show(label);

    wificombo = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(wificombo), "无");
    gtk_combo_box_append_text(GTK_COMBO_BOX(wificombo), "WEP认证");
    gtk_combo_box_append_text(GTK_COMBO_BOX(wificombo), "WPA/WPA2 PSK认证");
    gtk_combo_box_append_text(GTK_COMBO_BOX(wificombo), "WPA/WPA2 EAP认证");
    gtk_combo_box_set_active (GTK_COMBO_BOX(wificombo), 0);
    gtk_widget_set_size_request(wificombo, 242, 30);
    gtk_widget_show(wificombo);

    g_signal_connect(G_OBJECT(wificombo), "changed", G_CALLBACK(ui_combo_select_result), NULL);

    hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), wificombo, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

    align = gtk_alignment_new(0, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

    return align;
}

GtkWidget *mk_title_bar(char *title, void *data)
{
	GtkWidget *title_bar;
    GtkWidget *label1;
	btn_apend_t *button_quit;

	title_bar = gtk_hbox_new(FALSE,0);
    label1 = gtk_label_new(title);
	set_widget_font_size(label1, 12, "#666666");
    gtk_widget_set_size_request(label1, 460-25-35, 20);
    gtk_misc_set_alignment(GTK_MISC(label1), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(title_bar), label1, FALSE, FALSE, 10);

    button_quit = create_button("./icon/btn_close.png", "./icon/btn_close_h.png", NULL, NULL, -1, -1, G_CALLBACK(ui_wifi_pwd_destroy_callback), data);
	gtk_box_pack_end(GTK_BOX(title_bar), button_quit->container, FALSE, FALSE, 5);

    gtk_widget_show(label1);
	gtk_widget_show(title_bar);
	
	return title_bar;
}

static GtkWidget *create_wifipwd_dialog_eap(void)
{
    GtkWidget *eap_usrname;
	GtkWidget *eap_passwd;
    GtkWidget *eap_checkbox;
    GtkWidget *vbox;
    GtkWidget *align;

    vbox = gtk_vbox_new(FALSE, 0);

    eap_usrname = mk_verify_entry(UI_USERNAME_ENTRY, 242, 30, "用户名称：");
    eap_passwd = mk_verify_entry(UI_PASSWORD_ENTRY, 242, 30, "密码：");
    eap_checkbox = mk_check_button();
    g_signal_connect(G_OBJECT(eap_usrname),"focus",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), eap_usrname, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), eap_passwd, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), eap_checkbox, FALSE, FALSE, 10);

    gtk_widget_show(vbox);

    align = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), vbox);
    gtk_widget_show(align);

    return align;
}

static GtkWidget *create_wifipwd_dialog_wp(void)
{
	GtkWidget *wp_passwd;
    GtkWidget *wp_checkbox;
    GtkWidget *vbox;
    GtkWidget *align;

    vbox = gtk_vbox_new(FALSE, 0);

    wp_passwd = mk_verify_entry(UI_PASSWORD_ENTRY, 242, 30, "密码：");
    wp_checkbox = mk_check_button();
    g_signal_connect(G_OBJECT(wp_passwd),"focus",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), wp_passwd, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), wp_checkbox, FALSE, FALSE, 10);

    gtk_widget_show(vbox);

    align = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), vbox);
    gtk_widget_show(align);

    return align;
}

static GtkWidget *create_wifipwd_dialog_hide_net(void)
{
    GtkWidget *hide_ssid;
    GtkWidget *hide_combo;
    GtkWidget *vbox;
    GtkWidget *align;

    vbox = gtk_vbox_new(FALSE, 0);

    hide_ssid = mk_verify_entry(UI_SSID_ENTRY, 242, 30, "SSID：");
    hide_usrname = mk_verify_entry(UI_USERNAME_ENTRY, 242, 30, "用户名称：");
    hide_passwd = mk_verify_entry(UI_PASSWORD_ENTRY, 242, 30, "密码：");
    g_signal_connect(G_OBJECT(hide_ssid),"focus",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    hide_combo = mk_verify_type();    
    gtk_widget_hide(hide_usrname);
    gtk_widget_hide(hide_passwd);

    gtk_box_pack_start(GTK_BOX(vbox), hide_ssid, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hide_usrname, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hide_passwd, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hide_combo, FALSE, FALSE, 5);

    gtk_widget_show(vbox);

    align = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), vbox);
    gtk_widget_show(align);

    return align;
}

static int ui_dialog_wifipwd_init(void)
{
	GtkWidget *title_bar = NULL;
	GtkWidget *action_area = NULL;
    GtkWidget *enter_area = NULL;
    char title[128] = "";

    dialog = gtk_dialog_new();
    g_signal_connect (G_OBJECT(dialog), "realize",
                      G_CALLBACK(ui_compt_realize), &ui_dialog_wifipwd);
    
    gtk_widget_set_size_request(dialog, 460, 310);
    ui_dialog_white_background(dialog);
    update_widget_bg(dialog, "./icon/wireless/frame_wifi_verify.png", -1, -1);
    gtk_window_set_decorated(GTK_WINDOW(dialog),FALSE);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

    if (ssid_t == NULL || strcmp(ssid_t, "") == 0) {
        strcpy(title, "连接无线网络");
    } else {
        strcpy(title, ssid_t);
    }
    pwd_visibility = FALSE;

    switch (mode) {
    case UI_DIALOG_WIFIPWD_EAP:
        title_bar = mk_title_bar(title, dialog);
        action_area = mk_action_area(G_CALLBACK(ui_dialog_eap_handle));
        enter_area = create_wifipwd_dialog_eap();
        break;
    case UI_DIALOG_WIFIPWD_WEP:
        title_bar = mk_title_bar(title, dialog);
        action_area = mk_action_area(G_CALLBACK(ui_dialog_wp_handle));
        enter_area = create_wifipwd_dialog_wp();
        break;
    case UI_DIALOG_WIFIPWD_PSK:
        title_bar = mk_title_bar(title, dialog);
        action_area = mk_action_area(G_CALLBACK(ui_dialog_wp_handle));
        enter_area = create_wifipwd_dialog_wp();
        break;
    case UI_DIALOG_WIFIPWD_HIDE_NET:
        title_bar = mk_title_bar("连接其他网络", dialog);
        action_area = mk_action_area(G_CALLBACK(ui_dialog_hide_net_handle));
        enter_area = create_wifipwd_dialog_hide_net();
        break;
    default:
        return 0;
    }

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), title_bar, FALSE, FALSE, 3);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), enter_area);
    gtk_box_pack_end(GTK_BOX(GTK_DIALOG(dialog)->vbox), action_area, FALSE, FALSE, 0);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
    return 0;
}

static void ui_dialog_wifipwd_show(void)
{
    if (dialog != NULL) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
    ui_dialog_wifipwd_init();
    gtk_widget_show(dialog);
}

static void ui_dialog_wifipwd_hide()
{
    if(dialog != NULL) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
    mode = UI_DIALOG_WIFIPWD_NONE;
    ssid_t = NULL;
}

static int ui_dialog_wifipwd_ctrl(void *data)
{
  	ui_msg_t *msg = data;

    mode = msg->sub_obj;
    ssid_t = (gchar *)(((ui_string_arg_t *)(msg->args))->str);

    return 0;
}

static void ui_dialog_wifipwd_adapt()
{
    if (dialog) {
        if (ui_dialog_wifipwd.status == UI_STATUS_SHOW) {
            logi("ui_dialog_wifipwd_adapt\n");
            gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
        }
    }
}

static void ui_dialog_wifipwd_destroy(void)
{
    if (dialog) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }

    mode = UI_DIALOG_WIFIPWD_NONE;
    ssid_t = NULL;
}


struct ui_comp_s ui_dialog_wifipwd = {
    .type = UI_TYPE_DIALOG_WIFIPWD,
    .subtype = UI_TOPLEVEL_TYPE_DIALOG,
    .is_init = UI_NEED_INIT,
    .widget = &dialog,
    .init = ui_dialog_wifipwd_init,
    .show = ui_dialog_wifipwd_show,
    .hide = ui_dialog_wifipwd_hide,
    .ctrl = ui_dialog_wifipwd_ctrl,
    .destroy = ui_dialog_wifipwd_destroy,
    .adapt = ui_dialog_wifipwd_adapt,
};
