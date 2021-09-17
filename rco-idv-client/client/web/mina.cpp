#include "mina.h"
#include "application.h"
#include "rc/rc_public.h"
#include "rc_json.h"
#include "rc/rc_checknetifval.h"

using namespace RcJson;
Mina::Mina(Application* app, int port)
    :MinaMessageHandler (*this)
    ,_process_loop (this)
    ,_app (app)
    ,_port (port)
    ,_keepalive_timer (new KeepAliveTimer(this))
    ,_connect_timer (new ConnectTimer(this))
    ,_keepalive (0)
    ,_connected (false)
    ,_connect_fail_count (0)
{ 
    _fd = INFINITE;

    set_handler(RCD_HANDLE_KEEPALIVE,               &Mina::handle_keepalive);

    set_handler(RCD_HANDLE_SYNC_SERVER_TYPE,        &Mina::handle_check_server_type);
    set_handler(RCD_HANDLE_SYNC_DEB_PATH,           &Mina::handle_sync_deb_path);

    set_handler(RCD_HANDLE_SYNC_SOFTWARE_VERSION,   &Mina::handle_sync_software_version);
    set_handler(RCD_HANDLE_SYNC_PUBLIC_POLICY,      &Mina::handle_sync_public_policy);
    set_handler(RCD_HANDLE_SYNC_MODE,               &Mina::handle_sync_mode);
    set_handler(RCD_HANDLE_SYNC_HOSTNAME,           &Mina::handle_sync_hostname);
    set_handler(RCD_HANDLE_SYNC_IMAGE,              &Mina::handle_sync_image);
    set_handler(RCD_HANDLE_SYNC_IPXE,               &Mina::handle_sync_ipxe);
    set_handler(RCD_HANDLE_SYNC_RECOVER_IMAGE,      &Mina::handle_sync_recover_image);
    set_handler(RCD_HANDLE_SYNC_RESET_TERMINAL,     &Mina::handle_sync_reset_terminal);
    set_handler(RCD_HANDLE_SYNC_SERVER_IP,          &Mina::handle_sync_server_ip);
    set_handler(RCD_HANDLE_SYNC_ALL_USERINFO,       &Mina::handle_sync_all_userinfo);
    //set_handler(RCD_HANDLE_REQ_DEV_POLICY,			&Mina::handle_sync_dev_policy);
    set_handler(RCD_HANDLE_REQ_DEV_POLICY,			&Mina::handle_default);
    set_handler(RCD_HANDLE_SYNC_DEV_POLICY,			&Mina::handle_sync_dev_policy);
    set_handler(RCD_HANDLE_SYNC_PORT_MAPPING,       &Mina::handle_sync_port_mapping);
    set_handler(RCD_HANDLE_SYNC_REQUEST_SSID,       &Mina::handle_sync_ssid_whitelist);
    set_handler(RCD_HANDLE_SYNC_PRINTER_SWITCH,     &Mina::handle_sync_printer_switch);
    set_handler(RCD_HANDLE_SYNC_DESKTOP_REDIR,      &Mina::handle_sync_desktop_redir);
    set_handler(RCD_HANDLE_SYNC_REQUEST_USBCONF,    &Mina::handle_sync_rcdusbconf);

    set_handler(RCD_HANDLE_SYNC_DRIVER_INSTALL,		&Mina::handle_sync_driver_install);
    set_handler(RCD_HANDLE_SYNC_RELOAD_IMAGE,       &Mina::handle_sync_reload_image);
    set_handler(RCD_HANDLE_SYNC_DELETE_TEACHERDISK, &Mina::handle_sync_delete_teacherdisk);
    set_handler(RCD_HANDLE_SYNC_MAIN_WINDOW,        &Mina::handle_sync_main_window);

    set_handler(RCD_HANDLE_SYNC_SERVER_TIME,        &Mina::handle_sync_server_time);
    set_handler(RCD_HANDLE_SYNC_TERMINAL_PASSWD,    &Mina::handle_sync_terminal_passwd);
    set_handler(RCD_HANDLE_UPLOAD_BASICINFO,        &Mina::handle_upload_basicinfo);
    set_handler(RCD_HANDLE_UPLOAD_LOCAL_NETWORK,    &Mina::handle_upload_local_network);
    set_handler(RCD_HANDLE_UPLOAD_VM_NETWORK,       &Mina::handle_upload_vm_network);
    set_handler(RCD_HANDLE_UPLOAD_DOWNLOADINFO,     &Mina::handle_upload_downloadinfo);
    set_handler(RCD_HANDLE_UPLOAD_VMINFO,           &Mina::handle_upload_vminfo);
    set_handler(RCD_HANDLE_UPLOAD_NEW_TERMINAL_COMPLETE,        &Mina::handle_default);
    set_handler(RCD_HANDLE_UPLOAD_DRIVER_INSTALL_RESULT,        &Mina::handle_default);

    
    set_handler(RCD_HANDLE_LOGIN,                   &Mina::handle_login);
    set_handler(RCD_HANDLE_MODIFY_PASSWORD,         &Mina::handle_modifypassword);
    set_handler(RCD_HANDLE_BIND,                    &Mina::handle_bind);

    set_handler(RCD_HANDLE_SYNC_UNKNOW_DEV,         &Mina::handle_default);
    set_handler(RCD_HANDLE_COLLECT_LOG_COMPLETE,    &Mina::handle_default);
    set_handler(RCD_HANDLE_CHANGE_MODE,             &Mina::handle_default);
    set_handler(RCD_HANDLE_CHANGE_HOSTNAME,         &Mina::handle_default);
    set_handler(RCD_HANDLE_SEND_GUESTTOOL_MSG,      &Mina::handle_default);
    set_handler(RCD_HANDLE_NOTIFY_PARTITION,        &Mina::handle_default);
    //set_handler(RCD_HANDLE_NOTIFY_SET_DEVPOLOCY,    &Mina::handle_default);
    set_handler(RCD_HANDLE_WEB_CHECK_VM_STATUS,     &Mina::handle_web_check_vm_status);
    set_handler(RCD_HANDLE_WEB_SHUTDOWN,            &Mina::handle_web_shutdown);
    set_handler(RCD_HANDLE_WEB_REBOOT,              &Mina::handle_web_reboot);
    set_handler(RCD_HANDLE_WEB_RECORVER_IMAGE,      &Mina::handle_web_recover_image);
    set_handler(RCD_HANDLE_WEB_MODIFY_PASSWORD,     &Mina::handle_web_modify_password);
    set_handler(RCD_HANDLE_WEB_MODIFY_LOCAL_NETWORK,&Mina::handle_web_modify_local_network);
    set_handler(RCD_HANDLE_WEB_MODIFY_VM_NETWORK,   &Mina::handle_web_modify_vm_network);
    set_handler(RCD_HANDLE_WEB_COLLECT_LOG,         &Mina::handle_web_collect_log);
    set_handler(RCD_HANDLE_WEB_MODIFY_HOSTNAME,     &Mina::handle_web_modify_hostname);
    set_handler(RCD_HANDLE_WEB_RESET_TO_INITIAL,    &Mina::handle_web_reset_to_initial);
    set_handler(RCD_HANDLE_WEB_RESYNC,              &Mina::handle_web_resync);
    set_handler(RCD_HANDLE_WEB_DO_IPXE,             &Mina::handle_web_do_ipxe);
    set_handler(RCD_HANDLE_WEB_RELOAD_IMAGE,        &Mina::handle_web_reload_image);
    set_handler(RCD_HANDLE_WEB_DELETE_TEACHERDISK,  &Mina::handle_web_delete_teacherdisk);
    set_handler(RCD_HANDLE_WEB_DRIVER_INSTALL_ACTION,           &Mina::handle_web_driver_install_action);
    set_handler(RCD_HANDLE_WEB_DO_PORT_MAPPING,     &Mina::handle_web_do_port_mapping);
    set_handler(RCD_HANDLE_WEB_NOTIFY_SSID_WHITE,   &Mina::handle_web_notify_ssid_whitelist);
    set_handler(RCD_HANDLE_WEB_NOTIFY_RESET_NETDISK,&Mina::handle_web_notify_reset_netdisk);
    
    set_handler(RCD_HANDLE_WEB_NOTIFY_HTTP_PORT,    &Mina::handle_web_notify_http_port);
    
    set_handler(RCD_HANDLE_WEB_SET_MAIN_WINDOW,     &Mina::handle_web_set_main_window);
    set_handler(RCD_HANDLE_REQ_DEV_INTERFACE_INFO,  &Mina::handle_web_dev_interface_info);

    set_handler(RCD_HANDLE_RECV_GUESTTOOL_MSG,      &Mina::handle_web_recv_guesttool_msg);

    new_info_timers();
    _process_loop.activate_interval_timer(_keepalive_timer, KEEPALIVE_INTERVAL);
    _thread = new UnjoinableThread(Mina::thread_main, this);
}

Mina::~Mina()
{
    _process_loop.deactivate_interval_timer(_keepalive_timer);
    _keepalive_timer->unref();
    _process_loop.deactivate_interval_timer(_connect_timer);
    _connect_timer->unref();
    delete_info_timers();
    _thread->cancel();
    delete _thread;
}

void* Mina::thread_main(void* data)
{
    Mina* mina = (Mina*)data;
    mina->_process_loop.run();
    return NULL;
}

void Mina::new_info_timers()
{
#define MINA_NEW_INFO_TIMER(id)\
    _info_timers[id] = new InfoTimer(this, id);

    MINA_NEW_INFO_TIMER(RCD_HANDLE_SYNC_SOFTWARE_VERSION);
    MINA_NEW_INFO_TIMER(RCD_HANDLE_SYNC_PUBLIC_POLICY);
    MINA_NEW_INFO_TIMER(RCD_HANDLE_SYNC_MODE);
    MINA_NEW_INFO_TIMER(RCD_HANDLE_SYNC_HOSTNAME);
    MINA_NEW_INFO_TIMER(RCD_HANDLE_SYNC_IMAGE);
    MINA_NEW_INFO_TIMER(RCD_HANDLE_SYNC_RECOVER_IMAGE);
    MINA_NEW_INFO_TIMER(RCD_HANDLE_SYNC_IPXE);
    MINA_NEW_INFO_TIMER(RCD_HANDLE_SYNC_RESET_TERMINAL);
    MINA_NEW_INFO_TIMER(RCD_HANDLE_SYNC_SERVER_IP);
    MINA_NEW_INFO_TIMER(RCD_HANDLE_SYNC_ALL_USERINFO);
    MINA_NEW_INFO_TIMER(RCD_HANDLE_SYNC_MAIN_WINDOW);

    MINA_NEW_INFO_TIMER(RCD_HANDLE_LOGIN);
    MINA_NEW_INFO_TIMER(RCD_HANDLE_MODIFY_PASSWORD);
    MINA_NEW_INFO_TIMER(RCD_HANDLE_BIND);
    MINA_NEW_INFO_TIMER(RCD_HANDLE_COLLECT_LOG_COMPLETE);
    MINA_NEW_INFO_TIMER(RCD_HANDLE_CHANGE_MODE);
    MINA_NEW_INFO_TIMER(RCD_HANDLE_CHANGE_HOSTNAME);
    MINA_NEW_INFO_TIMER(RCD_HANDLE_SYNC_SERVER_TYPE);
    MINA_NEW_INFO_TIMER(RCD_HANDLE_SYNC_DEB_PATH);
    MINA_NEW_INFO_TIMER(RCD_HANDLE_REQ_DEV_INTERFACE_INFO);

#undef MINA_NEW_INFO_TIMER
}

