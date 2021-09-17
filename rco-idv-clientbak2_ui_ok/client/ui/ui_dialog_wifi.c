#include "ui_main.h"
#include "ui_dialog_wifi.h"
#include "ui_dialog_prop.h"

#define WIFI_DISENABLE "无线未启用"
#define NET_DISCONNECT "未连接任何网络"
#define WIFI_CONNECT "已连接无线网络"
#define ETH_CONNECT "已连接有线网络"
#define WHFI_UNCHECK_SSID "未检测到授权无线网络，请重新打开窗口或联系管理员"

#define HIDE_NET "其他网络"
#define BTN_OFF "关"
#define BTN_ON "开"
#define BTN_DISCONNECT "断开"
#define BTN_CONNECT "连接"
#define SSID_FONT_SIZE 12
#define SSID_MAX_NUM    3
static GtkWidget *dialog = NULL;

//area后缀的控件保证控件位置正确
static GtkWidget *netstatus_blank_area;
static GtkWidget *netstatus_blank;
static GtkWidget *netstatus_bar;
static GtkWidget *netstatus_area;
static GtkWidget *wifistatus_bar;
static GtkWidget *wifistatus_area;
static GtkWidget *wifienable_btn;
static GtkWidget *wifienable_area;
static GtkWidget *wifi_list;
static GtkWidget *viable_table;
static GtkWidget *list_view;
static btn_apend_t  *wifi_switch;

static int switch_status;

static gint ui_dialog_button_click_prop(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    ui_msg_t msg;

    logi("ui_dialog_button_click_prop: enter\n");

    if(data != NULL) {
        /*显示未连接WiFi的属性*/
        msg.sub_obj = UI_DIALOG_PROP_USABLE;
        msg.args = data;

        ui_tab[UI_TYPE_DIALOG_PROP]->ctrl(&msg);
        ui_tab[UI_TYPE_DIALOG_PROP]->show();
    } else {
        logi("ui_dialog_button_click_prop: data is null!\n");
    }

    return TRUE;
}

static gint ui_dialog_button_click_prop1(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    ui_msg_t msg;

    logi("ui_dialog_button_click_prop1: enter\n");

    if(data != NULL) {
        /*获取已连接WiFi信息*/
        msg.sub_obj = UI_DIALOG_PROP_CONNECT;
        msg.args = data;

        ui_tab[UI_TYPE_DIALOG_PROP]->ctrl(&msg);
        ui_tab[UI_TYPE_DIALOG_PROP]->show();
    } else {
        logi("ui_dialog_button_click_prop1: data is null!\n");
    }

    return TRUE;
}

static gint ui_dialog_button_click_prop2(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    ui_msg_t msg;

    /*获取已连接ETH信息*/
    msg.sub_obj = UI_DIALOG_PROP_ETH;

    ui_tab[UI_TYPE_DIALOG_PROP]->ctrl(&msg);
    ui_tab[UI_TYPE_DIALOG_PROP]->show();

    return TRUE;
}

static gint ui_dialog_button_click_switch(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    if(switch_status == TRUE ) {
        /*开启无线网卡接口*/
        ui_extern_wireless_netcard_enable(TRUE);
    } else {
        /*关闭无线网卡接口*/
        ui_extern_wireless_netcard_enable(FALSE);
    }

    return TRUE;
}

static gint ui_dialog_button_click_link(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    ui_scanresult *scanresult;

    scanresult = (ui_scanresult *)data;

    if(data != NULL) {
        /* 调用连接无线网络接口 */
        ui_extern_wireless_netcard_connect(FALSE, scanresult);
    } else {
        logi("ui_dialog_button_click_link: data is null!\n");
    }

    return TRUE;
}

static gint ui_dialog_button_click_link1(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    /* 调用连接隐藏网络接口 */
    ui_extern_wireless_netcard_connect(TRUE, NULL);

    return TRUE;
}

static gint ui_dialog_button_click_down(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    /* 调用断开无线网络接口           */
    ui_extern_wireless_netcard_disconnect();

    return TRUE;
}

