#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <malloc.h>
#include <pthread.h>
#include "ui_main.h"
#include "ui_test.h"

#define SHOW_COMMAND  "/usr/bin/onboard &"
#define KILL_COMMAND  "kill `ps -ef | grep python3 | grep /usr/bin/onboard | grep -v sh | awk '{print $2}'`"
ui_main_t    g_win_manager;
char  update_image_size[6];

struct ui_comp_s *ui_tab[] = {
    &ui_win_main,
    &ui_win_button_box,
    &ui_win_user_login,
    &ui_win_password,
    &ui_win_status,
    NULL,//&ui_win_network_status,
    &ui_win_bindusr,
    &ui_win_wifi_btnbox,
    &ui_win_auth_btnbox,
    &ui_dialog_config,
    &ui_dialog_about,
    &ui_dialog_shutdown,
    &ui_dialog_settype,
    &ui_dialog_download,
    &ui_dialog_adminpwd,
    &ui_dialog_adminpwd_upgrade,
    &ui_dialog_tips,
    &ui_dialog_interactive,
    &ui_dialog_wifi,
    &ui_dialog_prop,
    &ui_dialog_wifipwd,
    &ui_dialog_authpwd,
    &ui_dialog_newdeploy_connect,
    &ui_dialog_status,
};

void ui_onboard_init()
{
    char str[80];
    sprintf(str, "/etc/RCC_Client/scripts/set_onboard_height.sh %d", g_win_manager.r_y/5);
    system(str);
}
 void ui_win_pop_up_keyboard()
{
   
    //logi("ui_win_pop_up_keyboard\n");
    system(SHOW_COMMAND);
    /*
    char cmd[100];
    int y,size_x,size_y;
    y = (int)(g_win_manager.r_y*0.75);
    size_x = (int)(g_win_manager.r_x);
    size_y = (int)(g_win_manager.r_y/4);
    //sprintf(cmd,"/usr/bin/onboard -x 10 -y %d --size=%dx%d &",y,size_x,size_y);
    sprintf(cmd,"/usr/bin/onboard  -x 10 -y 100 --size=%d*%d &",size_x,size_y);
    //logi("onboard = %s",cmd)
   
    */
}

 
 void ui_win_close_keyboard()
{
    //ui_tab[0]->adapt();
    logi("ui_win_close_keyboard");
    system(KILL_COMMAND);
}
void ui_win_authbtn_move_greate_pos(GtkWidget *widget, double x_sub, double y_sub, int is_move)
{
    double wifibtn_x = 94;
    double wifibtn_y = 90.33;
    double x_scale, y_scale;
    
    x_scale = g_win_manager.r_x * (wifibtn_x / 100) - x_sub;
    y_scale = g_win_manager.r_y * (wifibtn_y / 100) - y_sub;
    if (is_move) {
        gtk_fixed_move(GTK_FIXED(g_win_manager.win_fixed), widget, (int) x_scale, (int) y_scale);
    } else {
        gtk_fixed_put(GTK_FIXED(g_win_manager.win_fixed), widget, (int) x_scale, (int) y_scale);
    }
}

void ui_win_set_pos(GtkWidget *widget, double x_scale, double y_scale)
{
    double x = g_win_manager.r_x * (x_scale / 100);
    double y = g_win_manager.r_y * (y_scale / 100);

    //FIXME: 4:3 or 16:9 please adjust 

    gtk_fixed_put(GTK_FIXED(g_win_manager.win_fixed), widget, (int)x, (int)y);
}

void ui_win_move_pos(GtkWidget *widget, double x_scale, double y_scale)
{
    double x = g_win_manager.r_x * (x_scale / 100);
    double y = g_win_manager.r_y * (y_scale / 100);

    //FIXME: 4:3 or 16:9 please adjust 
    gtk_fixed_move(GTK_FIXED(g_win_manager.win_fixed), widget, (int)x, (int)y);
}


void ui_win_put_nice_position(GtkWidget *widget, int type, int width, double y_scale)
{
    int x, y;
    x = (g_win_manager.r_x - width) / 2;
    y = g_win_manager.r_y * (y_scale / 100);
    logi("move position x = %d, y = %d\n", x, y);

    if (type == 1) {
        gtk_fixed_move(GTK_FIXED(g_win_manager.win_fixed), widget, x, y);
    } else {
        gtk_fixed_put(GTK_FIXED(g_win_manager.win_fixed), widget, x, y);
    }
}

