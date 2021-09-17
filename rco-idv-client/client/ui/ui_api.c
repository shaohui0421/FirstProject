#include "ui_main.h"
#include <malloc.h>
#include "ui_api.h"
#include "ui_dialog_wifi.h"
#include "ui_dialog_wifipwd.h"
#include "ui_dialog_prop.h"
#include "ui_dialog_authpwd.h"
#include "ui_win_auth_btnbox.h"
#include "ui_win_main.h"

static void ui_send_handle_msg(int object, int sub_obj, void *args)
{
    ui_msg_t *msg;
    
    msg = malloc(sizeof(ui_msg_t));
    msg->object = object;
    msg->sub_obj = sub_obj;
    msg->args = args;
    ui_handle_msg(msg);
}
//显示下载失败对话框
void l2u_show_upgrade_faile()
{
    logi("enter %s \n", __func__);
    ui_send_handle_msg(UI_CTRL_SHOW_WIN,UI_WIN_STATUS_UPGRADE_FAIL, NULL);
}
//显示设置自动关机对话框
void l2u_show_auto_shutdown(void)
{
    ui_send_handle_msg(UI_CTRL_SHOW_AUTO_SHUTDOWN, 0,  NULL);
}

void l2u_show_iso_upgrade_err(void * callback, void * args)
{
	ui_callback_t * func;

	func = calloc(1, sizeof(ui_callback_t));
	func->callback = callback;
	func->args = args;
    ui_send_handle_msg(UI_CTRL_SHOW_ISO_UPGRADE_ERR, 0,  func);
}

// 显示一键部署提示框
void l2u_show_easydeploy_tips(void * callback, void * args)
{
	ui_callback_t * func;

	func = calloc(1, sizeof(ui_callback_t));
	func->callback = callback;
	func->args = args;
    ui_send_handle_msg(UI_CTRL_SHOW_EASYDEPLOY_TIPS, 0,  func);
}

// 显示一键部署错误提示
void l2u_show_easydeploy_err(void * callback, void * args)
{
    ui_callback_t * func;

    func = calloc(1, sizeof(ui_callback_t));
    func->callback = callback;
    func->args = args;
    ui_send_handle_msg(UI_CTRL_SHOW_EASYDEPLOY_ERR, 0, func);
}

// 显示虚机错误提示框
void l2u_show_vmerr_tips(void * callback, void * args, void *data, int result)
{
    ui_callback_t * func;

    func = calloc(1, sizeof(ui_callback_t));
    func->callback = callback;
    func->args = args;
    func->data = data;

    switch(result) {
    case UI_TIPS_VM_START_FAILED:
       ui_send_handle_msg(UI_CTRL_SHOW_VMERR_TIPS, 0,  func);
        break;
    case UI_TIPS_VM_OSTYPE_UNKNOWN:
        // 显示操作系统错误提示框
        ui_send_handle_msg(UI_CTRL_SHOW_OSTYPEERR_TIPS, 0, func);
        break;
    case UI_TIPS_VM_NOT_SUPPORT_CPU:
        ui_send_handle_msg(UI_CTRL_SHOW_CPUNOSPORT_TIPS, 0, func);
        break;
    case UI_TIPS_VM_NOT_SUPPORT_OSTYPE:
        // 显示操作系统不支持提示框
        ui_send_handle_msg(UI_CTRL_SHOW_OSTYPENOSPORT_TIPS, 0, func);
        break;
    case UI_TIPS_VM_VMMODEINT_LOST:
        // 显示虚机启动文件丢失提示框
        ui_send_handle_msg(UI_CTRL_SHOW_VMMODEINILOST_TIPS, 0, func);
        break;
    case UI_TIPS_VM_DRIVER_OSTYPE_NOTADAPT:
        ui_send_handle_msg(UI_CTRL_SHOW_DRIVERNOTADAPT_TIPS, 0, func);
        break;
    case UI_TIPS_VM_CREATE_CDISK_FAILED:
        ui_send_handle_msg(UI_CTRL_SHOW_CREATE_CDISK_FAIL_TIPS, 0, func);
        break;
    case UI_TIPS_VM_START_LAYER_TEMPLETE:
        ui_send_handle_msg(UI_CTRL_SHOW_VM_LAYER_TEMPLETE_TIPS, 0, func);
        break;
    case UI_TIPS_VM_START_INTEL_NO_AUDIO_DEVICE:
        ui_send_handle_msg(UI_CTRL_SHOW_NO_AUDIO_DEVICE_TIPS, 0, func);
        break;
    default:
        break;
    }
}


//显示镜像下载确认提示框
void l2u_show_download_confirm_tips(void * callback, void * args, void * confirm)
{
    ui_callback_t * func;

    func = calloc(1, sizeof(ui_callback_t));
    func->callback = callback;
    func->args = args;
    func->data = confirm;
    ui_send_handle_msg(UI_CTRL_SHOW_DOWNLOAD_CONFIRM_TIPS, 0,  func);
}
//显示镜像下载模式选择
void l2u_show_download_mode_select_tips(void * callback, void * args, void * confirm)
{
    ui_callback_t * func;

    func = calloc(1, sizeof(ui_callback_t));
    func->callback = callback;
    func->args = args;
    func->data = confirm;
    ui_send_handle_msg(UI_CTRL_SHOW_DOWNLOAD_MODE_SELECT_TIPS, 0,  func);
}
//提示5秒后重启 然后merge
void l2u_show_need_merge_tips(void * callback, void * args, void * confirm)
{
    ui_callback_t * func;

    func = calloc(1, sizeof(ui_callback_t));
    func->callback = callback;
    func->args = args;
    func->data = confirm;
    ui_send_handle_msg(UI_CTRL_SHOW_NEED_MERGE_TIPS, 0,  func);
}
void l2u_set_update_image_size(char * size)
{
    strncpy(update_image_size, size, 6); 
}

