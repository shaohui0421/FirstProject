#ifndef _LOCAL_NETWORK_DATA_H
#define _LOCAL_NETWORK_DATA_H

#include <string>
#include <list>
#include "common.h"
#include "net_api.h"

class Application;

#define WPA_CONF         "/etc/wpa_supplicant/wpa_supplicant.conf"
#define WIRED_WPA_CONF   "/etc/wpa_supplicant/wired_wpa_supplicant.conf"

enum NetStatus{
    NET_STATUS_UNKNOWN = 0,
    NET_STATUS_ETH_UP = 1,
    NET_STATUS_WLAN_UP = 2,
    NET_STATUS_LINK_DOWN = 3,        
};


//wifi unit test
#if 0
namespace net
{
    
    #define NETWORK_UID (0x4E455400)

    enum KeyMgmt {
        KEY_OPEN         = 0,
        KEY_WEP          = 1,
        KEY_WPA_WAP2_PSK = 2,
        KEY_WPA_WAP2_EAP = 3,
        KEY_OTHER        = 4,
    };
    
    enum NetStatus {
        NET_UNAVAILABLE = 0x0,
        ETH_DOWN        = 0x1,
        ETH_UP          = 0x2,
        WLAN_DOWN       = 0x4,
        WLAN_UP         = 0x8,
    };
    
    enum EapMethod {
        EAP_PEAP  = 0,
        EAP_TLS   = 1,
        EAP_MD5   = 2,
        EAP_TTLS  = 3,
        EAP_OTHER = 4,
    };
    
    enum WifiEventInfo {
        WIFI_EVENT_SCAN_RESULT      = 0,
        WIFI_EVENT_CONNECT_SUCCESS  = 1,
        WIFI_EVENT_CONNECT_FAILED   = 2,
        WIFI_EVENT_CONNECT_START    = 3,
        WIFI_EVENT_DISCONNECT       = 4,
        WIFI_EVENT_RESERVED         = 5,
    };

    enum NetworkType {
        NetworkWired = 0,
        NetworkWireless = 1,
        NetworkOther = 2,
    };
    
    class WifiConfig
    {
    public:
        WifiConfig() : priority(0),net_id(-1),key_mgmt(KEY_OTHER),scan_id(false),
                    eap(EAP_PEAP),saved(true),flags(0) {}
        ~WifiConfig() {}
    
        string       ssid;
        string       ssid_mac;
        int          priority;
        int          net_id;
        unsigned int key_mgmt;
        bool         scan_id;
        string       psk;
        string       wep_key0;
        unsigned int eap;
        string       identity;
        string       password;
        bool         saved;
        int          flags;
    private:
    };
    
    class ScanResult
    {
    public:
        ScanResult():freq(0),idensity(0), key_mgmt(KEY_OTHER) {}
    
        ~ScanResult() {}
    
        ScanResult(const ScanResult* other)
        {
            ssid = other->ssid;
            ssid_mac = other->ssid_mac;
            freq = other->freq;
            idensity = other->idensity;
            key_mgmt = other->key_mgmt;
            flag = other->flag;
        }
    
        ScanResult(const ScanResult& other)
        {
            ssid = other.ssid;
            ssid_mac = other.ssid_mac;
            freq = other.freq;
            idensity = other.idensity;
            key_mgmt = other.key_mgmt;
            flag = other.flag;
        }
    
        ScanResult& operator=(const ScanResult & other)
        {
            if (this == &other) return *this;
    
            ssid = other.ssid;
            ssid_mac = other.ssid_mac;
            freq = other.freq;
            idensity = other.idensity;
            key_mgmt = other.key_mgmt;
            flag = other.flag;
    
            return *this;
        }
    
        string       ssid;
        string       ssid_mac;
        unsigned int freq;
        int          idensity;
        int          key_mgmt;
        string       flag;
    private:
    };
    
    class IPInfo
    {
    public:
        IPInfo() : dhcp (true), auto_dns(true) {}
    
        IPInfo(const IPInfo* other)
        {
            ip_addr = other->ip_addr;
            mask = other->mask;
            gate = other->gate;
            dns = other->dns;
            dns_back = other->dns_back;
            mac = other->mac;
            dhcp = other->dhcp;
            auto_dns = other->auto_dns;
        }
    
        IPInfo(const IPInfo& other)
        {
            ip_addr = other.ip_addr;
            mask = other.mask;
            gate = other.gate;
            dns = other.dns;
            dns_back = other.dns_back;
            mac = other.mac;
            dhcp = other.dhcp;
            auto_dns = other.auto_dns;
        }
    
        IPInfo& operator=(const IPInfo & other)
        {
            if (this == &other) return *this;
    
            ip_addr = other.ip_addr;
            mask = other.mask;
            gate = other.gate;
            dns = other.dns;
            dns_back = other.dns_back;
            mac = other.mac;
            dhcp = other.dhcp;
            auto_dns = other.auto_dns;
            return *this;
        }
    