static void set_signal_icon(int indensity, unsigned int key_mgmt, char *icon_path)
{
    if(indensity >= -100 && indensity < -85) {
        if(key_mgmt == 0) {
            strcpy(icon_path, "./icon/wireless/wifi_signal_1.png");
        } else {
            strcpy(icon_path, "./icon/wireless/wifi_signal_k1.png");
        }
    } else if(indensity >= -85 && indensity < -70) {
        if(key_mgmt == 0) {
            strcpy(icon_path, "./icon/wireless/wifi_signal_2.png");
        } else {
            strcpy(icon_path, "./icon/wireless/wifi_signal_k2.png");
        }
    } else if(indensity >= -70 && indensity < -55) {
        if(key_mgmt == 0) {
            strcpy(icon_path, "./icon/wireless/wifi_signal_3.png");
        } else {
            strcpy(icon_path, "./icon/wireless/wifi_signal_k3.png");
        }
    } else if(indensity >= -55 && indensity <= 0) {
        if(key_mgmt == 0) {
            strcpy(icon_path, "./icon/wireless/wifi_signal_4.png");
        } else {
            strcpy(icon_path, "./icon/wireless/wifi_signal_k4.png");
        }
    } else {
        strcpy(icon_path, "./icon/wireless/wifi_signal_0.png");
        logi("Invalid signal strength!\n");
    }
}

static GtkWidget *create_connect_prompt(char *icon, char *text, int is_eth)
{
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *p_image;
    GtkWidget *p_label;
    GdkPixbuf *src_pixbuf = NULL;

    vbox = gtk_vbox_new(FALSE, 20);

    src_pixbuf = gdk_pixbuf_new_from_file(icon, NULL);
    p_image = gtk_image_new_from_pixbuf(src_pixbuf);
    g_object_unref(src_pixbuf);

    hbox = gtk_hbox_new(FALSE, 0);

    p_label = gtk_label_new(text);
    gtk_widget_set_size_request(p_label, 160, -1);
    gtk_label_set_justify(GTK_LABEL(p_label), GTK_JUSTIFY_LEFT);
    set_widget_font_size(p_label, 16, "#666666");
    gtk_box_pack_start(GTK_BOX(hbox), p_label, FALSE, FALSE, 0);

    if(is_eth) {
        GtkWidget *label = gtk_label_new(NULL);
        gtk_widget_set_size_request(label, -1, -1);
        gtk_label_set_markup(GTK_LABEL(label),
                "<span foreground='#3ECACB' underline='single' underline_color='#3ECACB' font_desc='16'>查看属性</span>");
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

        GtkWidget *eventbox = gtk_event_box_new();
        gtk_event_box_set_visible_window(GTK_EVENT_BOX(eventbox), FALSE);
        gtk_container_add(GTK_CONTAINER(eventbox), label);
        g_signal_connect(G_OBJECT(eventbox), "button_release_event", G_CALLBACK(ui_dialog_button_click_prop2), NULL);
        g_signal_connect(G_OBJECT(eventbox),"enter_notify_event",G_CALLBACK(label_mouse_handle), NULL);
        g_signal_connect(G_OBJECT(eventbox),"leave_notify_event",G_CALLBACK(label_mouse_handle), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), eventbox, FALSE, FALSE, 0);

        gtk_widget_show(label);
        gtk_widget_show(eventbox);
    }

    GtkWidget *align = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

    gtk_box_pack_start(GTK_BOX(vbox), p_image, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 0);

    gtk_widget_show(hbox);
    gtk_widget_show(vbox);
    gtk_widget_show(p_image);
    gtk_widget_show(p_label);

    return vbox;
}

GtkWidget *create_net_status_prompt(int net_status)
{
    GtkWidget *align;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *wifi_prompt;

    vbox = gtk_vbox_new(FALSE, 0);

    label = gtk_label_new("");
    gtk_widget_set_size_request(label, 1, 30);
    gtk_widget_show(label);

    if(net_status == UI_DIALOG_WIFI_CONNECT) {
        wifi_prompt = create_connect_prompt("./icon/wireless/connect_eth_success.png", WIFI_CONNECT, FALSE);
    } else if(net_status == UI_DIALOG_ETH_CONNECT) {
        wifi_prompt = create_connect_prompt("./icon/wireless/connect_eth_success.png", ETH_CONNECT, TRUE);
    } else {
        wifi_prompt = create_connect_prompt("./icon/wireless/connect_fail.png", NET_DISCONNECT, FALSE);
    }

    gtk_box_pack_start(GTK_BOX(vbox), wifi_prompt, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
    gtk_widget_show(vbox);

    align = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), vbox);
    gtk_widget_show(align);

    return align;
}

