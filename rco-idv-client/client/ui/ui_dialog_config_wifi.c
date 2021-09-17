#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ui_main.h"
#include "ui_dialog_config.h"
#include "ui_test.h"
#ifdef IDV_CLIENT
#include "../../include/application_c_interfaces.h"
#include "rc/rc_netif.h"
#endif

#define SWITCH_ON_PATH          "./icon/wireless/switch_open.png"
#define SWITCH_OFF_PATH         "./icon/wireless/switch_close.png"
#define SSID_OFF_EN_PATH        "./icon/wireless/ssid_off_en.png" 
#define SSID_OFF_DIS_PATH       "./icon/wireless/ssid_off_dis.png"
#define SSID_OPEN_TIP_LABEL     "已启用，该终端仅允许连接有线网络或以下无线网络"
#define SSID_MAX_NUM            3

enum E_NetStatus{
    E_NETSTATUS_UNKNOWN = 0,
    E_NET_STATUS_ETH_UP = 1,
    E_NET_STATUS_WLAN_UP = 2,
    E_NET_STATUS_LINK_DOWN = 3,
};

static struct wifi_switch_t switch_status = {TRUE, TRUE, TRUE, TRUE};
static btn_apend_t *wifi_switch_btn;
static btn_apend_t *ssid_switch_btn;
static char ssid_list[SSID_MAX_NUM][128];
static int ssid_white_size = 0;
static GtkWidget *labelssid[SSID_MAX_NUM];
static GtkWidget *open_tip = NULL;
static GtkWidget *ssidhbox = NULL;
static GtkWidget * net_wifi_vbox = NULL;

static GtkWidget *mk_show_whitessid(int ssid_size, gboolean wifi_sw);
static btn_apend_t *create_switch_button(gboolean sw_mode, char *path_on, char *path_off, GCallback callback);
static void set_button_icon(btn_apend_t *object, char *icon, char *icon_mouse_on);
static GtkWidget *mk_show_unopened_whitessid(void);

void get_sw_status(struct wifi_switch_t *status)
{   
    if (status != NULL) {
         memcpy(status, &switch_status, sizeof(struct wifi_switch_t));
    } 
}

static void clear_ssid_hbox()
{
    int i = 0;
    gtk_widget_destroy(ssidhbox);

    if (ssidhbox) {
        ssidhbox = NULL;
    }

    if (open_tip) {
        open_tip = NULL;
    }

    if (ssid_switch_btn) {
        ssid_switch_btn = NULL;
    }

    for (i = 0; i < SSID_MAX_NUM; i++) {
        if (labelssid[i]) {
            labelssid[i] = NULL;
        }
    }
}


static void set_ssid_label_color(gboolean sw_mode)
{
    int i = 0;
    GdkColor color;

    if (open_tip == NULL) {
        return;
    }

    if (sw_mode == FALSE) {
        gdk_color_parse("gray",&color);
    } else {
        gdk_color_parse("black",&color);
    }

    gtk_widget_modify_fg(open_tip, GTK_STATE_NORMAL, &color);
    gtk_widget_show(open_tip);
    
    for (i = 0; i < ssid_white_size; i++) {
        gtk_widget_modify_fg(labelssid[i], GTK_STATE_NORMAL, &color);
        gtk_widget_show(labelssid[i]);
    }
}