void Mina::delete_info_timers()
{
#define MINA_DELETE_INFO_TIMER(id)\
    _info_timers[id]->unref();

    MINA_DELETE_INFO_TIMER(RCD_HANDLE_SYNC_SOFTWARE_VERSION);
    MINA_DELETE_INFO_TIMER(RCD_HANDLE_SYNC_PUBLIC_POLICY);
    MINA_DELETE_INFO_TIMER(RCD_HANDLE_SYNC_MODE);
    MINA_DELETE_INFO_TIMER(RCD_HANDLE_SYNC_HOSTNAME);
    MINA_DELETE_INFO_TIMER(RCD_HANDLE_SYNC_IMAGE);
    MINA_DELETE_INFO_TIMER(RCD_HANDLE_SYNC_RECOVER_IMAGE);
    MINA_DELETE_INFO_TIMER(RCD_HANDLE_SYNC_IPXE);
    MINA_DELETE_INFO_TIMER(RCD_HANDLE_SYNC_RESET_TERMINAL);
    MINA_DELETE_INFO_TIMER(RCD_HANDLE_SYNC_SERVER_IP);
    MINA_DELETE_INFO_TIMER(RCD_HANDLE_SYNC_ALL_USERINFO);
    MINA_DELETE_INFO_TIMER(RCD_HANDLE_SYNC_MAIN_WINDOW);

    MINA_DELETE_INFO_TIMER(RCD_HANDLE_LOGIN);
    MINA_DELETE_INFO_TIMER(RCD_HANDLE_MODIFY_PASSWORD);
    MINA_DELETE_INFO_TIMER(RCD_HANDLE_BIND);
    MINA_DELETE_INFO_TIMER(RCD_HANDLE_COLLECT_LOG_COMPLETE);
    MINA_DELETE_INFO_TIMER(RCD_HANDLE_CHANGE_MODE);
    MINA_DELETE_INFO_TIMER(RCD_HANDLE_CHANGE_HOSTNAME);
    MINA_DELETE_INFO_TIMER(RCD_HANDLE_SYNC_SERVER_TYPE);
    MINA_DELETE_INFO_TIMER(RCD_HANDLE_SYNC_DEB_PATH);
    MINA_DELETE_INFO_TIMER(RCD_HANDLE_REQ_DEV_INTERFACE_INFO);

#undef MINA_DELETE_INFO_TIMER
}

void Mina::activate_info_timer(int id, const string& data)
{
    unsigned int millisec = 0;
    if(_info_timers.count(id) > 0)
    {
        _info_timers[id]->set_data(data);
        if (id == RCD_HANDLE_LOGIN)
        {
            millisec = 18*1000;
        }
        else
        {
            millisec = 3*1000;
        }
        _process_loop.activate_interval_timer(_info_timers[id], millisec);
    }
}

void Mina::deactivate_info_timer(int id)
{
    if(_info_timers.count(id) > 0)
    {
        _process_loop.deactivate_interval_timer(_info_timers[id]);
    }
}

void Mina::on_establish_connection()
{
    _connect_fail_count = 0;
	_app->lm_web_update_onLineTime();
    _app->web_mina_connection_established();
}

void Mina::on_destroy_connection()
{
    _connect_fail_count = 0;
	_app->lm_web_update_onLineTime();
    _app->web_mina_connection_destroyed();
}

bool Mina::establish_connection()
{
    int val = 0;
    int ret = 0;
    struct timeval timeout = {CONNECT_TIME_S, 0};

    if(_fd != INFINITE || _connected == true)
    {
        LOG_INFO("connnection has been established _fd=%d, _connected=%d", _fd, _connected);
        goto connect_fail;
    }
    
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if(_fd == -1)
    {
        LOG_WARNING("failed to create mina socket: errno = %d, strerror:%s", errno, strerror(errno));
        goto connect_fail;
    }
    
    val = fcntl(_fd, F_GETFD);
    if(val == -1)
    {
        LOG_WARNING("fcntl _fd F_GETFD failed errno = %d, strerror:%s", errno, strerror(errno));
    }
    val|= FD_CLOEXEC;
    ret = fcntl(_fd,F_SETFD,val);
    if(ret == -1)
    {
        LOG_WARNING("fcntl _fd F_SETFD failed errno = %d, strerror:%s", errno, strerror(errno));
    }

    ret = connect_with_timeout(_fd, mina_get_server_ip(), _port, &timeout);

    if(ret == -1)
    {
        LOG_DEBUG("failed to connect mina socket errno = %d, strerror:%s", errno, strerror(errno));
        close(_fd);
        _fd = INFINITE;
        goto connect_fail;
    }

    _process_loop.add_eventsource(*this);
    _keepalive = 0;
    _connected = true;
    shake_hand();
    on_establish_connection();
    return true;

connect_fail:
    if( ((CONNECT_TIMEOUT_S) / (CONNECT_INTERVAL/1000)) == (_connect_fail_count))
    {
        _app->web_show_ui_disconnect();
    }
    _connect_fail_count++;
    return false;
}

void Mina::destroy_connection()
{
    if(_fd == INFINITE || _connected == false)
    {
        LOG_INFO("connnection has been destroyed _fd=%d, _connected=%d", _fd, _connected);
        return;
    }
    _process_loop.remove_eventsource(*this);
    close(_fd);
    _fd = INFINITE;
    _keepalive = 0;
    _connected = false;
    on_destroy_connection();
}

bool Mina::send_message_struct(int handle, const string& data)
{
    if(_fd == INFINITE || _connected == false)
    {
        LOG_WARNING("mina can not send data");
        return false;
    }

    bool return_value = false;
    int pos = 0;
    int send_length = 0;
    const char *send_msg = data.c_str();
    msg_head_t *head_msg = (msg_head_t *)send_msg;
    int total_length = ntohl(head_msg->length) + sizeof(msg_head_t);
    LOG_DEBUG("total_length:%d", total_length);

    while (pos < total_length) 
    {
        send_length = send(_fd, send_msg + pos, total_length - pos, 0);
		if(send_length < 0)
        {
            LOG_WARNING("mina send data ret <= 0, ret = %d, errno = %d", send_length, errno);
            switch(errno)
            {
                case EPIPE:
                {
                    destroy_connection();
                    return_value = false;
                    goto quit;
                }
                case EINTR:
                {
                    continue;
                }
                case EAGAIN:
                {
                    continue;
                }
                default:
                {
                    return_value = false;
                    goto quit;
                }
            }
        }
		pos += send_length;
    }
    return_value = true;

quit:
    return return_value;
}


bool Mina::send_message(int handle, const string& data)
{
    if(_fd == INFINITE || _connected == false)
    {
        LOG_WARNING("mina can not send data");
        return false;
    }

    bool return_value = false;
    int pos = 0;
    int send_length = 0;
    int total_length = VERSION_LENGTH + sizeof(int) + sizeof(short) + sizeof(int) + data.length();
    char* send_msg = new char[total_length];
    char* pointer = send_msg;

    memset(pointer, 0, VERSION_LENGTH);
    pointer += VERSION_LENGTH;
    pointer += sizeof(int);
    mina_write_short(pointer, (short)handle);
    pointer += sizeof(short);
    mina_write_int(pointer, (int)data.length());
    pointer += sizeof(int);
    mina_write_char(pointer, data.c_str(), data.length());

    while (pos < total_length) 
    {
        send_length = send(_fd, send_msg + pos, total_length - pos, 0);
		if(send_length < 0)
        {
            LOG_WARNING("mina send data ret <= 0, ret = %d, errno = %d", send_length, errno);
            switch(errno)
            {
                case EPIPE:
                {
                    destroy_connection();
                    return_value = false;
                    goto quit;
                }
                case EINTR:
                {
                    continue;
                }
                case EAGAIN:
                {
                    continue;
                }
                default:
                {
                    return_value = false;
                    goto quit;
                }
            }
        }
		pos += send_length;
    }
    return_value = true;

quit:
    delete[] send_msg;
    return return_value;
}

#define MINA_RECV(recv_buffer, length, error_action)\
    ret = recv(_fd, recv_buffer, length, MSG_WAITALL);\
    if(ret < 0)\
    {\
        LOG_WARNING("mina recv ret <= 0, ret = %d, errno = %d", ret, errno);\
        switch(errno)\
        {\
            case EPIPE:\
            {\
                destroy_connection();\
                error_action;\
            }\
            case EINTR:\
            {\
                error_action;\
            }\
            case EAGAIN:\
            {\
                error_action;\
            }\
            default:\
            {\
                error_action;\
            }\
        }\
    }\
    else if(ret == 0)\
    {\
        destroy_connection();\
        error_action;\
    }


void Mina::message_endian_parse(int handle, char *data)
{
    switch (handle) {
    case RCD_HANDLE_SYNC_SERVER_TYPE:
        {
            msg_handle_type_t *info = (msg_handle_type_t *)data;
            info->result = ntohs(info->result);
            info->action = ntohs(info->action);
        }
        break;
    case RCD_HANDLE_SYNC_DEB_PATH:
        {
            msg_handle_path_t *info = (msg_handle_path_t *)data;
            info->result = ntohs(info->result);
            info->action = ntohs(info->action);
        }
        break;
    default:
        break;
    }
    return;
}

bool Mina::recv_message(int fd, int& id, string& buf)
{
    char recv_buffer[RCD_BUFFER_LEN];
    int ret = 0;
    int length = 0;
    int reserved = 0;
    short handle = 0;
    char *data = NULL;

    memset(recv_buffer, 0 , sizeof(recv_buffer));
    MINA_RECV(recv_buffer, VERSION_LENGTH, return false); //version
    MINA_RECV(recv_buffer, sizeof(int), return false);    //reserved
    mina_read_int(reserved, recv_buffer);
    MINA_RECV(recv_buffer, sizeof(short), return false);  //handle
    mina_read_short(handle, recv_buffer);
    MINA_RECV(recv_buffer, sizeof(int), return false);    //length
    mina_read_int(length, recv_buffer);
    LOG_DEBUG("recv Head completely, handle = %x, length = %d, reserved= %d", handle, length, reserved);

    switch (handle) {
    case RCD_HANDLE_SYNC_SERVER_TYPE:
    case RCD_HANDLE_SYNC_DEB_PATH:
        {
            data = (char *)malloc(length + sizeof(msg_head_t));
            if (data == NULL)
                return false;
            memset(data, 0, length + sizeof(msg_head_t));
            memcpy(data + VERSION_LENGTH, &reserved, sizeof(int));
            MINA_RECV(data + sizeof(msg_head_t), length, goto free_data_quit2);
            message_endian_parse(handle, data);
            string data2string(data, length + sizeof(msg_head_t));
            id = handle;
            buf = data2string;
            deactivate_info_timer(id);
            if (data)
                free(data);
        }
        break;
    default:
        {
            data = new char[length+1];
            memset(data, 0, length+1);
            MINA_RECV(data, length, goto free_data_quit);//length
            LOG_INFO("recv handle %x json %s", handle, data);
            id = handle;
            buf = data;
            deactivate_info_timer(id);
            delete[] data;
        }
        break;
    }

    return true;

free_data_quit:
    delete[] data;
    return false;
free_data_quit2:
    if (data)
        free(data);
    return false;
}

#undef MINA_RECV

void Mina::action()
{
    ASSERT(_fd != INFINITE && _connected == true);
    handle_message(_fd);
}

#define GET_JSON_JUDGE_NULL_INIT(pMsg, dst, json, key)\
    pMsg = cJSON_GetObjectItem (json, key.c_str());\
    if(pMsg == NULL)\
    {\
        LOG_WARNING("can not find json key %s", key.c_str());\
        return;\
    }

void Mina::get_json_valuebool(bool& dst, cJSON* json, const string& key)
{
    cJSON* pMsg = NULL;
    GET_JSON_JUDGE_NULL_INIT(pMsg, dst, json, key);
    dst = pMsg->valueint;
}
void Mina::get_json_valueint(int& dst, cJSON* json, const string& key)
{
    cJSON* pMsg = NULL;
    GET_JSON_JUDGE_NULL_INIT(pMsg, dst, json, key);
    dst = pMsg->valueint;
}

