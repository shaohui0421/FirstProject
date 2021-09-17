#include "local_network_data.h"
#include "wifi_interactive.h"
#include "application.h"

//wifi unit test
#if 0
void*       net::WifiManager::mUser = NULL;
net::onWlanEvent net::WifiManager::mCb = NULL;
net::WifiInfo net::WifiManager::_wifi_info;
std::list<net::ScanResult> net::WifiManager::_scan_list;
#endif
//wifi unit test
struct rc_setnetifdevip_thread_t {
    int (*callback)(int ret, void *data);
    void *data;
    void *user;
};


LocalNetworkData::LocalNetworkData(Application* app, LocalNetworkDataCallback callback, void* callback_data)
    :_app (app)
    ,_callback (callback)
    ,_callback_data (callback_data)
    ,_dev(new rc_netifdev_t)
    ,_dev_net_status(NET_STATUS_UNKNOWN)
    ,_last_dev_net_status(NET_STATUS_UNKNOWN)
    ,_dev_net2_status(NET_STATUS_UNKNOWN)
    ,_last_dev_net2_status(NET_STATUS_UNKNOWN)
{
    _wifi_manager = NULL;
    _network_manager = NULL;
    
    if(app) {
        if(LocalNetworkData::netMsgCallback == NULL)
        {
            LOG_WARNING("LocalNetworkData::netMsgCallback == NULL");
        }
        _network_manager = new net::NetworkManager(LocalNetworkData::netMsgCallback, this);
        _network_manager->setOptionCb(net::NetworkWireless, Application::handle_vendor_encapsulated_options);
        _network_manager->setOptionCb(net::NetworkWired, Application::handle_vendor_encapsulated_options);
        if (is_wifi_terminal()) {
            _wifi_manager = new net::WifiManager(LocalNetworkData::wlanEventCallback, this);
        }
    }
    update();
    if(LocalNetworkData::netMsgCallback == NULL)
    {
        LOG_WARNING("LocalNetworkData::netMsgCallback == NULL");
    }
}


void LocalNetworkData::update()
{
    int ret = 0;
    /********
    FIXME: change the way to get dev.dev
    ********/
    strcpy(_dev->dev, RCC_DEFAULT_IFNAME);
    ret = rc_getnetifdevinfo(_dev);
    if(ret != 0)
    {
        LOG_ERR("rc_getnetifdevinfo error, ret = %d", ret);
    }
    _network_info.netcard_speed = _dev->link_speed;
    _network_info.dhcp          = (_dev->iptype==RC_IPTYPE_DHCP);
    _network_info.ip            = inet_ntoa(_dev->ip);
    _network_info.submask       = inet_ntoa(_dev->netmask);
    _network_info.gateway       = inet_ntoa(_dev->gateway);
    _network_info.auto_dns      = (_dev->dnstype==RC_DNSTYPE_DHCP);
    LOG_DEBUG("auto_dns=%d, dnstype=%d", _network_info.auto_dns, _dev->dnstype);
    /*
     *if (INADDR_NONE == _dev->dns.s_addr) || (INADDR_ANY == _dev->dns.s_addr)
     *we would get "0.0.0.0" or "255.255.255.255" which is invalid
     *now we clean dns section in _network_info to unify ui
     */
    if((INADDR_NONE == _dev->dns1.s_addr) || (INADDR_ANY == _dev->dns1.s_addr))
    {
        _network_info.main_dns.clear();
    }
    else
    {
        _network_info.main_dns = inet_ntoa(_dev->dns1);
    }

    if((INADDR_NONE == _dev->dns2.s_addr) || (INADDR_ANY == _dev->dns2.s_addr))
    {
        _network_info.back_dns.clear();
    }
    else
    {
        _network_info.back_dns = inet_ntoa(_dev->dns2);
    }
}

void LocalNetworkData::activate()
{
    int ret = 0;
    /********
    FIXME: change the way to get dev.dev
    ********/
    LOG_DEBUG("set_dns is start %s, %s\n", _network_info.main_dns.c_str(), _network_info.back_dns.c_str());
    if (_network_info.auto_dns == TRUE) {
        string tmp_dns = "";
        _network_manager->setDns(tmp_dns, tmp_dns);
    } else {
        ret = _network_manager->setDns(_network_info.main_dns, _network_info.back_dns);
        if(ret != 0)
        {
            LOG_ERR("rc_setdns fail!, ret=%d", ret);	
        }
    }

    struct rc_setnetifdevip_thread_t *arg;
    arg = (rc_setnetifdevip_thread_t*)malloc(sizeof(struct rc_setnetifdevip_thread_t));
    if (NULL == arg){
        LOG_PERROR("malloc memory error");
        return;
    }
    arg->callback =_callback;
    arg->data = _callback_data;
    arg->user = this;
    UnjoinableThread  set_ip_thread (LocalNetworkData::set_ip_info_thread, (void *)arg);
}

