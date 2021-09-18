#include "ui_extern.h"
#include "liblog.h"
#include "ui_dialog_config_scrnmanage.h"
#include <stdio.h>
// add extern module headers
#ifdef IDV_CLIENT
#include "application_c_interfaces.h"
#endif

#define PWD     "111"

void ui_extern_enter_settings()
{
#ifdef IDV_CLIENT
	ui_enter_settings();
    return;
#else
    return;
#endif
}

void ui_extern_giveup_connect()
{
#ifdef IDV_CLIENT
    ui_destroy_connection();
    return;
#else
    return;
#endif
}

void ui_extern_reconnect()
{
#ifdef IDV_CLIENT
    ui_establish_connection();
    return;
#else
    return;
#endif
}

void ui_extern_local_mode()
{
#ifdef IDV_CLIENT
    ui_enter_local_mode();
    return;
#else
    return;
#endif
}

int ui_extern_verify_adminpwd(char *pwd)
{
#ifdef IDV_CLIENT
    return ui_check_admin_passwd(pwd);
#else
    return strcmp(pwd,PWD);
#endif
}

void ui_extern_user_login(char *username, char *pwd, int active)
{
#ifdef IDV_CLIENT
    ui_account_login(username, pwd, active);
    return;
#else
    return;
#endif
}

void ui_extern_back2_netmode(int value)
{
#ifdef IDV_CLIENT
	ui_localmode_quit(value);
    return;
#else
    return;
#endif
}

void ui_extern_bind_usr(int result)
{
#ifdef IDV_CLIENT
    ui_bind_user(result);
    logi("ui_extern_bind_usr %d\n", result);
    return;
#else
    return;
#endif

}


void ui_extern_shutdown(int i)
{
#ifdef IDV_CLIENT
    if (i == 0) {
        ui_shutdown_terminal();
    } else {
        ui_reboot_terminal();
    }
    return;
#else
    logi("ui_extern_shutdown:%d\n", i);
    return;
#endif

}

void ui_extern_public_login()
{
#ifdef IDV_CLIENT
    ui_public_login();
    return;
#else
    logi("ui_extern_public_login\n");
    return;
#endif
}

void ui_extern_cancal_setting()
{
#ifdef IDV_CLIENT
    ui_cancel_settings();
    logi("ui_extern_cancal_setting\n");
    return;
#else
    logi("ui_extern_cancal_setting\n");
    return;
#endif
}

void ui_extern_cancal_auto_shutdown()
{
#ifdef IDV_CLIENT
    ui_cancel_auto_shutdown();
    logi("ui_cancal_auto_shutdown\n");
    return;
#else
    logi("ui_cancal_auto_shutdown\n");
    return;
#endif
}

void ui_extern_guest_login()
{
#ifdef IDV_CLIENT
    ui_guest_login();
    return;
#else
    logi("ui_extern_guest_login\n");
    return;
#endif
}


void ui_extern_get_server_ip(char *ip)
{
#ifdef IDV_CLIENT
    ui_get_server_ip(ip);
    return;
#else
    strcpy(ip, "1.1.1.1");
    return;
#endif
}

void ui_extern_get_hostname(char *name)
{
#ifdef IDV_CLIENT
    ui_get_host_name(name);
    return;
#else
    strcpy(name, "xxx");
    return;
#endif
}


void ui_extern_get_terminal_mode(int *mode, int *readonly)
{
#ifdef IDV_CLIENT
    ui_get_terminal_mode(mode, readonly);
    return;
#else
    *mode = 0;
    *readonly = 1;
    return;
#endif
}


void ui_extern_download_retry()
{
#ifdef IDV_CLIENT
    ui_show_retry_downloading();
    return;
#else
    return;
#endif
}


void ui_extern_tips_confirm(int i)
{
#ifdef IDV_CLIENT
    ui_tips_ok(i);
    return;
#else
    logi("ui_extern_tips_confirm : %d\n", i);
    return;
#endif
}

void ui_extern_enter_local_mode(void)
{
#ifdef IDV_CLIENT
	ui_enter_local_mode();
    return;
#else
    logi("ui_extern_enter_local_mode : %d\n");
    return;
#endif
}

