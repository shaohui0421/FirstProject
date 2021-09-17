#include <list>
#include "wifi_interactive.h"
#include "application.h"

ui_wifiinfo* WifiInteractive::wifi_status_query_result(void)
{
    net::WifiInfo wifi_info;
    int result;
    Application* app;

    Lock lock(_info_lock);
    
    app = Application::get_application();
    //TODO :判断wifi是否已经连接
    result = app->get_local_network_data().get_net_status();
    if(result != NET_STATUS_WLAN_UP) {
        LOG_WARNING("wifi don't connect");
        memset(&info2ui, 0, sizeof(ui_wifiinfo));
        return NULL;
    }

    wifi_info = app->get_local_network_data().get_connect_wifi_info();
    if((strlen(wifi_info.ssid.c_str()) == 0) && (strlen(wifi_info.ssid_mac.c_str()) == 0)) {
        LOG_WARNING("wifi don't connect");
        memset(&info2ui, 0, sizeof(ui_wifiinfo));
        return NULL;
    }

    memset(&info2ui, 0, sizeof(ui_wifiinfo));
    strcpy(info2ui.ssid, wifi_info.ssid.c_str());
    strcpy(info2ui.ssid_mac, wifi_info.ssid_mac.c_str());
    info2ui.freq = wifi_info.freq;
    info2ui.idensity = wifi_info.signal;
    info2ui.key_mgmt = wifi_info.key_mgmt;
    strcpy(info2ui.ip_info.ip_addr, wifi_info.ip_info.ip_addr.c_str());
    strcpy(info2ui.ip_info.mask, wifi_info.ip_info.mask.c_str());
    strcpy(info2ui.ip_info.gate, wifi_info.ip_info.gate.c_str());
    strcpy(info2ui.ip_info.dns, wifi_info.ip_info.dns.c_str());
    strcpy(info2ui.ip_info.dns_back, wifi_info.ip_info.dns_back.c_str());

    return &info2ui;
}

ui_scanresult* WifiInteractive::wifi_list_scan_result(int *length)
{
    int index;
    std::list<net::ScanResult> scan_list;
    std::list<net::ScanResult> scan_list_uq;
    std::list<net::ScanResult>::iterator it;
    std::list<net::ScanResult>::iterator it_uq;
    Application* app;

    Lock lock(_list_lock);

    app = Application::get_application();
    memset(list2ui, 0, sizeof(net::ScanResult)*WIFI_LIST_MAX);

    scan_list = app->get_local_network_data().get_scan_result();
    if (scan_list.empty())
    {
        *length = 0;
        LOG_WARNING("wifi list is null");

        return NULL;
    }

    // ignore connected wifi
    LOG_INFO("connected wifi: %s", info2ui.ssid);
    for (it = scan_list.begin(); it != scan_list.end();) {
        
        if (strcmp(info2ui.ssid, "") != 0 && strcmp(info2ui.ssid, it->ssid.c_str()) == 0) {
            scan_list.erase(it++);
        } else {
            it++;
        }
    }
    
    // copy scan list to list2ui
    for(it_uq = scan_list.begin(), index = 0; it_uq != scan_list.end(); ++it_uq, ++index) {
        // ignore elements if index exceed WIFI_LIST_MAX
        if (index >= WIFI_LIST_MAX) {
            LOG_WARNING("wifi list upper limit %d", WIFI_LIST_MAX);
            break;
        }
        strcpy(list2ui[index].ssid, it_uq->ssid.c_str());
        strcpy(list2ui[index].ssid_mac, it_uq->ssid_mac.c_str());
        strcpy(list2ui[index].flag, it_uq->flag.c_str());
        list2ui[index].freq = it_uq->freq;
        list2ui[index].idensity = it_uq->idensity;
        list2ui[index].key_mgmt = it_uq->key_mgmt;
    }

    LOG_INFO("wifi list get success, length is %d", index);
    *length = index;

    return list2ui;
}

