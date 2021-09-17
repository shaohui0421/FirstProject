#include "ui_dialog_authpwd.h"


static GtkWidget *dialog;
static GtkWidget *authcheckbox;

static GtkWidget *authpasswd;
static GtkWidget *authusername;
static GtkWidget * authconnect_remind_area;
static GtkWidget * auth_remind_text_area;

static btn_apend_t  *connect_button;
static btn_apend_t *pwd_visible_button;
static btn_apend_t *button_quit;

static gboolean pwd_visibility = FALSE;
static auth_info_t auth_info;
static guint auth_remind_text_timer;

static int ui_dialog_authpwd_init(void);
static gint ui_auth_pwd_destroy_callback(GtkWidget *widget, GdkEventButton *event, gpointer data);
static GtkWidget *mk_action_area(GCallback callback, char *text);
static int ui_dialog_authpwd_reflash(int auth_status);
static void ui_dialog_authpwd_remind(int auth_status);
static int ui_dialog_authpwd_ctrl(void *data);


static void trim_space(char* str)
{
    int i;
    int len;

    if (str == NULL || *str == '\0') {
        return;
    }
    len = strlen(str);
    //find rtrim pos
    for (i = len - 1; i >= 0; i--) {
        if (str[i] == ' ') {
            str[i] = '\0';
        } else {
            break;
        }
    }
    //find ltrim pos
    for (i = 0; i < len; i++) {
        if (str[i] != ' ') {
            break;
        }
    }
    if (i > 0) {
        memmove(str, str + i, len + 1 - i);
    }
}

static void ui_dialog_button_status(int auth_status)
{
    switch (auth_status) {
    case UI_NET_AUTHSTATUS_AUTHING:
         set_btn_effect(connect_button, FALSE);
         set_btn_effect(button_quit, FALSE);
         break;
     default:
         set_btn_effect(connect_button, TRUE);
         set_btn_effect(button_quit, TRUE);
         break;
    }
}

static GtkWidget *mk_prompt(char * lab_text, int lab_font_size, char *color_str)
{
    GtkWidget * align;
    GtkWidget * label = NULL;
    GtkWidget * hbox;

    if (lab_text) {
        label = gtk_label_new(lab_text);
        set_widget_font_size(label, 12, color_str);
        gtk_widget_show(label);		
}

    hbox = gtk_hbox_new(FALSE, 5);
    if (label) {
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    }

    gtk_widget_show(hbox);

    align = gtk_alignment_new(0.4,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

    return align;
}

static void ui_authentry_load(int auth_status)
{
    ui_extern_get_auth_info(&auth_info);
    if (auth_info.auto_connect == AUTO_CONNECT) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(authcheckbox), TRUE);
    } else if (auth_info.auto_connect == DISAUTO_CONNECT) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(authcheckbox), FALSE);
        if (auth_status != UI_NET_AUTHSTATUS_SUCCESS 
            && auth_status != UI_NET_AUTHSTATUS_AUTHING) {
            ui_extern_delete_auth_info();
            strcpy(auth_info.username, "");
            strcpy(auth_info.password, "");
        }
    }

    logi("auto_connect %d username %s\n", auth_info.auto_connect, auth_info.username);
    gtk_entry_set_text(GTK_ENTRY(authusername), auth_info.username);
    gtk_entry_set_text(GTK_ENTRY(authpasswd), auth_info.password);
}

static int check_auth_empty(void)
{
    char username_tmp[256] = {0};
    char passwd_tmp[256] = {0};

    strcpy(username_tmp, gtk_entry_get_text(GTK_ENTRY(authusername)));
    strcpy(passwd_tmp, gtk_entry_get_text(GTK_ENTRY(authpasswd)));

    if (strcmp("", username_tmp) == 0 || strcmp("", passwd_tmp) == 0) {
        return AUTH_CHECK_ERR_USER_OR_PASSWD_EMPTY;
    }

    trim_space(username_tmp);
    if(strlen(username_tmp) == 0) {
        return AUTH_CHECK_ERR_USER_OR_PASSWD_BLAKE;
    }

    return AUTH_CHECK_OK;
}

