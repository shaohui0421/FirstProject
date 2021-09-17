#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ui_main.h"
#include "ui_dialog_config.h"
#include "ui_test.h"
#include "ui_dialog_config_scrnmanage.h"
#ifdef IDV_CLIENT
#include "../../include/application_c_interfaces.h"
#include "rc/rc_netif.h"
#include "rc/rc_checknetifval.h"
#endif

#define INVALID_IP_ADDR_MIN 0
#define INVALID_IP_ADDR_MAX 0xffffffff
#define LABEL_WIDTH  (132)
#define LABEL_HEIGHT (20)

#define SWITCH_ON_PATH          "./icon/switch_on.png"
#define SWITCH_OFF_PATH         "./icon/switch_off.png"


static GtkWidget *dialog, *label;
static btn_apend_t  * okay_button;
static btn_apend_t  * cancal_button;

static GtkAdjustment * dev_page_ajust;

GtkWidget * dev_page;
GtkWidget * vm_page;
GtkWidget * ping_page;
GtkWidget * netwifi_page = NULL;
GtkWidget * ip_err_page;
GtkWidget * display_page = NULL;
GtkWidget * other_page;
GtkWidget * action_area;

btn_apend_t  * dev_set;
btn_apend_t  * vm_set;

btn_apend_t  * ping;
btn_apend_t  * ip_err;
btn_apend_t  * display_set = NULL;
btn_apend_t  * other_set;
btn_apend_t  * netwifi_set = NULL;

static GtkWidget *tab_vbox[3];
//static GtkWidget *table[2];
//static GtkWidget *terminal_name;
//static GtkWidget *server_address;
static GtkWidget *combo;
static GtkWidget *terminal_name;
static GtkWidget *name_err;
static GtkWidget *name_err_hide;

static GtkWidget *server_address;
static GtkWidget *address_err;
static GtkWidget *address_err_hide;

static GtkWidget *ip_button[2];
static GtkWidget *ip_config[3]; // [0] ip [1] netmask [2]gateway
static GtkWidget *ip_err_show[3]; 
static GtkWidget *ip_err_hide[3]; 
static GtkWidget *ip_err_label[3];

static GtkWidget *dns_button[2];
static GtkWidget *dns_config[2]; // [0] primary dns [1] secondary dns
static GtkWidget *dns_err_show[2]; // [0] primary dns [1] secondary dns
static GtkWidget *dns_err_hide[2]; // [0] primary dns [1] secondary dns
static GtkWidget *dns_err_label[3];

static GtkWidget *vm_ip_button[2];
static GtkWidget *vm_ip_config[3]; // [0] ip [1] netmask [2]gateway
static GtkWidget *vm_ip_err_show[3]; 
static GtkWidget *vm_ip_err_hide[3]; 
static GtkWidget *vm_ip_err_label[3];

static GtkWidget *vm_dns_button[2];
static GtkWidget *vm_dns_config[2]; // [0] primary dns [1] secondary dns
static GtkWidget *vm_dns_err_show[3]; 
static GtkWidget *vm_dns_err_hide[3]; 
static GtkWidget *vm_dns_err_label[3];

static GtkWidget *bt_service_checkbox = NULL;
static GtkWidget *power_boot_checkbox = NULL;
static GtkWidget *boot_speedup_checkbox = NULL;
static GtkWidget *hdmi_audio_checkbox = NULL;
static GtkWidget *e1000_netcard_checkbox = NULL;
static GtkWidget *usb_emulation_checkbox = NULL;
static gboolean layer_manage_switch_status;
static btn_apend_t *layer_manage_switch_btn = NULL;

static input_box_t input_box[20];
static int index_box;
extern gint ui_win_button_click_config(GtkWidget *widget,GdkEventButton *event, gpointer data);
static void set_switch_button_icon(btn_apend_t *object, char *icon, char *icon_mouse_on);


void ui_dialog_config_destroy(void);

void set_ajust_min(GtkAdjustment * ajust)
{
	gtk_adjustment_set_value(GTK_ADJUSTMENT(ajust), gtk_adjustment_get_lower(GTK_ADJUSTMENT(ajust)));
}

void set_ajust_max(GtkAdjustment * ajust)
{
	gdouble value = gtk_adjustment_get_upper(GTK_ADJUSTMENT(ajust)) - gtk_adjustment_get_page_size(GTK_ADJUSTMENT(ajust));

	gtk_adjustment_set_value(GTK_ADJUSTMENT(ajust), value);
}

static gint btn_disable_all(void)
{
	set_widget_font_size(dev_set->label, -1, "black");
	gtk_widget_hide(dev_page);
	gtk_widget_hide(dev_set->under_img_h);
	
	set_widget_font_size(vm_set->label, -1, "black");
	gtk_widget_hide(vm_page);
	gtk_widget_hide(vm_set->under_img_h);

	set_widget_font_size(ping->label, -1, "black");
	gtk_widget_hide(ping_page);
	gtk_widget_hide(ping->under_img_h);

    if (netwifi_page != NULL && netwifi_set != NULL) {
        set_widget_font_size(netwifi_set->label, -1, "black");
    	gtk_widget_hide(netwifi_page);
    	gtk_widget_hide(netwifi_set->under_img_h);
    }
    
	set_widget_font_size(ip_err->label, -1, "black");
	gtk_widget_hide(ip_err_page);
	gtk_widget_hide(ip_err->under_img_h);

    if (display_page != NULL && display_set != NULL) {
        set_widget_font_size(display_set->label, -1, "black");
        gtk_widget_hide(display_page);
        gtk_widget_hide(display_set->under_img_h);
    }

    set_widget_font_size(other_set->label, -1, "black");
    gtk_widget_hide(other_page);
    gtk_widget_hide(other_set->under_img_h);

    gtk_widget_hide(action_area);
    return FALSE;
}

static void show_devpage(void)
{
	btn_disable_all();
	
	set_widget_font_size(dev_set->label, -1, "LightSeaGreen");
	gtk_widget_show(dev_page);
	gtk_widget_show(dev_set->under_img_h);
	
	gtk_widget_show(action_area);	
	return;
}

static void show_vmpage(void)
{
	btn_disable_all();
	
	set_widget_font_size(vm_set->label, -1, "LightSeaGreen");
	gtk_widget_show(vm_page);
	gtk_widget_show(vm_set->under_img_h);
	gtk_widget_show(action_area);	
	return;
}

static void show_netwifipage(void)
{
    btn_disable_all();

    if (netwifi_page != NULL && netwifi_set != NULL) {
        set_widget_font_size(netwifi_set->label, -1, "LightSeaGreen");
        gtk_widget_show(netwifi_page);
        gtk_widget_show(netwifi_set->under_img_h);
    }
	gtk_widget_show(action_area);
    return;
}

static int _check_ip(const char *ip_str, struct sockaddr_in *sa_ip)
{
    int result;

    result = inet_pton(AF_INET, ip_str, &(sa_ip->sin_addr));
    if (result <= 0 || sa_ip->sin_addr.s_addr == INVALID_IP_ADDR_MIN
        || sa_ip->sin_addr.s_addr == INVALID_IP_ADDR_MAX) {
        return 1;
    }

    return 0;
}

static int _check_mask(const char *netmask, struct sockaddr_in *sa_netmask)
{
    int result;
    unsigned int n[4];
    unsigned i;
    unsigned int x;
    unsigned int y;
    unsigned int z;
    
    /* check netmask */
    result = inet_pton(AF_INET, netmask, &(sa_netmask->sin_addr));
    if (result <= 0 || sa_netmask->sin_addr.s_addr == 0) {
        //fprintf(stderr, "invalid netmask 1\n");
        return 1;
    }
    sscanf(netmask, "%u.%u.%u.%u", &n[3], &n[2], &n[1], &n[0]);
    x = 0;
    for (i = 0; i < 4; i++) {
        x += n[i] << (i * 8);
    }
    y = ~x;
    z = y + 1;
    //fprintf(stdout, "x 0x%08x, y 0x%08x, z 0x%08x\n", x, y, z);
    if ((y & z) != 0) {
        //fprintf(stderr, "invalid netmask 2\n");
        return 1;
    }

    return 0;
}

static void input_box_err_show(input_box_t *box)
{
    gtk_widget_show(box->show);
    gtk_widget_hide(box->hide);
}


static void input_box_err_hide(input_box_t *box)
{
    gtk_widget_show(box->hide);
    gtk_widget_hide(box->show);
}

static gboolean check_hostname(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    input_box_t *box = (input_box_t *)user_data;
    int ret = 0;

    if (!gtk_entry_get_text_length(GTK_ENTRY(box->input))) {
        return FALSE;
    }

#ifdef IDV_CLIENT
    ret = rc_check_hostname(gtk_entry_get_text(GTK_ENTRY(box->input)), 20);
#endif
    if (ret != 0) {
        input_box_err_show(box);
    } else {
        input_box_err_hide(box);
    }

    return FALSE;
}


static gboolean check_ip(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    input_box_t *box = (input_box_t *)user_data;
    struct sockaddr_in sa_ip;
    int ret;

    if (!gtk_entry_get_text_length(GTK_ENTRY(box->input))) {
        return FALSE;
    }

    ret = _check_ip(gtk_entry_get_text(GTK_ENTRY(box->input)), &sa_ip);

    if (ret != 0) {
        input_box_err_show(box);
    } else {
        input_box_err_hide(box);
    }

    return FALSE;
}

static gboolean check_netmask(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    input_box_t *box = (input_box_t *)user_data;
    struct sockaddr_in sa_netmask;
    int ret;

    if (!gtk_entry_get_text_length(GTK_ENTRY(box->input))) {
        return FALSE;
    }

    ret = _check_mask(gtk_entry_get_text(GTK_ENTRY(box->input)), &sa_netmask);

    if (ret != 0) {
        input_box_err_show(box);
    } else {
        input_box_err_hide(box);
    }
    
    return FALSE;
}