void Mina::get_json_valuestring(string& dst, cJSON* json, const string& key)
{
    cJSON* pMsg = NULL;
    GET_JSON_JUDGE_NULL_INIT(pMsg, dst, json, key);
    dst = pMsg->valuestring;
}

void Mina::get_json_child(string& dst, cJSON* json, const string& key)
{
    cJSON* pMsg = NULL;
    GET_JSON_JUDGE_NULL_INIT(pMsg, dst, json, key);
    dst = cJSON_PrintUnformatted(pMsg);
}
#undef GET_JSON_JUDGE_NULL_INIT



void Mina::handle_keepalive(string data)
{
	static int i = 0;
	if(i++ > 200)
	{
		i = 0;
		_app->lm_web_update_onLineTime();
	}
    _keepalive = 0;
}

void Mina::handle_sync_software_version(string data)
{
    VersionInfo info;
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    string errmsg;
    get_json_valueint   (ret,                   json, "error");
    get_json_valuestring(errmsg,                json, "msg");
    get_json_valueint   (info.main_version,     json, "main_version");
    get_json_valueint   (info.minor_version,    json, "minor_version");
    get_json_valueint   (info.third_version,    json, "third_version");
    get_json_valueint   (info.fourth_version,   json, "fourth_version");
    get_json_valuestring(info.extra_first,      json, "extra_first");
    get_json_valuestring(info.extra_second,     json, "extra_second");
    get_json_valuestring(info.build_date,       json, "build_date");
    cJSON_Delete(json);
    LOG_DEBUG("%d %d %d %d %s %s %s", info.main_version, info.minor_version, info.third_version,\
         info.fourth_version, info.extra_first.c_str(), info.extra_second.c_str(), info.build_date.c_str());
    if(ret == 0)
    {
        _app->web_sync_software_version(info);
    }
    else
    {
        _app->web_init_sync_error(ret);
    }
}

void Mina::handle_sync_public_policy(string data)
{
    PolicyInfo info;
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    string errmsg;

    info.pub.outctrl_day = 0;
    get_json_valueint   (ret,                  json, "error");
    get_json_valuestring(errmsg,               json, "msg");
    get_json_child      (info.usb_policy,      json, "usb_policy");
    get_json_valuebool  (info.net_policy,      json, "net_policy");
    get_json_valueint   (info.pub.outctrl_day, json, "outctrl_day");
    cJSON_Delete(json);
    LOG_DEBUG("%d %s", info.net_policy, info.usb_policy.c_str());
    if(ret == 0)
    {
        _app->web_sync_public_policy(info);
    }
    else
    {
        _app->web_init_sync_error(ret);
    }
}

void Mina::handle_sync_mode(string data)
{
    ModeInfo info;
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    int tmp;//get mode from web
    string errmsg;
    get_json_valueint   (ret,                      json, "error");
    get_json_valuestring(errmsg,                   json, "msg");
    get_json_valueint   (tmp,                      json, "mode");
    get_json_valuestring(info.bind_user.username,  json, "bind_user");
    info.mode = (Mode)tmp;
    cJSON_Delete(json);
    LOG_DEBUG("%d %s", info.mode, info.bind_user.username.c_str());
    info.bind_user.username = gloox::password_codec_xor(info.bind_user.username, false);
    if(ret == 0)
    {
        _app->web_sync_mode(info);
    }
    else
    {
        _app->web_init_sync_error(ret);
    }
}

void Mina::handle_sync_dev_policy(string data)
{
    DevPolicyInfo info;
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    //int tmp;//get mode from web
    string errmsg;

    info.allow_guestlogin = -1;
    info.allow_netdisk = -1;
    info.allow_otherslogin = -1;
    info.allow_recovery = -1;
    info.allow_userdisk = -1;
    info.id = 0;
    get_json_valueint   (ret,					json, "error");
    get_json_valuestring(errmsg,				json, "msg");
    get_json_valueint   (info.allow_guestlogin,	json, "allow_guestlogin");
    get_json_valueint   (info.allow_netdisk,	json, "allow_netdisk");
    get_json_valueint   (info.allow_otherslogin,json, "allow_otherslogin");
    get_json_valueint   (info.allow_recovery,	json, "allow_recovery");
    get_json_valueint   (info.allow_userdisk,	json, "allow_userdisk");
    get_json_valueint   (info.id,				json, "timestamp");
    cJSON_Delete(json);
    LOG_DEBUG("get dev policy error %d,  allow_guestlogin %d, allow_netdisk %d, allow_otherslogin %d, allow_recovery %d, allow_userdisk %d\n",
    		ret, info.allow_guestlogin, info.allow_netdisk, info.allow_otherslogin, info.allow_recovery, info.allow_userdisk);

    if(ret == 0)
    {
    	_app->web_sync_dev_policy(info);
        _app->web_sync_dev_policy_complete();
    }
    else
    {
        _app->web_init_sync_error(ret);
    }
}

void Mina::handle_sync_hostname(string data)
{
    string info;
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    string errmsg;
    get_json_valueint   (ret,   json, "error");
    get_json_valuestring(errmsg,json, "msg");
    get_json_valuestring(info,  json, "hostname");
    cJSON_Delete(json);
    LOG_DEBUG("%s", info.c_str());
    if(ret == 0)
    {
        _app->web_sync_hostname(info);
    }
    else
    {
        _app->web_init_sync_error(ret);
    }
}

void Mina::handle_sync_image(string data)
{
    ImageInfo info;
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    string errmsg;

    info.real_size = 0;
    info.virt_size = 0;
    info.set_size = 0;
    info.ostype = "";
    info.layer_info.layer_on_1 = "";

    get_json_valueint   (ret,                  json, "error");
    get_json_valuestring(errmsg,               json, "msg");
    //get_json_valuebool  (info.recovery,        json, "image_recovery");
    get_json_valueint   (info.id,              json, "image_id");
    get_json_valuestring(info.name,            json, "image_name");
    get_json_valuestring(info.version,         json, "image_version");
    get_json_valuestring(info.torrent_url,     json, "image_torrent_url");
    get_json_valuestring(info.md5,             json, "image_md5");
    get_json_valuestring(info.ostype,          json, "ostype");
    get_json_valueint   (info.real_size,       json, "real_size");
    get_json_valueint   (info.virt_size,       json, "virt_size");
    get_json_valueint   (info.set_size,        json, "set_size");

    get_json_valuestring(info.layer_info.layer_on_1,  json, "layer_on");
    get_json_valuestring(info.layer_info.layer_x64_1, json, "x64");

    if (info.ostype.empty()) {
        info.ostype = "Unknown";
    }

    if (info.layer_info.layer_on_1.empty()) {
        info.layer_info.layer_on_1 = "N";
    }

    // default value
    info.layer_info.layer_disk_number = 1;
    info.layer_info.layer_disk_serial_1 = "layer001";

    cJSON_Delete(json);
    LOG_DEBUG(" %d %s %s %s %s %s %d %d %d %s %s",ret, info.name.c_str(), info.version.c_str()\
        , info.torrent_url.c_str(), info.md5.c_str(), info.ostype.c_str(), info.real_size, info.virt_size, info.set_size\
        , info.layer_info.layer_on_1.c_str(), info.layer_info.layer_x64_1.c_str());

    if(ret == 0) {
        _app->web_sync_image(info);
    } else {
        _app->web_image_sync_error(ret);
    }
}

void Mina::handle_sync_ipxe(string data)
{
    IpxeInfo info;
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    int timestamp = 0;
    int ipxe = 0;
    int ipxe_env = IPXE_ACK_NONE;

    info.iso_name = "";
    info.iso_version = "";
    get_json_valueint   (timestamp,                 json, "timestamp");
    get_json_valuestring(info.iso_name,             json, "iso_name");
    get_json_valuestring(info.iso_version,          json, "iso_version");
    cJSON_Delete(json);
    LOG_DEBUG("%d %s %s", ipxe, info.iso_name.c_str(), info.iso_version.c_str());

    unsigned int local = info.iso_version.find("IDV-RainOS", 0);
    if ((local != string::npos) && (local == 0)) {
        info.iso_version = "RCO-IDV-Rain_V6.0_R0" \
                           + info.iso_version.substr(info.iso_version.find_first_of('_') + 5) + "." \
                           + info.iso_version.substr(info.iso_version.find_first_of('_') + 1, 4);
        LOG_INFO("new iso version:%s", info.iso_version.c_str());
    }

    ipxe_env = _app->web_do_ipxe_check_env();
    if (ipxe_env != IPXE_ACK_NONE) {
        LOG_DEBUG("do not do ipxe, ipxe_env = %d", ipxe_env);
        info.iso_name.clear();
    }
    ret = _app->web_sync_ipxe(info);
}

void Mina::handle_sync_recover_image(string data)
{
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    int has_command = 0;
    get_json_valueint   (ret,           json, "error");
    get_json_valueint   (has_command,   json, "has_command");
    cJSON_Delete(json);
    LOG_DEBUG("%d", has_command);
    
    if(ret == 0)
    {
        _app->web_sync_recover_image(has_command);
    }
    else
    {
        _app->web_init_sync_error(ret);
    }
}

void Mina::handle_sync_reset_terminal(string data)
{
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    int has_command = 0;
    get_json_valueint   (ret,           json, "error");
    get_json_valueint   (has_command,   json, "has_command");
    cJSON_Delete(json);
    LOG_DEBUG("%d", has_command);

    if(ret == 0)
    {
        _app->web_sync_reset_terminal(has_command);
    }
    else
    {
        _app->web_init_sync_error(ret);
    }

}

void Mina::handle_sync_server_ip(string data)
{
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    get_json_valueint   (ret,           json, "error");
    cJSON_Delete(json);
    if(ret == 0)
    {
        _app->web_sync_server_ip(0);
    }
    else
    {
        _app->web_init_sync_error(ret);
    }

}

void Mina::handle_sync_all_userinfo(string data)
{
    std::vector<UserInfo> userinfos;
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    string json_array_string;
    get_json_valueint   (ret,               json, "error");
    get_json_child      (json_array_string, json, "userinfos");
    cJSON_Delete(json);

    if(ret == 0)
    {
        //parse array json
        cJSON* json_array = cJSON_Parse(json_array_string.c_str());
        int size = cJSON_GetArraySize(json_array);
        userinfos.resize(size);
        LOG_DEBUG("userinfos size: %d\n", size);
        int i = 0;
        for(i = 0; i < size; i++)
        {
            //parse every element
            cJSON * json_userinfo = cJSON_GetArrayItem(json_array, i);
            if(NULL == json_userinfo)
            {
                LOG_WARNING("cJSON_GetArrayItem get null");
                continue;
            }
            
            get_json_valuestring(userinfos[i].username,                         json_userinfo, "username");
            get_json_valuestring(userinfos[i].password,                         json_userinfo, "password");
            get_json_valuebool  (userinfos[i].netdisk_info.netdisk_enable,      json_userinfo, "netdisk_enable");
            get_json_valuestring(userinfos[i].netdisk_info.netdisk_username,    json_userinfo, "netdisk_username");
            get_json_valuestring(userinfos[i].netdisk_info.netdisk_password,    json_userinfo, "netdisk_password");
            get_json_valuestring(userinfos[i].netdisk_info.netdisk_ip,          json_userinfo, "netdisk_ip");
            get_json_valuestring(userinfos[i].netdisk_info.netdisk_path,        json_userinfo, "netdisk_path");
            get_json_valueint   (userinfos[i].group_id,                         json_userinfo, "group_id");
            get_json_child      (userinfos[i].policy_info.usb_policy,           json_userinfo, "usb_policy");
            get_json_valuebool  (userinfos[i].policy_info.net_policy,           json_userinfo, "net_policy");
            get_json_valuestring(userinfos[i].user_auth_info.user_auth_type,    json_userinfo, "auth_type");
            get_json_valuestring(userinfos[i].user_auth_info.ad_domain,         json_userinfo, "ad_domain");

            userinfos[i].username                       = gloox::password_codec_xor(userinfos[i].username, false);
            userinfos[i].netdisk_info.netdisk_username  = gloox::password_codec_xor(userinfos[i].netdisk_info.netdisk_username, false);
        }

        _app->web_sync_all_username(userinfos);
        cJSON_Delete(json_array);
    }
    else
    {
        _app->web_init_sync_error(ret);
    }

}