static void ui_dialog_config_auth_save()
{
    gboolean auto_connect = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(authcheckbox));

    if (authusername && authpasswd) { 
        strcpy(auth_info.username, gtk_entry_get_text(GTK_ENTRY(authusername)));
        strcpy(auth_info.password, gtk_entry_get_text(GTK_ENTRY(authpasswd)));
    }

    if (auth_info.auth_type == UI_AUTH_TYPE_NONE) {
        if (auto_connect == FALSE) {
            strcpy(auth_info.username, "");
            strcpy(auth_info.password, "");
        }
    }

    if (auto_connect == FALSE) {
        ui_extern_save_auto_connect_info(DISAUTO_CONNECT);
    } else {
        ui_extern_save_auto_connect_info(AUTO_CONNECT);
    }

    ui_extern_save_auth_info(&auth_info);
    ui_extern_save_auth_setting(&auth_info);
}

static void ui_dialog_auth_break_handler(GtkWidget *widget, gpointer data)
{
    auth_info.auth_type = UI_AUTH_TYPE_NONE;
    ui_dialog_config_auth_save();
    return;
}

static void ui_dialog_auth_handler(GtkWidget *widget, gpointer data)
{
    int auth_check_ret = AUTH_CHECK_OK;
    ui_msg_t msg = {0,};

    auth_check_ret = check_auth_empty();
    if (auth_check_ret != AUTH_CHECK_OK) {
        switch(auth_check_ret) {
        case AUTH_CHECK_ERR_USER_OR_PASSWD_EMPTY:
            ui_dialog_authpwd_remind(UI_TIPS_AUTHSTATUS_USER_ERR);
            break;
        case AUTH_CHECK_ERR_USER_OR_PASSWD_BLAKE:
            ui_dialog_authpwd_remind(UI_TIPS_AUTHSTATUS_USERPASSWD_BLAKE);
            break;
        default:
            break;
        }

        msg.object = UI_TYPE_WIN_AUTH_BTNBOX;
        msg.sub_obj = UI_NET_AUTHSTATUS_FAIL;
        // reflash authbtn box
        if (ui_tab[UI_TYPE_WIN_AUTH_BTNBOX]) {
            ui_tab[UI_TYPE_WIN_AUTH_BTNBOX]->ctrl(&msg);
        }
        return;
    }

    auth_info.auth_type = UI_AUTH_TYPE_DOT1X;
    ui_dialog_config_auth_save();
}

static gboolean auth_remind_text_timer_callback()
{
    ui_msg_t msg = {0,};
    msg.object = UI_TYPE_WIN_AUTH_BTNBOX;
    msg.sub_obj = UI_NET_AUTHSTATUS_FAIL;

    ui_dialog_authpwd_reflash(UI_TIPS_AUTHSTATUS_TIMEOUT);
    // reflash authbtn box
    if (ui_tab[UI_TYPE_WIN_AUTH_BTNBOX]) {
        ui_tab[UI_TYPE_WIN_AUTH_BTNBOX]->ctrl(&msg);
    }
    return 0;
}

static void ui_dialog_authpwd_remind(int auth_status)
{
    if (!dialog) {
        return;
    }

    if (auth_remind_text_area != NULL) {
        gtk_widget_destroy(auth_remind_text_area);
        auth_remind_text_area = NULL;
    }

    if (auth_remind_text_timer) {
        g_source_remove(auth_remind_text_timer);
        auth_remind_text_timer = 0;
    }

    switch (auth_status) {
    case UI_TIPS_AUTHSTATUS_USER_ERR:
        auth_remind_text_area = mk_prompt("  用户名或密码不能为空", 12, "#F53C3C");
         break;
    case UI_TIPS_AUTHSTATUS_USERPASSWD_BLAKE:
        auth_remind_text_area = mk_prompt("       输入的内容不允许全为空格", 12, "#F53C3C");
         break;
    case UI_NET_AUTHSTATUS_AUTHING:
         auth_remind_text_area = mk_prompt("正在连接，请稍等...", 12, "#E4B31E");
         auth_remind_text_timer = g_timeout_add_seconds(90, auth_remind_text_timer_callback, NULL);
         break;
    case UI_NET_AUTHSTATUS_UNNECESSARY:
    case UI_NET_AUTHSTATUS_FAIL:
    case UI_NET_AUTHSTATUS_OTHER:
    case UI_TIPS_AUTHSTATUS_TIMEOUT:
         auth_remind_text_area = mk_prompt("      连接失败，请检查用户名密码及相关配置", 12, "#F53C3C");
         break;
    default:
        return;
    }

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), auth_remind_text_area, FALSE, FALSE, 5);
    return;
}