static gboolean check_gateway(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    input_box_t *box = (input_box_t *)user_data;
    struct sockaddr_in sa_ip;
    struct sockaddr_in sa_netmask;
    struct sockaddr_in sa_gateway;
    int ret3, ret2, ret1;

    if (!gtk_entry_get_text_length(GTK_ENTRY(box->input))) {
        return FALSE;
    }

    ret3 = _check_ip(gtk_entry_get_text(GTK_ENTRY(box->input)), &sa_gateway);

    if (ret3 != 0) {
        input_box_err_show(box);
        return FALSE;
    }

    ret2 = _check_mask(gtk_entry_get_text(GTK_ENTRY(box[-1].input)), &sa_netmask);
    ret1 = _check_ip(gtk_entry_get_text(GTK_ENTRY(box[-2].input)), &sa_ip);

    if ( ret1 == 0 && ret2 == 0) {
        if ((sa_ip.sin_addr.s_addr & sa_netmask.sin_addr.s_addr) !=
            (sa_gateway.sin_addr.s_addr & sa_netmask.sin_addr.s_addr)) {
            input_box_err_show(box);
            return FALSE;
        } else {
            input_box_err_hide(box);
        }
    }

    return FALSE;
}

static void input_box_init(GtkWidget *input, GtkWidget *show, GtkWidget *hide, input_box_cb cb)
{
    input_box[index_box].input = input;
    input_box[index_box].show = show;
    input_box[index_box].hide = hide;

    if (cb) {
        gtk_signal_connect(GTK_OBJECT(input), "focus-out-event", G_CALLBACK(cb), &input_box[index_box]);
    }
    index_box++;
}

//static GtkWidget *label_status;

static void disable_btn_widget()
{
	set_btn_effect(okay_button, FALSE);
	set_btn_effect(cancal_button, FALSE);
}

static void able_btn_widget()
{
	set_btn_effect(okay_button, TRUE);
	set_btn_effect(cancal_button, TRUE);
}


static void ip_sessentive_change(GtkWidget **ip, int j, int status)
{
    int i;
    for (i = 0; i < j; i++) {
        gtk_widget_set_sensitive(ip[i], status);
    }
}

static gboolean ip_unsessentive_3 (GtkWidget *widget, gpointer *data)
{   
    ip_sessentive_change((GtkWidget **)data, 3, FALSE);
    return FALSE;
}

static gboolean ip_sessentive_3 (GtkWidget *widget, gpointer *data)
{   
    ip_sessentive_change((GtkWidget **)data, 3, TRUE);
    return FALSE;
}

static gboolean ip_unsessentive_2 (GtkWidget *widget, gpointer *data)
{   
    ip_sessentive_change((GtkWidget **)data, 2, FALSE);
    return FALSE;
}

static gboolean ip_sessentive_2 (GtkWidget *widget, gpointer *data)
{   
    ip_sessentive_change((GtkWidget **)data, 2, TRUE);
    return FALSE;
}

static gboolean disable_auto_dns_btn (GtkWidget *widget, gpointer *data)
{
	GtkWidget * dns_auto = (GtkWidget *)data[0];
	GtkWidget * dns_static = (GtkWidget *)data[1];

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dns_static), TRUE);
	gtk_widget_set_sensitive(dns_auto, FALSE);

    return FALSE;
}

static gboolean able_auto_dns_btn (GtkWidget *widget, gpointer *data)
{

	GtkWidget * dns_auto = (GtkWidget *)data[0];
	gtk_widget_set_sensitive(dns_auto, TRUE);

    return FALSE;
}


#ifdef IDV_CLIENT
static void ui_dialog_config_fill_value(struct rc_netifdev_t* info_netifdev, GtkWidget **ip_btn, GtkWidget **dns_btn, GtkWidget **ip_conf, GtkWidget **dns_conf)
{
	char buf[32];
    if (info_netifdev->iptype == RC_IPTYPE_STATIC) {
        gtk_button_clicked(GTK_BUTTON(ip_btn[1]));
    } else {
        gtk_button_clicked(GTK_BUTTON(ip_btn[0]));
    }
    gtk_entry_set_text(GTK_ENTRY(ip_conf[0]) , inet_ntoa(info_netifdev->ip));
    gtk_entry_set_text(GTK_ENTRY(ip_conf[1]) , inet_ntoa(info_netifdev->netmask));
    gtk_entry_set_text(GTK_ENTRY(ip_conf[2]) , inet_ntoa(info_netifdev->gateway));

    if (info_netifdev->dnstype == RC_IPTYPE_STATIC) {
        gtk_button_clicked(GTK_BUTTON(dns_btn[1]));
    } else {
        gtk_button_clicked(GTK_BUTTON(dns_btn[0]));
    }
    
    memset(buf, 0, sizeof(buf));
    strcpy(buf, inet_ntoa(info_netifdev->dns1));
    if ((strcmp(buf, "0.0.0.0") == 0) || (strcmp(buf, "255.255.255.255") == 0)) {
    	gtk_entry_set_text(GTK_ENTRY(dns_conf[0]) , "");
    } else {
    	gtk_entry_set_text(GTK_ENTRY(dns_conf[0]) , buf);
    }

    memset(buf, 0, sizeof(buf));
    strcpy(buf, inet_ntoa(info_netifdev->dns2));
    if ((strcmp(buf, "0.0.0.0") == 0) || (strcmp(buf, "255.255.255.255") == 0)) {
    	gtk_entry_set_text(GTK_ENTRY(dns_conf[1]) , "");
    } else {
    	gtk_entry_set_text(GTK_ENTRY(dns_conf[1]) , buf);
    }
}

static void ui_dialog_config_save_value(struct rc_netifdev_t* info_netifdev, GtkWidget **ip_btn, GtkWidget **dns_btn, GtkWidget **ip_conf, GtkWidget **dns_conf)
{
    char buf[32];
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ip_btn[0]))) {
        info_netifdev->iptype = RC_IPTYPE_DHCP;
    } else {
        info_netifdev->iptype = RC_IPTYPE_STATIC;
        inet_aton(gtk_entry_get_text(GTK_ENTRY(ip_conf[0])) , &info_netifdev->ip);
        inet_aton(gtk_entry_get_text(GTK_ENTRY(ip_conf[1])) , &info_netifdev->netmask);
        inet_aton(gtk_entry_get_text(GTK_ENTRY(ip_conf[2])) , &info_netifdev->gateway);
    }
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dns_btn[0]))) {
        info_netifdev->dnstype = RC_IPTYPE_DHCP;
    } else {
        info_netifdev->dnstype = RC_IPTYPE_STATIC;
        inet_aton(gtk_entry_get_text(GTK_ENTRY(dns_conf[0])) , &info_netifdev->dns1);
        strcpy(buf, gtk_entry_get_text(GTK_ENTRY(dns_conf[1])));
        if(strcmp(buf, "") == 0) {
            info_netifdev->dns2.s_addr = INADDR_NONE;
        } else {
            inet_aton(gtk_entry_get_text(GTK_ENTRY(dns_conf[1])) , &info_netifdev->dns2);
        }
    }
}

#endif