void Mina::handle_sync_ssid_whitelist(string data)
{
    std::vector<string> whitelist;
    cJSON* json = NULL;
    int timestamp = 0;
    string json_array_string;
    cJSON * json_ssids = NULL;
    string msg;
    int i = 0;
    int ret = 0;
    int size = 0;

    json = cJSON_Parse(data.c_str());
    if (json == NULL) {
        LOG_ERR("recv handle_sync_ssid_whitelist is error");
        return;
    }

    LOG_DEBUG("handle_sync_ssid_whitelist %s\n", data.c_str());
    get_json_valueint   (ret,              json, "error");
    get_json_valueint   (timestamp,        json, "timestamp");
    get_json_valuestring(msg,              json, "msg");
    get_json_child      (json_array_string,json, "whitelist");
    cJSON_Delete(json);

    if (ret == 0) {
        cJSON* json_array = cJSON_Parse(json_array_string.c_str()); 
        size = cJSON_GetArraySize(json_array);
        whitelist.resize(size);

        for (i = 0; i < size; i++) {
            json_ssids = cJSON_GetArrayItem(json_array, i);
            if (json_ssids == NULL) {
                LOG_WARNING("cJSON_GetArrayItem get null");
                continue;
            }
            get_json_valuestring(whitelist[i], json_ssids, "ssid");
        }
        _app->web_sync_ssid_whitelist(whitelist);
        cJSON_Delete(json_array);
    } else {
         _app->web_init_sync_error(ret);
    }
}

void Mina::handle_sync_printer_switch(string data)
{
    cJSON* json = NULL;
    int printer_manager = 0;
    int ret = 0;

    json = cJSON_Parse(data.c_str());
    if (json == NULL) {
        LOG_ERR("recv handle_sync_printer_switch is error");
        return;
    }

    get_json_valueint      (ret,             json, "error");
    get_json_valueint      (printer_manager, json, "printerManager");

    cJSON_Delete(json);

    if (ret == 0) {
        _app->web_sync_printer_switch(printer_manager);
    } else {
        _app->web_init_sync_error(ret);
    }
}

void Mina::handle_sync_desktop_redir(string data)
{
    cJSON* json = cJSON_Parse(data.c_str());
    string redir_switch;
    int ret = 0;

    get_json_valueint      (ret,             json, "error");
    get_json_valuestring   (redir_switch,    json, "desktopRedirect");

    cJSON_Delete(json);

    if (ret == 0) {
        _app->web_sync_desktop_redir(redir_switch);
    } else {
        _app->web_init_sync_error(ret);
    }
}

void Mina::handle_sync_rcdusbconf(string data)
{
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    string usb_conf;

    get_json_valueint   (ret,           json, "error");
    get_json_valuestring(usb_conf,      json, "usb_conf");

    cJSON_Delete(json);

    if(ret == 0) {
        // recv then get to write to usbconf
        usb_conf = gloox::Base64::decode64(usb_conf);
        _app->web_sync_rcdusbconf_info(usb_conf);
    } else {
        LOG_DEBUG("handle_sync_rcdusbconf error %d", ret);
    }
}


void Mina::handle_sync_port_mapping(string data)
{
    std::vector<PortMappingInfo> port_mapping_infos;
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    string json_array_string;
    get_json_valueint   (ret,               json, "error");
    get_json_child      (json_array_string, json, "ports");
    cJSON_Delete(json);

    if(ret == 0)
    {
        //parse array json
        cJSON* json_array = cJSON_Parse(json_array_string.c_str());
        int size = cJSON_GetArraySize(json_array);
        port_mapping_infos.resize(size);
        LOG_DEBUG("port_maaping_infos size: %d\n", size);
        int i = 0;
        for(i = 0; i < size; i++)
        {
            //parse every element
            cJSON * json_userinfo = cJSON_GetArrayItem(json_array, i);
            if(NULL == json_userinfo)
            {
                LOG_WARNING("cJSON_GetArrayItem get null");
                continue;
            }

            get_json_valueint   (port_mapping_infos[i].src_port, json_userinfo, "type");
            get_json_valueint   (port_mapping_infos[i].dst_port, json_userinfo, "port");

        }

        _app->web_sync_port_mapping(port_mapping_infos);
        cJSON_Delete(json_array);
    }
    else
    {
        _app->web_init_sync_error(ret);
    }

}

void Mina::handle_sync_server_time(string data)
{
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    int timestamp;
    string server_time;
    
    get_json_valueint(ret,                  json, "error");
    get_json_valueint(timestamp,            json, "timestamp");
    get_json_child(server_time,             json, "Time");
    
    cJSON_Delete(json);
    if(ret == 0) {
        _app->web_sync_server_time(server_time);
    } else {
        _app->web_init_sync_error(ret);
    }
}

void Mina::handle_sync_driver_install(string data)
{
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    get_json_valueint   (ret,               json, "error");
    cJSON_Delete(json);
    if(ret == 0)
    {
        _app->web_sync_driver_install(data);
    }
}

void Mina::handle_sync_reload_image(string data)
{
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    int has_command = 0;
    get_json_valueint   (ret,           json, "error");
    get_json_valueint   (has_command,   json, "has_command");
    cJSON_Delete(json);
    LOG_DEBUG("%d", has_command);
    
    if(ret == 0)
    {
        //_app->web_sync_reload_image(has_command);
    }
    else
    {
        _app->web_init_sync_error(ret);
    }
}

void Mina::handle_sync_delete_teacherdisk(string data)
{
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    int has_command = 0;
    get_json_valueint   (ret,           json, "error");
    get_json_valueint   (has_command,   json, "has_command");
    cJSON_Delete(json);
    LOG_DEBUG("%d", has_command);
    
    if(ret == 0)
    {
        _app->web_sync_delete_teacherdisk(has_command);
    }
    else
    {
        _app->web_init_sync_error(ret);
    }
}

void Mina::handle_check_server_type(string data)
{
    msg_handle_type_t *info = (msg_handle_type_t *)data.c_str();
    LOG_DEBUG("==========0x0181==========", info->head.reserved);
    LOG_DEBUG("reserved:%d", info->head.reserved);
    LOG_DEBUG("result:  %d", info->result);
    LOG_DEBUG("action:  %d", info->action);
    LOG_DEBUG("version: %s", info->version);
    LOG_DEBUG("==========================", info->result);

    int reserved = (int)(info->head.reserved & 0x00FF);

    UpgradeInfo classUpgradeInfo;
    VersionInfo versionInfo;
    sscanf(info->version, "%d.%d.%d.%d", &(versionInfo.main_version), \
           &(versionInfo.minor_version), \
           &(versionInfo.third_version), \
           &(versionInfo.fourth_version));
    classUpgradeInfo.versionInfo = versionInfo;
    classUpgradeInfo.serverType = reserved;
    LOG_INFO("version: %s, reserved:%d", info->version, reserved);

    switch (reserved) {
    case TYPE_NEW_RCO_SERVER:
        _app->web_upgrade_for_office();
        break;
    case TYPE_NEW_RCC_SERVER:
        _app->set_upgrade_server_info(classUpgradeInfo);
        _app->web_sync_deb_path();
        break;
    case TYPE_UNKNOW_SERVER:
        if (((versionInfo.main_version == 3) && (versionInfo.minor_version < 3)) \
            || (versionInfo.main_version < 3))
        {
            _app->web_upgrade_for_office();
        } else {
            classUpgradeInfo.serverType = TYPE_OLD_RCC_SERVER;
            _app->set_upgrade_server_info(classUpgradeInfo);
            _app->web_sync_deb_path();
        }
        break;
    default:
        break;
    }

    return;
}

void Mina::handle_sync_deb_path(string data)
{
    UpgradeInfo classUpgradeInfo;
    msg_handle_path_t *info = (msg_handle_path_t *)data.c_str();

    LOG_DEBUG("==========0x0182==========", info->head.reserved);
    LOG_DEBUG("result:  %d", info->result);
    LOG_DEBUG("action:  %d", info->action);
    LOG_DEBUG("version: %s", info->update_path);
    LOG_DEBUG("==========================", info->result);

    classUpgradeInfo = _app->get_upgrade_server_info();
    _app->web_upgrade_for_class(classUpgradeInfo);
    return;
}

void Mina::handle_sync_main_window(string data)
{
    string json_array_string;
    std::vector<MainWindowInfo> picinfos;
    int i;

    cJSON *json = cJSON_Parse(data.c_str());
    get_json_child(json_array_string, json, "themeStrategyConfig");
    cJSON_Delete(json);

    //parse array json
    cJSON* json_array = cJSON_Parse(json_array_string.c_str());
    if (json_array == NULL) {
        LOG_ERR("json_array is NULL");
        return;
    }

    int size = cJSON_GetArraySize(json_array);
    picinfos.resize(size);
    LOG_DEBUG("picinfos size: %d\n", size);

    for(i = 0; i < size; i++) {
        //parse every element
        cJSON * json_picinfos = cJSON_GetArrayItem(json_array, i);
        if(NULL == json_picinfos) {
            LOG_WARNING("cJSON_GetArrayItem get null");
            continue;
        }
        get_json_valuestring(picinfos[i].pictureMD5, json_picinfos, "pictureMD5");
        get_json_valuestring(picinfos[i].pictureUrl, json_picinfos, "pictureUrl");
        get_json_valuestring(picinfos[i].type,       json_picinfos, "type");
        LOG_DEBUG("picinfos:%d", i);
        LOG_DEBUG("type:%s", picinfos[i].type.c_str());
        LOG_DEBUG("pictureMD5:%s", picinfos[i].pictureMD5.c_str());
        LOG_DEBUG("pictureUrl:%s", picinfos[i].pictureUrl.c_str());
    }
    cJSON_Delete(json_array);

    _app->web_sync_main_window(picinfos);
    return;
}

void Mina::handle_upload_basicinfo(string data)
{
    cJSON* json = cJSON_Parse(data.c_str());
    string userConfiguredIP;

    LOG_DEBUG(" ");
    int ret = 0;
    int new_terminal = 0;
    int default_mode = 0;
    int hide_guest_login = 0;

    HttpPortInfo hpi;
    
    hpi.public_port = 80;
    hpi.private_port = 80;
    hpi.public_ip = "";
    hpi.private_ip = "";

    get_json_valueint   (ret,                              json, "error");
    get_json_valueint   (new_terminal,                     json, "new_terminal");
    get_json_valueint   (default_mode,                     json, "mode");
    get_json_valueint   (hide_guest_login,                 json, "hide_guest_login");
    get_json_valueint   (hpi.public_port,  json, "pubPort");
    get_json_valuestring(hpi.public_ip,    json, "pubIp");
    get_json_valuestring(hpi.private_ip,   json, "priIp");
    get_json_valuestring(hpi.btPortsRange,   json, "btPortsRange");

    cJSON_Delete(json);

    // TODO:we only send one parament to application, so -1 means it is not new terminal
    // TODO: if there be more parament , we should define a datastruct
    if(ret == 0)
    {
        if(new_terminal == 0)
        {
            default_mode = -1;
        }
        _app->web_set_hide_guest_login(hide_guest_login);

        userConfiguredIP = mina_get_server_ip();
        LOG_DEBUG("user config ip is %s.", userConfiguredIP);
        if (userConfiguredIP == hpi.private_ip) {
            LOG_DEBUG("Skipping set port mapping due to configured ip %s is the same as private ip\n",
               userConfiguredIP);
        } else {
            _app->web_notify_http_port(hpi);
        }

        _app->web_ready(default_mode);
    }
    else
    {
        _app->web_init_sync_error(ret);
    }
}

