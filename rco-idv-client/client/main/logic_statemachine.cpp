/**
 * Copyright(C) 2017 Ruijie Network Inc. All rights reserved.
 *
 * logic_statemachine.cpp
 * Original Author: yejx@ruijie.com.cn, 2017-2-22
 *
 * Logic state machine processes.
 */

#include "application.h"
#include "ui_api.h"

const static string LogicEventDesc[] = {
    "L2L: init event",
    "L2L: easy deploy",
    "L2L: enter new deploy",
    "L2L: enter running vm",
    "L2L: link up",
    "L2L: link down",
    "L2L: link1 up",
    "L2L: link1 down",
    "L2L: data protect process",
    "L2L: download success tips",
    
    "U2L: start new deploy",
    "U2L: set local network info",
    "U2L: set VM network info",
    "U2L: set server IP",
    "U2L: set hostname",
    "U2L: set wifi status",
    "U2L: set user mode",
    "U2L: set auth info",
	"U2L: quit local mode",
	"U2L: check local mode error",
    "U2L: user login",
    "U2L: public user login",
    "U2L: guest login",
    "U2L: bind user",
    "U2L: modify user password",
    "U2L: system shutdown",
    "U2L: system reboot",
    "U2L: estabish connection",
    "U2L: cancel connection",
    "U2L: enter local mode",
    "U2L: enter settings",
    "U2L: enter wifi config",
    "U2L: enter auth config",    
    "U2L: notify setting result",
    "U2L: notify wifi config result",
    "U2L: notify auth config result",
    "U2L: enter save power boot",
    "U2L: enter usb copy base",
    "U2L: upgrade for class",
    
    "W2L: connection established",
    "W2L: connection lost",
    "W2L: web ready",
    "W2L: upgrade for office",
    "W2L: upgrade for class",
    "W2L: sync deb path",
    "W2L: sync soft version",
    "W2L: sync terminal password",
    "W2L: sync public policy",
    "W2L: sync user mode",
    "W2L: sync hostname",
    "W2L: sync printer switch",
    "W2L: sync desktop redir",
    "W2L: sync image",
    "W2L: sync ipxe",
    "W2L: sync recovery",
    "W2L: sync initial",
    "W2L: sync server ip",
    "W2L: sync all userinfo",
    "W2L: sync server time",
    "W2L: sync dev policy",
    "W2L: dev interface info",
    "W2L: sync driver install",
    //"W2L: sync reload image",
    "W2L_SYNC_PORT_MAPPING",
    "W2L: sync ssid white list",
    "W2L: sync rcdusbconf info",
    "W2L: sync delete teacherdisk",
    "W2L: sync main window",
    "W2L: init sync error",
    "W2L: image sync error",
    "W2L: login successfully",
    "W2L: login failed",
    "W2L: bind successfully",
    "W2L: bind failed",
    "W2L: check VM status",
    "W2L: system shutdown",
    "W2L: system reboot",
    "W2L: recover image",
    "W2L: collect log",
    "W2L: modify system network",
    "W2L: modify VM network",
    "W2L: modify hostname",
    "W2L: reset to initial",
    "W2L: resync",
    "W2L: do ipxe",
    "W2L: notify ssid white list",
    "W2L: notify reset netdisk",
    //"W2L_NOTIFY_RELOAD_IMAGE",
    "W2L_NOTIFY_DELETE_TEACHERDISK",

    "W2L: notify http port",

    "W2L: modify user password",
    "W2L: user password modified successfully",
    "W2L: user password modified failed",
    "W2L: web to guesttool msg",
    "V2L: image check result",
    "V2L: VM starting result",
    "V2L: VM shutdown result",
    "V2L: VM reboot result",
    "V2L: VM download progress",
    "V2L: VM download quit",
    "V2L: get VM info",
    "V2L: set VM network policy",   //FIXME: unuseful event
    "V2L: set VM USB policy",       //FIXME: unuseful event
    "V2L: set VM network result",   //FIXME: unuseful event
    "V2L: get VM network result",   //FIXME: unuseful event
    "V2L: set VM diskinfo result",  //FIXME: unuseful event
    "V2L_SET_LOCAL_WIRED_NETWORK",
    "Unknown event",        //EVENT_TYPE_MAX
};

const static string LogicStateDesc[] = {
    "NULL state",
    "Initialization",
    "Staying on Local Mode",
    "Waiting User Login",
    "Login Authentication",
    "Preparing download image",
    "Downloading image",
    "Running VM",
    "New device deploy",
    "Unknown logic state",  //STATUS_UNKNOWN
};

const string &Application::logic_event_desc(int event)
{
    if (event >= L2L_INIT_SELF && event < EVENT_TYPE_MAX)
        return LogicEventDesc[event];
    else
        return LogicEventDesc[EVENT_TYPE_MAX];
}

const string &Application::logic_state_desc(int state)
{
    if (state >= STATUS_NONE && state < STATUS_UNKNOWN)
        return LogicStateDesc[state];
    else
        return LogicStateDesc[STATUS_UNKNOWN];
}