static void ui_dialog_config_reload()
{
    index_box = 0;
    input_box_init(terminal_name, name_err, name_err_hide, check_hostname);
    input_box_init(server_address, address_err, address_err_hide, check_ip);

    input_box_init(ip_config[0], ip_err_show[0], ip_err_hide[0], check_ip);
    input_box_init(ip_config[1], ip_err_show[1], ip_err_hide[1], check_netmask);
    input_box_init(ip_config[2], ip_err_show[2], ip_err_hide[2], check_gateway);
    input_box_init(dns_config[0], dns_err_show[0], dns_err_hide[0], check_ip);
    input_box_init(dns_config[1], dns_err_show[1], dns_err_hide[1], check_ip);

    input_box_init(vm_ip_config[0], vm_ip_err_show[0], vm_ip_err_hide[0], check_ip);
    input_box_init(vm_ip_config[1], vm_ip_err_show[1], vm_ip_err_hide[1], check_netmask);
    input_box_init(vm_ip_config[2], vm_ip_err_show[2], vm_ip_err_hide[2], check_gateway);
    input_box_init(vm_dns_config[0], vm_dns_err_show[0], vm_dns_err_hide[0], check_ip);
    input_box_init(vm_dns_config[1], vm_dns_err_show[1], vm_dns_err_hide[1], check_ip);

    g_signal_connect(G_OBJECT(ip_config[0]),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    g_signal_connect(G_OBJECT(ip_config[1]),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    g_signal_connect(G_OBJECT(ip_config[2]),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    g_signal_connect(G_OBJECT(dns_config[0]),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    g_signal_connect(G_OBJECT(dns_config[1]),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    
    g_signal_connect(G_OBJECT(vm_ip_config[0]),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    g_signal_connect(G_OBJECT(vm_ip_config[1]),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    g_signal_connect(G_OBJECT(vm_ip_config[2]),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    g_signal_connect(G_OBJECT(vm_dns_config[0]),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    g_signal_connect(G_OBJECT(vm_dns_config[1]),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    g_signal_connect (GTK_OBJECT (ip_button[0]),"clicked",G_CALLBACK (ip_unsessentive_3), ip_config);
    g_signal_connect (GTK_OBJECT (ip_button[1]),"clicked",G_CALLBACK (ip_sessentive_3), ip_config);
    g_signal_connect (GTK_OBJECT (dns_button[0]),"clicked",G_CALLBACK (ip_unsessentive_2), dns_config);
    g_signal_connect (GTK_OBJECT (dns_button[1]),"clicked",G_CALLBACK (ip_sessentive_2), dns_config);
    
    g_signal_connect (GTK_OBJECT (vm_ip_button[0]),"clicked",G_CALLBACK (ip_unsessentive_3), vm_ip_config);
    g_signal_connect (GTK_OBJECT (vm_ip_button[1]),"clicked",G_CALLBACK (ip_sessentive_3), vm_ip_config);
    g_signal_connect (GTK_OBJECT (vm_dns_button[0]),"clicked",G_CALLBACK (ip_unsessentive_2), vm_dns_config);
    g_signal_connect (GTK_OBJECT (vm_dns_button[1]),"clicked",G_CALLBACK (ip_sessentive_2), vm_dns_config);

    //g_signal_connect (GTK_OBJECT (ip_button[0]),"clicked", G_CALLBACK (), dns_button);
    g_signal_connect (GTK_OBJECT (ip_button[0]),"clicked", G_CALLBACK (able_auto_dns_btn), dns_button);
    g_signal_connect (GTK_OBJECT (ip_button[1]),"clicked", G_CALLBACK (disable_auto_dns_btn), dns_button);

    g_signal_connect (GTK_OBJECT (vm_ip_button[0]),"clicked", G_CALLBACK (able_auto_dns_btn), vm_dns_button);
    g_signal_connect (GTK_OBJECT (vm_ip_button[1]),"clicked", G_CALLBACK (disable_auto_dns_btn), vm_dns_button);

    //g_signal_connect (GTK_OBJECT(bt_service_checkbox), "clicked", G_CALLBACK(ui_dialog_bt_service_click), NULL);
    
    char ip[20];
    ui_extern_get_server_ip(ip);
    gtk_entry_set_text(GTK_ENTRY(server_address), ip);

    char hostname[100];
    ui_extern_get_hostname(hostname);
    gtk_entry_set_text(GTK_ENTRY(terminal_name), hostname);

    int mode;
    int readonly;
    ui_extern_get_terminal_mode(&mode, &readonly);
    gtk_combo_box_set_active (GTK_COMBO_BOX(combo), mode);
    if (readonly) {
        gtk_widget_set_sensitive(combo, FALSE);
    } else {
    	gtk_widget_set_sensitive(combo, TRUE);
    }

    int bt_service;
    ui_extern_get_bt_service(&bt_service);
    if (bt_service == 1) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bt_service_checkbox), TRUE);
    } else if (bt_service == 0) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bt_service_checkbox), FALSE);
    }

    int power_boot;
    ui_extern_get_power_boot(&power_boot);
    if (power_boot == 1) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(power_boot_checkbox), TRUE);
    } else if (power_boot == 0) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(power_boot_checkbox), FALSE);
    }
	
    int boot_speedup;
    ui_extern_get_boot_speedup(&boot_speedup);
    if (boot_speedup == 1) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(boot_speedup_checkbox), TRUE);
    } else if (boot_speedup == 0) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(boot_speedup_checkbox), FALSE);
    }


    if(ui_extern_is_hdmi_connected() && ui_extern_start_vmmode_is_emulation ()) {
        int hdmi_audio;
        hdmi_audio = ui_extern_get_hdmi_audio();
        if (hdmi_audio == 1) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hdmi_audio_checkbox), TRUE);
        } else if (hdmi_audio == 0) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hdmi_audio_checkbox), FALSE);
        }
    }

    if (e1000_netcard_checkbox) {
        int e1000_netcard = ui_extern_using_e1000_netcard();
        if (e1000_netcard == 1) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(e1000_netcard_checkbox), TRUE);
        } else if (e1000_netcard == 0) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(e1000_netcard_checkbox), FALSE);
        }
    }

    if (usb_emulation_checkbox) {
        int usb_emulation = ui_extern_is_usb_emulation();
        if (usb_emulation == 1) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(usb_emulation_checkbox), TRUE);
        } else if (usb_emulation == 0) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(usb_emulation_checkbox), FALSE);
        }
    }

    if (layer_manage_switch_btn) {
        layer_manage_switch_status = ui_extern_is_using_app_layer();
        if (layer_manage_switch_status == 1) {
            set_switch_button_icon(layer_manage_switch_btn, SWITCH_ON_PATH, SWITCH_ON_PATH);
        } else if (layer_manage_switch_status == 0) {
            set_switch_button_icon(layer_manage_switch_btn, SWITCH_OFF_PATH, SWITCH_OFF_PATH);
        }
    }

    gtk_button_clicked(GTK_BUTTON(ip_button[0]));
    gtk_button_clicked(GTK_BUTTON(dns_button[0]));
    gtk_button_clicked(GTK_BUTTON(vm_ip_button[0]));
    gtk_button_clicked(GTK_BUTTON(vm_dns_button[0]));
#ifdef IDV_CLIENT
    struct rc_netifdev_t* info_netifdev;
    info_netifdev = malloc(sizeof(struct rc_netifdev_t));
    ui_get_net_info(info_netifdev);

    ui_dialog_config_fill_value(info_netifdev, ip_button, dns_button, ip_config, dns_config);

    struct rc_netifdev_t* info_netifdev_vm;
    info_netifdev_vm = malloc(sizeof(struct rc_netifdev_t));
    ui_get_vm_net_info(info_netifdev_vm);
    ui_dialog_config_fill_value(info_netifdev_vm, vm_ip_button, vm_dns_button, vm_ip_config, vm_dns_config);

    free(info_netifdev);
    free(info_netifdev_vm);
#endif
    /*当连接无线网络时，禁用终端网络配置和虚机网络配置*/
    int result = ui_extern_net_status_query_result();
    if(result == 2) {
        gtk_widget_set_sensitive(ip_button[0], FALSE);
        gtk_widget_set_sensitive(ip_button[1], FALSE);
        gtk_widget_set_sensitive(dns_button[0], FALSE);
        gtk_widget_set_sensitive(dns_button[1], FALSE);
        gtk_widget_set_sensitive(vm_ip_button[0], FALSE);
        gtk_widget_set_sensitive(vm_ip_button[1], FALSE);
        gtk_widget_set_sensitive(vm_dns_button[0], FALSE);
        gtk_widget_set_sensitive(vm_dns_button[1], FALSE);
        gtk_widget_set_sensitive(ip_config[0], FALSE);
        gtk_widget_set_sensitive(ip_config[1], FALSE);
        gtk_widget_set_sensitive(ip_config[2], FALSE);
        gtk_widget_set_sensitive(dns_config[0], FALSE);
        gtk_widget_set_sensitive(dns_config[1], FALSE);
        gtk_widget_set_sensitive(vm_ip_config[0], FALSE);
        gtk_widget_set_sensitive(vm_ip_config[1], FALSE);
        gtk_widget_set_sensitive(vm_ip_config[2], FALSE);
        gtk_widget_set_sensitive(vm_dns_config[0], FALSE);
        gtk_widget_set_sensitive(vm_dns_config[1], FALSE);
    }

    return;
}

#ifdef IDV_CLIENT
static int ui_dialog_check_ip_dns(GtkWidget **ip_btn, GtkWidget **dns_btn, input_box_t *input, input_box_t ** err_box1, input_box_t ** err_box2)
{
    int ret;
    char *tmp;
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ip_btn[0]))) {
        // no need
    } else {
        if ((ret = rc_check_input_network( gtk_entry_get_text(GTK_ENTRY(input[0].input)),
                                           gtk_entry_get_text(GTK_ENTRY(input[1].input)),
                                           gtk_entry_get_text(GTK_ENTRY(input[2].input)) )) != 0) {
            switch(ret) {
            case RC_IP_ERR_INVALID_IP:
                //input_box_err_show(&input[0]);
                *err_box1 = &input[0];
                break;
            case RC_IP_ERR_INVALID_NETMASK:
                //input_box_err_show(&input[1]);
                *err_box1 = &input[1];
                break;
            case RC_IP_ERR_INVALID_GATEWAY:
                //input_box_err_show(&input[2]);
                *err_box1 = &input[2];
                break;
            case RC_IP_ERR_CONFLICT_IP_GATEWAY:
                //input_box_err_show(&input[0]);
                //input_box_err_show(&input[2]);
                *err_box1 = &input[0];
                *err_box2 = &input[2];
                break;
            }
            return -1;
        }
    }
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dns_btn[0]))) {
        // do nothing
    } else {    
        tmp = (char *)gtk_entry_get_text(GTK_ENTRY(input[3].input));
        if (strcmp (tmp, "127.0.0.1") != 0 && rc_check_input_ip(tmp) != 0) {
            //input_box_err_show(&input[3]);
            *err_box1 = &input[3];
            return -1;
        }

        tmp = (char *)gtk_entry_get_text(GTK_ENTRY(input[4].input));
/*		if (rc_check_input_ip(tmp) != 0) {
			input_box_err_show(&input[4]);
			return -1;
		}        
 */       
       if (strcmp (tmp, "") == 0 || strcmp (tmp, "127.0.0.1") == 0) {
        	return 0;
        } else {
            if (rc_check_input_ip(tmp) != 0) {
                //input_box_err_show(&input[4]);
                *err_box1 = &input[4];
                return -1;
            }
        }


    }

    return 0;
}

#endif

static int ui_dialog_check_all_config()
{

#ifdef IDV_CLIENT
	int scroll = 0;

	input_box_t * err_box1 = NULL;
	input_box_t * err_box2 = NULL;

	gtk_label_set_text(GTK_LABEL(vm_ip_err_label[0]), "IP地址输入错误，请重新输入");
	input_box_err_hide(&(input_box[7]));
    if (rc_check_hostname((char *)gtk_entry_get_text(GTK_ENTRY(input_box[0].input)), 20) != 0) {
    	err_box1 = &(input_box[0]);
        //input_box_err_show(&(input_box[0]));
    	scroll = 0;
        goto devpage;
    }
    if (rc_check_input_ip(gtk_entry_get_text(GTK_ENTRY(input_box[1].input))) != 0) {
    	err_box1 = &(input_box[1]);
        //input_box_err_show(&(input_box[1]));
    	scroll = 0;
        goto devpage;
    }

    int result = ui_extern_net_status_query_result();

    //cannot change ip if wlan up
    if(result != 2) {
        if (ui_dialog_check_ip_dns(ip_button, dns_button, &(input_box[2]), &err_box1, &err_box2) != 0) {
        	scroll = 1;
            goto devpage;
        }

        if (ui_dialog_check_ip_dns(vm_ip_button, vm_dns_button, &input_box[7], &err_box1, &err_box2) != 0) {
            goto vmpage;
        }

    	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ip_button[1])) && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(vm_ip_button[1]))) {
    		if ((strcmp (gtk_entry_get_text(GTK_ENTRY(ip_config[0])) , gtk_entry_get_text(GTK_ENTRY(vm_ip_config[0]))) == 0)
    				&& (strcmp (gtk_entry_get_text(GTK_ENTRY(ip_config[1])) , gtk_entry_get_text(GTK_ENTRY(vm_ip_config[1]))) == 0))
    		{
    			gtk_label_set_text(GTK_LABEL(vm_ip_err_label[0]), "虚机IP地址和终端一致，请修订");
    			err_box1 = &(input_box[7]);
    			goto vmpage;
    		}
    	}
    }

    return 0;