static void ui_dialog_auth_entry(int status)
{
    if (authusername && authpasswd) {
        gtk_widget_set_sensitive(authusername, status);
        gtk_widget_set_sensitive(authpasswd, status);
        gtk_widget_set_sensitive(authcheckbox, status);
    }
}

static void ui_dialog_authbtn_load(int auth_status)
{
    if (authconnect_remind_area) {
        gtk_widget_destroy(authconnect_remind_area);
        authconnect_remind_area = NULL;
    }

    switch (auth_status) {
    case UI_NET_AUTHSTATUS_AUTHING:
    case UI_NET_AUTHSTATUS_SUCCESS:
        //TODO:1.disable all btnbox 
        ui_dialog_auth_entry(FALSE);
        //TODU:2.change the ok button to disconnect button
        authconnect_remind_area = mk_action_area(G_CALLBACK(ui_dialog_auth_break_handler), "断开连接");
        break;
    case UI_NET_AUTHSTATUS_UNNECESSARY:
    case UI_NET_AUTHSTATUS_FAIL:
    case UI_NET_AUTHSTATUS_OTHER:
    case UI_TIPS_AUTHSTATUS_USER_ERR:
    case UI_TIPS_AUTHSTATUS_TIMEOUT:
    case UI_TIPS_AUTHSTATUS_DISCONNECT:
    default:
        ui_dialog_auth_entry(TRUE);
        authconnect_remind_area = mk_action_area(G_CALLBACK(ui_dialog_auth_handler), "连接");
        break;
    }

    if (authconnect_remind_area) {
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), authconnect_remind_area, FALSE, FALSE, 10);
    }
}

static int ui_dialog_authpwd_reflash(int auth_status)
{
    if (!dialog) {
        return 0;
    }

    logi("%s, auth_status is %d\n", __func__, auth_status);
    ui_dialog_authbtn_load(auth_status);
    ui_dialog_button_status(auth_status);
    ui_dialog_authpwd_remind(auth_status);
    return 0;
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

    authcheckbox = gtk_check_button_new();
    gtk_widget_show(authcheckbox);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(authcheckbox), FALSE);


    g_signal_connect(GTK_TOGGLE_BUTTON(authcheckbox), "clicked", G_CALLBACK(NULL), NULL);
    hbox=gtk_hbox_new(FALSE, 3);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), authcheckbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), text, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

    align = gtk_alignment_new(0, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

    return align;
}

