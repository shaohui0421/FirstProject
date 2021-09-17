#ifndef UI_MAIN_H
#define UI_MAIN_H
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "ui_basic_widget.h"

#include "ui_extern.h"

#include "ui_win_status.h"
#include "ui_dialog_download.h"
#include "ui_win_user_login.h"
#include "ui_win_bindusr.h"
#include "ui_dialog_tips.h"
#include "ui_win_password.h"
#include "ui_win_btn.h"
#include "liblog.h"
#include "ui_api.h"
#include "ui_win_settype.h"
#include "ui_dialog_interactive.h"
#include "ui_win_wifi_btnbox.h"
#include "ui_win_auth_btnbox.h"
#include "ui_dialog_newdeploy_connect.h"
#include "ui_dialog_status.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define UI_STR_ARGS_SIZE	     (128)

typedef enum {
    UI_CTRL_SHOW_WIN = 0,
    UI_CTRL_SHOW_USER_LOGIN,
    UI_CTRL_SHOW_PASSWORD,
    UI_CTRL_SHOW_USER_BINDER,
    UI_CTRL_SHOW_IMAGE_UPDATE,
    UI_CTRL_SHOW_IMAGE_UPDATING,
    UI_CTRL_SHOW_IMAGE_INITING,
    UI_CTRL_SHOW_DOWNLOAD_PROGRESS,
    UI_CTRL_SHOW_USB_COPY_PROGRESS,
    UI_CTRL_SHOW_CONFIG,
    UI_CTRL_SHOW_CONFIG_RESULT,
    UI_CTRL_SHOW_USER_LOGIN_RESULT,
    UI_CTRL_SHOW_PASSWORD_RESULT,
    UI_CTRL_SHOW_SETTYPE,
    UI_CTRL_GTK_THREAD_QUIT,
    UI_CTRL_SHOW_USER_BINDER_RESULT,
    UI_CTRL_WINBTN_SET,
    UI_CTRL_SHOW_SETTYPE_RESULT,
    UI_CTRL_SHOW_AUTO_SHUTDOWN,
    UI_CTRL_SHOW_ISO_UPGRADE_ERR,
    UI_CTRL_SHOW_EASYDEPLOY_TIPS,
    UI_CTRL_SHOW_VMERR_TIPS,
    UI_CTRL_SHOW_OSTYPEERR_TIPS,
    UI_CTRL_SHOW_OSTYPENOSPORT_TIPS,
    UI_CTRL_SHOW_CPUNOSPORT_TIPS,
    UI_CTRL_SHOW_VMMODEINILOST_TIPS,
    UI_CTRL_SHOW_DRIVERNOTADAPT_TIPS,
    UI_CTRL_SHOW_VM_LASTERR_TIPS,
    UI_CTRL_SHOW_CREATE_CDISK_FAIL_TIPS,
    UI_CTRL_SHOW_VM_LAYER_TEMPLETE_TIPS,
    UI_CTRL_SHOW_NO_AUDIO_DEVICE_TIPS,
    UI_CTRL_SHOW_DOWNLOAD_CONFIRM_TIPS,
    UI_CTRL_SHOW_IMG_BIG_TIPS,
    UI_CTRL_SHOW_BAD_DRIVER_TIPS,
    UI_CTRL_SHOW_NOTSUPPORTXP_TIPS,
    UI_CTRL_SHOW_NOTSUPPORTWIN10_32_TIPS,
    UI_CTRL_SHOW_IMAGE_ABNORMAL_TIPS,
    UI_CTRL_SHOW_WIFI_BTNBOX,
    UI_CTRL_SHOW_WIFI_DIALOG,
    UI_CTRL_SHOW_WIFI_PROP,
    UI_CTRL_SHOW_WIFI_PWD,
    UI_CTRL_HIDE_WIFI_PWD,
    UI_CTRL_DESTORY_ALL_SCREEN,
    UI_CTRL_SHOW_NEWDEPLOY_CONNECT,
    UI_CTRL_SHOW_DIALOG_TIPS,
    UI_CTRL_HIDE_DIALOG_TIPS,
    UI_CTRL_SHOW_DIALOG,
    UI_CTRL_REDISPLAY_SCREEN,
    UI_CTRL_SHOW_EASYDEPLOY_ERR,
    UI_CTRL_SHOW_AUTH_BTNBOX,
    UI_CTRL_HIDE_AUTH_BTNBOX,
    UI_CTRL_SHOW_AUTH_PWD,
    UI_CTRL_DESTORY_AUTHPWD,
    UI_CTRL_SHOW_WIN_MAIN,
    UI_CTRL_SHOW_DOWNLOAD_MODE_SELECT_TIPS,
    UI_CTRL_SHOW_NEED_MERGE_TIPS,
} ui_ctrl_show_enum;