devpage:
	
	show_devpage();
	if (scroll == 0) {
		set_ajust_min(dev_page_ajust);
	} else {
		set_ajust_max(dev_page_ajust);
	}

	if (err_box1) {
		input_box_err_show(err_box1);
	}
	if (err_box2) {
		input_box_err_show(err_box2);
	}	
	return -1;
	
vmpage:
	show_vmpage();
	if (err_box1) {
		input_box_err_show(err_box1);
	}
	if (err_box2) {
		input_box_err_show(err_box2);
	}	
	return -1;
#else 
	return 0;
#endif
}


static gboolean ui_dialog_config_save(GtkWidget *widget,gpointer data)
{
    ui_set_unconfig();

    if (ui_dialog_check_all_config() != 0) {
        logi("config check fail\n");
    	able_btn_widget();
    	btn_normal_show(okay_button);
        return FALSE;
    } else {
        logi("config check ok\n");
    }
#ifdef IDV_CLIENT
    int error;

    struct rc_netifdev_t* info_netifdev;
    struct wifi_switch_t* switch_status;

    switch_status = malloc(sizeof(struct wifi_switch_t));
    info_netifdev = malloc(sizeof(struct rc_netifdev_t));

    get_sw_status(switch_status);
    ui_get_net_info(info_netifdev);
    ui_dialog_config_save_value(info_netifdev, ip_button, dns_button, ip_config, dns_config);
    

    struct rc_netifdev_t* vm_info_netifdev;
    vm_info_netifdev = malloc(sizeof(struct rc_netifdev_t));
    ui_get_vm_net_info(vm_info_netifdev);
    ui_dialog_config_save_value(vm_info_netifdev, vm_ip_button, vm_dns_button, vm_ip_config, vm_dns_config);

    ui_save_settings(gtk_entry_get_text(GTK_ENTRY(terminal_name)), 
                     gtk_entry_get_text(GTK_ENTRY(server_address)), 
                     info_netifdev, 
                     vm_info_netifdev,
                     switch_status,
                     gtk_combo_box_get_active(GTK_COMBO_BOX(combo)),
                     &error);
    free(info_netifdev); 
    free(vm_info_netifdev);
    if (switch_status != NULL) {
        free(switch_status);
        switch_status = NULL;
    }
#endif
	disable_btn_widget();
	return FALSE;
    //gtk_widget_destroy(dialog);
}


static int ui_dialog_config_ctrl(void *p)
{
    ui_dialog_config_msg_t *msg = p;

    switch (msg->tip_type) {
    case UI_DIALOG_CONFIG_SAVING:
        gtk_label_set_text(GTK_LABEL(label), "Saving config");
        break;
    case UI_DIALOG_CONFIG_RESULT_FAIL:
        gtk_label_set_text(GTK_LABEL(label), "Save fail");
        //show some config that can't apply
        break;
    case UI_DIALOG_CONFIG_RESULT_OK:
        gtk_label_set_text(GTK_LABEL(label), "Save ok");
        //timeout 1s, auto destroy the dialog
        break;
    default:
        break;
    }
	able_btn_widget();
    g_free(msg);

    return 0;
}

gboolean ui_dialog_bt_service_click(GtkWidget *widget, gpointer data)
{
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
        ui_save_bt_service(TRUE);
        logi("bt service set active true\n");
    } else {
        ui_save_bt_service(FALSE);
        logi("bt service set active false\n");
    }	
	return 0;
}

gboolean ui_dialog_power_boot_click(GtkWidget *widget, gpointer data)
{
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
        ui_save_power_boot(TRUE);
        logi("power_boot set active true\n");
    } else {
        ui_save_power_boot(FALSE);
        logi("power_boot set active false\n");
    }	
	return 0;
}
gboolean ui_dialog_boot_speedup_click(GtkWidget *widget, gpointer data)
{
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
        ui_save_boot_speedup(TRUE);
        logi("boot_speedup set active true\n");
    } else {
        ui_save_boot_speedup(FALSE);
        logi("boot_speedup set active false\n");
    }	
	return 0;
}


gboolean ui_dialog_hdmi_audio_click(GtkWidget *widget, gpointer data)
{
#define SUPPORT   1
#define NONSUPPORT 0

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
        ui_extern_set_hdmi_audio(SUPPORT);
        logi("power_boot set active true\n");
    } else {
         ui_extern_set_hdmi_audio(NONSUPPORT);
        logi("power_boot set active false\n");
    }
    
#undef SUPPORT
#undef NONSUPPORT
    return 0;
}

gboolean ui_dialog_e1000_netcard_click(GtkWidget *widget, gpointer data)
{
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
        ui_extern_save_e1000_netcard(TRUE);
        logi("e1000 netcard set active true\n");
    } else {
        ui_extern_save_e1000_netcard(FALSE);
        logi("e1000 netcard set active false\n");
    }	
	return 0;
}

gboolean ui_dialog_usb_emulation_click(GtkWidget *widget, gpointer data)
{
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
        ui_extern_save_usb_emulation(TRUE);
        logi("usb emulation set active true\n");
    } else {
        ui_extern_save_usb_emulation(FALSE);
        logi("usb emulation set active false\n");
    }	
	return 0;
}

static GtkWidget * mk_action_area()
{
	//GtkWidget * tbl;
	GtkWidget * vbox;
	GtkWidget * btn_hbox;
	//GtkWidget * bhox;
	GtkWidget * hid_label;
	GdkPixbuf *src_pixbuf = NULL;
	GdkPixbuf * dst_pixbuf = NULL;
	GtkWidget * image;
	GtkWidget * align;
	
    okay_button = create_button("./icon/btn_green.png", "./icon/btn_green_h.png", "./icon/btn_process.gif", "保存",
    		-1 ,-1, G_CALLBACK (ui_dialog_config_save), NULL);
	set_widget_font_size(okay_button->label, -1, "Snow");
	
    cancal_button = create_button("./icon/btn_gray.png", "./icon/btn_gray_h.png", NULL, "取消",
    		-1, -1, G_CALLBACK (ui_destory_dialog_callback_notify), dialog);

	hid_label = gtk_label_new("");
	gtk_widget_show(hid_label);

    btn_hbox = gtk_hbox_new(FALSE,30);
    
    gtk_box_pack_start(GTK_BOX(btn_hbox), okay_button->container, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(btn_hbox), cancal_button->container, FALSE, FALSE, 5);
    //gtk_box_pack_start(GTK_BOX(btn_hbox), hid_label, FALSE, FALSE, 120);
    gtk_widget_show(btn_hbox);

	align = gtk_alignment_new(0.5,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), btn_hbox);
    gtk_widget_show(align);

    //tbl = gtk_table_new(1, 3, TRUE);

    //gtk_table_attach_defaults (GTK_TABLE (tbl), btn_hbox, 1, 2, 0, 1);
    //gtk_widget_show (tbl);
    
	//src_pixbuf = gdk_pixbuf_new_from_file("./icon/btn_bj.png", NULL);
	src_pixbuf = gdk_pixbuf_new_from_file("./icon/shadow.png", NULL);
    dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, 830, 10, GDK_INTERP_BILINEAR);
	image = gtk_image_new_from_pixbuf(dst_pixbuf);
	g_object_unref(src_pixbuf);
	g_object_unref(dst_pixbuf);
	gtk_widget_show(image);


	vbox = gtk_vbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(vbox), image, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 0);
	gtk_widget_show(vbox);
    return vbox;
}

static GtkWidget *mk_input(GtkWidget **entry_ptr, char * icon, char * text, int width, int height, GtkWidget *widget)
{
	GdkPixbuf *src_pixbuf = NULL;
	GtkWidget * image = NULL;
	GtkWidget * label;
	GtkWidget * hbox;
	GtkWidget * entry;
	GtkWidget * container;

	if (icon) {
	    src_pixbuf = gdk_pixbuf_new_from_file(icon, NULL);
	    image = gtk_image_new_from_pixbuf(src_pixbuf);
	    g_object_unref(src_pixbuf);
	    gtk_widget_show(image);
	}

    label = gtk_label_new(text);
    //set_widget_font_size(label, 20, NULL);
    if (width != -1 && height != -1) {
        gtk_widget_set_size_request(label, width, height);
        gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
    }
    gtk_widget_show(label);

    entry = gtk_entry_new_with_max_length(20);
    if (entry_ptr) {
    	*entry_ptr = entry;
    }

	//entry = gtk_entry_new_with_max_length(20);
	
	container = add_entry_style(entry,  NULL,  NULL, 260, 30, TRUE);
	//gtk_widget_set_size_request(entry, 225, 30);
	gtk_widget_show(entry);

	hbox = gtk_hbox_new(FALSE, 3);
	if (image) {
		gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
	}
	if (widget) {
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
	}
	
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), container, FALSE, FALSE, 0);

	gtk_widget_show(hbox);
	return hbox;
}

static GtkWidget * mk_dev_type(void)
{
	GtkWidget *label;
	//GtkWidget * combo;
	GtkWidget * hbox;
	//GList *glist=NULL;

    label = gtk_label_new("终端类型：");
    gtk_widget_set_size_request(label, LABEL_WIDTH, LABEL_HEIGHT);
    gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
    gtk_widget_show(label);
    //gtk_table_attach_defaults(GTK_TABLE(table[0]), lb, 1, 2, i, i + 1);
#if 0
    combo=gtk_combo_new();
    glist = g_list_append(glist, "专用账号");
    glist = g_list_append(glist, "无账号");
    glist = g_list_append(glist, "多账号");
    gtk_combo_set_popdown_strings(GTK_COMBO(combo),glist);
    gtk_editable_set_editable(GTK_EDITABLE(GTK_ENTRY(GTK_COMBO(combo)->entry)), FALSE);
#endif
    combo = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo),"单用户终端");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo),"多用户终端");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo),"公用终端");
    gtk_widget_set_size_request(combo, 260, 30);
    gtk_widget_show(combo);
    
    hbox = gtk_hbox_new(FALSE, 3);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), combo, FALSE, FALSE, 0);
	gtk_widget_show(hbox);
	
    return hbox;
}