void ui_win_put_nice_position_cb(GtkWidget *widget, GtkAllocation *allocation, void *data) 
{
    struct ui_comp_s *ui_win = data;

    logi("width = %d, height = %d, status = %d\n", allocation->width, allocation->height,
            ui_win->status);

    ui_win_put_nice_position(widget, 1,  allocation->width, ui_win->height);

    g_signal_handler_disconnect(G_OBJECT (widget), ui_win->connect_id);
    ui_win->connect_id = 0;
}

static void ui_win_screen_init(void)
{   
    char width[32] = {0,};
    char height[32] = {0,};

    ui_extern_get_current_dpi(width, height);
    g_win_manager.r_x = atoi(width);
    g_win_manager.r_y = atoi(height);
   
    
    logi("g_win_manager.r_x is %d, g_win_manager.r_y is %d\n", g_win_manager.r_x, g_win_manager.r_y);
    if (g_win_manager.r_x == 1280 && g_win_manager.r_y == 720) {
    }
    else {
        if (g_win_manager.r_x < 1024) {
            g_win_manager.r_x = 1024;
        }
        if (g_win_manager.r_y < 768) {
            g_win_manager.r_y = 768;
        }
    }
    g_win_manager.r_y = g_win_manager.r_y-2;
}

static int ui_manager_init(void)
{
    /* try to get  */
    int i;

    ui_win_screen_init();
    ui_onboard_init();
    for (i = 0; i < ARRAY_SIZE (ui_tab); i++) {
        if (ui_tab[i]) {
            if (ui_tab[i]->is_init == UI_NEED_INIT) {
                ui_tab[i]->init();
            }
        }
    }

//    ui_tab[UI_TYPE_WIN_MAIN]->init();
//    ui_tab[UI_TYPE_WIN_BUTTON_BOX]->init();

    return 0;
}

void ui_manager_config_disable()
{
    g_win_manager.config_disable = 1;
}

void ui_manager_config_enable()
{
    g_win_manager.config_disable = 0;
}

static gboolean ui_manager_publogin_timeout(gpointer user_data)
{
    g_win_manager.pub_login_timer = 0;
    g_win_manager.timer_hold = 0;
   // ui_manager_config_disable();
    ui_extern_public_login();
    
    return FALSE;
}

void ui_manager_hold_timer()
{
    g_win_manager.timer_hold = 1;
}

void ui_manager_publogin_timeout_disable()
{
    if (g_win_manager.pub_login_timer) {
        g_source_remove(g_win_manager.pub_login_timer);
        g_win_manager.pub_login_timer = 0;
       // ui_manager_config_enable();
    }
}

void ui_manager_publogin_timeout_enable()
{
    ui_manager_publogin_timeout_disable();
    g_win_manager.pub_login_timer = g_timeout_add_seconds(5, ui_manager_publogin_timeout, NULL);
}

#define OFFLINE_AUTOLOGIN_TIMEOUT_SECONDS 15
/* auto login if offline for a few seconds */

static gboolean ui_manager_offline_autologin_timer_expire(gpointer user_data)
{
    g_win_manager.offlinelogin_timer = 0;
    g_win_manager.timer_hold = 0;
   // ui_manager_config_disable();
    ui_extern_enter_local_mode();

    return FALSE;
}


void ui_manager_offline_autologin_timer_disable()
{
    if (g_win_manager.offlinelogin_timer) {
        g_source_remove(g_win_manager.offlinelogin_timer);
        g_win_manager.offlinelogin_timer = 0;
       // ui_manager_config_enable();
    }
}

void ui_manager_offline_autologin_timer_enable()
{
    ui_manager_offline_autologin_timer_disable();
    g_win_manager.offlinelogin_timer = g_timeout_add_seconds(OFFLINE_AUTOLOGIN_TIMEOUT_SECONDS,
        ui_manager_offline_autologin_timer_expire, NULL);
}



static void ui_compt_change_status(struct ui_comp_s *compt, int status)
{
    compt->status = status;
    logi("change type:%d status:%d\n", compt->type, compt->status);
}

static void ui_compt_change_hide(GtkWidget *widget, gpointer data)
{
    struct ui_comp_s *compt = data;
    ui_compt_change_status(compt, UI_STATUS_HIDE);
}