void Application::on_entering_initing()
{
    int last_saved_state = _userinfomgr.get_saved_state();
    LOG_INFO("##########STATUS_MACHINE########## initing");
    LOG_DEBUG("last_saved_state = %d, ui_locking = %d", last_saved_state, _ui_locking);
    if (_ui_locking) {
        return;
    }

    _userinfomgr.saveCurrentState(STATUS_INITING);
    //step1:init variables
    static bool init_check = false;
    _logined = false;
    _allow_newdeploy = true;
    //_ui_has_sync_flag = 0;
    //_ui_need_sync_flag = 0;
    //_ui_setting_error = 0;
    _localmode_startvm = false;
    _download_retry = false;
    _new_deploy_processing = false;
    _init_sync_processing = false;
    _wait_tips_ok = false;
    _input_user_info.username.clear();
    _input_user_info.password.clear();
    _logined_user_info.username.clear();
    _logined_user_info.password.clear();
    _upgradeInfo.clear();
    vector<string> ssid_list;
    //step2: destroy connection to server
    //LOG_DEBUG("call mina_destroy_connection _error_tips=%d", _error_tips);
    //_mina->mina_destroy_connection();
    //LOG_DEBUG("call mina_destroy_connection");

    _userinfomgr.PublicPolicyRead(&_public_policy_info);
    LOG_INFO("system default out control day is %d, this may be change by web", _public_policy_info.pub.outctrl_day);

    // check disk status
    if(init_check == false)
    {
        string disk_status;
        int ret = get_disk_status(&disk_status);
        if (ret != 0)
        {
            _ui_locking = true;
            LOG_DEBUG("call l2u_show_recover_disk(%s, %d)", disk_status.c_str(), disk_status.size());
            l2u_show_recover_disk(disk_status.c_str(), disk_status.size());
            return;
        }
    }

#if 0 /* is this necessary, may cause problem, if server ip changes */
    if(init_check == false) {
    	LOG_DEBUG("####invoke http port convert");
        _local_network_data.set_port_convert();
    }
#endif

    init_check = true;

    // check if User Logined & VM running.
    // the state on file is not right when IDV_Client is killed on vm is running, then we should add other condition
    if (last_saved_state == STATUS_RUNNING_VM || _vm->vm_is_vm_running()) {
        LOG_INFO("Client state reset from exception, last saved state: %s\n",
            logic_state_desc(STATUS_RUNNING_VM).c_str());
        _logined = _userinfomgr.user_logined();
        if (_logined) {     //ASSERT(_logined);
            _input_user_info.username = _userinfomgr.getCurrentLoginedUser();
            _logined_user_info.username = _userinfomgr.getCurrentLoginedUser();
            if (_mode_data.get().mode == PUBLIC_MODE
                || _input_user_info.username == "guest") {
                //public or guest logined, using public policy.
                _userinfomgr.PublicPolicyRead(&_logined_user_info.policy_info);
            } else {
                //user logined, reading user netdisk & group policy.
                _userinfomgr.UserInfoRead(_input_user_info.username, &_logined_user_info);
            }
            //ensure mina destroyed before entering running vm
            //LogicEvent* event = new LogicEvent(this, L2L_ENTER_RUNNING_VM, NULL, 0);
            //_process_loop.push_event(event);
            //event->unref();
            _status_machine.change_status(EVENT_VM_RUNNING);

            if (_userinfomgr.on_local_mode())
                return;
        }
    }

    // check if current is reboot from VM started failed exception
    {
        struct stat statbuf;
        if (stat("/root/.rcc_exception_reboot", &statbuf) == 0) {
            UIPopEvent sync_event("Reboot from exception that VM started failed");
            rc_system("rm -f /root/.rcc_exception_reboot");
            l2u_show_vm_lasterr_tips((void *)Application::uipop_sync ,(void *)&sync_event);
            sync_event.wait();
        }
    }

    if (_ui_locking == false) {
        l2u_show_connecting_wrap();
    }
//    while (is_dev_network_unknown()) {
//        // wait until device network status initialized.
//        usleep(50000);
//    }
    //wait 5s to show connecting ui if link down
    int wait_link_count = 0;
    if (!get_file_exist("/tmp/rco_link_init_check")) {
//        while (is_dev_network_down() && wait_link_count < 50) {
//            usleep(100000);
//            wait_link_count++;
//        }
        rc_system("touch /tmp/rco_link_init_check");
    }

    if (is_wifi_terminal()) {
        if ((_userinfomgr.readssidWhiteList(ssid_list)) != 0) {
            LOG_DEBUG("set_white_ssid_list error"); 
        } else {
            _wifi_interactive->wifi_forget_not_whitessid_list(ssid_list);
        }
    }
    
    //new terminal should start new deploy
    LOG_INFO("check newdeploy: new_terminal: %d, download_status: %d", _newdeploy_manage->is_new_terminal(), _newdeploy_manage->get_download_status());
    if(_newdeploy_manage->is_new_terminal() == true || _newdeploy_manage->get_download_status() == true)
    {
        //ensure mina destroyed before entering new deploy
        LogicEvent* event = new LogicEvent(this, L2L_ENTER_NEW_DEPLOY, NULL, 0);
        _process_loop.push_event(event);
        event->unref();
        return;
    }

    //if has error tips, do not conect server or show any UI
    if(_error_tips == false)
    {
        //step3: if server ip is not set, show unconfig UI, else show connecting UI;
        if(_mina->mina_get_server_ip().empty())
        {
            l2u_show_unconfig_wrap();
        }
        else if(is_dev_network_up())
        {
            //if server ip is set, try to establish connection
            LOG_DEBUG("server ip is %s", _mina->mina_get_server_ip().c_str());
            _mina->mina_establish_connection();
            if(_ui_locking==false)
            {
                l2u_show_connecting_wrap();
            }
        }
        else if(is_dev_network_down() && _ui_locking==false && _status_machine.get_status() != STATUS_CHECKING_LOCAL)
        {
            LOG_DEBUG("call l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT)");
			LOG_INFO("call l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT)1414141414");
			//NoNetwireConnect = 1;
            l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT);
        }
    }
}

void Application::on_entering_checking_local()
{
    struct ModeInfo mode;
    char command_result[128] = {0,};
    int ret = 0;
    int command_result_len = 0;

    mode = _mode_data.get();
    LOG_DEBUG("##########entering_checking_local##########");

    _userinfomgr.saveCurrentState(STATUS_CHECKING_LOCAL);
	if (NULL == localmode) {
		LOG_DEBUG("create localmode obj");
		localmode = new LocalMode(this, _mode_data.get());
	}else {
		localmode->ResetStatus();
	}
    _userinfomgr.enter_local_mode();
    _localmode_startvm = true;
  
    command_result_len = sizeof((command_result));
    ret = rc_system_rw("dpkg -l | grep estserver | awk \'{print $1}\'",(unsigned char*) command_result, &command_result_len, "r");
    LOG_DEBUG("%s, %d, %d\n", command_result,strlen(command_result), ret);
    if((ret != 0) || (strlen(command_result) != 0)) {
            LOG_DEBUG(" the rc_system_rw is error");
    }
    if ( strncmp(command_result, "ii", (strlen(command_result) - 1)) != 0) {
         l2u_show_upgrade_faile();
         return;
    }
    if (localmode->UI_Req_ChangeToLocalMode() < 0) {
    	LOG_ERR("entering localmode error");
        _userinfomgr.leave_local_mode();
    	_status_machine.change_status(EVENT_LOCAL_CHECK_FAILED);
    }
}

void Application::on_entering_waiting_login() 
{
    Mode mode;

    LOG_INFO("##########STATUS_MACHINE########## waiting_login");
    _userinfomgr.saveCurrentState(STATUS_WAITING_LOGIN);
    mode = _mode_data.get().mode;

    if(_input_user_info.username.empty() && _input_user_info.password.empty())
    {
        //input empty, has not logined or guest login faild
        if((_logined_user_info.username.compare("guest")==0) && (_logined_user_info.password.compare("guest")==0))
        {
            //if guest enter waiting login status, indicate login fail, may be no image in web
            LOG_WARNING("guest enter waiting login status, indicate login fail, may be no image in web");
        }
        else
        {
            //has not logined, request UI to show login interface
            if (mode == PUBLIC_MODE)
            {
                if(_ui_locking == false)
                {
                    LOG_DEBUG("call l2u_show_publiclogin");
                    l2u_show_publiclogin();
                }
            }
            else
            {
                if(_ui_locking == false)
                {
                    LOG_DEBUG("call l2u_show_userlogin");
                    ui_show_login();
                }
            }
        }
    }
    else
    {   
        //now sync image error would not happen after login, so _error_tips should not be true here!
       
        // if username in not empty, login or bind fail
    }

    //reset vars
    _logined = false;
    _input_user_info.username.clear();
    _input_user_info.password.clear();
    _logined_user_info.username.clear();
    _logined_user_info.password.clear();
}

void Application::on_entering_checking_login() 
{ 
    LOG_INFO("##########STATUS_MACHINE########## checking_login");
    _userinfomgr.saveCurrentState(STATUS_CHECKING_LOGIN);

    //ui->show_logining
    //we do not need to call this interface currently.

    //send username & password to web 
    _mina->mina_web_login(_input_user_info);
}

void Application::on_entering_preparing_image()
{
    struct ImageInfo info;

    LOG_INFO("##########STATUS_MACHINE########## preparing_image");
    _userinfomgr.saveCurrentState(STATUS_PREPARING_IMAGE);
    LOG_INFO("mina_web_request_image");
    info = _vm->vm_get_vm_imageinfo();
    _mina->mina_web_request_image(info);

    //ui->show_checking_image
    //chenli: shold show image update UI when need download
    if (false == _download_retry) {
    	//todo
    	LOG_INFO("to fix, show image update ui");
//    	l2u_show_imageupdate();
    }
}