void ui_extern_get_last_logined_user(char *username, int user_len, char *password, int passwd_len, int *remember_flag)
{
#ifdef IDV_CLIENT
    ui_get_last_logined_user(username, user_len, password, passwd_len, remember_flag);
    return;
#else
    strcpy(username, "xxx");
    strcpy(password, "xxx");
    *remember_flag = 0;
    return;
#endif

}


void ui_extern_modify_passwd(const char* username, const char* password, const char* new_passwd)
{
#ifdef IDV_CLIENT
    ui_modify_passwd(username, password, new_passwd);
    return;
#else
    logi("user:%s, pwd:%s, new_pwd:%s\n", username, password, new_passwd);
    return;
#endif

}

void ui_extern_new_deploy(int mode, const char* bind_user)
{
#ifdef IDV_CLIENT
    ui_new_deploy(mode, bind_user);
    return;
#else
    logi("mode:%d, bind_user:%s\n", mode, bind_user);
    return;
#endif

}

void ui_extern_get_bt_service(int *bt_service)
{
#ifdef IDV_CLIENT
    ui_get_bt_service(bt_service);
    return;
#else
    *bt_service = 0;
    return;
#endif

}

void ui_extern_get_power_boot(int *power_boot)
{
#ifdef IDV_CLIENT
    ui_get_power_boot(power_boot);
    return;
#else
    *power_boot = 0;
    return;
#endif

}

//void ui_extern_get_boot_speedup(int *boot_speedup)
//{
//#ifdef IDV_CLIENT
//	ui_get_boot_speedup(boot_speedup);
//	return;
//#else
//	*boot_speedup = 0;
//	return;
//#endif
//
//}


void ui_extern_is_new_deploy(int *is_new_deploy)
{
#ifdef IDV_CLIENT
    ui_is_new_deploy(is_new_deploy);
    return;
#else
    *is_new_deploy = 0;
    return;
#endif

}

void ui_extern_is_wifi_terminal(int *wifi_terminal)
{
#ifdef IDV_CLIENT
    ui_is_wifi_terminal(wifi_terminal);
    return;
#else
    *wifi_terminal = 0;
    return;
#endif

}

void ui_extern_enter_wifi_config()
{
#ifdef IDV_CLIENT
	ui_enter_wifi_config();
    return;
#else
    return;
#endif
}
void ui_extern_enter_auth_config(void)
{
#ifdef IDV_CLIENT
    ui_enter_auth_config();
    return;
#else
    return;
#endif
}

void ui_extern_wireless_netcard_enable(const int mode)
{
#ifdef IDV_CLIENT
    ui_wireless_netcard_enable(mode);
    return;
#else
    logi("mode:%d\n", mode);
    return;
#endif

}

void ui_extern_wireless_netcard_disconnect(void)
{
#ifdef IDV_CLIENT
    ui_wireless_netcard_disconnect();
    return;
#else
    logi("ui extern wireless netcard disconnect\n");
    return;
#endif

}

void ui_extern_wireless_netcard_connect(const int hide_net, const ui_scanresult *data)
{
#ifdef IDV_CLIENT
    ui_wireless_netcard_connect(hide_net, data);
    return;
#else
    logi("hide net: %d\n", hide_net);
    return;
#endif

}

void ui_extern_wifi_authenticate_enter(const int type, const upload_config_s config)
{
#ifdef IDV_CLIENT
    ui_wifi_authenticate_enter(type, config);
    return;
#else
    logi("type:%d\n", type);
    return;
#endif

}

ui_scanresult* ui_extern_wifi_list_scan_result(int *length)
{
#ifdef IDV_CLIENT
    return ui_wifi_list_scan_result(length);
#else
    logi("ui extern wifi list scan result\n");
    return NULL;
#endif

}

ui_wifiinfo* ui_extern_wifi_status_query_result(void)
{
#ifdef IDV_CLIENT
    return ui_wifi_status_query_result();
#else
    logi("ui extern wifi status query result\n");
    return NULL;
#endif

}

int ui_extern_wireless_netcard_query_result(void)
{
#ifdef IDV_CLIENT
    return ui_wireless_netcard_query_result();
#else
    logi("ui wireless netcard query result\n");
    return FALSE;
#endif

}

