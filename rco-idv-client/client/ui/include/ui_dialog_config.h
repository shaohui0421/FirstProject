#ifndef UI_DIALOG_CONFIG_H
#define UI_DIALOG_CONFIG_H

#ifdef IDV_CLIENT
#include "../../include/application_c_interfaces.h"
#endif

typedef enum {
    UI_DIALOG_NONE = 0,
    UI_DIALOG_CONFIG_SAVING,
    UI_DIALOG_CONFIG_RESULT_OK,
    UI_DIALOG_CONFIG_RESULT_FAIL
} ui_dialog_config_ctrl_enum;

typedef struct ui_dialog_config_msg_s {
    int tip_type;
} ui_dialog_config_msg_t;

typedef gboolean input_box_cb (GtkWidget *widget, GdkEvent  *event, gpointer   user_data);

typedef struct {
    GtkWidget *input;
    GtkWidget *show;
    GtkWidget *hide;
} input_box_t;

GtkWidget *ui_config_scrnmanage_init(void);

GtkWidget *ui_config_ping_init(void);
GtkWidget *ui_config_wifimanage_init(void);
GtkWidget *ui_config_ip_err_init(void);
void get_sw_status(struct wifi_switch_t *status);
#endif
