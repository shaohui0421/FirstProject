#ifndef UI_API_H
#define UI_API_H
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif
#define MAX_EXT_SCREEN_NUM        2

#define UI_USER_LOGO                             "./icon/user_logo"
#define UI_USER_BG                               "./icon/user_bg"
#define UI_USER_TMP_LOGO                         "/tmp/tmp_user_logo"
#define UI_USER_TMP_BG                           "/tmp/tmp_user_bg"

#define LIMIT_DISPLAY_WIDTH    1024
#define LIMIT_DISPLAY_HEIGHT   720
#define LIMIT_DISPLAY_HEIGHT_SPEC   768

#define EXT_LIMIT_DISPLAY_WIDTH     800
#define EXT_LIMIT_DISPLAY_HEIGHT    600

typedef struct ui_resolution_info_t {
    int width;    //the width of resolution
    int height;   //the height of resolution
    int flag;     //Resolution level bit 0 :RES_BEST bit 1:RES_CURRENT
    int custom;   //custom resolution
} ui_res_info;

typedef struct ui_ext_resolution_info_t {
    int width[MAX_EXT_SCREEN_NUM];    //the width of resolution
    int height[MAX_EXT_SCREEN_NUM];   //the height of resolution
    int flag[MAX_EXT_SCREEN_NUM];     //Resolution level bit 0 :RES_BEST bit 1:RES_CURRENT
    int custom[MAX_EXT_SCREEN_NUM];   //custom resolution
} ui_ext_res_info;

typedef struct l2u_download_s{
    int         status; // 参见ui_dialog_download_status_enum
    char       *speed;  //下载速度例如"2.3MB/s"
    double      process;    //下载百分比
    int         err_code;
    int         filesize;
    int         downloaded;
    int         remain_sec;
} l2u_download_t;

typedef struct ui_ipinfo_t {
	char ip_addr[20];
	char mask[20];
    char gate[20];
    char dns[20];
    char dns_back[20];
    char mac[20];
    int mode;
} ui_ipinfo;

typedef struct ui_scanresult_t {
	char ssid[128];
	char ssid_mac[20];
    unsigned int freq;
    int idensity;
    unsigned int key_mgmt;
    char flag[128];
} ui_scanresult;

typedef struct ui_wifiinfo_t {
	char ssid[128];
    char ssid_mac[20];
	unsigned int freq;
    int idensity;
    unsigned int key_mgmt;
    ui_ipinfo ip_info;
} ui_wifiinfo;

typedef struct ui_wificonfig_t {
    int manual;
    int nokey;
	char ssid[128];
    char ssid_mac[20];
	unsigned int net_id;
    unsigned int priority;
    unsigned int key_mgmt;
    int scan_ssid;
    unsigned int pairwise;
	char psk[128];
    char wep_key0[128];
    unsigned int eap;
    char identity[128];
    char password[128];
	int saved;
    int flags;
} ui_wificonfig;

typedef struct upload_config_t {
    char ssid[128];
    int key_mgmt;
    char usrname[128];
    char passwd[128];
    int save;
} upload_config_s;

typedef struct ui_save_dpi_info_t {
    int res_index;
    int custom; 
} ui_save_dpi_info;

typedef enum {
    UI_DIALOG_DOWNLOAD_ST_NORMAL = 0,   //download
    UI_DIALOG_DOWNLOAD_ST_CHECKING,     //checking
    UI_DIALOG_DOWNLOAD_ST_ERROR,        //error, retry
    UI_DIALOG_DOWNLOAD_ST_INITING,      //bt initing
    UI_DIALOG_CP_BASE_ST_NORMAL,
    UI_DIALOG_CP_BASE_ST_ERROR,
    UI_DIALOG_DOWNLOAD_ST_MERGE,     //MERGE
} ui_dialog_download_status_enum;

typedef enum {
    UI_DIALOG_DOWNLOAD_ERR_UNKNOWN = 0,   
    UI_DIALOG_DOWNLOAD_ERR_TIMEOUT,
    UI_DIALOG_DOWNLOAD_ERR_NOSPACE,
    UI_DIALOG_DOWNLOAD_ERR_MD5,
    UI_DIALOG_CP_BASE_MOUNTED_DEV_NOT_EXIST,
    UI_DIALOG_CP_BASE_UPDATE_STATUS_ERROR
} ui_dialog_download_err_enum;

