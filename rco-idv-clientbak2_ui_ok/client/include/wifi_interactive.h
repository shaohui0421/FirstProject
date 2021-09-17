#ifndef _WIFIINTERACTIVE_H
#define _WIFIINTERACTIVE_H
#include <string>
#include <vector>
#include "common.h"
#include "ui_api.h"
#include "local_network_data.h"
#include "ui_dialog_wifipwd.h"
#include "ui_win_wifi_btnbox.h"
#include "thread.h"

#define WIFI_LIST_MAX 50

typedef enum {
    ENABLE_WIFI = 1,
    DISABLE_WIFI,
    REFLASH_WIFI,
    CHANGE_NETSTAT,
} dialog_show_ctl_enum;

class WifiInteractive
{
public:

    WifiInteractive(){}
    virtual ~WifiInteractive(){}

    void net_status_change_handle(const int status);
    void wifi_scan_result_handle(void);
    void wifi_authenticate_result_handle(const int is_success, void *info);
    void wifi_disconnect_result_handle(void *info);
    void wifi_enable_button_handle(const int mode);
    void wifi_disconnect_button_handle(void);
    void wifi_enter_button_handle(const int type, const upload_config_s config);
    void wifi_cancel_button_handle(void);
    void wifi_cancel_show_wifipwd(void);
    void wifi_connect_button_handle(const int hide_net, const ui_scanresult *data);
    ui_scanresult* wifi_list_scan_result(int *length);
    ui_wifiinfo* wifi_status_query_result(void);
    int wireless_netcard_query_result(void);
    int wifi_forget_net_handle(const int net_id);
    int wifi_saved_net_query_result(const char *ssid, const char *ssid_mac, const int proto);
    int get_ssid_his_ssid_list(list<net::WifiConfig>& his_list);
    int set_white_ssid_list(std::vector<string> &ssid_list);
    int wifi_forget_not_whitessid_list(list<net::WifiConfig>& his_ssidlist, std::vector<string>& whitelist);
    int wifi_forget_not_whitessid_list(std::vector<string>& whitelist);
    bool wifi_white_list_is_empty(void);
private:

    Mutex _info_lock;
    Mutex _list_lock;
    ui_wifiinfo info2ui;
    ui_scanresult list2ui[WIFI_LIST_MAX];
    ui_wificonfig autconfig;

    void wifi_connect_status_refresh(int type);
    void print_config_info(ui_wificonfig *config);
    net::WifiConfig convert_config_info(ui_wificonfig *config);

};
#endif