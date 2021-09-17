#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <map>
#include <vector>
#include "note.h"
#include "common.h"
#include "process_loop.h"
#include "mina.h"
#include "VM.h"
#include "application_data.h"
#include "status_machine.h"
#include "localmode.h"
#include "newdeploy_manage.h"
#include "wifi_interactive.h"
#include "auth_manager.h"
#include "device_interface.h"
#include "user_mgr.h"
#include "ui_api.h"
#include "cJSON.h"
#include <fstream>

#if 0
#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"

#ifdef __cplusplus
}
#endif
#endif

enum UpdataType
{
	POLICY_UPDATA,
	//DEFAULT_UPDATA,
	FORCE_UPDATA,
};

enum Resync
{
    RESYNC_ACTION_UNBIND        = 0,
    RESYNC_ACTION_BIND          = 1,
    RESYNC_ACTION_CHANGE_MODE   = 2,
};

enum VM_START_MODES
{
	VM_SPECIAL_MODE = 0,	
	VM_MULTIPLE_MODE,	//non binding user or multiple user 
	VM_PUBLIC_MODE,
	VM_GUEST_MODE,
};

enum ErrorTips
{
	IMAGE_SYNC_ERROR_TIPS = 1,
	INIT_SYNC_ERROR_TIPS = 2,
	IMAGE_RECOVERY_TIPS = 4,
	SERVER_ERROR_TIPS = 5,
	UPGRADE_ERROR_TIPS = 9,
};

enum ShutdownFlag
{
    TERMINAL_SHUTDOWN = 1,
    TERMINAL_REBOOT = 2,
    TERMINAL_LOGOUT = 3,
};

enum ServerType
{
    TYPE_UNKNOW_SERVER,
    TYPE_NEW_RCO_SERVER,
    TYPE_NEW_RCC_SERVER,
    TYPE_OLD_RCO_SERVER,
    TYPE_OLD_RCC_SERVER,
};

enum ClientType
{
    TYPE_UNKNOW_CLIENT,
    TYPE_NEW_CLIENT,       //mutual client
    TYPE_OLD_RCO_CLIENT,   //rco exclusive client
    TYPE_OLD_RCC_CLIENT,   //rcc exclusive client
};

enum IpxeAckType
{
    IPXE_ACK_NONE = 0,
    IPXE_ACK_WIFI_ENV = 7,
    IPXE_ACK_DOT1X_ENV = 13,
};

enum EventType
{
    //init event
    L2L_INIT_SELF = 0,
    L2L_EASY_DEPLOY,
    L2L_ENTER_NEW_DEPLOY,
    L2L_ENTER_RUNNING_VM,
    L2L_LINK_UP,
    L2L_LINK_DOWN,
    L2L_LINK1_UP,
    L2L_LINK1_DOWN,
    L2L_DATA_PROTECT,
    L2L_DOWNLOAD_SUCCESS_TIPS,

    /***UI***/
    U2L_PASS_NEW_DEPLOY_MODEINFO,

    U2L_SET_NET_INFO,
    U2L_SET_VM_NET_INFO,
    U2L_SET_SERVER_IP,
    U2L_SET_HOST_NAME,
    U2L_SET_WIFI_STATUS,
    U2L_SET_TERMINAL_MODE,
    U2L_SET_AUTH_INFO,
    U2L_LOCALMODE_QUIT,
    U2L_LOCALMODE_CHECKERROR,
    U2L_ACCOUNT_LOGIN,
    U2L_PUBLIC_LOGIN,
    U2L_GUEST_LOGIN,
    U2L_BIND_USER,
    U2L_MODIFY_PASSWORD,
    U2L_SHUTDOWN_TERMINAL,
    U2L_REBOOT_TERMINAL,
    U2L_ESTABLISH_CONNECTION,
    U2L_CANCEL_CONNECTION,
    U2L_ENTER_LOCAL_MODE,
    U2L_ENTER_SETTINGS,
    U2L_ENTER_WIFI_CONFIG,
    U2L_ENTER_AUTH_CONFIG,
    U2L_NOTIFY_SETTING_RESULT,
    U2L_NOTIFY_WIFI_CONFIG_RESULT,
    U2L_NOTIFY_AUTH_CONFIG_RESULT,
    U2L_SAVE_POWER_BOOT,
    U2L_SAVE_BOOT_SPEEDUP,
    U2L_USB_COPY_BASE,
    U2L_UPGRADE_FOR_CLASS,

    //events from logic to UI
    /***UI_END***/


    /***WEB***/
    W2L_MINA_CONNECTION_ESTABLISHED,
    W2L_MINA_CONNECTION_DESTROYED,

    W2L_MINA_READY,

    /* UPDATE PROCESS */
    W2L_UPGRADE_FOR_OFFICE,
    W2L_UPGRADE_FOR_CLASS,
    W2L_SYNC_DEB_PATH,
    W2L_SYNC_SOFTWARE_VERSION,
    W2L_SYNC_TERMINAL_PSW,
    W2L_SYNC_PUBLIC_POLICY,
    W2L_SYNC_MODE,
    W2L_SYNC_HOSTNAME,
    W2L_SYNC_PRINTER_SWITCH,
    W2L_SYNC_DESKTOP_REDIR,
    W2L_SYNC_IMAGE,
    W2L_SYNC_IPXE,
    W2L_SYNC_RECOVER_IMAGE,
    W2L_SYNC_RESET_TERMINAL,
    W2L_SYNC_SERVER_IP,
    W2L_SYNC_ALL_USERINFO,
    W2L_SYNC_SERVER_TIME,
    W2L_SYNC_DEV_POLICY,
    W2L_GET_VM_DEV_INTERFACE_INFO,
    W2L_SYNC_DRIVER_INSTALL,
    //W2L_SYNC_RELOAD_IMAGE,
    W2L_SYNC_PORT_MAPPING,
    W2L_SYNC_SSID_WHITELIST,
    W2L_SYNC_RCDUSBCONF_INFO,
    W2L_SYNC_DELETE_TEACHERDISK,
    W2L_SYNC_MAIN_WINDOW,
    W2L_INIT_SYNC_ERROR,
    W2L_IMAGE_SYNC_ERROR,