void *LocalNetworkData::set_ip_info_thread(void *data)
{

    int ret;
    string interface = RCC_DEFAULT_IFNAME;
    struct rc_setnetifdevip_thread_t *arg = (rc_setnetifdevip_thread_t*)data;
    Application* app = ((LocalNetworkData*)arg->user)->_app;
    NetworkInfo network_info;
    net::IPInfo ip_info;

    network_info = app->get_local_network_data().static_thread_get_ip_info();
    ip_info.dhcp = network_info.dhcp;
    if (ip_info.dhcp == FALSE) {
        ip_info.auto_dns = network_info.auto_dns;
        ip_info.dns      = network_info.main_dns;
        ip_info.dns_back = network_info.back_dns;
        ip_info.gate     = network_info.gateway;
        ip_info.ip_addr  = network_info.ip;
        ip_info.mask     = network_info.submask;
    }
    LOG_DEBUG("%d, %d, %d,%d",network_info.auto_dns,network_info.main_dns, network_info.gateway, network_info.ip);

    ret = app->get_local_network_data()._network_manager->setIPInfo(interface, ip_info);
    if (ret == 0) {
        LOG_ERR("set ip erros \n");
    }
    if (arg->callback){
        arg->callback(ret, arg->data);
    }
    
    free(data);

    return NULL;
}

