#include <gtk/gtk.h>
#include <string.h>
#include "ui_main.h"
#include "ui_test.h"
#ifdef IDV_CLIENT
#include "rc/rc_checknetifval.h"
#endif

#define	ENTRY_TEXT_LEN	20


static GtkWidget *vbox_all;
static GtkWidget * bg_image;

static GtkWidget *user_entry;
static GtkWidget *oldpwd_entry;
static GtkWidget *newpwd_entry;
static GtkWidget *confirm_pwd_entry;

static GtkWidget * default_pwd_tip;
static GtkWidget * err_prompt;
static btn_apend_t * btn_sure;
static btn_apend_t * btn_cancel;
static GtkWidget * vbox_err;
static GtkWidget * passwd_level_lable;
static int width;

/*
void disable_btn_widget()
{
	set_btn_effect(btn_user, FALSE);
	set_btn_effect(btn_guest, FALSE);
}

void able_btn_widget()
{
	set_btn_effect(btn_user, TRUE);
	set_btn_effect(btn_guest, TRUE);
}
*/

GtkWidget * mk_pass_input(GtkWidget *entry, GtkWidget *tips, char *path, gboolean visible, char *text, int width, int height)
{
	GtkWidget * hbox;
	
	GtkWidget * label;
	GtkWidget * align;

	add_entry_style(entry, path, NULL, width, height, visible);
	
	//gtk_entry_set_icon_tooltip_text(GTK_ENTRY(entry), GTK_ENTRY_ICON_PRIMARY, text);
	label = gtk_label_new(text);
	set_widget_font_size(label, 12, NULL);
    gtk_widget_set_size_request(label, 65, height);
    gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
	gtk_widget_show(label);
	
	gtk_widget_show(entry);

    hbox = gtk_hbox_new(FALSE, 3);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
    if (tips) {
        gtk_box_pack_start(GTK_BOX(hbox), tips, FALSE, FALSE, 0);
        gtk_widget_show(tips);
    }
    //gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 5);
    gtk_widget_show(hbox);
    
    align = gtk_alignment_new(0,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);    
    
    return align;
}

static int check_passwd_level(char *passwd)
{
    int i = 0, ret = 0;
    int check_sum = 0, is_num = 0, is_lower = 0, is_upper = 0, is_special = 0;

    if (passwd == NULL) {
        return PASSWD_LEVEL_ERR;
    }

    if (strlen(passwd) == 0) {
        return PASSWD_LEVEL_NONE;
    }
    for (i = 0; i < strlen(passwd); i++) {
        if ('0' <= passwd[i] && passwd[i] <= '9') {
            is_num = 1;
        } else if ('a' <= passwd[i] && passwd[i] <= 'z') {
            is_lower = 1;
        } else if ('A' <= passwd[i] && passwd[i] <= 'Z') {
            is_upper = 1;
        } else {
            is_special = 1;
        }
    }

    check_sum = is_num + is_lower + is_upper + is_special;
    switch (check_sum) {
    case PASSWD_LEVEL_WEAK:
        ret = PASSWD_LEVEL_WEAK;
        break;
    case PASSWD_LEVEL_MIDIUM:
        ret = PASSWD_LEVEL_MIDIUM;
        break;
    case PASSWD_LEVEL_STRONG:
    default:
        ret = PASSWD_LEVEL_STRONG;
        break;
    }
    return ret;
}

static gboolean on_changed_confirm_password_entry(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    char passwd[128] = {0};
    int level = PASSWD_LEVEL_WEAK;

    if (passwd_level_lable == NULL) {
        return TRUE;
    }

    strcpy(passwd, gtk_entry_get_text(GTK_ENTRY(widget)));

    level = check_passwd_level(passwd);
    switch (level) {
    case PASSWD_LEVEL_ERR:
        return TRUE;
    case PASSWD_LEVEL_NONE:
        gtk_widget_hide(passwd_level_lable);
        return TRUE;
    case PASSWD_LEVEL_WEAK:
        gtk_label_set_text (GTK_LABEL(passwd_level_lable), "弱");
        set_widget_font_size(passwd_level_lable, 8, "red");
        break;
    case PASSWD_LEVEL_MIDIUM:
        gtk_label_set_text (GTK_LABEL(passwd_level_lable), "中");
        set_widget_font_size(passwd_level_lable, 8, "orange");
        break;
    case PASSWD_LEVEL_STRONG:
        gtk_label_set_text (GTK_LABEL(passwd_level_lable), "强");
        set_widget_font_size(passwd_level_lable, 8, "green");
        break;
    }

    gtk_misc_set_alignment(GTK_MISC(passwd_level_lable), 0, 0.5);
    gtk_widget_show(passwd_level_lable);
    return TRUE;
}