void Application::on_entering_downloading_image()
{
    struct ImageInfo image_info = _vm->vm_get_vm_imageinfo();
    bool download_status = _vm->vm_get_vm_download_status();


    LOG_INFO("##########STATUS_MACHINE########## downloading_image");
    _ui_locking = false;
    _userinfomgr.saveCurrentState(STATUS_DOWNLOADING_IMAGE);
    ASSERT((_server_image_info.id != image_info.id) || (_server_image_info.version != image_info.version) || (download_status == true));
	if (DOWNLOAD_IMAGE_SILENT_DOWNLOAD != _vm->vm_get_image_downmode())
    {
    	l2u_download_info.status = UI_DIALOG_DOWNLOAD_ST_INITING;
    	l2u_download_info.speed = NULL;
    	l2u_download_info.process = 0.0;
        l2u_show_image_initing(&l2u_download_info);
    }
    //l2u_show_download(&l2u_download_info);
    _vm->vm_start_download_image(&_server_image_info);
}

void Application::on_entering_running_vm()
{
    struct ModeInfo mode = _mode_data.get();
    LayerDiskInfo layerinfo;
    bool recovery;
    bool usedisk;

    LOG_INFO("##########STATUS_MACHINE########## running_vm");
    _userinfomgr.saveCurrentState(STATUS_RUNNING_VM);
    LOG_INFO("Logined User: username:%s passwd:%s\n", _logined_user_info.username.c_str(), _logined_user_info.password.c_str());
    _logined = true;    //for public mode

    if (_dev_policy.check_recovery()) {
        recovery = _dev_policy.allow_recovery();
    } else if(_mode_data.get().mode == SPECIAL_MODE) {
        recovery = false;
    } else {
        recovery = true;
    }

    if (!_vm->vm_is_vm_running()) {
        if (_mode_data.get().mode != PUBLIC_MODE) {
            LOG_INFO("call l2u_show_login_tip");
            l2u_show_login_tip();
        }

        l2u_ctrl_winbtn(false);
        l2u_show_wifi_btnbox(2, -1, -1);

        if (_net_auth->getAuthExist() == 0) {
            l2u_show_net_auth_winbtn(UI_AUTHBTN_DISABLED);
        }
    }

    vm_ui_thread_quit();

#if 0
    if (_mode_data.get().mode == SPECIAL_MODE && _vm->vm_check_personal_img_exist()) {
    	recovery = false;
    } else {
    	if (_dev_policy.check_recovery()) {
    		recovery = _dev_policy.allow_recovery();
    	} else if(_mode_data.get().mode == SPECIAL_MODE) {
    		recovery = false;
    	} else {
    		recovery = true;
    	}
    }

    if (_mode_data.get().mode == SPECIAL_MODE && _vm->vm_check_usedisk_exist()) {
    	usedisk = true;
    } else {
    	usedisk =  _dev_policy.allow_userdisk();
    }
#endif

    usedisk =  _dev_policy.allow_userdisk();
    _vm->vm_start_vm(&mode, &_logined_user_info, recovery, usedisk, _driver_install_parament);
    //TODO: if VM starts failed, what should we do next?
}

void Application::on_entering_new_device_deploy()
{
	LOG_INFO("##########STATUS_MACHINE########## new deploy");
	_userinfomgr.saveCurrentState(STATUS_NEW_DEPLOY);
    _newdeploy_manage->start_new_deploy();
}


