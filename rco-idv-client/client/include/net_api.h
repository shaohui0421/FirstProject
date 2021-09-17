#ifndef _NET_API_H
#define _NET_API_H

#include <list>
#include <string>
#include <memory>

using std::string;
using std::list;

namespace net
{

#define WIFI_SCAN_LIST_MAX_NUM 50

enum KeyMgmt {
    KEY_OPEN         = 0,
    KEY_WEP          = 1,
    KEY_WPA_WAP2_PSK = 2,
    KEY_WPA_WAP2_EAP = 3,
    KEY_OTHER        = 4,
};

enum NetStatus {
    NET_UNAVAILABLE  = 0x0,
    ETH_DOWN         = 0x1,
    ETH_UP           = 0x2,
    WLAN_DOWN        = 0x4,
    WLAN_UP          = 0x8,
    WLAN_NO_IP       = 0x10,
    ETH_NO_IP        = 0x20,
    ETH_EAP_AUTH_SUCCESS = 0x30,
    ETH_EAP_AUTH_FAILED  = 0x40,
    ETH_EAP_AUTH_DISCON  = 0x80,
    ETH_EAP_AUTH_EXIST     = 0x81,
    ETH_EAP_AUTH_NOT_EXIST     = 0x82,
    ETH1_DOWN                 = 0x83,
    ETH1_UP                   = 0x84,
    TERMINATING      = 0xff,
};

enum EapMethod {
    EAP_PEAP  = 0,
    EAP_TLS   = 1,
    EAP_MD5   = 2,
    EAP_TTLS  = 3,
    EAP_OTHER = 4,
};

enum WiredAuthMethod {
    WiredAuthNone  = 0,
    WiredAuthDot1x = 1,
    WiredAuthWeb   = 2,
    WiredAuthOther = 3,
};

enum WiredAuthStatus {
    WiredAuthSettingIp = -2,
    WiredAuthStatusUnnecessary  = -1,
    WiredAuthStatusSuccess      = 0,
    WiredAuthStatusAuthing      = 1,
    WiredAuthStatusFailed       = 2,
    WiredAuthStatusDisconnect   = 3,
    WiredAuthStatusOther        = 4,
};

enum WifiEventInfo {
    WIFI_EVENT_SCAN_RESULT       = 0,
    WIFI_EVENT_CONNECT_SUCCESS   = 1,
    WIFI_EVENT_CONNECT_FAILED    = 2,
    WIFI_EVENT_CONNECT_START     = 3,
    WIFI_EVENT_DISCONNECT        = 4,
    WIFI_EVENT_TIMEOUT           = 5,
    WIFI_EVENT_TERMINATE         = 6,
    WIFI_EVENT_DISCONNECT_FAILED = 7,
    WIFI_EVENT_RESERVED          = 8
};

enum NetworkType {
    NetworkWired    = 0,
    NetworkWireless = 1,
    NetworkOther    = 2,
};

enum NetJsonHandle {
    /* GuestTool */
    NET_HANDLE_SET_NONE             = 0,
    NET_HANDLE_SET_NET_MSG          = 1,
    NET_HANDLE_SET_WIFI_INFO        = 2,
    NET_HANDLE_SET_SCAN_RESULT      = 3,
    NET_HANDLE_SET_CHECK_SAVED      = 4,
    NET_HANDLE_SET_WIFI_EVENT       = 5,
    NET_HANDLE_SET_WIRED_MAC        = 6,
    NET_HANDLE_WIRELESS_STATUS      = 7,
    NET_HANDLE_SET_SCAN_TIME         = 8,
    NET_HANDLE_SET_BUTT,

    NET_HANDLE_REQ_NONE             = 100,
    NET_HANDLE_REQ_SCAN_RESULT      = 101,
    NET_HANDLE_REQ_CHECK_SAVED      = 102,
    NET_HANDLE_REQ_CONNECT_WITH_CFG = 103,
    NET_HANDLE_REQ_CONNECT_WITH_ID  = 104,
    NET_HANDLE_REQ_WIFI_INFO        = 105,
    NET_HANDLE_REQ_DISCONNECT       = 106,
    NET_HANDLE_REQ_FORGET_WIFI      = 107,
    NET_HANDLE_REQ_NET_STATUS       = 108,
    NET_HANDLE_REQ_WIRED_MAC        = 109,
    NET_HANDLE_REQ_WIRELESS_STATUS  = 110,
    NET_HANDLE_REQ_SCAN_TIME  = 111,
    NET_HANDLE_REQ_BUTT,