static GtkWidget * mk_item_label(char * text)
{
	GtkWidget * vbox;
	GtkWidget * hide_label;
	GtkWidget * label;
	
    label = gtk_label_new(text);
    set_widget_font_size(label, 12, NULL);
    gtk_widget_show(label);	
	
	hide_label = gtk_label_new("");
	
    vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vbox), hide_label, TRUE, FALSE, 3);
	gtk_widget_show(vbox);
	return vbox;
		
}


static GtkWidget * mk_red_prefix(char * text, int w, int h)
{
	GtkWidget * label;
	label = gtk_label_new(text);
	set_widget_font_size(label, -1, "red");
    if (w != -1 && h != -1) {
        gtk_widget_set_size_request(label, w, h);
        gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
    }
	gtk_widget_show(label);
	return label;
}

static GtkWidget * mk_prompt(char *icon, char * lab_text, int lab_font_size, gfloat x_align, GtkWidget ** err_label)
{
	GtkWidget * align;
	GtkWidget * label = NULL;
	GtkWidget * hbox;
	GtkWidget * image = NULL;
	GdkPixbuf *src_pixbuf = NULL;
	
	if (icon) {
	    src_pixbuf = gdk_pixbuf_new_from_file(icon, NULL);
	    image = gtk_image_new_from_pixbuf(src_pixbuf);
	    g_object_unref(src_pixbuf);
	    gtk_widget_show(image);		
	}

	if (lab_text) {
	    label = gtk_label_new(lab_text);
	    set_widget_font_size(label, lab_font_size, NULL);
	    //gtk_widget_set_size_request(label, set_width, set_height);
	    if (err_label) {
	    	*err_label = label;
	    }

	    gtk_widget_show(label);		
	}

    hbox = gtk_hbox_new(FALSE, 5);
    if (image) {
    	gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
    }
    if (label) {
    	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    }

	
	gtk_widget_show(hbox);

	align = gtk_alignment_new(x_align,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

	return align;
}


static GtkWidget * vbox_pack_err_prompt(GtkWidget * item, GtkWidget * err_prompt, GtkWidget ** expand_widget)
{
	GtkWidget *vbox;
	
	GtkWidget * hid_label;
	
	if (expand_widget) {
		hid_label = gtk_label_new("");		
		gtk_widget_show(hid_label);	
		*expand_widget = hid_label;
	}

	gtk_widget_hide(err_prompt);
    vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), item, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), err_prompt, FALSE, FALSE, 0);	
	if (expand_widget) {
		gtk_box_pack_start(GTK_BOX(vbox), hid_label, FALSE, FALSE, 0);	
	}
	
	
	gtk_widget_show(vbox);
	return vbox;
}

static GtkWidget * mk_basic_info()
{
	GtkWidget *hbox;
	GtkWidget *vbox;

	GtkWidget * label;
	GtkWidget * name;
	GtkWidget * addr;
	GtkWidget * type;
	
	GtkWidget * tips;
	GtkWidget * name_hbox;

	GtkWidget * addr_hbox;
	GtkWidget * prefix = NULL;
	GtkWidget * hid_label;
	//GtkWidget * err_prompt;
	
    label = mk_item_label("基本信息");

	prefix = mk_red_prefix("*", 62, 20);
    name = mk_input(&terminal_name, NULL, "终端名称：", -1, -1, prefix);
    tips = gtk_label_new("(最多20个字符，不可输入特殊字符)");
    set_widget_font_size(tips, 8, NULL);
    gtk_widget_show(tips);
    name_hbox = gtk_hbox_new(FALSE, 6);
    gtk_box_pack_start(GTK_BOX(name_hbox), name, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(name_hbox), tips, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(terminal_name),"focus",G_CALLBACK(ui_win_pop_up_keyboard),NULL);
    g_signal_connect(G_OBJECT(terminal_name),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    gtk_widget_show(name_hbox);

    prefix = mk_red_prefix("*", 39, 20);
    addr = mk_input(&server_address, NULL, "云主机IP地址：", -1, -1, prefix);
    g_signal_connect(G_OBJECT(server_address),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    addr_hbox = gtk_hbox_new(FALSE, 6);
    gtk_box_pack_start(GTK_BOX(addr_hbox), addr, FALSE, FALSE, 0);
/*
	btn_apend_t  * btn;
    btn = create_button("./icon/btn_a.png", "./icon/btn_a_h.png", NULL, "云主机网络检测", -1, -1, NULL, NULL);
    set_widget_font_size(btn->label, -1, "Snow");
    
    gtk_box_pack_start(GTK_BOX(addr_hbox), btn->container, FALSE, FALSE, 0);
*/
    gtk_widget_show(addr_hbox);    


    
    type = mk_dev_type();

    vbox = gtk_vbox_new(FALSE, 0);
    
    name_err = mk_prompt("./icon/ico_abnormal_mm.png", "终端名称输入错误，请重新输入", 8, 0.4, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), vbox_pack_err_prompt(name_hbox, name_err, &name_err_hide), FALSE, FALSE, 0);
	
	address_err = mk_prompt("./icon/ico_abnormal_mm.png", "云主机地址输入错误，请重新输入", 8, 0.4, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), vbox_pack_err_prompt(addr_hbox, address_err, &address_err_hide), FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(vbox), type, FALSE, FALSE, 0);
	gtk_widget_show(vbox);

	hid_label = gtk_label_new("");
	gtk_widget_show(hid_label);
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), hid_label, FALSE, FALSE, 40);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 30);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);

	gtk_widget_show(hbox);

	return hbox;
}


static GtkWidget * radio_cntner(GtkWidget * radio)
{
	GtkWidget * hbox;

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), radio, FALSE, FALSE, 0);
	gtk_widget_show(hbox);
	return hbox;
}

static GtkWidget * checkbox_cntner(GtkWidget * checkbox)
{
	GtkWidget * hbox;

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), checkbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);
	return hbox;
}

static GtkWidget * mk_ip_info(GtkWidget ** radio_lst, GtkWidget ** conf_lst, GtkWidget ** err_lst, GtkWidget ** err_hide_lst, GtkWidget ** err_label, gboolean is_devpage)
{
	//GtkWidget * label;
	GtkWidget * cnter;
	//GtkWidget * hbox;
	GtkWidget * vbox;
	GSList *ip_group = NULL;
	
	GtkWidget * ip_hbox;


	radio_lst[0] = gtk_radio_button_new_with_label(ip_group,"自动获取IP地址");
	ip_group = gtk_radio_button_group (GTK_RADIO_BUTTON(radio_lst[0]));
	gtk_widget_show(radio_lst[0]);
	//radio_cntner(radio_lst[0]);
	//hbox = gtk_hbox_new(FALSE, 0);
	//gtk_box_pack_start(GTK_BOX(box), radio_lst[0], FALSE, FALSE, 5);
	

	radio_lst[1] = gtk_radio_button_new_with_label(ip_group,"使用下列IP地址");
	ip_group = gtk_radio_button_group (GTK_RADIO_BUTTON(radio_lst[1]));
	gtk_widget_show(radio_lst[1]);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), radio_cntner(radio_lst[0]), FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), radio_cntner(radio_lst[1]), FALSE, FALSE, 5);
	//gtk_widget_show(vbox);

	//vbox = gtk_vbox_new(FALSE, 0);

    if (is_devpage) {
	    cnter = mk_input(conf_lst  , NULL, "有线网络IP地址：", LABEL_WIDTH, LABEL_HEIGHT, NULL);
    } else {
	    cnter = mk_input(conf_lst  , NULL, "IP地址：", LABEL_WIDTH, LABEL_HEIGHT, NULL);
    }

    ip_hbox = gtk_hbox_new(FALSE, 6);
    gtk_box_pack_start(GTK_BOX(ip_hbox), cnter, FALSE, FALSE, 0);
/*
	btn_apend_t  * btn;    
    btn = create_button("./icon/btn_a.png", "./icon/btn_a_h.png", NULL, "IP冲突检测", -1, -1, NULL, NULL);
    set_widget_font_size(btn->label, -1, "Snow");
    gtk_box_pack_start(GTK_BOX(ip_hbox), btn->container, FALSE, FALSE, 0);
*/
    gtk_widget_show(ip_hbox);  	


    err_lst[0] = mk_prompt("./icon/ico_abnormal_mm.png", "IP地址输入错误，请重新输入", 8, 0.7, err_label);
	gtk_box_pack_start(GTK_BOX(vbox), vbox_pack_err_prompt(ip_hbox, err_lst[0], err_hide_lst), FALSE, FALSE, 0);

	cnter = mk_input(conf_lst+1, NULL, "子网掩码：", LABEL_WIDTH, LABEL_HEIGHT, NULL);
	err_lst[1] = mk_prompt("./icon/ico_abnormal_mm.png", "子网掩码输入错误，请重新输入", 8, 0.7, err_label + 1);
	gtk_box_pack_start(GTK_BOX(vbox), vbox_pack_err_prompt(cnter, err_lst[1], err_hide_lst + 1), FALSE, FALSE, 0);	
	//gtk_box_pack_start(GTK_BOX(vbox), cnter, FALSE, FALSE, 10);

	cnter = mk_input(conf_lst+2, NULL, "默认网关：", LABEL_WIDTH, LABEL_HEIGHT, NULL);
	err_lst[2] = mk_prompt("./icon/ico_abnormal_mm.png", "默认网关输入错误，请重新输入", 8, 0.7, err_label + 2);
	gtk_box_pack_start(GTK_BOX(vbox), vbox_pack_err_prompt(cnter, err_lst[2], err_hide_lst + 2), FALSE, FALSE, 0);		
	gtk_widget_show(vbox);

	return vbox;
}



