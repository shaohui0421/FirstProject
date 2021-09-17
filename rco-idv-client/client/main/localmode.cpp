/*
 * localmode.cpp
 *
 *  Created on: Feb 16, 2017
 *      Author: zhf
 */

#include <user_mgr.h>
#include "rc/rc_public.h"
#include "localmode.h"

#define LASTONLINETIME	RCC_DATA_PATH "lastonlinetime"

LocalMode::LocalMode(Application* _app, struct ModeInfo _mode)
	: app(_app),
	  mode(_mode),
	  curstatus(LocalMode_Status_Init),
	  last_err_status(LocalMode_Status_Init)
{
	LOG_DEBUG("create LocalMode Class");
}

LocalMode::~LocalMode(){
	LOG_DEBUG("del LocalMode Class");
}

int LocalMode::ResetStatus(){
	UserInfo tmp;
	userinfo = tmp;
	curstatus = LocalMode_Status_Init;
	mode = app->_mode_data.get();
	last_err_status = LocalMode_Status_Init;
	return 0;
}

int LocalMode::LM_WEB_Update_OnLineTime(){
	LOG_INFO("update online time");
	return rc_system("date +%s > " LASTONLINETIME);
}

int LocalMode::push_event(Event* event) {
	app->Req_PushEvent(event);
	return 0;
}

int LocalMode::UI_Req_ChangeToLocalMode() {
	if (LocalMode_Status_Init == curstatus) {
#if 1
		curstatus = LocalMode_Status_CheckImage;
		if(app->_vm->vm_check_local_base_exist() == 1) {
			return VM_Ack_LocalImageStatus(LocalImageStatus_exist);
		}else {
			return VM_Ack_LocalImageStatus(LocalImageStatus_none);
		}
		return -1;
#else
		int ret = 0;
		app->_vm->vm_check_base_exist();
		if (ret == 0) {
			LOG_INFO("Entry Check LocalImage if OK will into LocalMode");
			curstatus = LocalMode_Status_CheckImage;
			return 0;
		}else {
			LOG_ERR("failed to check LocalImage");
		}
		return ret;
#endif
	}
	LOG_ERR("invalid ChangeTo LocalMode");
	return -1;
}


int LocalMode::callback(int ret, void *obj, void *data) {
	return 0;
}

int LocalMode::LM_LocalCheckError()
{
	switch (curstatus)
	{
		case LocalMode_Status_CheckImage:
			last_err_status = LocalMode_Status_CheckImage;
			LOG_ERR("locale image no exist");
			l2u_show_tips(UI_TIPS_DEV_NO_IMG_OPT);
//			l2u_ctrl_winbtn(false); //disable setting about poweroff button
			curstatus = LocalMode_Status_CheckError;
			return 0;
		case LocalMode_Status_CheckOffLineTimeOut:
			last_err_status = LocalMode_Status_CheckOffLineTimeOut;
			LOG_ERR("localemode offline timeout");
			l2u_show_tips(UI_TIPS_DEV_FORBIDED_OPT);
//			l2u_ctrl_winbtn(false); //disable setting about poweroff button
			curstatus = LocalMode_Status_CheckError;
			return 0;
		default:
			break;
	}
	return -1;
}

int LocalMode::VM_Ack_LocalImageStatus(LocalImageStatus ret) {
	if (LocalMode_Status_CheckImage == curstatus) {
		LOG_NOTICE("VM ack localimage status:%d", ret);
		if (LocalImageStatus_exist == ret) {
			if (PUBLIC_MODE == mode.mode) {
				curstatus = LocalMode_Status_CheckTermMode;
				return LM_Check_CurMode();
			}
			curstatus = LocalMode_Status_CheckOffLineTimeOut;
			return LM_Check_OffLineTimeOut();
		}else {
			return LM_LocalCheckError();
		}
	}else {
		LOG_ERR("current status invalid");
	}
	return -1;
}

