#ifndef UI_DIALOG_WIFIPWD_H
#define UI_DIALOG_WIFIPWD_H

typedef enum {
    UI_VERIFY_ENTRY_NONE = 0,
    UI_PASSWORD_ENTRY,
    UI_USERNAME_ENTRY,
    UI_SSID_ENTRY
} ui_verify_entry_enum;

typedef enum {
    UI_DIALOG_WIFIPWD_NONE = 0,
    UI_DIALOG_WIFIPWD_WEP,
    UI_DIALOG_WIFIPWD_PSK,
    UI_DIALOG_WIFIPWD_EAP,
    UI_DIALOG_WIFIPWD_HIDE_NET
} ui_dialog_wifipwd_enum;

#endif