static GtkWidget * mk_dns_info(GtkWidget ** radio_lst, GtkWidget ** conf_lst,  GtkWidget ** err_lst, GtkWidget ** err_hide_lst, GtkWidget ** err_label)
{
	//GtkWidget * label;
	GtkWidget * cnter;
	//GtkWidget * hbox;
	GtkWidget * vbox;
	GSList *ip_group = NULL;

	radio_lst[0] = gtk_radio_button_new_with_label(ip_group,"自动获取DNS地址");
	ip_group = gtk_radio_button_group (GTK_RADIO_BUTTON(radio_lst[0]));
	gtk_widget_show(radio_lst[0]);

	radio_lst[1] = gtk_radio_button_new_with_label(ip_group,"使用下列DNS地址");
	ip_group = gtk_radio_button_group (GTK_RADIO_BUTTON(radio_lst[1]));
	gtk_widget_show(radio_lst[1]);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), radio_cntner(radio_lst[0]), FALSE, FALSE, 10);
	gtk_box_pack_start(GTK_BOX(vbox), radio_cntner(radio_lst[1]), FALSE, FALSE, 10);
	//gtk_widget_show(vbox);

	//vbox = gtk_vbox_new(FALSE, 0);
	cnter = mk_input(conf_lst , NULL, "首选DNS地址：", LABEL_WIDTH, LABEL_HEIGHT, NULL);
	err_lst[0] = mk_prompt("./icon/ico_abnormal_mm.png", "DNS地址输入错误，请重新输入", 8, 0.7, err_label);
	gtk_box_pack_start(GTK_BOX(vbox), vbox_pack_err_prompt(cnter, err_lst[0], err_hide_lst), FALSE, FALSE, 0);		
	//gtk_box_pack_start(GTK_BOX(vbox), cnter, FALSE, FALSE, 10);

	cnter = mk_input(conf_lst+1, NULL, "备用DNS地址：", LABEL_WIDTH, LABEL_HEIGHT, NULL);
	err_lst[1] = mk_prompt("./icon/ico_abnormal_mm.png", "DNS地址输入错误，请重新输入", 8, 0.7, err_label + 1);
	gtk_box_pack_start(GTK_BOX(vbox), vbox_pack_err_prompt(cnter, err_lst[1], err_hide_lst + 1), FALSE, FALSE, 0);		
	//gtk_box_pack_start(GTK_BOX(vbox), cnter, FALSE, FALSE, 10);
	gtk_widget_show(vbox);

	return vbox;
}


static GtkWidget * mk_net_info(GtkWidget ** ip_radio, GtkWidget ** ip_conf, GtkWidget ** ip_err, GtkWidget ** ip_err_hide, GtkWidget ** ip_err_label,
		GtkWidget ** dns_radio, GtkWidget ** dns_conf, GtkWidget ** dns_err,  GtkWidget ** dns_err_hide, GtkWidget ** dns_err_label, gboolean is_devpage)
{
	GtkWidget * label;
	GtkWidget * hbox;
	GtkWidget * vbox;
	GtkWidget * ip;
	GtkWidget * dns;
	GtkWidget * hid_label;
    label = mk_item_label("网络配置");


    ip = mk_ip_info(ip_radio, ip_conf, ip_err, ip_err_hide, ip_err_label, is_devpage);
	dns = mk_dns_info(dns_radio, dns_conf, dns_err, dns_err_hide, dns_err_label);
	
    vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), ip, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), dns, FALSE, FALSE, 0);
	gtk_widget_show(vbox);


	hid_label = gtk_label_new("");
	gtk_widget_show(hid_label);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), hid_label, FALSE, FALSE, 40);    
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE,30);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);

	gtk_widget_show(hbox);

	return hbox;
}

static GtkWidget * mk_bt_service(void)
{
    //GtkWidget * label;
    GtkWidget * hbox;
    GtkWidget * vbox;
    GtkWidget * hid_label;
    GtkWidget * bt_service_explain;

    //label = mk_item_label("下载配置");
	hid_label = gtk_label_new("");
	gtk_widget_show(hid_label);

    bt_service_checkbox = gtk_check_button_new_with_label(" 镜像分发服务");    
    gtk_widget_show(bt_service_checkbox);    

    bt_service_explain = gtk_label_new(
        "　　当本地局域网与云主机的网络连接带宽较低时（低于50Mb/s），建议先在一台终端上下载好\n"
        "　　镜像文件，然后再开启终端的镜像分发服务。开启镜像分发服务后，该终端将成为其它终端的\n"
        "　　镜像分发种子机，使得其它终端有可能在局域网的最大带宽下，下载镜像文件，提高各终端的\n"
        "　　下载效率。");
    set_widget_font_size(bt_service_explain, 10, "#666666");
    gtk_label_set_justify(GTK_LABEL(bt_service_explain), GTK_JUSTIFY_LEFT);
    gtk_widget_show(bt_service_explain);

    vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), checkbox_cntner(bt_service_checkbox), FALSE, FALSE, 5);
    g_signal_connect (GTK_OBJECT(bt_service_checkbox), "clicked", G_CALLBACK(ui_dialog_bt_service_click), NULL);
	gtk_box_pack_start(GTK_BOX(vbox), bt_service_explain, FALSE, FALSE, 0);
	gtk_widget_show(vbox);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), hid_label, FALSE, FALSE, 70);
	//gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE,30);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);

    gtk_widget_show(hbox);

    return hbox;
}

static GtkWidget * mk_power_boot(void)
{
    GtkWidget * hbox;
    GtkWidget * vbox;
    GtkWidget * hid_label;
    GtkWidget * power_boot_explain;

	hid_label = gtk_label_new("");
	gtk_widget_show(hid_label);

    power_boot_checkbox = gtk_check_button_new_with_label(" 上电自启动");    
    gtk_widget_show(power_boot_checkbox);

    power_boot_explain = gtk_label_new("　　上电自启动配置需要重启后生效。\n");
    set_widget_font_size(power_boot_explain, 0, "#666666");
    gtk_label_set_justify(GTK_LABEL(power_boot_explain), GTK_JUSTIFY_LEFT);
    gtk_widget_show(power_boot_explain);

    vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), checkbox_cntner(power_boot_checkbox), FALSE, FALSE, 5);
    g_signal_connect (GTK_OBJECT(power_boot_checkbox), "clicked", G_CALLBACK(ui_dialog_power_boot_click), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), power_boot_explain, FALSE, FALSE, 0);
	gtk_widget_show(vbox);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), hid_label, FALSE, FALSE, 70);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox);    

    return hbox;
}

static GtkWidget *mk_boot_speedup(void)
{
    GtkWidget * hbox;
    GtkWidget * vbox;
    GtkWidget * hid_label;
    GtkWidget * boot_speedup_explain;

	hid_label = gtk_label_new("");
	gtk_widget_show(hid_label);

    boot_speedup_checkbox = gtk_check_button_new_with_label(" 开机加速");    
    gtk_widget_show(boot_speedup_checkbox);

    boot_speedup_explain = gtk_label_new("　　开启加速后，可以快速启动系统。网络不好的时候，减少离线登入时间。\n");
    set_widget_font_size(boot_speedup_explain, 0, "#666666");
    gtk_label_set_justify(GTK_LABEL(boot_speedup_explain), GTK_JUSTIFY_LEFT);
    gtk_widget_show(boot_speedup_explain);

    vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), checkbox_cntner(boot_speedup_checkbox), FALSE, FALSE, 5);
    g_signal_connect (GTK_OBJECT(boot_speedup_checkbox), "clicked", G_CALLBACK(ui_dialog_boot_speedup_click), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), boot_speedup_explain, FALSE, FALSE, 0);
	gtk_widget_show(vbox);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), hid_label, FALSE, FALSE, 70);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox);    

    return hbox;
}


static GtkWidget *mk_hdmi_audio(void)
{
    GtkWidget * hbox;
    GtkWidget * vbox;
    GtkWidget * hid_label;
    //GtkWidget * hdmi_audio_explain;

	hid_label = gtk_label_new("");
	gtk_widget_show(hid_label);

    hdmi_audio_checkbox = gtk_check_button_new_with_label(" 支持HDMI音频输出");    
    gtk_widget_show(hdmi_audio_checkbox);

#if 0
    hdmi_audio_explain = gtk_label_new("     支持HDMI音频输出配置立即生效。\n");
    set_widget_font_size(hdmi_audio_explain, 0, "#666666");
    gtk_label_set_justify(GTK_LABEL(hdmi_audio_explain), GTK_JUSTIFY_LEFT);
    gtk_widget_show(hdmi_audio_explain);
#endif

    vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), checkbox_cntner(hdmi_audio_checkbox), FALSE, FALSE, 5);
    g_signal_connect (GTK_OBJECT(hdmi_audio_checkbox), "clicked", G_CALLBACK(ui_dialog_hdmi_audio_click), NULL);
    //gtk_box_pack_start(GTK_BOX(vbox), hdmi_audio_explain, FALSE, FALSE, 0);
	gtk_widget_show(vbox);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), hid_label, FALSE, FALSE, 70);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox);    

    return hbox;
}

static GtkWidget * mk_e1000_netcard(void)
{
    GtkWidget * hbox;
    GtkWidget * vbox;
    GtkWidget * hid_label;

	hid_label = gtk_label_new("");
	gtk_widget_show(hid_label);

    e1000_netcard_checkbox = gtk_check_button_new_with_label(" E1000网卡");
    gtk_widget_show(e1000_netcard_checkbox);

    vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), checkbox_cntner(e1000_netcard_checkbox), FALSE, FALSE, 5);
    g_signal_connect (GTK_OBJECT(e1000_netcard_checkbox), "clicked", G_CALLBACK(ui_dialog_e1000_netcard_click), NULL);
	gtk_widget_show(vbox);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), hid_label, FALSE, FALSE, 70);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox);    

    return hbox;
}

static GtkWidget * mk_usb_emulation(void)
{
    GtkWidget * hbox;
    GtkWidget * vbox;
    GtkWidget * hid_label;

	hid_label = gtk_label_new("");
	gtk_widget_show(hid_label);

    usb_emulation_checkbox = gtk_check_button_new_with_label(" 虚拟USB控制器");    
    gtk_widget_show(usb_emulation_checkbox);

    vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), checkbox_cntner(usb_emulation_checkbox), FALSE, FALSE, 5);
    g_signal_connect (GTK_OBJECT(usb_emulation_checkbox), "clicked", G_CALLBACK(ui_dialog_usb_emulation_click), NULL);
	gtk_widget_show(vbox);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), hid_label, FALSE, FALSE, 70);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox);    

    return hbox;
}

/**
 * layer_manage widget
 */
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

static void set_switch_button_icon(btn_apend_t *object, char *icon, char *icon_mouse_on)
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

static void layer_manage_confirm_cb(void *data)
{
    logi("layer_manage_confirm_cb\n");
    layer_manage_switch_status = FALSE;
    ui_extern_set_app_layer_status(0);
    set_switch_button_icon(layer_manage_switch_btn, SWITCH_OFF_PATH, SWITCH_OFF_PATH);
}

