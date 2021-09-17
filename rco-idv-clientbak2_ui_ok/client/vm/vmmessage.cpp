/******************************************************************************
 * Copyright (C) 2017
 * file:    Vmmessage.cpp
 * author:  lijun <li_jun@ruijie.com.cn>
 * created: 2017-02-07 10:05
 * updated: 
 * author:  lijun <li_jun@ruijie.com.cn>
 * date:	2017-02-16
 ******************************************************************************/

#include "vmmessage.h"
#include "vm_common.h"

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <sys/un.h>
#include "rc_json.h"
#ifdef UNIT_TEST
#define	RCC_LOG_VMMESSAGE	"/var/log/vm_message_client.log"
#endif /* UNIT_TEST */
#define DEBUG_DETAIL    0

#define CLIENT_CHANNEL_PATH  "/tmp/app_channel.sock"
#define BUF_SIZE 16
#define HEAD_BUF_SIZE 	18
#define VERSION_VM_MESSAGE	1
#define SHAKE_TIMEOUT	3

// for Guesttool log record.
#define GUEST_LOGFILENAMESZ 128
#define GUEST_LOGFILE_MAXSZ (4 << 20)

//VMInfo vminfo_t;
NetworkInfo network_info_t;
NetdiskInfo netdisk_info_t;
PolicyInfo policy_info_t;
//int connect_to_guest;
//VMInfo Vmmessage::_vminfo;
struct event_base* Vmmessage::_base = NULL;

// get json to data
#define GET_JSON_JUDGE_NULL_INIT(pMsg, dst, json, key)\
		pMsg = cJSON_GetObjectItem (json, key.c_str());\
		if(pMsg == NULL)\
		{\
			LOG_WARNING("can not find json key %s", key.c_str());\
			return;\
		}
	
	void get_json_valuebool(bool& dst, cJSON* json, const string& key)
	{
		cJSON* pMsg = NULL;
		GET_JSON_JUDGE_NULL_INIT(pMsg, dst, json, key);
		dst = pMsg->valueint;
	}
	void get_json_valueint(int& dst, cJSON* json, const string& key)
	{
		cJSON* pMsg = NULL;
		GET_JSON_JUDGE_NULL_INIT(pMsg, dst, json, key);
		dst = pMsg->valueint;
	}
	void get_json_valuestring(string& dst, cJSON* json, const string& key)
	{
		cJSON* pMsg = NULL;
		GET_JSON_JUDGE_NULL_INIT(pMsg, dst, json, key);
		dst = pMsg->valuestring;
	}
	void get_json_child(string& dst, cJSON* json, const string& key)
	{
		cJSON* pMsg = NULL;
		GET_JSON_JUDGE_NULL_INIT(pMsg, dst, json, key);
		dst = cJSON_PrintUnformatted(pMsg);
	}
#undef GET_JSON_JUDGE_NULL_INIT

Vmmessage *Vmmessage::_vmmessage_self = NULL;
Vmmessage *Vmmessage::get_vmmessage_self()
{
	Application* app = Application::get_application();
    VM* vm = app->get_vm();
    return vm->get_vmMessage();
}

void Vmmessage::on_network_callback(string &msg, void *user)
{
    Vmmessage *vmmessage_self = get_vmmessage_self();
    vmmessage_self->message_terminal_system_msg(msg);
}

void Vmmessage::on_wifi_callback(string &msg, void *user)
{
    Vmmessage *vmmessage_self = get_vmmessage_self();
    vmmessage_self->message_terminal_system_msg(msg);
}

static void guesttool_write_logfile(string &logfile, string &content)
{
    char filename[GUEST_LOGFILENAMESZ];
    char filebak[GUEST_LOGFILENAMESZ];
    FILE *fp;
    time_t now;
    struct tm *pnowtime;
    char tmpbuf[128] = {0};
    long file_size = 0;

    snprintf(filename, GUEST_LOGFILENAMESZ, "/var/log/%s", logfile.c_str());
    fp = fopen(filename, "a");
    if (fp == NULL) {
        LOG_PERROR("Failed to open or create guesttool log file: %s", logfile.c_str());
        return;
    }

    time(&now);
    pnowtime = localtime(&now);
    strftime(tmpbuf, sizeof(tmpbuf), "%Y-%m-%d %H:%M:%S", pnowtime);
    fprintf(fp, "[%s] %s\n", tmpbuf, content.c_str());
    fflush(fp);
    file_size = ftell(fp);
    fclose(fp);

    if (file_size > GUEST_LOGFILE_MAXSZ) {
        LOG_INFO("Guesttool logfile %s beyond max size %d, override the old one.",
            logfile.c_str(), GUEST_LOGFILENAMESZ);
        snprintf(filebak, GUEST_LOGFILENAMESZ, "/var/log/%s.bak", logfile.c_str());
        rename(filename, filebak);
    }
}