typedef enum {
    UI_SHOW_MODE_BASIC = 0
    
} ui_show_mode_t;

typedef enum {
    UI_TOPLEVEL_TYPE_WIN,
    UI_TOPLEVEL_TYPE_DIALOG
} ui_subtype_t;


typedef enum {
    UI_MSG_OPS_NONE = 0,
    UI_MSG_OPS_SHOW,
    UI_MSG_OPS_DESTROY,
    UI_MSG_OPS_HIDE,
    UI_MSG_OPS_CTRL
} ui_msg_ops_type;

typedef enum {
    UI_TYPE_WIN_MAIN = 0,   // XXX: WIN_MAIN must be first place
    UI_TYPE_WIN_BUTTON_BOX,
    UI_TYPE_WIN_USER_LOGIN,
    UI_TYPE_WIN_PASSWORD,
    UI_TYPE_WIN_STATUS,
    UI_TYPE_WIN_NETWORK_STATUS,
    UI_TYPE_WIN_BINDUSR,
    UI_TYPE_WIN_WIFI_BTNBOX,
    UI_TYPE_WIN_AUTH_BTNBOX,
    UI_TYPE_DIALOG_CONFIG,
    UI_TYPE_DIALOG_ABOUT,
    UI_TYPE_DIALOG_SHUTDOWN,
    UI_TYPE_DIALOG_SETTYPE,
    UI_TYPE_DIALOG_DOWNLOAD,
    UI_TYPE_DIALOG_ADMINPWD,
    UI_TYPE_DIALOG_ADMINPWD_UPGRADE,
    UI_TYPE_DIALOG_TIPS,
    UI_TYPE_DIALOG_INTERACTIVE,
    UI_TYPE_DIALOG_WIFI,
    UI_TYPE_DIALOG_PROP,
    UI_TYPE_DIALOG_WIFIPWD,
    UI_TYPE_DIALOG_AUTHPWD,
    UI_TYPE_DIALOG_NEWDEPLOY_CONNECT,
    UI_TYPE_DIALOG_STATUS,
    UI_TYPE_END
} ui_widget_type;

typedef enum {
    UI_STATUS_NONE = 0,
    UI_STATUS_SHOW,
    UI_STATUS_HIDE,
    UI_STATUS_DESTROY
} ui_widget_status;

typedef enum {
    UI_NOT_NEED_INIT = 0,
    UI_NEED_INIT =1,
} ui_init_status;

struct ui_comp_s {
    int         type;
    int         subtype;
    int         is_init;
    GtkWidget **widget;

    int  (*init) (void);
    void (*show) (void);
    void (*hide) (void);
    void (*destroy) (void);
    int  (*ctrl) (void *);
    void (*adapt) (void);
    // can set
    int         status;
    double      height;
    guint       connect_id;
};

typedef struct ui_main {
    int         r_x;
    int         r_y;
    GtkWidget   *window;
    GtkWidget   *win_fixed;
    int         config_disable;
    guint        pub_login_timer;
    guint        offlinelogin_timer;
    int 		timer_hold;
} ui_main_t;