static void layer_manage_cancel_cb(void *data)
{
    logi("layer_manage_cancel_cb\n");
}

static gint layer_manage_button_callback(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    logi("layer_manage_button_callback\n");
    if (layer_manage_switch_status == TRUE) {
        ui_extern_show_dialog_tips_layer(layer_manage_confirm_cb, NULL, layer_manage_cancel_cb, NULL);
    } else {
        //TODO zjj: used for debugging
        //layer_manage_switch_status = TRUE;
        //ui_extern_set_app_layer_status(1);
        //set_switch_button_icon(layer_manage_switch_btn, SWITCH_ON_PATH, SWITCH_ON_PATH);
    }
    return 0;
}

static GtkWidget *mk_switch_box(gboolean sw_mode, char *text, char *path_on, char *path_off, btn_apend_t **switch_btn, GCallback callback)
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

static GtkWidget * mk_layer_manage(void)
{
    GtkWidget * hbox = NULL;
    GtkWidget * hid_label;
    GtkWidget * layer_switch_area;

    hid_label = gtk_label_new("");
    gtk_widget_show(hid_label);

    layer_switch_area = mk_switch_box(layer_manage_switch_status, " 应用分层管理", SWITCH_ON_PATH, SWITCH_OFF_PATH, &layer_manage_switch_btn, (GCallback)(layer_manage_button_callback));

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), hid_label, FALSE, FALSE, 70);
    gtk_box_pack_start(GTK_BOX(hbox), layer_switch_area, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

    return hbox;
}
/**
 * layer_manage widget
 */

static GtkWidget * mk_other_info()
{
	GtkWidget * vbox;
	GtkWidget * bt_service_area;
	GtkWidget * power_boot_area = NULL;
	GtkWidget * boot_speedup_area = NULL;
    GtkWidget * hdmi_audio_area = NULL;
    GtkWidget * e1000_netcard_area = NULL;
    GtkWidget * usb_emulation_area = NULL;
    GtkWidget * layer_manage_area = NULL;

    bt_service_area = mk_bt_service();
    if(ui_extern_is_using_power_boot() != -1) {
        power_boot_area = mk_power_boot();
    }
	boot_speedup_area = mk_boot_speedup();
	
    //if the IDV support hdmi connected and the vmmode is emulation
    if(ui_extern_is_hdmi_connected() && ui_extern_start_vmmode_is_emulation ()) {
        hdmi_audio_area = mk_hdmi_audio();
    }
    // win7-emulation-nonwifi mode, show e1000 netcard checkbox and usb emulation checkbox
    if (ui_extern_using_e1000_netcard() != -1) {
        e1000_netcard_area = mk_e1000_netcard();
    }
    if (ui_extern_is_usb_emulation() != -1) {    
        usb_emulation_area = mk_usb_emulation();
    }
    if (ui_extern_is_using_app_layer() != -1) {
        layer_manage_area = mk_layer_manage();
    }

    vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), bt_service_area, FALSE, FALSE, 5);
    if (power_boot_area != NULL){
        gtk_box_pack_start(GTK_BOX(vbox), power_boot_area, FALSE, FALSE, 5);
    }
	if(boot_speedup_area){
		gtk_box_pack_start(GTK_BOX(vbox), boot_speedup_area, FALSE, FALSE, 5);
	}
    if (hdmi_audio_area) {
        gtk_box_pack_start(GTK_BOX(vbox), hdmi_audio_area, FALSE, FALSE, 5);
    }
    if (e1000_netcard_area) {
        gtk_box_pack_start(GTK_BOX(vbox), e1000_netcard_area, FALSE, FALSE, 5);
    }
    if (usb_emulation_area) {
        gtk_box_pack_start(GTK_BOX(vbox), usb_emulation_area, FALSE, FALSE, 5);
    }
    if (layer_manage_area) {
       gtk_box_pack_start(GTK_BOX(vbox), layer_manage_area, FALSE, FALSE, 5);
    }

	return vbox;
}

static gint dev_set_handle(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
	show_devpage();
    return FALSE;
}


static gint vm_set_handle(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
	show_vmpage();
    return FALSE;
}

static gint ping_handle(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
	btn_disable_all();

	set_widget_font_size(ping->label, -1, "LightSeaGreen");
	gtk_widget_show(ping_page);
	gtk_widget_show(ping->under_img_h);
    return FALSE;
}

static gint net_wifi_handle(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
	show_netwifipage();
    return FALSE;
}


static gint ip_err_handle(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
	btn_disable_all();

	set_widget_font_size(ip_err->label, -1, "LightSeaGreen");
	gtk_widget_show(ip_err_page);
	gtk_widget_show(ip_err->under_img_h);
    return FALSE;
}

static gint display_set_handle(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
	btn_disable_all();

    if (display_page != NULL && display_set != NULL) {
	    set_widget_font_size(display_set->label, -1, "LightSeaGreen");
	    gtk_widget_show(display_page);
	    gtk_widget_show(display_set->under_img_h);
    }
    return FALSE;
}

static gint other_set_handle(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
	btn_disable_all();

	set_widget_font_size(other_set->label, -1, "LightSeaGreen");
	gtk_widget_show(other_page);
	gtk_widget_show(other_set->under_img_h);

    return FALSE;
}

static GtkWidget * btn_add_underline(btn_apend_t * btn, char * icon, char * icon_h, int width, int height)
{
    GdkPixbuf *src_pixbuf = NULL;
    GdkPixbuf *dst_pixbuf = NULL;
	GtkWidget * fixed;
    GtkWidget * align;
    
    if(icon) {
        src_pixbuf = gdk_pixbuf_new_from_file(icon, NULL);
        dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, width, height, GDK_INTERP_BILINEAR);
        btn->under_img = gtk_image_new_from_pixbuf(dst_pixbuf);
        g_object_unref(src_pixbuf);
        g_object_unref(dst_pixbuf);
        gtk_widget_show(btn->under_img);
    }

    if (icon_h) {
        src_pixbuf = gdk_pixbuf_new_from_file(icon_h, NULL);
		dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, width, height, GDK_INTERP_BILINEAR);
		btn->under_img_h = gtk_image_new_from_pixbuf(dst_pixbuf);
		g_object_unref(src_pixbuf);
		g_object_unref(dst_pixbuf);
		gtk_widget_hide(btn->under_img_h);
    }
    
    fixed = gtk_fixed_new();
    if (btn->under_img) {
    	 gtk_fixed_put(GTK_FIXED(fixed), btn->under_img, 0, 0);
    }

    if (btn->under_img_h) {
    	gtk_fixed_put(GTK_FIXED(fixed), btn->under_img_h, 0, 0);
    }

	gtk_widget_set_size_request(fixed, width, height); 
	   
	gtk_widget_show(fixed);

	align = gtk_alignment_new(0.5,0.5, 0,0);
    gtk_container_add(GTK_CONTAINER(align), fixed);
	gtk_widget_show(align);
	
	return align;
}


static GtkWidget * add_vbox_ctner(GtkWidget * widget1, GtkWidget * widget2)
{
	GtkWidget * vbox;
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), widget1, FALSE, FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox), widget2, FALSE, FALSE,0);
	gtk_widget_show(vbox);	
	return vbox;
}

static GtkWidget * mk_item_btn()
{
	GtkWidget * hbox;
	//GtkWidget * vbox1;
	//GtkWidget * vbox2;
	GtkWidget * ctner;
    GdkPixbuf *src_pixbuf = NULL;
    GdkPixbuf *dst_pixbuf = NULL;
    GtkWidget * image;
    GtkWidget * vbox_all;
    int wifi_terminal = 0;
    int hbox_sapce = 0;
    int is_emulation = 0;

    /* 判断是不是WIFI终端 */
    ui_extern_is_wifi_terminal(&wifi_terminal);
    is_emulation = ui_extern_start_vmmode_is_emulation();
    if (wifi_terminal && is_emulation) {
        hbox_sapce = 15;
    } else if (wifi_terminal || is_emulation){
        hbox_sapce = 25;
    } else {
        hbox_sapce = 35;
    }
    
    hbox = gtk_hbox_new(FALSE, 10);

	dev_set = create_button("./icon/btn_setup.png", "./icon/btn_gray_h.png", NULL, "终端配置", -1, -1, G_CALLBACK(dev_set_handle), &dev_set);
	ctner = btn_add_underline(dev_set, "./icon/btn_bj.png", "./icon/tab_current.png", 85, 2);
	
	gtk_box_pack_start(GTK_BOX(hbox), add_vbox_ctner(dev_set->container, ctner), FALSE, FALSE,hbox_sapce);
	set_widget_font_size(dev_set->label, 13, "LightSeaGreen");
	gtk_widget_show(dev_set->under_img_h);

	
	vm_set 	= create_button("./icon/btn_setup.png", "./icon/btn_gray_h.png", NULL, "虚机配置", -1, -1, G_CALLBACK(vm_set_handle), &vm_set);
	ctner = btn_add_underline(vm_set, "./icon/btn_bj.png", "./icon/tab_current.png", 85, 2);

	gtk_box_pack_start(GTK_BOX(hbox), add_vbox_ctner(vm_set->container, ctner), FALSE, FALSE, hbox_sapce);	
	set_widget_font_size(vm_set->label, 13, NULL);
	
	
	ping = create_button("./icon/btn_setup.png", "./icon/btn_gray_h.png", NULL, "Ping服务", -1, -1, G_CALLBACK(ping_handle), &ping);
	ctner = btn_add_underline(ping, "./icon/btn_bj.png", "./icon/tab_current.png", 85, 2);
	gtk_box_pack_start(GTK_BOX(hbox), add_vbox_ctner(ping->container, ctner), FALSE, FALSE, hbox_sapce);	
	set_widget_font_size(ping->label, 13, NULL);	
    
	if (wifi_terminal) {
        netwifi_set = create_button("./icon/btn_setup.png", "./icon/btn_gray_h.png", NULL, "无线管理", -1, -1, G_CALLBACK(net_wifi_handle), &ping);
    	ctner = btn_add_underline(netwifi_set, "./icon/btn_bj.png", "./icon/tab_current.png", 85, 2);
    	gtk_box_pack_start(GTK_BOX(hbox), add_vbox_ctner(netwifi_set->container, ctner), FALSE, FALSE, hbox_sapce);	
    	set_widget_font_size(netwifi_set->label, 13, NULL);
    }

    if (is_emulation) {
        display_set = create_button("./icon/btn_setup.png", "./icon/btn_gray_h.png", NULL, "屏幕设置", -1, -1, G_CALLBACK(display_set_handle), &display_set);
        ctner = btn_add_underline(display_set, "./icon/btn_bj.png", "./icon/tab_current.png", 85, 2);
        gtk_box_pack_start(GTK_BOX(hbox), add_vbox_ctner(display_set->container, ctner), FALSE, FALSE, hbox_sapce);	
        set_widget_font_size(display_set->label, 13, NULL);
    }

    other_set = create_button("./icon/btn_setup.png", "./icon/btn_gray_h.png", NULL, "其他设置", -1, -1, G_CALLBACK(other_set_handle), &other_set);
	ctner = btn_add_underline(other_set, "./icon/btn_bj.png", "./icon/tab_current.png", 85, 2);
	gtk_box_pack_start(GTK_BOX(hbox), add_vbox_ctner(other_set->container, ctner), FALSE, FALSE, hbox_sapce);	
	set_widget_font_size(other_set->label, 13, NULL);
    
	ip_err = create_button("./icon/btn_setup.png", "./icon/btn_gray_h.png", NULL, "IP冲突检测", 100, 38, G_CALLBACK(ip_err_handle), &ip_err);
	ctner = btn_add_underline(ip_err, "./icon/btn_bj.png", "./icon/tab_current.png", 100, 2);
	gtk_box_pack_start(GTK_BOX(hbox), add_vbox_ctner(ip_err->container, ctner), FALSE, FALSE, hbox_sapce);	
	set_widget_font_size(ip_err->label, 13, NULL);	
	gtk_widget_hide(ip_err->container);
	gtk_widget_hide(ctner);
	
	gtk_widget_show(hbox);


	src_pixbuf = gdk_pixbuf_new_from_file("./icon/btn_bj.png", NULL);
	dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, 830, 5, GDK_INTERP_BILINEAR);
	image = gtk_image_new_from_pixbuf(dst_pixbuf);
	g_object_unref(src_pixbuf);
	g_object_unref(dst_pixbuf);
	gtk_widget_show(image);

	vbox_all = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_all), hbox, FALSE, FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox_all), image, FALSE, FALSE,0);
	gtk_widget_show(vbox_all);
	return 	vbox_all;
}