int ui_extern_wifi_forget_net_handle(const int net_id)
{
#ifdef IDV_CLIENT
    return ui_wifi_forget_net_handle(net_id);
#else
    logi("ui wifi forget net handle\n");
    return FALSE;
#endif

}

int ui_extern_wifi_saved_net_query_result(const char *ssid, const char *ssid_mac, const int proto)
{
#ifdef IDV_CLIENT
    return ui_wifi_saved_net_query_result(ssid, ssid_mac, proto);
#else
    logi("ui wifi saved net query result\n");
    return FALSE;
#endif

}

int ui_extern_net_status_query_result(void)
{
#ifdef IDV_CLIENT
    return ui_net_status_query_result();
#else
    logi("ui net status query result\n");
    return FALSE;
#endif

}

void ui_extern_cancel_wifi_config()
{
#ifdef IDV_CLIENT
    ui_cancel_wifi_config();
    logi("ui_extern_cancel_wifi_config\n");
    return;
#else
    logi("ui_extern_cancel_wifi_config\n");
    return;
#endif
}

void ui_extern_wifi_cancel_button_handle(void)
{
#ifdef IDV_CLIENT
    ui_wifi_cancel_button_handle();
#else
    logi("ui wifi cancel button handle\n");
    return FALSE;
#endif

}
void ui_extern_cancel_auth_config()
{
#ifdef IDV_CLIENT
    ui_cancel_auth_config();
    logi("ui_extern_cancel_wifi_config\n");
    return;
#else
    logi("ui_extern_cancel_wifi_config\n");
    return;
#endif
}


void ui_extern_cancel_show_wifipwd()
{
#ifdef IDV_CLIENT
    ui_wifi_cancel_show_wifipwd();
    logi("ui_extern_cancel_show_wifipwd\n");
    return;
#else
    logi("ui_extern_cancel_show_wifipwd\n");
    return;
#endif
}

void ui_extern_usb_copy_base()
{
#ifdef IDV_CLIENT
    ui_usb_copy_base();
#else
    logi("ui extern usb copy base\n");
    return FALSE;
#endif 
}

int ui_extern_getssid_whitelist(char (*ssid)[128], int max_list_num)
{
#ifdef IDV_CLIENT
    if (ssid == NULL) {
        return 0;
    }
    return ui_getssid_whitelist(&ssid[0], max_list_num);
#else
    logi("ui extern getssid whitelist\n");
    return 0;
#endif
}

int ui_extern_get_whitelist_num(void)
{
#ifdef IDV_CLIENT
    return ui_get_whitelist_num();
#else
    logi("ui_extern_get_whitelist_num\n");
    return 0;
#endif
}

int ui_extern_set_resolution(const int width, const int height)
{
#ifdef IDV_CLIENT
    return ui_set_resolution(width, height);
#else
    logi("ui_extern_set_resolution\n");
    return 0;
#endif
}

int ui_extern_get_resolution_list(ui_res_info *res_info, int max_list_size)
{
#ifdef IDV_CLIENT
    return ui_get_resolution_list(res_info, max_list_size);
#else
    logi("ui_extern_get_resolution_list\n");
    return 0;
#endif
}

int ui_extern_set_displayinfo(const ui_res_info *res_info)
{
#ifdef IDV_CLIENT
    return ui_set_display_info(res_info);
#else 
    logi("ui_extern_set_displayinfo\n");
    return 0;
#endif
}

/**
*function : judget if vmmode is emulation
*
*int : 0: start vmmode is not emulation mode 1: start vmmode is emulation
*/
int ui_extern_start_vmmode_is_emulation()
{
#ifdef IDV_CLIENT
    return ui_vm_start_is_emulaton_vmmode();
#else 
    logi("ui_extern_start_vmmode_is_emulation\n");
    return 0;
#endif
}

/**
*function : get ui_userinfomgr_permitted status
*
*@return: ui permitted status
*/
int ui_extern_userinfomgr_permitted()
{
#ifdef IDV_CLIENT
    int permit = ui_userinfomgr_permitted();
    logi("ui_extern_userinfomgr_permitted %d\n", permit);
    return permit;
#else 
    logi("ui_extern_userinfomgr_permitted\n");
    return 0;
#endif
}