static void set_button_icon(btn_apend_t *object, char *icon, char *icon_mouse_on)
{
    GdkPixbuf *src_pixbuf = NULL;

    if (object == NULL) {
        return;
    }
    
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

static gint wifi_button_click_switch(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    logi("wifi button click switch\n");
    if (switch_status.new_wifi_status == FALSE) {
        switch_status.new_wifi_status = TRUE;
        set_button_icon(wifi_switch_btn, SWITCH_ON_PATH, SWITCH_ON_PATH);
    } else {
        switch_status.new_wifi_status = FALSE;
        set_button_icon(wifi_switch_btn, SWITCH_OFF_PATH, SWITCH_OFF_PATH);
    }

    if (ssid_white_size == 0 || !switch_status.new_wssid_status) {
        return 0;
    }

    if (switch_status.new_wifi_status) {
        set_button_icon(ssid_switch_btn, SSID_OFF_EN_PATH, SSID_OFF_EN_PATH);
    } else {
        set_button_icon(ssid_switch_btn, SSID_OFF_DIS_PATH, SSID_OFF_DIS_PATH);
    }
  
    set_ssid_label_color(switch_status.new_wifi_status);


    return 0;
}

static gint ssid_button_click_switch(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    logi("ssid button click switch\n");

    /* if wifi sw is close we shold not click button */
    if(switch_status.new_wifi_status == FALSE) {
        return 0;
    }

    switch_status.new_wssid_status = FALSE;
    if (ssidhbox) {
        clear_ssid_hbox();
    }

    /* close ssid function then we should show unopen tip ui */
    if (net_wifi_vbox) {
        ssidhbox = mk_show_whitessid(0, switch_status.new_wssid_status);
        gtk_box_pack_start(GTK_BOX(net_wifi_vbox), ssidhbox, FALSE, FALSE, 0);
        gtk_widget_show(net_wifi_vbox);
    }
    return 0;
}

static btn_apend_t *create_switch_button(gboolean sw_mode, char *path_on, char *path_off, GCallback callback)
{
    btn_apend_t *switch_btn;

    if(sw_mode) {
        switch_btn = create_button(path_on, path_on, NULL, NULL, -1, -1, callback, NULL);
       
    } else {
        switch_btn = create_button(path_off, path_off, NULL, NULL,-1, -1, callback, NULL);
    }

    return switch_btn;
}


static  GtkWidget *mk_switch_box(gboolean sw_mode, char *text, char *path_on, char *path_off, btn_apend_t **switch_btn, GCallback callback)
{
    GtkWidget *label = NULL;
    GtkWidget *h_box = NULL;
    
    if (text != NULL) {
        label = gtk_label_new(text);
    } else {
        label = gtk_label_new(NULL);
    }
    gtk_widget_set_size_request(label, 100, 20);
    gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
    gtk_widget_show(label);

    *switch_btn = create_switch_button(sw_mode, path_on, path_off, callback);
    h_box = gtk_hbox_new(FALSE, 8);
    gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(h_box), (*switch_btn)->container, FALSE, FALSE, 0);
    gtk_widget_show(h_box);

    return h_box;
}

static void wifi_switch_init()
{
    int sw_mode;

    /* get wifi enable switch status */
    sw_mode = ui_extern_wireless_netcard_query_result();
    if (sw_mode) {
        switch_status.new_wifi_status = TRUE;
        switch_status.old_wifi_status = TRUE;
    } else {
        switch_status.new_wifi_status = FALSE;
        switch_status.old_wifi_status = FALSE;
    }
}

static int ssid_switch_init()
{
    int size = 0;  
    memset(ssid_list, 0, sizeof(ssid_list));

    size = ui_extern_getssid_whitelist(&ssid_list[0], SSID_MAX_NUM);
    if (size <= 0){
        switch_status.new_wssid_status = FALSE;
        switch_status.old_wssid_status = FALSE;
    } else {
        switch_status.new_wssid_status = TRUE;
        switch_status.old_wssid_status = TRUE;
    }
    
    return size;
}

static GtkWidget *mk_tip_label(char *text)
{
    GtkWidget *label_tip = NULL;
    GdkColor color;

    label_tip = gtk_label_new(text);
    gdk_color_parse("gray",&color);
    gtk_widget_modify_fg(label_tip, GTK_STATE_NORMAL, &color);
    gtk_widget_set_size_request(label_tip, 380, 20);
    gtk_misc_set_alignment(GTK_MISC(label_tip), 0, 0);
    gtk_label_set_line_wrap(GTK_LABEL(label_tip), TRUE);
    gtk_widget_show(label_tip);
    return label_tip;
}

static GtkWidget *mk_show_unopened_whitessid(void)
{
    GtkWidget * label_vbox = NULL;
    GtkWidget *label_tip = NULL;
    
    label_vbox = gtk_vbox_new(FALSE, 24);

    label_tip = mk_tip_label("未启用");
    gtk_box_pack_start(GTK_BOX(label_vbox), label_tip, FALSE, FALSE, 0);

    label_tip = mk_tip_label("启用无线白名单请按以下步骤操作：");
    gtk_box_pack_start(GTK_BOX(label_vbox), label_tip, FALSE, FALSE, 0);
    
    label_tip = mk_tip_label("1、请至WEB管理端配置无线白名单");
    gtk_box_pack_start(GTK_BOX(label_vbox), label_tip, FALSE, FALSE, 0);

    label_tip = mk_tip_label("2、请连接网络来同步白名单");
    gtk_label_set_line_wrap(GTK_LABEL(label_tip), TRUE);
    gtk_box_pack_start(GTK_BOX(label_vbox), label_tip, FALSE, FALSE, 0);

    gtk_widget_show(label_vbox);
    return label_vbox;
}