void Mina::handle_upload_local_network(string data)
{
    cJSON* json = cJSON_Parse(data.c_str());
    LOG_DEBUG(" ");
    cJSON_Delete(json);
}

void Mina::handle_upload_vm_network(string data)
{
    cJSON* json = cJSON_Parse(data.c_str());
    LOG_DEBUG(" ");
    cJSON_Delete(json);
}

void Mina::handle_upload_downloadinfo(string data)
{
    cJSON* json = cJSON_Parse(data.c_str());
    LOG_DEBUG(" ");
    cJSON_Delete(json);
}

void Mina::handle_upload_vminfo(string data)
{
    cJSON* json = cJSON_Parse(data.c_str());
    LOG_DEBUG(" ");
    cJSON_Delete(json);
}

void Mina::handle_login(string data)
{
    UserInfo info;
    int ret = 0;
    string errmsg;
    cJSON* json = cJSON_Parse(data.c_str());

    get_json_valueint   (ret,                                       json, "error");
    if (ret == 0 || ret == 98) {
        get_json_valuestring(errmsg,                                    json, "msg");
        get_json_valuebool  (info.netdisk_info.netdisk_enable,          json, "netdisk_enable");
        get_json_valuestring(info.netdisk_info.netdisk_username,        json, "netdisk_username");
        get_json_valuestring(info.netdisk_info.netdisk_password,        json, "netdisk_password");
        get_json_valuestring(info.netdisk_info.netdisk_ip,              json, "netdisk_ip");
        get_json_valuestring(info.netdisk_info.netdisk_path,            json, "netdisk_path");
        get_json_valueint   (info.group_id,                             json, "group_id");
        get_json_child      (info.policy_info.usb_policy,               json, "usb_policy");
        get_json_valuebool  (info.policy_info.net_policy,               json, "net_policy");
        get_json_valuestring(info.username,                             json, "user_name");
        get_json_valuestring(info.user_auth_info.user_auth_type,        json, "auth_type");
        get_json_valuestring(info.user_auth_info.ad_domain,             json, "ad_domain");
        get_json_valuestring(info.pcname,                               json, "pcname");
        get_json_valuestring(info.desktop_redir,                        json, "desktopRedirect");

        info.netdisk_info.netdisk_username   = gloox::password_codec_xor(info.netdisk_info.netdisk_username, false);
        info.username                        = gloox::password_codec_xor(info.username, false);
   }

    cJSON_Delete(json);
    LOG_DEBUG("%d %d %d %s %s %s %s %s %s", info.netdisk_info.netdisk_enable, info.group_id,info.policy_info.net_policy\
        , info.netdisk_info.netdisk_username.c_str(), info.netdisk_info.netdisk_password.c_str(), info.netdisk_info.netdisk_ip.c_str()\
        , info.netdisk_info.netdisk_path.c_str(), info.policy_info.usb_policy.c_str(), info.pcname.c_str());
    LOG_DEBUG("username: %s, user_auth_type: %s, ad_domain: %s"
        , info.username.c_str(), info.user_auth_info.user_auth_type.c_str(), info.user_auth_info.ad_domain.c_str());
    if(ret == 0 || ret == 98)
    {
        _app->web_login_success(info);
    }
    else
    {
        _app->web_login_fail(ret);
    }
}

void Mina::handle_login_timeout()
{
    if(_app->get_mode_data().get().mode != PUBLIC_MODE)
    {
        _app->web_login_fail(-2);
    }
}

void Mina::handle_modifypassword(string data)
{
    int ret = 0;
    string errmsg;
    cJSON* json = cJSON_Parse(data.c_str());

    get_json_valueint   (ret,           json, "error");
    get_json_valuestring(errmsg,        json, "msg");
    cJSON_Delete(json);
    LOG_DEBUG("%d", ret);
    if(ret == 0)
    {
        _app->web_modify_password_success();
    }
    else
    {
        _app->web_modify_password_fail(ret);
    }

}

void Mina::handle_bind(string data)
{
    UserInfo info;
    int ret = 0;
    string errmsg;
    cJSON* json = cJSON_Parse(data.c_str());
    get_json_valueint   (ret,                                 json, "error");
    get_json_valuestring(errmsg,                              json, "msg");
    get_json_valuestring(info.username,                       json, "user_name");
    get_json_valuestring(info.user_auth_info.user_auth_type,  json, "auth_type");
    get_json_valuestring(info.user_auth_info.ad_domain,       json, "ad_domain");

    //modify binded username supported by web
    info.username    = gloox::password_codec_xor(info.username, false);
    //info.ad_info.ad_domain = gloox::Base64::decode64(info.ad_info.ad_domain);

    cJSON_Delete(json);
    LOG_DEBUG("username: %s, user_auth_type: %s, ad_domain: %s"
        , info.username.c_str(), info.user_auth_info.user_auth_type.c_str(), info.user_auth_info.ad_domain.c_str());
    if(ret == 0)
    {
    	//chenw maybe mina here get terminal policy
        _app->web_bind_success(info);
    }
    else
    {
        _app->web_bind_fail(ret);
    }
}

void Mina::handle_collect_log_complet(string data)
{
    cJSON* json = cJSON_Parse(data.c_str());
    LOG_DEBUG(" ");
    cJSON_Delete(json);
}

void Mina::handle_l2w_change_mode(string data)
{
    cJSON* json = cJSON_Parse(data.c_str());
    LOG_DEBUG(" ");
    cJSON_Delete(json);
}

void Mina::handle_l2w_change_hostname(string data)
{
    cJSON* json = cJSON_Parse(data.c_str());
    LOG_DEBUG(" ");
    cJSON_Delete(json);
}




void Mina::handle_web_check_vm_status(string data)
{
    int ret = 0;
    int timestamp = 0;
    cJSON* json = cJSON_Parse(data.c_str());
    get_json_valueint(timestamp, json, "timestamp");
    cJSON_Delete(json);
    ret = _app->web_notify_check_vm_status();
    send_web_ack(RCD_HANDLE_WEB_CHECK_VM_STATUS, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);
}

void Mina::handle_web_shutdown(string data)
{
    int ret = 0;
    int timestamp = 0;
    cJSON* json = cJSON_Parse(data.c_str());
    get_json_valueint(timestamp, json, "timestamp");
    cJSON_Delete(json);
    LOG_DEBUG(" ");
    if(_app->is_installing_driver())
    {
        ret = ERROR_DRIVER_INSTALLING;
    }
    else
    {
        ret = _app->web_notify_shutdown();
    }
    send_web_ack(RCD_HANDLE_WEB_SHUTDOWN, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);

}

void Mina::handle_web_reboot(string data)
{
    int ret = 0;
    int timestamp = 0;
    cJSON* json = cJSON_Parse(data.c_str());
    get_json_valueint(timestamp, json, "timestamp");
    cJSON_Delete(json);
    LOG_DEBUG(" ");
    if(_app->is_installing_driver())
    {
        ret = ERROR_DRIVER_INSTALLING;
    }
    else
    {
        ret = _app->web_notify_reboot();
    }
    send_web_ack(RCD_HANDLE_WEB_REBOOT, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);

}

void Mina::handle_web_recover_image(string data)
{
    int ret = 0;
    int timestamp = 0;
    cJSON* json = cJSON_Parse(data.c_str());
    get_json_valueint(timestamp, json, "timestamp");
    cJSON_Delete(json);
    LOG_DEBUG(" ");
    ret = _app->web_notify_recorver_image();
    send_web_ack(RCD_HANDLE_WEB_RECORVER_IMAGE, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);

}

void Mina::handle_web_modify_password(string data)
{
    UserInfo info;
    int ret = 0;
    int timestamp = 0;
    cJSON* json = cJSON_Parse(data.c_str());
    get_json_valueint   (timestamp,         json, "timestamp");
    get_json_valuestring(info.username,     json, "username");
    get_json_valuestring(info.password,     json, "password");
    get_json_valuestring(info.new_password, json, "password");

#if 0
    info.username       = gloox::Base64::decode64(info.username);
    info.password       = gloox::Base64::decode64(info.password);
    info.new_password   = gloox::Base64::decode64(info.new_password);
#endif
    cJSON_Delete(json);
    LOG_DEBUG("%s %s", info.username.c_str(), info.password.c_str());
    ret = _app->web_notify_modify_password(info);
    send_web_ack(RCD_HANDLE_WEB_MODIFY_PASSWORD, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);

}
void Mina::handle_web_modify_local_network(string data)
{
    NetworkInfo info;
    int ret = 0;
    int timestamp = 0;
    cJSON* json = cJSON_Parse(data.c_str());
    get_json_valueint   (timestamp,     json, "timestamp");
    get_json_valuebool  (info.dhcp,     json, "dhcp");
    get_json_valuestring(info.ip,       json, "ip");
    get_json_valuestring(info.submask,  json, "submask");
    get_json_valuestring(info.gateway,  json, "gateway");
    get_json_valuebool  (info.auto_dns, json, "auto_dns");
    get_json_valuestring(info.main_dns, json, "main_dns");
    get_json_valuestring(info.back_dns, json, "back_dns");
    cJSON_Delete(json);
    LOG_DEBUG("%d %s %s %s %d %s %s", info.dhcp, info.ip.c_str(), info.submask.c_str(), info.gateway.c_str()\
        , info.auto_dns, info.main_dns.c_str(), info.back_dns.c_str());

    if(_app->is_installing_driver())
    {
        ret = ERROR_DRIVER_INSTALLING;
    }
    else
    {
        ret = _app->web_notify_modify_local_network(info);
    }
    send_web_ack(RCD_HANDLE_WEB_MODIFY_LOCAL_NETWORK, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);

}
void Mina::handle_web_modify_vm_network(string data)
{
    NetworkInfo info;
    int ret = 0;
    int timestamp = 0;
    cJSON* json = cJSON_Parse(data.c_str());
    get_json_valueint   (timestamp,     json, "timestamp");
    get_json_valuebool  (info.dhcp,     json, "dhcp");
    get_json_valuestring(info.ip,       json, "ip");
    get_json_valuestring(info.submask,  json, "submask");
    get_json_valuestring(info.gateway,  json, "gateway");
    get_json_valuebool  (info.auto_dns, json, "auto_dns");
    get_json_valuestring(info.main_dns, json, "main_dns");
    get_json_valuestring(info.back_dns, json, "back_dns");
    cJSON_Delete(json);
    LOG_DEBUG("%d %s %s %s %d %s %s", info.dhcp, info.ip.c_str(), info.submask.c_str(), info.gateway.c_str()\
        , info.auto_dns, info.main_dns.c_str(), info.back_dns.c_str());
    ret = _app->web_notify_modify_vm_network(info);
    send_web_ack(RCD_HANDLE_WEB_MODIFY_VM_NETWORK, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);

}

void Mina::handle_web_collect_log(string data)
{
    int ret = 0;
    int timestamp = 0;
    FtpLogInfo info;

    cJSON* json = cJSON_Parse(data.c_str());
    if (json == NULL) {
        LOG_ERR("%s cJSON_Parse error", __func__);
        return;
    }

    get_json_valueint(timestamp, json, "timestamp");
    get_json_valuestring(info.ftpuser, json, "ftpuser");
    get_json_valuestring(info.ftppwd,  json, "ftppwd");
    cJSON_Delete(json);

    info.ftpuser = gloox::password_codec_xor(info.ftpuser, false, gloox::XOR_FTP_KEY_STRING);
    info.ftppwd = gloox::password_codec_xor(info.ftppwd, false, gloox::XOR_FTP_KEY_STRING);
    ret = _app->web_notify_collect_log(info);
    send_web_ack(RCD_HANDLE_WEB_COLLECT_LOG, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);

}
void Mina::handle_web_modify_hostname(string data)
{
    string hostname;
    int ret = 0;
    int timestamp = 0;
    cJSON* json = cJSON_Parse(data.c_str());
    get_json_valueint   (timestamp, json, "timestamp");
    get_json_valuestring(hostname,  json, "hostname");
    cJSON_Delete(json);
    LOG_DEBUG("%s", hostname.c_str());
    ret = _app->web_notify_modify_hostname(hostname);
    send_web_ack(RCD_HANDLE_WEB_MODIFY_HOSTNAME, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);

}