void WifiInteractive::wifi_connect_status_refresh(int type)
{
    ui_wifiinfo *wifi_info = NULL;
    ui_scanresult *wifi_list = NULL;
    int list_length;

    switch (type) {
    case 1:
        wifi_info = wifi_status_query_result();
        wifi_list = wifi_list_scan_result(&list_length);
        l2u_show_wifi_status(ENABLE_WIFI, wifi_info, wifi_list, list_length);
        break;
    case 2:
        l2u_show_wifi_status(DISABLE_WIFI, NULL, NULL, -1);
        break;
    case 3:
        wifi_info = wifi_status_query_result();
        wifi_list = wifi_list_scan_result(&list_length);
        l2u_show_wifi_status(REFLASH_WIFI, wifi_info, wifi_list, list_length);
        break;
    default:
        break;
    }

    return;
}

int WifiInteractive::wireless_netcard_query_result(void)
{
    Application* app;
    app = Application::get_application();
    //TODO : wireless netcard query result
    int result = app->get_local_network_data().get_netcard_enable(net::NetworkWireless);
    LOG_INFO("wireless netcard query result :%d", result);

    return result;
}

void WifiInteractive::wifi_connect_button_handle(const int hide_net, const ui_scanresult *data)
{
    Application* app;
    app = Application::get_application();

    memset(&autconfig, 0, sizeof(ui_wificonfig));

    if(hide_net != FALSE) {
        l2u_show_wifi_pwd_win(UI_DIALOG_WIFIPWD_HIDE_NET, NULL);
        LOG_INFO("wifi hide network do connect");
        return;
    }

    if(data == NULL) {
        LOG_WARNING("wifi_connect_button_handle: data is null");
        return;
    }

    strcpy(autconfig.ssid, data->ssid);
    strcpy(autconfig.ssid_mac, data->ssid_mac);
    autconfig.key_mgmt = data->key_mgmt;
    autconfig.scan_ssid = FALSE;

    //TODO: Determine whether it is stored NET
    int net_id = app->get_local_network_data().check_wifi_saved(autconfig.ssid, autconfig.ssid_mac, autconfig.key_mgmt);

    if(net_id >= 0) {
        l2u_show_dialog_connect_network(UI_TIPS_CONNECTING);
        //TODO Interface:connect 1
        //autconfig.manual = TRUE;
        bool result = app->get_local_network_data().connect_wifi(net_id);

        if(result) {
            autconfig.manual = TRUE;
            autconfig.nokey = TRUE;
            LOG_INFO("wifi is stored network: connect1");
        } else {
            LOG_WARNING("wifi is stored network: connect1 failed");
            l2u_show_dialog_connect_network(UI_TIPS_AUTHENTICE_FAIL);
        }

        return;
    }

    LOG_INFO("wifi connect key-mgmt: %d", autconfig.key_mgmt);
    if(autconfig.key_mgmt == 0) {
        l2u_show_dialog_connect_network(UI_TIPS_CONNECTING);
        //TODO Interface:connect 2
        //autconfig.manual = TRUE;
        autconfig.saved = TRUE;
        net::WifiConfig wificonfig = convert_config_info(&autconfig);
        bool result = app->get_local_network_data().connect_wifi(wificonfig);

        if(result) {
            autconfig.manual = TRUE;
            LOG_INFO("wifi connect type: open");
        } else {
            LOG_WARNING("wifi connect type: open failed");
            if (wifi_white_list_is_empty()) {
           		l2u_show_dialog_connect_network(UI_TIPS_AUTHENTICE_FAIL);
        	}
        }

    } else if(autconfig.key_mgmt == 1) {
        l2u_show_wifi_pwd_win(UI_DIALOG_WIFIPWD_WEP, autconfig.ssid);
    } else if(autconfig.key_mgmt == 2) {
        l2u_show_wifi_pwd_win(UI_DIALOG_WIFIPWD_PSK, autconfig.ssid);
    } else if(autconfig.key_mgmt == 3) {
        l2u_show_wifi_pwd_win(UI_DIALOG_WIFIPWD_EAP, autconfig.ssid);
    } else {
        //TODO : show tip cannot support this type
        l2u_show_dialog_connect_network(UI_TIPS_INVALID_TYPE);
        LOG_WARNING("key mgmt is error number");
    }

    return;
}