void LocalNetworkData::activate_old()
{
    int ret = 0;
    /********
    FIXME: change the way to get dev.dev
    ********/
    strcpy(_dev->dev, RCC_DEFAULT_IFNAME);
    
#define INET_ATON(cp, inp)\
    if (inet_aton(cp.c_str(), &inp) == 0) {\
       LOG_EMERG("inet_aton error! item:%s, val:%s", #inp, cp.c_str()); \
    }

    _dev->link_speed  = _network_info.netcard_speed;
    if(_network_info.dhcp)
    {
		_dev->iptype = RC_IPTYPE_DHCP;
    }
    else
    {
		_dev->iptype = RC_IPTYPE_STATIC;
    }
    if(_network_info.auto_dns)
    {
		_dev->dnstype = RC_DNSTYPE_DHCP;
    }
    else
    {
		_dev->dnstype = RC_DNSTYPE_STATIC;
    }
    INET_ATON(_network_info.ip,         _dev->ip);
    INET_ATON(_network_info.submask,    _dev->netmask);
    INET_ATON(_network_info.gateway,    _dev->gateway);

    /*
     *if main_dns or back_dns is empty
     *we regard it as cleaning the configure in system
     *because these configures can be cleaned actually
     */
    if(_network_info.main_dns.empty())
    {
        _dev->dns1.s_addr = INADDR_NONE;
    }
    else
    {
        INET_ATON(_network_info.main_dns,   _dev->dns1);
    }

    if(_network_info.back_dns.empty())
    {
        _dev->dns2.s_addr = INADDR_NONE;
    }
    else
    {
        INET_ATON(_network_info.back_dns,   _dev->dns2);
    }
#undef INET_ATON
	LOG_DEBUG("iptype=%d, dnstype=%d, ip=0x%x, netmask=0x%x, gateway=0x%x, dns1=0x%x, dns2=0x%x", 
	_dev->iptype, _dev->dnstype, _dev->ip.s_addr, _dev->netmask.s_addr, _dev->gateway.s_addr, _dev->dns1.s_addr, _dev->dns2.s_addr);

    LOG_DEBUG("_network_info DHCP:%d, auto dns:%d, ip:%s, submask:%s, gateway:%s, main_dns:%s, back_dns:%s, speed:%d", 
                       _network_info.dhcp, _network_info.auto_dns, _network_info.ip.c_str(), _network_info.submask.c_str(), _network_info.gateway.c_str(), _network_info.main_dns.c_str(), _network_info.back_dns.c_str(), _network_info.netcard_speed);
    if(_dev->dnstype==RC_DNSTYPE_DHCP)
    {
        _dev->dns1.s_addr = INADDR_NONE;
        _dev->dns2.s_addr = INADDR_NONE;
    }
    ret = rc_setdns(_dev->dns1, _dev->dns2);

	if(ret < 0)
	{
		LOG_ERR("rc_setdns fail!, ret=%d", ret);	
	}
    ret = rc_setnetifdevip_noblock(_dev, _callback, _callback_data);
    if(ret < 0)
	{
		LOG_ERR("rc_setnetifdevip_noblock fail!, ret=%d", ret);	
	}
}

bool LocalNetworkData::is_dev_network_unknown()
{
    return (get_net_status() == NET_STATUS_UNKNOWN);
}

bool LocalNetworkData::is_dev_network_up()
{
    int net_status = get_net_status();
    return (net_status == NET_STATUS_ETH_UP || net_status == NET_STATUS_WLAN_UP);
}

bool LocalNetworkData::is_dev_eth_up()
{
    return (get_net_status() == NET_STATUS_ETH_UP);
}

bool LocalNetworkData::is_dev_wlan_up()
{
    return (get_net_status() == NET_STATUS_WLAN_UP);
}

bool LocalNetworkData::is_dev_network_down()
{
    return (get_net_status() == NET_STATUS_LINK_DOWN);
}

void LocalNetworkData::clear_wpa_conf(void)
{
    vector<string> vecContent;
    char cmd[128];

    vecContent.push_back("ctrl_interface=/var/run/wpa_supplicant");
    vecContent.push_back("ctrl_interface_group=0");
    vecContent.push_back("update_config=1");
    vecContent.push_back("");
    vecContent.push_back("network={");
    vecContent.push_back("    ssid=\"ruijie\"");
    vecContent.push_back("    psk=\"ruijie.com\"");
    vecContent.push_back("    key_mgmt=WPA-PSK");
    vecContent.push_back("}");

    std::ofstream outFile(WPA_CONF);

    if (outFile.is_open()) {
        LOG_INFO("WPA_CONF open success");
        vector<string>::const_iterator iter = vecContent.begin();
        for(;vecContent.end() != iter; ++iter)
        {
            outFile.write((*iter).c_str(), (*iter).size());
            outFile << '\n';
        }
        outFile.flush();
        outFile.close();

        sprintf(cmd, "cp %s %s", WPA_CONF, "/opt/lessons/wpa_supplicant/wpa_supplicant.conf");
        rc_system(cmd);
        rc_system("sync");
    } else {
        LOG_WARNING("WPA_CONF open faild");
    }

    return;
}

void LocalNetworkData::clear_wired_wpa_conf(void)
{
    vector<string> vecContent;
    char cmd[128];

    vecContent.push_back("ctrl_interface=/var/run/wpa_supplicant");
    vecContent.push_back("ctrl_interface_group=0");
    vecContent.push_back("update_config=1");
    vecContent.push_back("ap_scan=0");

    std::ofstream outFile(WIRED_WPA_CONF);

    if (outFile.is_open()) {
        LOG_INFO("WIRED_WPA_CONF open success");
        vector<string>::const_iterator iter = vecContent.begin();
        for(;vecContent.end() != iter; ++iter)
        {
            outFile.write((*iter).c_str(), (*iter).size());
            outFile << '\n';
        }
        outFile.flush();
        outFile.close();

        sprintf(cmd, "cp %s %s", WIRED_WPA_CONF, "/opt/lessons/wpa_supplicant/wired_wpa_supplicant.conf");
        rc_system(cmd);
        rc_system("sync");
    } else {
        LOG_WARNING("WIRED_WPA_CONF open faild");
    }
}

//networkmanager event callback
void LocalNetworkData::netMsgCallback(net::NetMsg& msg, void *user)
{
    Application* app = ((LocalNetworkData*)user)->_app;

    switch (msg.status) {
    case net::NET_UNAVAILABLE:
    case net::ETH_DOWN:
    case net::ETH_UP:
    case net::WLAN_DOWN:
    case net::WLAN_UP:
        LOG_INFO("recv net callback: network change START, uid = %d, version = %d, status = %d, reserved = %d, name = %s",
                msg.uid, msg.version, msg.status, msg.reserved, msg.name.c_str());
        ((LocalNetworkData*)user)->network_changed_handle(msg);
        LOG_INFO("recv net callback: network change END");
        break;
    case net::WLAN_NO_IP:
    case net::ETH_NO_IP:
        //TODO: show ui tip
        break;
    case net::ETH_EAP_AUTH_SUCCESS:
    case net::ETH_EAP_AUTH_FAILED:
    case net::ETH_EAP_AUTH_DISCON:
        LOG_INFO("recv net callback: auth change START, uid = %d, version = %d, status = %d, reserved = %d, name = %s",
                msg.uid, msg.version, msg.status, msg.reserved, msg.name.c_str());
        app->get_auth_manager()->authResultHandle(msg.status);
        break;
    case net::ETH_EAP_AUTH_EXIST:
    case net::ETH_EAP_AUTH_NOT_EXIST:
        LOG_INFO("recv net callback: auth env check, uid = %d, version = %d, status = %d, reserved = %d, name = %s",
                msg.uid, msg.version, msg.status, msg.reserved, msg.name.c_str());
        app->get_auth_manager()->authEnvHandle(msg.status);
        break;
    case net::TERMINATING:
        LOG_INFO("recv net callback: TERMINATING");
        break;
    case net::ETH1_DOWN:
    case net::ETH1_UP:
        ((LocalNetworkData*)user)->network2_changed_handle(msg);
        break;
    }
}

//wifimanager event callback
void LocalNetworkData::wlanEventCallback(int event, void *user, void* info)
{
    Application* app = ((LocalNetworkData*)user)->_app;

    switch (event) {
	case net::WIFI_EVENT_SCAN_RESULT:
	    //scan timer notify event
        LOG_INFO("recv wlan callback: SCAN_RESULT START");
        app->get_wifi_interactive()->wifi_scan_result_handle();
        LOG_INFO("recv wlan callback: SCAN_RESULT END");
	    break;
	case net::WIFI_EVENT_CONNECT_SUCCESS:
	    //authenticate success
        LOG_INFO("recv wlan callback: AUTH_SUCCESS START, info: %s", (const char* )info);
        app->get_wifi_interactive()->wifi_authenticate_result_handle(true, NULL);
        LOG_INFO("recv wlan callback: AUTH_SUCCESS END");
	    break;
	case net::WIFI_EVENT_CONNECT_FAILED:
	    //authenticate fail
        LOG_INFO("recv wlan callback: AUTH_FAIL START, info: %s", (const char* )info);
        app->get_wifi_interactive()->wifi_authenticate_result_handle(false, info);
        LOG_INFO("recv wlan callback: AUTH_FAIL END");
	    break;
    case net::WIFI_EVENT_DISCONNECT:
        LOG_INFO("recv wlan callback: DISCONNECT START, info: %s", (const char* )info);
        app->get_wifi_interactive()->wifi_disconnect_result_handle(info);
        LOG_INFO("recv wlan callback: DISCONNECT END");
        break;
    case net::WIFI_EVENT_CONNECT_START:
    case net::WIFI_EVENT_TIMEOUT:
    case net::WIFI_EVENT_TERMINATE:
    case net::WIFI_EVENT_DISCONNECT_FAILED:
    case net::WIFI_EVENT_RESERVED:
	    //reserved
	    break;
	}
}


bool LocalNetworkData::set_netcard_enable(int network_type, bool enable)
{
    if (_network_manager == NULL) {
        LOG_ERR("network manager is NULL!!");
        return NULL;
    }
    bool ret = _network_manager->setNetworkEnable(network_type, enable);
    LOG_INFO("set netcard enable: network_type = %d, enable = %d, ret = %d", network_type, enable, ret);
    return ret;
}

bool LocalNetworkData::get_netcard_enable(int network_type)
{
    if (_network_manager == NULL) {
        LOG_ERR("network manager is NULL!!");
        return NULL;
    }
    bool ret = _network_manager->getNetworkEnable(network_type);
    LOG_INFO("get netcard enable: network_type = %d, ret = %d", network_type, ret);
    return ret;
}

bool LocalNetworkData::check_wired_plugged()
{
    if (_network_manager == NULL) {
        LOG_ERR("network manager is NULL!!");
        return false;
    }
    bool ret = _network_manager->checkWiredNetworkPluged();
    LOG_INFO("check wired network pluged: %s", (ret == true ? "PLUGGED" : "NO PLUGGED"));
    return ret;
}

int LocalNetworkData::connect_wired_auth(const net::WiredAuthConfig& config)
{
    if (_network_manager == NULL) {
        LOG_ERR("network manager is NULL!!");
        return -1;
    }
    int ret = _network_manager->setWiredAuthConfig(config);
    LOG_INFO("connect wired auth: ret = %d", ret);
    return ret;
}

int LocalNetworkData::cancel_wired_auth()
{
    if (_network_manager == NULL) {
        LOG_ERR("network manager is NULL!!");
        return -1;
    }
    int ret = _network_manager->cancelWiredAuth();
    LOG_INFO("cancel wired auth: ret = %d", ret);
    return ret;
}

int LocalNetworkData::get_wired_auth_status()
{
    if (_network_manager == NULL) {
        LOG_ERR("network manager is NULL!!");
        return -1;
    }
    int ret = _network_manager->getWiredAuthStatus();
    LOG_INFO("get wired auth status: ret = %d", ret);
    return ret;
}

/**
 * check 802.1x env is exist
 * -1: Wired1XNotExist
 * -2: WiredAuthSettingIp
 * 0: Wired1XExist
 */
int LocalNetworkData::get_wired_auth_exist()
{
    if (_network_manager == NULL) {
        LOG_ERR("network manager is NULL!!");
        return -1;
    }

    int ret = _network_manager->getWired1xAuthExist();
    LOG_INFO("get wired auth exist: ret = %d", ret);
    
    return ret;
}


string LocalNetworkData::get_wireless_mac()
{
    if (_network_manager == NULL) {
        LOG_ERR("network manager is NULL!!");
        return NULL;
    }
    string ret = _network_manager->getMac(net::NetworkWireless);
    LOG_INFO("get_wireless_mac = %s",  ret.c_str());
    return ret;
}

string LocalNetworkData::get_wired_mac()
{
    if (_network_manager == NULL) {
        LOG_ERR("network manager is NULL!!");
        return NULL;
    }
    string ret = _network_manager->getMac(net::NetworkWired);
    LOG_INFO("get_wired_mac = %s",  ret.c_str());
    return ret;
}


NetworkInfo LocalNetworkData::get_ip_info()
{
    NetworkInfo ret;
    if (_network_manager == NULL) {
        LOG_ERR("network manager is NULL!!");
    } else {
        net::IPInfo ip_info;
        _network_manager->getIPInfo(ip_info);
        ret.dhcp =      ip_info.dhcp;
        ret.ip =        ip_info.ip_addr;
        ret.submask =   ip_info.mask;
        ret.gateway =   ip_info.gate;
        ret.auto_dns=   ip_info.auto_dns;
        ret.main_dns=   ip_info.dns;
        ret.back_dns=   ip_info.dns_back;
        LOG_INFO("get ip info: dhcp %d, ip %s, mask %s, gate %s, autodns %d, dns %s, backdns %s", ret.dhcp,
            ret.ip.c_str(), ret.submask.c_str(), ret.gateway.c_str(), ret.auto_dns, ret.main_dns.c_str(), ret.back_dns.c_str());
    }
    return ret;
}

std::list<net::ScanResult> LocalNetworkData::get_scan_result_force()
{
    std::list<net::ScanResult> result;
    if (_wifi_manager == NULL) {
        LOG_ERR("wifi manager is NULL!!");
    } else {
        _wifi_manager->scan(result);
        LOG_INFO("get_scan_result_force, list size = %d", result.size());
    }
    return result;
}

std::list<net::ScanResult> LocalNetworkData::get_scan_result()
{
    std::list<net::ScanResult> result;
    if (_wifi_manager == NULL) {
        LOG_ERR("wifi manager is NULL!!");
    } else {
        _wifi_manager->getScanResult(result);
        LOG_INFO("get_scan_result, list size = %d", result.size());
    }
    return result;
}

net::WifiInfo LocalNetworkData::get_connect_wifi_info()
{
    net::WifiInfo ret;

    if (_wifi_manager == NULL) {
        LOG_ERR("wifi manager is NULL!!");
        return ret;
    }

    _wifi_manager->getConnectInfo(ret);
    LOG_INFO("wifi info: ssid = %s, ssid_mac = %s, freq = %u, signal = %d, key_mgmt = %u, ip_addr = %s",
        ret.ssid.c_str(), ret.ssid_mac.c_str(), ret.freq, ret.signal, ret.key_mgmt, ret.ip_info.ip_addr.c_str());
    return ret;
}

/** check wifi is saved
 *  Return:  >=  0: return ssid of recorded wifi
 *              == -1: wifi not saved
 */
int LocalNetworkData::check_wifi_saved(const string ssid, const string ssid_mac, const int proto)
{
    if (_wifi_manager == NULL) {
        LOG_ERR("wifi manager is NULL!!");
        return -1;
    }
    int ret = _wifi_manager->checkNetworkSaved(ssid, ssid_mac, proto);
    LOG_INFO("check_wifi_saved: ssid = %s, ssid_mac = %s, proto = %d, ret = %d", ssid.c_str(), ssid_mac.c_str(), proto, ret);
    return ret;
}

/**
* function: get history ssid from terminal lib 
* @ssid_list:save ssid list
* 
* return: -1 error, 0 ok
*/
int LocalNetworkData::get_ssid_his_list(list<net::WifiConfig>& his_list)
{
    if (_wifi_manager == NULL) {
        LOG_ERR("wifi manager is NULL!!");
        return -1;
    }
    int ret = _wifi_manager->getSavedList(his_list);
    LOG_INFO("ret = %d his_size = %d", ret, his_list.size());
    return ret;
}

/**
* function: set white ssid list to terminal
* @in_json: json data to lib
* @out_json: return json data 
*
*/
void LocalNetworkData::set_white_ssid_list(string &in_json, string &out_json)
{
    if (_wifi_manager == NULL) {
        LOG_ERR("wifi manager is NULL!!");
        return;
    } 
    _wifi_manager->doJsonCommand(in_json, out_json);
}

bool LocalNetworkData::connect_wifi(int net_id)
{
    if (_wifi_manager == NULL) {
        LOG_ERR("wifi manager is NULL!!");
        return false;
    }
    bool ret = _wifi_manager->connect(net_id);
    LOG_INFO("connect_wifi: net_id = %d, ret = %d", net_id, ret);
    return ret;
}

bool LocalNetworkData::connect_wifi(net::WifiConfig config)
{   
    if (_wifi_manager == NULL) {
        LOG_ERR("wifi manager is NULL!!");
        return false;
    }
    bool ret = _wifi_manager->connect(config);
    LOG_INFO("connect_wifi: config, ret = %d", ret);
    return ret;
}

bool LocalNetworkData::disconnect_wifi()
{
    if (_wifi_manager == NULL) {
        LOG_ERR("wifi manager is NULL!!");
        return false;
    }
    bool ret = _wifi_manager->disconnect();
    LOG_INFO("disconnect_wifi: ret = %d", ret);
    return ret;
}

int LocalNetworkData::get_net_status()
{
    if (_network_manager == NULL) {
        LOG_ERR("network manager is NULL!!");
        return -1;
    }
    int status = _network_manager->getStatus();
    if (status < 0)
    {
        LOG_DEBUG("get_net_status_err!! status FAIL, net_status = %d", status);
        return NET_STATUS_UNKNOWN;
    }
    if (status == (net::ETH_UP | net::WLAN_DOWN))
    {
        LOG_DEBUG("get_net_status: ETH_UP, net_status = %d", status);
        return NET_STATUS_ETH_UP;
    }    
    else if (status == (net::ETH_DOWN | net::WLAN_UP))
    {    
        LOG_DEBUG("get_net_status: WLAN_UP, net_status = %d", status);
        return NET_STATUS_WLAN_UP;
    }
    else if (status == net::NET_UNAVAILABLE)
    {
        LOG_DEBUG("get_net_status: LINK_DOWN, net_status = %d", status);
        return NET_STATUS_LINK_DOWN;
    }
    else if (status == (net::ETH_UP | net::WLAN_UP))
    {
        LOG_ERR("get_net_status_err!! status ETH_UP, net_status = %d", status);
        return NET_STATUS_ETH_UP;
    }
    else
    {
        LOG_ERR("unknown net status!! net_status = %d", status);
        return NET_STATUS_UNKNOWN;
    }

}

int LocalNetworkData::get_wifi_status()
{
    //0: wifi disconnect
    //1: wifi connected
    //2: wifi connecting   
    if (_wifi_manager == NULL) {
        LOG_ERR("wifi manager is NULL!!");
        return -1;
    }
    int status = _wifi_manager->getStatus();    
    if (status < 0)
    {
        LOG_DEBUG("call get_wifi_status fail");
        return -1;
    }
    LOG_DEBUG("get_wifi_status: 0x%x", status);
    return status;
}

//forget current wifi, cancel auto link
bool LocalNetworkData::forget_wifi(int net_id)
{
    if (_wifi_manager == NULL) {
        LOG_ERR("wifi manager is NULL!!");
        return false;
    }
    bool ret = _wifi_manager->forgetNetwork(net_id);
    LOG_DEBUG("forget_wifi: net_id = %d, ret = %d", net_id, ret);
    return ret;
}

void LocalNetworkData::network_changed_handle(net::NetMsg& msg)
{
    _last_dev_net_status = _dev_net_status;
    _dev_net_status = get_net_status();
    LOG_DEBUG("last net status: %d, cur net status: %d", _last_dev_net_status, _dev_net_status);
    
    network_ui_change_handle(msg.status);
    network_status_process();
}

void LocalNetworkData::network2_changed_handle(net::NetMsg& msg)
{
    _last_dev_net2_status = _dev_net2_status;
    int status = msg.status;
    if (status == net::ETH1_UP)
    {
        LOG_DEBUG(" ETH1_UP, net_status = %d", status);
        _dev_net2_status = NET_STATUS_ETH_UP;
    }    

    else if (status == net::ETH1_DOWN)
    {
        LOG_DEBUG(" ETH1_DOWN, net_status = %d", status);
        _dev_net2_status = NET_STATUS_LINK_DOWN;
    }
    LOG_DEBUG("last net2 status: %d, cur net2 status: %d", _last_dev_net2_status, _dev_net2_status);
    network2_status_process();
}

void LocalNetworkData::network_ui_change_handle(const int net_status)
{
    _app->get_wifi_interactive()->net_status_change_handle(net_status);
}

void LocalNetworkData::network_status_process()
{
    if (_last_dev_net_status == NET_STATUS_UNKNOWN || _last_dev_net_status == NET_STATUS_LINK_DOWN) {
        if (_dev_net_status == NET_STATUS_ETH_UP || _dev_net_status == NET_STATUS_WLAN_UP) {            
            link_down_to_up();  //link  up
        } else {
            //do nothing
        }
    } else if (_last_dev_net_status == NET_STATUS_ETH_UP) {
        if (_dev_net_status == NET_STATUS_UNKNOWN || _dev_net_status == NET_STATUS_LINK_DOWN) {
            link_up_to_down();  //link  down
        } else if (_dev_net_status == NET_STATUS_WLAN_UP) {
            //eth to wifi
            link_up_to_down();
            link_down_to_up();
        } else {
            // do nothing
        }
    } else if (_last_dev_net_status == NET_STATUS_WLAN_UP) {
        if (_dev_net_status == NET_STATUS_UNKNOWN || _dev_net_status == NET_STATUS_LINK_DOWN) {
            link_up_to_down();  //link  down
        } else if (_dev_net_status == NET_STATUS_ETH_UP) {
            //wifi to eth            
            link_up_to_down();
            link_down_to_up();
        } else {
            // do nothing
        }      
    } else {
        LOG_ERR("unknown net status!! net_status = %d", _last_dev_net_status);
    }
}


void LocalNetworkData::link_down_to_up()
{
    _app->logic_link_up();
}

void LocalNetworkData::link_up_to_down()
{    
    _app->logic_link_down(); 
}

void LocalNetworkData::network2_status_process()
{
    if (_last_dev_net2_status == NET_STATUS_UNKNOWN || _last_dev_net2_status == NET_STATUS_LINK_DOWN) {
        if (_dev_net2_status == NET_STATUS_ETH_UP) {            
            link_down_to_up2();  //link  up
        } else if (_dev_net2_status == NET_STATUS_UNKNOWN || _dev_net2_status == NET_STATUS_LINK_DOWN) {
            link_up_to_down2();  //link  down
        } else {
            // do nothing
        }
    } else if (_last_dev_net2_status == NET_STATUS_ETH_UP) {
        if (_dev_net2_status == NET_STATUS_UNKNOWN || _dev_net2_status == NET_STATUS_LINK_DOWN) {
            link_up_to_down2();  //link  down
        }  else {
            // do nothing
        }
    }  else {
        LOG_ERR("unknown net status!! net_status = %d", _last_dev_net2_status);
    }
}


void LocalNetworkData::link_down_to_up2()
{
    _app->logic_link1_up();
}

void LocalNetworkData::link_up_to_down2()
{    
    _app->logic_link1_down(); 
}

bool LocalNetworkData::get_net2_status()
{
    bool status = _network_manager->getNet2Status();
    return status;
}

#if 0
int LocalNetworkData::invoke_iptable_scripts(const string& wanip, const string& lanip, int dport, int dnatport, int type)
{
#define IPTABLE_CONVERT_SCRIPT_PATH  "/etc/rc_script/iptables_convert_port.sh"

    char cmd[256] = {0};
    int ret = -1;

    LOG_DEBUG("wanip is %s, dport is %d, dnaport is %d, type is %d, lanip is %s",  \
            wanip.c_str(), dport, dnatport, type, lanip.c_str());

    if (access(IPTABLE_CONVERT_SCRIPT_PATH, F_OK) != 0) {
        LOG_ERR("setting nat iptables rule scripts  not exist");
        return -1;
    }

    if (wanip.empty()) {
        LOG_ERR("wanip is empty, return");
        return -1;
    }

    snprintf(cmd, sizeof(cmd), "bash "IPTABLE_CONVERT_SCRIPT_PATH" %s %d %d %d %s",  \
            wanip.c_str(), dport, dnatport, type, lanip.c_str());

    ret = rc_system(cmd);

    return ret;

#undef IPTABLE_CONVERT_SCRIPT_PATH
}
#endif

void LocalNetworkData::add_or_remove_port_convertion_rules(
    const string& wanip, const string& wanport,
    const string& privateip, string& privateport,
    const string& btPortsRange, bool add)
{
    int operation;
    int rv;

    if (add) {
        operation = 0;
    } else {
        operation = 1;
    }

    /* http mina, translate port only*/
    rv = _network_manager->setPortConvert(wanip, wanport,
          wanip, privateport, "tcp", operation);
    if (rv != 0) {
       LOG_ERR("%s nat rule for http mina meets error %d.", add?"Adding":"Removing",  rv);
    } else {
       LOG_DEBUG("Nat rule for http mina is %s.\n", add?"added":"removed");
    }

    /* torrent wget http, lan->wan, privateport-> public port */
    rv = _network_manager->setPortConvert(wanip, wanport,
       privateip, privateport , "tcp", operation);
    if (rv != 0) {
       LOG_ERR("%s nat rule for torrent wget meets error %d.", add?"Adding":"Removing", rv);
    } else {
        LOG_DEBUG("Nat rule for torrent wget is %s.\n", add?"added":"removed");
     }

    /* in bt ports range [a:b], lan ip->wan ip */
    rv = _network_manager->setPortConvert(wanip, "",
       privateip, btPortsRange, "tcp", operation);
    if (rv != 0) {
        LOG_ERR("%s nat rule for torrent ranges meets error %d.", add?"Adding":"Removing", rv);
    } else {
        LOG_DEBUG("Nat rule for bt is %s.\n", add?"added":"removed");
     }



}

int LocalNetworkData::set_port_convert()
{
	HttpPortInfo now, last;
	std::string privatePort;
	std::string publicPort;

    _app->get_UsrUserInfoMgr()->get_http_port_map(now);
    _app->get_UsrUserInfoMgr()->get_http_last_port_map(last);

    //del last rule
    if (last.public_port != 80) {
        if (!last.private_ip.empty() && !last.public_ip.empty()) {
            LOG_DEBUG("DEL, pri(%s:%d)=>pub(%s:%d), bt ports range %s",
               last.private_ip.c_str(), last.private_port,
               last.public_ip.c_str(), last.public_port,
               last.btPortsRange);

           publicPort = std::to_string(last.public_port);
           privatePort = std::to_string(last.private_port);

           add_or_remove_port_convertion_rules(last.public_ip, publicPort, last.private_ip,
               privatePort, last.btPortsRange, false);

        } else if (last.private_ip.empty()) {
            LOG_DEBUG("last private ip is empty");
        }else {
            LOG_DEBUG("last public ip is empty");
        }
    } else {
        LOG_DEBUG("public port is 80, skip removing nat rule.\n");
    }

    //add new rule
    if (now.public_port != 80) {
        if (!now.private_ip.empty() && !now.public_ip.empty()) {
            LOG_DEBUG("ADD pri(%s:%d)=>pub(%s:%d), bt ports range %s",
                now.private_ip.c_str(), now.private_port,
                now.public_ip.c_str(), now.public_port, now.btPortsRange);

            publicPort = std::to_string(now.public_port);
            privatePort = std::to_string(now.private_port);

            add_or_remove_port_convertion_rules(now.public_ip, publicPort, now.private_ip,
                privatePort, now.btPortsRange, true);
        } else if (now.private_ip.empty()) {
            LOG_DEBUG("new private ip is empty");
        } else {
            LOG_DEBUG("new public ip is empty");
        }
    } else {
        LOG_DEBUG("public port is 80, skip add nat rule.\n");
    }

    return 0;
}