void Vmmessage_Thread::callback_cb(int sock, short event, void *arg)
{
	char recv_buffer[HEAD_BUF_SIZE];
	char* data = NULL;
	string tmp = "";
	int ret = 0;
	int length = 0;
	short handle = 0;
	short id_tmp = 0;
    int result;
    int target = 0;
    int source = 0;
    int status = 0;
    string vm_ip = "";
    string guesttool_filename, logdata;
	VM *_vm_t = Application::get_vm();
	//VM *_vm_t = VM::get_vmself();
	Vmmessage*_vmmessage_t = Vmmessage::get_vmmessage_self();
    Application *app = Application::get_application();

    /**
     * libevent will trigger on_read() callback only if the port could be read,
     * so it does not need to use while loop here.
     */
	//LOG_DEBUG("START ON READ");
	memset(recv_buffer, 0 , sizeof(recv_buffer));
	ret = recv(sock, recv_buffer, sizeof(short), MSG_WAITALL);//version
	ret = recv(sock, recv_buffer, sizeof(short), MSG_WAITALL);//target
	vmmessage_read_short(target, recv_buffer);
	ret = recv(sock, recv_buffer, sizeof(short), MSG_WAITALL);//head_length
	ret = recv(sock, recv_buffer, sizeof(short), MSG_WAITALL);//id
	vmmessage_read_short(id_tmp, recv_buffer);
	if(ret <= 0) {
        LOG_PERROR("vmmessage recv error! ret=%d.", ret);
        struct event_base* base = _vmmessage_t->get_event_base();
        if ((ret == 0) && (base != NULL)) {
            LOG_WARNING("vmmessage close event loop");
            event_base_loopbreak(base);
        }
        return;
	}
	ret = recv(sock, recv_buffer, sizeof(short), MSG_WAITALL);//handle
	vmmessage_read_short(handle, recv_buffer);
	ret = recv(sock, recv_buffer, sizeof(int), MSG_WAITALL);//length
	vmmessage_read_int(length, recv_buffer);
	ret = recv(sock, recv_buffer, sizeof(short), MSG_WAITALL);//source
	vmmessage_read_short(source, recv_buffer);
    ret = recv(sock, recv_buffer, sizeof(short), MSG_WAITALL);//reserve
#if DEBUG_DETAIL
	LOG_DEBUG("recv Head completely, id = %d, handle = %d, length = %d", id_tmp, handle, length);
#endif
	if (length > 0) {
		data = new char[length+1];
		memset(data, 0, length+1);
		ret = recv(sock, data, length, MSG_WAITALL);//length

        if (id_tmp == GUEST_NET_ID && handle == GUEST_TERMINAL_SYSTEM_MSG) {
            //no need to print detail, see idv.log
		    LOG_DEBUG("recv vm json [%d:%d]", id_tmp, handle);
        } else {
		    LOG_DEBUG("recv vm json [%d:%d] %s", id_tmp, handle, data);
        }
	}

    //LOG_DEBUG("start check handle");
	cJSON* json_t = cJSON_CreateObject();
	switch (handle) {	
	case GUEST_MERGE_INFO:
	    if (id_tmp==GUEST_ZHJS_ID) {
	        json_t = cJSON_Parse(data);
    		get_json_valueint(result, json_t, "IsUpdate");
            if (result==1){
    		    app->v2l_reboot_system_merge_image();
    		}
    		else{
    		    app->v2l_clean_download_success_tips();
    		}
	    }
	    break;
	case GUEST_KEEPALIVE:
		LOG_DEBUG("GUEST IS ALIVED");
		_vmmessage_t->_keep_alive_time = 0;
		if (_vmmessage_t->_connected_to_guest == GUEST_DISCONNECT) {
			_vmmessage_t->_connected_to_guest = GUEST_ON_CONNECT;

            // send VM info to Geusttool.
			if (!_vm_t->get_net_policy()) {
				_vmmessage_t->message_set_guest_netdisable();
                _vm_t->vm_disable_netuse(); // disable VM netcard.
			//} else if (!app->is_dev_network_up()) {
			/*注释，x盘问题，在线登录虚机时,拔插网线，后面再插入网线，x盘没有出来*/
			//    //device network down, only disable VM netcard.
			//    _vm_t->vm_disable_netuse();
			} else {
                if (app->get_nat_policy()) {
                    _vm_t->switch_nat();
                } else {
                    _vm_t->switch_bridge();
                }
                _vm_t->vm_enable_netuse();
				_vmmessage_t->_networkinfo = _vm_t->vm_get_vm_netinfo();
				_vmmessage_t->message_upload_vm_network_info(_vmmessage_t->_networkinfo);
				sleep(1);
				if (_vm_t->get_is_netdisk()) {
					_vmmessage_t->_netdiskinfo = _vm_t->get_netdisk_info();
					_vmmessage_t->message_upload_netdisk(_vmmessage_t->_netdiskinfo);
					sleep(1);
				}
                _vmmessage_t->message_upload_sunny_info();
                sleep(1);
			}
			_vmmessage_t->_usb_policyinfo = _vm_t->get_usb_policy();
			_vmmessage_t->message_set_guest_usb_polocy(_vmmessage_t->_usb_policyinfo);
			sleep(1);
		} else if (_vmmessage_t->_connected_to_guest == GUEST_ON_CONNECT) {
			_vmmessage_t->_connected_to_guest = GUEST_ON_CONTINUE;
		}
		break;
	case GUEST_GET_VM_INFO:
		LOG_DEBUG("GUEST_GET_VM_INFO");
        {
            string str_tmp;
    		json_t = cJSON_Parse(data);
    		// the data will be received from VM
    		_vmmessage_t->_vminfo.running = true;
    		get_json_valueint(_vmmessage_t->_vminfo.cpu, json_t, "cpu-use");
    		get_json_valueint(_vmmessage_t->_vminfo.memory_total, json_t, "mem-MBs");
    		get_json_valueint(_vmmessage_t->_vminfo.memory_use, json_t, "mem-use-MBs");
    		get_json_valueint(_vmmessage_t->_vminfo.storage_total, json_t, "storage-MBs");
    		get_json_valueint(_vmmessage_t->_vminfo.storage_use, json_t, "storage-use-MBs");
#if DEBUG_DETAIL
    		LOG_DEBUG("READ CPU MEMORY OK");
    		LOG_DEBUG("CPU using rate: %d%%.", _vmmessage_t->_vminfo.cpu);
            LOG_DEBUG("Memory: Total %dMB, uses %dMB.",
                _vmmessage_t->_vminfo.memory_total,
                _vmmessage_t->_vminfo.memory_use);
            LOG_DEBUG("Storage: Total %dMB, uses %dMB.\n",
                _vmmessage_t->_vminfo.storage_total,
                _vmmessage_t->_vminfo.storage_use);
#endif
            // upload vm ip info to web
            get_json_valuestring(str_tmp, json_t, "vm_dhcp");
            _vmmessage_t->_vminfo.vm_net.dhcp = (str_tmp == "true" ? true : false);
            get_json_valuestring(_vmmessage_t->_vminfo.vm_net.ip, json_t, "vm_ip");
            get_json_valuestring(_vmmessage_t->_vminfo.vm_net.submask, json_t, "vm_mask");
            get_json_valuestring(_vmmessage_t->_vminfo.vm_net.gateway, json_t, "vm_gate");
            get_json_valuestring(str_tmp, json_t, "vm_auto_dns");
            _vmmessage_t->_vminfo.vm_net.auto_dns = (str_tmp == "true" ? true : false);
            get_json_valuestring(_vmmessage_t->_vminfo.vm_net.main_dns, json_t, "vm_main_dns");
            get_json_valuestring(_vmmessage_t->_vminfo.vm_net.back_dns, json_t, "vm_back_dns");
            app->vm_get_vm_info_status(&_vmmessage_t->_vminfo);
        }
		break;
	case GUEST_SET_NET_DISABLE:
        json_t = cJSON_Parse(data);
        get_json_valueint(result, json_t, "result");
		LOG_DEBUG("GUEST_SET_NET_DISABLE result: %d", result);
        app->vm_set_net_policy_status(result);
		break;
	case GUEST_SET_USB_POLOCY:
        json_t = cJSON_Parse(data);
        get_json_valueint(result, json_t, "result");
		LOG_DEBUG("GUEST_SET_USB_POLOCY result: %d", result);
        app->vm_set_usb_policy_status(result);
		break;
	case GUEST_SET_NETWORK:
        json_t = cJSON_Parse(data);
        get_json_valueint(result, json_t, "result");
		LOG_DEBUG("GUEST_SET_NETWORK result: %d", result);
        app->vm_set_vm_netinfo_status(result);
		break;
    case GUEST_SET_SUNNY_INFO:
        json_t = cJSON_Parse(data);
        get_json_valueint(result, json_t, "result");
        LOG_DEBUG("GUEST_SET_SUNNY_INFO result: %d", result);
        //TODO: need to report result?
        break;
	case GUEST_GET_NETWORK:
		LOG_DEBUG("GUEST_GET_NETWORK RESULT");
		//TODO: report result.
		break;
	case GUEST_GET_DISKINFO:
		LOG_DEBUG("GUEST_GET_DISKINFO RESULT");
		//TODO: report result.
		break;
    case GUEST_SET_DISKINFO:
        LOG_DEBUG("GUEST_SET_DISKINFO RESULT");
        //TODO
        break;
	case GUEST_GET_VM_LOG:
#if DEBUG_DETAIL
        LOG_DEBUG("GET GUESTTOOL LOG DATA");
#endif
		json_t = cJSON_Parse(data);
		get_json_valuestring(guesttool_filename, json_t, "filename");
		get_json_valuestring(logdata, json_t, "logstr");
#if DEBUG_DETAIL
		LOG_DEBUG("guest log : filename: %s, log data: %s",
            guesttool_filename.c_str(), logdata.c_str());
#endif
		guesttool_write_logfile(guesttool_filename, logdata);
		break;

    case GUEST_SEND_VM_INFO_TO_WEB:
        LOG_DEBUG("GUESTTOOL send VM info to WEB");
        {
            // send data to web
            Application *app = Application::get_application();
            string vm_msg = data;
            if(app->get_server_connected())
                app->vm_send_guesttool_info(vm_msg, source, target, id_tmp);
            else
                app->vm_send_guesttool_info(vm_msg, source, target, id_tmp);
        }
        break;

    case GUEST_LOGOUT:
        LOG_DEBUG("GUESTTOOL get logout");
        {
            app->vm_logout(data);
        }
        break;

    case GUEST_SEND_WIRED_MAC:
        LOG_DEBUG("GUESTTOOL get wired mac address");
        _vmmessage_t->message_send_wired_mac();
        break;

    case GUEST_QUERY_TIME:
        LOG_DEBUG("GUEST_QUERY_TIME");
        _vmmessage_t->message_update_vm_time();
        break;

    case GUEST_ADD_DIMM:
        LOG_DEBUG("GUEST ADD DIMM");
        {
            string sh_result = VM_Common::execute_command("/usr/bin/add_dimm.sh");
            if (sh_result == "error") {
                LOG_ERR("excute add_dimm.sh fail");
            }
        }
        break;

    case GUEST_TERMINAL_SYSTEM_MSG:
        LOG_DEBUG("GUEST_TERMINAL_SYSTEM_MSG send VM info to terminal system");
        {
            string out_msg;
            string detail;
            string import;
            detail = RcJson::rc_json_get_child(data, "subjson");
            import = RcJson::rc_json_get_string(data, "import");
            if(import == "wifi_manager") {
                if (_vmmessage_t->_wifi_manager == NULL) {
                    LOG_ERR("wifi manager is NULL!!");
                    break;
                }
                _vmmessage_t->_wifi_manager->doJsonCommand(detail, out_msg);
            } else if(import == "network_manager") {
                if (_vmmessage_t->_network_manager == NULL) {
                    LOG_ERR("network manager is NULL!!");
                    break;
                }
                _vmmessage_t->_network_manager->doJsonCommand(detail, out_msg);
            }
            _vmmessage_t->message_terminal_system_msg(out_msg);
            status = _vmmessage_t->_network_manager->getStatus();
			//LOG_INFO("dchstatus  = %d  \n", status ); 
			if (status == (net::ETH_UP | net::WLAN_DOWN))
		    {
            	_vmmessage_t->message_upload_network_info_to_zhjsgt("0.0.0.0","eth");
		    }    
		    else if (status == (net::ETH_DOWN | net::WLAN_UP))
		    {    
            	vm_ip = VM_Common::execute_command("ifconfig  wlan0 |grep 'inet addr'|cut -d: -f2|cut -d' ' -f1");
            	_vmmessage_t->message_upload_network_info_to_zhjsgt(vm_ip, "wlan");
		    }
		    else{
            	_vmmessage_t->message_upload_network_info_to_zhjsgt("0.0.0.0","abnormal");
		    }
        }
    break;
#if 0
    case GUEST_SEND_DEV_INFO_TO_VM:
        LOG_DEBUG("GUEST_SEND_DEV_INFO_TO_VM");
        {
        }
        break;
    case GUEST_SEND_VM_INFO_TO_DEV:
        LOG_DEBUG("GUEST_SEND_VM_INFO_TO_DEV");
        {
            int handle_id;
            string detail;
            handle_id = RcJson::rc_json_get_int(data, "handle");
            app->get_device_interface()->doJsonCommand(handle_id, data);
        }
        break;
#endif
    case GUEST_MODIFY_TERMINAL_IP:
        LOG_DEBUG("GUEST_MODIFY_TERMINAL_IP");
        {
            NetworkInfo info;
            string type;
            json_t = cJSON_Parse(data);
            get_json_valuestring(type,          json_t, "type");
            get_json_valuestring(info.ip,       json_t, "ip");
            get_json_valuestring(info.submask,  json_t, "mask");
            get_json_valuestring(info.gateway,  json_t, "gateway");
            get_json_valuestring(info.main_dns, json_t, "dns1");
            get_json_valuestring(info.back_dns, json_t, "dns2");
            if(info.ip == "0.0.0.0")
            {
                info.dhcp = true;
            }
            else
            {
                info.dhcp = false;
            }
            if(info.main_dns == "0.0.0.0")
            {
                info.auto_dns = true;
            }
            else
            {
                info.auto_dns = false;
            }
            
            if(type == "terminal")
                app->vm_set_local_wired_network_info(info);
            if(type == "vm")
                _vm_t->vm_set_vm_netinfo_to_ini(info);
        }
    break;
    case GUEST_REQUEST_GET_VM_IP:
        LOG_DEBUG("GUEST_REQUEST_GET_VM_IP");
        {
            _vmmessage_t->_networkinfo = _vm_t->vm_get_vm_netinfo();
            _vmmessage_t->message_upload_vm_network_info(_vmmessage_t->_networkinfo, GUEST_NET_ID);
        }
    break;
    case GUEST_REQUEST_GET_TERMINAL_IP:
        LOG_DEBUG("GUEST_REQUEST_GET_TERMINAL_IP");
        {
            NetworkInfo info = app->get_local_network_data().get();
            _vmmessage_t->message_upload_local_network_info(info);
        }
    break;

	default:
		break;
	}

    //LOG_DEBUG("end the on read");
	if (length > 0)
		delete[] data;
	cJSON_Delete(json_t);
	//LOG_DEBUG("END ON READ");
}