static void okay_button_enter_handle(GtkWidget *widget, gpointer data)
{
    g_signal_emit_by_name(G_OBJECT(connect_button->event), "button_release_event");
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
    gtk_entry_set_visibility(GTK_ENTRY(authpasswd), pwd_visibility);

    return FALSE;
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
        authpasswd = add_entry_style(entry, NULL, NULL, width, height, FALSE);
        gtk_entry_set_max_length(GTK_ENTRY(authpasswd), 32);
        gtk_widget_show(authpasswd);
        gtk_box_pack_start(GTK_BOX(hbox), authpasswd, FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(authpasswd), "activate", G_CALLBACK(okay_button_enter_handle), NULL);
    g_signal_connect(G_OBJECT(authpasswd),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);

        pwd_visible_button = create_button("./icon/wireless/wifipwd_invisible.png", "./icon/wireless/wifipwd_invisible.png", NULL, NULL,
            -1 ,-1, G_CALLBACK(ui_pwd_visibility_btn_callback), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), pwd_visible_button->container, FALSE, FALSE, 5);        
    } else if(type == UI_USERNAME_ENTRY) {
        entry = gtk_entry_new();
        authusername = add_entry_style(entry, NULL, NULL, width, height, FALSE);
        gtk_entry_set_visibility(GTK_ENTRY(authusername), TRUE);
        gtk_entry_set_max_length(GTK_ENTRY(authusername), 32);
        gtk_widget_show(authusername);
        gtk_box_pack_start(GTK_BOX(hbox), authusername, FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(authusername), "activate", G_CALLBACK(okay_button_enter_handle), NULL);
    g_signal_connect(G_OBJECT(authusername),"focus",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    g_signal_connect(G_OBJECT(authusername),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    }
    
    gtk_widget_show(hbox);

    align = gtk_alignment_new(0, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

    return align;
}

static GtkWidget *create_authpwd_dialog(void)
{
    GtkWidget *auth_usrname;
    GtkWidget *auth_passwd;
    GtkWidget *auth_checkbox;
    GtkWidget *vbox;
    GtkWidget *align;

    vbox = gtk_vbox_new(FALSE, 0);

    auth_usrname = mk_verify_entry(UI_USERNAME_ENTRY, 242, 30, "用户名称：");
    auth_passwd = mk_verify_entry(UI_PASSWORD_ENTRY, 242, 30, "用户密码：");
    auth_checkbox = mk_check_button();


    gtk_box_pack_start(GTK_BOX(vbox), auth_usrname, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), auth_passwd, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), auth_checkbox, FALSE, FALSE, 10);

    gtk_widget_show(vbox);

    align = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), vbox);
    gtk_widget_show(align);

    return align;
}

static GtkWidget *mk_title_bar(char *title, void *data)
{
    GtkWidget *title_bar;
    GtkWidget *label1;


    title_bar = gtk_hbox_new(FALSE,0);
    label1 = gtk_label_new(title);
    set_widget_font_size(label1, 12, "#666666");
    gtk_widget_set_size_request(label1, 460-25-35, 20);
    gtk_misc_set_alignment(GTK_MISC(label1), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(title_bar), label1, FALSE, FALSE, 10);

    button_quit = create_button("./icon/btn_close.png", "./icon/btn_close_h.png", NULL, NULL, -1, -1, G_CALLBACK(ui_auth_pwd_destroy_callback), data);
    gtk_box_pack_end(GTK_BOX(title_bar), button_quit->container, FALSE, FALSE, 5);

    gtk_widget_show(label1);
    gtk_widget_show(title_bar);

    return title_bar;
}

gint ui_auth_pwd_destroy_callback(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    
    if (dialog) {
          gtk_widget_destroy(dialog);
          dialog = NULL;
    }

    if (auth_remind_text_timer) {
        g_source_remove(auth_remind_text_timer);
        auth_remind_text_timer = 0;
    }

    bubble_normal_show(UI_AUTH_STATUS_REQUEST);
    ui_extern_cancel_auth_config();

    return 0;
}

static GtkWidget *mk_action_area(GCallback callback, char *text)
{
    GtkWidget * vbox;
    GtkWidget * btn_hbox;
    GdkPixbuf * src_pixbuf = NULL;
    GdkPixbuf * dst_pixbuf = NULL;
    GtkWidget * image;
    GtkWidget * align;


    connect_button = create_button("./icon/btn_green.png", "./icon/btn_green_h.png", NULL, text,
        -1, -1, G_CALLBACK(callback), dialog);
    set_widget_font_size(connect_button->label, -1, "Snow");

    btn_hbox = gtk_hbox_new(FALSE, 30);
    gtk_box_pack_start(GTK_BOX(btn_hbox), connect_button->container, FALSE, FALSE, 5);
    gtk_widget_show(btn_hbox);

    align = gtk_alignment_new(0.5, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), btn_hbox);
    gtk_widget_show(align);

    src_pixbuf = gdk_pixbuf_new_from_file("./icon/shadow.png", NULL);
    dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, 445, 5, GDK_INTERP_BILINEAR);
    image = gtk_image_new_from_pixbuf(dst_pixbuf);
    g_object_unref(src_pixbuf);
    g_object_unref(dst_pixbuf);
    gtk_widget_show(image);

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), image, FALSE, FALSE, 6);
    gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 5);
    gtk_widget_show(vbox);

    return vbox;
}