LogicStatusMachine::LogicStatusMachine(Application* app)
    :StatusMachine<Application> (app)
{
    _status = STATUS_NONE;
    set_handler(STATUS_INITING,             &Application::on_entering_initing);
    set_handler(STATUS_CHECKING_LOCAL,      &Application::on_entering_checking_local);
    set_handler(STATUS_WAITING_LOGIN,       &Application::on_entering_waiting_login);
    set_handler(STATUS_CHECKING_LOGIN,      &Application::on_entering_checking_login);
    set_handler(STATUS_PREPARING_IMAGE,     &Application::on_entering_preparing_image);
    set_handler(STATUS_DOWNLOADING_IMAGE,   &Application::on_entering_downloading_image);
    set_handler(STATUS_RUNNING_VM,          &Application::on_entering_running_vm);
    set_handler(STATUS_NEW_DEPLOY,          &Application::on_entering_new_device_deploy);

    /**
     * Paired events:
     * W2L_NOTIFY_CHECK_VM_STATUS -> V2L_GET_VM_INFO_RESULT
     */

    append_valid_event_type(STATUS_NONE, W2L_MINA_CONNECTION_DESTROYED);//only use for reset status machine
    append_valid_event_type(STATUS_NONE, U2L_SET_NET_INFO);//only called when both set serverip and net info at setting page
    append_valid_event_type(STATUS_NONE, U2L_SET_VM_NET_INFO);
    append_valid_event_type(STATUS_NONE, U2L_SET_SERVER_IP);
    append_valid_event_type(STATUS_NONE, U2L_SET_HOST_NAME);
    append_valid_event_type(STATUS_NONE, U2L_SET_WIFI_STATUS);
    append_valid_event_type(STATUS_NONE, U2L_SET_TERMINAL_MODE);
    append_valid_event_type(STATUS_NONE, U2L_SET_AUTH_INFO);
    // on init state.
    append_valid_event_type(STATUS_INITING, L2L_EASY_DEPLOY);
    append_valid_event_type(STATUS_INITING, L2L_ENTER_NEW_DEPLOY);
    append_valid_event_type(STATUS_INITING, L2L_ENTER_RUNNING_VM);
    append_valid_event_type(STATUS_INITING, L2L_LINK_UP);
    append_valid_event_type(STATUS_INITING, L2L_LINK_DOWN);
    append_valid_event_type(STATUS_INITING, L2L_LINK1_UP);
    append_valid_event_type(STATUS_INITING, L2L_LINK1_DOWN);
    append_valid_event_type(STATUS_INITING, L2L_DATA_PROTECT);   
    append_valid_event_type(STATUS_INITING, L2L_DOWNLOAD_SUCCESS_TIPS); 
    append_valid_event_type(STATUS_INITING, U2L_SET_NET_INFO);
    append_valid_event_type(STATUS_INITING, U2L_SET_VM_NET_INFO);
    append_valid_event_type(STATUS_INITING, U2L_SET_SERVER_IP);
    append_valid_event_type(STATUS_INITING, U2L_SET_HOST_NAME);
    append_valid_event_type(STATUS_INITING, U2L_SET_WIFI_STATUS);
    append_valid_event_type(STATUS_INITING, U2L_SET_AUTH_INFO);
    append_valid_event_type(STATUS_INITING, U2L_SHUTDOWN_TERMINAL);
    append_valid_event_type(STATUS_INITING, U2L_REBOOT_TERMINAL);
    append_valid_event_type(STATUS_INITING, U2L_ESTABLISH_CONNECTION);
    append_valid_event_type(STATUS_INITING, U2L_CANCEL_CONNECTION);
    append_valid_event_type(STATUS_INITING, U2L_ENTER_LOCAL_MODE);
    append_valid_event_type(STATUS_INITING, U2L_ENTER_SETTINGS);
    append_valid_event_type(STATUS_INITING, U2L_ENTER_WIFI_CONFIG);
    append_valid_event_type(STATUS_INITING, U2L_ENTER_AUTH_CONFIG);
    append_valid_event_type(STATUS_INITING, U2L_NOTIFY_SETTING_RESULT);
    append_valid_event_type(STATUS_INITING, U2L_NOTIFY_WIFI_CONFIG_RESULT);
    append_valid_event_type(STATUS_INITING, U2L_NOTIFY_AUTH_CONFIG_RESULT);
    append_valid_event_type(STATUS_INITING, U2L_SAVE_POWER_BOOT);
    append_valid_event_type(STATUS_INITING, U2L_UPGRADE_FOR_CLASS);

    append_valid_event_type(STATUS_INITING, W2L_MINA_CONNECTION_ESTABLISHED);
    append_valid_event_type(STATUS_INITING, W2L_MINA_CONNECTION_DESTROYED);
    append_valid_event_type(STATUS_INITING, W2L_UPGRADE_FOR_OFFICE);
    append_valid_event_type(STATUS_INITING, W2L_UPGRADE_FOR_CLASS);
    append_valid_event_type(STATUS_INITING, W2L_SYNC_DEB_PATH);
    append_valid_event_type(STATUS_INITING, W2L_MINA_READY);
    append_valid_event_type(STATUS_INITING, W2L_NOTIFY_CHECK_VM_STATUS);
    append_valid_event_type(STATUS_INITING, W2L_SYNC_SOFTWARE_VERSION);
    append_valid_event_type(STATUS_INITING, W2L_SYNC_TERMINAL_PSW);
    append_valid_event_type(STATUS_INITING, W2L_SYNC_PUBLIC_POLICY);
    append_valid_event_type(STATUS_INITING, W2L_SYNC_MODE);
    append_valid_event_type(STATUS_INITING, W2L_SYNC_HOSTNAME);
    append_valid_event_type(STATUS_INITING, W2L_SYNC_SERVER_TIME);
    append_valid_event_type(STATUS_INITING, W2L_SYNC_IPXE);
    append_valid_event_type(STATUS_INITING, W2L_SYNC_RECOVER_IMAGE);
    //append_valid_event_type(STATUS_INITING, W2L_SYNC_RELOAD_IMAGE);
    append_valid_event_type(STATUS_INITING, W2L_SYNC_DELETE_TEACHERDISK);
    
    append_valid_event_type(STATUS_INITING, W2L_NOTIFY_HTTP_PORT);
    
    append_valid_event_type(STATUS_INITING, W2L_SYNC_RESET_TERMINAL);
    append_valid_event_type(STATUS_INITING, W2L_SYNC_ALL_USERINFO);
    append_valid_event_type(STATUS_INITING, W2L_SYNC_SERVER_IP);
    append_valid_event_type(STATUS_INITING, W2L_NOTIFY_SHUTDOWN);
    append_valid_event_type(STATUS_INITING, W2L_NOTIFY_REBOOT);
    append_valid_event_type(STATUS_INITING, W2L_NOTIFY_RECORVER_IMAGE);
    append_valid_event_type(STATUS_INITING, W2L_NOTIFY_COLLECT_LOG);
    append_valid_event_type(STATUS_INITING, W2L_NOTIFY_MODIFY_LOCAL_NETWORK);
    append_valid_event_type(STATUS_INITING, W2L_NOTIFY_MODIFY_VM_NETWORK);
    append_valid_event_type(STATUS_INITING, W2L_NOTIFY_MODIFY_HOSTNAME);
    append_valid_event_type(STATUS_INITING, W2L_NOTIFY_MODIFY_PASSWORD);
    append_valid_event_type(STATUS_INITING, W2L_MODIFY_PASSWORD_SUCCESS);
    append_valid_event_type(STATUS_INITING, W2L_MODIFY_PASSWORD_FAIL);
    append_valid_event_type(STATUS_INITING, W2L_NOTIFY_RESET_TO_INITIAL);
    //append_valid_event_type(STATUS_INITING, W2L_NOTIFY_RELOAD_IMAGE);
    append_valid_event_type(STATUS_INITING, W2L_NOTIFY_DELETE_TEACHERDISK);
    append_valid_event_type(STATUS_INITING, W2L_INIT_SYNC_ERROR);
    append_valid_event_type(STATUS_INITING, W2L_NOTIFY_RESYNC);
    append_valid_event_type(STATUS_INITING, W2L_NOTIFY_DO_IPXE);
    append_valid_event_type(STATUS_INITING, W2L_SYNC_DEV_POLICY);
    append_valid_event_type(STATUS_INITING, W2L_SYNC_DRIVER_INSTALL);
    append_valid_event_type(STATUS_INITING, W2L_SYNC_PORT_MAPPING);
    append_valid_event_type(STATUS_INITING, W2L_SYNC_SSID_WHITELIST);
    append_valid_event_type(STATUS_INITING, W2L_SYNC_PRINTER_SWITCH);
    append_valid_event_type(STATUS_INITING, W2L_SYNC_DESKTOP_REDIR);
    append_valid_event_type(STATUS_INITING, W2L_SYNC_RCDUSBCONF_INFO);
    append_valid_event_type(STATUS_INITING, V2L_GET_VM_INFO_RESULT);
    append_valid_event_type(STATUS_INITING, W2L_NODIFY_SSID_WHITELIST);
    append_valid_event_type(STATUS_INITING, W2L_NODIFY_RESET_NETDISK);
    append_valid_event_type(STATUS_INITING, W2L_SYNC_MAIN_WINDOW);
    append_valid_event_type(STATUS_INITING, W2L_GET_VM_DEV_INTERFACE_INFO);
    // on local mode
    append_valid_event_type(STATUS_CHECKING_LOCAL, U2L_SET_NET_INFO);
    append_valid_event_type(STATUS_CHECKING_LOCAL, U2L_SET_VM_NET_INFO);
    append_valid_event_type(STATUS_CHECKING_LOCAL, U2L_SET_SERVER_IP);
    append_valid_event_type(STATUS_CHECKING_LOCAL, U2L_SET_HOST_NAME);
    append_valid_event_type(STATUS_CHECKING_LOCAL, U2L_SET_WIFI_STATUS);
	append_valid_event_type(STATUS_CHECKING_LOCAL, U2L_SET_AUTH_INFO);
	append_valid_event_type(STATUS_CHECKING_LOCAL, U2L_LOCALMODE_QUIT);
    append_valid_event_type(STATUS_CHECKING_LOCAL, U2L_LOCALMODE_CHECKERROR);
    append_valid_event_type(STATUS_CHECKING_LOCAL, U2L_ACCOUNT_LOGIN);
    append_valid_event_type(STATUS_CHECKING_LOCAL, U2L_PUBLIC_LOGIN);
    append_valid_event_type(STATUS_CHECKING_LOCAL, U2L_GUEST_LOGIN);
    append_valid_event_type(STATUS_CHECKING_LOCAL, U2L_SHUTDOWN_TERMINAL);
    append_valid_event_type(STATUS_CHECKING_LOCAL, U2L_REBOOT_TERMINAL);
    append_valid_event_type(STATUS_CHECKING_LOCAL, U2L_ENTER_SETTINGS);
    append_valid_event_type(STATUS_CHECKING_LOCAL, U2L_ENTER_WIFI_CONFIG);
    append_valid_event_type(STATUS_CHECKING_LOCAL, U2L_ENTER_AUTH_CONFIG);
    
    append_valid_event_type(STATUS_CHECKING_LOCAL, U2L_NOTIFY_SETTING_RESULT);
    append_valid_event_type(STATUS_CHECKING_LOCAL, U2L_NOTIFY_WIFI_CONFIG_RESULT);
    append_valid_event_type(STATUS_CHECKING_LOCAL, U2L_NOTIFY_AUTH_CONFIG_RESULT);
    append_valid_event_type(STATUS_CHECKING_LOCAL, U2L_SAVE_POWER_BOOT);

    append_valid_event_type(STATUS_CHECKING_LOCAL, V2L_IMAGE_EXIST_RESULT);

    // on waiting login state.
    append_valid_event_type(STATUS_WAITING_LOGIN, L2L_EASY_DEPLOY);
    append_valid_event_type(STATUS_WAITING_LOGIN, L2L_LINK_UP);
    append_valid_event_type(STATUS_WAITING_LOGIN, L2L_LINK_DOWN);
    append_valid_event_type(STATUS_WAITING_LOGIN, L2L_LINK1_UP);
    append_valid_event_type(STATUS_WAITING_LOGIN, L2L_LINK1_DOWN);
    append_valid_event_type(STATUS_WAITING_LOGIN, L2L_DOWNLOAD_SUCCESS_TIPS);
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_SET_NET_INFO);
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_SET_VM_NET_INFO);
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_SET_SERVER_IP);
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_SET_HOST_NAME);
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_SET_WIFI_STATUS);
    // Action that UI changes mode, could do only on waiting login interface.
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_SET_TERMINAL_MODE);
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_SET_AUTH_INFO);
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_ACCOUNT_LOGIN);
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_PUBLIC_LOGIN);
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_GUEST_LOGIN);
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_MODIFY_PASSWORD);
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_SHUTDOWN_TERMINAL);
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_REBOOT_TERMINAL);
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_ENTER_SETTINGS);
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_ENTER_WIFI_CONFIG);
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_ENTER_AUTH_CONFIG);
    
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_NOTIFY_SETTING_RESULT);
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_NOTIFY_WIFI_CONFIG_RESULT);
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_NOTIFY_AUTH_CONFIG_RESULT);
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_SAVE_POWER_BOOT);
    append_valid_event_type(STATUS_WAITING_LOGIN, U2L_ENTER_LOCAL_MODE);

    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_MINA_CONNECTION_ESTABLISHED);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_UPGRADE_FOR_OFFICE);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_MINA_CONNECTION_DESTROYED);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_NOTIFY_CHECK_VM_STATUS);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_NOTIFY_SHUTDOWN);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_NOTIFY_REBOOT);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_NOTIFY_RECORVER_IMAGE);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_NOTIFY_COLLECT_LOG);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_NOTIFY_MODIFY_LOCAL_NETWORK);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_NOTIFY_MODIFY_VM_NETWORK);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_NOTIFY_MODIFY_HOSTNAME);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_NOTIFY_MODIFY_PASSWORD);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_MODIFY_PASSWORD_SUCCESS);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_MODIFY_PASSWORD_FAIL);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_NOTIFY_RESET_TO_INITIAL);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_NOTIFY_RESYNC);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_NOTIFY_DO_IPXE);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_NODIFY_SSID_WHITELIST);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_NODIFY_RESET_NETDISK);
    //append_valid_event_type(STATUS_WAITING_LOGIN, W2L_NOTIFY_RELOAD_IMAGE);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_NOTIFY_DELETE_TEACHERDISK);
    
    /* idv over wan */
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_NOTIFY_HTTP_PORT);

    append_valid_event_type(STATUS_WAITING_LOGIN, V2L_GET_VM_INFO_RESULT);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_SYNC_DEV_POLICY);
    append_valid_event_type(STATUS_WAITING_LOGIN, W2L_SYNC_MAIN_WINDOW);

    // on checking login state.
    append_valid_event_type(STATUS_CHECKING_LOGIN, U2L_BIND_USER);
    append_valid_event_type(STATUS_CHECKING_LOGIN, L2L_LINK_UP);
    append_valid_event_type(STATUS_CHECKING_LOGIN, L2L_LINK_DOWN);
    append_valid_event_type(STATUS_CHECKING_LOGIN, L2L_LINK1_UP);
    append_valid_event_type(STATUS_CHECKING_LOGIN, L2L_LINK1_DOWN);
    append_valid_event_type(STATUS_CHECKING_LOGIN, L2L_DOWNLOAD_SUCCESS_TIPS);
    append_valid_event_type(STATUS_CHECKING_LOGIN, W2L_MINA_CONNECTION_ESTABLISHED);
    append_valid_event_type(STATUS_CHECKING_LOGIN, W2L_UPGRADE_FOR_OFFICE);
    append_valid_event_type(STATUS_CHECKING_LOGIN, W2L_MINA_CONNECTION_DESTROYED);
    append_valid_event_type(STATUS_CHECKING_LOGIN, W2L_NOTIFY_CHECK_VM_STATUS);
    append_valid_event_type(STATUS_CHECKING_LOGIN, W2L_LOGIN_SUCCESS);
    append_valid_event_type(STATUS_CHECKING_LOGIN, W2L_LOGIN_FAIL);
    append_valid_event_type(STATUS_CHECKING_LOGIN, W2L_BIND_SECCESS);
    append_valid_event_type(STATUS_CHECKING_LOGIN, W2L_BIND_FAIL);
    append_valid_event_type(STATUS_CHECKING_LOGIN, W2L_NOTIFY_SHUTDOWN);
    append_valid_event_type(STATUS_CHECKING_LOGIN, W2L_NOTIFY_REBOOT);
    //append_valid_event_type(STATUS_CHECKING_LOGIN, W2L_NOTIFY_RECORVER_IMAGE);
    //TODO: Shall we recover image here?
    append_valid_event_type(STATUS_CHECKING_LOGIN, W2L_NOTIFY_COLLECT_LOG);
    append_valid_event_type(STATUS_CHECKING_LOGIN, W2L_NOTIFY_MODIFY_LOCAL_NETWORK);
    //TODO: if local network modified, connection will be destroyed. Is it valid?
    append_valid_event_type(STATUS_CHECKING_LOGIN, W2L_NOTIFY_MODIFY_VM_NETWORK);
    append_valid_event_type(STATUS_CHECKING_LOGIN, W2L_NOTIFY_MODIFY_HOSTNAME);
    append_valid_event_type(STATUS_CHECKING_LOGIN, W2L_NOTIFY_RESYNC);

    append_valid_event_type(STATUS_CHECKING_LOGIN, V2L_GET_VM_INFO_RESULT);
    append_valid_event_type(STATUS_CHECKING_LOGIN, W2L_SYNC_DEV_POLICY);
    append_valid_event_type(STATUS_CHECKING_LOGIN, W2L_SYNC_DEV_POLICY);
    
    append_valid_event_type(STATUS_CHECKING_LOGIN, W2L_NOTIFY_HTTP_PORT);
    
    append_valid_event_type(STATUS_CHECKING_LOGIN, W2L_SYNC_MAIN_WINDOW);

    // on preparing image state.
    append_valid_event_type(STATUS_PREPARING_IMAGE, L2L_LINK_UP);
    append_valid_event_type(STATUS_PREPARING_IMAGE, L2L_LINK_DOWN);
    append_valid_event_type(STATUS_PREPARING_IMAGE, L2L_LINK1_UP);
    append_valid_event_type(STATUS_PREPARING_IMAGE, L2L_LINK1_DOWN);
    append_valid_event_type(STATUS_PREPARING_IMAGE, L2L_DOWNLOAD_SUCCESS_TIPS);
    append_valid_event_type(STATUS_PREPARING_IMAGE, W2L_MINA_CONNECTION_ESTABLISHED);
    append_valid_event_type(STATUS_PREPARING_IMAGE, W2L_UPGRADE_FOR_OFFICE);
    append_valid_event_type(STATUS_PREPARING_IMAGE, W2L_MINA_CONNECTION_DESTROYED);
    append_valid_event_type(STATUS_PREPARING_IMAGE, W2L_NOTIFY_CHECK_VM_STATUS);
    append_valid_event_type(STATUS_PREPARING_IMAGE, W2L_SYNC_IMAGE);
    append_valid_event_type(STATUS_PREPARING_IMAGE, W2L_NOTIFY_SHUTDOWN);
    append_valid_event_type(STATUS_PREPARING_IMAGE, W2L_NOTIFY_REBOOT);
    append_valid_event_type(STATUS_PREPARING_IMAGE, W2L_NOTIFY_COLLECT_LOG);
    append_valid_event_type(STATUS_PREPARING_IMAGE, W2L_NOTIFY_MODIFY_VM_NETWORK);
    append_valid_event_type(STATUS_PREPARING_IMAGE, W2L_NOTIFY_MODIFY_HOSTNAME);
    append_valid_event_type(STATUS_PREPARING_IMAGE, W2L_IMAGE_SYNC_ERROR);
    append_valid_event_type(STATUS_PREPARING_IMAGE, W2L_NOTIFY_RESYNC);
    append_valid_event_type(STATUS_PREPARING_IMAGE, W2L_NOTIFY_DO_IPXE);
    append_valid_event_type(STATUS_PREPARING_IMAGE, W2L_NODIFY_SSID_WHITELIST);
    append_valid_event_type(STATUS_PREPARING_IMAGE, W2L_NODIFY_RESET_NETDISK);
    
    append_valid_event_type(STATUS_PREPARING_IMAGE, W2L_NOTIFY_HTTP_PORT);
    
    append_valid_event_type(STATUS_PREPARING_IMAGE, W2L_SYNC_MAIN_WINDOW);
    append_valid_event_type(STATUS_PREPARING_IMAGE, V2L_IMAGE_EXIST_RESULT);
    append_valid_event_type(STATUS_PREPARING_IMAGE, V2L_GET_VM_INFO_RESULT);
    append_valid_event_type(STATUS_PREPARING_IMAGE, W2L_SYNC_DEV_POLICY);
    append_valid_event_type(STATUS_PREPARING_IMAGE, U2L_SET_NET_INFO);
    append_valid_event_type(STATUS_PREPARING_IMAGE, U2L_SET_VM_NET_INFO);
    append_valid_event_type(STATUS_PREPARING_IMAGE, U2L_SET_SERVER_IP);
    append_valid_event_type(STATUS_PREPARING_IMAGE, U2L_SET_HOST_NAME);
    append_valid_event_type(STATUS_PREPARING_IMAGE, U2L_SET_WIFI_STATUS);
    append_valid_event_type(STATUS_PREPARING_IMAGE, U2L_SET_TERMINAL_MODE);
    append_valid_event_type(STATUS_PREPARING_IMAGE, U2L_SET_AUTH_INFO);
    append_valid_event_type(STATUS_PREPARING_IMAGE, U2L_SHUTDOWN_TERMINAL);
    append_valid_event_type(STATUS_PREPARING_IMAGE, U2L_REBOOT_TERMINAL);
    append_valid_event_type(STATUS_PREPARING_IMAGE, U2L_ENTER_SETTINGS);
    append_valid_event_type(STATUS_PREPARING_IMAGE, U2L_ENTER_WIFI_CONFIG);
    append_valid_event_type(STATUS_PREPARING_IMAGE, U2L_ENTER_AUTH_CONFIG);
    
    append_valid_event_type(STATUS_PREPARING_IMAGE, U2L_NOTIFY_SETTING_RESULT);
    append_valid_event_type(STATUS_PREPARING_IMAGE, U2L_NOTIFY_WIFI_CONFIG_RESULT);
    append_valid_event_type(STATUS_PREPARING_IMAGE, U2L_NOTIFY_AUTH_CONFIG_RESULT);
    append_valid_event_type(STATUS_PREPARING_IMAGE, U2L_SAVE_POWER_BOOT);

    // on downloading image state.
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, L2L_LINK_UP);
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, L2L_LINK_DOWN);
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, L2L_LINK1_UP);
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, L2L_LINK1_DOWN);
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, L2L_DOWNLOAD_SUCCESS_TIPS);
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, W2L_MINA_CONNECTION_ESTABLISHED);
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, W2L_UPGRADE_FOR_OFFICE);
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, W2L_MINA_CONNECTION_DESTROYED);
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, W2L_NOTIFY_CHECK_VM_STATUS);
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, W2L_NOTIFY_SHUTDOWN);
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, W2L_NOTIFY_REBOOT);
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, W2L_NOTIFY_COLLECT_LOG);
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, W2L_NOTIFY_MODIFY_VM_NETWORK);
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, W2L_NOTIFY_MODIFY_HOSTNAME);
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, W2L_NODIFY_SSID_WHITELIST);
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, W2L_NODIFY_RESET_NETDISK);
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, W2L_NOTIFY_RESYNC);
    
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, W2L_NOTIFY_HTTP_PORT);
    
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, W2L_SYNC_MAIN_WINDOW);

    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, V2L_VM_DOWNLOAD_PROGRESS_STATUS);
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, V2L_VM_DOWNLOAD_QUIT_RESULT);
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, V2L_GET_VM_INFO_RESULT);
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, W2L_SYNC_DEV_POLICY);
    append_valid_event_type(STATUS_DOWNLOADING_IMAGE, U2L_USB_COPY_BASE);

    // on running vm state.
    append_valid_event_type(STATUS_RUNNING_VM, W2L_MINA_CONNECTION_ESTABLISHED);
    append_valid_event_type(STATUS_RUNNING_VM, W2L_UPGRADE_FOR_OFFICE);
    append_valid_event_type(STATUS_RUNNING_VM, W2L_MINA_CONNECTION_DESTROYED);
    append_valid_event_type(STATUS_RUNNING_VM, W2L_MINA_READY);
    append_valid_event_type(STATUS_RUNNING_VM, W2L_NOTIFY_CHECK_VM_STATUS);
    append_valid_event_type(STATUS_RUNNING_VM, W2L_NOTIFY_SHUTDOWN);
    append_valid_event_type(STATUS_RUNNING_VM, W2L_NOTIFY_REBOOT);
    append_valid_event_type(STATUS_RUNNING_VM, W2L_NOTIFY_COLLECT_LOG);
    append_valid_event_type(STATUS_RUNNING_VM, W2L_NOTIFY_MODIFY_LOCAL_NETWORK);
    append_valid_event_type(STATUS_RUNNING_VM, W2L_NOTIFY_MODIFY_VM_NETWORK);
    append_valid_event_type(STATUS_RUNNING_VM, W2L_NOTIFY_MODIFY_HOSTNAME);
    append_valid_event_type(STATUS_RUNNING_VM, W2L_NODIFY_RESET_NETDISK);
    append_valid_event_type(STATUS_RUNNING_VM, W2L_RECV_GUESTTOOL_MSG);
    append_valid_event_type(STATUS_RUNNING_VM, W2L_NOTIFY_DO_IPXE);
    append_valid_event_type(STATUS_RUNNING_VM, W2L_NOTIFY_HTTP_PORT);

    append_valid_event_type(STATUS_RUNNING_VM, V2L_VM_START_RESULT);
    append_valid_event_type(STATUS_RUNNING_VM, V2L_VM_SHUTDOWN_RESULT);
    append_valid_event_type(STATUS_RUNNING_VM, V2L_VM_REBOOT_RESULT);
    append_valid_event_type(STATUS_RUNNING_VM, V2L_GET_VM_INFO_RESULT);
    // FIXME: events as following are unuseful?
    append_valid_event_type(STATUS_RUNNING_VM, V2L_SET_VM_NET_POLICY_RESULT);
    append_valid_event_type(STATUS_RUNNING_VM, V2L_SET_VM_USB_POLICY_RESULT);
    append_valid_event_type(STATUS_RUNNING_VM, V2L_SET_VM_DISKINFO_RESULT);
    append_valid_event_type(STATUS_RUNNING_VM, V2L_SET_VM_NETINFO_RESULT);
    append_valid_event_type(STATUS_RUNNING_VM, V2L_GET_VM_NETINFO_RESULT);
    append_valid_event_type(STATUS_RUNNING_VM, V2L_SET_LOCAL_WIRED_NETWORK);
    //append_valid_event_type(STATUS_RUNNING_VM, W2L_SYNC_DEV_POLICY);
    append_valid_event_type(STATUS_RUNNING_VM, L2L_LINK_UP);
    append_valid_event_type(STATUS_RUNNING_VM, L2L_LINK_DOWN);
    append_valid_event_type(STATUS_RUNNING_VM, L2L_LINK1_UP);
    append_valid_event_type(STATUS_RUNNING_VM, L2L_LINK1_DOWN);
    append_valid_event_type(STATUS_RUNNING_VM, L2L_DOWNLOAD_SUCCESS_TIPS);
    


    // on new device deploy state.
    append_valid_event_type(STATUS_NEW_DEPLOY, U2L_PASS_NEW_DEPLOY_MODEINFO);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_MINA_CONNECTION_ESTABLISHED);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_MINA_CONNECTION_DESTROYED);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_UPGRADE_FOR_OFFICE);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_UPGRADE_FOR_CLASS);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_DEB_PATH);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_MINA_READY);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_SOFTWARE_VERSION);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_TERMINAL_PSW);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_PUBLIC_POLICY);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_MODE);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_HOSTNAME);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_IPXE);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_RECOVER_IMAGE);
    //append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_RELOAD_IMAGE);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_PORT_MAPPING);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_DELETE_TEACHERDISK);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_RESET_TERMINAL);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_ALL_USERINFO);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_SSID_WHITELIST);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_PRINTER_SWITCH);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_DESKTOP_REDIR);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_RCDUSBCONF_INFO);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_SERVER_TIME);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_MAIN_WINDOW);

    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_NODIFY_SSID_WHITELIST);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_NODIFY_RESET_NETDISK);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_SERVER_IP);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_INIT_SYNC_ERROR);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_BIND_SECCESS);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_BIND_FAIL);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_IMAGE);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_IMAGE_SYNC_ERROR);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_NOTIFY_CHECK_VM_STATUS);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_NOTIFY_SHUTDOWN);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_NOTIFY_REBOOT);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_NOTIFY_COLLECT_LOG);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_NOTIFY_DO_IPXE);
    
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_NOTIFY_HTTP_PORT);
    
    append_valid_event_type(STATUS_NEW_DEPLOY, V2L_VM_DOWNLOAD_PROGRESS_STATUS);
    append_valid_event_type(STATUS_NEW_DEPLOY, V2L_VM_DOWNLOAD_QUIT_RESULT);
    append_valid_event_type(STATUS_NEW_DEPLOY, V2L_GET_VM_INFO_RESULT);

    append_valid_event_type(STATUS_NEW_DEPLOY, L2L_EASY_DEPLOY);
    append_valid_event_type(STATUS_NEW_DEPLOY, L2L_LINK_UP);
    append_valid_event_type(STATUS_NEW_DEPLOY, L2L_LINK_DOWN);
    append_valid_event_type(STATUS_NEW_DEPLOY, L2L_LINK1_UP);
    append_valid_event_type(STATUS_NEW_DEPLOY, L2L_LINK1_DOWN);   
    append_valid_event_type(STATUS_NEW_DEPLOY, L2L_DATA_PROTECT); 
    append_valid_event_type(STATUS_NEW_DEPLOY, L2L_DOWNLOAD_SUCCESS_TIPS); 
    append_valid_event_type(STATUS_NEW_DEPLOY, U2L_SET_NET_INFO);
    append_valid_event_type(STATUS_NEW_DEPLOY, U2L_SET_VM_NET_INFO);
    append_valid_event_type(STATUS_NEW_DEPLOY, U2L_SET_SERVER_IP);
    append_valid_event_type(STATUS_NEW_DEPLOY, U2L_SET_HOST_NAME);
    append_valid_event_type(STATUS_NEW_DEPLOY, U2L_SET_WIFI_STATUS);
    append_valid_event_type(STATUS_NEW_DEPLOY, U2L_SHUTDOWN_TERMINAL);
    append_valid_event_type(STATUS_NEW_DEPLOY, U2L_REBOOT_TERMINAL);
    append_valid_event_type(STATUS_NEW_DEPLOY, U2L_ENTER_LOCAL_MODE);
    append_valid_event_type(STATUS_NEW_DEPLOY, U2L_ENTER_SETTINGS);
    append_valid_event_type(STATUS_NEW_DEPLOY, U2L_ENTER_WIFI_CONFIG);
    append_valid_event_type(STATUS_NEW_DEPLOY, U2L_ENTER_AUTH_CONFIG);
    append_valid_event_type(STATUS_NEW_DEPLOY, U2L_NOTIFY_SETTING_RESULT);
    append_valid_event_type(STATUS_NEW_DEPLOY, U2L_NOTIFY_WIFI_CONFIG_RESULT);
    append_valid_event_type(STATUS_NEW_DEPLOY, U2L_NOTIFY_AUTH_CONFIG_RESULT);
    append_valid_event_type(STATUS_NEW_DEPLOY, U2L_SET_TERMINAL_MODE);
    append_valid_event_type(STATUS_NEW_DEPLOY, U2L_SET_AUTH_INFO);
    append_valid_event_type(STATUS_NEW_DEPLOY, U2L_SAVE_POWER_BOOT);
    append_valid_event_type(STATUS_NEW_DEPLOY, U2L_USB_COPY_BASE);
    append_valid_event_type(STATUS_NEW_DEPLOY, U2L_UPGRADE_FOR_CLASS);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_DEV_POLICY);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_SYNC_DRIVER_INSTALL);
    append_valid_event_type(STATUS_NEW_DEPLOY, W2L_GET_VM_DEV_INTERFACE_INFO);
}