    W2L_LOGIN_SUCCESS,
    W2L_LOGIN_FAIL,
    W2L_BIND_SECCESS,
    W2L_BIND_FAIL,

    W2L_NOTIFY_CHECK_VM_STATUS,
    W2L_NOTIFY_SHUTDOWN,
    W2L_NOTIFY_REBOOT,
    W2L_NOTIFY_RECORVER_IMAGE,
    W2L_NOTIFY_COLLECT_LOG,
    W2L_NOTIFY_MODIFY_LOCAL_NETWORK,
    W2L_NOTIFY_MODIFY_VM_NETWORK,
    W2L_NOTIFY_MODIFY_HOSTNAME,
    W2L_NOTIFY_RESET_TO_INITIAL,
    W2L_NOTIFY_RESYNC,
    W2L_NOTIFY_DO_IPXE,
    //W2L_NOTIFY_RELOAD_IMAGE,
    W2L_NODIFY_SSID_WHITELIST,
    W2L_NODIFY_RESET_NETDISK,
    W2L_NOTIFY_DELETE_TEACHERDISK,

    /* idv over wan */
    W2L_NOTIFY_HTTP_PORT,

    W2L_NOTIFY_MODIFY_PASSWORD,
    W2L_MODIFY_PASSWORD_SUCCESS,
    W2L_MODIFY_PASSWORD_FAIL,

    W2L_RECV_GUESTTOOL_MSG,
    /***WEB_END***/

    //events from logic to web
    //L2W_SET_SERVER_IP,
    //L2W_SET_VM_NET,
    //L2W_SET_HOST_NAME,
    //L2W_CHECK_LOGIN,
    //L2W_BIND_USER,
    //L2W_NOTIFY_VM_START,
    //L2W_NOTIFY_VM_SHUTDOWN,
    //L2W_NOTIFY_VM_REBOOT,
    //L2W_DOWNLOAD_PROGRESS,


    //zhf
    //v2l api for vm
    //events from VM to logic
    V2L_IMAGE_EXIST_RESULT,
    V2L_VM_START_RESULT,
    V2L_VM_SHUTDOWN_RESULT,
    V2L_VM_REBOOT_RESULT,
    V2L_VM_DOWNLOAD_PROGRESS_STATUS,
    V2L_VM_DOWNLOAD_QUIT_RESULT,
    V2L_GET_VM_INFO_RESULT,
    V2L_SET_VM_NET_POLICY_RESULT,   //FIXME: unuseful event
    V2L_SET_VM_USB_POLICY_RESULT,   //FIXME: unuseful event
    V2L_SET_VM_DISKINFO_RESULT,     //FIXME: unuseful event
    V2L_SET_VM_NETINFO_RESULT,      //FIXME: unuseful event
    V2L_GET_VM_NETINFO_RESULT,      //FIXME: unuseful event

    V2L_SET_LOCAL_WIRED_NETWORK,

    //events from logic to VM
    //L2V_START_DOWNLOAD,
    //L2V_CANCEL_DOWNLOAD,
    //L2V_DELETE_IMAGE,
    //L2V_DELETE_DIFF,
    //L2V_DELETE_DATA,
    //L2V_RECOVER_IMAGE,
    //L2V_START_VM,
    //L2V_SHUTDOWN_VM,
    //L2V_REBOOT_VM,
    //L2V_SET_VM_NET, policy

    EVENT_TYPE_MAX,
};

enum UISyncFlag
{
	UI_HOSTNAME_SYNC = 1<<0,
	UI_SERVER_IP_SYNC = 1<<1,
	UI_NET_INFO_SYNC = 1<<2,
	UI_VM_NET_INFO_SYNC = 1<<3,
	UI_MODE_SYNC = 1<<4,
	UI_WIFI_STATUS_SYNC = 1<<5,
};

enum UISettingErr
{
	UI_HOSTNAME_ERR = 1<<0,
	UI_SERVER_IP_ERR = 1<<1,
	UI_NET_INFO_ERR = 1<<2,
	UI_VM_NET_INFO_ERR = 1<<3,
	UI_MODE_ERR = 1<<4,
	UI_WIFI_ERR = 1<<5,
	UI_AUTH_ERR = 1<<6,//��?
};

enum UISettingTip
{
    UI_SETTING_OK = 0,
    UI_SETTING_FAIL = 1,
    UI_SETTING_FAIL_REBOOT = 2,
    UI_SETTING_WLAN_SAVE_IP = 3,
    UI_SETTING_AUTH_FAIL = 4,
    UI_SETTING_AUTH_TIMEOUT = 5,
};

#define CLIENT_ALLOW_IMAGE_SYNC_FLAG_FILE "/tmp/rcc_client_allow_download"

class ReDownloadTimer;
class ConfigEventMgr;
class Application;
class LogicEvent;
class LogicStatusMachine;
class LocalMode;
class VM;
class NewDeployManage;
class DeviceInterface;

#if 0
struct StateTable
{
	LogicState cur_state;
	EventType event_type;
	int (Application::*func) (LogicEvent*, LogicState);
	LogicState next_state;
	struct list_head list;
};
#endif

enum LogicStateType
{
    STATUS_NONE = 0,
	STATUS_INITING = 1,
    STATUS_CHECKING_LOCAL,
    STATUS_WAITING_LOGIN,
    STATUS_CHECKING_LOGIN,
    STATUS_PREPARING_IMAGE,
    STATUS_DOWNLOADING_IMAGE,
    STATUS_RUNNING_VM,
    STATUS_NEW_DEPLOY,
    STATUS_UNKNOWN,
};

