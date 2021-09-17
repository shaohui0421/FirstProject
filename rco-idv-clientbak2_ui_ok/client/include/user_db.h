/**
 * Copyright(C) 2017 Ruijie Network Inc. All rights reserved.
 *
 * user_db.h
 * Original Author: yejx@ruijie.com.cn, 2017-2-19
 *
 * User Info Database operations.
 */
#ifndef _USER_DB_H_
#define _USER_DB_H_

#include <iniparser/dictionary.h>
#include "data_struct.h"
#include "common.h"
#include <vector>
#include "event.h"


using std::vector;

class DevPolicyEvent;


#define USER_INFO_INI               RCC_DATA_PATH "user_info.ini"
#define USER_NETDISK_INI            RCC_DATA_PATH "user_netdisk.ini"
#define USER_GROUP_INI              RCC_DATA_PATH "group_info.ini"
#define USER_AD_DOMAIN_INI          RCC_DATA_PATH "user_ad_domain.ini"
#define DEV_INTERFACE_INI           RCC_DATA_PATH "dev_interface.ini"

#define USER_STATUS_INI             "/tmp/user_status.ini"
#define USER_LAST_LOGINED_INI       RCC_DATA_PATH "last_logined_user.ini"
#define USER_SSID_WHITE_LIST        RCC_DATA_PATH "ssid_white_list.ini"

#define RCC_VMMODE_INI              RCC_SYS_PATH "vmmode.ini"
#define USER_DISPLAY_INFO_INI       RCC_DATA_PATH "displayinfo.ini"
#define USER_HDMI_AUDIO_INI         RCC_DATA_PATH "hdmi_audio_info.ini"
#define DISKINFO_INI                RCC_DATA_PATH "diskinfo.ini"
#define SHORT_KEY_INI               RCC_DATA_PATH "short_key.ini"
#define USER_OTHER_CONFIG_INI       RCC_DATA_PATH "other_config.ini"

#define HTTP_PORT_MAP_INI           RCC_DATA_PATH "http_port_map.ini"

class UserDB {
public:
    UserDB(const char *inifile);
    virtual ~UserDB() {}
    int saveUserDB(void);
    //virtual int readUserInfo(struct UserInfo *user_info) = 0;
    //virtual int storeUserInfo(Mode mode, struct UserInfo &user_info) = 0;

protected:
    int UserDBcheckError() { return _occured_error; }
    void load(void);
    void unload(void);

    const char *getEntry(const char *section, const char *key, const char *def);
    int getEntry(const char *section, const char *key, int def);
    void setEntry(const char *section, const char *key, const char *value);
    void setEntry(const char *section, const char *key, int value);
    void deleteEntry(const char *section, const char *key);
    bool findEntry(const char *section, const char *key);
    void deleteSection(const char *section);
    bool findSection(const char *section);
    const char **getSections_withEntry(const char *key, const char *value, int *sec_count);
    const char **getSections_withEntry(const char *key, int value, int *sec_count);
    const char **getAllSections(int *sec_count);

private:
    const char *_inifile;
    dictionary *_ini;
    int _occured_error;
    bool _changed;
    const char *_get_entry(const char *section, const char *key, const char *def);
    int _get_entry(const char *section, const char *key, int def);
    bool _check_if_entry_changed(const char *section, const char *key, const char *value);
    bool _check_if_entry_changed(const char *section, const char *key, int value);
    int _add_new_section(const char *section);
};

class UserInfoDB: public UserDB {
public:
    UserInfoDB(): UserDB(USER_INFO_INI) { load(); }
    virtual ~UserInfoDB() { unload(); }
    int userInfoInit();
    int readUserInfo(struct UserInfo *user_info);
    int readUserInforUsbPolicy(string &username, struct PolicyInfo *policyinfo);
    int storeUserInfo(struct UserInfo &user_info);
    int deleteUserInfo(string &username);
    string *getUsers_withGroup(int &group_id, int *user_count, int *error);
    int getAllUsers(vector<string> &users);
    int convertToShortName(string &full_name);
private:
    bool _adname_match_rule(const string &full_name, const string &user_name, const string &domain);
};

class UserNetdiskDB: public UserDB {
public:
    UserNetdiskDB(): UserDB(USER_NETDISK_INI) { load(); }
    virtual ~UserNetdiskDB() { unload(); }
    int readUserInfo(struct UserInfo *user_info);
    int storeUserInfo(struct UserInfo &user_info);
    int deleteUserInfo(string &username);
};

class DevInterfaceDB: public UserDB {
public:
    DevInterfaceDB(): UserDB(DEV_INTERFACE_INI) { load(); }
    virtual ~DevInterfaceDB() { unload(); }
    int readDevInterfaceInfo(DevInterfaceInfo &inter_info);
    int storeDevInterfaceInfo(const DevInterfaceInfo &inter_info);
};