//显示上次虚机启动错误提示框
void l2u_show_vm_lasterr_tips(void * callback, void * args)
{
    ui_callback_t * func;

    func = calloc(1, sizeof(ui_callback_t));
    func->callback = callback;
    func->args = args;
    ui_send_handle_msg(UI_CTRL_SHOW_VM_LASTERR_TIPS, 0,  func);
}


void l2u_show_img_big_tips(void * callback, void * args)
{
    ui_callback_t * func;

    func = calloc(1, sizeof(ui_callback_t));
    func->callback = callback;
    func->args = args;
    ui_send_handle_msg(UI_CTRL_SHOW_IMG_BIG_TIPS, 0,  func);
}

void l2u_show_bad_driver_tips(void * callback, void * args)
{
    ui_callback_t * func;

    func = calloc(1, sizeof(ui_callback_t));
    func->callback = callback;
    func->args = args;
    ui_send_handle_msg(UI_CTRL_SHOW_BAD_DRIVER_TIPS, 0,  func);
}

void l2u_show_image_abnormal_tips(void * callback, void * args)
{
    ui_callback_t * func;

    func = calloc(1, sizeof(ui_callback_t));
    func->callback = callback;
    func->args = args;
    ui_send_handle_msg(UI_CTRL_SHOW_IMAGE_ABNORMAL_TIPS, 0,  func);
}

void l2u_show_nonsupportxp_tips(void * callback, void * args)
{
    ui_callback_t * func;

    func = calloc(1, sizeof(ui_callback_t));
    func->callback = callback;
    func->args = args;
    ui_send_handle_msg(UI_CTRL_SHOW_NOTSUPPORTXP_TIPS, 0, func);
}

void l2u_show_nonsupportwin10_32_tips(void * callback, void * args)
{
    ui_callback_t * func;

    func = calloc(1, sizeof(ui_callback_t));
    func->callback = callback;
    func->args = args;
    ui_send_handle_msg(UI_CTRL_SHOW_NOTSUPPORTWIN10_32_TIPS, 0, func);
}


//显示设置登陆类型对话框
void l2u_show_settype(const char *name, int size)
{
	ui_string_arg_t * args;

    ui_send_handle_msg(UI_CTRL_SHOW_SETTYPE, 0,  NULL);
    if (name && size != 0) {
    	args = calloc(sizeof(ui_string_arg_t), 1);
    	if (args == NULL) {
    		logi("calloc err\n");
    		return;
    	}
    	if (size >= sizeof(args->str)) {
            free(args);
    		logi("arg len is too long\n");
    		return;
    	}
    	strncpy(args->str, name, sizeof(args->str)-1);
    	ui_send_handle_msg(UI_CTRL_SHOW_SETTYPE_RESULT, UI_WIN_SETTYPE_FIX_USER,  (void *)args);
    }
}
//显示未配置提示信息
void l2u_show_unconfig(void)
{
    ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_UNCONFIG,  NULL);
}
//显示正在连接提示信息
void l2u_show_connecting(void)
{
    ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_CONNECTING, NULL);
}

//显示用户登录提示界面
void l2u_show_userlogin(void)
{
    ui_send_handle_msg(UI_CTRL_SHOW_USER_LOGIN, 0, NULL);
}

void l2u_show_password(void)
{
    ui_send_handle_msg(UI_CTRL_SHOW_PASSWORD, 0, NULL);
}
//显示用户绑定提示界面
void l2u_show_userbind(void)
{
    ui_send_handle_msg(UI_CTRL_SHOW_USER_BINDER, 0, NULL);
}
//显示镜像是否需要更新提示界面
void l2u_show_imageupdate(void)
{
    ui_send_handle_msg(UI_CTRL_SHOW_IMAGE_UPDATE, 0, NULL);
}
//正在更新镜像中
void l2u_show_imageupdating(void)
{
    ui_send_handle_msg(UI_CTRL_SHOW_IMAGE_UPDATING, 0,NULL);
}
//正在准备下载环境中
void l2u_show_image_initing(l2u_download_t* args)
{
    l2u_download_t *bak = NULL;
    if (args) {
        bak = malloc(sizeof(l2u_download_t));
        memcpy(bak, args, sizeof(l2u_download_t));
        if (args->speed) {
            bak->speed = strdup(args->speed);
        }
    }
    ui_send_handle_msg(UI_CTRL_SHOW_IMAGE_INITING, 0, bak);
}
//显示/更新下载提示框（可选的参数：当前状态，下载进度，速率等）
void l2u_show_download(l2u_download_t* args)
{
    l2u_download_t *bak = NULL;
    if (args) {
        bak = malloc(sizeof(l2u_download_t));
        memcpy(bak, args, sizeof(l2u_download_t));
        if (args->speed) {
            bak->speed = strdup(args->speed);
        }
    }
    ui_send_handle_msg(UI_CTRL_SHOW_DOWNLOAD_PROGRESS, 0, bak);
}

//显示/更新拷贝提示框（可选的参数：当前状态，拷贝进度，速率等）
void l2u_show_usb_copy(l2u_download_t* args)
{
    l2u_download_t *bak = NULL;
    if (args) {
        bak = malloc(sizeof(l2u_download_t));
        memcpy(bak, args, sizeof(l2u_download_t));
        if (args->speed) {
            bak->speed = strdup(args->speed);
        }
    }
    ui_send_handle_msg(UI_CTRL_SHOW_USB_COPY_PROGRESS, 0, bak);
}


