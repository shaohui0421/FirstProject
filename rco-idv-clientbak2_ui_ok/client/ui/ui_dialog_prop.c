#include "ui_main.h"
#include "ui_dialog_wifi.h"
#include "ui_dialog_prop.h"
#include "../../include/application_c_interfaces.h"
#include "rc/rc_netif.h"
#include "rc/rc_checknetifval.h"

#define FONT_SIZE 12
#define WIDTH 380

static GtkWidget *prop = NULL;
static btn_apend_t *forget_btn;
//static GtkWidget *forget_tip;
static GtkWidget *label;
static GtkWidget *credential;
static btn_apend_t *eth_config;

static int mode = 0;
static void *p_info = NULL;
static int net_id;

static gint ui_ethernet_net_config_handle(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    ui_tab[UI_TYPE_DIALOG_ADMINPWD]->show();
    ui_tab[UI_TYPE_DIALOG_WIFI]->hide();
    ui_tab[UI_TYPE_DIALOG_PROP]->hide();

    return TRUE;
}

static gint ui_forget_net_enter_handle(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    int result = ui_extern_wifi_forget_net_handle(net_id);

    if(result != TRUE) {
        logi("ui_extern_wifi_forget_net_handle: execute failed!\n");
        return FALSE;
    }

    gtk_widget_set_size_request(label, WIDTH, 26);
    gtk_widget_hide(forget_btn->container);
    //gtk_widget_hide(forget_tip);
    gtk_label_set_text(GTK_LABEL(credential), "未保存");

    return TRUE;
}
/*
static gint ui_dialog_forget_net_handle(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    //TODO : 弹出提示框
    l2u_show_dialog_copy_base(1);

    return TRUE;
}
*/
static GtkWidget *mk_title_bar(char *title, void *data)
{
	GtkWidget *title_bar;
    GtkWidget *label1;
	btn_apend_t *button_quit;

	title_bar = gtk_hbox_new(FALSE, 0);
    label1 = gtk_label_new(title);
    set_widget_font_size(label1, FONT_SIZE, "#666666");
    gtk_widget_set_size_request(label1, 460-25-35, 20);
    gtk_misc_set_alignment(GTK_MISC(label1), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(title_bar), label1, FALSE, FALSE, 10);

    button_quit = create_button("./icon/btn_close.png", "./icon/btn_close_h.png", NULL, NULL, -1, -1, G_CALLBACK(ui_destory_dialog_callback), data);
    gtk_box_pack_end(GTK_BOX(title_bar), button_quit->container, FALSE, FALSE, 5);

    gtk_widget_show(label1);
	gtk_widget_show(title_bar);

	return title_bar;
}