GtkWidget *mk_net_status_prompt(int net_status)
{
    GtkWidget *align;

    netstatus_bar = create_net_status_prompt(net_status);

    align = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), netstatus_bar);
    gtk_widget_show(align);

    return align;
}

static GtkWidget *create_connect_bar(char *signal_icon, char *ssid, int show_prop, ui_wifiinfo *connect_info, char *btn_icon, char *mouse_icon, int is_effect)
{
    GtkWidget *hbox;
    GtkWidget *fixed;
    GtkWidget *align;
    GtkWidget *label;
    GtkWidget *b_image;
    GtkWidget *bg_img;
    GtkWidget *b_label;
    btn_apend_t *wifi_button;
    btn_apend_t *prop_button;

    GdkPixbuf *src_pixbuf = NULL;
    int	set_height;

    hbox = gtk_hbox_new(FALSE, 5);

    src_pixbuf = gdk_pixbuf_new_from_file(signal_icon, NULL);
    set_height = gdk_pixbuf_get_width(src_pixbuf);
    b_image = gtk_image_new_from_pixbuf(src_pixbuf);
    g_object_unref(src_pixbuf);

    b_label = gtk_label_new(ssid);
    gtk_widget_set_size_request(b_label, 155, set_height);
    gtk_misc_set_alignment(GTK_MISC(b_label), 0, 0.5);
    set_widget_font_size(b_label, SSID_FONT_SIZE, "#3ECACB");

    gtk_box_pack_start(GTK_BOX(hbox), b_image, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), b_label, FALSE, FALSE, 0);

    label = gtk_label_new("");
    gtk_widget_show(label);

    if(show_prop) {
        prop_button = create_button("./icon/wireless/btn_prop.png", "./icon/wireless/btn_prop.png", NULL,
                NULL, -1, -1, GTK_SIGNAL_FUNC(ui_dialog_button_click_prop1), connect_info);
        gtk_box_pack_start(GTK_BOX(hbox), prop_button->container, FALSE, FALSE, 0);

        gtk_widget_set_size_request(label, 10, set_height);
    } else {
        gtk_widget_set_size_request(label, 10+18+5, set_height);
    }
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    wifi_button = create_button(btn_icon, mouse_icon, NULL,
            BTN_DISCONNECT, -1, -1, GTK_SIGNAL_FUNC(ui_dialog_button_click_down), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), wifi_button->container, FALSE, FALSE, 0);

    if(is_effect) {
        set_widget_font_size(wifi_button->label, -1, "Snow");
        set_btn_effect(wifi_button, TRUE);
    } else {
        set_widget_font_size(wifi_button->label, -1, "#999999");
        set_btn_effect(wifi_button, FALSE);
    }

    fixed = gtk_fixed_new();
    bg_img = gtk_image_new_from_file("./icon/wireless/bar_wifi_connect.png");
    gtk_fixed_put(GTK_FIXED(fixed), bg_img, 0, 0);
    gtk_fixed_put(GTK_FIXED(fixed), hbox, 10, 6);

    align = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), fixed);
    gtk_widget_show(align);

    gtk_widget_show(hbox);
    gtk_widget_show(fixed);
    gtk_widget_show(b_image);
    gtk_widget_show(bg_img);
    gtk_widget_show(b_label);

    return align;
}

GtkWidget *create_wifi_status_bar(int is_connect, char *icon, char *text, ui_wifiinfo *info)
{
    GtkWidget *align;
    GtkWidget *wifi_status;

    if(is_connect) {
        wifi_status = create_connect_bar(icon, text, TRUE, info,
                "./icon/wireless/btn_green_80x26.png", "./icon/wireless/btn_green_80x26_h.png", TRUE);
    } else {
        wifi_status = create_connect_bar("./icon/wireless/wifi_signal_0.png", "", FALSE, NULL,
                "./icon/wireless/btn_grey_80x26.png", "./icon/wireless/btn_grey_80x26_h.png", FALSE);
    }

    align = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), wifi_status);
    gtk_widget_show(align);

    return align;
}