enum LogicMachineEventType
{
	EVENT_WEB_INFO_SYNCHRONIZED = 1,
    EVENT_LOCAL_MODE_CHOOSEN,
    EVENT_LOGIN_BUTTON_ENTERED,
    EVENT_LOGIN_FAILED,
    EVENT_LOGIN_SUCCESS,
    EVENT_IMAGE_NEED_DOWNLOAD,
    EVENT_IMAGE_LATEST,
    EVENT_IMAGE_SYNC_ERROR,
    EVENT_DOWNLOAD_IMAGE_SUCCEESS,
    EVENT_DOWNLOAD_IMAGE_FAILED,
    EVENT_LOCAL_CHECK_SUCCESS,
    EVENT_LOCAL_CHECK_FAILED,
    EVENT_VM_RUNNING,   // VM has been running, occured only on recovering from exception.

    EVENT_ENTER_NEW_DEPLOY,
    EVENT_NEW_DEPLOY_SUCCESS,

    EVENT_DRIVER_INSTALL_RECEIVED,

    EVENT_INIT = 0x100,
    EVENT_WEB_DISCONNECT = 0x101,
};

class LogicStatusMachine: public StatusMachine<Application>
{
public:
    LogicStatusMachine(Application* app);
    virtual bool check_valid_event_type(int type);
	void reset_status();
    
protected:
    virtual int get_next_status(int event_type);

private:
    int get_initing_next_status(int event_type);
    int get_checking_local_next_status(int event_type);
    int get_waiting_login_next_status(int event_type);
    int get_checking_login_next_status(int event_type);
    int get_preparing_image_next_status(int event_type);
    int get_downloading_image_next_status(int event_type);
    int get_new_deploy_next_status(int event_type);
};

class LogicUISyncEvent: public SyncEvent {
public:
    LogicUISyncEvent(): SyncEvent(), _ui_started(false) {}
    virtual ~LogicUISyncEvent() {}
    virtual void do_response();
    bool ui_check_started() { return _ui_started; }

private:
    bool _ui_started;           //indicate that UI has started.
};

class VMUISyncEvent: public SyncEvent {
public:
    VMUISyncEvent(UserInfoMgr *usermgr): SyncEvent(), _ui_stopped(false)
    {
        _usermgr = usermgr;
    }
    virtual ~VMUISyncEvent() {}
    virtual void do_response();
    bool ui_check_stopped() { return _ui_stopped; }

private:
    bool _ui_stopped;           //indicate that UI has started.
    UserInfoMgr *_usermgr;
};

class UIWaitSyncEvent: public SyncEvent {
public:
    UIWaitSyncEvent(): SyncEvent(){}
    virtual ~UIWaitSyncEvent() {}
    virtual void do_response();
};

class UIPopEvent: public SyncEvent {
public:
    UIPopEvent(): SyncEvent()
    {
        _event_desc.clear();
    }
    UIPopEvent(string desc): SyncEvent()
    {
        _event_desc = desc;
        print_event_desc();
    }
    virtual ~UIPopEvent() {}
    virtual void do_response();
    void print_event_desc();
    void set_event_desc(string desc) { _event_desc = desc; }

private:
    string _event_desc;
};


class DevPolicyEvent: public SyncEvent {
public:
	DevPolicyEvent(): SyncEvent()
    {
        _event_desc = "dev policy get sync";
    }
	DevPolicyEvent(string desc): SyncEvent()
    {
        _event_desc = desc;
        print_event_desc();
    }
    virtual ~DevPolicyEvent() {}
    virtual void do_response();
    void print_event_desc();
    void set_event_desc(string desc) { _event_desc = desc; }
private:
    string _event_desc;
};


class Application
{
public:
    typedef int (Application::*Handler)(LogicEvent* event);
    typedef std::map<int, Handler> Handlers;
    typedef std::vector<UserInfo> UserInfos;
    typedef std::vector<string> WhiteList;
    typedef std::vector<PortMappingInfo> PortMappingInfos;
    typedef std::vector<MainWindowInfo> MainWindowInfos;

	void main(int argc, char* argv[]);
	int Req_PushEvent(Event *_event);
	static Application* get_application();
    static VM* get_vm();
    static WifiInteractive* get_wifi_interactive();
    static DeviceInterface* get_device_interface();
    static UserInfoMgr* get_UsrUserInfoMgr();
    static ModeInfoDB& get_mode_data();
    static AuthManager* get_auth_manager();
    static void uipop_sync(void *event);
    void handle_xserver_exited_on_vm_fail();
    void handle_install_driver_on_vm_fail();
    void handle_xserver_alive_on_vm_fail();
    void handle_on_vm_start_success();
    void handle_on_vm_stop_success();
	
    void config_net_auth_start(void* data);
    void config_net_auth_cancel(void* data);
    int config_sync_event_response(int event_id, void* data);
    ~Application();

private:
	Application();
    static Application* _application;

    //ApplicationGarbo:only to delete Application when the programe quit
    class ApplicationGarbo   
    {
    public:  
        ~ApplicationGarbo()  
        {  
            if(Application::_application)  
                delete Application::_application;  
        }  
    };
    static Mutex _singleton_mutex;
    static ApplicationGarbo Garbo;

public:
    //logic interface
    int logic_init_self();
    int logic_link_up();
    int logic_link_down();
    int logic_link1_up();
    int logic_link1_down();
    int logic_reset_status_machine();
    int logic_download_success_tips();

    bool get_nat_policy();
    //web interface
    int web_ready(int default_mode);

    int web_upgrade_for_office(void);
    int web_upgrade_for_class(const UpgradeInfo& info);
    int web_sync_deb_path(void);

