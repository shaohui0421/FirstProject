#ifndef _APPLICATION_C_INTERFACES_H
#define _APPLICATION_C_INTERFACES_H

#include <sys/types.h>
#include "ui_api.h"

#ifdef __cplusplus  
extern "C" {  
#endif 

struct setting_param
{
	char* hostname;
	char* serverip;
	struct rc_netifdev_t* net_info;
	struct rc_netifdev_t* vm_net_info;
	int mode;//0: special mode 1: multiuser mode 2:public mode
};

struct about_info
{
	char model[64];
	char hw_ver[64];
	char sw_ver[64];
	char os_ver[64];
	char sn[64];
	char mac[64];
    char wlan_mac[64];
    char hostname[64];
    char ip[64];
};

struct wifi_switch_t
{
    gboolean new_wifi_status;
    gboolean old_wifi_status;
    gboolean new_wssid_status;
    gboolean old_wssid_status;
};

int ui_get_about_info(struct about_info* about_info);
int ui_get_net_info(struct rc_netifdev_t* info);
int ui_get_vm_net_info(struct rc_netifdev_t* info);
int ui_get_server_ip(char* ip);
int ui_get_host_name(char* name);

/*
mode: 0: special 1:multiuser 2:public
readonly: 0:can set mode in setting, 1:can not set mode in setting
*/
int ui_get_terminal_mode(int* mode, int* readonly);
int ui_get_connection_info(int* connected);
void ui_get_last_logined_user(char *username, size_t user_len,
    char *password, size_t psw_len, int *remember_flag);
int ui_localmode_quit(int result);
int ui_localmode_checkerror(int result);
int ui_account_login(char* username, char* passwd, int remember_flag);
int ui_bind_user(int bind);
int ui_check_admin_passwd(char* passwd);
int ui_shutdown_terminal();
int ui_guest_login();
int ui_public_login();
int ui_establish_connection();
int ui_destroy_connection();
int ui_enter_local_mode();
void ui_show_retry_downloading();
void ui_get_auth_info(auth_info_t *info);
void ui_save_auth_info(const auth_info_t *info);
void ui_save_auto_connect_info(int auto_connect);


int ui_getssid_whitelist(char (*ssid)[128], int max_list_num);
int ui_get_whitelist_num(void);
int ui_get_resolution_list(ui_res_info *res_info,int max_list_size);
int ui_set_display_info(const ui_res_info *res_info);
int ui_set_resolution(const int width, const int height);
void ui_get_display_info(int* width, int* height, int* custom);
int ui_is_display_info_ini_exist();
int ui_delete_display_info_ini();
int ui_set_ext_display_info(const ui_res_info *res_info, int num);
void ui_get_ext_display_info(ui_ext_res_info *res_info);
int ui_is_display_info_section_exist(const char *section);
int ui_delete_resolution_info_section();
void ui_show_dialog_tips_dpi(void *callback1, void* args1, void* callback2, void* args2);
void ui_set_hdmiaudio_info(int hdmiaudio);
int ui_get_hdmiaudio_info();
int ui_is_hdmi_connected();

int ui_get_display_port_number();
int ui_get_display_port_connected_number();
int ui_is_display_connected(int port);
int ui_is_display_enable(int port);
int ui_get_display_resolution_list(int port, ui_res_info *res_info, int max_list_size);


/*
type: 
1:no image in web
2:no policy in web
4:image recovery
*/
void ui_tips_ok(int type);
int ui_modify_passwd(const char* username, const char* password, const char* new_passwd);
int ui_reboot_terminal();

int ui_enter_settings();
int ui_notify_setting_result();
//return value: -1: input param err, 0: same setting, not set; 1: set success; 2: set but has error, error store in err
#if 1
int ui_save_settings(const char* hostname, const char* serverip, struct rc_netifdev_t* net_info, struct rc_netifdev_t* vm_net_info, struct wifi_switch_t* sw, int mode, int* err);
void ui_save_setting_auth(const auth_info_t       *auth_info);

#else
int ui_save_settings(struct setting_param* param, int* err);
#endif
//used when user canceled entering admin passwd, or canceled setting
void ui_cancel_settings();
void ui_cancel_auto_shutdown();
int ui_new_deploy(const int mode, const char* bind_user);
int ui_get_bt_service(int *bt_service);
int ui_save_bt_service(int bt_service);
int ui_get_power_boot(int *power_boot);
int ui_save_power_boot(int power_boot);
int ui_get_boot_speedup(int *boot_speedup);
int ui_save_boot_speedup(int boot_speedup);
int ui_is_new_deploy(int *is_new_deploy);
int ui_is_wifi_terminal(int *wifi_terminal);
int ui_vm_start_is_emulaton_vmmode();
int ui_userinfomgr_permitted();
int ui_enter_wifi_config();
int ui_enter_auth_config();
int ui_notify_wifi_config_result();
int ui_notify_auth_config_result();
void ui_wireless_netcard_enable(const int mode);
void ui_wireless_netcard_disconnect(void);
void ui_wireless_netcard_connect(const int hide_net, const ui_scanresult *data);
void ui_wifi_authenticate_enter(const int type, const upload_config_s config);
ui_scanresult* ui_wifi_list_scan_result(int *length);
ui_wifiinfo* ui_wifi_status_query_result(void);
int ui_wireless_netcard_query_result();
int ui_wifi_forget_net_handle(const int net_id);
int ui_wifi_saved_net_query_result(const char *ssid, const char *ssid_mac, const int proto);
int ui_net_status_query_result(void);
void ui_cancel_wifi_config();
void ui_cancel_auth_config();
void ui_wifi_cancel_button_handle(void);
void ui_wifi_cancel_show_wifipwd();
int ui_usb_copy_base(void);
int ui_get_hide_guest_login();
int ui_regex_match(const char *input_str, const char *regex_str);
int ui_upgrade_for_class(void);
void ui_start_net_auth(void);
int ui_get_auth_status();

void ui_get_current_dpi(char *width, char *height);
int ui_get_wired1xauth_exist();
int ui_delete_auth_info();
void ui_save_e1000_netcard(int e1000_netcard);
int ui_using_e1000_netcard();
void ui_save_usb_emulation(int usb_emulation);
int ui_is_usb_emulation();
int ui_is_using_app_layer();
void ui_set_app_layer_status(int status);
void ui_show_dialog_tips_layer(void *callback1, void* args1, void* callback2, void* args2);

void ui_reset_logic_machine();
int ui_using_powerboot();

#if 0
int ui_set_net_info(struct rc_netifdev_t* info);
int ui_set_vm_net_info(struct rc_netifdev_t* info);
int ui_set_server_ip(char* ip);
int ui_set_host_name(char* name);
int ui_set_terminal_mode(int mode);
#endif

#ifdef __cplusplus  
};  
#endif 

#endif//_APPLICATION_C_INTERFACES_H