typedef struct ui_msg_s {
    int     object;
    int     sub_obj;
    void    *args;
} ui_msg_t;


typedef struct ui_callback_s {
	void	* callback;
    void    * args;
    void 	* data;
} ui_callback_t;

typedef struct ui_callbacks_s {
    ui_callback_t * cb;
    int             cb_num;
} ui_callbacks_t;

typedef struct ui_int_arg_s {
	int 	val;
} ui_int_arg_t;

typedef struct ui_string_arg_s {
	char	str[UI_STR_ARGS_SIZE];
} ui_string_arg_t;

typedef struct ui_wifi_btn_s {
    int type;
    int intensity;
} ui_wifi_btn_t;

typedef struct ui_wifi_dialog_s {
    ui_wifiinfo *info;
    ui_scanresult *list;
    int length;
} ui_wifi_dialog_t;

typedef struct ui_display_arg_s {
    int width;
    int height;
} ui_display_arg_t;

typedef struct ui_auth_btn_s {
    int type;
    int intensity;
}ui_auth_btn_t;
extern struct ui_comp_s *ui_tab[];
extern ui_main_t g_win_manager;
extern char update_image_size[];
extern struct ui_comp_s ui_dialog_adminpwd;
extern struct ui_comp_s ui_dialog_adminpwd_upgrade;
extern struct ui_comp_s ui_win_bindusr;
extern struct ui_comp_s ui_dialog_config;
extern struct ui_comp_s ui_win_button_box;
extern struct ui_comp_s ui_win_main;
extern struct ui_comp_s ui_win_network_status;
extern struct ui_comp_s ui_win_status;
extern struct ui_comp_s ui_win_user_login;
extern struct ui_comp_s ui_win_password;
extern struct ui_comp_s ui_win_wifi_btnbox;
extern struct ui_comp_s ui_win_auth_btnbox;
extern struct ui_comp_s ui_dialog_download;
extern struct ui_comp_s ui_dialog_interactive;
extern struct ui_comp_s ui_dialog_tips;
extern struct ui_comp_s ui_dialog_about;
extern struct ui_comp_s ui_dialog_shutdown;
extern struct ui_comp_s ui_dialog_settype;
extern struct ui_comp_s ui_dialog_newdeploy_connect;
extern struct ui_comp_s ui_dialog_status;
extern struct ui_comp_s ui_dialog_wifi;
extern struct ui_comp_s ui_dialog_prop;
extern struct ui_comp_s ui_dialog_wifipwd;
extern struct ui_comp_s ui_dialog_authpwd;

void ui_handle_msg(void *);
void ui_compt_realize(GtkWidget *widget, gpointer data);
void ui_win_pop_up_keyboard();
void ui_win_close_keyboard();
void ui_win_set_pos(GtkWidget *widget, double x_scale, double y_scale);
void ui_win_move_pos(GtkWidget *widget, double x_scale, double y_scale);
void ui_manager_publogin_timeout_enable();
void ui_manager_publogin_timeout_disable();

void ui_manager_offline_autologin_timer_enable();
void ui_manager_offline_autologin_timer_disable();

void ui_manager_hold_timer();
void ui_manager_config_enable();
void ui_manager_config_disable();
void ui_win_put_nice_position(GtkWidget *widget, int type, int width, double y_scale);
void ui_win_put_nice_position_cb(GtkWidget *widget, GtkAllocation *allocation, void *data);
void ui_win_btn_all_disable(void);
void ui_win_btn_all_enable(void);
void ui_win_btn_disable_config(void);
void ui_settype_timer_disable(void);
void ui_settype_err_prompt_hide(void);
int ui_get_current_show_widget(void);
void redisplay_adapt_control(int width, int height);
void ui_win_authbtn_move_greate_pos(GtkWidget *widget, double x_sub, double y_sub, int is_move);

#endif /* #ifdef UI_MAIN_H */