bool LogicStatusMachine::check_valid_event_type(int type)
{
    itValidEvents it;
    it = _status_valid_events[_status].find(type);
    if (it == _status_valid_events[_status].end()) {
#if 0
        LOG_WARNING("invalid event, cur status is %d, event is %d", _status, type);
#else
        LOG_WARNING("Event '%s' ignored, for current is on state '%s'\n",
            Application::logic_event_desc(type).c_str(),
            Application::logic_state_desc(_status).c_str());
#endif
        return false;
    } else {
        return true;
    }
}

int LogicStatusMachine::get_next_status(int event_type)
{
    int next_status = 0;

    // any state received events as follows forced to be changed.
    if (event_type == EVENT_WEB_DISCONNECT && _status != STATUS_CHECKING_LOCAL) {
        next_status = STATUS_INITING;
        return next_status;
    }

    switch (_status) {
    case STATUS_NONE:
        ASSERT(event_type == EVENT_INIT);
        next_status = STATUS_INITING;
        break;
    case STATUS_INITING:
        next_status = get_initing_next_status(event_type);
        break;
    case STATUS_CHECKING_LOCAL:
        next_status = get_checking_local_next_status(event_type);
        break;
    case STATUS_WAITING_LOGIN:
        next_status = get_waiting_login_next_status(event_type);
        break;
    case STATUS_CHECKING_LOGIN:
        next_status = get_checking_login_next_status(event_type);
        break;
    case STATUS_PREPARING_IMAGE:
        next_status = get_preparing_image_next_status(event_type);
        break;
    case STATUS_DOWNLOADING_IMAGE:
        next_status = get_downloading_image_next_status(event_type);
        break;
    case STATUS_RUNNING_VM:
        next_status = _status;  //STATUS_RUNNING_VM is a final state.
        break;
    case STATUS_NEW_DEPLOY:
        next_status = get_new_deploy_next_status(event_type);
        break;
    default:
        ASSERT(0);
        break;
    }
    LOG_INFO("event_type: %d, cur_status: %d, next_status: %d", event_type, _status, next_status);

    return next_status;
}