GtkWidget *mk_wifi_status_bar(int is_connect, char *icon, char *text, ui_wifiinfo *info)
{
    GtkWidget *align;

    wifistatus_bar = create_wifi_status_bar(is_connect, icon, text, info);

    align = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), wifistatus_bar);
    gtk_widget_show(align);

    return align;
}

static GtkWidget *create_wifi_item(int is_hidenet, char* icon, char *text, char *button_label, ui_scanresult *prop_info)
{
    GtkWidget *align;
    GtkWidget *hbox;
    GtkWidget *image;
    GtkWidget *label;
    GtkWidget *_label;
    btn_apend_t *wifi_button;
    btn_apend_t *prop_button;
    GdkPixbuf *src_pixbuf = NULL;
    int	set_height;

    hbox = gtk_hbox_new(FALSE, 5);

    src_pixbuf = gdk_pixbuf_new_from_file(icon, NULL);
    set_height = gdk_pixbuf_get_width(src_pixbuf);
    image = gtk_image_new_from_pixbuf(src_pixbuf);
    g_object_unref(src_pixbuf);

    _label = gtk_label_new("");
    gtk_widget_set_size_request(_label, 10, set_height);
    gtk_widget_show(_label);

    label = gtk_label_new(text);
    gtk_widget_set_size_request(label, 135, set_height);
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    set_widget_font_size(label, SSID_FONT_SIZE, "#666666");

    gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    if(is_hidenet) {
        gtk_widget_set_size_request(label, 135+18+5, set_height);
        wifi_button = create_button("./icon/wireless/btn_green_80x26.png", "./icon/wireless/btn_green_80x26_h.png", NULL,
        button_label, -1, -1, GTK_SIGNAL_FUNC(ui_dialog_button_click_link1), NULL);
    } else {
        prop_button = create_button("./icon/wireless/btn_prop.png", "./icon/wireless/btn_prop.png", NULL,
		NULL, -1, -1, GTK_SIGNAL_FUNC(ui_dialog_button_click_prop), prop_info);
        gtk_box_pack_start(GTK_BOX(hbox), prop_button->container, FALSE, FALSE, 0);

        wifi_button = create_button("./icon/wireless/btn_green_80x26.png", "./icon/wireless/btn_green_80x26_h.png", NULL,
        button_label, -1, -1, GTK_SIGNAL_FUNC(ui_dialog_button_click_link), prop_info);
    }
    set_widget_font_size(wifi_button->label, -1, "Snow");

    gtk_box_pack_start(GTK_BOX(hbox), _label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), wifi_button->container, FALSE, FALSE, 0);

    align = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), hbox);

    gtk_widget_show(hbox);
    gtk_widget_show(align);
    gtk_widget_show(image);
    gtk_widget_show(label);

    return align;
}

static GtkWidget *create_wifi_list(ui_scanresult *list, int length)
{
    int _section;
    GtkWidget *_list;
    GtkWidget *_item;
    GtkWidget *_align;
    GtkWidget *_label;
    GtkWidget *label_ssid;
    GtkWidget *hide_item;
    char signal_icon[128]; 

    _list = gtk_vbox_new(FALSE, 0);

    _label = gtk_label_new("");
    gtk_widget_set_size_request(_label, 1, 6);
    gtk_widget_show(_label);
    gtk_box_pack_start(GTK_BOX(_list), _label, FALSE, FALSE, 0);

    for(_section = 0; _section < length; _section++) {
        set_signal_icon(list[_section].idensity, list[_section].key_mgmt, signal_icon);
        _item = create_wifi_item(FALSE, signal_icon, list[_section].ssid, BTN_CONNECT, &list[_section]);
        gtk_box_pack_start(GTK_BOX(_list), _item, FALSE, FALSE, 6);
    }

    if (list == NULL) {
        if (ui_extern_get_whitelist_num() > 0) {
            label_ssid = gtk_label_new(WHFI_UNCHECK_SSID);
            gtk_misc_set_alignment(GTK_MISC(label_ssid), 0, 0.5);
            gtk_widget_set_size_request(label_ssid, 150, 30);
            set_widget_font_size(label_ssid, 9, "#666666");
            gtk_widget_show(label_ssid);
            gtk_box_pack_start(GTK_BOX(_list), label_ssid, FALSE, FALSE, 6);
        }
    }
    
    hide_item = create_wifi_item(TRUE, "./icon/wireless/wifi_signal_0.png", HIDE_NET, BTN_CONNECT, NULL);
    gtk_box_pack_start(GTK_BOX(_list), hide_item, FALSE, FALSE, 6);

    _align = gtk_alignment_new(0.5, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(_align), _list);

    gtk_widget_show(_list);
    gtk_widget_show(_align);

    return _align;
}