class GroupInfoDB: public UserDB {
public:
    GroupInfoDB(): UserDB(USER_GROUP_INI) { load(); }
    virtual ~GroupInfoDB() { unload(); }
    int readUserInfo(struct UserInfo *user_info);
    int storeUserInfo(struct UserInfo &user_info);
    int readPublicPolicy(struct PolicyInfo *public_policy);
    int storePublicPolicy(struct PolicyInfo &public_policy);
    int deleteUserGroup(int &group_id);
    int deletePublicGroup();
    int getAllGroups(vector<int> &groups);

private:
    void _groupPolicyLoad(const char *section, struct PolicyInfo *group_policy);
    void _groupPolicyStore(const char *section, struct PolicyInfo &group_policy);
};

// add for logined user saved & state machine reset.
class UserStatusDB: public UserDB {
public:
    UserStatusDB(): UserDB(USER_STATUS_INI) { load(); }
    virtual ~UserStatusDB() { unload(); }
    string getLoginedUser();
    void setLoginedUser(string &username);
    bool getUserLoginedStatus();
    void setUserLoginedStatus(bool logined);
    int getSavedState();
    void setSavedState(int state);
    bool getUIPermitted();
    void setUIPermitted(bool permitted);
    bool get_localmode_flag();
    void set_localmode_flag(bool flag);
    int delete_user_status_ini();
};

// add for user remember function.
class UserLastLoginedDB: public UserDB {
public:
    UserLastLoginedDB(): UserDB(USER_LAST_LOGINED_INI) { load(); }
    virtual ~UserLastLoginedDB() { unload(); }
    void getLoginedUser(string *username, string *password, int *remember_flag);
    void setLoginedUser(string &username, string &password, int &remember_flag);
};

//add for ssid whitelist function
class SsidWhiteListDB: public UserDB {
public:
    SsidWhiteListDB(): UserDB(USER_SSID_WHITE_LIST) { load(); }
    virtual ~SsidWhiteListDB() { unload(); }
    int readssidWhiteUser(vector<string> &whitelist);
    void storessidWhiteUser(vector<string> &whitelist);
    int deletessidWhite();
};

//add for vmmode.ini
class VmmodeInfoDB:public UserDB {
public:
    VmmodeInfoDB():UserDB(RCC_VMMODE_INI) { load(); }
    virtual ~VmmodeInfoDB() { unload(); }

    bool checkvmmode_section(const string &section);
    bool checkvmmode_entry(const string &section, const string &entry);
    int getvmmode_value(const string &section, const string &entry, string &value);
};

class DisplayinfoDB: public UserDB {
public:
    DisplayinfoDB(): UserDB(USER_DISPLAY_INFO_INI) { load (); }
    virtual ~DisplayinfoDB() { unload(); }
    void set_display_info(const struct DisplayInfo &dpi_info);
    void get_display_info(int *width, int *height, int *custom);
    void set_ext_display_info(const struct DisplayInfo &dpi_info, int num);
    void get_ext_display_info(ExtDisplayInfo *res_info);
    void get_ext_display_info(int port, string &width, string &height, int &custom);
    int delete_display_info_ini();
    int is_display_info_section_exist(const char* section);
    int delete_resolution_info_section();
};

//add for hdmi_audio.ini
class HdmiAudioinfoDB: public UserDB {
public:
    HdmiAudioinfoDB(): UserDB(USER_HDMI_AUDIO_INI) { load(); }
    virtual ~HdmiAudioinfoDB() { unload(); }
    void set_hdmi_audio_info(int hdmiaudio);
    int get_hdmi_audio_info();
    void delete_hdmi_audio_info();
};

class DiskInfoDB: public UserDB {
public:
    DiskInfoDB(): UserDB(DISKINFO_INI) { load (); }
    virtual ~DiskInfoDB() { unload(); }

    void set_disk_info(const DiskInfo_t &diskinfo);
    int get_disk_info(const string &disk_name, DiskInfo_t &diskinfo);
    int delete_diskinfo_ini();
};

// add for web & 802.1x authentication info.
class AuthDB: public UserDB {
public:
    AuthDB(): UserDB(RCC_AUTH_MGR_INI_FILENAME) { load(); }
    virtual ~AuthDB() { unload(); }
    int authIniInit();
    int getAuthInfo(int &auth_type, int &auto_connect, string &username, string &password);
    int setAuthConnect(int auto_connect);
    int setAuthInfo(struct AuthInfo &auth_info);
    int delete_auth_info();

};