void* Vmmessage_Thread::vmmessage_thread(void* arg)
{
	// because of the pipe of vmmessage cannot be destoryed, else the guesttool will not be connectted all the time
	// so the connection will be in the first time of openning the box, other time will not be reconnectted
		//Vmmessage* vmm_t = (Vmmessage*)data;
		Vmmessage*_vmmessage_t = Vmmessage::get_vmmessage_self();
		// LOG_INFO("sock = %d after\n", sock); 
		//-----init libevent，set callback on_read()------------
		struct event_base* base = event_base_new();
		if (!base){
			LOG_ERR("couldn't open event base\n");
			// return ;
		}
		// get data from socket after reading
		//LOG_DEBUG("READ BEFORE");
		struct event* read_ev = event_new(base, _vmmessage_t->get_fd(), EV_READ|EV_PERSIST, callback_cb, NULL);
	
		event_base_set(base, read_ev);
		event_add(read_ev, NULL);
		event_base_dispatch(base);
		//LOG_DEBUG("READ AFTER");	
		//--------------
		event_del(read_ev);
		free(read_ev);
		event_base_free(base);
		return NULL;
}

static inline void message_write_int(void *dest, int src)
{
	int tmp = src;
    //int tmp = htonl(src);
    memcpy(dest, &tmp, sizeof(int));
}