typedef enum {
    UI_TIPS_NOCONF_IMAGE_OPT = 1,
    UI_TIPS_NOCONF_POLICY_OPT = 2,
    UI_TIPS_RECOVERY_IMAGE_OPT = 4,
    UI_TIPS_SERVER_ERROR_OPT = 5,
    UI_TIPS_CLIENT_INIT_OPT = 6,
    UI_TIPS_DEV_NO_IMG_OPT = 7,
    UI_TIPS_DEV_FORBIDED_OPT = 8,
    UI_TIPS_DEV_MODE_CHANGE_OPT = 9,
    UI_TIPS_DEV_USER_CHANGE_OPT = 10,
    UI_TIPS_NET_DISCONNETCT_OPT = 11,
    UI_TIPS_LICENSE_OVERRUN_OPT = 14,
    UI_TIPS_DEV_OVERRUN_OPT = 15,
    UI_TIPS_IMG_BIG_OPT = 16,
    UI_TIPS_DELETE_TEACHERDISK_OPT = 17,
    UI_TIPS_DEV_LOCKED_OPT = 19,
    UI_TIPS_IMG_BAD_DRIVER_OPT = 20,
    UI_TIPS_IMG_USER_ERR_OPT = 25,
    UI_TIPS_IMG_NOT_FOUND_OPT = 26,
    UI_TIPS_IMG_ABNORMAL_OPT = 27,
    UI_TIPS_SERVER_NOT_VALID_OPT = 28,
    UI_TIPS_SERVER_IS_CLASS_OPT = 29,
    UI_TIPS_DEV_SERVER_MAINTAIN_OPT = 30,
    UI_TIPS_IMG_VERSION_GT_OUTDATED_OPT = 31,
    UI_TIPS_DEV_NONSPUPPORT_OSTYPE32_OPT = 32,
    UI_TIPS_DEV_NONSPUPPORT_OSTYPEXP_OPT = 33,
    UI_TIPS_AUTUHOR_NOT_FOUND_OPT = 122,
    UI_TIPS_AUTUHOR_OUT_OF_RANGE_OPT = 123,
} ui_tips_opt_enum;


typedef enum {
    UI_TIPS_NOCONF_IMAGE_RET = 1,
    UI_TIPS_NOCONF_POLICY_RET = 2,
    UI_TIPS_RECOVERY_IMAGE_RET = 4,
    UI_TIPS_SERVER_ERROR_RET = 5,
    UI_TIPS_CLIENT_INIT_RET = 6,
    UI_TIPS_DEV_NO_IMG_RET = 7,
    UI_TIPS_DEV_FORBIDED_RET = 8,
    UI_CLIENT_UPDATE_FAIL_RET = 9,
    UI_TIPS_NET_DISCONNETCT_RET = 11,
    UI_TIPS_DEV_MODE_CHANGE_RET = 12,
    UI_TIPS_DEV_USER_CHANGE_RET = 13,
    UI_TIPS_LICENSE_OVERRUN_RET = 14,
    UI_TIPS_DEV_OVERRUN_RET = 15,
    UI_TIPS_IMG_BIG_RET = 16,
    UI_TIPS_DELETE_TEACHERDISK_RET = 17,
    UI_TIPS_DEV_LOCKED_RET = 19,
    UI_TIPS_IMG_BAD_DRIVER_RET = 20,
    UI_TIPS_IMG_USER_ERR_RET = 25,
    UI_TIPS_IMG_NOT_FOUND_RET = 26,
    UI_TIPS_IMG_ABNORMAL_RET = 27,
    UI_TIPS_SERVER_NOT_VALID_RET = 28,
    UI_TIPS_SERVER_IS_CLASS_OPT_RET = 29,
    UI_TIPS_DEV_SERVER_MAINTAIN_RET = 30,
    UI_TIPS_IMG_VERSION_GT_OUTDATED_RET = 31,
    UI_TIPS_DEV_NONSPUPPORT_OSTYPE32_RET = 32,
    UI_TIPS_DEV_NONSPUPPORT_OSTYPEXP_RET = 33,
    UI_TIPS_UPGRADE_FAIL_RET = 34,
} ui_tips_ret_enum;