        ~IPInfo() {}
    
        string ip_addr;
        string mask;
        string gate;
        string dns;
        string dns_back;
        string mac;
        bool dhcp ;
        bool auto_dns;
    };
    
    class WifiInfo
    {
    public:
        WifiInfo() : freq(0), signal(0),key_mgmt(KEY_OTHER) {}
    
        WifiInfo(const WifiInfo* other)
        {
            ssid = other->ssid;
            freq = other->freq;
            signal = other->signal;
            key_mgmt = other->key_mgmt;
            ip_info = other->ip_info;
        }
    
        WifiInfo(const WifiInfo& other)
        {
            ssid = other.ssid;
            freq = other.freq;
            signal = other.signal;
            key_mgmt = other.key_mgmt;
            ip_info = other.ip_info;
        }
    
        WifiInfo& operator=(const WifiInfo & other)
        {
            if (this == &other) return *this;
    
            ssid = other.ssid;
            freq = other.freq;
            signal = other.signal;
            key_mgmt = other.key_mgmt;
            ip_info = other.ip_info;
    
            return *this;
        }
    
        ~WifiInfo() {}
    
        string          ssid;
        unsigned int    freq;
        int             signal;
        unsigned int    key_mgmt;
        IPInfo          ip_info;
    
    };
    
    class NetMsg
    {
    public:
        NetMsg() : uid(NETWORK_UID), version(1), status(NET_UNAVAILABLE), reserved(0) {}
        ~NetMsg() {}
    
        int    uid;
        int    version;
        int    status;
        int    reserved;
        string name;
    
    private:
    };


    typedef void (*onNetEvent) (NetMsg* msg, void *user);
    class NetworkManager
    {
    public:
    
        NetworkManager(onNetEvent cb, void* user)
            :mUser(user)
            ,mCb(cb)
            {
            }
        ~NetworkManager(){}
        
        int getStatus() { return 3; }
        bool setNetworkEnable(int network_type, bool enable) { return true; }
        bool getNetworkEnable(int network_type) { return false; }
    
        string getMac(const char* interface) { return ""; }
        IPInfo getIPInfo() { return NULL; }
    
    private:
        void*       mUser;
        onNetEvent     mCb;
    
    };
    
    typedef void (*onWlanEvent)(int event, void *user, void* info);
    //typedef void (*onJsonEvent)(string msg, void* user);
    class WifiManager
    {
    public:
        WifiManager(onWlanEvent cb, void* user)
        {
            mUser = user;
            mCb = cb;

            //wifi_info
            IPInfo ip_info;
            ip_info.ip_addr = "172.21.5.118";
            ip_info.mask = "255.255.255.0";
            ip_info.gate = "172.21.5.1";
            ip_info.dns = "114.114.114.114";
            ip_info.dns_back = "114.114.114.113";
            
            _wifi_info.ssid = "ruijie_802.1x";
            _wifi_info.freq = 2400;
            _wifi_info.signal = -59;
            _wifi_info.key_mgmt = 1;
            _wifi_info.ip_info = ip_info;

            //scan_result
            char buf[64];
            for(int i=0; i<scan_list_size; i++) {
                ScanResult sr;
                sprintf(buf, "ruijie%d_802.1x", i);
                sr.ssid = buf;
                sr.ssid_mac = "58:69:6c:97:d5:dd";
                sr.key_mgmt = i;
                sr.idensity = -1 - i*15;
                sr.freq = 2400;
                sr.flag = "58:69:6c:97:d5:dd";
                _scan_list.push_back(sr);
            }

            //timer
            pthread_t thread_id;
            pthread_create(&thread_id, NULL, WifiManager::timer, NULL);
            
        }
        //WifiManager(onJsonEvent cb, void* user);
        ~WifiManager()
        {
        }

        static void* timer(void* data)
        {
            while (1)
            {
                sleep(1);
                //WifiManager::mCb(0, WifiManager::mUser, NULL);
                //update_wifi();
            }
        }
        static void update_wifi()
        {
            //wifi info            
            if (_wifi_info.signal >= 0)
            {
                _wifi_info.signal = -100;
            }
            _wifi_info.signal += 20;
            
            //scan list
            std::list<ScanResult>::iterator it;
            it = _scan_list.begin();
            if (it->idensity >= 0)
            {
                it->idensity = -100;
            }
            it->idensity += 10;
        }
        std::list<ScanResult> scan()
        {
            return _scan_list; 
        }
        std::list<ScanResult> getScanResult()
        {
            return _scan_list; 
        }
        WifiInfo getConnectInfo()
        {
            return _wifi_info;
        }
    
        int  checkNetworkSaved(const string ssid,      const string ssid_mac, const int proto){ return 0; }
        int  getStatus(){ return 0; }
        bool connect(int net_id){ return true; }
        bool connect(WifiConfig config){ return true; }
        bool disconnect(){ return true; }
        bool forgetNetwork(int net_id){ return true; }
    