static GtkWidget *mk_function_bar(int is_wifi)
{
    GtkWidget *hbox;
    GtkWidget *align;

    hbox = gtk_hbox_new(FALSE, 0);

    label = gtk_label_new("属性");
    set_widget_font_size(label, FONT_SIZE, "#666666");
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_widget_set_size_request(label, WIDTH, 20);
    gtk_widget_show(label);

    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    if(is_wifi) {
        if(net_id >= 0) {
            //gtk_widget_set_size_request(label, WIDTH-70-80, 20);
            //forget_tip = gtk_label_new("");
            //gtk_widget_set_size_request(forget_tip, 70, 20);
            //gtk_widget_show(forget_tip);
            gtk_widget_set_size_request(label, WIDTH-80, 20);

            forget_btn = create_button("./icon/wireless/btn_green_80x26.png", "./icon/wireless/btn_green_80x26_h.png", NULL, "忘记",
                    -1, -1, GTK_SIGNAL_FUNC(ui_forget_net_enter_handle), NULL);
            set_widget_font_size(forget_btn->label, -1, "Snow");
            //gtk_box_pack_start(GTK_BOX(hbox), forget_tip, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), forget_btn->container, FALSE, FALSE, 0);
        }
    } else {
        //gtk_widget_set_size_request(label, WIDTH, 26);
        gtk_widget_set_size_request(label, WIDTH-80, 20);
        
        eth_config = create_button("./icon/wireless/btn_green_80x26.png", "./icon/wireless/btn_green_80x26_h.png", NULL, "网络配置",
                -1, -1, GTK_SIGNAL_FUNC(ui_ethernet_net_config_handle), NULL);
        set_widget_font_size(eth_config->label, -1, "Snow");    
        gtk_box_pack_start(GTK_BOX(hbox), eth_config->container, FALSE, FALSE, 0);
    }

    gtk_widget_show(hbox);

    align = gtk_alignment_new(0.5, 1, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

    return align;
}

static GtkWidget *create_prop_item(int is_cred, char *prop, char *text, int length)
{
    GtkWidget *hbox;
    GtkWidget *prop_label;
    GtkWidget *text_label;

    hbox = gtk_hbox_new(FALSE, 0);

    prop_label = gtk_label_new(prop);
    set_widget_font_size(prop_label, FONT_SIZE, "#666666");
    gtk_misc_set_alignment(GTK_MISC(prop_label), 0, 0.5);
    gtk_widget_set_size_request(prop_label, length, 20);
    gtk_box_pack_start(GTK_BOX(hbox), prop_label, FALSE, FALSE, 0);

    if(is_cred) {
        credential = gtk_label_new(text);
        set_widget_font_size(credential, FONT_SIZE, "#666666");
        gtk_misc_set_alignment(GTK_MISC(credential), 0, 0.5);
        gtk_widget_set_size_request(credential, WIDTH-length, 20);
        gtk_box_pack_start(GTK_BOX(hbox), credential, FALSE, FALSE, 0);
        gtk_widget_show(credential);
    } else {
        text_label = gtk_label_new(text);
        set_widget_font_size(text_label, FONT_SIZE, "#666666");
        gtk_misc_set_alignment(GTK_MISC(text_label), 0, 0.5);
        gtk_widget_set_size_request(text_label, WIDTH-length, 20);
        gtk_box_pack_start(GTK_BOX(hbox), text_label, FALSE, FALSE, 0);
        gtk_widget_show(text_label);
    }

    gtk_widget_show(prop_label);
    gtk_widget_show(hbox);

    return hbox;
}

static void parsing_security_type(unsigned int key_mgmt, char *buf)
{
    switch(key_mgmt) {
    case 0:
        strcpy(buf, "OPEN");
        break;
    case 1:
        strcpy(buf, "WEP");
        break;
    case 2:
        strcpy(buf, "WPA/WPA2 PSK");
        break;
    case 3:
        strcpy(buf, "WPA/WPA2 EAP");
        break;
    default:
        strcpy(buf, "OTHER AUTHENTICATION TYPE");
        break;
    }
}

static GtkWidget *mk_scan_info_table(ui_scanresult *scan_info)
{
    GtkWidget *vbox;
    GtkWidget *align;
    char buf[32];

    if(scan_info == NULL) {
        logi("mk_scan_info_table faild: scan_info is null\n");
        return NULL;
    }

    vbox = gtk_vbox_new(FALSE, 7);

    gtk_box_pack_start(GTK_BOX(vbox), create_prop_item(FALSE, "SSID :", scan_info->ssid, 55), FALSE, FALSE, 0);
    sprintf(buf, "%.3fGhz", (double)scan_info->freq/1000);
    gtk_box_pack_start(GTK_BOX(vbox), create_prop_item(FALSE, "频段 :",   buf, 50), FALSE, FALSE, 0);
    sprintf(buf, "%ddB", scan_info->idensity);
    gtk_box_pack_start(GTK_BOX(vbox), create_prop_item(FALSE, "信号强度 :", buf, 80), FALSE, FALSE, 0);
    parsing_security_type(scan_info->key_mgmt, buf);
    gtk_box_pack_start(GTK_BOX(vbox), create_prop_item(FALSE, "安全类型 :", buf, 80), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_prop_item(FALSE, "SSID MAC :", scan_info->ssid_mac, 90), FALSE, FALSE, 0);

    if(net_id >= 0) {
        strcpy(buf, "已保存");
    } else {
        strcpy(buf, "未保存");
    }

    gtk_box_pack_start(GTK_BOX(vbox), create_prop_item(TRUE, "认证信息 :", buf, 80), FALSE, FALSE, 0);

    gtk_widget_show(vbox);

    align = gtk_alignment_new(0.5, 1, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), vbox);
    gtk_widget_show(align);

    return align;
}