    int web_sync_software_version(const VersionInfo& info);
    int web_sync_public_policy(const PolicyInfo& info);
    int web_sync_hostname(const string& info);
    int web_sync_mode(const ModeInfo& info);
    int web_sync_printer_switch(const int& printer_switch);
    int web_sync_desktop_redir(const string& info);
    int web_sync_dev_policy(const DevPolicyInfo &info);
    int web_sync_dev_policy_complete();
    int web_sync_image(const ImageInfo& info);
    int web_sync_ipxe(const IpxeInfo& info);
    int web_sync_recover_image(int info);
    int web_sync_reset_terminal(int info);
    int web_sync_server_ip(int info);
    int web_sync_all_username(const UserInfos& info);
    int web_get_dev_interface_info(const DevInterfaceInfo& info);
    int web_sync_ssid_whitelist(const WhiteList& infos);
    int web_sync_rcdusbconf_info(const string &usbinfo);
    int web_sync_unknow_devinfo(const string &devinfo);
    int web_sync_port_mapping(const PortMappingInfos& infos);
    int web_sync_server_time(const string &server_time);
    int web_sync_driver_install(const string& msg);
    int web_sync_terminal_password(const string& pwd);
    int web_sync_main_window(const MainWindowInfos& info);
    //int web_sync_reload_image(int info);
    int web_sync_delete_teacherdisk(int info);
    int web_init_sync_error(const int error);
    int web_image_sync_error(const int error);

    int web_login_success(const UserInfo& info);
    int web_login_fail(const int error);
    int web_modify_password_success();
    int web_modify_password_fail(const int error);
	int web_bind_success(const UserInfo& info);
    int web_bind_fail(const int error);

    int web_notify_check_vm_status();
    int web_notify_shutdown();
    int web_notify_reboot();
    int web_notify_recorver_image();
    int web_notify_modify_password(const UserInfo& info);
    int web_notify_modify_local_network(const NetworkInfo& info);
    int web_notify_modify_vm_network(const NetworkInfo& info);
    int web_notify_collect_log(const FtpLogInfo& info);
    int web_notify_modify_hostname(const string& info);
    int web_notify_reset_to_initial();
    int web_notify_resync(int action);
    int web_notify_do_ipxe(const IpxeInfo& info);
    int web_notify_ssid_whitelist(const std::vector<string>& whitelist);
    int web_notify_reset_netdisk(const NetdiskInfo& info);
    int web_notify_do_port_mapping(int src_port, int dst_port)
    {
        unsigned int i = 0;
        bool is_contains_port = false;
        
        for(i = 0; i < _port_mapping_infos.size(); i++)
        {
            if (_port_mapping_infos[i].src_port == src_port) {
                _port_mapping_infos[i].dst_port = dst_port;
                is_contains_port = true;
            }
        }
        if (!is_contains_port) {
            PortMappingInfo info;
            info.src_port = src_port;
            info.dst_port = dst_port;
            _port_mapping_infos.push_back(info);
        }
        set_net_disk_port();
        return SUCCESS;
    }
    //int web_notify_reload_image();
    int web_notify_delete_teacherdisk();
    
    int web_notify_http_port(const HttpPortInfo &info);

    int web_recv_guesttool_msg(const string& info);

    int web_mina_connection_established();
    int web_mina_connection_destroyed();

    int web_show_ui_disconnect();
    int web_set_hide_guest_login(int hide_guest_login)
    {
        _hide_guest_login = hide_guest_login;
        return SUCCESS;
    }
    int web_do_ipxe_check_env();

	//ui interface
	int ui_get_basic_info(BasicInfo* info);
	int ui_get_net_info(NetworkInfo* info);
	int ui_get_vm_net_info(NetworkInfo* info);
	int ui_get_server_ip(char* ip);
	int ui_get_host_name(char* name);
	int ui_get_terminal_mode(int* mode, int* readonly );
	int ui_get_connection_info(int* connected);
    int ui_get_last_logined_user(struct UserInfo *user_info);
    int ui_get_bt_service(int* bt_service);
    int ui_get_power_boot(int* power_boot);
	int ui_get_boot_speedup(int* boot_speedup);
    int ui_is_new_deploy(int *is_new_deploy);
    int ui_is_wifi_terminal(int *wifi_terminal);

    int ui_pass_new_deploy_modeinfo(const ModeInfo& info);

    int ui_set_net_info(const NetworkInfo& info);
    int ui_set_vm_net_info(const NetworkInfo& info);
    int ui_set_server_ip(const string& ip);
    int ui_set_host_name(const string& name);
    int ui_localmode_quit(int result);
    int ui_localmode_checkerror(int result);
    int ui_account_login(const UserInfo& info);
    int ui_bind_user(int bind);
    int ui_set_terminal_mode(int mode);
    int ui_set_auth_info(const struct AuthInfo& info);
    int ui_set_wifi_mode(const struct wifi_switch_t& wifi_status);
    int ui_check_admin_passwd(char* password);
    int ui_shutdown_terminal();
    int ui_reboot_terminal();
    int ui_guest_login();
    int ui_public_login();
    int ui_establish_connection();
    int ui_destroy_connection();
    int ui_modify_passwd(const UserInfo& info);
    int ui_enter_local_mode();
	void ui_set_sync_flag(int flag){_ui_need_sync_flag = flag;};
	void ui_clean_all_set_flag(){_ui_need_sync_flag = 0; _ui_has_sync_flag = 0; _ui_setting_error = 0;};
	void ui_set_error_flag(int err) {_ui_setting_error = err;};
	void ui_show_next_picture();
    void ui_show_wifi_next_picture();
    void ui_show_auth_next_picture();
    void ui_cancel_auto_shutdown();
	int ui_show_retry_downloading();
	void ui_tips_ok(int type);
    int ui_enter_settings();
    int ui_enter_wifi_config();
    int ui_enter_auth_config();
    int ui_notify_setting_result();
    int ui_notify_wifi_config_result();
    int ui_notify_auth_config_result();
    void ui_show_login();
    int ui_save_bt_service(int bt_service);
    int ui_save_power_boot(const int power_boot);
	int ui_save_boot_speedup(const int boot_speedup);
    int ui_usb_copy_base();
    int ui_get_hide_guest_login()
    {
        return _hide_guest_login;
    }
    //void ui_read_auth_config(int *auth_type, string &username, string &password);
    void ui_read_auth_config(struct AuthInfo &info);
    void ui_save_auth_config(struct AuthInfo &auth_info);
    
    int ui_upgrade_for_class(void);