int LocalMode::LM_Check_OffLineTimeOut() {
	if (LocalMode_Status_CheckOffLineTimeOut == curstatus) {
		int ret, size;
		long int offlinetime;
		int outctrl_day = 0;
		unsigned char lastonlinetime[0x20], curtime[0x20];
		LOG_NOTICE("localimage check offline timeout");
		size = sizeof(lastonlinetime);
		ret = rc_system_rw("cat " LASTONLINETIME, lastonlinetime, &size, "r");
		if (ret != 0){
			LOG_ERR("read last onlinetime error");
			return LM_LocalCheckError();
		}
		lastonlinetime[sizeof(lastonlinetime) - 1] = 0;
		LOG_DEBUG("lastonlinetime:%s", lastonlinetime);

		size = sizeof(curtime);
		ret = rc_system_rw("date +%s", curtime, &size, "r");
		if (ret != 0){
			LOG_ERR("read last onlinetime error");
			return LM_LocalCheckError();
		}
		curtime[sizeof(curtime) - 1] = 0;
		LOG_DEBUG("curtime:%s", curtime);

		offlinetime = atol((const char *)curtime) - atol((const char *)lastonlinetime);

		LOG_DEBUG("offlinetime:%ld", offlinetime);
        //permit server and client has 5min time difference
		outctrl_day = app->get_public_policy().pub.outctrl_day;
		if (outctrl_day < 0) {
			outctrl_day = 0;
		}

		LOG_INFO("out control day is %d", outctrl_day);
		if (outctrl_day == 0 || ((offlinetime > -300) && (offlinetime < outctrl_day*24*3600))) {
			curstatus = LocalMode_Status_CheckTermMode;
			return LM_Check_CurMode();
		}else {
			return LM_LocalCheckError();
		}
	}else {
		LOG_ERR("current status invalid");
	}
	return -1;
}

int LocalMode::LM_Check_CurMode(){
	int ret = -1;
	if (LocalMode_Status_CheckTermMode != curstatus) {
		LOG_ERR("status error");
		return -1;
	}
	LOG_INFO("current term mode:%d", mode.mode);
	if (PUBLIC_MODE == mode.mode) {
//		l2u_show_publiclogin();
//		curstatus = LocalMode_Status_WaitUILoginReq;
//		return 0;
		if (0 == (ret = LM_Req_AuthInfo("public", "public", &userinfo))){
			curstatus = LocalMode_Status_ReqStartVM;
			if (0 != (ret = LM_Req_VM_StartVM(&userinfo))){
				LOG_ERR("starvm req failed");
			}
		}else {
			LOG_ERR("get authinfo failed");
		}
		//todo need to show in ui???
		return ret;
	}else if ((SPECIAL_MODE == mode.mode) || (MULTIUSER_MODE == mode.mode)){
		curstatus = LocalMode_Status_WaitUILoginReq;
		if (0 == (ret = LM_Req_UI_LoginAuth())){
			LOG_INFO("localmode wait ui input");
		}else {
			LOG_ERR("req ui show login failed");
		}
	}else {
		LOG_ERR("unknow term mode");
	}
	return ret;
}

int LocalMode::LM_Req_AuthInfo(string username, string password, struct UserInfo *user_info) {
	int ret;
    UserInfoMgr usermgr;

    LOG_INFO("get login auth info");
	if ((NULL != username.c_str()) && (NULL != password.c_str())){
		LOG_DEBUG("username:%s password:%s", username.c_str(), password.c_str());
	}
	ret = usermgr.UserLoginAuthentication(mode.mode, username, password, user_info);
	LOG_NOTICE("UserMgr Auth return msg:%s", usermgr.usrmgr_strerror(ret).c_str());
	return ret;
}

int LocalMode::LM_Req_VM_StartVM(struct UserInfo *user_info){
	if (curstatus != LocalMode_Status_ReqStartVM){
		LOG_ERR("invalid startvm req");
		return -1;
	}
	app->_logined_user_info = *user_info;
	app->_logined = true;
	LOG_INFO("start to startvm  username:%s passwd:%s ",user_info->username.c_str(), user_info->password.c_str());
	app->_status_machine.change_status(EVENT_LOCAL_CHECK_SUCCESS);
	return 0;
}