void WifiInteractive::wifi_enter_button_handle(const int type, const upload_config_s config)
{
    Application* app;
    app = Application::get_application();

    LOG_INFO("wifi enter type :%d", type);
    switch (type) {
    case UI_DIALOG_WIFIPWD_WEP:
        strcpy(autconfig.wep_key0, config.passwd);
        autconfig.saved = config.save;
        break;
    case UI_DIALOG_WIFIPWD_PSK:
        strcpy(autconfig.psk, config.passwd);
        autconfig.saved = config.save;
        break;
    case UI_DIALOG_WIFIPWD_EAP:
        strcpy(autconfig.password, config.passwd);
        strcpy(autconfig.identity, config.usrname);
        autconfig.saved = config.save;
        break;
    case UI_DIALOG_WIFIPWD_HIDE_NET:
        memset(&autconfig, 0, sizeof(ui_wificonfig));
        autconfig.scan_ssid = TRUE;
        strcpy(autconfig.ssid, config.ssid);
        autconfig.key_mgmt = config.key_mgmt;
        autconfig.saved = FALSE;
        LOG_INFO("hide net enter key-mgmt :%d", autconfig.key_mgmt);

        if(autconfig.key_mgmt == 0) {
            LOG_INFO("wifi connect type :open");
        } else if(autconfig.key_mgmt == 1) {
            strcpy(autconfig.wep_key0, config.passwd);
        } else if(autconfig.key_mgmt == 2) {
            strcpy(autconfig.psk, config.passwd);
        } else if(autconfig.key_mgmt == 3) {
            strcpy(autconfig.identity, config.usrname);
            strcpy(autconfig.password, config.passwd);
        } else {
            LOG_WARNING("key mgmt is error number");
        }
        break;
    default:
        break;
    }

    //print_config_info(&autconfig);
    l2u_show_dialog_connect_network(UI_TIPS_CONNECTING);
    //TODO : connect 2
    //autconfig.manual = TRUE;
    net::WifiConfig wificonfig = convert_config_info(&autconfig);
    bool result = app->get_local_network_data().connect_wifi(wificonfig);

    if(result) {
        autconfig.manual = TRUE;
        LOG_INFO("wifi_enter_button_handle: connect2");
    } else {
        LOG_WARNING("wifi_enter_button_handle: connect2 faild");
        if (wifi_white_list_is_empty()) {
           /* whitelist exist then not show this tip dialog */
           l2u_show_dialog_connect_network(UI_TIPS_AUTHENTICE_FAIL);
        }       
    }

    return;
}

bool WifiInteractive::wifi_white_list_is_empty(void)
{
    Application* app;
    int ret = 0;
    vector <string>  whitelist;
    app = Application::get_application();

    ret = app->get_UsrUserInfoMgr()->readssidWhiteList(whitelist);
    if (ret == 0) {
        if (whitelist.size() == 0) {
            return true;
        } else {
            return false;
        }
    }
    return true;
}

void WifiInteractive::wifi_cancel_button_handle(void)
{
    memset(&autconfig, 0, sizeof(ui_wificonfig));
    return;
}

void WifiInteractive::wifi_cancel_show_wifipwd(void)
{
    autconfig.manual = FALSE;
    autconfig.nokey = FALSE;
}

void WifiInteractive::wifi_disconnect_button_handle(void)
{
    Application* app;
    app = Application::get_application();
    LOG_INFO("wifi_disconnect_button_handle");
    //TODO Interface:disconnect wifi
    bool result = app->get_local_network_data().disconnect_wifi();

    if(result) {
        LOG_INFO("disconnect wifi success");
    } else {
        LOG_WARNING("disconnect wifi faild");
    }

    return;
}

void WifiInteractive::wifi_enable_button_handle(const int mode)
{
    Application* app;
    app = Application::get_application();

    if(!mode) {
        //TODO Interface:disable wireless card
        bool result = app->get_local_network_data().set_netcard_enable(net::NetworkWireless, FALSE);
        if(result) {
            LOG_INFO("disable wireless card success");
            wifi_connect_status_refresh(DISABLE_WIFI);
        } else {
            LOG_WARNING("disable wireless card faild");
        }

        return;
    }

    //TODO Interface:enable wireless card
    bool result = app->get_local_network_data().set_netcard_enable(net::NetworkWireless, TRUE);
    if(result) {
        LOG_INFO("enable wireless card success");
        wifi_connect_status_refresh(ENABLE_WIFI);
    } else {
        LOG_WARNING("enable wireless card faild");
    }

    return;
}

