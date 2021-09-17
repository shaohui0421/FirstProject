/******************************************************************************
 * Copyright (C) 2017
 * file:    Vmmessage.h
 * author:  lijun <li_jun@ruijie.com.cn>
 * created: 2017-02-07 10:05
 * updated: 
 * author:  lijun <li_jun@ruijie.com.cn>
 * date:	2017-02-16
 ******************************************************************************/


#ifndef _VMMESSAGE_H
#define _VMMESSAGE_H

#include "cJSON.h"
#include "common.h"
#include "process_loop.h"
#include "application.h"
#include "data_struct.h"

#include <map>
#include <string>

#include "net_api.h"

using std::string;

#define GUEST_OK_ID             35
#define ALIVE_ID                135
#define GUESTTOOL_TYPE          2
#define TERMINAL_IDV_TYPE       4
#define HEAD_LENGTH             18

#define GUEST_NET_ID            160
#define GUEST_PUSHINSTALL_ID    137
#define GUEST_DEV_ID            180
#define GUEST_ZHJS_ID            579

template <class HandlerClass>
class GuestMessageHandler
{
    typedef void (HandlerClass::*Handler)(int id, string data);
    typedef std::map<int, Handler> Handlers;
public:
    GuestMessageHandler(HandlerClass& obj) :_obj (obj) {}
    virtual ~GuestMessageHandler() {}

protected:
    void set_handler(int hd, Handler handle)
    {
        _handlers[hd] = handle;
    }

    //handle a readable fd,return false if it comes into an error while receiving
    bool handle_message(int fd)
    {
        int id;
		int hd;
        string buf;
        if(false == recv_message(fd, id, hd, buf))
        {
            return false;
        }
        else
        {
            (_obj.*_handlers[hd])(id, buf);
            return true;
        }
    }

    virtual bool recv_message(int fd, int& id, int& hd, string& buf){return false;}
private:
    HandlerClass& _obj;
    Handlers _handlers;

};

bool send_message(int id, int handle, const string& data, int sock);

// handle for client & guest
enum GuestHandle
{
    GUEST_KEEPALIVE					=   1,
    
    GUEST_GET_VM_INFO				=   3,
    GUEST_SET_NET_DISABLE			=   4,

	GUEST_SET_USB_POLOCY			=	9,
	GUEST_SET_NETWORK				=	10,
	GUEST_SET_SUNNY_INFO            =   20,

	GUEST_SET_DISKINFO				=	91,
	
	GUEST_GET_NETWORK				=	101,
	GUEST_GET_DISKINFO				=	102,

	GUEST_GET_VM_LOG				=	105,
    GUEST_QUERY_TIME                =   106,
    GUEST_LOGOUT                    =   109,
    GUEST_SEND_WIRED_MAC            =   110,
    GUEST_ADD_DIMM                  =   129,

    GUEST_SEND_WEB_INFO_TO_VM       =   200,
    GUEST_SEND_VM_INFO_TO_WEB       =   201,

    GUEST_TERMINAL_SYSTEM_MSG       =   210,

    GUEST_SEND_DEV_INFO_TO_VM       =   230,
    GUEST_SEND_VM_INFO_TO_DEV       =   231,

    GUEST_MODIFY_TERMINAL_IP        =   302,
    GUEST_REQUEST_GET_VM_IP         =   303,
    GUEST_REQUEST_GET_TERMINAL_IP   =   304,
    
    
    UPLOAD_DOWNLOAD_INFO_ZHJSGT   =   401,
    UPLOAD_NETWORK_INFO_ZHJSGT   =   402,
    GUEST_MERGE_INFO   =   403,

};

enum GuestConnect
{
    GUEST_DISCONNECT                =   0,
    GUEST_ON_CONNECT                =   1,
    GUEST_ON_CONTINUE               =   2,
};
#define vmmessage_read_int(dest, src)\
    memcpy(&dest, src, sizeof(int));\
    //dest = ntohl(dest);

#define vmmessage_read_short(dest, src)\
    memcpy(&dest, src, sizeof(short));\
    //dest = ntohs(dest);

#define vmmessage_read_64int(dest, src)\
    memcpy(&dest, src, sizeof(long long));\
    //dest = ntohll(dest);

#define vmmessage_read_char(dest, src, size)\
    strncpy(dest, (char*)src, size);