    int vm_upload_unknown_device();
    int vm_image_exist_status(int status);
    int vm_ui_thread_quit();
    int vm_start_vm_status(int status);
    int vm_shutdown_vm_status(int status);
    int vm_reboot_vm_status(int status);
    int vm_download_progress_status(struct DownloadInfo *download_info);
    int vm_download_quit_status(int status);
    void vm_usb_download_disconnect();
	int vm_get_vm_info_status(VMInfo* vminfo);
	int vm_set_net_policy_status(int status);
	int vm_set_usb_policy_status(int status);
	int vm_set_vm_netinfo_status(int status);
	int vm_get_vm_netinfo_status(NetworkInfo* networkInfo);
	int vm_set_vm_diskinfo_status(int status);
    int vm_get_basic_info(BasicInfo* info);
    int vm_get_sunny_info(bool *recovery, string *server_ip, string *logined_user);
    int vm_logout(string action);
    int vm_send_guesttool_info(const string &vm_msg, int source, int target, int module_id);
    int vm_stop_redownload_timer();

    int vm_set_local_wired_network_info(const NetworkInfo& info);
    void vm_change_shutdown_flag();
    int lm_web_update_onLineTime();

    const BasicInfo& get_basic_info()
    {
        return _basic_data.get();
    }

    const string& get_server_ip()
    {
        return _mina->mina_get_server_ip();
    }

    bool  get_server_connected()
    {
        return _mina->mina_get_connected();
    }

    inline LocalNetworkData& get_local_network_data()
    {
        return _local_network_data;
    }

    inline bool is_installing_driver()
    {
        return (!_driver_install_parament.empty());
    }

    inline const ModeInfo& get_mode_info()
    {
        return _mode_data.get();
    }
    
    inline const UserInfo& get_logined_user_info()
    {
        return _logined_user_info;
    }

    inline const string& get_hostname()
    {
        return _hostname_data.get_hostname_info();
    }

    static const string &logic_event_desc(int event);
    static const string &logic_state_desc(int state);

    bool is_localmode_startvm() { return _localmode_startvm; }
    struct PolicyInfo get_public_policy() { return _public_policy_info; }
    ReservedMemoryInfoDB& get_reserved_memory_data() { return _reserved_memory_data; }

    inline void wait_localnet()
    {
        Lock lock(_localnet_mutex);
        _localnet_condition.wait(lock);
    }
    
    inline void notify_localnet()
    {
        _localnet_condition.notify_all();
    }

    bool is_dev_network_unknown();
    bool is_dev_network_up();
    bool is_dev_eth_up();
    bool is_dev_wlan_up();
    bool is_dev_wired_dot1x_up();
    bool is_dev_network_down();
    bool is_wired_plugged();
    bool get_net2_status();

    const NetworkInfo& get_local_networkinfo()
    {
        return _local_network_data.get();
    }

    bool is_waiting_tips_ok()
    {
        return _wait_tips_ok;
    }
    
    void set_net_disk_port();

    void set_upgrade_server_info(UpgradeInfo info)
    {
        _upgradeInfo.versionInfo = info.versionInfo;
        _upgradeInfo.serverType  = info.serverType;
    }

    UpgradeInfo get_upgrade_server_info(void)
    {
        return _upgradeInfo;
    }

    bool upgrade_class_software_version(UpgradeInfo info);

private:
	void on_entering_initing();
    void on_entering_checking_local();
    void on_entering_waiting_login();
    void on_entering_checking_login();
    void on_entering_preparing_image();
    void on_entering_downloading_image();
    void on_entering_running_vm();
    void on_entering_new_device_deploy();

	void var_init();
    int ui_thread_pre_init();
	
	int delete_VM_all();
	int reset_terminal();
	
	//init & deinit functions
	//void addstate(LogicState curstate, EventType event, int (Application::*func)(LogicEvent*, LogicState), LogicState nextstate);
	//void CreateEvents();
	void CreateStateTable();
	//void DestroyEvents();
	void DestroyStateTable();
	void SetFuncArray();

	//event handle function
	int HandleEvent(LogicEvent* event);
	int l2l_init_self(LogicEvent* event);
    int l2l_enter_new_deploy(LogicEvent* event);
    int l2l_enter_running_vm(LogicEvent* event);
    int l2l_link_up(LogicEvent* event);
    int l2l_link_down(LogicEvent* event);
    int l2l_link1_up(LogicEvent* event);
    int l2l_link1_down(LogicEvent* event);
    int l2l_data_protect_process(LogicEvent* event);
    int l2l_download_success_tips(LogicEvent* event);

	//u2l events handles
	int u2l_pass_new_deploy_modeinfo(LogicEvent* event);

	int u2l_set_net_info(LogicEvent* event);
	int u2l_set_vm_net_info(LogicEvent* event);
	void clear_iptables_upon_ui_serverip_changed();
	int u2l_set_server_ip(LogicEvent* event);
	int u2l_set_host_name(LogicEvent* event);
	int _config_auth_cancel_handle();
    int _config_auth_success_handle();
    int u2l_set_auth_info(LogicEvent* event);
    int u2l_set_wifi_mode(LogicEvent* event);
	int u2l_localmode_loginauth(UserInfo* userinfo);
	int u2l_localmode_quit(LogicEvent* event);
	int u2l_localmode_checkerror(LogicEvent* event);
	int u2l_account_login(LogicEvent* event);
	int u2l_bind_user(LogicEvent* event);
	int u2l_set_terminal_mode(LogicEvent* event);
	//int u2l_check_admin_passwd(LogicEvent* event);
	int u2l_shutdown_terminal(LogicEvent* event);
	int u2l_reboot_terminal(LogicEvent* event);
	int u2l_guest_login(LogicEvent* event);
	int u2l_cancel_connection(LogicEvent* event);
    int u2l_establish_connection(LogicEvent* event);
	int u2l_public_login(LogicEvent* event);
	int u2l_modify_passwd(LogicEvent* event);
	int u2l_enter_local_mode(LogicEvent* event);
    int u2l_save_power_boot(LogicEvent* event);
    static void confirm_usb_copy_base(void* args);
    int u2l_usb_copy_base(LogicEvent* event);
	int u2l_enter_settings(LogicEvent* event);
	int u2l_enter_wifi_config(LogicEvent* event);
    int u2l_enter_auth_config(LogicEvent * event);
    int u2l_upgrade_for_class(LogicEvent* event);
	void l2u_show_setting_tip(int result);
	int l2u_notify_setting_result(LogicEvent* event);
    int l2u_notify_wifi_config_result(LogicEvent* event);
    int l2u_notify_auth_config_result(LogicEvent* event);
    void l2u_hotplug_show_public_login();