static int ui_dialog_config_init(void)
{
    GtkWidget *scrolled_window;
    GtkWidget * align_set_vm;	
    GtkWidget * align_wifi;

    GtkWidget * basic_info;
    GtkWidget * net_info;
    GtkWidget * vm_net_info;
    GtkWidget * title_bar;
    GtkWidget * item_btn;
    GtkWidget * wifi_info;

    GtkWidget * view;
    GtkWidget *align;
    int wifi_terminal;

    netwifi_page = NULL;
    display_page = NULL;
    display_set = NULL;
    netwifi_set = NULL;

    //int i = 0;
    /* Create the widgets */
    dialog = gtk_dialog_new();
	
	title_bar = title_bar_init("设置", dialog, 1);
	item_btn = mk_item_btn();

    gtk_box_pack_start(GTK_BOX( GTK_DIALOG(dialog)->vbox), title_bar, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX( GTK_DIALOG(dialog)->vbox), item_btn, FALSE, FALSE, 15);
	
    action_area = mk_action_area();
    //gtk_box_pack_start(GTK_BOX( GTK_DIALOG(dialog)->vbox), item_btn, FALSE, FALSE, 15);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), action_area);
    /* Ensure that the dialog box is destroyed when the user clicks ok. */
    g_signal_connect (G_OBJECT (dialog), "realize",
                      G_CALLBACK (ui_compt_realize), &ui_dialog_config);

    basic_info = mk_basic_info();
    net_info = mk_net_info(ip_button, ip_config, ip_err_show, ip_err_hide,ip_err_label ,dns_button, dns_config, dns_err_show, dns_err_hide, dns_err_label, TRUE);
    tab_vbox[0] = gtk_vbox_new(FALSE,10);
    gtk_box_pack_start(GTK_BOX(tab_vbox[0]), basic_info, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(tab_vbox[0]), net_info, FALSE, FALSE, 5);
    gtk_widget_show(tab_vbox[0]);

	align = gtk_alignment_new(0.0,0, 0,0);
    gtk_container_add(GTK_CONTAINER(align), tab_vbox[0]);
	gtk_widget_show(align);

    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    
    ui_dialog_white_background(scrolled_window);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), GTK_SHADOW_NONE);
    //gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), GTK_SHADOW_IN);
    gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 0);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    
    view = gtk_viewport_new(NULL, NULL);
    gtk_viewport_set_shadow_type(GTK_VIEWPORT(view), GTK_SHADOW_NONE);
    ui_dialog_white_background(view);
    gtk_widget_show(view);
    gtk_container_add(GTK_CONTAINER (view), align);
    gtk_container_add(GTK_CONTAINER (scrolled_window), view);
    
   //gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window), align);                                    
    /* 对话框窗口内部包含一个vbox构件 */								
    //gtk_box_pack_start (GTK_BOX (GTK_DIALOG(window)->vbox), scrolled_window, TRUE, TRUE, 0);
    dev_page_ajust = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
    gtk_widget_set_size_request(scrolled_window, 800, 400);
    gtk_widget_show (scrolled_window);
    //gtk_widget_hide (scrolled_window);
	dev_page = scrolled_window;
	
	gtk_box_pack_start(GTK_BOX( GTK_DIALOG(dialog)->vbox), dev_page, FALSE, FALSE, 10);
    
    
    vm_net_info = mk_net_info(vm_ip_button, vm_ip_config, vm_ip_err_show, vm_ip_err_hide, vm_ip_err_label,vm_dns_button, vm_dns_config, vm_dns_err_show, vm_dns_err_hide, vm_dns_err_label, FALSE);
    tab_vbox[1] = gtk_vbox_new(FALSE,10);
    gtk_box_pack_start(GTK_BOX(tab_vbox[1]), vm_net_info, FALSE, FALSE, 10);

    gtk_widget_show(tab_vbox[1]);


	align_set_vm = gtk_alignment_new(0.0, 0, 0,0);
    gtk_container_add(GTK_CONTAINER(align_set_vm), tab_vbox[1]);
	gtk_widget_hide(align_set_vm);
	
	vm_page = align_set_vm;
	gtk_box_pack_start(GTK_BOX( GTK_DIALOG(dialog)->vbox), vm_page, FALSE, FALSE, 0);


	ping_page = ui_config_ping_init();
	gtk_widget_hide(ping_page);
	gtk_box_pack_start(GTK_BOX( GTK_DIALOG(dialog)->vbox), ping_page, FALSE, FALSE, 0);

    /* 判断是不是WIFI终端 */
    ui_extern_is_wifi_terminal(&wifi_terminal);
    if (wifi_terminal) {
        wifi_info = ui_config_wifimanage_init();
        tab_vbox[2] = gtk_vbox_new(FALSE,10);
        gtk_box_pack_start(GTK_BOX(tab_vbox[2]), wifi_info, FALSE, FALSE, 10);
        gtk_widget_show(tab_vbox[2]);
        align_wifi = gtk_alignment_new(0.6, 0, 0,0);
        gtk_container_add(GTK_CONTAINER(align_wifi), tab_vbox[2]);
        gtk_widget_hide(align_wifi);
        
        netwifi_page = align_wifi;  
        gtk_box_pack_start(GTK_BOX( GTK_DIALOG(dialog)->vbox), netwifi_page, FALSE, FALSE, 50);
    }
    
	ip_err_page = ui_config_ip_err_init();
	gtk_widget_hide(ip_err_page);
	gtk_box_pack_start(GTK_BOX( GTK_DIALOG(dialog)->vbox), ip_err_page, FALSE, FALSE, 0);

    // if start with emulation then show display control
    if (ui_extern_start_vmmode_is_emulation()) {
        display_page = ui_config_scrnmanage_init();
        gtk_widget_hide(display_page);
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), display_page, FALSE, FALSE, 10);
    }

    other_page = mk_other_info();
    gtk_widget_hide(other_page);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), other_page, FALSE, FALSE, 10);   
	//gtk_fixed_put(GTK_FIXED(GTK_DIALOG(dialog)->vbox), other_page_scrn, 50, 50);
	//gtk_box_pack_start(GTK_BOX( other_page), other_page_scrn, FALSE, FALSE, 10);   
	//other_page_scrn = ui_config_scrnmanage_init();
	//gtk_widget_hide(scrn_page);
	//gtk_box_pack_start(GTK_BOX( GTK_DIALOG(dialog)->vbox), other_page, FALSE, FALSE, 0);

	//gtk_window_set_decorated(GTK_WINDOW(dialog), FALSE);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_decorated(GTK_WINDOW(dialog),FALSE);    
    gtk_widget_set_size_request(dialog, 850, 580);
	gtk_window_set_position( GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
	ui_dialog_white_background(dialog);

    ui_dialog_config_reload();
    return 0;
}


void ui_dialog_config_hide()
{
    if (dialog) {
        gtk_widget_hide(dialog);
    }
}


void ui_dialog_config_show(void)
{
    if (dialog != NULL) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
    ui_dialog_config_init();
    gtk_widget_show(dialog);
}

void ui_dialog_config_adapt(void)
{
    if(dialog) {
        logi("ui_dialog_config_adapt\n");
        gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
    }
}

void ui_dialog_config_destroy(void)
{
    if (dialog) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
}

struct ui_comp_s ui_dialog_config = {
    .type = UI_TYPE_DIALOG_CONFIG,
    .subtype = UI_TOPLEVEL_TYPE_DIALOG,
    .is_init = UI_NEED_INIT,
    .widget = &dialog,
    .init = ui_dialog_config_init,
    .hide = ui_dialog_config_hide,
    .show = ui_dialog_config_show,
    .adapt = ui_dialog_config_adapt,
    .ctrl = ui_dialog_config_ctrl,
    .destroy = ui_dialog_config_destroy,
};


