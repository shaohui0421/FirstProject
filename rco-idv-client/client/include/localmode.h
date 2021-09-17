/*
 * localmode.h
 *
 *  Created on: Feb 15, 2017
 *      Author: zhf
 */

#ifndef __LOCALMODE_H__
#define __LOCALMODE_H__

#include "callback.h"
#include "common.h"
#include "process_loop.h"
#include "application.h"
#include "application_data.h"

class LocalMode;
class Application;
class LogicEvent;

enum LocalMode_Status {
	LocalMode_Status_Init = 0,
	LocalMode_Status_CheckImage,
	LocalMode_Status_CheckOffLineTimeOut,
	LocalMode_Status_CheckError,
	LocalMode_Status_CheckTermMode,
	LocalMode_Status_WaitUILoginReq,
	LocalMode_Status_CheckUserPasswd,
	LocalMode_Status_ReqStartVM,
	LocalMode_Status_StartVMOK,
};

enum LocalImageStatus{
	LocalImageStatus_exist,
	LocalImageStatus_none,
};

enum LocalStartVMStatus{
	LocalStartVMStatus_success,
	LocalStartVMStatus_failed,
};

class LocalMode {
public:
	LocalMode(Application *_app, struct ModeInfo _mode);
	~LocalMode();
	int ResetStatus();
	int push_event(Event* event);
	int UI_Req_ChangeToLocalMode();
	int VM_Ack_LocalImageStatus(LocalImageStatus ret);
	int UI_Ack_LM_LoginAuth(string username, string password, int remember_flag);
	int LM_LocalMode_UpdateUI();
	int LM_LocalMode_Quit(int result);
	int UI_Ack_LM_CheckError(int result);
	int VM_Ack_LM_StartVM(LocalStartVMStatus ret);
	static int LM_WEB_Update_OnLineTime();
	int callback(int ret, void *obj, void *data);
private:
	Application *app;
	struct ModeInfo mode;
	LocalMode_Status curstatus;
	struct UserInfo userinfo;
	LocalMode_Status last_err_status;
	int LM_Check_CurMode();
	int LM_LocalCheckError();
	int LM_Check_OffLineTimeOut();
	int LM_Req_AuthInfo(string username, string password, struct UserInfo *user_info);
	int LM_Req_VM_StartVM(struct UserInfo *user_info);
	int LM_Req_UI_LoginAuth();
	int LM_Req_UI_LoginAuth2(int ret);
	void LM_LocalMode_ShowLogin(int errorcode);
};

#endif