int LogicStatusMachine::get_initing_next_status(int event_type)
{
    switch(event_type)
    {
        case EVENT_DRIVER_INSTALL_RECEIVED:
            return STATUS_RUNNING_VM;

        case EVENT_WEB_INFO_SYNCHRONIZED:
            return STATUS_PREPARING_IMAGE;
            
        case EVENT_LOCAL_MODE_CHOOSEN:
            return STATUS_CHECKING_LOCAL;

        case EVENT_VM_RUNNING:
            return STATUS_RUNNING_VM;
            
        case EVENT_ENTER_NEW_DEPLOY:
            return STATUS_NEW_DEPLOY;

        default:
            return _status;
    }
}

int LogicStatusMachine::get_checking_local_next_status(int event_type)
{ 
    switch(event_type)
    {
        case EVENT_LOCAL_CHECK_SUCCESS:
            return STATUS_RUNNING_VM;
            
        case EVENT_LOCAL_CHECK_FAILED:
            return STATUS_INITING;
            
        default:
            return _status;
    }
}

int LogicStatusMachine::get_waiting_login_next_status(int event_type)
{
    bool image_exist = false;

    if(_obj->_vm->vm_check_local_base_exist() == 1)
    {
        image_exist = true;
    }
    
    switch(event_type)
    {
        case EVENT_LOGIN_BUTTON_ENTERED:
            if(_obj->_mode_data.get().mode == PUBLIC_MODE)
            {
                //check if image exist
                if(image_exist)
                {
                    return STATUS_RUNNING_VM;
                }
                else
                {
                    //return to login UI
                    LOG_DEBUG("public mode has no image!");
                    l2u_result_user(2);
                    /* why commented? */
//                  sleep(2);
//                  LOG_DEBUG("call l2u_show_publiclogin");
//                  l2u_show_publiclogin();
                }
            }
            else if(_obj->_logined && _obj->_logined_user_info.username == "guest")
            {
                return STATUS_RUNNING_VM;
            }
            else
            {
                return STATUS_CHECKING_LOGIN;
            }
        case EVENT_LOCAL_MODE_CHOOSEN:
            if (_obj->_mode_data.get().mode != PUBLIC_MODE && (!_obj->_logined && _obj->_logined_user_info.username != "guest")) {
                return STATUS_CHECKING_LOCAL;
            }
        default:
            return _status;
    }
}