static GtkWidget *mk_wifi_info_table(ui_wifiinfo *wifi_info)
{
    GtkWidget *vbox;
    GtkWidget *align;
    char buf[32];

    if(wifi_info == NULL) {
        logi("mk_wifi_info_table faild: wifi_info is null\n");
        return NULL;
    }

    vbox = gtk_vbox_new(FALSE, 5);

    gtk_box_pack_start(GTK_BOX(vbox), create_prop_item(FALSE, "SSID :", wifi_info->ssid, 55), FALSE, FALSE, 0);
    sprintf(buf, "%.3fGhz", (double)wifi_info->freq/1000);
    gtk_box_pack_start(GTK_BOX(vbox), create_prop_item(FALSE, "频段 :", buf, 50), FALSE, FALSE, 0);
    sprintf(buf, "%ddB", wifi_info->idensity);
    gtk_box_pack_start(GTK_BOX(vbox), create_prop_item(FALSE, "信号强度 :", buf, 80), FALSE, FALSE, 0);
    parsing_security_type(wifi_info->key_mgmt, buf);
    gtk_box_pack_start(GTK_BOX(vbox), create_prop_item(FALSE, "安全类型 :", buf, 80), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_prop_item(FALSE, "IPv4地址 :", wifi_info->ip_info.ip_addr, 80), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_prop_item(FALSE, "子网掩码 :", wifi_info->ip_info.mask, 80), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_prop_item(FALSE, "网关 :", wifi_info->ip_info.gate, 50), FALSE, FALSE, 0);

    if(net_id >= 0) {
        strcpy(buf, "已保存");
    } else {
        strcpy(buf, "未保存");
    }

    gtk_box_pack_start(GTK_BOX(vbox), create_prop_item(TRUE, "认证信息 :", buf, 80), FALSE, FALSE, 0);

    gtk_widget_show(vbox);

    align = gtk_alignment_new(0.5, 1, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), vbox);
    gtk_widget_show(align);

    return align;
}

static GtkWidget *mk_eth_info_table(void)
{
    logi("mk_eth_info_table faild: wifi_info is null\n");
    GtkWidget *vbox;
    GtkWidget *align;
    char buf[32];

    struct rc_netifdev_t* info_netifdev;
    info_netifdev = malloc(sizeof(struct rc_netifdev_t));
    ui_get_net_info(info_netifdev);

    vbox = gtk_vbox_new(FALSE, 7);

    gtk_box_pack_start(GTK_BOX(vbox), create_prop_item(FALSE, "IPv4地址 :", inet_ntoa(info_netifdev->ip), 80), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_prop_item(FALSE, "子网掩码 :",      inet_ntoa(info_netifdev->netmask), 80), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_prop_item(FALSE, "网关 :", inet_ntoa(info_netifdev->gateway), 50), FALSE, FALSE, 0);

    memset(buf, 0, sizeof(buf));
    strcpy(buf, inet_ntoa(info_netifdev->dns1));
    if ((strcmp(buf, "0.0.0.0") == 0) || (strcmp(buf, "255.255.255.255") == 0)) {
        strcpy(buf, "");
    }
    gtk_box_pack_start(GTK_BOX(vbox), create_prop_item(FALSE, "DNS地址 :", buf, 85), FALSE, FALSE, 0);

    memset(buf, 0, sizeof(buf));
    strcpy(buf, inet_ntoa(info_netifdev->dns2));
    if ((strcmp(buf, "0.0.0.0") == 0) || (strcmp(buf, "255.255.255.255") == 0)) {
        strcpy(buf, "");
    }
    gtk_box_pack_start(GTK_BOX(vbox), create_prop_item(FALSE, "备用DNS地址 :", buf, 115), FALSE, FALSE, 0);

    memset(buf, 0, sizeof(buf));
    if(info_netifdev->iptype == RC_IPTYPE_STATIC) {
        strcpy(buf, "关闭");
    } else {
        strcpy(buf, "开启");
    }
    gtk_box_pack_start(GTK_BOX(vbox), create_prop_item(FALSE, "DHCP模式 :", buf, 90), FALSE, FALSE, 0);

    gtk_widget_show(vbox);

    align = gtk_alignment_new(0.5, 1, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), vbox);
    gtk_widget_show(align);

    free(info_netifdev);

    return align;
}