void ui_extern_get_displayinfo(int* width, int* height, int * custom)
{
#ifdef IDV_CLIENT
    ui_get_display_info(width, height, custom);
#else 
    logi("ui_extern_get_displayinfo\n");
#endif
}

int ui_extern_is_display_info_ini_exist()
{
#ifdef IDV_CLIENT
    return ui_is_display_info_ini_exist();
#else 
    logi("ui_extern_is_display_info_ini_exist\n");
    return 0;
#endif
}

int ui_extern_delete_display_info_ini()
{
#ifdef IDV_CLIENT
    return ui_delete_display_info_ini();
#else
    logi("ui_extern_is_display_info_ini_exist\n");
    return 0;
#endif
}

int ui_extern_set_ext_displayinfo(const ui_res_info *res_info, int num)
{
#ifdef IDV_CLIENT
    return ui_set_ext_display_info(res_info, num);
#else 
    logi("ui_extern_set_displayinfo\n");
    return 0;
#endif
}

void ui_extern_get_ext_displayinfo(ui_ext_res_info *res_info)
{
#ifdef IDV_CLIENT
    ui_get_ext_display_info(res_info);
#else 
    logi("ui_extern_get_displayinfo\n");
#endif
}

int ui_extern_is_display_info_section_exist(const char *section)
{
#ifdef IDV_CLIENT
    return ui_is_display_info_section_exist(section);
#else 
    logi("ui_extern_is_resolution_info_section_exist\n");
    return 0;
#endif
}

int ui_extern_delete_resolution_info_section()
{
#ifdef IDV_CLIENT
    return ui_delete_resolution_info_section();
#else
    logi("ui_extern_delete_resolution_info_section\n");
    return 0;
#endif
}

void ui_extern_show_dialog_tips_dpi(void *callback1, void* args1, void* callback2, void* args2)
{
#ifdef IDV_CLIENT
    ui_show_dialog_tips_dpi(callback1, args1, callback2, args2);
#else 
    logi("ui_extern_show_dialog_tips_dpi\n");
#endif
}

void ui_extern_upgrade_for_class(void)
{
#ifdef IDV_CLIENT
    ui_upgrade_for_class();
#else 
    logi("ui_extern_upgrade_for_class\n");
#endif
}

int ui_extern_get_hdmi_audio()
{
#ifdef IDV_CLIENT
    return ui_get_hdmiaudio_info();
#else 
    logi("ui_extern_show_dialog_tips_dpi\n");
    return 0;
#endif
}

void ui_extern_set_hdmi_audio(int hdmiaudio)
{
#ifdef IDV_CLIENT
    ui_set_hdmiaudio_info(hdmiaudio);
#else 
    logi("ui_extern_show_dialog_tips_dpi\n");
#endif
}

int ui_extern_is_hdmi_connected()
{
#ifdef IDV_CLIENT
    return ui_is_hdmi_connected();
#else 
    logi("ui_extern_show_dialog_tips_dpi\n");
    return 0;
#endif
}

void ui_extern_get_auth_info(auth_info_t *info)
{
#ifdef IDV_CLIENT
    ui_get_auth_info(info);
#else
    logi("ui extern get auth_info");
#endif
}

int ui_extern_get_auth_status()
{
#ifdef IDV_CLIENT
    return ui_get_auth_status();
#else
    logi("ui_extern get_auth_type");
#endif
}
void ui_extern_save_auth_info(const auth_info_t *info)
{
#ifdef IDV_CLIENT
    ui_save_auth_info(info);
#else
    logi("ui extern save auth_info\n");
#endif
}
void ui_extern_save_auto_connect_info(int auto_connect)
{
#ifdef IDV_CLIENT
    ui_save_auto_connect_info(auto_connect);
#else
    logi("ui extern save auto connect");
#endif
}
void ui_extern_save_auth_setting(const auth_info_t *info)
{
#ifdef IDV_CLIENT
    ui_save_setting_auth(info);
#else
    logi("ui extern auth setting");
#endif
}

