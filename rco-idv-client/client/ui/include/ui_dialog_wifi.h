#ifndef UI_DIALOG_WIFI_H
#define UI_DIALOG_WIFI_H
#include "ui_basic_widget.h"

typedef enum {
    UI_WIFI_DISABLE_MODE = 0,
    UI_WIFI_ENABLE_MODE,
    UI_NONE_WIFI_CLIENT
} ui_wifi_switch_enum;

typedef enum {
    UI_DIALOG_NONE = 0,
    UI_DIALOG_ETH_CONNECT,
    UI_DIALOG_WIFI_CONNECT,
    UI_DIALOG_NET_DOWN
} ui_dialog_net_status_enum;

typedef enum {
    UI_DIALOG_WIFI_NONE = 0,
    UI_DIALOG_WIFI_ENABLE,
    UI_DIALOG_WIFI_DISABLE,
    UI_DIALOG_WIFI_REFRESH,
    UI_DIALOG_NET_CHANGE
} ui_dialog_wifi_enum;

#endif