void Mina::handle_web_reset_to_initial(string data)
{
    int ret = 0;
    int timestamp = 0;
    cJSON* json = cJSON_Parse(data.c_str());
    get_json_valueint   (timestamp, json, "timestamp");
    cJSON_Delete(json);
    ret = _app->web_notify_reset_to_initial();
    send_web_ack(RCD_HANDLE_WEB_RESET_TO_INITIAL, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);

}

void Mina::handle_web_resync(string data)
{
    int ret = 0;
    int timestamp = 0;
    int action = 0;
    ModeInfo info;
    cJSON* json = cJSON_Parse(data.c_str());
    get_json_valueint   (timestamp,                 json, "timestamp");
    get_json_valueint   (action,                    json, "action");
    cJSON_Delete(json);
    ret = _app->web_notify_resync(action);
    send_web_ack(RCD_HANDLE_WEB_RESYNC, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);
}

void Mina::handle_web_do_ipxe(string data)
{
	IpxeInfo info;
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    int timestamp = 0;
    int ipxe = 0;
    int ipxe_env = IPXE_ACK_NONE;

    info.iso_name = "";
    info.iso_version = "";
    get_json_valueint   (timestamp,                 json, "timestamp");
    get_json_valuestring(info.iso_name,             json, "iso_name");
    get_json_valuestring(info.iso_version,          json, "iso_version");
    cJSON_Delete(json);
    LOG_DEBUG("%d %s %s", ipxe, info.iso_name.c_str(), info.iso_version.c_str());

    //chenw debug
    //info.iso_version = "RCO-IDV-RainOS_V2.0_R0.1";

    //new format:IDV-RainOS_abcd[Te] -> RCO-IDV-Rain_V6.0_R0[Te].abcd
    unsigned int local = info.iso_version.find("IDV-RainOS", 0);
    if ((local != string::npos) && (local == 0)) {
        info.iso_version = "RCO-IDV-Rain_V6.0_R0" \
                           + info.iso_version.substr(info.iso_version.find_first_of('_') + 5) + "." \
                           + info.iso_version.substr(info.iso_version.find_first_of('_') + 1, 4);
        LOG_INFO("new iso version:%s", info.iso_version.c_str());
    }

    ipxe_env = _app->web_do_ipxe_check_env();
    if (ipxe_env != IPXE_ACK_NONE) {
        send_web_ack(RCD_HANDLE_WEB_DO_IPXE, ipxe_env, std::to_string(ipxe_env), timestamp);
    } else {
        ret = _app->web_notify_do_ipxe(info);
        send_web_ack(RCD_HANDLE_WEB_DO_IPXE, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);
    }
}

void Mina::handle_web_reload_image(string data)
{
    int ret = 0;
    int timestamp = 0;
    cJSON* json = cJSON_Parse(data.c_str());
    get_json_valueint(timestamp, json, "timestamp");
    cJSON_Delete(json);
    LOG_DEBUG(" ");
    //ret = _app->web_notify_reload_image();
    send_web_ack(RCD_HANDLE_WEB_RELOAD_IMAGE, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);

}

void Mina::handle_web_delete_teacherdisk(string data)
{
    int ret = 0;
    int timestamp = 0;
    cJSON* json = cJSON_Parse(data.c_str());
    get_json_valueint(timestamp, json, "timestamp");
    cJSON_Delete(json);
    LOG_DEBUG(" ");
    ret = _app->web_notify_delete_teacherdisk();
    send_web_ack(RCD_HANDLE_WEB_DELETE_TEACHERDISK, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);

}

void Mina::handle_web_notify_http_port(string data)
{
    int ret = 0;
    int timestamp = 0;
    HttpPortInfo hpi;
    string userConfiguredIP;
    hpi.public_port = 80;
    hpi.private_port = 80;
    hpi.public_ip = "";
    hpi.private_ip = "";

    cJSON *json = cJSON_Parse(data.c_str());
    get_json_valueint(timestamp, json, "timestamp");
    get_json_valueint   (hpi.public_port,  json, "pubPort");
    get_json_valuestring(hpi.public_ip,    json, "pubIp");
    get_json_valuestring(hpi.private_ip,   json, "priIp");
    cJSON_Delete(json);

    userConfiguredIP = mina_get_server_ip();
    LOG_DEBUG("user config ip is %s.", userConfiguredIP);
    if (userConfiguredIP == hpi.private_ip) {
        LOG_DEBUG("Skipping set port mapping due to configured ip %s is the same as private ip\n",
           userConfiguredIP);
        ret = SUCCESS;
        send_web_ack(RCD_HANDLE_WEB_NOTIFY_HTTP_PORT, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);
        return;
    }
    LOG_DEBUG("mina invoke app to handle http port mapping");
    ret = _app->web_notify_http_port(hpi);
    send_web_ack(RCD_HANDLE_WEB_NOTIFY_HTTP_PORT, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);
}

void Mina::handle_web_driver_install_action(string data)
{
    int action;
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    int timestamp = 0;
    get_json_valueint   (timestamp,                 json, "timestamp");
    get_json_valueint   (action,                    json, "action");
    cJSON_Delete(json);
    
    ret = _app->web_notify_reboot();

    send_web_ack(RCD_HANDLE_WEB_DRIVER_INSTALL_ACTION, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);
}

void Mina::handle_web_do_port_mapping(string data)
{
    int src_port;
    int dst_port;
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    int timestamp = 0;
    get_json_valueint   (timestamp,                 json, "timestamp");
    get_json_valueint   (src_port,                  json, "type");
    get_json_valueint   (dst_port,                  json, "port");
    cJSON_Delete(json);
    
    ret = _app->web_notify_do_port_mapping(src_port, dst_port);

    send_web_ack(RCD_HANDLE_WEB_DO_PORT_MAPPING, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);
}

void Mina::handle_web_notify_ssid_whitelist(string data)
{
    if (is_wifi_terminal() == false) {
        return;
    }
    
    int ret = 0;
    int timestamp = 0;
    int size = 0;
    int i = 0;
    string json_array_string;
    cJSON* json_array = NULL;
    cJSON *json_ssid = NULL;
    std::vector<string> whitelist;
    cJSON *json = cJSON_Parse(data.c_str());
    
    get_json_valueint(timestamp, json, "timestamp");
    get_json_child(json_array_string, json, "whitelist");
    cJSON_Delete(json);

    //parse array json
    json_array = cJSON_Parse(json_array_string.c_str());
    size = cJSON_GetArraySize(json_array);
    whitelist.resize(size);
    LOG_DEBUG("whitelist size: %d\n", size);
  
    for (i = 0; i < size; i++) {
        json_ssid = cJSON_GetArrayItem(json_array, i);
        if (json_ssid == NULL) {
            LOG_WARNING("cJSON_GetArrayItem get null");
            continue;
        }
        get_json_valuestring(whitelist[i], json_ssid, "ssid");
    }
    cJSON_Delete(json_array); 

    ret = _app->web_notify_ssid_whitelist(whitelist); 
    send_web_ack(RCD_HANDLE_WEB_NOTIFY_SSID_WHITE, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);
}


void Mina::handle_sync_terminal_passwd(string data)
{
    string newpasswd;
    int ret = 0;

    cJSON* json = cJSON_Parse(data.c_str());
    if (json == NULL) {
        LOG_ERR("recv handle_sync_terminal_passwd is error");
        return;
    }

    get_json_valuestring(newpasswd, json, "newPwd");
    get_json_valueint (ret, json, "ret");
    cJSON_Delete(json);

    LOG_DEBUG("terminal new password %s", newpasswd.c_str());
    if (ret == 0) {
        _app->web_sync_terminal_password(newpasswd);
    }
}

void Mina::handle_web_notify_reset_netdisk(string data)
{
    int ret = 0;
    int timestamp = 0;
    NetdiskInfo info;

    cJSON *json = cJSON_Parse(data.c_str());

    get_json_valueint(timestamp, json, "timestamp");
    get_json_valuebool  (info.netdisk_enable,          json, "netdisk_enable");
    get_json_valuestring(info.netdisk_username,        json, "netdisk_username");
    get_json_valuestring(info.netdisk_password,        json, "netdisk_password");
    get_json_valuestring(info.netdisk_ip,              json, "netdisk_ip");
    get_json_valuestring(info.netdisk_path,            json, "netdisk_path");
    cJSON_Delete(json);

    info.netdisk_username   = gloox::password_codec_xor(info.netdisk_username, false);
    //info.netdisk_password   = gloox::password_codec_xor(info.netdisk_password, false);

    ret = _app->web_notify_reset_netdisk(info);
    send_web_ack(RCD_HANDLE_WEB_NOTIFY_RESET_NETDISK, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);
}

void Mina::handle_web_set_main_window(string data)
{
    int timestamp = 0;
    int ret = 0, i;
    string json_array_string;
    std::vector<MainWindowInfo> picinfos;

    cJSON *json = cJSON_Parse(data.c_str());
    get_json_child(json_array_string, json, "themeStrategyConfig");
    cJSON_Delete(json);

    //parse array json
    cJSON* json_array = cJSON_Parse(json_array_string.c_str());
    if (json_array == NULL) {
        LOG_ERR("json_array is NULL");
        return;
    }

    int size = cJSON_GetArraySize(json_array);
    picinfos.resize(size);
    LOG_DEBUG("picinfos size: %d\n", size);

    for(i = 0; i < size; i++) {
        //parse every element
        cJSON * json_picinfos = cJSON_GetArrayItem(json_array, i);
        if(NULL == json_picinfos) {
            LOG_WARNING("cJSON_GetArrayItem get null");
            continue;
        }
        get_json_valuestring(picinfos[i].pictureMD5, json_picinfos, "pictureMD5");
        get_json_valuestring(picinfos[i].pictureUrl, json_picinfos, "pictureUrl");
        get_json_valuestring(picinfos[i].type,       json_picinfos, "type");
        LOG_DEBUG("picinfos:%d", i);
        LOG_DEBUG("type:%s", picinfos[i].type.c_str());
        LOG_DEBUG("pictureMD5:%s", picinfos[i].pictureMD5.c_str());
        LOG_DEBUG("pictureUrl:%s", picinfos[i].pictureUrl.c_str());
    }
    cJSON_Delete(json_array);

    ret = _app->web_sync_main_window(picinfos);
    send_web_ack(RCD_HANDLE_WEB_SET_MAIN_WINDOW, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);
}
//0x761 Interface Return information
void Mina::handle_web_dev_interface_info(string data)
{
    int ret = 0;
    string errmsg = "";
    string interface_info = "";
    int net_passthrough = 0;
    DevInterfaceInfo interfaces;
    cJSON *json = cJSON_Parse(data.c_str());
    get_json_valueint   (ret,                json, "error");
    get_json_valuestring(errmsg,             json, "msg");
    get_json_valueint   (net_passthrough,    json, "net_passthrough");
    get_json_child(interface_info, json, "interface_info");
    /*get_json_child(info.net_passthrough, json, "net_passthrough");
    get_json_child(info.serial_port, json, "serial_port");
    get_json_child(info.parallel_port, json, "parallel_port");*/
    if (ret != 0){
        LOG_DEBUG("errmsg:%s", errmsg.c_str());
        interface_info = "";
        net_passthrough = 0;
    }
    interfaces.interface_info = interface_info;
    interfaces.net_passthrough = net_passthrough;
    cJSON_Delete(json);
    _app->web_get_dev_interface_info(interfaces);
}