static GtkWidget * mk_top_title(char * text, char * color_str, int font_size)
{
	GtkWidget *title;	
	
    title = gtk_label_new(text);
	gtk_label_set_justify(GTK_LABEL(title), GTK_JUSTIFY_CENTER);
	//gtk_widget_set_size_request(title, width, height);
	set_widget_font_size(title, font_size, color_str);
	gtk_widget_show(title);
	
	return title;	
}

static gint ui_win_password_event_cancal(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
	l2u_show_userlogin();
	return FALSE;
}

static void disable_btn_widget()
{
	set_btn_effect(btn_sure, FALSE);
	set_btn_effect(btn_cancel, FALSE);
}

static void able_btn_widget()
{
	set_btn_effect(btn_sure, TRUE);
	set_btn_effect(btn_cancel, TRUE);
	btn_normal_show(btn_sure);
}

static gint ui_win_password_event_ok(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    disable_btn_widget();

    if (strcmp (gtk_entry_get_text(GTK_ENTRY(user_entry)) , "") == 0)
    {
        //username empty
        l2u_result_user(105);
        return FALSE;
    }
    
    if (strcmp (gtk_entry_get_text(GTK_ENTRY(oldpwd_entry)) , "") == 0)
    {
        //old password empty
        l2u_result_user(107);
        return FALSE;
    }

    if (strcmp (gtk_entry_get_text(GTK_ENTRY(newpwd_entry)) , gtk_entry_get_text(GTK_ENTRY(confirm_pwd_entry))) != 0) {
        //NOT MATCH
        l2u_result_user(100);
        return FALSE;
    }

    if ((strcmp (gtk_entry_get_text(GTK_ENTRY(newpwd_entry)) , "") == 0) || (strcmp (gtk_entry_get_text(GTK_ENTRY(confirm_pwd_entry)) , "") == 0))
    {
        //new password empty
        l2u_result_user(106);
        return FALSE;
    }

    if ((strcmp (gtk_entry_get_text(GTK_ENTRY(newpwd_entry)) , gtk_entry_get_text(GTK_ENTRY(oldpwd_entry))) == 0))
    {
        //password the same
        l2u_result_user(103);
        return FALSE;
    }

    if ((strcmp (gtk_entry_get_text(GTK_ENTRY(newpwd_entry)) , "123456") == 0))
    {
        //new passwd is default passwd
        l2u_result_user(110);
        return FALSE;
    }

    if (rc_check_password(gtk_entry_get_text(GTK_ENTRY(newpwd_entry)), 32) != 0)
    {
        //password contains special character
        l2u_result_user(104);
        return FALSE;
    }

    ui_extern_modify_passwd(gtk_entry_get_text(GTK_ENTRY(user_entry)), 
                            gtk_entry_get_text(GTK_ENTRY(oldpwd_entry)), 
                            gtk_entry_get_text(GTK_ENTRY(newpwd_entry)));

	return FALSE;
} 

GtkWidget * mk_acttion_area(void)
{
	GtkWidget *align;
	GtkWidget *hbox;
    btn_sure = create_button("./icon/btn_green.png", "./icon/btn_green_h.png", "./icon/btn_process.gif", "确认", -1, -1, G_CALLBACK(ui_win_password_event_ok), NULL);
    set_widget_font_size(btn_sure->label, 12, "Snow");
	btn_cancel = create_button("./icon/btn_gray.png", "./icon/btn_gray_h.png", NULL, "取消", -1, -1, G_CALLBACK(ui_win_password_event_cancal), NULL);
	set_widget_font_size(btn_cancel->label, 12, NULL);
	
	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(hbox), btn_sure->container, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), btn_cancel->container, FALSE, FALSE, 0);
	gtk_widget_show(hbox);
	
	align = gtk_alignment_new(0.5,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);
    able_btn_widget();
	return align;
}

