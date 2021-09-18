#ifndef UI_EXTERN_H
#define UI_EXTERN_H

#include <string.h>
#include "ui_api.h"

void ui_extern_giveup_connect();
void ui_extern_reconnect();
void ui_extern_local_mode();
int ui_extern_verify_adminpwd(char *);
void ui_extern_user_login(char *username, char *pwd, int);
void ui_extern_bind_usr(int);
void ui_extern_shutdown(int);
void ui_extern_public_login();
void ui_extern_cancal_setting();
void ui_extern_cancal_auto_shutdown();
void ui_extern_guest_login();
void ui_extern_get_server_ip(char *);
void ui_extern_get_hostname(char *);
void ui_extern_get_terminal_mode(int *, int *);
void ui_extern_download_retry();
void ui_extern_tips_confirm(int);
void ui_extern_enter_local_mode();
void ui_extern_get_last_logined_user(char *, int, char *, int, int *);
void ui_extern_modify_passwd(const char*, const char*, const char*);
void ui_extern_new_deploy(int mode, const char* bind_user);
void ui_extern_back2_netmode(int value);
void ui_extern_enter_settings();
void ui_extern_get_bt_service(int *bt_service);
void ui_extern_get_power_boot(int *power_boot);
//void ui_extern_get_boot_speedup(int *boot_speedup);
int ui_extern_get_hdmi_audio();
void ui_extern_set_hdmi_audio(int hdmiaudio);
int ui_extern_is_hdmi_connected();
void ui_extern_is_new_deploy(int *is_new_deploy);
void ui_extern_is_wifi_terminal(int *wifi_terminal);
void ui_extern_enter_wifi_config();
void ui_extern_enter_auth_config(void);

void ui_extern_wireless_netcard_enable(const int mode);
void ui_extern_wireless_netcard_disconnect(void);
void ui_extern_wireless_netcard_connect(const int hide_net, const ui_scanresult *data);
void ui_extern_wifi_authenticate_enter(const int type, const upload_config_s config);
ui_scanresult* ui_extern_wifi_list_scan_result(int *length);
ui_wifiinfo* ui_extern_wifi_status_query_result();
int ui_extern_wireless_netcard_query_result();
int ui_extern_start_vmmode_is_emulation();
int ui_extern_userinfomgr_permitted();
int ui_extern_wifi_forget_net_handle(const int net_id);
int ui_extern_wifi_saved_net_query_result(const char *ssid, const char *ssid_mac, const int proto);
int ui_extern_net_status_query_result();
void ui_extern_cancel_wifi_config();
void ui_extern_wifi_cancel_button_handle(void);
void ui_extern_cancel_auth_config(void);
void ui_extern_cancel_show_wifipwd();
void ui_extern_usb_copy_base();
int ui_extern_getssid_whitelist(char (*ssid)[128], int max_list_num);
int ui_extern_get_whitelist_num(void);
int ui_extern_wifi_forget_notwhite_ssid();
int ui_extern_set_resolution(const int width, const int height);
int ui_extern_get_resolution_list(ui_res_info *res_info, int max_list_size);
int ui_extern_set_displayinfo(const ui_res_info* res_info);
void ui_extern_get_displayinfo(int* width, int* height, int* custom);
int ui_extern_is_display_info_ini_exist();
int ui_extern_delete_display_info_ini();
int ui_extern_set_ext_displayinfo(const ui_res_info *res_info, int num);
void ui_extern_get_ext_displayinfo(ui_ext_res_info* res_info);
int ui_extern_is_display_info_section_exist(const char *section);
int ui_extern_delete_resolution_info_section();
void ui_extern_show_dialog_tips_dpi(void *callback1, void* args1, void* callback2, void* args2);
void ui_extern_upgrade_for_class(void);
void ui_extern_get_auth_info(auth_info_t *info);
int ui_extern_get_auth_status();
void ui_extern_save_auth_setting(const auth_info_t *info);

void ui_extern_save_auth_info(const auth_info_t *info);
void ui_extern_save_auto_connect_info(int auto_connect);
int ui_extern_get_wired1xauth_exist();
int ui_extern_delete_auth_info();

void ui_extern_get_current_dpi(char *width,char *height);
int ui_extern_get_display_port_number();
int ui_extern_get_display_port_connected_number();
int ui_extern_is_display_enable(int port);
int ui_extern_is_display_connected(int port);
int ui_extern_get_display_resolution_list(int port, ui_res_info *res_info, int max_list_size);
void ui_extern_save_boot_speedup(int boot_speedup);
void ui_extern_save_e1000_netcard(int e1000_netcard);
int ui_extern_using_e1000_netcard();
void ui_extern_save_usb_emulation(int usb_emulation);
int ui_extern_is_usb_emulation();
int ui_extern_is_using_app_layer();
int ui_extern_is_using_power_boot();
void ui_extern_set_app_layer_status(int status);
void ui_extern_show_dialog_tips_layer(void *callback1, void* args1, void* callback2, void* args2);

void ui_extern_reset_logic_machine();

#endif