	void l2u_show_unconfig_wrap();
	void l2u_show_connecting_wrap();
	void l2u_show_disconnect_wrap();
	void l2u_show_updateclient_wrap();
	void l2u_result_updateclient_wrap(int result);
	void l2u_show_not_whitessid_show(vector<string> &whitelist);
	//w2l events handles
	int w2l_ready(LogicEvent* event);
    
	int w2l_mina_connection_established(LogicEvent* event);
	int w2l_mina_connection_destroyed(LogicEvent* event);

    /*
     *sync web information one by one
     */
    void sync_next_event(EventType current_event);
    bool __upgrade_check_product_id(const VersionInfo& sw_ver, const string& product_id);
    bool __upgrade_rcc_check_product_id(const VersionInfo& sw_ver, const string& product_id);
    int __upgrade_check_product_type(const string& product_id);
    bool __upgrade_rcc_check_os(const VersionInfo& sw_ver, const int os_version);

    int w2l_upgrade_for_office(LogicEvent* event);
    int w2l_upgrade_for_class(LogicEvent* event);
    int w2l_sync_deb_path(LogicEvent* event);
    int w2l_sync_software_version(LogicEvent* event);
    int w2l_sync_public_policy(LogicEvent* event);
    int w2l_sync_mode(LogicEvent* event);
    int w2l_sync_dev_policy(LogicEvent* event);
    int w2l_sync_hostname(LogicEvent* event);
    int w2l_sync_image(LogicEvent* event);
    int w2l_sync_ipxe(LogicEvent* event);
    int w2l_sync_recover_image(LogicEvent* event);
    int w2l_sync_reset_terminal(LogicEvent* event);
    int w2l_sync_server_ip(LogicEvent* event);
    int w2l_sync_all_userinfo(LogicEvent* event);
    int w2l_sync_ssid_whitelist(LogicEvent* event);
    int w2l_sync_terminal_password(LogicEvent* event);
    int w2l_sync_printer_switch(LogicEvent* event);
    int w2l_sync_desktop_redir(LogicEvent* event);
    int w2l_sync_unkown_devinfo(LogicEvent* event);
    int w2l_sync_rcdusbconf_info(LogicEvent* event);
    bool __mount_nfs(const string& remote_path, const string& local_path);
    int w2l_sync_server_time(LogicEvent * event);
    int w2l_sync_driver_install(LogicEvent* event);
    //int w2l_sync_reload_image(LogicEvent* event);
    int w2l_sync_delete_teacherdisk(LogicEvent* event);
    int w2l_sync_port_mapping(LogicEvent* event);
    int w2l_sync_main_window(LogicEvent* event);
    int w2l_init_sync_error(LogicEvent* event);
    int w2l_image_sync_error(LogicEvent* event);
    
    int w2l_login_success(LogicEvent* event);
    int w2l_login_fail(LogicEvent* event);
    int w2l_modify_password_success(LogicEvent* event);
    int w2l_modify_password_fail(LogicEvent* event);
    int w2l_bind_seccess(LogicEvent* event);
    int w2l_bind_fail(LogicEvent* event);

    int w2l_notify_check_vm_status(LogicEvent* event);
    int w2l_notify_shutdown(LogicEvent* event);
    int w2l_notify_reboot(LogicEvent* event);
    int w2l_notify_recorver_image(LogicEvent* event);
    int w2l_notify_modify_password(LogicEvent* event);
    int w2l_notify_modify_local_network(LogicEvent* event);
    int w2l_notify_modify_vm_network(LogicEvent* event);
    int w2l_notify_collect_log(LogicEvent* event);
    int w2l_notify_modify_hostname(LogicEvent* event);
    int w2l_notify_reset_to_initial(LogicEvent* event);
    int w2l_notify_resync(LogicEvent* event);
    int w2l_notify_do_ipxe(LogicEvent* event);
    //int w2l_notify_reload_image(LogicEvent* event);
    int w2l_notify_ssid_whitelist(LogicEvent* event);
    int w2l_notify_reset_netdisk(LogicEvent* event);
    int w2l_notify_delete_teacherdisk(LogicEvent* event);
    
    int w2l_notify_http_port(LogicEvent* event);

    int w2l_recv_guesttool_msg(LogicEvent* event);
    void __update_origin_picture(MainWindowInfo &info);

    //zhf
    //v2l api for vm
	//v2l events handles
    int v2l_image_exist_result(LogicEvent* event);
    void __upload_vm_info(bool is_vm_running);
	int v2l_vm_start_result(LogicEvent* event);
	int v2l_vm_shutdown_result(LogicEvent* event);
	int v2l_vm_reboot_result(LogicEvent* event);
	int v2l_vm_download_progress_status(LogicEvent* event);
	int v2l_vm_download_quit_result(LogicEvent* event);
    int v2l_get_vm_info_result(LogicEvent* event);
    int v2l_set_vm_net_policy_result(LogicEvent* event);
    int v2l_set_vm_usb_policy_result(LogicEvent* event);
    int w2l_get_vm_dev_interface_info(LogicEvent* event);
    int v2l_set_vm_netinfo_result(LogicEvent* event);
    int v2l_get_vm_netinfo_result(LogicEvent* event);
    int v2l_set_vm_diskinfo_result(LogicEvent* event);
    int v2l_set_local_wired_network_info(LogicEvent* event);
    int handle_easy_deploy(LogicEvent* event);