void Mina::handle_web_recv_guesttool_msg(string data)
{
    cJSON* json = cJSON_Parse(data.c_str());
    int ret = 0;
    int timestamp = 0;
    get_json_valueint   (timestamp,                 json, "timestamp");
    cJSON_Delete(json);

    ret = _app->web_recv_guesttool_msg(data);
    send_web_ack(RCD_HANDLE_RECV_GUESTTOOL_MSG, ret, ErrorNote::get_instance()->error_notes[ret], timestamp);
}


void Mina::handle_default(string data)
{
    return;
}



void Mina::common_establish_connection(void* data)
{
    _connect_fail_count = 0;
    establish_connection();
    _process_loop.activate_interval_timer(_connect_timer, CONNECT_INTERVAL);
}

void Mina::common_destroy_connection(void* data)
{
    _connect_fail_count = 0;
    destroy_connection();
    _process_loop.deactivate_interval_timer(_connect_timer);
}


int Mina::mina_check_web_alive(const string& server_ip)
{
    int ret = 0;
    char result_buf[256];
    int result_len = sizeof(result_buf);
    string curl_buf = "curl http://" + server_ip + "/module/fusion/terminalCheck/webAlive  --max-time 1";
    ret = rc_system_rw(curl_buf.c_str(), (unsigned char*)result_buf, &result_len, "r");
    if((ret != 0) || (strlen(result_buf) == 0))
    {
        return -1;
    }
    else
    {
        return SUCCESS;
    }
}

int Mina::mina_establish_connection()
{
    MinaCommonEvent* event = new MinaCommonEvent\
        (this, MINA_COMMON_EVENT_ESTABLISH_CONECTION, NULL);
    _process_loop.push_event(event);
    event->unref();
    return 0;
}

int Mina::mina_destroy_connection()
{
    MinaCommonEvent* event = new MinaCommonEvent\
        (this, MINA_COMMON_EVENT_DESTROY_CONECTION, NULL);
    _process_loop.push_event(event);
    event->unref();
    return 0;
}

int Mina::mina_set_server_ip(const string& server_ip)
{
    if(server_ip == mina_get_server_ip())
    {
        return 0;
    }
    _db_serveripinfo.set_serverip_info(server_ip);
    if(_connected)
    {
        _app->logic_reset_status_machine();
    }
    return 0;
}

int Mina::mina_save_server_ip(const string& server_ip)
{
    if(server_ip == mina_get_server_ip())
    {
        return 0;
    }
    _db_serveripinfo.set_serverip_info(server_ip);
    return 0;
}

const string& Mina::mina_get_server_ip()
{
    return _db_serveripinfo.get_serverip_info();
}

int Mina::mina_set_last_server_ip(const string& last_server_ip)
{
    if(last_server_ip == mina_get_last_server_ip())
    {
        return 0;
    }
    _db_serveripinfo.set_last_serverip_info(last_server_ip);
    return 0;
}

const string& Mina::mina_get_last_server_ip()
{
    return _db_serveripinfo.get_last_serverip_info();
}


#define JSON_WRITE_PARSE_INIT(json, json_string)\
    string json_string;\
    char* json_text;\
    cJSON * json = cJSON_CreateObject();

#define JSON_WRITE_PARSE_FREE(json, handle)\
    json_text = cJSON_PrintUnformatted(json);\
    json_string = json_text;\
    LOG_DEBUG("create json_text \n%s", json_text);\
    SendMessageEvent* event = new SendMessageEvent(this, handle, json_string);\
    _process_loop.push_event(event);\
    event->unref();\
    if(json_text)\
    {\
        free(json_text);\
    }\
    cJSON_Delete(json);

#define STRUCT_WRITE_PARSE_FREE(data, handle)\
    LOG_DEBUG("create struct handle:%d", handle);\
    SendMessageStructEvent* event = new SendMessageStructEvent(this, handle, data);\
    _process_loop.push_event(event);\
    event->unref();\
    if (data) \
    {\
        free(data);\
    }\

int Mina::mina_web_request_software_version()
{
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", 0);
    
    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_SOFTWARE_VERSION);
    return 0;
}

int Mina::mina_web_request_terminal_password()
{
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", 0);

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_TERMINAL_PASSWD);
    return 0;
}


int Mina::mina_web_request_public_policy()
{
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", 0);
    
    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_PUBLIC_POLICY);
    return 0;
}

int Mina::mina_web_request_hostname(const string& hostname)
{
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", 0);
    
    cJSON_AddStringToObject(json, "hostname", hostname.c_str());

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_HOSTNAME);
    return 0;
}

int Mina::mina_web_request_mode(const ModeInfo& info)
{
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", 0);
    
    
    cJSON_AddNumberToObject(json, "mode", info.mode);

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_MODE);
    
	return 0;
}

int Mina::mina_web_request_printer_switch()
{
    LOG_DEBUG("mina_web_request_printer_switch");

    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", 0);

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_PRINTER_SWITCH);

    return 0;
}

int Mina::mina_web_request_desktop_redir()
{
    LOG_DEBUG("mina_web_request_desktop_redir");

    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", 0);

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_DESKTOP_REDIR);

    return 0;
}

int Mina::mina_web_request_dev_policy(const int mode, const string& bind_user)
{
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", 0);
    cJSON_AddNumberToObject(json, "mode", mode);
    cJSON_AddStringToObject(json, "bind_user", (gloox::password_codec_xor(bind_user, true)).c_str());
    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_REQ_DEV_POLICY);
    return 0;
}

int Mina::mina_web_request_driver_install()
{
    JSON_WRITE_PARSE_INIT(json, json_string);
    cJSON_AddNumberToObject(json, "timestamp", 0);
    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_DRIVER_INSTALL);
    return 0;
}

int Mina::mina_web_request_server_time()
{
    LOG_DEBUG("mina_web_request_server_time in");
    JSON_WRITE_PARSE_INIT(json, json_string);
    cJSON_AddNumberToObject(json, "timestamp", 0);
    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_SERVER_TIME);

    return 0;
}

int Mina::mina_web_request_main_window()
{
    LOG_DEBUG("mina_web_request_main_window");
    JSON_WRITE_PARSE_INIT(json, json_string);
    cJSON_AddNumberToObject(json, "timestamp", 0);
    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_MAIN_WINDOW);

    return 0;
}

int Mina::mina_web_request_dev_interface_info(const string& product_id)
{
    JSON_WRITE_PARSE_INIT(json, json_string);
    cJSON_AddNumberToObject(json, "timestamp", 0);
    cJSON_AddStringToObject(json, "product_id", product_id.c_str());
    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_REQ_DEV_INTERFACE_INFO);
    return 0;
}

int Mina::mina_web_request_reload_image()
{
    JSON_WRITE_PARSE_INIT(json, json_string);
        
    cJSON_AddNumberToObject(json, "timestamp", 0);


    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_RELOAD_IMAGE);
    return 0;
}
int Mina::mina_web_request_delete_teacherdisk()
{
    JSON_WRITE_PARSE_INIT(json, json_string);
        
    cJSON_AddNumberToObject(json, "timestamp", 0);


    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_DELETE_TEACHERDISK);
    return 0;
}    

int Mina::mina_web_request_image(const ImageInfo& info)
{
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", 0);
    
    cJSON_AddNumberToObject(json, "image_id",       info.id);
    cJSON_AddStringToObject(json, "image_name",     info.name.c_str());
    cJSON_AddStringToObject(json, "image_version",  info.version.c_str());
    cJSON_AddStringToObject(json, "image_md5",      info.md5.c_str());
    cJSON_AddStringToObject(json, "ostype",         info.ostype.c_str());

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_IMAGE);
    return 0;
}

int Mina::mina_web_request_ipxe()
{
    JSON_WRITE_PARSE_INIT(json, json_string);
    
    cJSON_AddNumberToObject(json, "timestamp", 0);


    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_IPXE);
    return 0;
}

int Mina::mina_web_request_recover_image()
{
    JSON_WRITE_PARSE_INIT(json, json_string);
        
    cJSON_AddNumberToObject(json, "timestamp", 0);


    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_RECOVER_IMAGE);
    return 0;
}

int Mina::mina_web_request_reset_terminal()
{
    JSON_WRITE_PARSE_INIT(json, json_string);
        
    cJSON_AddNumberToObject(json, "timestamp", 0);


    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_RESET_TERMINAL);
    return 0;
}

int Mina::mina_web_request_check_server_ip(const string& server_ip)
{
    JSON_WRITE_PARSE_INIT(json, json_string);
        
    cJSON_AddNumberToObject(json, "timestamp", 0);

    cJSON_AddStringToObject(json, "server_ip",     server_ip.c_str());

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_SERVER_IP);
    return 0;
}

int Mina::mina_web_request_all_userinfo(const std::vector<string>& usernames)
{
    JSON_WRITE_PARSE_INIT(json, json_string);
        
    cJSON_AddNumberToObject(json, "timestamp", 0);

    cJSON * username_array = cJSON_CreateArray();
    unsigned int i = 0;
    for(i = 0; i < usernames.size(); i++)
    {
        cJSON_AddStringToObject(username_array, "usernames", (gloox::password_codec_xor((usernames[i]).c_str(), true)).c_str());
    }
    cJSON_AddItemToObject(json, "usernames", username_array);
    //cJSON_Delete(username_array); //XXX: it will be deleted later while json root being deleted.

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_ALL_USERINFO);
    return 0;
}

int Mina::mina_web_request_port_mapping()
{
    JSON_WRITE_PARSE_INIT(json, json_string);
        
    cJSON_AddNumberToObject(json, "timestamp", 0);

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_PORT_MAPPING);
    return 0;
}

int Mina::mina_web_request_ssid_whitelist()
{
    LOG_DEBUG("mina_web_request_ssid_whitelist");
    
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", 0);
    
    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_REQUEST_SSID);

    return 0;
}

int Mina::mina_web_upload_unknow_devinfo(const struct Unknown_UsbInfo &undev_info)
{
    LOG_DEBUG("mina_web_upload_unknow_devinfo");

    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp",        0);

    cJSON_AddNumberToObject(json, "idv",      undev_info.idv);

    cJSON_AddNumberToObject(json, "pid",      undev_info.pid);

    cJSON_AddNumberToObject(json, "bcd",      undev_info.bcd);

    cJSON_AddStringToObject(json, "manufacturer",      undev_info.manufacturer.c_str());

    cJSON_AddStringToObject(json, "product",      undev_info.product.c_str());

    cJSON_AddStringToObject(json, "serial",      undev_info.serial.c_str());

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_UNKNOW_DEV);

    return 0;
}

int Mina::mina_web_request_rcdusbconf()
{
    LOG_DEBUG("mina_web_request_rcdusbconf");

    JSON_WRITE_PARSE_INIT(json, json_string);
        
    cJSON_AddNumberToObject(json, "timestamp", 0);

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_REQUEST_USBCONF);

    return 0;
}

/**
 * function:send server type request
 * rcc handle: 0x0181
 */
int Mina::mina_web_check_server_type(void)
{
    msg_sync_type_t *buf = NULL;
    char *msg = NULL;
    int length;

    length = sizeof(msg_sync_type_t);
    msg = (char *)malloc(length);
    if (msg == NULL) {
        LOG_WARNING("malloc error");
        return -1;
    }
    memset(msg, 0, length);

    buf = (msg_sync_type_t *)msg;
    buf->head.handle = htons(RCD_HANDLE_SYNC_SERVER_TYPE);
    buf->head.length = htonl(sizeof(buf->action));
    buf->action      = htons(105);

    LOG_DEBUG("send server type request");
    STRUCT_WRITE_PARSE_FREE(msg, RCD_HANDLE_SYNC_SERVER_TYPE);
    return 0;
}

/**
 * function:send download path request
 * rcc handle: 0x0182
 */