    /* Client */
    NET_HANDLE_CLT_SET_WHITE_LIST          = 200,

    NET_HANDLE_CLT_REQ_WHITE_LIST_SETTING  = 300,
};


class WifiConfig
{
public:
    WifiConfig();
    ~WifiConfig() {}

    WifiConfig& operator=(const WifiConfig & );
    WifiConfig(const WifiConfig & );
    void clear();

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
};

class WiredAuthConfig
{
public:
    WiredAuthConfig();
    ~WiredAuthConfig() {}

    WiredAuthConfig& operator=(const WiredAuthConfig & );
    WiredAuthConfig(const WiredAuthConfig & );

    int     type;
    string  identity;
    string  password;
    bool    saved;
    int     eap;
    int     flags;
};

class ScanResult
{
public:
    ScanResult();
    ~ScanResult() {}

    ScanResult(const ScanResult* other);
    ScanResult(const ScanResult& other);

    ScanResult& operator=(const ScanResult & other);
    bool operator==(const ScanResult & other);

    static bool compare(const ScanResult& a, const ScanResult& b);

    string       ssid;
    string       ssid_mac;
    unsigned int freq;
    int          idensity;
    int          key_mgmt;
    string       flag;
};

class IPInfo
{
public:
    IPInfo();
    IPInfo(const IPInfo* other);
    IPInfo(const IPInfo& other);
    IPInfo& operator=(const IPInfo & other);
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
    WifiInfo();
    WifiInfo(const WifiInfo* other);
    WifiInfo(const WifiInfo& other);
    WifiInfo& operator=(const WifiInfo & other);

    ~WifiInfo() {}

    string          ssid;
    string          ssid_mac;
    unsigned int    freq;
    int             signal;
    unsigned int    key_mgmt;
    IPInfo          ip_info;

};

class NetMsg
{
public:
    NetMsg() : uid(0), version(0), status(-1), reserved(0) {}
    ~NetMsg() {}

    int    uid;
    int    version;
    int    status;
    int    reserved;
    string name;
};

typedef void (*onJsonEvent)(string &msg, void *user);
typedef void (*onNetEvent)(NetMsg &msg, void *user);
typedef void (*onOption)(const char* msg, void* ctx);

class NetworkManagerImpl;
class WifiManagerImpl;

class NetworkManager
{
public:
    NetworkManager(onNetEvent cb, void *user);
    NetworkManager(onJsonEvent cb, void *user);
    ~NetworkManager();

    static bool checkWifiRequirement();

    int getStatus();

    bool setNetworkEnable(int network_type, bool enable);
    bool getNetworkEnable(int network_type);
    int setIPInfo(const string& interface, IPInfo& info);
    int setDns(const string& dns, const string& dns_back);

    bool checkWiredNetworkPluged();

    int setWiredAuthConfig(const WiredAuthConfig& config);
    int cancelWiredAuth();
    int  getWiredAuthStatus();
    int getWired1xAuthExist();

    int setOptionCb(int type, onOption cb);

    string getMac(int network_type);

    bool getIPInfo(IPInfo& info);

    int setPortConvert(const string& new_dip, const string& new_dport, const string& origin_dip,
        const string& origin_dport, const string& proto, int add);


    void doJsonCommand(const string& in_json, string& out_json);

    bool getNet2Status();

private:
    std::unique_ptr<NetworkManagerImpl> mNMI;

    NetworkManager(const NetworkManager& );
    NetworkManager& operator=(const NetworkManager &);
};

typedef void (*onWlanEvent)(int event, void *user, void* info);

class WifiManager
{
public:
    WifiManager(onWlanEvent cb, void* user);
    WifiManager(onJsonEvent cb, void* user);
    ~WifiManager();

    bool scan(list<ScanResult>& scan_result);
    bool getScanResult(list<ScanResult>& scan_result);
    bool setScanTerminal(const  unsigned int& scan_time);
    bool setRoamMode(const int& roam_mode);
    bool setRoamSensitivity(const string &level);
    int  getSavedList(list<WifiConfig>& saved_list);
    bool getConnectInfo(WifiInfo& info);
    int  getStatus();

    int  checkNetworkSaved(const string& ssid, const string& ssid_mac, int proto);

    bool connect(int net_id);
    bool connect(WifiConfig& config);
    bool disconnect();
    bool forgetNetwork(int net_id);

    void doJsonCommand(const string& in_json, string& out_json);

private:
    std::unique_ptr<WifiManagerImpl> mWMI;

    WifiManager(const WifiManager& );
    WifiManager& operator=(const WifiManager &);
};
}
#endif