static int ui_dialog_prop_init(void)
{
    GtkWidget *title;
    GtkWidget *prop_table;
    GtkWidget *func_bar;
    GtkWidget *label;
    ui_scanresult *s_info;
    ui_wifiinfo *w_info;
    int spacing;

    prop = gtk_dialog_new();
    gtk_widget_set_size_request(prop, 460, 310);
    ui_dialog_white_background(prop);
    update_widget_bg(prop, "./icon/wireless/frame_wifi_verify.png", -1, -1);
    gtk_window_set_decorated(GTK_WINDOW(prop), FALSE);
    gtk_window_set_modal(GTK_WINDOW(prop), TRUE);
    g_signal_connect (G_OBJECT(prop), "realize",
                        G_CALLBACK(ui_compt_realize), &ui_dialog_prop);

    switch(mode) {
    case UI_DIALOG_PROP_USABLE:
        if(p_info == NULL) {
            logi("UI_DIALOG_PROP_USABLE: p_info == NULL\n");
            return 0;
        }

        s_info = p_info;
        title = mk_title_bar(s_info->ssid, prop);
        /*查询是否是已保存WiFi*/
        net_id = ui_extern_wifi_saved_net_query_result(s_info->ssid,
                s_info->ssid_mac, s_info->key_mgmt);
        func_bar = mk_function_bar(TRUE);
        prop_table = mk_scan_info_table(s_info);
        spacing = 15;

        break;
    case UI_DIALOG_PROP_CONNECT:
        if(p_info == NULL) {
            logi("UI_DIALOG_PROP_CONNECT: p_info == NULL\n");
            return 0;
        }

        w_info = p_info;
        title = mk_title_bar(w_info->ssid, prop);
        /*查询是否是已保存WiFi*/
        net_id = ui_extern_wifi_saved_net_query_result(w_info->ssid,
                w_info->ssid_mac, w_info->key_mgmt);
        func_bar = mk_function_bar(TRUE);
        prop_table = mk_wifi_info_table(w_info);
        spacing = 10;

        break;
    case UI_DIALOG_PROP_ETH:
        title = mk_title_bar("有线网络", prop);
        func_bar = mk_function_bar(FALSE);
        prop_table = mk_eth_info_table();
        spacing = 15;

        break;
    default:
        return 0;
    }

    label = gtk_label_new("");
    gtk_widget_set_size_request(label, 10, 10);
    gtk_widget_show(label);

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(prop)->vbox), title, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(prop)->vbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(prop)->vbox), func_bar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(prop)->vbox), prop_table, FALSE, FALSE, spacing);
    gtk_window_set_position(GTK_WINDOW(prop), GTK_WIN_POS_CENTER_ALWAYS);
    return 0;
}

static int ui_dialog_prop_ctrl(void *p)
{
    ui_msg_t *msg = p;

    mode = msg->sub_obj;
    p_info = msg->args;

    return 0;
}

static void ui_dialog_prop_show(void)
{
    if (prop != NULL) {
        gtk_widget_destroy(prop);
        prop = NULL;
    }
	ui_dialog_prop_init();
	gtk_widget_show(prop);
}

static void ui_dialog_prop_hide(void)
{
    if(prop != NULL) {
        gtk_widget_destroy(prop);
        prop = NULL;
    }
    mode = UI_DIALOG_PROP_NONE;
    p_info = NULL;
}

static void ui_dialog_prop_adapt(void)
{
    logi("ui_dialog_prop_adapt\n");
}

static void ui_dialog_prop_destroy(void)
{
    if(prop != NULL) {
        gtk_widget_destroy(prop);
        prop = NULL;
    }

    mode = UI_DIALOG_PROP_NONE;
    p_info = NULL;
}

struct ui_comp_s ui_dialog_prop = {
    .type = UI_TYPE_DIALOG_PROP,
    .subtype = UI_TOPLEVEL_TYPE_DIALOG,
    .is_init = UI_NEED_INIT,
    .widget = &prop,
    .init = ui_dialog_prop_init,
    .show = ui_dialog_prop_show,
    .hide = ui_dialog_prop_hide,
    .ctrl = ui_dialog_prop_ctrl,
    .destroy = ui_dialog_prop_destroy,
    .adapt = ui_dialog_prop_adapt,
};
