#ifndef _MINA_H
#define _MINA_H

#include "message_handler.h"
#include "common.h"
#include "process_loop.h"
#include "message.h"
#include <map>
#include <vector>
#include "base64.h"
#include "user_db.h"

class Mina;
class Application;
typedef MessageHandler<Mina> MinaMessageHandler;

#pragma pack(push)
#pragma pack(1)
typedef struct msg_head_s {
    char        version[32];
    int         reserved;
    short       handle;
    int         length;
} msg_head_t;

typedef struct msg_sync_type_s {
    msg_head_t  head;
    short       action;
} msg_sync_type_t;

typedef struct msg_handle_type_s {
    msg_head_t  head;
    short       result;
    short       action;
    char        version[32];
} msg_handle_type_t;

typedef struct msg_sync_path_s {
    msg_head_t  head;
    short       action;
} msg_sync_path_t;

typedef struct msg_handle_path_s{
    msg_head_t  head;
    short       result;
    short       action;
    char        update_path[256];
} msg_handle_path_t;
#pragma pack(pop)

class Mina:public EventSource, public MinaMessageHandler
{
class KeepAliveTimer;
class SendMessageEvent;
class ConnectTimer;
class MinaCommonEvent;
class InfoTimer;
    enum Port { MINA_DEFAULT_PORT = 9109,};
    enum Interval
    {
        CONNECT_TIME_S = 2,
        CONNECT_INTERVAL = 5*1000,
        KEEPALIVE_INTERVAL = 10*1000,
        CONNECT_TIMEOUT_S = 10,
    };
    enum Length { VERSION_LENGTH = 32,};
    typedef std::map<int, InfoTimer*> InfoTimers;
public:
    Mina(Application* app, int port = Mina::MINA_DEFAULT_PORT);
    virtual ~Mina();

    int mina_check_web_alive(const string& server_ip);
    int mina_establish_connection();
    int mina_destroy_connection();
    bool mina_get_connected() {return _connected;}

    int mina_set_server_ip(const string& server_ip);
    int mina_save_server_ip(const string& server_ip);
    const string& mina_get_server_ip();
    int mina_set_last_server_ip(const string& last_server_ip);
    const string& mina_get_last_server_ip();
    
    int mina_web_request_software_version();
    int mina_web_request_terminal_password();
    int mina_web_request_public_policy();
    int mina_web_request_hostname(const string& hostname);
    int mina_web_request_mode(const ModeInfo& info);
    int mina_web_request_printer_switch();
    int mina_web_request_desktop_redir();
    int mina_web_request_image(const ImageInfo& info);
    int mina_web_request_ipxe();
    int mina_web_request_recover_image();
    int mina_web_request_reset_terminal();
    int mina_web_request_check_server_ip(const string& server_ip);
    int mina_web_request_all_userinfo(const std::vector<string>& usernames);
    int mina_web_request_dev_policy(const int mode, const string& bind_user);
    int mina_web_request_driver_install();
    int mina_web_request_reload_image();
    int mina_web_request_delete_teacherdisk();
    int mina_web_request_port_mapping();
    int mina_web_request_ssid_whitelist();
    int mina_web_request_server_time();
    int mina_web_request_rcdusbconf();
    int mina_web_request_main_window();
    int mina_web_upload_unknow_devinfo(const struct Unknown_UsbInfo &undev_info);

    int mina_web_check_server_type(void);
    int mina_web_request_deb_path(void);
    int mina_web_upload_basic_info(const BasicInfo& info, const NetworkInfo& net, const string& wireless_mac, const string& wired_mac, int net_type, const string& hostname,const SystemHardwareInfo& hardInfo);
    int mina_web_upload_local_network_info(const NetworkInfo& info);
    int mina_web_upload_vm_network_info(const NetworkInfo& info, const string& vm_mac);
    int mina_web_upload_download_info(const DownloadInfo& info);
    int mina_web_upload_vm_info(const VMInfo& vm_info, const UserInfo& user_info);
    int mina_web_upload_new_terminal_complete();
    int mina_web_upload_driver_install_result(int error);
    int mina_web_request_dev_interface_info(const string& product_id);
    int mina_web_login(const UserInfo& info);
    int mina_web_modify_password(const UserInfo& info);
    int mina_web_bind(const UserInfo& info);
    int mina_web_collect_log_complete(int status, const string& log_name);
    int mina_l2w_change_mode(Mode mode);
    int mina_l2w_change_hostname(const string& hostname);
    int mina_l2w_notify_partition(int c_disk_size, int d_disk_size, int max_c_disk_size);
    int mina_l2w_notify_set_devpolicy(int id, bool set_recovery, bool allow_recovery, bool set_userdisk, bool allow_userdisk);
    int mina_guesttool_msg(const string& guesttool_msg, int source, int target, int module_id);