int Mina::mina_web_request_deb_path(void)
{
    msg_sync_path_t *buf = NULL;
    char *msg =NULL;
    int length;

    length = sizeof(msg_sync_path_t);
    msg = (char *)malloc(length);
    if (msg == NULL) {
        LOG_WARNING("malloc error");
        return -1;
    }
    memset(msg, 0, length);

    buf = (msg_sync_path_t *)msg;
    buf->head.handle = htons(RCD_HANDLE_SYNC_DEB_PATH);
    buf->head.length = htonl(sizeof(buf->action));
    buf->action      = htons(101);

    LOG_DEBUG("send download path request");
    STRUCT_WRITE_PARSE_FREE(msg, RCD_HANDLE_SYNC_DEB_PATH);
    return 0;
}

int Mina::mina_web_upload_basic_info(const BasicInfo& info, const NetworkInfo& net, const string& wireless_mac, \
          const string& wired_mac, int net_type, const string& hostname,const SystemHardwareInfo& hardInfo)
{
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", 0);
    
        
    cJSON_AddStringToObject(json, "serial_number",      info.serial_number.c_str());
    cJSON_AddStringToObject(json, "product_id",         info.product_id.c_str());
    cJSON_AddStringToObject(json, "mac",                info.mac.c_str());
    cJSON_AddStringToObject(json, "software_version",   info.software_version.c_str());
    cJSON_AddStringToObject(json, "hardware_version",   info.hardware_version.c_str());
    cJSON_AddStringToObject(json, "os_version",         info.os_version.c_str());
    cJSON_AddStringToObject(json, "cpu",                info.cpu.c_str());
    cJSON_AddStringToObject(json, "memory",             info.memory.c_str());
    //cJSON_AddStringToObject(json, "storage",            info.storage.c_str());
    cJSON_AddStringToObject(json, "hostName",           hostname.c_str());
    cJSON_AddStringToObject(json, "storage",        hardInfo.disk_size.c_str());
    cJSON_AddBoolToObject(json, "wifi_function",      hardInfo.wifi_function);
    cJSON_AddStringToObject(json, "wifi_mac",           wireless_mac.c_str());
    cJSON_AddNumberToObject(json, "net_type",           net_type);
    if(rc_check_input_ip(net.ip.c_str()) == 0)
    {
        LOG_DEBUG("send web ip");
        cJSON_AddStringToObject(json, "ip",                 net.ip.c_str());
    }
    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_UPLOAD_BASICINFO);
    return 0;
}

int Mina::mina_web_upload_local_network_info(const NetworkInfo& info)
{
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", 0);
    
    
    cJSON_AddBoolToObject(json,     "dhcp",         info.dhcp);
    cJSON_AddStringToObject(json,   "ip",           info.ip.c_str());
    cJSON_AddStringToObject(json,   "submask",      info.submask.c_str());
    cJSON_AddStringToObject(json,   "gateway",      info.gateway.c_str());
    cJSON_AddBoolToObject(json,     "auto_dns",     info.auto_dns);
    cJSON_AddStringToObject(json,   "main_dns",     info.main_dns.c_str());
    cJSON_AddStringToObject(json,   "back_dns",     info.back_dns.c_str());

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_UPLOAD_LOCAL_NETWORK);
    return 0;
}

int Mina::mina_web_upload_vm_network_info(const NetworkInfo& info, const string& vm_mac)
{
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", 0);
    
        
    cJSON_AddBoolToObject(json,     "dhcp",         info.dhcp);
    cJSON_AddStringToObject(json,   "ip",           info.ip.c_str());
    cJSON_AddStringToObject(json,   "submask",      info.submask.c_str());
    cJSON_AddStringToObject(json,   "gateway",      info.gateway.c_str());
    cJSON_AddBoolToObject(json,     "auto_dns",     info.auto_dns);
    cJSON_AddStringToObject(json,   "main_dns",     info.main_dns.c_str());
    cJSON_AddStringToObject(json,   "back_dns",     info.back_dns.c_str());

    cJSON_AddStringToObject(json,   "vm_mac",       vm_mac.c_str());

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_UPLOAD_VM_NETWORK);
    return 0;
}

int Mina::mina_web_upload_download_info(const DownloadInfo& info)
{
    int tmp = 0;//to record value send to web
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", 0);
    
    if(info.status == DOWNLOAD_IMAGE_DOWNLOADING)
    {
        tmp = 1;
    }
    else if (info.status == DOWNLOAD_IMAGE_SUCCESS)
    {
        tmp = 2;
    }
    else
    {
        tmp = 0;
    }

    cJSON_AddNumberToObject(json, "status",         tmp);
    cJSON_AddNumberToObject(json, "image_id",       info.image_id);
    cJSON_AddStringToObject(json, "image_name",     info.image_name.c_str());
    cJSON_AddStringToObject(json, "image_version",  info.image_version.c_str());
    cJSON_AddStringToObject(json, "percent",        info.percent.c_str());
    cJSON_AddStringToObject(json, "speed",          info.speed.c_str());

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_UPLOAD_DOWNLOADINFO);
    return 0;
}

int Mina::mina_web_upload_vm_info(const VMInfo& vm_info, const UserInfo& user_info)
{
    JSON_WRITE_PARSE_INIT(json, json_string);
    
    cJSON_AddNumberToObject(json, "timestamp", 0);
    

    cJSON_AddBoolToObject(json, "running", vm_info.running);
    cJSON_AddNumberToObject(json, "vm_cpu", vm_info.cpu);
    cJSON_AddNumberToObject(json, "vm_memory_total", vm_info.memory_total);
    cJSON_AddNumberToObject(json, "vm_memory_use", vm_info.memory_use);
    cJSON_AddNumberToObject(json, "vm_storage_total", vm_info.storage_total);
    cJSON_AddNumberToObject(json, "vm_storage_use", vm_info.storage_use);
    cJSON_AddStringToObject(json, "username", (gloox::password_codec_xor(user_info.username, true)).c_str());
    cJSON_AddBoolToObject(json, "vm_dhcp", vm_info.vm_net.dhcp);
    cJSON_AddStringToObject(json, "vm_ip", vm_info.vm_net.ip.c_str());
    cJSON_AddStringToObject(json, "vm_submask", vm_info.vm_net.submask.c_str());
    cJSON_AddStringToObject(json, "vm_gateway", vm_info.vm_net.gateway.c_str());
    cJSON_AddBoolToObject(json, "vm_autodns", vm_info.vm_net.auto_dns);
    cJSON_AddStringToObject(json, "vm_maindns", vm_info.vm_net.main_dns.c_str());
    cJSON_AddStringToObject(json, "vm_backdns", vm_info.vm_net.back_dns.c_str());

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_UPLOAD_VMINFO);
    return 0;

}

int Mina::mina_web_upload_new_terminal_complete()
{
    JSON_WRITE_PARSE_INIT(json, json_string);
        
    cJSON_AddNumberToObject(json, "timestamp", 0);


    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_UPLOAD_NEW_TERMINAL_COMPLETE);
    return 0;

}

int Mina::mina_web_upload_driver_install_result(int error)
{
    JSON_WRITE_PARSE_INIT(json, json_string);
        
    cJSON_AddNumberToObject(json, "timestamp", 0);
    
    cJSON_AddNumberToObject(json, "error", error);
    cJSON_AddStringToObject(json, "msg", ErrorNote::get_instance()->error_notes[error].c_str());

    LOG_DEBUG("send mina_web_upload_driver_install_result");
    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_UPLOAD_DRIVER_INSTALL_RESULT);
    return 0;

}



int Mina::mina_web_login(const UserInfo& info)
{
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", 0);

    cJSON_AddStringToObject(json, "username", (gloox::password_codec_xor(info.username, true).c_str()));
    cJSON_AddStringToObject(json, "password", info.password.c_str());

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_LOGIN);
    return 0;
}

int Mina::mina_web_modify_password(const UserInfo& info)
{
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", 0);

    cJSON_AddStringToObject(json, "username",       (gloox::password_codec_xor(info.username, true)).c_str());
    cJSON_AddStringToObject(json, "password",       info.password.c_str());
    cJSON_AddStringToObject(json, "new_password",   info.new_password.c_str());

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_MODIFY_PASSWORD);
    return 0;
}

int Mina::mina_web_bind(const UserInfo& info)
{
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", 0);
    

    cJSON_AddStringToObject(json, "username", (gloox::password_codec_xor(info.username, true)).c_str());

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_BIND);
    return 0;
}

int Mina::mina_web_collect_log_complete(int status, const string& log_name)
{
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", 0);
    cJSON_AddNumberToObject(json, "status", status);
    cJSON_AddStringToObject(json, "log_name", log_name.c_str());

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_COLLECT_LOG_COMPLETE);
    return 0;

}
int Mina::mina_l2w_change_mode(Mode mode)
{
    JSON_WRITE_PARSE_INIT(json, json_string);
    
    cJSON_AddNumberToObject(json, "timestamp", 0);


    cJSON_AddNumberToObject(json, "mode", mode);

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_CHANGE_MODE);
    return 0;
}

int Mina::mina_l2w_notify_partition(int c_disk_size, int d_disk_size, int max_c_disk_size)
{
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", 0);


    cJSON_AddNumberToObject(json, "c_disk_size", c_disk_size);
    cJSON_AddNumberToObject(json, "d_disk_size", d_disk_size);
    cJSON_AddNumberToObject(json, "max_c_disk_size", max_c_disk_size);

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_NOTIFY_PARTITION);
    return 0;
}

int Mina::mina_l2w_notify_set_devpolicy(int id, bool set_recovery, bool allow_recovery, bool set_userdisk, bool allow_userdisk)
{
    JSON_WRITE_PARSE_INIT(json, json_string);

	cJSON_AddNumberToObject(json, "timestamp", id);
	cJSON_AddNumberToObject(json, "error", 0);
    cJSON_AddNumberToObject(json, "recovery_set_valid", set_recovery);
    cJSON_AddNumberToObject(json, "allow_recovery", allow_recovery);
    if (set_recovery) {
    	cJSON_AddStringToObject(json, "recovery_set_msg", "属性设置成功");
    } else {
    	cJSON_AddStringToObject(json, "recovery_set_msg", "设备锁定，无法设置策略");
    }

    cJSON_AddNumberToObject(json, "userdisk_set_valid", set_userdisk);
    cJSON_AddNumberToObject(json, "allow_userdisk", allow_userdisk);
    if (set_userdisk) {
    	cJSON_AddStringToObject(json, "userdisk_set_msg", "属性设置成功");
    } else {
    	cJSON_AddStringToObject(json, "userdisk_set_msg", "设备锁定，无法设置策略");
    }
    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SYNC_DEV_POLICY);
    return 0;
}

int Mina::mina_l2w_change_hostname(const string& hostname)
{
    JSON_WRITE_PARSE_INIT(json, json_string);
        
    cJSON_AddNumberToObject(json, "timestamp", 0);


    cJSON_AddStringToObject(json, "hostname", hostname.c_str());

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_CHANGE_HOSTNAME);
    return 0;
}

int Mina::mina_guesttool_msg(const string& guesttool_msg, int source, int target, int module_id)
{
    JSON_WRITE_PARSE_INIT(json, json_string);
            
    cJSON_AddNumberToObject(json, "timestamp", 0);

    cJSON_AddNumberToObject(json, "source",         source);
    cJSON_AddNumberToObject(json, "target",         target);
    cJSON_AddNumberToObject(json, "module_id",      module_id);
    cJSON_AddStringToObject(json, "guesttool_msg",  guesttool_msg.c_str());

    JSON_WRITE_PARSE_FREE(json, RCD_HANDLE_SEND_GUESTTOOL_MSG);
    return 0;

}


void Mina::send_web_ack(int handle, int error, string error_msg, int timestamp)
{
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", timestamp);
    cJSON_AddNumberToObject(json, "error", error);
    cJSON_AddStringToObject(json, "msg", error_msg.c_str());
    JSON_WRITE_PARSE_FREE(json, handle);
}