//显示公共用户登录界面
void l2u_show_publiclogin(void)
{
    ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_PUBLIC_LOGIN, NULL);
}
//显示用户登录界面
void l2u_show_login_tip(void)
{
    ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_LOGIN, NULL);
}
void l2u_show_config(void)
{
    ui_send_handle_msg(UI_CTRL_SHOW_CONFIG, 0, NULL);
}
//与云主机断开连接
void l2u_show_disconnect(void)
{
    ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_DISCONNECT, NULL);
}
//正在升级软件，请稍候
void l2u_show_updateclient(void)
{
    ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_CLIENT_UPDATE, NULL);
}
//返回配置保存结果
void l2u_result_config(int result)
{
    if(ui_tab[UI_TYPE_DIALOG_CONFIG]->status != UI_STATUS_SHOW) {
        logi("l2u_show_net_status: ui status != show\n");
        return;
    }

    switch (result) {
    case 0:
        //SUCCESS
        ui_send_handle_msg(UI_CTRL_SHOW_CONFIG_RESULT, UI_DIALOG_TIPS_CONFIG_OK, NULL);
        break;
    case 1:
        //FAIL
        ui_send_handle_msg(UI_CTRL_SHOW_CONFIG_RESULT, UI_DIALOG_TIPS_CONFIG_ERROR, NULL);
        break;
    case 2:
        //FAIL
        ui_send_handle_msg(UI_CTRL_SHOW_CONFIG_RESULT, UI_DIALOG_TIPS_CONFIG_OK_REBOOT, NULL);
        break;
    case 3:
        //FAIL, wlan save ip
        ui_send_handle_msg(UI_CTRL_SHOW_CONFIG_RESULT, UI_DIALOG_TIPS_CONFIG_WLAN_SAVE_IP, NULL);        
        break;
    case 4:
        ui_send_handle_msg(UI_CTRL_SHOW_CONFIG_RESULT, UI_DIALOG_TIPS_NET_AUTH_FAIL, NULL);
        break;
    case 5:
        ui_send_handle_msg(UI_CTRL_SHOW_CONFIG_RESULT, UI_DIALOG_TIPS_NET_AUTH_TIMEOUT, NULL);
        break;
    }    
}

//返回绑定结果
void l2u_result_binduser(int result)
{
	switch (result) {
    case 0:
        //SUCCESS
        break;
    case 1:
        //USER PWD ERROR
        ui_send_handle_msg(UI_CTRL_SHOW_USER_BINDER_RESULT, UI_WIN_BINDUSR_RET_ERROR, NULL);
        break;
	}
}

//返回设置类型界面结果
void l2u_result_settype(int result)
{
	switch (result) {
	case 1:
		ui_send_handle_msg(UI_CTRL_SHOW_SETTYPE_RESULT, UI_WIN_SETTYPE_USER_IVALID, NULL);
		break;
	case 2:
		ui_send_handle_msg(UI_CTRL_SHOW_SETTYPE_RESULT, UI_WIN_SETTYPE_USER_BINDED, NULL);
		break;
	case 3:
		ui_send_handle_msg(UI_CTRL_SHOW_SETTYPE_RESULT, UI_WIN_SETTYPE_TERM_BINDED, NULL);
		break;
	}
}

void l2u_show_recover_disk(const char * status, int size)
{
	ui_string_arg_t * args;
  	args = calloc(sizeof(ui_string_arg_t), 1);
	if (args == NULL) {
		logi("calloc err\n");
		return;
	}
	if (size >= sizeof(args->str)) {
        free(args);
		logi("arg len is too long\n");
		return;
    }
    strncpy(args->str, status, sizeof(args->str)-1);
    ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_RECOVER_DISK, (void *)args);
}