static GtkWidget *mk_viable_table(ui_scanresult *list, int length)
{
    GtkWidget *scrolled_window;
    GtkWidget *table;
    GtkWidget *bg_image;
    GtkWidget *align;

    wifi_list = create_wifi_list(list, length);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), GTK_SHADOW_NONE);
    gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    list_view = gtk_viewport_new(NULL, NULL);
    gtk_viewport_set_shadow_type(GTK_VIEWPORT(list_view), GTK_SHADOW_NONE);
    ui_dialog_white_background(list_view);
    gtk_widget_show(list_view);
    gtk_widget_set_size_request(scrolled_window, 332, 173);

    gtk_container_add(GTK_CONTAINER(list_view), wifi_list);
    gtk_container_add(GTK_CONTAINER(scrolled_window), list_view);
    gtk_widget_show(scrolled_window);

    table = gtk_fixed_new();
    bg_image = gtk_image_new_from_file("./icon/wireless/frame_wifi_list.png");
    gtk_fixed_put(GTK_FIXED(table), bg_image, 0, 0);
    gtk_fixed_put(GTK_FIXED(table), scrolled_window, 3, 0);
    gtk_widget_show(table);
    gtk_widget_show(bg_image);

    align = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), table);
    gtk_widget_show(align);

    return align;
}

static GtkWidget *create_hide_label(int sw_mode)
{
    GtkWidget *vbox;

    vbox = gtk_vbox_new(FALSE, 0);

    netstatus_blank = gtk_label_new("         ");
    switch (sw_mode) {
    case UI_WIFI_DISABLE_MODE:
        //gtk_widget_set_size_request(netstatus_blank, -1, 80);
        gtk_widget_set_size_request(netstatus_blank, -1, 80+32);
        break;
    case UI_WIFI_ENABLE_MODE:
        gtk_widget_set_size_request(netstatus_blank, -1, 20);
        break;
    case UI_NONE_WIFI_CLIENT:
        gtk_widget_set_size_request(netstatus_blank, -1, 80+32);
        break;
    default:
        break;
    }
    gtk_widget_show(netstatus_blank);
    gtk_box_pack_start(GTK_BOX(vbox), netstatus_blank, FALSE, FALSE, 0);
    gtk_widget_show(vbox);

    return vbox;
}

static GtkWidget *mk_hide_label_area(int sw_mode)
{
    GtkWidget *align;

    netstatus_blank = create_hide_label(sw_mode);
    align = gtk_alignment_new(0.9, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), netstatus_blank);
    gtk_widget_show(align);

    return align;
}