static inline void message_write_short(void *dest, short src)
{
	short tmp = src;
    //short tmp = htons(src);
    memcpy(dest, &tmp, sizeof(short));
}

#define message_write_char(dest, src, size) strncpy(dest, src, size)

Vmmessage::Vmmessage()
	:VmmessageMessageHandler (*this)
    ,_keep_alive_time (0)
    ,_process_loop (this)
    ,_keepalive_timer (new KeepAliveTimer(this))
{
	//_fd = INFINITE;
	// write
/*		set_handler(GUEST_VM_INFO,			&message_upload_vm_info);
	set_handler(GUEST_SET_NET_DISABLE,	&message_set_guest_netdisable);
	set_handler(GUEST_SET_USB_POLOCY,	&message_set_guest_usb_polocy);
	set_handler(GUEST_SET_NETWORK,		&message_upload_vm_network_info);
	set_handler(GUEST_SET_DISKINFO, 	&message_upload_netdisk);	*/
	// read
	//set_handler(GUEST_GET_NETWORK,		&Vmmessage::vm_get_guest_network);
	//set_handler(GUEST_GET_DISKINFO,		&Vmmessage::vm_get_guest_diskinfo);
	//set_handler(GUEST_SET_LOG,			&Vmmessage::vm_set_guest_log);

    //_app = Application::get_application();
    //_vm = VM::get_vmself();
	_process_loop.activate_interval_timer(_keepalive_timer, KEEPALIVE_INTERVAL);	
	_thread = new UnjoinableThread(Vmmessage::thread_main, this);

    _network_manager = NULL;
    _wifi_manager = NULL;
    _network_manager = new net::NetworkManager(on_network_callback, this);
    if (is_wifi_terminal()) {
        _wifi_manager = new net::WifiManager(on_wifi_callback, this);
    }
}

Vmmessage::~Vmmessage()
{
    destroy_connection();
	_process_loop.deactivate_interval_timer(_keepalive_timer);
	_keepalive_timer->unref();
	_thread->cancel();
	delete _thread;
    delete _keepalive_timer;
    if (_network_manager) {
        delete _network_manager;
    }
    if (_wifi_manager) {
        delete _wifi_manager;
    }
}

void* Vmmessage::thread_main(void* data)
{
	bool ret;
	Vmmessage* vmmessage = (Vmmessage*)data;
	do{
		ret = vmmessage->establish_connection();
		sleep(5);
	}
	while(false == ret);	
	vmmessage->init_read_event_thread();
	vmmessage->_process_loop.run();
	return NULL;
}


void Vmmessage::on_establish_connection()
{
	_connected_to_guest = GUEST_DISCONNECT;
    LOG_DEBUG("Establish message connection OK.");
}

void Vmmessage::on_destroy_connection()
{
	_connected_to_guest = GUEST_DISCONNECT;
	LOG_DEBUG("Destroy message connection OK.");
}

bool Vmmessage::establish_connection()
{
	//Vmmessage*_vmmessage_t = Vmmessage::get_vmmessageself();
	_fd = socket( AF_LOCAL, SOCK_STREAM, 0 );
	struct sockaddr_un client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	client_addr.sun_family = AF_LOCAL;
	strcpy(client_addr.sun_path, CLIENT_CHANNEL_PATH);

	if (_fd == -1) {
		LOG_PERROR("Failed to create socket for vmmessage communication!");
		return false;
	}
	int con_result;
	con_result = connect(_fd, (struct sockaddr*) &client_addr, sizeof(client_addr));
	if (con_result < 0){
		LOG_PERROR("Connect to vm-message server failed!");
		close(_fd);
		return false;
	}
	//	init_read_event_thread(socket_fd);
	LOG_DEBUG("establish connect");
	_process_loop.add_eventsource(*this);
	//_keepalive = 0;
	//_connected = false;
	_keep_alive_time = 0;
	on_establish_connection();
	return true;
}