void ui_extern_get_current_dpi(char *width, char *height)
{
#ifdef IDV_CLIENT
    ui_get_current_dpi(width, height);
#else
    logi("ui extern get current dpi\n");
#endif
}
int ui_extern_get_wired1xauth_exist()
{
#ifdef IDV_CLIENT
        logi("ui_extern_cancel_wifi_config\n");
        return ui_get_wired1xauth_exist();
#else
        logi("ui_extern_cancel_wifi_config\n");
        return;
#endif
}

int ui_extern_delete_auth_info()
{
#ifdef IDV_CLIENT
    logi("ui_extern_delete_auth_info");
    ui_delete_auth_info();
    return 0;
#else
    logi("ui_extern_delete_auth_info");
    return 0;
#endif
}


int ui_extern_get_display_port_number()
{
    int ret = 0;
#ifdef IDV_CLIENT
    ret = ui_get_display_port_number();
#else
    logi("ui_extern_get_display_port_number\n");
#endif
    return ret;
}

int ui_extern_get_display_port_connected_number()
{
    int ret = 0;
#ifdef IDV_CLIENT
    ret = ui_get_display_port_connected_number();
#else
    logi("ui_extern_get_display_port_number\n");
#endif
    return ret;
}

int ui_extern_is_display_enable(int port)
{
    int ret = 0;
#ifdef IDV_CLIENT
    ret = ui_is_display_enable(port);
#else
    logi("ui_extern_is_display_enable\n");
#endif
    return ret;
}

int ui_extern_is_display_connected(int port)
{
    int ret = 0;
#ifdef IDV_CLIENT
    ret = ui_is_display_connected(port);
#else
    logi("ui_extern_is_display_enable\n");
#endif
    return ret;
}

int ui_extern_get_display_resolution_list(int port, ui_res_info *res_info, int max_list_size)
{
    int ret = 0;
#ifdef IDV_CLIENT
    ret = ui_get_display_resolution_list(port, res_info, max_list_size);
#else
    logi("ui_extern_get_display_resolution_list\n");
#endif
    return ret;
}

void ui_extern_save_boot_speedup(int boot_speedup)
{
#ifdef IDV_CLIENT
    ui_save_boot_speedup(boot_speedup);
#else
    logi("ui_extern_save_boot_speedup\n");
#endif
}


void ui_extern_save_e1000_netcard(int e1000_netcard)
{
#ifdef IDV_CLIENT
    ui_save_e1000_netcard(e1000_netcard);
#else
    logi("ui_extern_save_e1000_netcard\n");
#endif
}

int ui_extern_using_e1000_netcard()
{
#ifdef IDV_CLIENT
    return ui_using_e1000_netcard();
#else
    logi("ui_extern_using_e1000_netcard\n");
#endif
}

void ui_extern_save_usb_emulation(int usb_emulation)
{
#ifdef IDV_CLIENT
    ui_save_usb_emulation(usb_emulation);
#else
    logi("ui_extern_save_usb_emulation\n");
#endif
}

int ui_extern_is_usb_emulation()
{
#ifdef IDV_CLIENT
    return ui_is_usb_emulation();
#else
    logi("ui_extern_is_usb_emulation\n");
#endif
}

int ui_extern_is_using_app_layer()
{
#ifdef IDV_CLIENT
    return ui_is_using_app_layer();
#else
    logi("ui_extern_is_using_app_layer\n");
#endif
}

int ui_extern_is_using_power_boot()
{
#ifdef IDV_CLIENT
    return ui_using_powerboot();
#else
    logi("ui_extern_is_using_power_boot\n");
#endif
}

void ui_extern_set_app_layer_status(int status)
{
#ifdef IDV_CLIENT
    ui_set_app_layer_status(status);
#else
    logi("ui_extern_set_app_layer_status\n");
#endif
}

void ui_extern_show_dialog_tips_layer(void *callback1, void* args1, void* callback2, void* args2)
{
#ifdef IDV_CLIENT
    ui_show_dialog_tips_layer(callback1, args1, callback2, args2);
#else
    logi("ui_extern_show_dialog_tips_layer\n");
#endif
}

void ui_extern_reset_logic_machine()
{
#ifdef IDV_CLIENT
    ui_reset_logic_machine();
#else
    logi("ui_extern_reset_logic_machine");
#endif
}

