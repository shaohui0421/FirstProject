#ifndef _MESSAGE_H
#define _MESSAGE_H

#include "common.h"

#define RCD_BUFFER_LEN                 32

enum RcdHandle
{
    RCD_HANDLE_KEEPALIVE                =   0X0001,

    RCD_HANDLE_SYNC_SERVER_TYPE         =   0x0181,
    RCD_HANDLE_SYNC_DEB_PATH            =   0X0182,

    RCD_HANDLE_SYNC_SOFTWARE_VERSION    =   0X0701,
    RCD_HANDLE_SYNC_PUBLIC_POLICY       =   0X0703, //
    RCD_HANDLE_SYNC_HOSTNAME            =   0X0704,
    RCD_HANDLE_SYNC_MODE                =   0X0705,
    RCD_HANDLE_SYNC_IMAGE               =   0X0706,
    RCD_HANDLE_SYNC_RECOVER_IMAGE       =   0X0707,
    RCD_HANDLE_SYNC_IPXE                =   0X0708,
    RCD_HANDLE_SYNC_RESET_TERMINAL      =   0X0709,
    RCD_HANDLE_SYNC_ALL_USERINFO        =   0X070a,
    RCD_HANDLE_REQ_DEV_POLICY        	=   0X070b,
    RCD_HANDLE_SYNC_DRIVER_INSTALL      =   0X070c,
    RCD_HANDLE_SYNC_RELOAD_IMAGE        =   0X070d,
    RCD_HANDLE_SYNC_DELETE_TEACHERDISK  =   0X070e,
    RCD_HANDLE_SYNC_SERVER_IP           =   0X070f,

    RCD_HANDLE_UPLOAD_BASICINFO         =   0X0711,
    RCD_HANDLE_UPLOAD_LOCAL_NETWORK     =   0X0712,
    RCD_HANDLE_UPLOAD_VM_NETWORK        =   0X0713,
    RCD_HANDLE_UPLOAD_DOWNLOADINFO      =   0X0714,
    RCD_HANDLE_UPLOAD_VMINFO            =   0X0715,
    RCD_HANDLE_UPLOAD_NEW_TERMINAL_COMPLETE =   0X0716,
    RCD_HANDLE_UPLOAD_DRIVER_INSTALL_RESULT =   0X0717,
    
    RCD_HANDLE_LOGIN                    =   0X0721,
    RCD_HANDLE_MODIFY_PASSWORD          =   0X0722,
    RCD_HANDLE_BIND                     =   0X0723,
    RCD_HANDLE_COLLECT_LOG_COMPLETE     =   0X0724,
    RCD_HANDLE_CHANGE_MODE              =   0X0725,
    RCD_HANDLE_CHANGE_HOSTNAME          =   0X0726,
    RCD_HANDLE_NOTIFY_PARTITION         =   0X0729,
    RCD_HANDLE_SYNC_REQUEST_SSID        =   0X072a,
    RCD_HANDLE_SYNC_PRINTER_SWITCH      =   0X0735,
    RCD_HANDLE_SYNC_DESKTOP_REDIR       =   0X0736,
    RCD_HANDLE_SYNC_PORT_MAPPING        =   0X0741,
    RCD_HANDLE_SYNC_REQUEST_USBCONF     =   0X0742,
    RCD_HANDLE_SYNC_UNKNOW_DEV          =   0X0743,
    //RCD_HANDLE_NOTIFY_SET_DEVPOLOCY		= 	0X0730,
    RCD_HANDLE_SYNC_SERVER_TIME         =   0X0750,
    RCD_HANDLE_SYNC_TERMINAL_PASSWD     =   0X0751,
    RCD_HANDLE_SYNC_MAIN_WINDOW         =   0X0760,
    RCD_HANDLE_REQ_DEV_INTERFACE_INFO   =   0X0761,
    

    RCD_HANDLE_SEND_GUESTTOOL_MSG       =   0X07ff,

    //
    RCD_HANDLE_WEB_CHECK_VM_STATUS      =   0X0601,
    RCD_HANDLE_WEB_SHUTDOWN             =   0X0602,
    RCD_HANDLE_WEB_REBOOT               =   0X0603,
    RCD_HANDLE_WEB_RECORVER_IMAGE       =   0X0604,
    RCD_HANDLE_WEB_MODIFY_PASSWORD      =   0X0605,
    RCD_HANDLE_WEB_MODIFY_LOCAL_NETWORK =   0X0606,
    RCD_HANDLE_WEB_MODIFY_VM_NETWORK    =   0X0607,
    RCD_HANDLE_WEB_COLLECT_LOG          =   0x0608,
    RCD_HANDLE_WEB_MODIFY_HOSTNAME      =   0X0609,
    RCD_HANDLE_WEB_RESET_TO_INITIAL     =   0X0610,
    RCD_HANDLE_WEB_RESYNC               =   0X0611,
    RCD_HANDLE_WEB_DO_IPXE              =   0x0612,
    RCD_HANDLE_WEB_RELOAD_IMAGE         =   0X0613,
    RCD_HANDLE_WEB_DELETE_TEACHERDISK   =   0X0614,
    RCD_HANDLE_WEB_DRIVER_INSTALL_ACTION=   0X0615,
    RCD_HANDLE_SYNC_DEV_POLICY          =   0X0616,
    RCD_HANDLE_WEB_DO_PORT_MAPPING      =   0X0618,
    RCD_HANDLE_WEB_NOTIFY_SSID_WHITE    =   0X061a,
    RCD_HANDLE_WEB_NOTIFY_RESET_NETDISK =   0X061b,
    RCD_HANDLE_WEB_SET_MAIN_WINDOW      =   0X0621,
    RCD_HANDLE_WEB_NOTIFY_HTTP_PORT     =   0X0630,
    RCD_HANDLE_RECV_GUESTTOOL_MSG       =   0X06ff,
};


#define mina_read_int(dest, src)\
    memcpy(&dest, src, sizeof(int));\
    dest = ntohl(dest);

#define mina_read_short(dest, src)\
    memcpy(&dest, src, sizeof(short));\
    dest = ntohs(dest);

#define mina_read_64int(dest, src)\
    memcpy(&dest, src, sizeof(long long));\
    dest = ntohll(dest);

#define mina_read_char(dest, src, size)\
    strncpy(dest, (char*)src, size);



void mina_write_int(void* dest, int src);

void mina_write_short(void* dest, short src);

void mina_write_64int(void* dest, long long src);

#define mina_write_char(dest, src, size)\
    strncpy(dest, (char*)src, size);



#define MINA_HEAD_LENGTH (RCD_BUFFER_LEN + sizeof(int))

#endif //_MESSAGE_H