void Vmmessage::destroy_connection()
{
	_process_loop.remove_eventsource(*this);
	close(_fd);
	//_keepalive = 0;
	//_connected = false;
	on_destroy_connection();
}

void Vmmessage::shake_hand()
{
	_keep_alive_time ++;
	if (_keep_alive_time > SHAKE_TIMEOUT){
		_connected_to_guest = GUEST_DISCONNECT;
	}
	send_message(ALIVE_ID, GUEST_KEEPALIVE, "");
}

bool Vmmessage::send_message(int id, int handle, const string& data, int target)
{
    Lock send_lock(_send_mutex);
	int fd_tmp = get_fd();
    if(fd_tmp <= 0)
    {
        LOG_INFO("now fd is not ready fd = %d", fd_tmp);
        return false;
    }
	bool return_value = false;
	int pos = 0;
	int send_length = 0;
	int total_length = sizeof(short) + sizeof(short) + sizeof(short) + sizeof(short) + 
		sizeof(short) + sizeof(int) + sizeof(int) + data.length();
	char* send_msg = new char[total_length];
	char* pointer = send_msg;

    memset(pointer, 0, total_length);
#if DEBUG_DETAIL
    LOG_DEBUG("Sending message for handle [%d:%d] with total length %d ...",
        id, handle, total_length);
#endif
	message_write_short(pointer, (short)VERSION_VM_MESSAGE);	//version
	pointer += sizeof(short);
	message_write_short(pointer, (short)target);		    //target
	pointer += sizeof(short);
	message_write_short(pointer, (short)HEAD_LENGTH);		//head_length
	pointer += sizeof(short);

	message_write_short(pointer, (short)id);		//ID
	pointer += sizeof(short);
	message_write_short(pointer, (short)handle);	//handle
	pointer += sizeof(short);

	message_write_int(pointer, (int)data.length());	//length
	pointer += sizeof(int);
	message_write_short(pointer, (short)TERMINAL_IDV_TYPE);	//source
	pointer += sizeof(short);
    message_write_short(pointer, (short)0);	        //reserve
	pointer += sizeof(short);

	if (data.length() > 0) {
		message_write_char(pointer, data.c_str(), data.length());
        
        if (id == GUEST_NET_ID && handle == GUEST_TERMINAL_SYSTEM_MSG) {
            //no need to print detail, see idv.log
        } else {
		    LOG_DEBUG("VM Message DATA [%d:%d] %s", id, handle, data.c_str());
        }
    }

	while (pos < total_length) 
	{
		// send_length = write(_fd, send_msg + pos, total_length - pos);
		send_length = send(fd_tmp, send_msg + pos, total_length - pos, 0);
		if(send_length < 0)
		{
			LOG_WARNING("message send data ret <= 0, ret = %d, errno = %d, strerror:%s\n",
                send_length, errno, strerror(errno));
			switch(errno)
			{
				case EPIPE:
				{
					LOG_ERR("The socket is shut down\n");
					//_connected = false;
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
	LOG_DEBUG("Message for handle [%d:%d] was sent OK.\n", id, handle);
	return_value = true;

quit:
	delete[] send_msg;
	return return_value;
}

#define MESSAGE_RECV(recv_buffer, length, error_action)\
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
			
			bool Vmmessage::recv_message(int fd, int& id, int& hd, string& buf)
			{/*
				char recv_buffer[HEAD_BUF_SIZE];
				int ret = 0;
				int length = 0;
				short handle = 0;
				short id_tmp = 0;
			
				memset(recv_buffer, 0 , sizeof(recv_buffer));
				MESSAGE_RECV(recv_buffer, sizeof(short), return false);//version
				MESSAGE_RECV(recv_buffer, sizeof(short), return false);//type
				MESSAGE_RECV(recv_buffer, sizeof(short), return false);//head_length
				MESSAGE_RECV(recv_buffer, sizeof(short), return false);//id
				vmmessage_read_short(id_tmp, recv_buffer);
				MESSAGE_RECV(recv_buffer, sizeof(short), return false);//handle
				vmmessage_read_short(handle, recv_buffer);
				MESSAGE_RECV(recv_buffer, sizeof(int), return false);//length
				vmmessage_read_int(length, recv_buffer);
				MESSAGE_RECV(recv_buffer, sizeof(int), return false);//reserved					
				LOG_DEBUG("recv Head completely, id = %d, handle = %d, length = %d", id, handle, length);
				char* data = new char[length+1];
				memset(data, 0, length+1);
				MESSAGE_RECV(data, length, goto free_data_quit);//length
			
				LOG_INFO("recv handle %d json %s ", handle, data);
				id = (int)id_tmp;
				hd = (int)handle;
				buf = data;
				delete[] data;
				return true;
			free_data_quit:
				delete[] data;
				return false;
				LOG_DEBUG("vmmessage_recv_message");
				return true;*/
				return true;
			}
			
#undef MESSAGE_RECV

/*void Vmmessage::send_guest_message(int handle, int error, string error_msg, int timestamp)
{
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddNumberToObject(json, "timestamp", timestamp);
    cJSON_AddNumberToObject(json, "error", error);
    cJSON_AddStringToObject(json, "msg", error_msg.c_str());
    JSON_WRITE_PARSE_FREE(json, handle);
}*/

void Vmmessage::action()
{
    //ASSERT(_fd != INFINITE && _connected == true);
    //handle_message(_fd);
}

void Vmmessage::handle_keepalive(string data)
{
	//_keepalive = 0;
}


// write data to json
#define JSON_WRITE_PARSE_INIT(json, json_string)\
    string json_string;\
    char* json_text;\
    cJSON * json = cJSON_CreateObject();

#define JSON_WRITE_PARSE_FREE(id, json, handle)\
    json_text = cJSON_PrintUnformatted(json);\
    json_string = json_text;\
    LOG_DEBUG("create json_text \n%s", json_text);\
    send_message(id, handle, json_string); \
    if(json_text)\
    {\
        free(json_text);\
    }\
    cJSON_Delete(json);


void Vmmessage::message_upload_vm_info()
{
	string data = "";
	LOG_DEBUG("GET VM INFO ...");
	send_message(ALIVE_ID, GUEST_GET_VM_INFO, data.c_str());
}

// setting guest network
void Vmmessage::message_upload_vm_network_info(NetworkInfo& networkinfo, int id)
{
	int handle = GUEST_SET_NETWORK;
	//int sock = get_fd();
	LOG_DEBUG("SET GUEST NETWORK");
    JSON_WRITE_PARSE_INIT(json, json_string);

    if (networkinfo.dhcp) {
        // DHCP, set IP as 0.0.0.0 for Guesttool.
        cJSON_AddStringToObject(json,   "ip",       "0.0.0.0");
    } else {
        cJSON_AddStringToObject(json,   "ip",       networkinfo.ip.c_str());
    }
    cJSON_AddStringToObject(json,   "mask",      	networkinfo.submask.c_str());
    cJSON_AddStringToObject(json,   "gateway",      networkinfo.gateway.c_str());
    if (networkinfo.auto_dns) {
        // Auto DNS, set main DNS as 0.0.0.0 for Guesttool.
        cJSON_AddStringToObject(json,   "dns1",     "0.0.0.0");
    } else {
        cJSON_AddStringToObject(json,   "dns1",     networkinfo.main_dns.c_str());
    }
    cJSON_AddStringToObject(json,   "dns2",		    networkinfo.back_dns.c_str());
	cJSON_AddStringToObject(json,   "lock",     	"yes");

    JSON_WRITE_PARSE_FREE(id, json, handle);

}

void Vmmessage::message_upload_local_network_info(NetworkInfo& networkinfo, int id)
{
	int handle = GUEST_REQUEST_GET_TERMINAL_IP;
	//int sock = get_fd();
	LOG_DEBUG("SET GUEST NETWORK");
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddBoolToObject(json,     "auto_dhcp",    networkinfo.dhcp);
    cJSON_AddStringToObject(json,   "ip",       networkinfo.ip.c_str());
    cJSON_AddStringToObject(json,   "mask",      	networkinfo.submask.c_str());
    cJSON_AddStringToObject(json,   "gateway",      networkinfo.gateway.c_str());
    cJSON_AddBoolToObject(json,     "auto_dns",    networkinfo.auto_dns);
    cJSON_AddStringToObject(json,   "dns1",     networkinfo.main_dns.c_str());
    cJSON_AddStringToObject(json,   "dns2",		    networkinfo.back_dns.c_str());

    JSON_WRITE_PARSE_FREE(id, json, handle);

}

void Vmmessage::message_upload_download_info_to_zhjsgt(string downloaded_size,string total_size,string percent,string rate)
{
	int id = GUEST_ZHJS_ID;
	int handle = UPLOAD_DOWNLOAD_INFO_ZHJSGT;
	LOG_DEBUG("UPLOAD DOWNLOAD INFO TO ZhjsGT");
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddStringToObject(json,   "download_size",               downloaded_size.c_str());
    cJSON_AddStringToObject(json,   "total_size",		    total_size.c_str());
    cJSON_AddStringToObject(json,   "progress",      	percent.c_str());
    cJSON_AddStringToObject(json,   "download_speed",      	rate.c_str());
    JSON_WRITE_PARSE_FREE(id, json, handle);

}

void Vmmessage::message_upload_network_info_to_zhjsgt(string ip, string net_mode)
{
	int id = GUEST_ZHJS_ID;
	int handle = UPLOAD_NETWORK_INFO_ZHJSGT;
	LOG_DEBUG("UPLOAD NETWORK INFO TO ZhjsGT");
    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddStringToObject(json,   "ip",               ip.c_str());
    cJSON_AddStringToObject(json,   "vm_ip",		    ip.c_str());
    cJSON_AddStringToObject(json,   "net_mode",      	net_mode.c_str());
    JSON_WRITE_PARSE_FREE(id, json, handle);

}

// setting guest netdisk
void Vmmessage::message_upload_netdisk(NetdiskInfo& netdiskinfo)
{
	int id = ALIVE_ID;
	int handle = GUEST_SET_DISKINFO;
	//int sock = get_fd();
	LOG_DEBUG("SET GUEST NETDISK");
    JSON_WRITE_PARSE_INIT(json, json_string);

	cJSON_AddStringToObject(json, "path", netdiskinfo.netdisk_path.c_str());
    cJSON_AddStringToObject(json, "ip", netdiskinfo.netdisk_ip.c_str());
    cJSON_AddStringToObject(json, "user", netdiskinfo.netdisk_username.c_str());
    cJSON_AddStringToObject(json, "psw", netdiskinfo.netdisk_password.c_str());
	cJSON_AddStringToObject(json, "disk", "X");
	
    JSON_WRITE_PARSE_FREE(id, json, handle);
}

void Vmmessage::message_upload_sunny_info()
{
    bool recovery;
    string server_ip;
    string logined_username;
    int id = ALIVE_ID;
    int handle = GUEST_SET_SUNNY_INFO;
    Application *app = Application::get_application();

    LOG_DEBUG("Sending sunny info ...");
    app->vm_get_sunny_info(&recovery, &server_ip, &logined_username);
    LOG_DEBUG("Sunny Info: recovery: %s, server_ip: %s, logined_user: %s\n",
        (recovery ? "true" : "false"), server_ip.c_str(), logined_username.c_str());
    JSON_WRITE_PARSE_INIT(json, json_string);
    cJSON_AddStringToObject(json, "rcdip", server_ip.c_str());
    if (recovery)   // guesttool supports only string json value.
        cJSON_AddStringToObject(json, "guesttype", "4");
    else
        cJSON_AddStringToObject(json, "guesttype", "5");
    cJSON_AddStringToObject(json, "username", logined_username.c_str());
    JSON_WRITE_PARSE_FREE(id, json, handle);
}

void Vmmessage::message_upload_web_info(const string &web_msg, int id, int target)
{
	int handle = GUEST_SEND_WEB_INFO_TO_VM;

    send_message(id, handle, web_msg, target);
}

void Vmmessage::message_terminal_system_msg(const string &msg)
{
	int handle = GUEST_TERMINAL_SYSTEM_MSG;
    //TODO:add if ctr
    int id = GUEST_NET_ID;
    send_message(id, handle, msg);
}

void Vmmessage::message_terminal_device_msg(const string &msg)
{
    int handle = GUEST_SEND_DEV_INFO_TO_VM;
    int id = GUEST_DEV_ID;

    send_message(id, handle, msg);
}

void Vmmessage::message_set_guest_netdisable()
{
	int id = ALIVE_ID;
	int handle = GUEST_SET_NET_DISABLE;
	//int sock = get_fd();

    LOG_DEBUG("SET GUEST NETDISABLE");
    JSON_WRITE_PARSE_INIT(json, json_string);
    cJSON_AddStringToObject(json, "text", "网络被禁用，请联系管理员");
    JSON_WRITE_PARSE_FREE(id, json, handle);
}
void Vmmessage::message_set_guest_merge_tips()
{
	int id = GUEST_ZHJS_ID;
	int handle = GUEST_MERGE_INFO;

    send_message(id, handle, "needmerge");

}

void Vmmessage::message_set_guest_usb_polocy(string& usb_policyinfo)
{
	string data = usb_policyinfo;
	//int id = ALIVE_ID;
	//int handle = GUEST_SET_NETWORK;
	//int sock = get_fd();
	LOG_DEBUG("SET GUEST USBPOLOCY");
	send_message((int)ALIVE_ID, (int)GUEST_SET_USB_POLOCY, data);
    //_app->web_set_public_policy(info);
}

void Vmmessage::message_update_vm_time()
{
    int id = ALIVE_ID;
    int handle = GUEST_QUERY_TIME;
    string client_time = VM_Common::execute_command("date +%s");
    if (client_time == "error")
    {
        client_time = "0";
    }

    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddStringToObject(json, "handle", "querytime");
    cJSON_AddStringToObject(json, "time", client_time.c_str());

    JSON_WRITE_PARSE_FREE(id, json, handle);
}

void Vmmessage::message_set_guest_disconnected()
{
    /**
     * When VM network disabled, we set guest connection status to GUEST_DISCONNECT,
     * which will force vmmessage to re-forward group polocy to VM on next keepalive timer.
     */
    _connected_to_guest = GUEST_DISCONNECT;
}

void Vmmessage::message_send_wired_mac()
{
    int id = GUEST_PUSHINSTALL_ID;
    int handle = GUEST_SEND_WIRED_MAC;    
	Application* app = Application::get_application();
    string wired_mac = app->get_local_network_data().get_wired_mac();

    JSON_WRITE_PARSE_INIT(json, json_string);

    cJSON_AddStringToObject(json, "mac", wired_mac.c_str());

    JSON_WRITE_PARSE_FREE(id, json, handle);
}

#if 0
// sending message to guesttool
	bool send_message(int id, int handle, const string& data, int sock)
	{
	
		bool return_value = false;
		int pos = 0;
		int send_length = 0;
		int total_length = sizeof(short) + sizeof(short) + sizeof(short) + sizeof(short) + 
			sizeof(short) + sizeof(int) + sizeof(int) + data.length();
		char* send_msg = new char[total_length];
		char* pointer = send_msg;

		message_write_short(pointer, (short)VERSION_VM_MESSAGE);	//version
		pointer += sizeof(short);
		message_write_short(pointer, (short)MESSAGE_TYPE);		//type
		pointer += sizeof(short);
		message_write_short(pointer, (short)HEAD_LENGTH);		//head_length
		pointer += sizeof(short);

		message_write_short(pointer, (short)id);		//ID
		pointer += sizeof(short);
		message_write_short(pointer, (short)handle);	//handle
		pointer += sizeof(short);

		message_write_int(pointer, (int)data.length());	//length
		pointer += sizeof(int);
		message_write_int(pointer, (int)0);				//reserve
		pointer += sizeof(int);

		if (data.length() > 0)	
			message_write_char(pointer, data.c_str(), data.length());
	
		while (pos < total_length) 
		{
			// send_length = write(_fd, send_msg + pos, total_length - pos);
			send_length = send(sock, send_msg + pos, total_length - pos, 0);
			if(send_length < 0)
			{
				LOG_WARNING("message send data ret <= 0, ret = %d, errno = %d", send_length, errno);
				switch(errno)
				{
					case EPIPE:
					{
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

void Vmmessage::on_read(int sock, short event, void* arg)
{/*
	char recv_buffer[HEAD_BUF_SIZE];
	char* data = NULL;
	string tmp = "";
	int ret = 0;
	int length = 0;
	short handle = 0;
	short id_tmp = 0;
	VM *_vm_t = VM::get_vmself();
	//Vmmessage*_vmmessage_t = Vmmessage::get_vmmessageself();

	//Vmmessage vmmessage = new Vmmessage(this);
	LOG_DEBUG("START ON READ");
	//--本来应该用while一直循环，但由于用了libevent，只在可以读的时候才触发on_read(),故不必用while了
	memset(recv_buffer, 0 , sizeof(recv_buffer));
	ret = recv(sock, recv_buffer, sizeof(short), MSG_WAITALL);//version
	ret = recv(sock, recv_buffer, sizeof(short), MSG_WAITALL);//type
	ret = recv(sock, recv_buffer, sizeof(short), MSG_WAITALL);//head_length
	ret = recv(sock, recv_buffer, sizeof(short), MSG_WAITALL);//id
	vmmessage_read_short(id_tmp, recv_buffer);
	ret = recv(sock, recv_buffer, sizeof(short), MSG_WAITALL);//handle
	vmmessage_read_short(handle, recv_buffer);
	ret = recv(sock, recv_buffer, sizeof(int), MSG_WAITALL);//length
	vmmessage_read_int(length, recv_buffer);
	ret = recv(sock, recv_buffer, sizeof(int), MSG_WAITALL);//reserved 				
	LOG_DEBUG("recv Head completely, id = %d, handle = %d, length = %d", id_tmp, handle, length);
	if (length > 0){
		data = new char[length+1];
		memset(data, 0, length+1);
		ret = recv(sock, data, length, MSG_WAITALL);//length
	
		LOG_INFO("recv handle %d json %s ", handle, data);
	}
	LOG_DEBUG("start check handle");
	cJSON* json_t = cJSON_CreateObject();
	switch (handle){
		case GUEST_KEEPALIVE:
			LOG_DEBUG("GUEST IS ALIVED");
			if (GUEST_DISCONNECT == _connected_to_guest)
			{
				_connected_to_guest = GUEST_ON_CONNECT;
				_usb_policyinfo = _vm_t->get_usb_policy();
				message_set_guest_usb_polocy(_usb_policyinfo);
				sleep(1);
				_networkinfo = _vm_t->vm_get_vm_netinfo();				
				message_upload_vm_network_info(_networkinfo);
				sleep(1);
				_netdiskinfo = _vm_t->get_netdisk_info();
				message_upload_netdisk(_netdiskinfo);
				sleep(1);
				
			}
			else if (GUEST_ON_CONNECT == _connected_to_guest)
				_connected_to_guest = GUEST_ON_CONTINUE;
			break;
		case GUEST_VM_INFO:
			json_t = cJSON_Parse(data);
			// the data will be recieved from 
			_vminfo.running = true;
			_vminfo.storage_total = 30000;
			_vminfo.storage_use = 0;
			get_json_valueint  (_vminfo.cpu, json_t, "cpu-use");
			get_json_valueint  (_vminfo.memory_total, json_t, "mem-MBs");
			get_json_valueint  (_vminfo.memory_use, json_t, "mem-use-MBs");
			LOG_DEBUG("READ CPU MEMORY OK");
			//LOG_DEBUG("cpu = %d, memory_total = %d, memory_use = %d", cpu, mt, mu);
			//_app->vm_get_vm_info_status(&_vminfo);
			break;
		case GUEST_SET_NET_DISABLE:
			LOG_DEBUG("set net disable is OK");
			break;
		case GUEST_SET_USB_POLOCY:
			LOG_DEBUG("set guest usb polocy is OK");
			break;
		case GUEST_SET_NETWORK:
			LOG_DEBUG("set guest network is OK");
			break;
		case GUEST_GET_NETWORK:
			//message_upload_vm_network_info(network_info_t, id_tmp, handle, sock);
			break;
		case GUEST_GET_DISKINFO:
			LOG_DEBUG("set guest diskinfo OK");
			//message_upload_netdisk(netdisk_info_t, id_tmp, handle, sock);
			break;
		case GUEST_SET_LOG:
			json_t = cJSON_Parse(data);
			get_json_valuestring(guesttool_filename, json_t, "filename");
			get_json_valuestring(logdata, json_t, "logstr");
			LOG_DEBUG("guest log : filename :%s, log is %s", guesttool_filename.c_str(), logdata.c_str());
			//send_message(id_tmp, (int)GUEST_SET_LOG, tmp, sock);
			break;
		default:
			break;
	}
	LOG_DEBUG("end the on read");
	if (length >0)
		delete[] data;
	cJSON_Delete(json_t);
	LOG_DEBUG("END ON READ");*/
}
#endif

void* Vmmessage::init_read_event(void *data)
{
// because of the pipe of vmmessage cannot be destoryed, else the guesttool will not be connectted all the time
// so the connection will be in the first time of openning the box, other time will not be reconnectted
	Vmmessage* vmm_t = (Vmmessage*)data;

	// LOG_INFO("sock = %d after\n", sock);	
	//-----init libevent，set callback on_read()------------
	_base = event_base_new();
	if (!_base){
		LOG_ERR("couldn't open event base\n");
		// return ;
	}
	// get data from socket after reading
	LOG_DEBUG("READ BEFORE");
	//struct event* read_ev = event_new(base, vmm_t->get_fd(), EV_READ|EV_PERSIST, vmm_t->on_read, NULL);
	struct event* read_ev = event_new(_base, vmm_t->get_fd(), EV_READ|EV_PERSIST, Vmmessage_Thread::callback_cb, NULL);
	event_base_set(_base, read_ev);
	event_add(read_ev, NULL);
	event_base_dispatch(_base);
	LOG_DEBUG("READ AFTER");	
	//--------------
	event_del(read_ev);
	free(read_ev);
	event_base_free(_base);
	return NULL;
}

void Vmmessage::init_read_event_thread()
{
	LOG_DEBUG("INIT_READ_EVENT_THREAD");
	pthread_t thread;
	//pthread_create(&thread, NULL, Vmmessage_Thread::vmmessage_thread, NULL);
	pthread_create(&thread, NULL, init_read_event, this);
	pthread_detach(thread);
}

struct event_base* Vmmessage::get_event_base(void)
{
    return _base;
}


#ifdef UNIT_TEST

int test()
{
	string data_tmp = "";

	//Vmmessage* vmmessage = new Vmmessage(Application* app);

	init_log(RCC_LOG_VMMESSAGE, RCC_LOG_FILE_MAXSIZE, USER_DEBUG, NULL);
	sleep(2);
	// set for Global variable
	//bool ret;
	//VMInfo vminfo_t;
	NetworkInfo network_info_t;
	NetdiskInfo netdisk_info_t;
	string logdata, guesttool_filename;
	PolicyInfo policy_info_t;
	//int connect_to_guest;	
	// set test info
	network_info_t.ip = "172.21.139.225";
	network_info_t.submask = "255.255.252.0";
	network_info_t.gateway = "172.21.136.1";
	network_info_t.main_dns = "192.168.58.110";
	network_info_t.back_dns = "192.168.58.111";
	
	netdisk_info_t.netdisk_ip = "172.21.111.193";
	netdisk_info_t.netdisk_path = "/f2e5b384a3528c37c555";
	netdisk_info_t.netdisk_username =  "f2e5b384a3528c37c555";
	netdisk_info_t.netdisk_password = "1234567";

	policy_info_t.net_policy = 0;
	// connect_to_guest = 0;
	// test


	// the thread will not be destroyed, for thread_cancel will not be realized
	//vmmessage->init_read_event_thread();
	while(1)
		{
			sleep(1000);
		}
/*
	while(1){
		ret = send_message((int)ALIVE_ID, (int)GUEST_KEEPALIVE, data_tmp, vmmessage->get_fd());
		if (connect_to_guest == 1)
		{
			send_message((int)ALIVE_ID, (int)GUEST_VM_INFO, data_tmp, vmmessage->get_fd());
			sleep(1);
			message_set_guest_netdisable((int)ALIVE_ID, (int)GUEST_SET_NET_DISABLE, vmmessage->get_fd());
			sleep(1);
			message_upload_vm_network_info(network_info_t, (int)ALIVE_ID, (int)GUEST_SET_NETWORK, vmmessage->get_fd());
			sleep(1);
			message_upload_netdisk(netdisk_info_t, (int)ALIVE_ID, (int)GUEST_SET_DISKINFO, vmmessage->get_fd());
		}
		sleep(15);
	}*/

	/*if (test())
		LOG_ERR("vmmessage-test code is fail\n");
	*/
	return 0;
}

#endif /* UNIT_TEST */