static int ui_win_password_init(void)
{
    GtkWidget *title;
    GtkWidget * user_name;
    GtkWidget * old_pass;
    GtkWidget * new_pass;
    GtkWidget * renew_pass;

    GtkWidget * btn;
    GtkWidget * vbox_up;
    GtkWidget * vbox_down;
    GdkPixbuf * pixbuf = NULL;
    passwd_level_lable = NULL;

    title = mk_top_title("修改密码", NULL, 13);

    user_entry = gtk_entry_new_with_max_length(20);
    oldpwd_entry = gtk_entry_new_with_max_length(32);
    newpwd_entry = gtk_entry_new_with_max_length(32);
    confirm_pwd_entry = gtk_entry_new_with_max_length(32);

    passwd_level_lable = gtk_label_new(" ");
    gtk_widget_set_size_request(passwd_level_lable, 30, 33);

    g_signal_connect(G_OBJECT(user_entry),"focus",G_CALLBACK(ui_win_pop_up_keyboard),NULL);
    g_signal_connect(G_OBJECT(user_entry),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    g_signal_connect(G_OBJECT(oldpwd_entry),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    g_signal_connect(G_OBJECT(newpwd_entry),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    g_signal_connect(G_OBJECT(confirm_pwd_entry),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    
    user_name = mk_pass_input(user_entry, NULL, NULL, TRUE, "用户名", 240, 33);
    old_pass = mk_pass_input(oldpwd_entry, NULL, NULL, FALSE, "原密码", 240, 33);
    renew_pass = mk_pass_input(confirm_pwd_entry, NULL, NULL, FALSE, "确认密码", 240, 33);
    new_pass = mk_pass_input(newpwd_entry, passwd_level_lable, NULL, FALSE, "新密码", 240, 33);

    default_pwd_tip = gtk_label_new("初始密码为：123456");

    vbox_err = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX (vbox_err), default_pwd_tip, FALSE, FALSE, 0);
    gtk_widget_show(default_pwd_tip);
    gtk_widget_show(vbox_err);

    btn = mk_acttion_area();

    vbox_up = gtk_vbox_new(FALSE, 10);
    gtk_box_pack_start(GTK_BOX (vbox_up), title, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX (vbox_up), user_name, FALSE, FALSE, 3);
    gtk_box_pack_start(GTK_BOX (vbox_up), old_pass, FALSE, FALSE, 3);
    gtk_box_pack_start(GTK_BOX (vbox_up), new_pass, FALSE, FALSE, 3);
    gtk_box_pack_start(GTK_BOX (vbox_up), renew_pass, FALSE, FALSE, 3);
    gtk_misc_set_alignment(GTK_MISC(vbox_up), 0, 0);
    gtk_widget_show(vbox_up);

    vbox_down = gtk_vbox_new(FALSE, 25);
    gtk_box_pack_start(GTK_BOX (vbox_down), vbox_err, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX (vbox_down), btn, FALSE, FALSE, 0);

    gtk_widget_show(vbox_down);

	vbox_all = gtk_vbox_new(FALSE, 10);

    g_signal_connect(GTK_ENTRY(newpwd_entry), "changed",
                        G_CALLBACK(on_changed_confirm_password_entry), NULL);

    g_signal_connect (G_OBJECT (vbox_all), "realize",
                      G_CALLBACK (ui_compt_realize), &ui_win_password);

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
    ui_win_set_pos(vbox_all, 0, 0);
    gtk_window_set_focus(GTK_WINDOW(g_win_manager.window), user_entry);

    return 0;
} 


static void ui_win_password_show(void)
{
    if (vbox_all) {
        gtk_widget_destroy(vbox_all);
        vbox_all = NULL;
        err_prompt = NULL;
    }
    ui_win_password_init();
	gtk_widget_show(bg_image);
    gtk_widget_show(vbox_all);

}

static int ui_win_password_ctrl(void *data)
{
	ui_msg_t *msg = data;

    if (default_pwd_tip) {
        gtk_widget_destroy(default_pwd_tip);
        default_pwd_tip = NULL;
    }
    if (err_prompt) {
        gtk_widget_destroy(err_prompt);
        err_prompt = NULL;
    }
	switch (msg->sub_obj) {
    case UI_WIN_PASSWD_RET_OK:
        err_prompt = create_err_prompt("./icon/ico_correct32.png", "密码修改成功", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        break;
    case UI_WIN_PASSWD_RET_ERR:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "用户名或密码错误", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        able_btn_widget();
        break;
	case UI_WIN_PASSWD_RET_OLDPWD_NO_CORRECT:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "原密码不正确", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        able_btn_widget();
        break;
	case UI_WIN_PASSWD_RET_NEWPWD_NO_MATCH:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "新密码与确认密码不匹配", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        able_btn_widget();
        break;
	case UI_WIN_PASSWD_RET_PWD_NO_CHANGE:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "旧密码和新密码一致", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        able_btn_widget();
        break;
    case UI_WIN_PASSWD_RET_PWD_HAS_SPECIAL_CHAR:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "密码含有特殊字符", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        able_btn_widget();
        break;
    case UI_WIN_PASSWD_RET_USER_EMPTY:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "请输入用户名", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        able_btn_widget();
        break;
    case UI_WIN_PASSWD_RET_NEWPWD_EMPTY:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "请输入修改后的密码", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        able_btn_widget();
        break;
    case UI_WIN_PASSWD_RET_OLDPWD_EMPTY:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "请输入原密码", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        able_btn_widget();
        break;
    case UI_WIN_PASSWD_RET_ERR_AD_DOMAIN:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "AD域用户不支持修改密码", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        able_btn_widget();
        break;
    case UI_WIN_PASSWD_RET_ERR_LDAP_USER:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "LDAP用户不支持修改密码", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        able_btn_widget();
        break;
    case UI_WIN_PASSWD_RET_ERR_DEFAULT_PASSWD:
        err_prompt = create_err_prompt("./icon/ico_abnormal_mm.png", "新密码不能是初始密码123456", -1, -1);
        gtk_box_pack_start(GTK_BOX (vbox_err), err_prompt, FALSE, FALSE, 0);
        able_btn_widget();
        break;
	}

	gtk_widget_show(err_prompt);
	


	return 0;
}


static void ui_win_password_hide (void)
{
    gtk_widget_hide(bg_image);
    gtk_widget_hide(vbox_all);
}

static void ui_win_password_adapt(void)
{
    if (bg_image) {
        ui_win_put_nice_position(bg_image, 1, width, 25);
    }

    if (vbox_all) {
        ui_win_password.connect_id = g_signal_connect(G_OBJECT (vbox_all), "size-allocate", 
                                                G_CALLBACK(ui_win_put_nice_position_cb), &ui_win_password);
    }

    if( ui_win_password.status == UI_STATUS_SHOW) {
        if (bg_image && vbox_all) {
            gtk_widget_hide(bg_image);
            gtk_widget_hide(vbox_all);
            gtk_widget_show(bg_image);
            gtk_widget_show(vbox_all);
        }
    }
    
}

static void ui_win_password_destroy(void)
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

struct ui_comp_s ui_win_password = {
    .type = UI_TYPE_WIN_PASSWORD,
    .subtype = UI_TOPLEVEL_TYPE_WIN,
    .is_init = UI_NEED_INIT,
    .widget = &vbox_all,
    .init = ui_win_password_init,
    .show = ui_win_password_show,
    .hide = ui_win_password_hide,
    .ctrl = ui_win_password_ctrl,
    .adapt = ui_win_password_adapt,
    .destroy = ui_win_password_destroy,
    .height = 25,
    //.ctrl = ui_win_user_login_ctrl,
};