int LocalMode::VM_Ack_LM_StartVM(LocalStartVMStatus ret){
	if (curstatus != LocalMode_Status_ReqStartVM){
		LOG_ERR("invalid startvm ack");
		return -1;
	}
	if (LocalStartVMStatus_success == ret){
		LOG_NOTICE("startvm OK...");
		curstatus = LocalMode_Status_StartVMOK;
		return 0;
	}else {
		LOG_ERR("failed to startvm%d", ret);
		curstatus = LocalMode_Status_Init;
		return -1;
	}
	return -1;
}

int LocalMode::LM_Req_UI_LoginAuth(){
	if (curstatus != LocalMode_Status_WaitUILoginReq){
		LOG_ERR("invalid loginauth req");
		return -1;
	}
	LOG_INFO("prepare Check logininfo");
	LM_LocalMode_ShowLogin(0);
	return 0;
}

int LocalMode::LM_Req_UI_LoginAuth2(int ret){
	if (curstatus != LocalMode_Status_WaitUILoginReq){
		LOG_ERR("invalid loginauth req");
		return -1;
	}
	LOG_INFO("Check logininfo again:%d", ret);
	LM_LocalMode_ShowLogin(ret);
	return ret;
}


int LocalMode::UI_Ack_LM_LoginAuth(string username, string password, int remember_flag) {
	int ret;
    UserInfoMgr usermgr;

    if (curstatus != LocalMode_Status_WaitUILoginReq){
		LOG_ERR("invalid loginauth ack");
		return -1;
	}
	if (0 == (ret = LM_Req_AuthInfo(username, password, &userinfo))){
		curstatus = LocalMode_Status_ReqStartVM;
        usermgr.setUserLogined(true);
        usermgr.saveLoginedUser(username);
        usermgr.saveLastLoginedUser(username, password, remember_flag);

        // start VM.
		if (0 != (ret = LM_Req_VM_StartVM(&userinfo))){
			LOG_ERR("starvm req failed");
		}
		return ret;
	}else {
		LOG_ERR("get authinfo failed or username and password error");
		if (0 != (ret = LM_Req_UI_LoginAuth2(ret))){
			LOG_ERR("req ui show login failed(2)");
		}
		return ret;
	}
	return -1;
}

void LocalMode::LM_LocalMode_ShowLogin(int errorcode){
	LOG_INFO("show login ui errorcode:%d", errorcode);
	if (errorcode){
		//todo trance error code
		switch (-errorcode){
			case USRERR_NOUSER:
				errorcode = 4;	//user not exist
				break;
			case USRERR_PSWERROR:	//passwd error
				errorcode = 1;
				break;
			default:
				errorcode = 5;	//data error
				break;
		}
		l2u_result_user(errorcode);
	}else {
		l2u_show_userlogin();
	}
	l2u_result_user(10);	//hide passwd button
	l2u_result_user(11);	//hide guest login button
}

int LocalMode::LM_LocalMode_UpdateUI(){
	LOG_INFO("curstatus:%d", curstatus);
	switch (curstatus){
		case LocalMode_Status_WaitUILoginReq:
			LOG_ERR("localemode WaitUILoginReq");
			LM_LocalMode_ShowLogin(0);
			return 0;
		case LocalMode_Status_CheckError:
			LOG_INFO("last_err_status:%d", last_err_status);
			switch (last_err_status) {
				case LocalMode_Status_CheckImage:
					LOG_ERR("localemode image no exist");
					l2u_show_tips(UI_TIPS_DEV_NO_IMG_OPT);
					return 0;
				case LocalMode_Status_CheckOffLineTimeOut:
					LOG_ERR("localemode offline timeout");
					l2u_show_tips(UI_TIPS_DEV_FORBIDED_OPT);
					return 0;
				default:
					LOG_ERR("localemode last_err_status:%d todo", last_err_status);
					break;
			}
			break;
		default:
			LOG_ERR("localemode curstatus:%d todo", curstatus);
			break;
	}
	return -1;
}

int LocalMode::LM_LocalMode_Quit(int result) {
	curstatus = LocalMode_Status_Init;
	return -1;
}

int LocalMode::UI_Ack_LM_CheckError(int result) {
	return LM_LocalMode_Quit(0);
}