//返回登录结果
void l2u_result_user(int result)
{
    switch (result) {
    case 0:
        //SUCCESS
        break;
    case 1:
        //USER PWD ERROR
        ui_send_handle_msg(UI_CTRL_SHOW_USER_LOGIN_RESULT, UI_WIN_USER_LOGIN_RET_ERROR, NULL);
        break;
    case 2:
        //NO IMAGE
        ui_send_handle_msg(UI_CTRL_SHOW_USER_LOGIN_RESULT, UI_WIN_USER_LOGIN_RET_NOIMAGE, NULL);
        break;
    case 3:
        //BIND ERR
        ui_send_handle_msg(UI_CTRL_SHOW_USER_LOGIN_RESULT, UI_WIN_USER_LOGIN_RET_BIND_ERR, NULL);
        break;
    case 4:
    	ui_send_handle_msg(UI_CTRL_SHOW_USER_LOGIN_RESULT, UI_WIN_USER_LOGIN_RET_EUSER, NULL);
    	break;
    case 5:
    	ui_send_handle_msg(UI_CTRL_SHOW_USER_LOGIN_RESULT, UI_WIN_USER_LOGIN_RET_EUNKNOW, NULL);
    	break;
    case 6:
    	ui_send_handle_msg(UI_CTRL_SHOW_USER_LOGIN_RESULT, UI_WIN_USER_LOGIN_RET_NOBIND, NULL);
    	break;
    case 7:
    	ui_send_handle_msg(UI_CTRL_SHOW_USER_LOGIN_RESULT, UI_WIN_USER_LOGIN_DEF_PWD_WARN, NULL);
    	break;
    case 10:
        ui_send_handle_msg(UI_CTRL_SHOW_USER_LOGIN_RESULT, UI_WIN_USER_LOGIN_HIDE_MODPWD, NULL);
        break;
    case 11:
        ui_send_handle_msg(UI_CTRL_SHOW_USER_LOGIN_RESULT, UI_WIN_USER_LOGIN_HIDE_GUEST, NULL);
        break;
    case 12:
        ui_send_handle_msg(UI_CTRL_SHOW_USER_LOGIN_RESULT, UI_WIN_USER_LOGIN_RET_USER_BINDED, NULL);
        break;
    case 20:
        ui_send_handle_msg(UI_CTRL_SHOW_USER_LOGIN_RESULT, UI_WIN_AD_LOGIN_RET_DOMAIN_EXCEPTION, NULL);
        break;
    case 21:
        ui_send_handle_msg(UI_CTRL_SHOW_USER_LOGIN_RESULT, UI_WIN_AD_LOGIN_RET_USER_DISABLE, NULL);
        break;
    case 22:
        ui_send_handle_msg(UI_CTRL_SHOW_USER_LOGIN_RESULT, UI_WIN_AD_LOGIN_RET_USER_EXPIRE, NULL);
        break;
    case 23:
        ui_send_handle_msg(UI_CTRL_SHOW_USER_LOGIN_RESULT, UI_WIN_AD_LOGIN_RET_USER_NOT_AUTH_RIGHT_NOW, NULL);
        break;
    case 24:
        ui_send_handle_msg(UI_CTRL_SHOW_USER_LOGIN_RESULT, UI_WIN_LDAP_LOGIN_RET_ERROR, NULL);
        break;
    case 99:
        ui_send_handle_msg(UI_CTRL_SHOW_USER_LOGIN_RESULT, UI_WIN_USER_LOGIN_RET_TIMEOUT, NULL);
        break;
    case 100:
        ui_send_handle_msg(UI_CTRL_SHOW_PASSWORD_RESULT, UI_WIN_PASSWD_RET_NEWPWD_NO_MATCH, NULL);
        break;
    case 101:
        ui_send_handle_msg(UI_CTRL_SHOW_PASSWORD_RESULT, UI_WIN_PASSWD_RET_ERR, NULL);
        break;
    case 102:
        ui_send_handle_msg(UI_CTRL_SHOW_PASSWORD_RESULT, UI_WIN_PASSWD_RET_OK, NULL);
        break;
    case 103:
        ui_send_handle_msg(UI_CTRL_SHOW_PASSWORD_RESULT, UI_WIN_PASSWD_RET_PWD_NO_CHANGE, NULL);
        break;
    case 104:
        ui_send_handle_msg(UI_CTRL_SHOW_PASSWORD_RESULT, UI_WIN_PASSWD_RET_PWD_HAS_SPECIAL_CHAR, NULL);
        break;
    case 105:
        ui_send_handle_msg(UI_CTRL_SHOW_PASSWORD_RESULT, UI_WIN_PASSWD_RET_USER_EMPTY, NULL);
        break;
    case 106:
        ui_send_handle_msg(UI_CTRL_SHOW_PASSWORD_RESULT, UI_WIN_PASSWD_RET_NEWPWD_EMPTY, NULL);
        break;
    case 107:
        ui_send_handle_msg(UI_CTRL_SHOW_PASSWORD_RESULT, UI_WIN_PASSWD_RET_OLDPWD_EMPTY, NULL);
        break;
    case 108:
        ui_send_handle_msg(UI_CTRL_SHOW_PASSWORD_RESULT, UI_WIN_PASSWD_RET_ERR_AD_DOMAIN, NULL);
        break;
    case 109:
        ui_send_handle_msg(UI_CTRL_SHOW_PASSWORD_RESULT, UI_WIN_PASSWD_RET_ERR_LDAP_USER, NULL);
        break;
    case 110:
        ui_send_handle_msg(UI_CTRL_SHOW_PASSWORD_RESULT, UI_WIN_PASSWD_RET_ERR_DEFAULT_PASSWD, NULL);
        break;
    case 200:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_SERVER_MAINTAIN, NULL);
        break;
    }

}
//升级软件完成，系统将在5秒后重启
void l2u_result_updateclient(int result)
{
    switch (result) {
    case 0:
        //SUCCESS
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_CLIENT_UPDATE_SUCCESS, NULL);
        break;
    case 1:
        //FAIL
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_CLIENT_UPDATE_FAIL, NULL);
        break;
    case 2:
        //FAIL
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_CLIENT_UPDATE_FAIL_DEGRADE, NULL);
        break;
    case 3:
        //FAIL
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_CLIENT_UPDATE_FAIL_PID, NULL);
        break;
    case 4:
        //FAIL
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_CLIENT_UPDATE_FAIL_UDISK, NULL);
        break;
    case 5:
        //FAIL
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_CLIENT_UPDATE_FAIL_OS, NULL);
        break;
    case 6:
        //FAIL
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_CLIENT_UPDATE_FAIL_RCC, NULL);
        break;
    case 7:
        //SUCCESS
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_CLIENT_UPDATE_SUCCESS_RCC, NULL);
        break;
    case 8:
        //FAIL
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_CLIENT_UPDATE_FAIL_OS_DEGRADE, NULL);
        break;
    }
}

void l2u_ui_thread_quit(void)
{
    ui_send_handle_msg(UI_CTRL_GTK_THREAD_QUIT, 0, NULL);
}

void l2u_show_tips(int type)
{
    switch(type) {
    case UI_TIPS_AUTUHOR_NOT_FOUND_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_AUTUHOR_NOT_FOUND, NULL);
        break;
    case UI_TIPS_AUTUHOR_OUT_OF_RANGE_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_AUTUHOR_OUT_OF_RANGE, NULL);
        break;
    case UI_TIPS_NOCONF_IMAGE_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_NOCONF_IMAGE, NULL);
        break;
    case UI_TIPS_NOCONF_POLICY_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_NOCONF_POLICY, NULL);
        break;
    case UI_TIPS_RECOVERY_IMAGE_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_RECOVERY_IMAGE, NULL);
        break;
    case UI_TIPS_SERVER_ERROR_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_SERVER_ERROR, NULL);
        break;
    case UI_TIPS_CLIENT_INIT_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_CLIENT_INIT, NULL);
        break;
    case UI_TIPS_DEV_NO_IMG_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_DEV_NO_IMG, NULL);
        break;
    case UI_TIPS_DEV_FORBIDED_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_DEV_FORBIDED, NULL);
        break;
    case UI_TIPS_DEV_MODE_CHANGE_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_DEV_MODE_CHANGE, NULL);
        break;
    case UI_TIPS_DEV_USER_CHANGE_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_DEV_USER_CHANGE, NULL);
        break;
    case UI_TIPS_NET_DISCONNETCT_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_NET_DISCONNETCT, NULL);
        break;
    case UI_TIPS_LICENSE_OVERRUN_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_LICENSE_OVERRUN, NULL);
        break;
    case UI_TIPS_DEV_OVERRUN_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_DEV_OVERRUN, NULL);
        break;
    case UI_TIPS_IMG_BIG_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_IMG_BIG, NULL);
        break;
    case UI_TIPS_DELETE_TEACHERDISK_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_DELETE_TEACHERDISK, NULL);
        break;
    //case UI_TIPS_DEV_LOCKED_OPT:
    //    ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_DEV_LOCKED, NULL);
    //    break;
    case UI_TIPS_IMG_BAD_DRIVER_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_IMG_BAD_DRIVER, NULL);
        break;
    case UI_TIPS_SERVER_NOT_VALID_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_SERVER_NOT_VALID, NULL);
        break;
    case UI_TIPS_SERVER_IS_CLASS_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_SERVER_IS_CLASS, NULL);
        break;
  //  case UI_TIPS_LAYER_MD5_ERR_OPT:
    //    ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_LAYER_MD5_ERR, NULL);
    //    break;
    }
}

