#include <gtk/gtk.h>
#include <string.h>
#include "ui_main.h"
#include "ui_test.h"
#include "../../include/application_c_interfaces.h"

#define	ENTRY_TEXT_LEN	128
#define DEF_PWD		"123456"

static int init = 0;

typedef struct login_info {
	char name[ENTRY_TEXT_LEN];
	char password[ENTRY_TEXT_LEN];
	gboolean remember;
	gboolean is_guest;
} login_info_t;

static login_info_t login_info;
static GtkWidget *vbox_all;
static GtkWidget * bg_image = NULL;

static GtkWidget * err_prompt;
static btn_apend_t * btn_user;
static btn_apend_t * btn_guest;
static GtkWidget * vbox_err;
static GtkWidget *user_entry;
static GtkWidget *pass_entry;
static GtkWidget *checkbox;
static btn_apend_t * btn_modpwd;
static btn_apend_t * btn_netmode;
static int width;
static int empty_str(char *buf)
{
	return buf[0] == '\0' ?  1 : 0;
}

static void trim_space(char* str)
{
    int i;
    int len;

    if (str == NULL || *str == '\0') {
        return;
    }
    len = strlen(str);
    //find rtrim pos
    for (i = len - 1; i >= 0; i--) {
        if (str[i] == ' ') {
            str[i] = '\0';
        } else {
            break;
        }
    }
    //find ltrim pos
    for (i = 0; i < len; i++) {
        if (str[i] != ' ') {
            break;
        }
    }
    printf("%s, %d, %d\n", str, len, i);
    if (i > 0) {
        memmove(str, str + i, len + 1 - i);
    }
}

void disable_btn_widget()
{
	set_btn_effect(btn_user, FALSE);
	set_btn_effect(btn_guest, FALSE);
	set_btn_effect(btn_modpwd, FALSE);
	set_btn_effect(btn_netmode, FALSE);
}

void able_btn_widget()
{
	set_btn_effect(btn_user, TRUE);
	set_btn_effect(btn_guest, TRUE);
	set_btn_effect(btn_modpwd, TRUE);
	set_btn_effect(btn_netmode, TRUE);
}

gint back2net_mode(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
	disable_btn_widget();
	ui_extern_back2_netmode(0);
	return FALSE;
} 


gint user_login(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
	login_info_t * info;
	info = (login_info_t *)data;
	
	info->is_guest = FALSE;

    //trim left or right space characters of username
    trim_space(info->name);
    logi("login:%s; name %s; check(%s)\n",info->is_guest ? "guest" : "user", info->name, info->remember ? "ok" : "no");
	disable_btn_widget();
    ui_extern_user_login(info->name, info->password, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox)));
    ui_set_login();
	return FALSE;
} 


gint guest_login(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
	login_info_t * info;
	info = (login_info_t *)data;

	info->is_guest = TRUE;
	logi("login:%s; name %s; pass %s, check(%s)\n",info->is_guest ? "guest" : "user", info->name, info->password, info->remember ? "ok" : "no");
	disable_btn_widget();
	ui_extern_guest_login();
	ui_set_login();
	//login_info.is_guest = TRUE;
	//g_print("login:%s; name %s; pass %s, check(%s)\n", login_info.is_guest ? "guest" : "user", login_info.name, login_info.password, login_info.remember ? "ok" : "no");
	return FALSE;	
	
} 

static void user_login_enter(GtkEntry *entry, gpointer  user_data)
{
	btn_loading_show(btn_user);
	user_login(NULL, NULL, &login_info);
    return;
}


void chbtn_click(GtkWidget *widget, gpointer data)
{
	//login_info_t * info;
	//info = (login_info_t *)data;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
    {
		//info->remember = TRUE;
		*((gboolean *)(data)) = TRUE;
    } else {
    	*((gboolean *)(data)) = FALSE;
		//info->remember = FALSE;
    }	
    //g_print("is %s \n",  login_info.remember ? "ok" : "no");
	return;
}


gint change_passwd_handle(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
/*
    btn_apend_t * btn_apend = (btn_apend_t *)data;
    if (!is_btn_able(btn_apend)) {
    	return TRUE;
    }

    if (btn_apend->loading) {
    	gtk_widget_show(btn_apend->loading);
    	if (btn_apend->label) {
    		gtk_widget_hide(btn_apend->label);
    	}
    }
*/
    l2u_show_password();
    return FALSE;
}