static void ui_dialog_authpwd_adapt(void)
{
    if (dialog) {
        if (ui_dialog_wifi.status == UI_STATUS_SHOW) {
            logi("ui_dialog_wifi_adapt\n");
            gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
        }
    }
}

static void ui_dialog_authpwd_destroy()
{
    if (dialog) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }

    if (auth_remind_text_timer) {
        g_source_remove(auth_remind_text_timer);
        auth_remind_text_timer = 0;
    }

    bubble_normal_show(UI_AUTH_STATUS_REQUEST);
    ui_extern_cancel_auth_config();
}

static int ui_dialog_authpwd_ctrl(void *data)
{
    ui_msg_t *msg = data;

    if (!dialog || !msg) {
        return 0;
    }

    logi("%s status %d\n", __func__, msg->sub_obj);
    switch (msg->sub_obj) {
    case UI_NET_AUTHSTATUS_UNNECESSARY:
    case UI_NET_AUTHSTATUS_AUTHING:
    case UI_NET_AUTHSTATUS_FAIL:
    case UI_TIPS_AUTHSTATUS_DISCONNECT:
    case UI_NET_AUTHSTATUS_OTHER:
    case UI_TIPS_AUTHSTATUS_USER_ERR:
    case UI_TIPS_AUTHSTATUS_TIMEOUT:
    case UI_TIPS_AUTHSTATUS_USERPASSWD_BLAKE:
        ui_dialog_authpwd_reflash(msg->sub_obj);
        break;
    case UI_NET_AUTHSTATUS_SUCCESS:
        ui_dialog_authpwd_destroy();
        break;
     default:
        ui_dialog_authpwd_reflash(UI_NET_AUTHSTATUS_FAIL);
        break;
    }

    return 0;
}

static void ui_dialog_authpwd_hide(void)
{
    if(dialog != NULL) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
    bubble_normal_show(UI_AUTH_STATUS_REQUEST);
}

static void ui_dialog_authpwd_show(void)
{
    if (dialog == NULL) {
        ui_dialog_authpwd_init();
    }

    gtk_widget_show(dialog);
}

static int ui_dialog_authpwd_init(void)
{
    int auth_status = 0;
    GtkWidget *title_bar = NULL;
    GtkWidget *enter_area = NULL;
    authconnect_remind_area = NULL;
    auth_remind_text_area = NULL;
    authusername = NULL;
    authpasswd = NULL;

    char title[128] = "网络认证设置";
    pwd_visibility = FALSE;
    dialog = gtk_dialog_new();
    g_signal_connect(G_OBJECT(dialog), "realize", 
                      G_CALLBACK(ui_compt_realize), &ui_dialog_authpwd);
    gtk_widget_set_size_request(dialog, 464, 323);

    ui_dialog_white_background(dialog);
    update_widget_bg(dialog, "./icon/auth/auth_bg.png", -1, -1);
    gtk_window_set_decorated(GTK_WINDOW(dialog),FALSE);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

    title_bar = mk_title_bar(title, dialog);
    enter_area = create_authpwd_dialog();

    auth_status = ui_extern_get_auth_status();
    ui_authentry_load(auth_status);
    //TODO:show different ui according to authing status
    ui_dialog_authbtn_load(auth_status);

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), title_bar, FALSE, FALSE, 15);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), enter_area, FALSE, FALSE, 5);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);

    return 0;
}

struct ui_comp_s ui_dialog_authpwd = {
    .type = UI_TYPE_DIALOG_AUTHPWD,
    .subtype = UI_TOPLEVEL_TYPE_DIALOG,
    .is_init = UI_NOT_NEED_INIT,
    .widget = &dialog,
    .init = ui_dialog_authpwd_init,
    .show = ui_dialog_authpwd_show,
    .hide = ui_dialog_authpwd_hide,
    .ctrl = ui_dialog_authpwd_ctrl,
    .destroy = ui_dialog_authpwd_destroy,
    .adapt = ui_dialog_authpwd_adapt,
};