int WifiInteractive::wifi_forget_net_handle(const int net_id)
{
    Application* app;
    app = Application::get_application();

    //TODO : forget wifi network
    bool result = app->get_local_network_data().forget_wifi(net_id);

    if(result) {
        LOG_INFO("forget wifi net success");
    } else {
        LOG_WARNING("forget wifi net faild");
    }

    return TRUE;
}

int WifiInteractive::wifi_saved_net_query_result(const char *ssid, const char *ssid_mac, const int proto)
{
    Application* app;
    app = Application::get_application();
    int net_id;

    //TODO : wifi saved net query result
    net_id = app->get_local_network_data().check_wifi_saved(ssid, ssid_mac, proto);
    LOG_INFO("wifi_saved_net_query_result: %d", net_id);

    return net_id;
}

/**
*function: get history ssid list from terminal lib
*@his_list: history ssid lsit save
*
* return: -1 error, 0 ok
*/
int WifiInteractive::get_ssid_his_ssid_list(list<net::WifiConfig>& his_list)
{
    Application* app;
    int ret = 0;
    list<net::WifiConfig>::iterator it;
    
    app = Application::get_application();
    ret = app->get_local_network_data().get_ssid_his_list(his_list);
    for (it = his_list.begin(); it != his_list.end(); it++) {
         LOG_DEBUG("his ssid:%s\n", it->ssid.c_str());
    }
    
    LOG_INFO("get_ssid_his_ssid_list: %d", his_list.size());
    return ret;
}

/**
*function: set white ssid to terminal lib
*@ssid_list: whitelist data
*
* return: -1 error, 0 ok
*/
int WifiInteractive::set_white_ssid_list(std::vector<string> &ssid_list)
{
    Application* app;
    int ret = 0;
    int i = 0;
    int ssid_size = 0;
    char keyjson[128] = {0};
    cJSON * json = NULL;
    cJSON * json_out = NULL;
    string in_json;
    string out_json;
    int handle = 0;
    
    app = Application::get_application();
    json = cJSON_CreateObject();
    if (json == NULL) {
        LOG_ERR("set white ssid list create json error");
        return -1;
    }
    ssid_size = ssid_list.size();
    cJSON_AddNumberToObject(json, "handle", net::NET_HANDLE_CLT_REQ_WHITE_LIST_SETTING);
    cJSON_AddNumberToObject(json, "num", ssid_size);

    for (i = 0; i < ssid_size; i++) {
        memset(keyjson, 0, sizeof(keyjson));
        sprintf(keyjson, "ssid%d", i);
        cJSON_AddStringToObject(json, keyjson, ssid_list[i].c_str());
    }
   
    in_json = cJSON_PrintUnformatted(json);
    LOG_INFO("in_json %s", in_json.c_str()); 
    app->get_local_network_data().set_white_ssid_list(in_json, out_json);
    cJSON_Delete(json);

    json = cJSON_Parse(out_json.c_str());
    if (json == NULL) {   
        LOG_INFO("set_white_ssid_list RETURN EEROR", out_json.c_str());
        return -1;
    }
    
    json_out = cJSON_GetObjectItem (json, "handle");
	if (json_out == NULL) {
		return -1;
	}
    
    LOG_INFO("sout_json %s", out_json.c_str()); 
    handle = json_out->valueint;
    if (handle != net::NET_HANDLE_CLT_SET_WHITE_LIST) {
        LOG_INFO("set white ssid list handle %d", handle);
    }
    
    json_out = cJSON_GetObjectItem (json, "ret");
	if (json_out == NULL) {
		return -1;
	}
    ret = json_out->valueint;
    cJSON_Delete(json);

    return ret;
}

int WifiInteractive::wifi_forget_not_whitessid_list(std::vector<string>& whitelist)
{
    list<net::WifiConfig> his_ssidlist;
    int ret = 0;
    
    ret = set_white_ssid_list(whitelist);
    if (ret < 0) {
        LOG_DEBUG("set_white_ssid_list error");
    } else {
        LOG_DEBUG("set_white_ssid_list whitelist size %d", whitelist.size());
        if (whitelist.size() > 0) {
            ret = get_ssid_his_ssid_list(his_ssidlist);
            if (ret < 0) {
                LOG_DEBUG("get_ssid_his_ssid_list error");
            } else {
                wifi_forget_not_whitessid_list(his_ssidlist, whitelist);
            }
        }
    }
    return ret;
}