    virtual void action();
private:
    static void* thread_main(void* data);
    void new_info_timers();
    void delete_info_timers();
    void activate_info_timer(int id, const string& data);
    void deactivate_info_timer(int id);
    void shake_hand(){send_message(RCD_HANDLE_KEEPALIVE, "00");}

    void get_json_valuebool(bool& dst, cJSON* json, const string& key);
    void get_json_valueint(int& dst, cJSON* json, const string& key);
    void get_json_valuestring(string& dst, cJSON* json, const string& key);
    void get_json_child(string& dst, cJSON* json, const string& key);

    void handle_keepalive(string data);

    void handle_check_server_type(string data);
    void handle_sync_deb_path(string data);
    void handle_sync_software_version(string data);
    void handle_sync_system_version(string data);
    void handle_sync_public_policy(string data);
    void handle_sync_mode(string data);
    void handle_sync_hostname(string data);
    void handle_sync_image(string data);
    void handle_sync_ipxe(string data);
    void handle_sync_recover_image(string data);
    void handle_sync_reset_terminal(string data);
    void handle_sync_server_ip(string data);
    void handle_sync_all_userinfo(string data);
    void handle_sync_ssid_whitelist(string data);
    void handle_sync_rcdusbconf(string data);
    void handle_sync_printer_switch(string data);
    void handle_sync_desktop_redir(string data);
    void handle_sync_dev_policy(string data);
    void handle_sync_driver_install(string data);
    void handle_sync_reload_image(string data);
    void handle_sync_delete_teacherdisk(string data);
    void handle_sync_port_mapping(string data);
    void handle_sync_server_time(string data);
    void handle_sync_main_window(string data);
    void handle_sync_terminal_passwd(string data);

    void handle_upload_basicinfo(string data);
    void handle_upload_local_network(string data);
    void handle_upload_vm_network(string data);
    void handle_upload_downloadinfo(string data);
    void handle_upload_vminfo(string data);
    
    void handle_login(string data);
    void handle_login_timeout();
    void handle_modifypassword(string data);
    void handle_bind(string data);
    void handle_collect_log_complet(string data);
    void handle_l2w_change_mode(string data);
    void handle_l2w_change_hostname(string data);



    void handle_web_check_vm_status(string data);
    void handle_web_shutdown(string data);
    void handle_web_reboot(string data);
    void handle_web_recover_image(string data);
    void handle_web_modify_password(string data);
    void handle_web_modify_local_network(string data);
    void handle_web_modify_vm_network(string data);
    void handle_web_collect_log(string data);
    void handle_web_modify_hostname(string data);
    void handle_web_reset_to_initial(string data);
    void handle_web_modify_mode(string data);
    void handle_web_resync(string data);
    void handle_web_do_ipxe(string data);
    void handle_web_reload_image(string data);
    void handle_web_delete_teacherdisk(string data);
    void handle_web_driver_install_action(string data);
    void handle_web_do_port_mapping(string data);
    void handle_web_notify_ssid_whitelist(string data);
    void handle_web_notify_reset_netdisk(string data);

    void handle_web_notify_http_port(string data);

    void handle_web_set_main_window(string data);
    void handle_web_dev_interface_info(string data);
    void handle_web_recv_guesttool_msg(string data);
    
    void handle_default(string data);


    void on_establish_connection();
    void on_destroy_connection();
    bool establish_connection();
    void destroy_connection();
    bool send_message(int handle, const string& data);
    virtual bool recv_message(int fd, int& id, string& buf);
    bool send_message_struct(int handle, const string& data);
    void message_endian_parse(int handle, char *data);


    void common_establish_connection(void* data);
    void common_destroy_connection(void* data);

    void send_web_ack(int handle, int error, string error_msg, int timestamp);

    
private:
    UnjoinableThread* _thread;
    ProcessLoop _process_loop;
    Application* _app;
    int _port;
    KeepAliveTimer* _keepalive_timer;
    ConnectTimer* _connect_timer;
    InfoTimers _info_timers;
    int _keepalive;
    bool _connected;
    ServeripInfoDB _db_serveripinfo;
    int _connect_fail_count;

friend class Mina::KeepAliveTimer;
friend class Mina::SendMessageEvent;
friend class Mina::ConnectTimer;
friend class Mina::MinaCommonEvent;
friend class Mina::InfoTimer;