void l2u_show_image_err_tips(int type, int errcode)
{
    switch(type) {
    case UI_TIPS_IMG_BAD_DRIVER_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_IMG_BAD_DRIVER, NULL);
        break;
    case UI_TIPS_IMG_USER_ERR_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_IMG_USER_ERR, NULL);
        break;
    case UI_TIPS_IMG_NOT_FOUND_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_IMG_NOT_FOUND, NULL);
        break;
    case UI_TIPS_IMG_ABNORMAL_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_IMG_ABNORMAL, NULL);
        break;
    case UI_TIPS_IMG_VERSION_GT_OUTDATED_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_IMG_GT_VERSION_OUTDATED, NULL);
        break;
     case UI_TIPS_DEV_NONSPUPPORT_OSTYPE32_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_DEV_NONSUPPORT_OSTYPE32, NULL);
        break;
     case UI_TIPS_DEV_NONSPUPPORT_OSTYPEXP_OPT:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_DEV_NONSUPPORT_OSTYPEXP, NULL);
        break;
    }
}

void l2u_destroy_screen(void)
{
    ui_send_handle_msg(UI_CTRL_DESTORY_ALL_SCREEN, 0, NULL);
}

void l2u_show_server_err(int type)
{
    switch(type) {
    case 1:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_SERVER_ERROR, NULL);
        break;
    case 2:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_IMG_STOP, NULL);
        break;
    case 3:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_IMG_ERR, NULL);
        break;
    case 4:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_IMG_LOST, NULL);
        break;
    case 5:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_LOG_ERR, NULL);
        break;
    case 6:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_BINDED, NULL);
        break;
    case 7:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_NO_USER, NULL);
        break;
    }
}

void l2u_ctrl_winbtn(gboolean value)
{
	if (value) {
		ui_send_handle_msg(UI_CTRL_WINBTN_SET, UI_WIN_BTN_ENABLE, NULL);
		//ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_ABLE_WINBTN, NULL);
	} else {
		ui_send_handle_msg(UI_CTRL_WINBTN_SET, UI_WIN_BTN_DISABLE, NULL);
		//ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_DISABLE_WINBTN, NULL);
	}
	return;
}

void l2u_show_start_vm_fail_hdmi_err()
{
    ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_START_VM_FAIL_HDMI_ERR, NULL);
}

void l2u_show_newdeploy_connect(int type)
{
    switch(type) {
    case 0:
        //connect fail
		ui_send_handle_msg(UI_CTRL_SHOW_NEWDEPLOY_CONNECT, UI_DIALOG_NEWDEPLOY_CONNECT_FAIL, NULL);
        break;
    case 1:
        //config server
		ui_send_handle_msg(UI_CTRL_SHOW_NEWDEPLOY_CONNECT, UI_DIALOG_NEWDEPLOY_CONNECT_CONFIG_SERVER, NULL);
        break;
    case 2:
        //noauth        
		ui_send_handle_msg(UI_CTRL_SHOW_NEWDEPLOY_CONNECT, UI_DIALOG_NEWDEPLOY_CONNECT_NOAUTH, NULL);
        break;
    case 3:
        //auth fail        
		ui_send_handle_msg(UI_CTRL_SHOW_NEWDEPLOY_CONNECT, UI_DIALOG_NEWDEPLOY_CONNECT_AUTH_FAIL, NULL);
        break;
    }
}

void l2u_show_dialog_connect_network(int type)
{
    if(ui_tab[UI_TYPE_DIALOG_WIFI]->status != UI_STATUS_SHOW) {
        logi("l2u_show_dialog_connect_network: ui status != show\n");
        return;
    }

    switch(type) {
    case UI_TIPS_CONNECTING:
		ui_send_handle_msg(UI_CTRL_SHOW_DIALOG_TIPS, UI_DIALOG_TIPS_CONNECTING, NULL);
        break;
    case UI_TIPS_CONNECT_SUCCESS:
		ui_send_handle_msg(UI_CTRL_SHOW_DIALOG_TIPS, UI_DIALOG_TIPS_CONNECT_SUCCESS, NULL);
        break;
    case UI_TIPS_CONNECT_FAIL:
		ui_send_handle_msg(UI_CTRL_SHOW_DIALOG_TIPS, UI_DIALOG_TIPS_CONNECT_FAIL, NULL);
        break;
    case UI_TIPS_AUTHENTICE_FAIL:
        ui_send_handle_msg(UI_CTRL_SHOW_DIALOG_TIPS, UI_DIALOG_TIPS_AUTHENTICE_FAIL, NULL);
        break;
    case UI_TIPS_INVALID_TYPE:
        ui_send_handle_msg(UI_CTRL_SHOW_DIALOG_TIPS, UI_DIALOG_TIPS_OTHRE_AUTHENTICE_TYPE, NULL);
        break;
    case UI_TIPS_NOT_SUPPORT_WEB_HOTSPOT:
        ui_send_handle_msg(UI_CTRL_SHOW_DIALOG_TIPS, UI_DIALOG_TIPS_NOT_SUPPORT_WEB_HOTSPOT, NULL);
        break;
    case UI_TIPS_CONNECT_NOT_WHITESSID:
         ui_send_handle_msg(UI_CTRL_SHOW_DIALOG_TIPS, UI_DIALOG_TIPS_CONNECT_NOT_WHITESSID, NULL);
        break;
    }
}