int WifiInteractive::wifi_forget_not_whitessid_list(list<net::WifiConfig>& his_ssidlist, std::vector<string>& whitelist)
{
    Application* app;
    int ret = 0;
    bool is_white = false;
    vector<string>::iterator iter;
    list<net::WifiConfig>::iterator his_iter;

    app = Application::get_application();

    if (whitelist.size() == 0) {
        return 0;
    }
    
    for (his_iter = his_ssidlist.begin();  his_iter != his_ssidlist.end(); his_iter++) {
        for (iter = whitelist.begin(); iter != whitelist.end(); iter++) {
            if ((his_iter->ssid) == *iter) {
                is_white = true;
                break;
            }
        }

        if (is_white == false) {
            ret = app->get_local_network_data().forget_wifi(his_iter->net_id);
            LOG_DEBUG("forget not_white ssidwifi: ssid = %s net_id = %d, ret = %d", (his_iter->ssid).c_str(), his_iter->net_id, ret);
        } else {
            is_white = false;
        }
    }
    return ret;
}

void WifiInteractive::wifi_authenticate_result_handle(const int is_success, void *info)
{
    //Application* app;
    //app = Application::get_application();

    //int net_id;

    if(is_success != FALSE) {
        LOG_INFO("wireless authentication success");
        l2u_hide_wifi_pwd_win();
        memset(&autconfig, 0, sizeof(ui_wificonfig));

        return;
    }

    LOG_INFO("wifi authentication failed");
    string str((const char* )info);
    string wrongkey = "true";
    string is_wrongkey;
    string::size_type pos = str.find("isWrongKey=");

    if(pos != string::npos) {
        is_wrongkey = str.substr(pos+strlen("isWrongKey="), strlen("true"));
        LOG_INFO("is_wrongkey: %s", is_wrongkey.c_str());

        if(is_wrongkey.compare(wrongkey)) {
            LOG_INFO("wifi connect failed");
            l2u_show_dialog_connect_network(UI_TIPS_CONNECT_FAIL);

            return;
        }
    }

    l2u_show_dialog_connect_network(UI_TIPS_AUTHENTICE_FAIL);

    if(autconfig.manual) {
        //TODO :determine whether it is stored net which connecting faild
        autconfig.manual = FALSE;
        LOG_INFO("wifi authenticate result handle: manual");
        //net_id = app->get_local_network_data().check_wifi_saved(autconfig.ssid, autconfig.ssid_mac, autconfig.key_mgmt);
        //if((net_id >= 0) && (autconfig.scan_ssid != TRUE)) {

        if(autconfig.nokey) {
            autconfig.nokey = FALSE;
            sleep(2);
            LOG_INFO("wifi re-authenticate num: %d", autconfig.key_mgmt);
            autconfig.saved = TRUE;

            if(autconfig.key_mgmt == 0) {
                LOG_INFO("wifi re-authenticate type: open");
            } else if(autconfig.key_mgmt == 1) {
                l2u_show_wifi_pwd_win(UI_DIALOG_WIFIPWD_WEP, autconfig.ssid);
            } else if(autconfig.key_mgmt == 2) {
                l2u_show_wifi_pwd_win(UI_DIALOG_WIFIPWD_PSK, autconfig.ssid);
            } else if(autconfig.key_mgmt == 3) {
                l2u_show_wifi_pwd_win(UI_DIALOG_WIFIPWD_EAP, autconfig.ssid);
            } else {
                LOG_INFO("wifi re-authenticate type: error");
            }
        }
    }

    return;
}

void WifiInteractive::wifi_disconnect_result_handle(void *info)
{
    string str((const char* )info);
    string::size_type pos = str.find("web认证方式");

    if(pos != string::npos) {
        LOG_INFO("wifi_disconnect_result_handle: cannot support web authentication");
        l2u_show_dialog_connect_network(UI_TIPS_NOT_SUPPORT_WEB_HOTSPOT);
    }
    
    pos = str.find("无法连接非白名单内的WIFI");
    if(pos != string::npos) {
        LOG_INFO("wifi_disconnect_result_handle: cannot connect not white ssid");
        l2u_show_dialog_connect_network(UI_TIPS_CONNECT_NOT_WHITESSID);
    }
    return;
}