typedef enum {
    UI_TIPS_CONNECTING = 0,
    UI_TIPS_CONNECT_SUCCESS,
    UI_TIPS_CONNECT_FAIL,
    UI_TIPS_AUTHENTICE_FAIL,
    UI_TIPS_INVALID_TYPE,
    UI_TIPS_NOT_SUPPORT_WEB_HOTSPOT,
    UI_TIPS_CONNECT_NOT_WHITESSID,
} ui_tips_connect_enum;


typedef enum 
{
    UI_TIPS_VM_START_FAILED = 0,
    UI_TIPS_VM_OSTYPE_UNKNOWN, 
    UI_TIPS_VM_NOT_SUPPORT_CPU,
    UI_TIPS_VM_NOT_SUPPORT_OSTYPE,
    UI_TIPS_VM_VMMODEINT_LOST,
    UI_TIPS_VM_DRIVER_OSTYPE_NOTADAPT,
    UI_TIPS_VM_CREATE_CDISK_FAILED,
    UI_TIPS_VM_START_LAYER_TEMPLETE,
    UI_TIPS_VM_START_INTEL_NO_AUDIO_DEVICE = 9,
} ui_tips_vmerr_enum;


typedef enum {
    UI_DEV_LOCK_NONE = 0,
    UI_DEV_LOCK_DIFF_MODE = 1<<0, 
    UI_DEV_LOCK_DIFF_BINDUSER = 1<<1,
    UI_DEV_LOCK_DIFF_RECOVERY = 1<<2,
    UI_DEV_LOCK_DIFF_USERDISK = 1<<3,
    UI_DEV_LOCK_LAYER_DOWNGRADE = 1<<4,
    UI_DEV_LOCK_MAX = 1<<5,
} ui_dev_lock_type_enum;

typedef enum {
    UI_TIPS_NET_AUTH_SUCCESS,
    UI_TIPS_NET_AUTHING,
    UI_TIPS_NET_AUTH_FAIL,
    UI_TIPS_NET_AUTH_FAIL_USER_ERR,
    UI_TIPS_NET_AUTH_TIMEOUT,
    UI_TIPS_NET_AUTH_DISCONNECT_SUCCESS,
    UI_TIPS_NET_AUTH_EXIST,
    UI_TIPS_NET_AUTH_DISEXIST,
    UI_TIPS_NET_AUTH_DISCONNECT,
    UI_TIPS_NET_AUTH_MAX,
} ui_tips_net_auth_enum;

typedef enum {
    UI_AUTHBTN_DISABLED = UI_TIPS_NET_AUTH_MAX,
    UI_AUTHBTN_ENABLED,
} ui_authbtn_status_enum;

//same as auth_manager.h AuthType
typedef enum {
    UI_AUTH_TYPE_NONE    = 0x00,    //no auth
    UI_AUTH_TYPE_DOT1X   = 0x01,    //802.1x auth
    UI_AUTH_TYPE_WEB     = 0x02,    //web auth
} ui_auth_type_enum;

typedef enum {
    UI_CALL_MAIN_LOGO_SET = 1,
    UI_CALL_MAIN_LOGO_RESET,
    UI_CALL_MAIN_BG_SET,
    UI_CALL_MAIN_BG_RESET,
} ui_set_main_enum;

typedef struct auth_info_s {
    int auth_type;                  //ui_auth_type_enum
    int auto_connect;
    char username[128];
    char password[128];
} auth_info_t;