//add for serverinfo.ini
class ServeripInfoDB:public UserDB {
public:
    ServeripInfoDB(): UserDB(RCC_SERVERIP_INI_FILENAME) { load(); }
    virtual ~ServeripInfoDB() { unload(); }
    void set_serverip_info(const string& _serverip);
    void set_last_serverip_info(const string& _lastserverip);
    const string& get_serverip_info();
    const string& get_last_serverip_info();

private:
    string _serverip;
    string _last_serverip;

};

//add for versioninfo.ini
class VersioninfoDB:public UserDB {
public:
    VersioninfoDB(): UserDB(RCC_VERSION_INI_FILENAME) { load(); }
    virtual ~VersioninfoDB() { unload();  }
    //void set_info_form_ini(); //the ini file already exit,this function store the the data in the file into the structure;
    const VersionInfo& get_version_info();

private:
    VersionInfo _version_info;

};

//add for modeinfo.ini
class ModeInfoDB:public UserDB
{
public:

    ModeInfoDB(): UserDB(RCC_LOGIC_INI_FILENAME) { load ();}
    virtual ~ModeInfoDB() { unload();}
    //void set_mode_info_init();
    void set(const ModeInfo& modeinfo);
    ModeInfo& get();

private:
    ModeInfo _mode_info;
    
};

//add for hostnameinfo.ini
class HostnameInfoDB:public UserDB
{
public:
    HostnameInfoDB(): UserDB(RCC_LOGIC_CONFIGURED_INI_FILENAME) { load (); }
    virtual ~HostnameInfoDB() { unload(); }
    void set_hostname_info(const string& hostname);
    const string& get_hostname_info();


private:
    string _hostname;
};

//add for RCC_Client_os_upgrade_rule.ini
class OsupdataPolicyInfoDB:public UserDB
{
public:
    enum UpdataType
    {
        POLICY_UPDATA,
    //DEFAULT_UPDATA,
        FORCE_UPDATA,
    };
    OsupdataPolicyInfoDB(): UserDB(RCC_OS_UPGRADE_POLICY_INI) { load(); set_Osupdatepllicy_inifo_init();};
    ~OsupdataPolicyInfoDB(){ unload(); }
    bool allow_version(const string& version_str);
    bool isNum(string str);
    bool str2version(string str, VersionInfo &info);
    void add_list(string &buf, vector<VersionInfo> &list);
    void get_version_list(vector<VersionInfo> &list, string kw);
    void show_version_info(vector<VersionInfo> &list);
    void set_Osupdatepllicy_inifo_init();

private:
    UpdataType	_type;
    char _section[40];
    vector<VersionInfo> _min_version;
    vector<VersionInfo> _white_list;
    vector<VersionInfo> _black_list;
};


//add for devpolicyinfo.ini

class DevpolicyInfoDB:public UserDB
{
public:
    DevpolicyInfoDB(): UserDB(RCC_DEV_POLICY_INI_FILENAME) { load (); set_devpolicy_info_init();}
    virtual ~DevpolicyInfoDB() { unload();  delete_update_sync();}
    void delete_update_sync();
    void set_devpolicy_info();
    void set_devpolicy_info_init();
    void update(const DevPolicyInfo& policy_info);
    const struct DevPolicyInfo& get();
    bool wait4update(unsigned long long nano);
    void update_finish();
    bool update_valid();
    bool check_recovery();
    bool allow_recovery();
    bool check_userdisk();
    int allow_userdisk();
    bool check_gusetlogin();
    bool allow_gusetlogin();
    bool check_otherslogin();
    bool allow_otherslogin();

private:
    bool _update;
    DevPolicyEvent *_update_sync;
    struct DevPolicyInfo _policy_info;

};

//add for dev_status.ini
class DevstatusInfoDB:public UserDB
{
public:
    DevstatusInfoDB():UserDB(RCC_DEV_STATUS_INI_FILENAME) { load();devstatus_info_init();}
    virtual ~DevstatusInfoDB() { unload();}
    void devstatus_info_init();
    void DevLock(bool lock);   
    const bool IsLocked();
    void set_mode_info(const ModeInfo& mode_info);
    ModeInfo get_mode_info();
    void set_dev_policy(const DevPolicyInfo& dev_info);
    DevPolicyInfo get_dev_policy();
    void set_lock_type(int lock_type);
    int get_lock_type();
private:
    bool     _locked;
    ModeInfo _mode_info;
    DevPolicyInfo _dev_info;
    int _lock_type;
};

//add for newdepolyctrl.ini no use
class ReservedMemoryInfoDB:public UserDB
{
public:
    ReservedMemoryInfoDB():UserDB(RCC_RESERVED_MEMORY_INI_FILENAME) { load();reserved_memory_info_init();}
    virtual ~ReservedMemoryInfoDB() { unload();}
    void reserved_memory_info_init();
    const int get_resvered_momory_info();
    void set_resvered_memory_info(int reserved_memory);

private:
        int _reserved_memory;

};