        //void doJsonCommand(string in_json, string& out_json);
    
        //static int onEvent(void* data, size_t len, void *ctx);
    
        //bool   doBoolCmd(string cmd);
        //int    doIntCmd(string cmd);
        //string doStringCmd(string cmd);
    
        //list<ScanResult > formatResult(string result);
    
        //pthread_mutex_t mMutex;
    
        //NetClient* mCmd;
        //NetClient* mMonitor;

        const int scan_list_size = 4;
        static WifiInfo _wifi_info;
        static std::list<ScanResult> _scan_list;

        static void*       mUser;
        static onWlanEvent mCb;
        //onJsonEvent mJcb;
    };

}

#endif
//wifi unit test


typedef int (*LocalNetworkDataCallback)(int, void *);

class LocalNetworkData
{
public:
    LocalNetworkData(Application* app, LocalNetworkDataCallback callback, void* callback_data);
    
    ~LocalNetworkData()
    {
		delete _dev;
        LOG_WARNING("~LocalNetworkData");
        if (_network_manager) {
            LOG_WARNING("~LocalNetworkData: _network_manager");
            delete _network_manager;
        }
        if (_wifi_manager) {
            LOG_WARNING("~LocalNetworkData: _wifi_manager");
            delete _wifi_manager;
        }
    }
    
    const NetworkInfo& get() 
    {
	    update(); 
	    return _network_info;
    }
    
    const NetworkInfo& static_thread_get_ip_info()
    {
        return _network_info;
    }

    void set(const NetworkInfo& info)
    {
        _network_info = info;
        LOG_DEBUG("_network_info DHCP:%d, auto dns:%d, ip:%s, summask:%s, gateway:%s, main_dns:%s, back_dns:%s, speed:%d", 
                           _network_info.dhcp, _network_info.auto_dns, _network_info.ip.c_str(), _network_info.submask.c_str(), _network_info.gateway.c_str(), _network_info.main_dns.c_str(), _network_info.back_dns.c_str(), _network_info.netcard_speed);
        if (_network_manager) {
            activate();
        } else {
            activate_old();
        }
    }
    void update();
    
    bool is_dev_network_unknown();
    bool is_dev_network_up();
    bool is_dev_eth_up();
    bool is_dev_wlan_up();
    bool is_dev_network_down();
    void clear_wpa_conf(void);
    void clear_wired_wpa_conf(void);

    //net::NetworkManager
    static void netMsgCallback(net::NetMsg& msg, void *user);
    void network_changed_handle(net::NetMsg& msg);
    void network2_changed_handle(net::NetMsg& msg);
    int get_net_status();
    bool get_net2_status();
    bool set_netcard_enable(int network_type, bool enable);
    bool get_netcard_enable(int network_type);
    bool check_wired_plugged();
    int connect_wired_auth(const net::WiredAuthConfig& config);
    int cancel_wired_auth();
    int get_wired_auth_status();
    string get_wireless_mac();
    string get_wired_mac();
    NetworkInfo get_ip_info();
    static void *set_ip_info_thread(void *data);
    int get_wired_auth_exist();
    
    int set_port_convert();

    //net::WifiManager
    static void wlanEventCallback(int event, void *user, void* info);
    std::list<net::ScanResult> get_scan_result_force();
    std::list<net::ScanResult> get_scan_result();
    net::WifiInfo get_connect_wifi_info();
    int check_wifi_saved(string ssid, string ssid_mac, int proto);
    int get_ssid_his_list(list<net::WifiConfig>& his_list);
    void set_white_ssid_list(string &in_json, string &out_json);
    bool connect_wifi(int net_id);
    bool connect_wifi(net::WifiConfig config);
    bool disconnect_wifi();
    int get_wifi_status();
    bool forget_wifi(int net_id);
    Application* get_application() { return _app; }
private:
    void activate();
    void activate_old();
    void network_ui_change_handle(const int net_status);
    void network_status_process();
    void link_down_to_up();
    void link_up_to_down();
    void network2_status_process();
    void link_down_to_up2();
    void link_up_to_down2();
    int invoke_iptable_scripts(const string& wanip, const string& lanip, int dport, int dnatport, int type);

    void add_or_remove_port_convertion_rules(
        const string& wanip, const string& wanport,
        const string& privateip, string& privateport,
        const string& btPortsRange, bool add);

    Application* _app;
    LocalNetworkDataCallback _callback;
    void* _callback_data;
    NetworkInfo _network_info;
    rc_netifdev_t* _dev;
    
    int _dev_net_status;
    int _last_dev_net_status;
    int _dev_net2_status;
    int _last_dev_net2_status;

    net::NetworkManager* _network_manager;
    net::WifiManager* _wifi_manager;
};

#endif//_LOCAL_NETWORK_DATA_H