static GtkWidget *mk_show_white_ssid(char (*ssid)[128], int list_num, gboolean sw_mode)
{
    GtkWidget *v_box = NULL;
    GtkWidget *align = NULL;
    int i = 0;

    if (list_num == 0) {
        return NULL;
    }

    v_box = gtk_vbox_new(FALSE, 10);
    open_tip = gtk_label_new(SSID_OPEN_TIP_LABEL);
    gtk_widget_set_size_request(open_tip, 380, 35);
    gtk_misc_set_alignment(GTK_MISC(open_tip), 0, 0);
    gtk_widget_show(open_tip);
    gtk_box_pack_start(GTK_BOX(v_box), open_tip, FALSE, FALSE, 0);
    
    for (i = 0; i < list_num; i++) {
        labelssid[i] = gtk_label_new(ssid[i]);
        gtk_widget_set_size_request(labelssid[i], 300, 35);
        gtk_label_set_line_wrap(GTK_LABEL(labelssid[i]), TRUE);
        gtk_misc_set_alignment(GTK_MISC(labelssid[i]), 0, 0);
        gtk_widget_show(labelssid[i]);
        gtk_box_pack_start(GTK_BOX(v_box), labelssid[i], FALSE, FALSE, 0);
    }
    set_ssid_label_color(sw_mode);

    if (sw_mode) {
        ssid_switch_btn = create_button(SSID_OFF_EN_PATH, SSID_OFF_EN_PATH, NULL, NULL, -1, -1, (GCallback)(ssid_button_click_switch), NULL);
    } else {
        ssid_switch_btn = create_button(SSID_OFF_DIS_PATH, SSID_OFF_DIS_PATH, NULL, NULL, -1, -1, (GCallback)(ssid_button_click_switch), NULL);
    }
    
    align = gtk_alignment_new(0, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), ssid_switch_btn->container);
    gtk_widget_show(align);
    gtk_box_pack_start(GTK_BOX(v_box), align, FALSE, FALSE, 0);

    gtk_widget_show(v_box);
    return v_box;
}

static GtkWidget *mk_show_white_status(char (*ssid)[128], int list_num, gboolean sw_status)
{
    if(list_num == 0) {
        return mk_show_unopened_whitessid();
    } else {
        if (list_num > SSID_MAX_NUM) {
            list_num = SSID_MAX_NUM;
        }
        return mk_show_white_ssid(&ssid_list[0], list_num, sw_status);
    }
}

static GtkWidget *mk_show_whitessid(int ssid_size, gboolean sw_status)
{
    GtkWidget * show_hbox = NULL; 
    GtkWidget * label_vbox = NULL;
    GtkWidget * label_ssid = NULL;
    GtkWidget * ssid_vbox = NULL;

    label_ssid = gtk_label_new("无线白名单:");
    gtk_widget_set_size_request(label_ssid, 100, 20);
    gtk_misc_set_alignment(GTK_MISC(label_ssid), 1, 0);
    gtk_widget_show(label_ssid); 

    label_vbox = gtk_vbox_new(FALSE, 3);
    gtk_box_pack_start(GTK_BOX(label_vbox), label_ssid, FALSE, FALSE, 0);
    gtk_widget_show(label_vbox);
    
    ssid_vbox = mk_show_white_status(&ssid_list[0], ssid_size, sw_status);
    show_hbox = gtk_hbox_new (FALSE, 8);
    gtk_box_pack_start(GTK_BOX(show_hbox), label_vbox, FALSE, FALSE, 0);

    if (ssid_vbox != NULL) {
        gtk_box_pack_start(GTK_BOX(show_hbox), ssid_vbox, FALSE, FALSE, 0);
        gtk_widget_show(ssid_vbox);
    }
    gtk_widget_show(show_hbox);
    return show_hbox;
}

GtkWidget *ui_config_wifimanage_init()
{
    GtkWidget * wifi_hbox = NULL;
    GtkWidget *align_wifi = NULL;
    
    wifi_switch_init();
    ssid_white_size = ssid_switch_init();
    
    wifi_hbox = mk_switch_box(switch_status.new_wifi_status, "无线开关:", SWITCH_ON_PATH, SWITCH_OFF_PATH, &wifi_switch_btn, (GCallback)(wifi_button_click_switch));
    ssidhbox = mk_show_whitessid(ssid_white_size, switch_status.new_wifi_status);

    net_wifi_vbox = gtk_vbox_new(FALSE, 24);
    gtk_box_pack_start(GTK_BOX(net_wifi_vbox), wifi_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(net_wifi_vbox), ssidhbox, FALSE, FALSE, 0);
    
    gtk_widget_show(net_wifi_vbox);

    align_wifi= gtk_alignment_new(0.5, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(align_wifi), net_wifi_vbox);
    gtk_widget_show(align_wifi);
    
    return align_wifi;
}