void WifiInteractive::wifi_scan_result_handle(void)
{
    Application* app;
    app = Application::get_application();

    bool ret = app->get_local_network_data().get_netcard_enable(net::NetworkWireless);
    if (!ret) {
        wifi_connect_status_refresh(DISABLE_WIFI);
    } else {
        wifi_connect_status_refresh(ENABLE_WIFI);
    }

    //TODO :判断wifi是否已经连接
    int result = app->get_local_network_data().get_net_status();
    if(result == NET_STATUS_WLAN_UP) {
        l2u_show_wifi_btnbox(UI_WIN_WIFI_BTNBOX_REFLASH, UI_BTNBOX_WIRELESS_CONNECT, info2ui.idensity);
    }

    return;
}

void WifiInteractive::net_status_change_handle(const int status)
{
    Application* app;
    app = Application::get_application();
    bool netcard_enable;
    int net_status;

    LOG_INFO("ENTER net_status_change_handle: status = %d", status);

    if (status == net::ETH_DOWN) {

        LOG_INFO("eth connect status: DOWN");
        //TODO :WiFi connect status whether is down
        net_status = app->get_local_network_data().get_net_status();
        LOG_INFO("net_status_change_handle: get_wifi_status: %d", net_status);
        if(net_status != NET_STATUS_WLAN_UP) {
            l2u_show_net_status(CHANGE_NETSTAT, NET_STATUS_LINK_DOWN);
            l2u_show_wifi_btnbox(UI_WIN_WIFI_BTNBOX_REFLASH, UI_BTNBOX_WIRE_DISCONNECT, -1);
        }

        l2u_show_net_auth_disconnect(UI_TIPS_NET_AUTH_DISCONNECT);
        //TODO:hide auth_win_btnbox;

    } else if (status == net::ETH_UP) {

        LOG_INFO("eth connect status: UP");
        if(is_wifi_terminal()) {
            LOG_INFO("is_wifi_termina");
            //TODO : wireless netcard query result
            netcard_enable = app->get_local_network_data().get_netcard_enable(net::NetworkWireless);
            LOG_INFO("net_status_change_handle: get_netcard_enable: %d", netcard_enable);
        
            if (!netcard_enable) {
                wifi_connect_status_refresh(DISABLE_WIFI);
            }
        }        
        l2u_show_net_status(CHANGE_NETSTAT, NET_STATUS_ETH_UP);
        l2u_show_wifi_btnbox(UI_WIN_WIFI_BTNBOX_REFLASH, UI_BTNBOX_WIRE_CONNECT, -1);

    } else if (status == net::WLAN_DOWN) {

        LOG_INFO("wifi connect status: DOWN");
        if(is_wifi_terminal()) {
            LOG_INFO("is_wifi_termina");
            //TODO : wireless netcard query result
            netcard_enable = app->get_local_network_data().get_netcard_enable(net::NetworkWireless);
            LOG_INFO("net_status_change_handle: get_netcard_enable: %d", netcard_enable);
        
            if (!netcard_enable) {
                wifi_connect_status_refresh(DISABLE_WIFI);
            } else {
                wifi_connect_status_refresh(ENABLE_WIFI);
            }
        }
        //TODO :ETH connect status whether is down
        net_status = app->get_local_network_data().get_net_status();
        LOG_INFO("net_status_change_handle: get_net_status: %d", net_status);
        if (net_status != NET_STATUS_ETH_UP) {
            l2u_show_net_status(CHANGE_NETSTAT, NET_STATUS_LINK_DOWN);
            l2u_show_wifi_btnbox(UI_WIN_WIFI_BTNBOX_REFLASH, UI_BTNBOX_WIRE_DISCONNECT, -1);
        }

    } else if (status == net::WLAN_UP) {

        LOG_INFO("wifi connect status: UP");
        l2u_show_dialog_connect_network(UI_TIPS_CONNECT_SUCCESS);
        l2u_show_net_status(CHANGE_NETSTAT, NET_STATUS_WLAN_UP);
        wifi_connect_status_refresh(REFLASH_WIFI);
        l2u_show_wifi_btnbox(UI_WIN_WIFI_BTNBOX_REFLASH, UI_BTNBOX_WIRELESS_CONNECT, info2ui.idensity);
        l2u_show_net_auth_disconnect(UI_TIPS_NET_AUTH_DISCONNECT);

    } else if(status == net::NET_UNAVAILABLE) {

        LOG_INFO("net connect status: UNAVAILABLE");
        if(is_wifi_terminal()) {
            LOG_INFO("is_wifi_termina");
            //TODO : wireless netcard query result
            netcard_enable = app->get_local_network_data().get_netcard_enable(net::NetworkWireless);
            LOG_INFO("net_status_change_handle: get_netcard_enable: %d", netcard_enable);
        
            if (!netcard_enable) {
                wifi_connect_status_refresh(DISABLE_WIFI);
            } else {
                wifi_connect_status_refresh(ENABLE_WIFI);
            }
        }
        l2u_show_net_status(CHANGE_NETSTAT, NET_STATUS_LINK_DOWN);
        l2u_show_wifi_btnbox(UI_WIN_WIFI_BTNBOX_REFLASH, UI_BTNBOX_WIRE_DISCONNECT, -1);

    }

    return;
}