class Vmmessage_Thread
{
public:
    static void* vmmessage_thread(void* arg);
    static void callback_cb(int sock, short event, void* arg);

private:

};

class VM;
class Vmmessage;
typedef GuestMessageHandler<Vmmessage> VmmessageMessageHandler;

class Vmmessage:public EventSource, public VmmessageMessageHandler
{
	class KeepAliveTimer;
	class SendMessageEvent;
	enum Interval
	{
		KEEPALIVE_INTERVAL = 10*1000,
	};

public:
    //Vmmessage(Application* app, VM* vm);
    Vmmessage();
    virtual ~Vmmessage();
//    bool message_send_test();
    //bool vmmessage_get_connected() {return _connected;}

    void init_read_event_thread();

    virtual void action();
    void message_upload_vm_info();
    void message_upload_vm_network_info(NetworkInfo& networkinfo, int id = ALIVE_ID);
    void message_upload_local_network_info(NetworkInfo& networkinfo, int id= GUEST_NET_ID);
    void message_upload_network_info_to_zhjsgt(string ip, string net_mode);
    void message_upload_download_info_to_zhjsgt(string downloaded_size,string total_size,string percent,string rate);
    void message_upload_netdisk(NetdiskInfo& netdiskinfo);
    void message_upload_sunny_info();
    void message_upload_web_info(const string &web_msg, int id, int target);
    void message_update_vm_time();
    void message_set_guest_netdisable();
    void message_set_guest_merge_tips();
    void message_set_guest_usb_polocy(string& usb_policyinfo);
    void message_set_guest_disconnected();
    void message_terminal_system_msg(const string &msg);
    void message_terminal_device_msg(const string &msg);
    void message_send_wired_mac();
    static Vmmessage *_vmmessage_self;
    static Vmmessage *get_vmmessage_self();

    static void on_network_callback(string &msg, void *user);
    static void on_wifi_callback(string &msg, void *user);

    int _connected_to_guest; 
    int _keep_alive_time;
    VMInfo _vminfo;
    string _usb_policyinfo;
    NetworkInfo _networkinfo;
    NetdiskInfo _netdiskinfo;

    net::NetworkManager* _network_manager;
    net::WifiManager* _wifi_manager;
    struct event_base* get_event_base(void);

private:
    Mutex _send_mutex;
    static void* thread_main(void* data);
    void shake_hand();
    bool establish_connection();

    static void* init_read_event(void* data);
    static void on_read(int sock, short event, void* arg);

	// add get_json

	//
    void handle_keepalive(string data);

	void on_establish_connection();
    void on_destroy_connection();

    void destroy_connection();
    bool send_message(int id, int handle, const string& data, int target = GUESTTOOL_TYPE);
    virtual bool recv_message(int fd, int& id, int& hd, string& buf);

//    void data_handle(short handle, char* data);
//    void handle_keepalive();

	//static client_message_head_t _client_message;
    void handle_set_usb_policy(string data);

    ProcessLoop _process_loop;

    KeepAliveTimer* _keepalive_timer;
    //Application* _app;
    //VM* _vm;
	UnjoinableThread* _thread;
    // int _fd;
    // bool _connected;
	// int _guest_status;
    static struct event_base* _base;

    class KeepAliveTimer:public Timer
    {
    public:
    	KeepAliveTimer(Vmmessage* vmmessage):_vmmessage(vmmessage) {}
    	virtual ~KeepAliveTimer(){}
    	virtual void response()
    	{
    		_vmmessage->shake_hand();
    		//if(_vmmessage->_keepalive++ >= 3)
    		{
    			//LOG_INFO("keep vmmessage alive is beyond 3, so destroy connect");
    			//_vmmessage->destroy_connection();
    		}
    	}

    private:
    	Vmmessage* _vmmessage;
    };

    class SendMessageEvent:public Event
    {
    public:
    	SendMessageEvent(Vmmessage* vmmessage, int id, int handle, string data)
    		:_vmmessage (vmmessage)
    		,_id (id)
    		,_handle (handle)
    		,_data (data)
    	{
    	}
    	virtual ~SendMessageEvent()
    	{
    	}
    	virtual void response()
    	{
    		_vmmessage->send_message(_id, _handle, _data);
    	}
    private:
    	Vmmessage* _vmmessage;
    	int _id;
    	int _handle;
    	string _data;
    };

};

#endif //_VMMESSAGE_H

