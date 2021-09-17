#ifndef _UI_WIN_WIFI_BTNBOX_H_
#define _UI_WIN_WIFI_BTNBOX_H_

typedef enum {
    UI_WIN_WIFI_BTNBOX_NONE = 0,
    UI_WIN_WIFI_BTNBOX_ENABLE,
    UI_WIN_WIFI_BTNBOX_DISABLE,
    UI_WIN_WIFI_BTNBOX_REFLASH
} ui_win_wifi_btnbox_enum;

typedef enum {
    UI_BTNBOX_NONE = 0,
    UI_BTNBOX_WIRE_CONNECT,
    UI_BTNBOX_WIRE_DISCONNECT,
    UI_BTNBOX_WIRELESS_CONNECT,
    UI_BTNBOX_WIRELESS_DISCONNECT
} ui_wifi_btnbox_netstat_enum;

void ui_win_wifibtn_disable(void);
void ui_win_wifibtn_enable(void);

#endif
