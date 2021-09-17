#ifndef UI_WIN_AUTH_BTNBOX_H
#define UI_WIN_AUTH_BTNBOX_H

#include <gtk/gtk.h>
#include "ui_basic_widget.h"

typedef enum {
    UI_AUTH_SETTING_IP         = -2, // when setting ip we can not call the func
    UI_AUTH_STATUS_UNNECESSARY = -1,    // -1 ~ 3 terminal report status to client
    UI_AUTH_STATUS_SUCCESS     = 0,
    UI_AUTH_STATUS_AUTHING     = 1,
    UI_AUTH_STATUS_FAILED      = 2,
    UI_AUTH_STATUS_OTHER       = 3,
    UI_AUTH_STATUS_MAX,
}ui_win_auth_btnbox_authstatus;

typedef enum {
    UI_AUTH_ENV_EXIST = UI_AUTH_STATUS_MAX,
    UI_AUTH_ENV_NOEXIST,
    UI_AUTH_ENV_MAX,
} ui_win_auth_env_enum;

typedef enum {
    UI_WIN_AUTH_BTNBOX_ENABLE = UI_AUTH_ENV_MAX,
    UI_WIN_AUTH_BTNBOX_DISABLE,
    UI_WIN_AUTH_BTNBOX_MAX,
} ui_win_auth_btnbox_enum;

typedef enum {
    UI_AUTH_STATUS_REQUEST = UI_WIN_AUTH_BTNBOX_MAX, // client request status from terminal
    UI_WIN_AUTH_USER_OPERATION_MAX,
} UI_WIN_AUTH_USER_OPERATION;

void bubble_normal_show(int auth_status);
void ui_win_authbtn_enable(void);
void ui_win_authbtn_disable(void);

#endif