static void ui_compt_change_show(GtkWidget *widget, gpointer data)
{
    struct ui_comp_s *compt = data;
    ui_compt_change_status(compt, UI_STATUS_SHOW);
}

static void ui_compt_change_destroy(GtkWidget *widget, gpointer data)
{
    struct ui_comp_s *compt = data;
    *compt->widget = NULL;
    ui_compt_change_status(compt, UI_STATUS_NONE);
}

static gboolean ui_compt_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    if (GDK_Escape == event->keyval)
    {
        logi("ui press esc\n");
        return TRUE;
    }
    return FALSE;
}

static gboolean deal_key_press(GtkWidget *widget, GdkEventKey  *event, gpointer data)
{
    gchar temp[512] = {0, };

    strcpy(temp, "press: ");
    if(event->state & GDK_CONTROL_MASK) {
        strcat(temp, "CTRL+");
    }
    
    if(event->state & GDK_SHIFT_MASK) {
        strcat(temp, "SHIFT+");
    }
    
    if(event->state & GDK_META_MASK) {
        strcat(temp, "ALT+");
    }
    
    if(event->state & GDK_LOCK_MASK) {
        strcat(temp, "LOCK+");  
    }
    
    if (event->keyval !=0) {
        strcat(temp, gdk_keyval_name(event->keyval));
    }

    if (strcmp(temp, "press: CTRL+SHIFT+D") == 0) {
        logi("allow download image\n");
        ui_extern_reset_logic_machine();
    }

    return FALSE;
}

void ui_compt_realize(GtkWidget *widget, gpointer data)
{
    struct ui_comp_s *compt = data;

    logi("realize type:%d\n", compt->type);
    g_signal_connect (G_OBJECT (*compt->widget), "destroy",
                      G_CALLBACK (ui_compt_change_destroy), compt);
    g_signal_connect (G_OBJECT (*compt->widget), "hide",
                      G_CALLBACK (ui_compt_change_hide), compt);
    g_signal_connect (G_OBJECT (*compt->widget), "show",
                      G_CALLBACK (ui_compt_change_show), compt);

    g_signal_connect(G_OBJECT (*compt->widget), "key-press-event",
                      G_CALLBACK(ui_compt_key_press_event), NULL);

    g_signal_connect(G_OBJECT(*compt->widget), "key_release_event",
                      G_CALLBACK(deal_key_press), NULL);

    if (compt->subtype == UI_TOPLEVEL_TYPE_WIN) {
        compt->connect_id = g_signal_connect(G_OBJECT (*compt->widget), "size-allocate", 
                                        G_CALLBACK(ui_win_put_nice_position_cb), compt);
    }

    ui_compt_change_show(widget, compt);
}

int ui_get_current_show_widget(void)
{
	int i;
    for (i = UI_TYPE_WIN_USER_LOGIN; i < UI_TYPE_END; i++) {
        if (ui_tab[i]) {
            if (ui_tab[i]->status == UI_STATUS_SHOW) {
            	return i;
            }
        }
    }
    return UI_TYPE_END;
}

static void ui_clean_screen()
{
    int i;
    
    for (i = UI_TYPE_WIN_USER_LOGIN; i < UI_TYPE_END; i++) {
        if (i == UI_TYPE_WIN_AUTH_BTNBOX) {
            continue;
        }
        if (ui_tab[i]) {
            if (ui_tab[i]->status == UI_STATUS_SHOW) {
                if (ui_tab[i]->hide) {
                    ui_tab[i]->hide();
                }
            }
        }
    }
}

static void ui_destroy_screen()
{
    int i;

    for (i = UI_TYPE_WIN_MAIN; i < UI_TYPE_END; i++) {
        if (ui_tab[i]) {
            if (ui_tab[i]->destroy) {
                logi("ui_destroy_screen type: %d\n", i);
                ui_tab[i]->destroy();
            }
        }
    }
}

static void ui_clean_screen_except(int type)
{
    int i;


    for (i = UI_TYPE_WIN_USER_LOGIN; i < UI_TYPE_END; i++) {
    	if (i == type) {
    		continue;
    	}
    	if (i == UI_TYPE_DIALOG_CONFIG || i == UI_TYPE_DIALOG_ABOUT || i == UI_TYPE_DIALOG_SHUTDOWN 
            || i == UI_TYPE_DIALOG_TIPS || i == UI_TYPE_WIN_AUTH_BTNBOX) {
            logi("%s is %d\n", __func__, i);
            continue;
    	}

        if (ui_tab[i]) {
            if (ui_tab[i]->status == UI_STATUS_SHOW) {
                if (ui_tab[i]->hide) {
                    ui_tab[i]->hide();
                }
            }
        }
    }
}