static GtkWidget *create_switch_button(int sw_mode)
{
    GtkWidget *vbox;
    GtkWidget *hide_label;

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox);

    hide_label = gtk_label_new("");
    gtk_widget_show(hide_label);

    if(sw_mode == UI_WIFI_ENABLE_MODE) {
        wifi_switch = create_button("./icon/wireless/switch_open.png", "./icon/wireless/switch_open.png", NULL, NULL,
                -1, -1, GTK_SIGNAL_FUNC(ui_dialog_button_click_switch), NULL);
        switch_status = FALSE;
        gtk_widget_set_size_request(hide_label, -1, -1);
        gtk_box_pack_start(GTK_BOX(vbox), wifi_switch->container, FALSE, FALSE, 0);
    } else if(sw_mode == UI_WIFI_DISABLE_MODE) {
        wifi_switch = create_button("./icon/wireless/switch_close.png", "./icon/wireless/switch_close.png", NULL, NULL,
                -1, -1, GTK_SIGNAL_FUNC(ui_dialog_button_click_switch), NULL);
        switch_status = TRUE;
        gtk_widget_set_size_request(hide_label, -1, 80);
        gtk_box_pack_start(GTK_BOX(vbox), wifi_switch->container, FALSE, FALSE, 0);
    } else {
        gtk_widget_set_size_request(hide_label, -1, 80+32);
    }

    gtk_box_pack_start(GTK_BOX(vbox), hide_label, FALSE, FALSE, 0);

    return vbox;
}

static GtkWidget *mk_switch_area(int is_enable)
{
    GtkWidget *align;

    wifienable_btn = create_switch_button(is_enable);

    align = gtk_alignment_new(0.9, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), wifienable_btn);
    gtk_widget_show(align);

    return align;
}

static GtkWidget *mk_action_area(void)
{
	GtkWidget * vbox;
	GtkWidget * btn_hbox;
	GdkPixbuf * src_pixbuf = NULL;
	GdkPixbuf * dst_pixbuf = NULL;
	GtkWidget * image;
	GtkWidget * align;
    btn_apend_t  *cancal_button;

    cancal_button = create_button("./icon/btn_green.png", "./icon/btn_green_h.png", NULL, "返回",
    		-1, -1, G_CALLBACK(ui_destory_wifi_config_callback_notify), dialog);
    set_widget_font_size(cancal_button->label, -1, "Snow");

    btn_hbox = gtk_hbox_new(FALSE, 30);
    gtk_box_pack_start(GTK_BOX(btn_hbox), cancal_button->container, FALSE, FALSE, 5);
    gtk_widget_show(btn_hbox);

	align = gtk_alignment_new(0.5, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), btn_hbox);
    gtk_widget_show(align);

	src_pixbuf = gdk_pixbuf_new_from_file("./icon/shadow.png", NULL);
    dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, 830, 10, GDK_INTERP_BILINEAR);
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

void ui_show_netstatus_blank(int sw_mode)
{
    if (dialog == NULL) {
        return;
    }

    if (netstatus_blank_area != NULL) {
        if (netstatus_blank != NULL) {
            gtk_widget_destroy(netstatus_blank);
            netstatus_blank = NULL;
        }
        netstatus_blank = create_hide_label(sw_mode);
        gtk_container_add(GTK_CONTAINER(netstatus_blank_area), netstatus_blank);
        gtk_widget_show(netstatus_blank_area);

        return;
    }

    netstatus_blank_area = mk_hide_label_area(sw_mode);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), netstatus_blank_area, FALSE, FALSE, 0);

    return;
}


void ui_show_network_status(int net_status)
{
    if(dialog == NULL) {
        return;
    }

    if(netstatus_area != NULL) {
        if(netstatus_bar != NULL) {
            gtk_widget_destroy(netstatus_bar);
            netstatus_bar = NULL;
        }
        netstatus_bar = create_net_status_prompt(net_status);
        gtk_container_add(GTK_CONTAINER(netstatus_area), netstatus_bar);
        gtk_widget_show(netstatus_area);

        return;
    }

    netstatus_area = mk_net_status_prompt(net_status);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), netstatus_area, FALSE, FALSE, 0);

    return;
}

void ui_show_wifi_status(int is_connect, char * icon, char * text, ui_wifiinfo *info)
{
    if(dialog == NULL) {
        return;
    }

    if(wifistatus_area != NULL) {
        if(wifistatus_bar != NULL) {
            gtk_widget_destroy(wifistatus_bar);
            wifistatus_bar = NULL;
        }
        wifistatus_bar = create_wifi_status_bar(is_connect, icon, text, info);
        gtk_container_add(GTK_CONTAINER(wifistatus_area), wifistatus_bar);
        gtk_widget_show(wifistatus_area);

        return;
    }

    wifistatus_area = mk_wifi_status_bar(is_connect, icon, text, info);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), wifistatus_area, FALSE, FALSE, 0);

    return;
}