GtkWidget * mk_check_prompt(char * text, int width, int height, void * data)
{
	GtkWidget *hbox;
	GtkWidget * align_begin;
	GtkWidget * align_end;
	//GtkWidget * event_label;

	checkbox = gtk_check_button_new_with_label(text);
    gtk_widget_set_size_request(checkbox, width, height);
    // gtk_toggle_button_set_active(checkbox,TRUE);
    gtk_signal_connect(GTK_OBJECT(checkbox), "clicked", G_CALLBACK(chbtn_click), data);
    gtk_widget_show(checkbox);
    
    //gtk_container_add(GTK_CONTAINER(hbox), image);
    align_begin = gtk_alignment_new(0,0,0,0);
    gtk_container_add(GTK_CONTAINER(align_begin), checkbox);
    gtk_widget_show(align_begin);


    //event_label = mk_event_label("修改密码", G_CALLBACK(change_passwd_handle), NULL);
    btn_modpwd = create_button(NULL, NULL, NULL,"修改密码", -1, -1, G_CALLBACK(change_passwd_handle), NULL);
    align_end = gtk_alignment_new(1,0,0,0);
    gtk_container_add(GTK_CONTAINER(align_end), btn_modpwd->container);
    gtk_widget_show(align_end);

    hbox = gtk_hbox_new(FALSE, 195);
    gtk_box_pack_start(GTK_BOX(hbox), align_begin, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), align_end, FALSE, FALSE, 0);
    gtk_widget_show(hbox);
	return hbox;
}


void get_input(GtkWidget *widget, gpointer buf)
{
	//login_info_t * info;
	//info = (login_info_t *)data;
	
	strcpy((char *)buf, gtk_entry_get_text(GTK_ENTRY(widget)));		
	return;	
	
}

GtkWidget * mk_login_input(GtkWidget *entry, char* icon,  char *text, int width, int height, gboolean visible, void* buf)
{
	GtkWidget * container;
	
    g_signal_connect(G_OBJECT(entry),"activate",G_CALLBACK(user_login_enter), &login_info);
	container = add_entry_style(entry,  icon,  text, width, height, visible);

	g_signal_connect(G_OBJECT(entry),"changed",G_CALLBACK(get_input), buf);

	return container;
}


static GtkWidget * mk_top_title(char * text, int width, int height)
{
	GtkWidget *title;	
	
    title = gtk_label_new(text);
	gtk_label_set_justify(GTK_LABEL(title), GTK_JUSTIFY_CENTER);
	gtk_widget_set_size_request(title, width, height);
	gtk_widget_show(title);
	
	return title;	
}