//显示下载失败提示界面
void l2u_show_upgrade_faile(void);
void l2u_destroy_screen(void);
void l2u_show_settype(const char * name, int size);
//显示未配置提示信息
void l2u_show_unconfig(void);
//显示正在连接提示信息
void l2u_show_connecting(void);
//显示用户登录提示界面
void l2u_show_userlogin(void);
//显示用户绑定提示界面
void l2u_show_userbind(void);
//显示镜像是否需要更新提示界面
void l2u_show_imageupdate(void);
//正在更新镜像中
void l2u_show_imageupdating(void);
//正在准备下载环境中
void l2u_show_image_initing(l2u_download_t* args);
//显示/更新下载提示框（可选的参数：当前状态，下载进度，速率等）
void l2u_show_download(l2u_download_t* args);
//显示/更新拷贝提示框（可选的参数：当前状态，拷贝进度，速率等）
void l2u_show_usb_copy(l2u_download_t* args);
//显示密码修订框
void l2u_show_password(void);
//显示公共用户登录界面
void l2u_show_publiclogin(void);
//显示用户登录界面
void l2u_show_login_tip(void);
//与云主机断开连接
void l2u_show_disconnect(void);
//正在升级软件，请稍候
void l2u_show_updateclient(void);
//正在升级软件，请稍候
void l2u_show_config(void);
//返回配置保存结果
void l2u_result_config(int result);
//返回登录结果
void l2u_result_user(int result);
//返回绑定结果
void l2u_result_binduser(int result);
//升级软件完成，系统将在5秒后重启
void l2u_result_updateclient(int result);
//UI主线程退出
void l2u_ui_thread_quit(void);
//UI主线程启动
int ui_main( int argc, char *argv[]);
int ui_thread_init(int argc, char *argv[]);
//相关提示信息
void l2u_show_tips(int type);

void l2u_show_layer_downgrade(int lock_type);

void l2u_show_image_err_tips(int type, int errcode);

void l2u_result_settype(int result);

void l2u_show_server_err(int type);

void l2u_ctrl_winbtn(gboolean value);

void l2u_show_auto_shutdown(void);

void l2u_show_iso_upgrade_err(void * callback, void * args);

void l2u_show_easydeploy_tips(void * callback, void * args);

void l2u_show_easydeploy_err(void * callback, void * args);

void l2u_show_vmerr_tips(void * callback, void * args, void *data, int result);

void l2u_show_download_confirm_tips(void * callback, void * args, void * confirm);

void l2u_show_download_mode_select_tips(void * callback, void * args, void * confirm);

void l2u_set_update_image_size(char * size);

void l2u_show_need_merge_tips(void * callback, void * args, void * confirm);

void l2u_show_vm_lasterr_tips(void * callback, void * args);

void l2u_show_bad_driver_tips(void * callback, void * args);

void l2u_show_img_big_tips(void * callback, void * args);

void l2u_show_image_abnormal_tips(void * callback, void * args);

void l2u_show_nonsupportxp_tips(void * callback, void * args);

void l2u_show_nonsupportwin10_32_tips(void * callback, void * args);

void l2u_show_recover_disk(const char * status, int size);

void l2u_show_start_vm_fail_hdmi_err();

void l2u_show_newdeploy_connect(int type);

void l2u_show_dialog_connect_network(int type);

void l2u_show_dialog_config_ip();

void l2u_show_dialog_copy_base(void* callback1, void* args1, void* callback2, void* args2);

void l2u_show_dialog_copy_base_insert_u_disk(void* callback, void* args);

void l2u_show_dialog_copy_base_fail(void* callback, void* args);

void l2u_show_dialog_copy_base_fail_no_space(void* callback, void* args);

void l2u_show_dialog_copy_base_fail_no_dev(void* callback, void* args);

void l2u_show_newdeploy_finish();

void l2u_show_wifi_pwd_win(int result, char* ssid);

void l2u_show_wifi_status(int result, ui_wifiinfo *info, ui_scanresult *list, int length);

void l2u_show_net_status(int result, int net_status);

void l2u_show_wifi_btnbox(int result , int type, int intensity);

void l2u_hide_wifi_pwd_win(void);

void l2u_hide_dialog_connect_network(void);

void l2u_show_wifi_auth(int type);

void l2u_show_dev_lock(int cur_mode, int lock_type);

void l2u_show_dialog_save_dpi(void *callback1, void* args1, void* callback2, void* args2);

void l2u_show_dialog_layer_manage(void *callback1, void* args1, void* callback2, void* args2);

void l2u_redisplay_adapt_control(const int width, const int height);

void l2u_show_net_auth(int type);

void l2u_show_main_window(int type);

void l2u_show_net_auth_disconnect(int type);

void l2u_show_net_auth_winbtn(int type);

#ifdef __cplusplus
}
#endif

#endif