void ui_show_wifi_table(ui_scanresult *list, int length)
{
    if(dialog == NULL) {
        return;
    }

    if(viable_table != NULL) {
        if (wifi_list != NULL) {
            gtk_widget_destroy(wifi_list);
            wifi_list = NULL;
        }
        wifi_list = create_wifi_list(list, length);
        gtk_container_add(GTK_CONTAINER(list_view), wifi_list);
        gtk_widget_show(viable_table);

        return;
    }

    viable_table = mk_viable_table(list, length);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), viable_table, FALSE, FALSE, 0);

    return;
}

void ui_show_enable_status(int sw_mode)
{
    if(dialog == NULL) {
        return;
    }

    /*refresh enable button*/
    if(wifienable_area != NULL) {
        if(wifienable_btn != NULL) {
            gtk_widget_destroy(wifienable_btn);
            wifienable_btn = NULL;
        }
        wifienable_btn = create_switch_button(sw_mode);
        gtk_container_add(GTK_CONTAINER(wifienable_area), wifienable_btn);
        gtk_widget_show(wifienable_area);

        return;
    }
    /*init enable button*/
    wifienable_area = mk_switch_area(sw_mode);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), wifienable_area, FALSE, FALSE, 0);

    return;
}

static void ui_dialog_wifi_refresh(void *args)
{
    ui_wifi_dialog_t *data= args;
    ui_wifiinfo *info = data->info;
    ui_scanresult *list = data->list;
    int length = data->length;

    char signal_icon[128];

#if 0
    /*调用接口：获取当前wifi连接信息*/
    info = ui_extern_wifi_status_query_result();
    /*调用接口：获取当前wifi列表信息*/
    list = ui_extern_wifi_list_scan_result(&length);
#endif

    if(info != NULL) {
        set_signal_icon(info->idensity, info->key_mgmt, signal_icon);
        ui_show_wifi_status(TRUE, signal_icon, info->ssid, info);
    } else {
        ui_show_wifi_status(FALSE, NULL, NULL, NULL);
    }

    if(list !=NULL) {
        ui_show_wifi_table(list, length);
    } else {
        ui_show_wifi_table(NULL, 0);
        logi("list is null!\n");
    }

    return;
}

static void ui_dialog_wifi_open(void *args)
{
   // ui_show_enable_status(TRUE);
    ui_show_netstatus_blank(UI_WIFI_ENABLE_MODE);
    ui_dialog_wifi_refresh(args);

    return;
}

static void ui_dialog_wifi_close(void)
{
    //ui_show_enable_status(FALSE);
    ui_show_netstatus_blank(UI_WIFI_DISABLE_MODE);
    //ui_show_wifi_status(FALSE, NULL, NULL, NULL);

    if (wifistatus_area != NULL) {
        gtk_widget_destroy(wifistatus_area);
        wifistatus_area = NULL;
    }

    if (viable_table != NULL) {
        gtk_widget_destroy(viable_table);
        viable_table = NULL;
    }
    return;
}

static void ui_dialog_net_change(void *args)
{
    if(args == NULL) {
        return;
    }

    int status = *((int *)args);
    ui_show_network_status(status);

    return;
}