net::WifiConfig WifiInteractive::convert_config_info(ui_wificonfig *config)
{
    net::WifiConfig wificonfig;

    if(config == NULL) {
        LOG_WARNING("convert_config_info: config is null");
        return wificonfig;
    }

    wificonfig.ssid = config->ssid;
    wificonfig.ssid_mac = config->ssid_mac;
    wificonfig.net_id = config->net_id;
    wificonfig.priority = config->priority;
    wificonfig.key_mgmt = config->key_mgmt;
    wificonfig.scan_id = config->scan_ssid;
    wificonfig.psk = config->psk;
    wificonfig.wep_key0 = config->wep_key0;
    wificonfig.eap = config->eap;
    wificonfig.identity = config->identity;
    wificonfig.password = config->password;
    wificonfig.saved = config->saved;
    wificonfig.flags = config->flags;

#if 0
    LOG_INFO("------------------WIFI CONFIG C++----------------------");
    LOG_INFO("<c++> ssid: %s", wificonfig.ssid.c_str());
    LOG_INFO("<c++> ssid mac: %s", wificonfig.ssid_mac.c_str());
    LOG_INFO("<c++> net id: %d", wificonfig.net_id);
    LOG_INFO("<c++> priority: %d", wificonfig.priority);
    LOG_INFO("<c++> key_mgmt: %d", wificonfig.key_mgmt);
    LOG_INFO("<c++> scan_ssid: %d", wificonfig.scan_id);
    LOG_INFO("<c++> psk: %s", gloox::password_codec_xor(wificonfig.psk, true));
    LOG_INFO("<c++> wep_key0: %s", gloox::password_codec_xor(wificonfig.wep_key0, true));
    LOG_INFO("<c++> eap: %d", wificonfig.eap);
    LOG_INFO("<c++> identity: %s", wificonfig.identity.c_str());
    LOG_INFO("<c++> password: %s", gloox::password_codec_xor(wificonfig.password.c_str(), true));
    LOG_INFO("<c++> saved: %d", wificonfig.saved);
    LOG_INFO("<c++> flags: %d", wificonfig.flags);
    LOG_INFO("-------------------------------------------------------");
#endif

    return wificonfig;
}

void WifiInteractive::print_config_info(ui_wificonfig *config)
{
    if(config == NULL) {
        LOG_WARNING("print_config_info: config is null");
        return;
    }

    LOG_INFO("----------------------WIFI CONFIG----------------------");
    LOG_INFO("ssid: %s", config->ssid);
    LOG_INFO("ssid_mac: %s", config->ssid_mac);
    LOG_INFO("net id: %d", config->net_id);
    LOG_INFO("priority: %d", config->priority);
    LOG_INFO("key_mgmt: %d", config->key_mgmt);
    LOG_INFO("scan_ssid: %d", config->scan_ssid);
    LOG_INFO("pairwise: %d", config->pairwise);
    LOG_INFO("psk: %s", gloox::password_codec_xor(config->psk, true).c_str());
    LOG_INFO("wep_key0: %s", gloox::password_codec_xor(config->wep_key0, true).c_str());
    LOG_INFO("eap: %d", config->eap);
    LOG_INFO("identity: %s", config->identity);
    LOG_INFO("password: %s", gloox::password_codec_xor(config->password, true).c_str());
    LOG_INFO("saved: %d", config->saved);
    LOG_INFO("flags: %d", config->flags);
    LOG_INFO("-------------------------------------------------------");

    return;
}