int LogicStatusMachine::get_checking_login_next_status(int event_type)
{
    switch(event_type)
    {
        case EVENT_LOGIN_SUCCESS:
            //special mode & fisrt bind, download user image, else running VM
            if(_obj->_mode_data.get().mode == SPECIAL_MODE && _obj->_mode_data.get().bind_user.username.empty())
            {
                return STATUS_PREPARING_IMAGE;
            }
            else
            {
                return STATUS_RUNNING_VM;
            }
            
        case EVENT_LOGIN_FAILED:
            return STATUS_WAITING_LOGIN;
            
        default:
            return _status;
    }
}

int LogicStatusMachine::get_preparing_image_next_status(int event_type)
{
    switch(event_type)
    {
        case EVENT_IMAGE_LATEST:
            switch(_obj->_mode_data.get().mode)
            {
                //SPECIAL_MODE may prepare image befor or after login
                case SPECIAL_MODE: 
                    if(_obj->_logined)
                    {
                        return STATUS_RUNNING_VM;
                    }
                    else
                    {
                        return STATUS_WAITING_LOGIN;
                    }
                    
                case MULTIUSER_MODE:
                case PUBLIC_MODE:
                    return STATUS_WAITING_LOGIN;
                    
                default:
                	return _status;
            }
            
        case EVENT_IMAGE_NEED_DOWNLOAD:
            return STATUS_DOWNLOADING_IMAGE;
            
        case EVENT_IMAGE_SYNC_ERROR:
            //this error happend only when no base in local, or sync bind user's image error, so must return to STATUS_INITING              
            return STATUS_INITING;
            
        default:
            return _status;
    }
}