static void ui_dialog_wifi_load(void)
{
    int result;
    int status;
    int wifi_terminal;

    /*判断是不是WIFI终端*/
    ui_extern_is_wifi_terminal(&wifi_terminal);
    if(!wifi_terminal)
    {
        /*调用接口：查询网络状态*/
        status = ui_extern_net_status_query_result();
        /*如果不是则隐藏wifi开关*/
        //ui_show_enable_status(UI_NONE_WIFI_CLIENT);
        ui_show_netstatus_blank(UI_NONE_WIFI_CLIENT);
        ui_show_network_status(status);

        return;
    }

    /*调用接口：查询wifi是否使能*/
    result = ui_extern_wireless_netcard_query_result();
    /*调用接口：查询网络状态*/
    status = ui_extern_net_status_query_result();

    if(!result) {
        //ui_show_enable_status(UI_WIFI_DISABLE_MODE);
        ui_show_netstatus_blank(UI_WIFI_DISABLE_MODE);
        ui_show_network_status(status);
        //ui_show_wifi_status(FALSE, NULL, NULL, NULL);

        return;
    }

    ui_wifi_dialog_t data;
    ui_wifiinfo *info = NULL;
    ui_scanresult *list = NULL;
    int length = 0;

    /*调用接口：获取当前wifi连接信息*/
    info = ui_extern_wifi_status_query_result();
    /*调用接口：获取当前wifi列表信息*/
    list = ui_extern_wifi_list_scan_result(&length);

    data.info = info;
    data.list = list;
    data.length = length;

    //ui_show_enable_status(UI_WIFI_ENABLE_MODE);
    ui_show_netstatus_blank(UI_WIFI_ENABLE_MODE);
    ui_show_network_status(status);
    ui_dialog_wifi_refresh(&data);

    return;
}

static int ui_dialog_wifi_init(void)
{
	GtkWidget *title_bar;
    GtkWidget *action_area;
    GtkWidget *label1;
    GtkWidget *wifi_tips;

    netstatus_blank_area = NULL;
    wifienable_area = NULL;
    netstatus_area = NULL;
    wifistatus_area = NULL;
    viable_table = NULL;

    dialog = gtk_dialog_new();
    g_signal_connect (G_OBJECT(dialog), "realize",
                      G_CALLBACK(ui_compt_realize), &ui_dialog_wifi);
    ui_dialog_white_background(dialog);

    title_bar = title_bar_init("网络设置", dialog, 3);

    action_area = mk_action_area();
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area), action_area);

    label1 = gtk_label_new("");
    gtk_widget_set_size_request(label1, 1, 40);
    gtk_widget_show(label1);

    wifi_tips = gtk_label_new("当前正在进行网络配置，无法进行其他操作");
    set_widget_font_size(wifi_tips, 9, "#999999");
    gtk_widget_show(wifi_tips);    

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), title_bar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label1, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(GTK_DIALOG(dialog)->vbox), wifi_tips, FALSE, FALSE, 0);

    ui_dialog_wifi_load();

    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_decorated(GTK_WINDOW(dialog), FALSE);
    gtk_widget_set_size_request(dialog, 850, 580);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
    return 0;
}

static void ui_dialog_wifi_hide(void)
{
    if(dialog != NULL) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }

    return;
}

static void ui_dialog_wifi_show(void)
{
    if (dialog != NULL) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
    ui_dialog_wifi_init();
    gtk_widget_show(dialog);

    return;
}

static int ui_dialog_wifi_ctrl(void *data)
{
    ui_msg_t *msg = data;

    switch (msg->sub_obj) {
	case UI_DIALOG_WIFI_ENABLE:
        ui_dialog_wifi_open(msg->args);
        break;
    case UI_DIALOG_WIFI_DISABLE:
        ui_dialog_wifi_close();
        break;
    case UI_DIALOG_WIFI_REFRESH:
        ui_dialog_wifi_refresh(msg->args);
        break;
    case UI_DIALOG_NET_CHANGE:
        ui_dialog_net_change(msg->args);
        break;
    }

    return 0;
}

static void ui_dialog_wifi_adapt(void)
{
    if (dialog) {
        if (ui_dialog_wifi.status == UI_STATUS_SHOW) {
            logi("ui_dialog_wifi_adapt\n");
            gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
        }
    }
}

static void ui_dialog_wifi_destroy(void)
{
    if (dialog) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
}

struct ui_comp_s ui_dialog_wifi = {
    .type = UI_TYPE_DIALOG_WIFI,
    .subtype = UI_TOPLEVEL_TYPE_DIALOG,
    .is_init = UI_NOT_NEED_INIT,
    .widget = &dialog,
    .init = ui_dialog_wifi_init,
    .hide = ui_dialog_wifi_hide,
    .show = ui_dialog_wifi_show,
    .ctrl = ui_dialog_wifi_ctrl,
    .adapt = ui_dialog_wifi_adapt,
    .destroy = ui_dialog_wifi_destroy,
};