    static int set_local_network_callback(int callback_ret, void *data);
public:
    /**
     * handle_vendor_encapsulated_options and reset server ip
     */
    static void handle_vendor_encapsulated_options(const char* json_input, void* data);
     /*add for data convert to usb info */
    void create_usbfilter(string &usb_policy);
    void prepare_merge_image();
    void send_download_progress_info_to_zhjsgt(string downloaded_size,string total_size,string percent,string rate);
    void v2l_reboot_system_merge_image();
    void v2l_clean_download_success_tips();
private:
    /**
     * signal action for easy deploy
     */
    static void easy_deploy(int sig);
    static void* ui_thread(void* data);
    static void * ui_top_thread(void *data);
    /**
     * new thread for collect log
     */
    static void* collect_log(void* data);

    /**
     * new thread for download picture
     */
    static void* download_picture(void* data);
    static void* sync_main_window(void* data);

    /**
     * new thread for sync time
     */
    static void* sync_time_thread(void* data);

    /**
     * new thread to monitor system status changed.
     *
     * 1) Monitor device network status.
     *    if network down, web disconnected.
     *    if network up, web re-connected.
     *    if device network status changed, then
     *      VM network status will be changed too.
     */
    //static void *monitor_system_status(void *data);
    //int create_network_monitor_socket();
    //void network_status_request(int sd);
    //void network_status_process(int sd);

    bool pre_software_upgrade(const char* version, const char* pool);
    bool post_software_upgrade(const char* version, const char* pool);
    bool upgrade_software_package(const char* first_package_name, const char* package_name, const char* version, const char* pool);
    bool upgrade_software_version(char* data);
    void check_software_upgrade(const char* version);
    bool post_rcc_software_upgrade(const char* version, const char* pool);
    bool check_old_system_version(const char *os_version, int size);
    bool match_old_system_version(int serverType);
    bool copy_rcc_daemon_script(int serverType);
    int get_system_version(void);

#ifdef UNIT_TEST
	static void* unittest(void*);
#endif

	//varaibles
    BasicData _basic_data;
	LogicStatusMachine _status_machine;
    ProcessLoop _process_loop;
    Mina* _mina;
	VM* _vm;
    LocalMode* localmode;
    NewDeployManage* _newdeploy_manage;
    bool _localmode_startvm;
    bool _on_easy_deploy;
    WifiInteractive* _wifi_interactive;
    DeviceInterface* _device_interface;
	AuthManager* _net_auth;
    
    
	ModeInfoDB _mode_data;			// bind user info		
    DevpolicyInfoDB _dev_policy;
    DevstatusInfoDB _dev_status;
    OsupdataPolicyInfoDB _updata_policy;
    UserInfo _logined_user_info;	//current logined userinfo
    UserInfo _input_user_info;		//user input userinfo, need check 
    PolicyInfo _public_policy_info;
    VersioninfoDB _version_data;
    string _guesttool_version;
    
    HostnameInfoDB _hostname_data;
    LocalNetworkData _local_network_data;
    ReservedMemoryInfoDB _reserved_memory_data;
    PortMappingInfos _port_mapping_infos;
    volatile bool _ui_locking;           //true indicate can not shouw other UI now
    int _ui_has_sync_flag;		//one bit indicate one of info has set
    int _ui_need_sync_flag;		//one bit indicate one of info need set
    int _ui_setting_error;		//one bit indicate one of info error
    int _ui_setting_tip;        //show setting tips
    bool _download_retry;
    l2u_download_t l2u_download_info;
    int _default_mode;//to save default mode from web, and use it after we sync ipxe
    bool _allow_newdeploy;
    int _vm_start_mode;
    int _logic_state; 
    bool _logined;			//user has logined
    string _admin_passwd;
    Handlers _event_func;
    ImageInfo _server_image_info;
    int _bt_service;
    int _power_boot;
	int _boot_speedup;
    int _hide_guest_login;
    UpgradeInfo _upgradeInfo;//storage for upgrade to class
    /**
     *if we change the server ip, we should upload the hostname and mode to the new server
     */
    bool _configure_serverip_sync_flag;
    string _configure_hostname;
    int _configure_mode;
    
	UserInfoMgr _userinfomgr;
    LogicUISyncEvent _uisync_event;
    VMUISyncEvent _vmsync_event;
    UIWaitSyncEvent _ui_waitsync_event; // sync destroy win main bg then quit ui thread
    ReDownloadTimer* _redownload_timer;
    ConfigEventMgr* _config_event_mgr;

    // TODO: FIXME this variable is difficult to handle
	bool _error_tips;

    Mutex _localnet_mutex;
    Condition _localnet_condition;
    int _shutdown_flag;	//0:unconfigured, 1:shutdown, 2:reboot
    bool _new_deploy_processing;
    bool _init_sync_processing;
    bool _wait_tips_ok; //indicate shoud wait for last web cmd(bind, unbind, mode change, reset) complete
    pthread_t _dhcp_option_threadid;
    string _driver_install_parament;
    int _argc;
    char **_argv;

    friend class LogicEvent;

    /********
    FIXME: delete friend class after we can get mode_data and other useful variable
    via public method
    ********/
    friend class LogicStatusMachine;
    friend class LocalMode;
    friend class ReDownloadTimer;
    friend class NewDeployManage;
    friend class LocalNetworkData;
    friend class DeviceInterface;
};

class LogicEvent:public Event
{
public:
    LogicEvent(Application* app, EventType type, void* data, int size)
        :_app (app)
        ,_type (type)
        ,_data(data)
        ,_size(size)
        ,_error(SUCCESS)
    {
    	
    }
    
    virtual int pre_push()
    {
        if(_app->_status_machine.get_status() == STATUS_NONE && _type == L2L_INIT_SELF)
            return SUCCESS;
        int ret = get_error();
#ifdef UNIT_TEST
        return SUCCESS;
#else
        return ret;
#endif /* UNIT_TEST */
    }