int LogicStatusMachine::get_downloading_image_next_status(int event_type)
{
    switch(event_type)
    {
        case EVENT_DOWNLOAD_IMAGE_SUCCEESS:
            switch(_obj->_mode_data.get().mode)
            {
                case SPECIAL_MODE:
                    if(_obj->_logined)
                    {
                        return STATUS_RUNNING_VM;
                    }
                    else
                    {
                        return STATUS_WAITING_LOGIN;
                    }
                    
                case MULTIUSER_MODE:
                case PUBLIC_MODE:
                    return STATUS_WAITING_LOGIN;
            }
            
        case EVENT_DOWNLOAD_IMAGE_FAILED:
            return STATUS_PREPARING_IMAGE;
        default:
            return _status;
    }
}

int LogicStatusMachine::get_new_deploy_next_status(int event_type)
{
    switch(event_type)
    {
        case EVENT_INIT:
            return STATUS_INITING;
        case EVENT_LOCAL_MODE_CHOOSEN:
            return STATUS_CHECKING_LOCAL;
        case EVENT_WEB_INFO_SYNCHRONIZED:
            return STATUS_PREPARING_IMAGE;
        case EVENT_DRIVER_INSTALL_RECEIVED:
            return STATUS_RUNNING_VM;
        case EVENT_DOWNLOAD_IMAGE_FAILED:
            return STATUS_INITING;
        default:
            return _status;
    }
}

void LogicStatusMachine::reset_status()
{
    LOG_DEBUG("======================reset status!=====================");
    _status = STATUS_NONE;
}