void l2u_hide_dialog_connect_network(void)
{
    ui_send_handle_msg(UI_CTRL_HIDE_DIALOG_TIPS, 0, NULL);
}

void l2u_show_dialog_config_ip()
{
    ui_send_handle_msg(UI_CTRL_SHOW_DIALOG_TIPS, UI_DIALOG_TIPS_CONFIG_IP_OK, NULL);
}

void l2u_show_dialog_copy_base(void* callback1, void* args1, void* callback2, void* args2)
{
    ui_callbacks_t * func;
    func = (ui_callbacks_t*)calloc(1, sizeof(ui_callbacks_t));
    if (func == NULL) {
        logi("calloc err\n");
        return;
    }
    func->cb_num = 2;
    func->cb = (ui_callback_t*)calloc(func->cb_num, sizeof(ui_callback_t));
    if (func->cb == NULL) {
        logi("calloc err\n");
        free(func);
        return;
    }
    func->cb[0].callback = callback1;
    func->cb[0].args = args1;
    func->cb[1].callback = callback2;
    func->cb[1].args = args2;
    ui_send_handle_msg(UI_CTRL_SHOW_DIALOG_TIPS, UI_DIALOG_TIPS_CP_BASE, func);
}

void l2u_show_dialog_copy_base_insert_u_disk(void* callback, void* args)
{
    ui_callbacks_t * func;
    func = (ui_callbacks_t*)calloc(1, sizeof(ui_callbacks_t));
    if (func == NULL) {
        logi("calloc err\n");
        return;
    }
    func->cb_num = 1;
    func->cb = (ui_callback_t*)calloc(func->cb_num, sizeof(ui_callback_t));
    if (func->cb == NULL) {
        logi("calloc err\n");
        free(func);
        return;
    }
    func->cb[0].callback = callback;
    func->cb[0].args = args;
    ui_send_handle_msg(UI_CTRL_SHOW_DIALOG_TIPS, UI_DIALOG_TIPS_CP_BASE_INSERT_U_DISK, func);
}

void l2u_show_dialog_copy_base_fail(void* callback, void* args)
{
    ui_callbacks_t * func;
    func = calloc(1, sizeof(ui_callback_t));
    if (func == NULL) {
        logi("calloc err\n");
        return;
    }
    func->cb_num = 1;
    func->cb = (ui_callback_t*)calloc(func->cb_num, sizeof(ui_callback_t));
    if (func->cb == NULL) {
        logi("calloc err\n");
        free(func);
        return;
    }
    func->cb[0].callback = callback;
    func->cb[0].args = args;
    ui_send_handle_msg(UI_CTRL_SHOW_DIALOG_TIPS, UI_DIALOG_TIPS_CP_BASE_FAIL, func);
}

void l2u_show_dialog_copy_base_fail_no_space(void* callback, void* args)
{
    ui_callbacks_t * func;
    func = calloc(1, sizeof(ui_callback_t));
    if (func == NULL) {
        logi("calloc err\n");
        return;
    }
    func->cb_num = 1;
    func->cb = (ui_callback_t*)calloc(func->cb_num, sizeof(ui_callback_t));
    if (func->cb == NULL) {
        logi("calloc err\n");
        free(func);
        return;
    }
    func->cb[0].callback = callback;
    func->cb[0].args = args;
    ui_send_handle_msg(UI_CTRL_SHOW_DIALOG_TIPS, UI_DIALOG_TIPS_CP_BASE_FAIL_NO_SPACE, func);
}

void l2u_show_dialog_copy_base_fail_no_dev(void* callback, void* args)
{
    ui_callbacks_t * func;
    func = calloc(1, sizeof(ui_callback_t));
    if (func == NULL) {
        logi("calloc err\n");
        return;
    }
    func->cb_num = 1;
    func->cb = (ui_callback_t*)calloc(func->cb_num, sizeof(ui_callback_t));
    if (func->cb == NULL) {
        logi("calloc err\n");
        free(func);
        return;
    }
    func->cb[0].callback = callback;
    func->cb[0].args = args;
    ui_send_handle_msg(UI_CTRL_SHOW_DIALOG_TIPS, UI_DIALOG_TIPS_CP_BASE_FAIL_NO_DEV, func);
}

void l2u_show_newdeploy_finish()
{
    ui_send_handle_msg(UI_CTRL_SHOW_DIALOG, UI_DIALOG_STATUS_NEWDEPLOY_FINISH, NULL);
}

void l2u_show_wifi_btnbox(int result , int type, int intensity)
{
    ui_wifi_btn_t *args = NULL;

    switch (result) {
    case 1:
        ui_send_handle_msg(UI_CTRL_SHOW_WIFI_BTNBOX, UI_WIN_WIFI_BTNBOX_ENABLE, NULL);
        break;
    case 2:
        ui_send_handle_msg(UI_CTRL_SHOW_WIFI_BTNBOX, UI_WIN_WIFI_BTNBOX_DISABLE, NULL);
        break;
    case 3:
        args = (ui_wifi_btn_t *)malloc(sizeof(ui_wifi_btn_t));
        if(args == NULL) {
            logi("l2u_show_wifi_btnbox: malloc err\n");
            return;
        }

        args->type = type;
        args->intensity = intensity;
        ui_send_handle_msg(UI_CTRL_SHOW_WIFI_BTNBOX, UI_WIN_WIFI_BTNBOX_REFLASH, args);
        break;
    }
}

