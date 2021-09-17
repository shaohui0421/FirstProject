#ifndef _NOTE_H
#define _NOTE_H

#include "common.h"
#include <map>

#define EVENT_TO_STR(EVENT)	(#EVENT)
#define STATE_TO_STR(STATE)	(#STATE)


enum Error
{
    SUCCESS                         =   0,
    ERROR_STATUS_INITING            =   -0x1000,
    ERROR_STATUS_CHECKING_LOCAL     =   -0x2000,
    ERROR_STATUS_WAITING_LOGIN      =   -0x3000,
    ERROR_MOUNT_NFS_FAIL            =   -0x3100,
    ERROR_STATUS_CHECKING_LOGIN     =   -0x4000,
    ERROR_STATUS_PREPARING_IMAGE    =   -0x5000,
    ERROR_STATUS_DOWNLOADING_IMAGE  =   -0x6000,
    ERROR_STATUS_RUNNING_VM         =   -0x7000,
    ERROR_START_VM_FAIL             =   -0x7100,
    ERROR_DRIVER_INSTALL_FAIL       =   -0x7101,
    ERROR_DRIVER_INSTALLING         =   -0x7102,
    ERROR_CHANGING_IP               =   -0x8000,
    ERROR_WAITING_CONFIRM           =   -0x8100,
    ERROR_EVENT					    =	-0x9000,
    //ERROR_EVENT_WLAN_SAVE_IP        =   -0x9100,
    ERROR_INPUT					    =	-0xa000,
    ERROR_PROCESSING			    =	-0xb000,
    ERROR_STATUS_NEW_DEPLOY         =   -0xc000,
    ERROR_DUPLICATED_DEPLOY         =   -0xc100,
    ERROR_ISO_VERSION_ERR			=   -0Xd000,
    //ERROR_IPXE_WLAN_UP              =   -0Xd100,
    ERROR_DRIVER_OSTYPE_NOTADAPT    =   -0Xd200,
};

class ErrorNote:public Singleton<ErrorNote>
{
public:
    ErrorNote()
        : Singleton<ErrorNote>(this)
    {
        error_notes[SUCCESS]                        = "成功";
        error_notes[ERROR_STATUS_INITING]           = "正在初始化中";
        error_notes[ERROR_STATUS_CHECKING_LOCAL]    = "正在校验本地信息";
        error_notes[ERROR_STATUS_WAITING_LOGIN]     = "正在等待用户登录";
        error_notes[ERROR_MOUNT_NFS_FAIL]           = "挂载nfs失败";
        error_notes[ERROR_STATUS_CHECKING_LOGIN]    = "正在登陆中";
        error_notes[ERROR_STATUS_PREPARING_IMAGE]   = "正在准备下载镜像";
        error_notes[ERROR_STATUS_DOWNLOADING_IMAGE] = "正在下载镜像";
        error_notes[ERROR_STATUS_RUNNING_VM]        = "正在运行虚拟机";
        error_notes[ERROR_START_VM_FAIL]            = "启动虚拟机失败";
        error_notes[ERROR_DRIVER_INSTALL_FAIL]      = "安装驱动失败";
        error_notes[ERROR_DRIVER_INSTALLING]        = "正在安装驱动";
        error_notes[ERROR_CHANGING_IP]              = "正在更改网络状态";
        error_notes[ERROR_WAITING_CONFIRM]          = "正在等待用户确认操作";
        error_notes[ERROR_EVENT]                    = "事件类型错误";
        //error_notes[ERROR_EVENT_WLAN_SAVE_IP]       = "无线连接不能设置IP";
        error_notes[ERROR_INPUT]                    = "事件输入错误";
        error_notes[ERROR_PROCESSING]               = "事件处理错误";
        error_notes[ERROR_STATUS_NEW_DEPLOY]        = "正在初次部署中";
        error_notes[ERROR_ISO_VERSION_ERR]          =  "iso版本不支持";
        error_notes[ERROR_DUPLICATED_DEPLOY]        = "重复触发初次部署";
        //error_notes[ERROR_IPXE_WLAN_UP]             = "无线不支持快速刷机";
        error_notes[ERROR_DRIVER_OSTYPE_NOTADAPT]   = "终端无法获取启动类型，驱动安装失败";
    }
public:
    static ErrorNote* get_instance()
    {
        if(instance == NULL)
        {
            instance = new ErrorNote();
        }
        return instance;
    }
public:
    std::map<int, string> error_notes;
};


#endif //_NOTE_H