    virtual int pre_response()
    {
        if(_app->_status_machine.get_status() == STATUS_NONE && _type == L2L_INIT_SELF)
        {
            _error = SUCCESS;
            return SUCCESS;
        }
    	_error = get_error();
        return SUCCESS;
    }

    virtual bool erase_check(int event_id)
    {
        if (_type == event_id)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

	void response()
	{
		_app->HandleEvent(this);
	}
	void* getusrdata(){
		return _data;
	}
	int getusrdatasize(){
		return _size;
	}
protected:
    Application* _app;

private:

    int get_error()
    {
        if(_app->_status_machine.check_valid_event_type(_type))
        {
    	    return check_extra_valid_event_type();
        }
        else
        {
            switch(_app->_status_machine.get_status())
            {
                case STATUS_INITING:
                    return ERROR_STATUS_INITING;
                case STATUS_CHECKING_LOCAL:
                    return ERROR_STATUS_CHECKING_LOCAL;
                case STATUS_WAITING_LOGIN:
                    return ERROR_STATUS_WAITING_LOGIN;
                case STATUS_CHECKING_LOGIN:
                    return ERROR_STATUS_CHECKING_LOGIN;
                case STATUS_PREPARING_IMAGE:
                    return ERROR_STATUS_PREPARING_IMAGE;
                case STATUS_DOWNLOADING_IMAGE:
                    return ERROR_STATUS_DOWNLOADING_IMAGE;
                case STATUS_RUNNING_VM:
                    return ERROR_STATUS_RUNNING_VM;
                case STATUS_NEW_DEPLOY:
                    return ERROR_STATUS_NEW_DEPLOY;
                default:
                    return -1;
            }
        }
    }

    int check_extra_valid_event_type()
    {
    
        int ret = SUCCESS;

        //step1: check ERROR_STATUS_WAITING_LOGIN
        switch(_type)
        {
            case W2L_NOTIFY_RECORVER_IMAGE:
            //case W2L_NOTIFY_RELOAD_IMAGE:
            case W2L_NOTIFY_DELETE_TEACHERDISK:
            case W2L_NOTIFY_MODIFY_LOCAL_NETWORK:
            case W2L_NOTIFY_MODIFY_VM_NETWORK:
            case W2L_NOTIFY_MODIFY_HOSTNAME:
            case W2L_NOTIFY_RESET_TO_INITIAL:
            case W2L_NOTIFY_RESYNC:
            //case W2L_NOTIFY_DO_IPXE:
            case W2L_NOTIFY_MODIFY_PASSWORD:
            {
                if(_app->is_waiting_tips_ok())
                {
                    return ERROR_WAITING_CONFIRM;
                }
                else
                {
                    ret = SUCCESS;
                }
                break;
            }
            default:
            {
                ret = SUCCESS;
            }
        }

        //step2: like step1

        return ret;
    }
    
	EventType _type;
	void* _data;
	int _size;
    int _error;
    //EventType _type;
    friend class Application;
};


/*****************
*repeat trying download every 30s if download failed
******************/
class ReDownloadTimer:public Timer
{
public:
    ReDownloadTimer(Application* app):_app (app) {}
    virtual ~ReDownloadTimer() {}
    virtual void response()
    {
        _app->_download_retry = true;
        _app->_process_loop.deactivate_interval_timer(this);
        int status = _app->_status_machine.get_status();
        if(status == STATUS_DOWNLOADING_IMAGE)
        {
            _app->_status_machine.change_status(EVENT_DOWNLOAD_IMAGE_FAILED);
        }
        else if(status == STATUS_NEW_DEPLOY)
        {
            _app->logic_reset_status_machine();
        }
    }
private:
    Application* _app;
};

enum ConfigSyncEventId {
    CONFIG_START_AUTH,
    CONFIG_CANCEL_AUTH,
};

const static string ConfigEventDesc[] = {
    "config start auth",
    "config cancel auth",
};

class ConfigSyncEvent: public SyncEvent {
public:
    ConfigSyncEvent(): SyncEvent(), _event_id(0) {}
    ConfigSyncEvent(int id): SyncEvent(), _event_id(id) {}
    virtual ~ConfigSyncEvent() {}
    virtual void do_response();
    int get_id() { return _event_id; }
    void set_id(int id) { _event_id = id; }
private:
    int _event_id;
};

class ConfigEventMgr {    
    typedef void (Application::*Handler)(void* data);
    typedef std::map<int, Handler> Handlers;
    typedef std::map<int, ConfigSyncEvent> ConfigEvents;
    typedef std::map<int, ConfigSyncEvent>::iterator ConfigEventsIter;
public:
    ConfigEventMgr(Application* app)
        :_app(app)
    {
        set_handler(CONFIG_START_AUTH, &Application::config_net_auth_start);
        set_handler(CONFIG_CANCEL_AUTH, &Application::config_net_auth_cancel);
    }
    virtual ~ConfigEventMgr() {}
    bool wait_timeout(int id, unsigned long long nano)
    {
        //create sync event
        ConfigSyncEvent event(id);
        //_config_events.insert(std::pair<int, ConfigSyncEvent>(id, event));
        _config_events[id] = event;
    	return _config_events[id].wait_timeout(nano);
    }
    void response(int id, void* data)
    {        
        (_app->*_handlers[id])(data);
        if (event_exist(id)) {
            _config_events[id].response();
        }
        //erase sync event
        _config_events_iter = _config_events.find(id);
        if (_config_events_iter != _config_events.end()) {
            _config_events.erase(_config_events_iter);
        }
    }
    
private:
    void set_handler(int id, Handler handler)
    {
        _handlers[id] = handler;
    }
    bool event_exist(int id)
    {
        _config_events_iter = _config_events.find(id);
        if (_config_events_iter != _config_events.end()) {
            return true;
        }
        return false;
    }
    Application* _app;
    Handlers _handlers;
    ConfigEvents _config_events;
    ConfigEventsIter _config_events_iter;
};

#endif //_APPLICATION_H