void l2u_show_wifi_status(int result, ui_wifiinfo *info, ui_scanresult *list, int length)
{
    if(ui_tab[UI_TYPE_DIALOG_WIFI]->status != UI_STATUS_SHOW) {
        logi("l2u_show_wifi_status: ui status != show\n");
        return;
    }

    ui_wifi_dialog_t *args = NULL;
    args = (ui_wifi_dialog_t *)malloc(sizeof(ui_wifi_dialog_t));
    if(args == NULL) {
        logi("l2u_show_wifi_status: malloc err\n");
        return;
    }

    args->info = info;
    args->list = list;
    args->length = length;

    switch (result) {
    case 1:
        ui_send_handle_msg(UI_CTRL_SHOW_WIFI_DIALOG, UI_DIALOG_WIFI_ENABLE, args);
        break;
    case 2:
        ui_send_handle_msg(UI_CTRL_SHOW_WIFI_DIALOG, UI_DIALOG_WIFI_DISABLE, args);
        break;
    case 3:
        ui_send_handle_msg(UI_CTRL_SHOW_WIFI_DIALOG, UI_DIALOG_WIFI_REFRESH, args);
        break;
    }
}

void l2u_show_net_status(int result, int net_status)
{
    int *args = NULL;

    if(ui_tab[UI_TYPE_DIALOG_WIFI]->status != UI_STATUS_SHOW) {
        logi("l2u_show_net_status: ui status != show\n");
        return;
    }

    args = (int *)malloc(sizeof(int));
    if(args == NULL) {
        logi("l2u_show_net_status: malloc error\n");
        return;
    }

    *args = net_status;

    ui_send_handle_msg(UI_CTRL_SHOW_WIFI_DIALOG, UI_DIALOG_NET_CHANGE, args);
}

void l2u_show_wifi_pwd_win(int result, char* ssid)
{
    if(ui_tab[UI_TYPE_DIALOG_WIFI]->status != UI_STATUS_SHOW) {
        logi("l2u_show_wifi_pwd_win: ui status != show\n");
        return;
    }
	ui_string_arg_t * args;
  	args = calloc(sizeof(ui_string_arg_t), 1);
	if (args == NULL) {
		logi("calloc err\n");
		return;
	}
    if (ssid != NULL) {
        strncpy(args->str, ssid, sizeof(args->str)-1);
    }

    switch (result) {
    case 1:
        ui_send_handle_msg(UI_CTRL_SHOW_WIFI_PWD, UI_DIALOG_WIFIPWD_WEP, (void *)args);
        break;
    case 2:
        ui_send_handle_msg(UI_CTRL_SHOW_WIFI_PWD, UI_DIALOG_WIFIPWD_PSK, (void *)args);
        break;
    case 3:
        ui_send_handle_msg(UI_CTRL_SHOW_WIFI_PWD, UI_DIALOG_WIFIPWD_EAP, (void *)args);
        break;
    case 4:
        ui_send_handle_msg(UI_CTRL_SHOW_WIFI_PWD, UI_DIALOG_WIFIPWD_HIDE_NET, (void *)args);
        break;
    }
}

void l2u_hide_wifi_pwd_win(void)
{
    if((ui_tab[UI_TYPE_DIALOG_WIFI]->status != UI_STATUS_SHOW) || (ui_tab[UI_TYPE_DIALOG_WIFIPWD]->status != UI_STATUS_SHOW)) {
        logi("l2u_hide_wifi_pwd_win: ui status != show\n");
        return;
    }

    ui_send_handle_msg(UI_CTRL_HIDE_WIFI_PWD, 0, NULL);
}

void l2u_show_wifi_auth(int type)
{
    switch (type) {
    case 1:        
        ui_send_handle_msg(UI_CTRL_SHOW_DIALOG_TIPS, UI_DIALOG_TIPS_AUTH_USER_EMPTY, NULL);
        break;
    case 2:
        ui_send_handle_msg(UI_CTRL_SHOW_DIALOG_TIPS, UI_DIALOG_TIPS_AUTH_PWD_EMPTY, NULL);
        break;
    case 3:
        ui_send_handle_msg(UI_CTRL_SHOW_DIALOG_TIPS, UI_DIALOG_TIPS_AUTH_SSID_EMPTY, NULL);
        break;
    }
}

void l2u_show_dev_lock(int cur_mode, int lock_type)
{
	ui_int_arg_t * args;
  	args = (ui_int_arg_t *)malloc(sizeof(ui_int_arg_t));
	if (args == NULL) {
		logi("malloc err\n");
		return;
	}
	args->val = lock_type;

    ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_DEV_LOCKED, args);
}

void l2u_show_layer_downgrade(int lock_type)
{
    ui_int_arg_t * args;
    args = (ui_int_arg_t *)malloc(sizeof(ui_int_arg_t));
    if (args == NULL) {
        logi("malloc err\n");
        return;
    }

    args->val = lock_type;
    ui_send_handle_msg(UI_CTRL_SHOW_WIN, UI_WIN_STATUS_LAYER_DOWNGRADE, args);

}

void l2u_show_dialog_save_dpi(void *callback1, void* args1, void* callback2, void* args2)
{
    ui_callbacks_t * func;

	if (!args1) {
		return ;
	}
	func = (ui_callbacks_t*)calloc(1, sizeof(ui_callbacks_t));
    if (func == NULL) {
        logi("calloc err\n");
        return;
    }
	func->cb_num = 2;
    func->cb = (ui_callback_t*)calloc(func->cb_num, sizeof(ui_callback_t));
    if (func->cb == NULL) {
        logi("calloc err\n");
        free(func);
        return;
    }
    func->cb[0].callback = callback1;
    func->cb[0].args = args1;
    func->cb[1].callback = callback2;
    func->cb[1].args = args2;

	ui_send_handle_msg(UI_CTRL_SHOW_DIALOG_TIPS,UI_DIALOG_TIPS_SAVE_DPI,func);
}

void l2u_show_dialog_layer_manage(void *callback1, void* args1, void* callback2, void* args2)
{
    ui_callbacks_t * func;

	func = (ui_callbacks_t*)calloc(1, sizeof(ui_callbacks_t));
    if (func == NULL) {
        logi("calloc err\n");
        return;
    }
	func->cb_num = 2;
    func->cb = (ui_callback_t*)calloc(func->cb_num, sizeof(ui_callback_t));
    if (func->cb == NULL) {
        logi("calloc err\n");
        free(func);
        return;
    }
    func->cb[0].callback = callback1;
    func->cb[0].args = args1;
    func->cb[1].callback = callback2;
    func->cb[1].args = args2;

	ui_send_handle_msg(UI_CTRL_SHOW_DIALOG_TIPS,UI_DIALOG_TIPS_LAYER_MANAGE,func);
}