    class KeepAliveTimer:public Timer
    {
    public:
        KeepAliveTimer(Mina* mina):_mina(mina) {}
        virtual ~KeepAliveTimer(){}
        virtual void response()
        {
            if(_mina->_connected)
            {
                _mina->shake_hand();
                if(_mina->_keepalive++ >= 3)
                {
                    LOG_INFO("keepalve is beyond 3, so destroy connect");
                    _mina->destroy_connection();
                }
            }
        }

    private:
        Mina* _mina;
    };

    class ConnectTimer:public Timer
    {
    public:
        ConnectTimer(Mina* mina):_mina(mina) {}
        virtual ~ConnectTimer(){}
        virtual void response()
        {
            if(!_mina->_connected)
            {
                _mina->establish_connection();
            }
        }

    private:
        Mina* _mina;
    };

    class SendMessageEvent:public Event
    {
    public:
        SendMessageEvent(Mina* mina, int handle, string data)
            :_mina (mina)
            ,_data (data)
            ,_handle (handle)
        {
        }
        virtual ~SendMessageEvent()
        {
        }
        virtual void response()
        {
            _mina->send_message(_handle, _data);
            _mina->activate_info_timer(_handle, _data);
        }
    private:
        Mina* _mina;
        string _data;
        int _handle;
    };

    /* send binary data for class interaction */
    class SendMessageStructEvent:public Event
    {
    public:
        SendMessageStructEvent(Mina* mina, int handle, char *data)
            :_mina (mina)
            ,_handle (handle)
        {
            string data2string;
            switch (handle) {
            case RCD_HANDLE_SYNC_SERVER_TYPE:
                data2string.assign(data, sizeof(msg_sync_type_t));
                break;
            case RCD_HANDLE_SYNC_DEB_PATH:
                data2string.assign(data, sizeof(msg_sync_path_t));
                break;
            default:
                data2string.assign(1, '\0');
                break;
            }
            _data = data2string;
        }
        virtual ~SendMessageStructEvent()
        {
        }
        virtual void response()
        {
            _mina->send_message_struct(_handle, _data);
            _mina->activate_info_timer(_handle, _data);
        }
    private:
        Mina* _mina;
        string _data;
        int _handle;
    };

    enum HandleId
    {
        MINA_COMMON_EVENT_ESTABLISH_CONECTION,
        MINA_COMMON_EVENT_DESTROY_CONECTION,
    };
    class MinaCommonEvent:public Event
    {
        typedef void (Mina::*Handler)(void* data);
        typedef std::map<int, Handler> Handlers;
    public:
        MinaCommonEvent(Mina* mina, int id, void* data)
            :_mina (mina)
            ,_data (data)
            ,_id (id)
        {
            set_handler(MINA_COMMON_EVENT_ESTABLISH_CONECTION, &Mina::common_establish_connection);
            set_handler(MINA_COMMON_EVENT_DESTROY_CONECTION, &Mina::common_destroy_connection);
        }
        virtual ~MinaCommonEvent()
        {
        }
        void set_handler(int id, Handler handle)
        {
            _handlers[id] = handle;
        }
        virtual void response()
        {
            (_mina->*_handlers[_id])(_data);
        }
    private:
        Mina* _mina;
        void* _data;
        int _id;
        Handlers _handlers;
    };

    class InfoTimer:public Timer
    {
    public:
        InfoTimer(Mina* mina, int id)
            :_mina (mina)
            ,_id (id)
            ,_count (0)
        {
        }
        virtual ~InfoTimer(){}

        void set_data(const string& data)
        {
            _data = data;
        }
        virtual void response()
        {
            LOG_WARNING("critical msg recv no ack, id = %x count = %d", _id, _count);
            if(_count++ >= 3)
            {
                _response_timeout_handle();
                return;
            }
            else
            {
                switch (_id) {
                case RCD_HANDLE_SYNC_SERVER_TYPE:
                case RCD_HANDLE_SYNC_DEB_PATH:
                    _mina->send_message_struct(_id, _data);
                    break;
                default:
                    _mina->send_message(_id, _data);
                    break;
                }
                return;
            }
        }
    private:
        void _response_timeout_handle()
        {
            if(_id == RCD_HANDLE_LOGIN)
            {
                _mina->_process_loop.deactivate_interval_timer(this);
                _mina->handle_login_timeout();
            }
            else
            {
                _mina->_process_loop.deactivate_interval_timer(this);
                _mina->destroy_connection();
            }
        }
        
        Mina* _mina;
        int _id;
        int _count;
        string _data;
    };
};

#endif //_MINA_H