static void ui_clean_screen_except_self(int type)
{
    int i;

    for (i = UI_TYPE_WIN_USER_LOGIN; i < UI_TYPE_END; i++) {
        if (i == type) {
            logi("%s is %d\n", __func__, i);
            continue;
        }

        if ( i == UI_TYPE_WIN_AUTH_BTNBOX) {
            logi("%s is %d\n", __func__, i);
            continue;
        }

        if (ui_tab[i]) {
            if (ui_tab[i]->status == UI_STATUS_SHOW) {
                if (ui_tab[i]->hide) {
                    ui_tab[i]->hide();
                }
            }
        }
    }
}

void redisplay_adapt_control(int width, int height)
{
    int i = 0;
    g_win_manager.r_x = width;
    g_win_manager.r_y = height;

    logi("redisplay_adapt_control: %dx%d\n",g_win_manager.r_x,g_win_manager.r_y);
    system(KILL_COMMAND);
    ui_onboard_init();
    if (g_win_manager.r_x == 1280 && g_win_manager.r_y == 720) {

    } else {
        if (g_win_manager.r_x < 1024) {
            g_win_manager.r_x = 1024;
        } if (g_win_manager.r_y < 768) {
            g_win_manager.r_y = 768;
        }
    }

    g_win_manager.r_y = g_win_manager.r_y-2;
    //system(KILL_COMMAND);
    for (i = 0; i < UI_TYPE_END; i++) {
        if (ui_tab[i]) {
            if (ui_tab[i]->adapt) {
                ui_tab[i]->adapt();
            }
        }
    }
}

static void ui_io_set_mode0(ui_msg_t *msg, int compt)
{
    ui_tab[compt]->ctrl(msg);
}

static void ui_io_set_mode1(ui_msg_t *msg, int compt)
{
    ui_tab[compt]->ctrl(msg);
    ui_tab[compt]->show();
}

static void ui_io_set_mode2(ui_msg_t *msg, int compt, int object)
{
    msg->object = object;
    ui_io_set_mode1(msg, compt);
}