void l2u_redisplay_adapt_control(const int width, const int height)
{
    ui_display_arg_t * args;

    args = (ui_display_arg_t*)calloc(1, sizeof(ui_display_arg_t));
    if (args == NULL) {
        logi("calloc err\n");
        return;
    }
    args->width = width;
    args->height = height;

    ui_send_handle_msg(UI_CTRL_REDISPLAY_SCREEN, 0, args);
}

void l2u_show_net_auth_disconnect(int type)
{
    logi("enter %s type %d\n", __func__, type);
    switch (type) {
    case UI_TIPS_NET_AUTH_DISCONNECT:
    {
        if (ui_tab[UI_TYPE_DIALOG_AUTHPWD]->status == UI_STATUS_SHOW) {
            ui_send_handle_msg(UI_CTRL_DESTORY_AUTHPWD, 0, NULL);
        }

        if (ui_tab[UI_TYPE_WIN_AUTH_BTNBOX]->status == UI_STATUS_SHOW) {
            ui_send_handle_msg(UI_CTRL_HIDE_AUTH_BTNBOX, 0, NULL);
        }
        break;
    }
    }
}

void l2u_show_net_auth_winbtn(int type)
{
    logi("enter %s type %d\n", __func__, type);

    switch (type) {
    case UI_AUTHBTN_DISABLED:
        ui_send_handle_msg(UI_CTRL_SHOW_AUTH_BTNBOX, UI_WIN_AUTH_BTNBOX_DISABLE, NULL);
        break;
    case UI_AUTHBTN_ENABLED:
        ui_send_handle_msg(UI_CTRL_SHOW_AUTH_BTNBOX, UI_WIN_AUTH_BTNBOX_ENABLE, NULL);
        break;
    case UI_TIPS_NET_AUTH_EXIST:
        ui_send_handle_msg(UI_CTRL_SHOW_AUTH_BTNBOX, UI_AUTH_ENV_EXIST, NULL);
        break;
    case UI_TIPS_NET_AUTH_DISEXIST:
        ui_send_handle_msg(UI_CTRL_SHOW_AUTH_BTNBOX, UI_AUTH_ENV_NOEXIST, NULL);
        break;
    case UI_TIPS_NET_AUTHING:
        ui_send_handle_msg(UI_CTRL_SHOW_AUTH_BTNBOX, UI_AUTH_STATUS_AUTHING, NULL);
        break;
    case UI_TIPS_NET_AUTH_SUCCESS:
        ui_send_handle_msg(UI_CTRL_SHOW_AUTH_BTNBOX, UI_AUTH_STATUS_SUCCESS, NULL);
        break;
    case UI_TIPS_NET_AUTH_FAIL:
        ui_send_handle_msg(UI_CTRL_SHOW_AUTH_BTNBOX, UI_AUTH_STATUS_FAILED, NULL);
        break;
    default:
        ui_send_handle_msg(UI_CTRL_SHOW_AUTH_BTNBOX, UI_AUTH_STATUS_FAILED, NULL);
        break;
    }
}

void l2u_show_net_auth(int type)
{
    logi("l2u_show_net_auth type is %d\n", type);

    if (ui_tab[UI_TYPE_DIALOG_AUTHPWD]->status != UI_STATUS_SHOW) {
        return;
    }

    switch (type) {
    // get auth sattus from terminal 
    case UI_TIPS_NET_AUTH_SUCCESS:
        ui_send_handle_msg(UI_CTRL_SHOW_AUTH_PWD, UI_NET_AUTHSTATUS_SUCCESS, NULL);
        break;
    case UI_TIPS_NET_AUTH_FAIL:
        ui_send_handle_msg(UI_CTRL_SHOW_AUTH_PWD, UI_NET_AUTHSTATUS_FAIL, NULL);
        break;
    // user opearate to connect
    case UI_TIPS_NET_AUTHING:
        ui_send_handle_msg(UI_CTRL_SHOW_AUTH_PWD, UI_NET_AUTHSTATUS_AUTHING, NULL);
        break;
    // defined by user rules
    case UI_TIPS_NET_AUTH_FAIL_USER_ERR:
        ui_send_handle_msg(UI_CTRL_SHOW_AUTH_PWD, UI_TIPS_AUTHSTATUS_USER_ERR, NULL);
        break;
    case UI_TIPS_NET_AUTH_TIMEOUT:
        ui_send_handle_msg(UI_CTRL_SHOW_AUTH_PWD, UI_TIPS_AUTHSTATUS_TIMEOUT, NULL);
        break;
    case UI_TIPS_NET_AUTH_DISCONNECT_SUCCESS:
        ui_send_handle_msg(UI_CTRL_SHOW_AUTH_PWD, UI_TIPS_AUTHSTATUS_DISCONNECT, NULL);
        break;
    }
}

void l2u_show_main_window(int type)
{
    logi("l2u_show_main_window type is %d\n", type);

    //if (!gtk_main_level())
    //    return;

    switch (type) {
    case UI_CALL_MAIN_LOGO_SET:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN_MAIN, UI_WIN_MAIN_SET_LOGO, NULL);
        break;
    case UI_CALL_MAIN_LOGO_RESET:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN_MAIN, UI_WIN_MAIN_RESET_LOGO, NULL);
        break;
    case UI_CALL_MAIN_BG_SET:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN_MAIN, UI_WIN_MAIN_SET_BG, NULL);
        break;
    case UI_CALL_MAIN_BG_RESET:
        ui_send_handle_msg(UI_CTRL_SHOW_WIN_MAIN, UI_WIN_MAIN_RESET_BG, NULL);
        break;
    }
}

