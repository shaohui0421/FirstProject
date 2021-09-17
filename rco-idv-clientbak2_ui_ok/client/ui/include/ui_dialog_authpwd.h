#ifndef UI_DIALOG_AUTHPWD_H
#define UI_DIALOG_AUTHPWD_H

#include "ui_main.h"
#include "ui_win_auth_btnbox.h"
#include "ui_dialog_wifipwd.h"


typedef enum {
    AUTH_CHECK_OK = 0,
    AUTH_CHECK_ERR_USER_OR_PASSWD_EMPTY = -1,
    AUTH_CHECK_ERR_USER_OR_PASSWD_BLAKE = -2,
}ui_authcheck_ret_code;

typedef enum {
    UI_NET_AUTHSTATUS_SETIP = -2,
    UI_NET_AUTHSTATUS_UNNECESSARY = -1,
    UI_NET_AUTHSTATUS_SUCCESS = 0,
    UI_NET_AUTHSTATUS_AUTHING = 1,
    UI_NET_AUTHSTATUS_FAIL = 2,
    UI_NET_AUTHSTATUS_OTHER = 3,
    UI_NET_AUTHSTATUS_MAX,
} ui_net_auth_status;

typedef enum {
    UI_TIPS_AUTHSTATUS_USER_ERR = UI_NET_AUTHSTATUS_MAX,
    UI_TIPS_AUTHSTATUS_USERPASSWD_BLAKE,
    UI_TIPS_AUTHSTATUS_TIMEOUT,
    UI_TIPS_AUTHSTATUS_DISCONNECT,
    UI_TIPS_AUTHSTATUS_MAX,
} ui_dialog_authpwd_reminds_enum;


typedef enum {
    DISAUTO_CONNECT,
    AUTO_CONNECT,
}ui_dialog_auth_pwd_automatic_connect_enum;

    
typedef struct ui_auth_msg {
    int     object;
    int     sub_obj;
    void    *args;
} ui_auth_msg_t;

#endif