static void ui_win_user_login_reload()
{
    char username[ENTRY_TEXT_LEN];
    char passwd[ENTRY_TEXT_LEN];
    int flag;

    memset(login_info.password, 0, sizeof(login_info.password));
   	memset(username, 0 , sizeof(username));
   	memset(passwd, 0, sizeof(passwd));
    ui_extern_get_last_logined_user(username, sizeof(username)-1, passwd, sizeof(passwd)-1, &flag);
    
    if (empty_str(login_info.name)) {
    	strncpy(login_info.name, username, sizeof(login_info.name) - 1);
    	//strncpy(login_info.password, passwd, sizeof(login_info.password) - 1);
    }

	if (login_info.remember == -1) {
		login_info.remember = flag;
	}

    gtk_entry_set_text(GTK_ENTRY(user_entry), login_info.name);
    //gtk_entry_set_text(GTK_ENTRY(pass_entry), login_info.password);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), login_info.remember);    
    
}
/*
static gint check_pwd(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
	if (strcmp(gtk_entry_get_text(GTK_ENTRY(widget)), DEF_PWD) == 0) {
		l2u_result_user(7);
	}
    return FALSE;
}
*/
static int ui_win_user_login_init(void)
{
	GtkWidget *title;
	GtkWidget * user_cntner;
	GtkWidget * pass_cntner;
	GtkWidget * checkbox;
	GtkWidget * hbox;
	GtkWidget * align;
	
	GtkWidget * vbox_up;
	GtkWidget * vbox_down;

	GdkPixbuf * pixbuf = NULL;

	if (init == 0) {
		init = 1;
    	memset(&login_info, 0, sizeof(login_info_t));
    	login_info.remember = -1;	
	}

	title = mk_top_title("登录云办公桌面",-1, 40);
	set_widget_font_size(title, 13, NULL);

	user_entry = gtk_entry_new_with_max_length(20);
	pass_entry = gtk_entry_new_with_max_length(32);
	user_cntner =  mk_login_input(user_entry, "./icon/ico_user.png", NULL, -1, -1, TRUE, (void *) (login_info.name));
	
  	pass_cntner =  mk_login_input(pass_entry, "./icon/ico_password.png",  NULL, -1, -1, FALSE, (void *) (login_info.password));
  	//g_signal_connect(G_OBJECT(pass_cntner),"changed",G_CALLBACK(check_pwd), NULL);

    g_signal_connect(G_OBJECT(user_cntner),"focus",G_CALLBACK(ui_win_pop_up_keyboard),NULL);
    g_signal_connect(G_OBJECT(user_cntner),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    g_signal_connect(G_OBJECT(pass_cntner),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    checkbox = mk_check_prompt("记住用户", -1, -1, (void *) (&(login_info.remember)));

    vbox_err = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox_err);
    
    btn_netmode = create_button("./icon/login_btn2.png", "./icon/login_btn2_h.png", "./icon/wait.gif", "恢复在线", 85, 25, G_CALLBACK(back2net_mode), NULL);
	set_widget_font_size(btn_netmode->label, 9, NULL);
    gtk_widget_hide(btn_netmode->container);
    
    
    btn_user = create_button("./icon/btn_green.png", "./icon/btn_green_h.png", "./icon/btn_process.gif", "登录", 200, 40, G_CALLBACK(user_login), (void *) (&login_info));
    set_widget_font_size(btn_user->label, 13, "Snow");
    
	btn_guest = create_button("./icon/login_btn2.png", "./icon/login_btn2_h.png", "./icon/wait.gif", "访客登录", 85, 25, G_CALLBACK(guest_login), (void *) (&login_info));
	set_widget_font_size(btn_guest->label, 9, NULL);
	
	hbox =  gtk_hbox_new(FALSE, 10);
    gtk_box_pack_start(GTK_BOX (hbox), btn_guest->container, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX (hbox), btn_netmode->container, TRUE, FALSE, 0);
 	gtk_widget_show(hbox);

    align = gtk_alignment_new(0.5,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);
	
    vbox_up = gtk_vbox_new(FALSE, 10);
    gtk_box_pack_start(GTK_BOX (vbox_up), title, TRUE, FALSE, 3);
    gtk_box_pack_start(GTK_BOX (vbox_up), user_cntner, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX (vbox_up), pass_cntner, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX (vbox_up), checkbox, TRUE, FALSE, 0);
    gtk_widget_show(vbox_up);

    vbox_down = gtk_vbox_new(FALSE, 10);
    gtk_box_pack_start(GTK_BOX (vbox_down), vbox_err, TRUE, FALSE, 0);
    //gtk_box_pack_start(GTK_BOX (vbox_down), btn_netmode->container, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX (vbox_down), btn_user->container, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX (vbox_down), align, TRUE, FALSE, 0);
    gtk_widget_show(vbox_down);

	vbox_all = gtk_vbox_new(FALSE, 10);

    g_signal_connect (G_OBJECT (vbox_all), "realize",
                      G_CALLBACK (ui_compt_realize), &ui_win_user_login);

	gtk_box_pack_start(GTK_BOX (vbox_all), vbox_up, TRUE, FALSE, 10);
	gtk_box_pack_start(GTK_BOX (vbox_all), vbox_down, TRUE, FALSE, 0);
    //gtk_window_set_default_size(GTK_WINDOW(vbox_all), 380, 415);
	gtk_widget_set_size_request(vbox_all, 325, 400);
    if (bg_image == NULL) {
        pixbuf = gdk_pixbuf_new_from_file("./icon/login_box.png", NULL);
        width = gdk_pixbuf_get_width(pixbuf);
        bg_image = gtk_image_new_from_pixbuf(pixbuf);
        g_object_unref(pixbuf);
        gtk_widget_hide(bg_image);
        ui_win_put_nice_position(bg_image, 0, width, 25);
    }
    //update_widget_bg(GTK_WINDOW(vbox_all), "./icon/login_box.png");
    ui_win_set_pos(vbox_all, 0, 0);

    gtk_window_set_focus(GTK_WINDOW(g_win_manager.window), user_cntner);
    ui_win_pop_up_keyboard();
    ui_win_user_login_reload();

    if(ui_get_hide_guest_login())
    {
        gtk_widget_hide(btn_guest->container);
    }


    return 0;
} 


static void ui_win_user_login_show(void)
{
    // restart all
    if (vbox_all) {
        gtk_widget_destroy(vbox_all);
        vbox_all = NULL;
        err_prompt = NULL;
    }
    ui_win_user_login_init();
    gtk_widget_show(bg_image);
    gtk_widget_show(vbox_all);

    if(ui_get_hide_guest_login())
    {
        gtk_widget_hide(btn_guest->container);
    }
}

void do_login_err(void)
{
	gtk_widget_show(err_prompt);
	btn_normal_show(btn_user);
    if(!ui_get_hide_guest_login())
    {
	    btn_normal_show(btn_guest);
    }
}


static int ui_win_user_login_ctrl(void *data)
{
	ui_msg_t *msg = data;

    if (msg->sub_obj == UI_WIN_USER_LOGIN_HIDE_MODPWD) {

        gtk_widget_hide(btn_modpwd->container);
        gtk_widget_show(btn_netmode->container);

        return 0;
    }

    if(ui_get_hide_guest_login())
    {
        gtk_widget_hide(btn_guest->container);
    }

    if (msg->sub_obj == UI_WIN_USER_LOGIN_HIDE_GUEST) {

        gtk_widget_hide(btn_guest->container);

        return 0;
    }

    if (err_prompt) {
        gtk_widget_destroy(err_prompt);
    }
	switch (msg->sub_obj) {
	case UI_WIN_USER_LOGIN_RET_ERROR:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "用户名或密码错误,请重新登录", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        break;
	case UI_WIN_USER_LOGIN_RET_EUSER:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "该用户没有开启IDV权限", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        break;        
	case UI_WIN_USER_LOGIN_RET_EUNKNOW:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "登录异常", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        break;          
	case UI_WIN_USER_LOGIN_RET_NOIMAGE:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "没有镜像！无法使用访客模式", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        break;
	case UI_WIN_USER_LOGIN_RET_BIND_ERR:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "用户绑定失败", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        break;
	case UI_WIN_USER_LOGIN_RET_NOBIND:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "终端未绑定！无法使用访客模式", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        break;
	case UI_WIN_USER_LOGIN_DEF_PWD_WARN:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "无法使用默认密码登录，请先修改密码", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        break;
    case UI_WIN_USER_LOGIN_RET_USER_BINDED:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "用户已在其他终端绑定，无法再绑定", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        break;
    case UI_WIN_USER_LOGIN_RET_TIMEOUT:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "连接云桌面超时，请稍候重试", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        break;        
    case UI_WIN_AD_LOGIN_RET_DOMAIN_EXCEPTION:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "AD域服务器异常，请管理员检测AD域服务器", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        break;
    case UI_WIN_AD_LOGIN_RET_USER_DISABLE:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "您的账户已被停用，请向系统管理员咨询", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        break;
    case UI_WIN_AD_LOGIN_RET_USER_EXPIRE:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "用户账户已过期", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        break;
    case UI_WIN_AD_LOGIN_RET_USER_NOT_AUTH_RIGHT_NOW:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "您的账户有时间限制，当前无法登录", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        break;
    case UI_WIN_LDAP_LOGIN_RET_ERROR:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "LDAP服务器异常，请联系管理员", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        break;
    }
    able_btn_widget();
    do_login_err();

	return 0;
}

static void ui_win_user_login_hide (void)
{
    gtk_widget_hide(bg_image);
    gtk_widget_hide(vbox_all);
}

static void ui_win_user_login_adapt(void)
{
    if (bg_image) {
        ui_win_put_nice_position(bg_image, 1, width, 25);
    }

    if (vbox_all) {
        ui_win_user_login.connect_id = g_signal_connect(G_OBJECT (vbox_all), "size-allocate", 
                                            G_CALLBACK(ui_win_put_nice_position_cb), &ui_win_user_login);
    }

    if(ui_win_user_login.status == UI_STATUS_SHOW) {
        if (bg_image && vbox_all) {
            gtk_widget_hide(bg_image);
            gtk_widget_hide(vbox_all);
            gtk_widget_show(bg_image);
            gtk_widget_show(vbox_all);
        }
    }
}

static void ui_win_user_login_destroy(void)
{
    if (bg_image) {
        gtk_widget_destroy(bg_image);
        bg_image = NULL;
    }

    if (vbox_all) {
        gtk_widget_destroy(vbox_all);
        vbox_all = NULL;
    }
}

struct ui_comp_s ui_win_user_login = {
    .type = UI_TYPE_WIN_USER_LOGIN,
    .subtype = UI_TOPLEVEL_TYPE_WIN,
    .is_init = UI_NEED_INIT,
    .widget = &vbox_all,
    .init = ui_win_user_login_init,
    .show = ui_win_user_login_show,
    .hide = ui_win_user_login_hide,
    .ctrl = ui_win_user_login_ctrl,
    .adapt = ui_win_user_login_adapt,
    .destroy = ui_win_user_login_destroy,
    .height = 25,
};