static void ui_io_set(ui_msg_t *msg)
{
    logi("io type:%d subtype:%d\n", msg->object, msg->sub_obj);

    switch (msg->object) {
    case UI_CTRL_SHOW_WIN:
        ui_clean_screen();
        ui_io_set_mode1(msg, UI_TYPE_WIN_STATUS);
        break;
    case UI_CTRL_SHOW_USER_LOGIN:
        ui_clean_screen();
        ui_tab[UI_TYPE_WIN_USER_LOGIN]->show();
        break; 
    case UI_CTRL_SHOW_PASSWORD:
        ui_clean_screen();
        ui_tab[UI_TYPE_WIN_PASSWORD]->show();
        break;
    case UI_CTRL_SHOW_SETTYPE:
        ui_clean_screen();
        ui_tab[UI_TYPE_DIALOG_SETTYPE]->show();
        break;
    case UI_CTRL_SHOW_AUTO_SHUTDOWN:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_AUTOSHUTDOWN);
        break;
    case UI_CTRL_SHOW_ISO_UPGRADE_ERR:
        //ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_UPGRADEERR);
        break;
    case UI_CTRL_SHOW_EASYDEPLOY_TIPS:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_EASYDEPLOY);
        break;
    case UI_CTRL_SHOW_VMERR_TIPS:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_VMERR);
        //ui_tab[UI_TYPE_DIALOG_INTERACTIVE]->show();
        break;
    case UI_CTRL_SHOW_OSTYPEERR_TIPS:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_OSTYPERR);
        break;
    case UI_CTRL_SHOW_OSTYPENOSPORT_TIPS:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_OSTYPENOTSPORT);
        break;
    case UI_CTRL_SHOW_CPUNOSPORT_TIPS:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_CPUNOTSPORT);
        break;
    case UI_CTRL_SHOW_VMMODEINILOST_TIPS:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_VMMODEINILOST);
        break;
    case UI_CTRL_SHOW_DRIVERNOTADAPT_TIPS:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_DRIVER_ERR);
        break;
    case UI_CTRL_SHOW_CREATE_CDISK_FAIL_TIPS:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_CDISK_ERR);
        break;
    case UI_CTRL_SHOW_VM_LAYER_TEMPLETE_TIPS:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_LAYER_TEMPLETE_ERR);
        break;
    case UI_CTRL_SHOW_NO_AUDIO_DEVICE_TIPS:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_INTEL_NO_AUDIO_DEVICE);
        break;
    case UI_CTRL_SHOW_DOWNLOAD_CONFIRM_TIPS:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_DOWNLOAD_CONFIRM);
        break;
   case UI_CTRL_SHOW_DOWNLOAD_MODE_SELECT_TIPS:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_DOWNLOAD_MODE_SELECT);
        break;
   case UI_CTRL_SHOW_NEED_MERGE_TIPS:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_NEED_MERGE);
        break;     
    case UI_CTRL_SHOW_VM_LASTERR_TIPS:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_VMLASTERR);
        break;
    case UI_CTRL_SHOW_IMG_BIG_TIPS:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_IMGBIG);
        break;
    case UI_CTRL_SHOW_BAD_DRIVER_TIPS:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_BADDRIVER);
        break;
    case UI_CTRL_SHOW_NOTSUPPORTXP_TIPS:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_NOTSUPPORT_XP);
        break;
    case UI_CTRL_SHOW_NOTSUPPORTWIN10_32_TIPS:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_NOTSUPPORT_WIN10_32);
        break;
    case UI_CTRL_SHOW_IMAGE_ABNORMAL_TIPS:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_IMGABNORMAL);
        break;
    case UI_CTRL_SHOW_USER_BINDER:
        ui_clean_screen();
        ui_tab[UI_TYPE_WIN_BINDUSR]->show();
        break; 
    case UI_CTRL_SHOW_IMAGE_UPDATE:
    	ui_clean_screen_except(UI_TYPE_DIALOG_DOWNLOAD);
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_DOWNLOAD, UI_DIALOG_DOWNLOAD_CHOICE);
        break;
    case UI_CTRL_SHOW_IMAGE_UPDATING:
    	ui_clean_screen_except(UI_TYPE_DIALOG_DOWNLOAD);
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_DOWNLOAD, UI_DIALOG_DOWNLOAD_UPDATEING);
        break;
    case UI_CTRL_SHOW_IMAGE_INITING:
        ui_clean_screen_except(UI_TYPE_DIALOG_DOWNLOAD);
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_DOWNLOAD, UI_DIALOG_DOWNLOAD_INITING);
        break;
    case UI_CTRL_SHOW_DOWNLOAD_PROGRESS:
    	ui_clean_screen_except(UI_TYPE_DIALOG_DOWNLOAD);
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_DOWNLOAD, UI_DIALOG_DOWNLOAD_PROGRESS);
        break;
    case UI_CTRL_SHOW_USB_COPY_PROGRESS:
    	ui_clean_screen_except(UI_TYPE_DIALOG_DOWNLOAD);
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_DOWNLOAD, UI_DIALOG_USB_COPY_PROGRESS);
        break;
    case UI_CTRL_SHOW_CONFIG:
        ui_clean_screen();
        ui_tab[UI_TYPE_DIALOG_CONFIG]->show();
        break;
    case UI_CTRL_SHOW_CONFIG_RESULT:
        ui_io_set_mode1(msg, UI_TYPE_DIALOG_TIPS);
        break;
    case UI_CTRL_GTK_THREAD_QUIT:
        //ensure ui event update before quit gtk thread.
        while(gtk_events_pending()) {
            gtk_main_iteration();
        }
        gtk_main_quit();
        break;
    case UI_CTRL_SHOW_USER_LOGIN_RESULT:
        ui_io_set_mode0(msg, UI_TYPE_WIN_USER_LOGIN);
        break;
    case UI_CTRL_SHOW_PASSWORD_RESULT:
        ui_io_set_mode0(msg, UI_TYPE_WIN_PASSWORD);
        break;
    case UI_CTRL_SHOW_USER_BINDER_RESULT:
        ui_io_set_mode0(msg, UI_TYPE_WIN_BINDUSR);
        break;
    case UI_CTRL_WINBTN_SET:
        ui_io_set_mode0(msg, UI_TYPE_WIN_BUTTON_BOX);
        break;
    case UI_CTRL_SHOW_SETTYPE_RESULT:
        ui_io_set_mode0(msg, UI_TYPE_DIALOG_SETTYPE);
        break;
    case UI_CTRL_SHOW_NEWDEPLOY_CONNECT:
        ui_io_set_mode1(msg, UI_TYPE_DIALOG_NEWDEPLOY_CONNECT);
        ui_clean_screen_except_self(UI_TYPE_DIALOG_NEWDEPLOY_CONNECT); 
        break;
    case UI_CTRL_SHOW_DIALOG_TIPS:
        ui_io_set_mode1(msg, UI_TYPE_DIALOG_TIPS);
        break;
    case UI_CTRL_SHOW_DIALOG:
        ui_clean_screen();
        ui_io_set_mode1(msg, UI_TYPE_DIALOG_STATUS);
        break;
    case UI_CTRL_SHOW_WIFI_BTNBOX:
        ui_io_set_mode0(msg, UI_TYPE_WIN_WIFI_BTNBOX);
        break;
    case UI_CTRL_SHOW_WIFI_DIALOG:
        ui_io_set_mode0(msg, UI_TYPE_DIALOG_WIFI);
        break;
    case UI_CTRL_SHOW_WIFI_PWD:
        ui_io_set_mode1(msg, UI_TYPE_DIALOG_WIFIPWD);
        break;
    case UI_CTRL_HIDE_WIFI_PWD:
        ui_tab[UI_TYPE_DIALOG_WIFIPWD]->hide();
        break;
    case UI_CTRL_DESTORY_ALL_SCREEN:
        ui_destroy_screen();
        break;
    case UI_CTRL_HIDE_DIALOG_TIPS:
        ui_tab[UI_TYPE_DIALOG_TIPS]->hide();
        break;
    case UI_CTRL_SHOW_WIFI_PROP:
        ui_io_set_mode1(msg, UI_TYPE_DIALOG_PROP);
        break;
    case UI_CTRL_REDISPLAY_SCREEN:
        redisplay_adapt_control(((ui_display_arg_t*)(msg->args))->width, ((ui_display_arg_t*)(msg->args))->height);
        break;
    case UI_CTRL_SHOW_EASYDEPLOY_ERR:
        ui_clean_screen();
        ui_io_set_mode2(msg, UI_TYPE_DIALOG_INTERACTIVE, UI_DIALOG_INTERACTIVE_SHOW_EASYDEPLOY_NOT_WIFI);
        break;
    case UI_CTRL_SHOW_AUTH_BTNBOX:
        ui_io_set_mode0(msg, UI_TYPE_WIN_AUTH_BTNBOX);
        break;
    case UI_CTRL_HIDE_AUTH_BTNBOX:
        ui_tab[UI_TYPE_WIN_AUTH_BTNBOX]->hide();
        break;
    case UI_CTRL_SHOW_AUTH_PWD:
        ui_io_set_mode0(msg, UI_TYPE_DIALOG_AUTHPWD);
        break;
    case UI_CTRL_DESTORY_AUTHPWD:
        ui_tab[UI_TYPE_DIALOG_AUTHPWD]->destroy();
        break;
    case UI_CTRL_SHOW_WIN_MAIN:
        ui_io_set_mode0(msg, UI_TYPE_WIN_MAIN);
        break;
    }
}

static gboolean ui_handle_msg_idle(gpointer user_data)
{
    ui_msg_t *msg = user_data;

    ui_io_set(msg);

    if (msg->args) {
        free(msg->args);
    }

    free(msg);
    return FALSE;
}


void ui_handle_msg(void *msg)
{
    g_idle_add(ui_handle_msg_idle, msg);
}

int ui_thread_init(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    ui_win_screen_init();

    gtk_main();
    return 0;
}

int ui_main( int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    log_init(LOG_FILE, "/var/log/idv_ui.log");
    log_set_split_size(3*1024*1024);

    ui_manager_init();
    if (ui_extern_userinfomgr_permitted()) {
        ui_tab[UI_TYPE_WIN_MAIN]->show();
        ui_tab[UI_TYPE_WIN_BUTTON_BOX]->show();
        ui_tab[UI_TYPE_WIN_WIFI_BTNBOX]->show();
    }
#if 0
    // for test
    pthread_t id_1;
    pthread_create(&id_1,NULL,(void *) ui_test_main,NULL); 
#endif
    gtk_main();
    return 0;
}

#ifndef IDV_CLIENT
int main( int argc, char *argv[])
{
    return ui_main(argc, argv);
}
#endif