//add for newdeploy_ctrlinfoDB.ini
class NewDeployCtrlInfoDB:public UserDB
{
public:
    NewDeployCtrlInfoDB():UserDB(RCC_NEWDEPLOY_CTRL_INI_FILENAME) { load(); newdeploy_ctrl_info_init();}
    virtual ~NewDeployCtrlInfoDB() {unload();};
    void newdeploy_ctrl_info_init();
    bool get_new_terminal();
    void set_new_terminal(bool new_terminal);
    bool get_download_status();
    void set_download_status(bool download_status);

private:
    bool _new_terminal;
    bool _download_status;
};
#if 0
//add for rccusbfilterinfo.ini
class RccUsbFliterInfoDB:public UserDB
{
public:
    RccUsbFliterInfoDB():UserDB(RCC_USB_FLITER){ load();}
    ~RccUsbFliterInfoDB(){ unload(); }
    int set_rccusbfilter_info(const void *date);
    int get_rccusbfilter_info(const void *buf);

private:
    UsbConfigInfo *_usbinfo;
};

//add for rcdusbconfinfo.ini
class RcdUsbConfInfoDB:public UserDB
{
public:
    RcdUsbConfInfoDB():UserDB(RCC_USB_CONF) { load();}
    ~RcdUsbConfInfoDB(){ unload(); }
    void set_rcdusbconf_info(const char *date);
    int get_rcdusbconf_info(const void * buf);
};
#endif

//add for vmnerworkinfoDB.ini
class VmNetworkInfoDB:public UserDB
{
public:
    VmNetworkInfoDB(const char *file_path):UserDB(file_path){load();set_network_info_init();}
    VmNetworkInfoDB():UserDB(VM_NETWORK_INFO_INI_FILENAME){load();set_network_info_init();}
    virtual ~VmNetworkInfoDB(){ unload();}
    void set_vm_network(const NetworkInfo& network_info);
    NetworkInfo get_vm_network();
    void set_network_info_init();

    
private:
    NetworkInfo _network_info;
};

class ImageManage;

class ImageInfoDB:public UserDB
{
public :
    ImageInfoDB(ImageManage* image_manage):UserDB(VM_IMAGE_INFO_INI_FILENAME),_image_manage(image_manage) {load();set_imaginfo_init();}
    void set(const ImageInfo& image_info, bool downloading);
    void set_ostype(const string &ostype);
    void get_layerdisk_info(LayerDiskInfo &layer_info);
    int get_ostype(string &ostype);
    void set_download_status(bool downloading);
    void set_download_basename(string name);
    void set_download_mode(int mode);
    int  get_download_mode();
    void set_new_base_name(string name);
    string  get_new_base_name();
    void set_new_base_version(string version);
    string  get_new_base_version();
    void set_new_base_id(int id);
    int  get_new_base_id();
    bool is_silent_download();
    bool is_need_merge();
    ImageInfo get();
    bool get_download_status();
    void set_imaginfo_init();
    int delete_image_info();
    virtual ~ImageInfoDB(){ unload();}
private:
    ImageManage* _image_manage;
    ImageInfo _image_info;
    bool _download_status;

};

//add for vm_configured.ini
class VMConfigDB:public UserDB
{
public:
    VMConfigDB(): UserDB(VM_CONFIGURED_INI_FILENAME) { load(); }
    virtual ~VMConfigDB() { unload(); }
    void set_vm_desktop_redir(const string &redir_switch);
    void get_vm_desktop_redir(string &redir_switch);
};

//add for other_config.ini
class OtherConfigDB:public UserDB
{
public:
    OtherConfigDB(): UserDB(USER_OTHER_CONFIG_INI) { load(); }
    virtual ~OtherConfigDB() { unload(); }
    void set_e1000_netcard(const bool &netcard_switch);
    bool is_using_e1000_netcard(void);
    void set_usb_emulation(const bool &is_emulation);
    bool is_usb_emulation(void);
    void set_app_layer_switch(const int &status);
    int get_app_layer_switch();
};

class HttpPortMapDB:public UserDB
{
public:
    HttpPortMapDB(): UserDB(HTTP_PORT_MAP_INI) { load(); }
    virtual ~HttpPortMapDB() { unload(); }
    void set_http_port_map(const HttpPortInfo &info);
    void get_http_port_map(HttpPortInfo &info);
    void set_http_last_port_map(const HttpPortInfo &info);
    void get_http_last_port_map(HttpPortInfo &info);
};


#endif /* _USER_DB_H_ */

