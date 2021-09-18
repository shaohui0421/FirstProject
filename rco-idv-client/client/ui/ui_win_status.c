#include "ui_main.h"

static int mode;
static int val;
static gchar *note;
static GtkWidget *hbox;

/*
static gint dev_update_retry(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
	ui_extern_tips_confirm(9);
    return FALSE;
}
*/

static gint ui_win_show_config(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    ui_tab[UI_TYPE_DIALOG_ADMINPWD]->show();
	return FALSE;
}

static gint ui_win_show_upgrade(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    ui_tab[UI_TYPE_DIALOG_ADMINPWD_UPGRADE]->show();
    return FALSE;
}

/*
static gint ui_win_status_connecting_cancal(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    ui_extern_giveup_connect();
    ui_manager_config_enable();
    return FALSE;
}
*/

static gint ui_win_status_disconnect_reconnect(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    ui_extern_reconnect();
    return FALSE;
}

static gint ui_win_status_disconnect_localmode(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    ui_extern_local_mode();
    return FALSE;
}

static gint ui_win_status_client_tips(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    int *p = data;
    
    ui_extern_tips_confirm((ulong)p);

    return FALSE;
}


static gint ui_win_status_enter_local_mode(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
	ui_extern_enter_local_mode();
    return FALSE;
}

static GtkWidget * mk_disconnect_widget(void)
{
	GtkWidget * vbox;
	GtkWidget *hbox;
	GtkWidget * align;
	GtkWidget * prompt;
	btn_apend_t * net_login;
	btn_apend_t * local_login;
	
	//label = gtk_label_new("");
	
	prompt = create_prompt("./icon/ico_abnormal.png", "网络连接断开！", 25, NULL, NULL, NULL);
	
	net_login = create_button("./icon/btn_setup.png", "./icon/btn_setup_h.png", NULL, "重新连接",
							 -1, -1, G_CALLBACK(ui_win_status_disconnect_reconnect), NULL);
	set_widget_font_size(net_login->label, 12, "Snow");
	
	local_login = create_button("./icon/btn_setup.png", "./icon/btn_setup_h.png", NULL, "脱网登录",
							 -1, -1, G_CALLBACK(ui_win_status_disconnect_localmode), NULL);	
	set_widget_font_size(local_login->label, 12, "Snow");
							 
	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(hbox), net_login->container, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox), local_login->container, FALSE, FALSE, 5);
	gtk_widget_show(hbox);
	
	align = gtk_alignment_new(0.5,0.5,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);
    
    	
	vbox = gtk_vbox_new(FALSE, 40);
	gtk_box_pack_start(GTK_BOX(vbox), prompt, FALSE, FALSE, 10);
	gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 5);
	gtk_widget_show(vbox);	
	return vbox;
}

static GtkWidget * mk_sever_maintain_win(char *icon, char *label_text, int lab_font_size)
{
    GtkWidget * hbox = NULL;
    GtkWidget * align = NULL;
    GtkWidget * prompt = NULL;
    btn_apend_t * retry_btn;
    btn_apend_t * local_login_btn;

    prompt = create_prompt(icon, label_text, lab_font_size, NULL, NULL, NULL);

    local_login_btn = create_button("./icon/btn_setup.png", "./icon/btn_setup_h.png", NULL, "脱网登录",
        -                           1, -1, G_CALLBACK(ui_win_status_enter_local_mode), NULL);
    set_widget_font_size(local_login_btn->label, 14, "Snow");

    retry_btn = create_button("./icon/btn_setup.png", "./icon/btn_setup_h.png", NULL, "重试",
                             -1, -1, G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_DEV_SERVER_MAINTAIN_RET);
    set_widget_font_size(retry_btn->label, 14, "Snow");


    hbox = gtk_hbox_new(FALSE, 10);
    gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(hbox), local_login_btn->container, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), retry_btn->container, FALSE, FALSE, 5);
    gtk_widget_show(hbox);

    align = gtk_alignment_new(0.5,0.5,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

    return align;
}

static GtkWidget *  align_center(GtkWidget * widget)
{
	GtkWidget * align;

	align = gtk_alignment_new(0.5,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), widget);
    gtk_widget_show(align);
    return align;
}

static  GtkWidget * mk_net_disconect_prompt(char *icon, int lab_font_size, char * btn_text, GCallback callback, void* data)
{
    GtkWidget * line1;
    GtkWidget * line2;

    GtkWidget * line1_hbox;
    GtkWidget * line2_hbox;
    GtkWidget * vbox;
    btn_apend_t * btn_cnter = NULL;
    GtkWidget * image = NULL;
    GdkPixbuf *src_pixbuf = NULL;

    src_pixbuf = gdk_pixbuf_new_from_file(icon, NULL);
    image = gtk_image_new_from_pixbuf(src_pixbuf);
    g_object_unref(src_pixbuf);
    gtk_widget_show(image);

    //, "无法连接云主机，请检查网线是否松脱、网络是否正常。也可选择脱网登录"

    line1 = gtk_label_new("无法连接云主机，请检查网线是否松脱、网络是否正常。");
    set_widget_font_size(line1, lab_font_size, "Snow");
    gtk_widget_show(line1);

    line1_hbox = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(line1_hbox), image, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(line1_hbox), line1, FALSE, FALSE, 8);
    gtk_widget_show(line1_hbox);


    line2 = gtk_label_new("即将脱网登录....");
    set_widget_font_size(line2, lab_font_size, "Snow");
    gtk_widget_show(line2);

    btn_cnter = create_button("./icon/btn_setup.png", "./icon/btn_setup_h.png", NULL, btn_text, -1, -1, (GCallback)callback, data);
    set_widget_font_size(btn_cnter->label, 14, "Snow");

    line2_hbox = gtk_hbox_new(FALSE, 15);
    gtk_box_pack_start(GTK_BOX(line2_hbox), line2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(line2_hbox), btn_cnter->container, FALSE, FALSE, 0);
    gtk_widget_show(line2_hbox);

    vbox = gtk_vbox_new(FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), align_center(line1_hbox), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), align_center(line2_hbox), FALSE, FALSE, 0);
    gtk_widget_show(vbox);
    return vbox;
}
#if 0
static void _dev_locked_tips(const int lock_type, char *tips)
{
    int diff_type[] = {UI_DEV_LOCK_DIFF_MODE, UI_DEV_LOCK_DIFF_BINDUSER, UI_DEV_LOCK_DIFF_RECOVERY, UI_DEV_LOCK_DIFF_USERDISK};
    char* diff_str[] = {"终端模式", "绑定用户", "桌面模式", "并开启本地盘"};
    int lock_sync_type = 0;
    int i, count;

    memset(tips, 0, sizeof(tips));
    if (lock_type == UI_DEV_LOCK_DIFF_USERDISK)
    {
        strcpy(tips, "开启本地盘");
        return;
    }

    count = ARRAY_SIZE(diff_type);
    for (i = 0; i < count; i++)
    {
        if ((lock_type & diff_type[i]) != 0)
        {
            strcat(tips, diff_str[i]);
            lock_sync_type |= diff_type[i];
            if (lock_type != lock_sync_type)
            {
                strcat(tips, "，");
            }
            else
            {
                strcat(tips, "。");
                break;
            }
        }
    }
    sprintf(tips, "设置正确的%s", tips);
}
#endif
const static char* _dev_locked_tips_line0[] = {
    "初始化终端（将清除用户数据）。",              // lock_type : 1-15
    "确认终端配置的云主机地址是否正确。", //  lock_type::16
};

const static char* _dev_locked_tips_line1[] = {
    "",
    "设置正确的终端模式。",
    "设置正确的绑定用户。",
    "设置正确的终端模式和绑定用户。",
    "设置正确的桌面模式，并重启终端。",
    "设置正确的终端模式和桌面模式，并重启终端。",
    "设置正确的绑定用户和桌面模式，并重启终端。",
    "设置正确的终端模式，绑定用户和桌面模式，",
    "开启本地盘，并重启终端。",
    "设置正确的终端模式，开启本地盘，并重启终端。",
    "设置正确的绑定用户，开启本地盘，并重启终端。",
    "设置正确的终端模式，绑定用户，开启本地盘，",
    "设置正确的桌面模式，开启本地盘，并重启终端。",
    "设置正确的终端模式，桌面模式，开启本地盘，",
    "设置正确的绑定用户，桌面模式，开启本地盘，",
    "设置正确的终端模式，绑定用户，桌面模式，",
    "初始化终端（将清除用户数据）。",
};
const static char* _dev_locked_tips_line2[] = {
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "并重启终端。",
    "",
    "",
    "",
    "并重启终端。",
    "",
    "并重启终端。",
    "并重启终端。",
    "开启本地盘，并重启终端。",
    "",
};
static  GtkWidget * mk_dev_locked_prompt(char *icon, int lock_type, char * btn_text, GCallback callback, void* data)
{
	GtkWidget * line1;
	GtkWidget * line2;
	GtkWidget * line3;
	GtkWidget * line4;
	GtkWidget * line5;
	GtkWidget * line6;

	GtkWidget * line1_hbox;
	GtkWidget * line2_hbox;
	GtkWidget * line3_hbox;
	GtkWidget * line4_hbox;
	GtkWidget * line5_hbox;
	GtkWidget * line6_hbox;
	GtkWidget * vbox;
	btn_apend_t * btn_cnter = NULL;
	GtkWidget * image = NULL;
	GdkPixbuf *src_pixbuf = NULL;

    const int font_size1 = 25;
    const int font_size2 = 19;

    char buf[256] = {0};
    char line1_buf[64];

    if(lock_type <=UI_DEV_LOCK_NONE || lock_type >=UI_DEV_LOCK_MAX) {
        logi("show dev locked err!!\n");
        return NULL;
    }

	src_pixbuf = gdk_pixbuf_new_from_file(icon, NULL);
	image = gtk_image_new_from_pixbuf(src_pixbuf);
	g_object_unref(src_pixbuf);
	gtk_widget_show(image);

    memset(line1_buf, 0, sizeof(line1_buf));
    if ((lock_type&UI_DEV_LOCK_LAYER_DOWNGRADE) == 0) {
        if ((lock_type&UI_DEV_LOCK_DIFF_MODE)!=0) {
            sprintf(line1_buf, "终端模式");
        } else if ((lock_type&UI_DEV_LOCK_DIFF_BINDUSER)!=0) {
            sprintf(line1_buf, "绑定用户");
        } else if ((lock_type&UI_DEV_LOCK_DIFF_RECOVERY)!=0) {
            sprintf(line1_buf, "桌面模式");
        } else if ((lock_type&UI_DEV_LOCK_DIFF_USERDISK)!=0) {
            sprintf(line1_buf, "本地盘设置");
        }
        //memset(buf, 0, sizeof(buf));
        sprintf(buf, "服务器未准备就绪，检查到%s已发生变更", line1_buf);
    } else {
        strcpy(buf, "应用集中更新模式下，强制降级将导致用户数据丢失。");
    }

    line1 = gtk_label_new(buf);
    set_widget_font_size(line1, font_size1, "Snow");
    gtk_widget_show(line1);

    btn_cnter = create_button("./icon/btn_setup.png", "./icon/btn_setup_h.png", NULL, btn_text, -1, -1, (GCallback)callback, data);
    set_widget_font_size(btn_cnter->label, 14, "Snow");

    line1_hbox = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(line1_hbox), image, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(line1_hbox), line1, FALSE, FALSE, 8);
    gtk_box_pack_start(GTK_BOX(line1_hbox), btn_cnter->container, FALSE, FALSE, 0);
    gtk_widget_show(line1_hbox);

    line2 = gtk_label_new("请尝试以下操作：");
    set_widget_font_size(line2, font_size2, "Snow");
    gtk_widget_show(line2);

    line2_hbox = gtk_hbox_new(FALSE, 15);
    gtk_box_pack_start(GTK_BOX(line2_hbox), line2, FALSE, FALSE, 113);
    gtk_widget_show(line2_hbox);

    line3 = gtk_label_new("1）脱网登录。");
    set_widget_font_size(line3, font_size2, "Snow");
    gtk_widget_show(line3);

    line3_hbox = gtk_hbox_new(FALSE, 15);
    gtk_box_pack_start(GTK_BOX(line3_hbox), line3, FALSE, FALSE, 113);
    gtk_widget_show(line3_hbox);

    sprintf(buf, "2）联系管理员，%s", _dev_locked_tips_line0[lock_type/(UI_DEV_LOCK_LAYER_DOWNGRADE-1)]);
    line4 = gtk_label_new(buf);
    set_widget_font_size(line4, font_size2, "Snow");
    gtk_widget_show(line4);

    line4_hbox = gtk_hbox_new(FALSE, 15);
    gtk_box_pack_start(GTK_BOX(line4_hbox), line4, FALSE, FALSE, 113);
    gtk_widget_show(line4_hbox);

    //memset(buf, 0, sizeof(buf));
    sprintf(buf, "3）联系管理员，%s", _dev_locked_tips_line1[lock_type]);
    line5 = gtk_label_new(buf);
    set_widget_font_size(line5, font_size2, "Snow");
    gtk_widget_show(line5);

    line5_hbox = gtk_hbox_new(FALSE, 15);
    gtk_box_pack_start(GTK_BOX(line5_hbox), line5, FALSE, FALSE, 113);
    gtk_widget_show(line5_hbox);

    //memset(buf, 0, sizeof(buf));
    strcpy(buf, _dev_locked_tips_line2[lock_type]);
    line6 = gtk_label_new(buf);
    set_widget_font_size(line6, font_size2, "Snow");
    gtk_widget_show(line6);

    line6_hbox = gtk_hbox_new(FALSE, 15);
    gtk_box_pack_start(GTK_BOX(line6_hbox), line6, FALSE, FALSE, 152);
    gtk_widget_show(line6_hbox);

    vbox = gtk_vbox_new(FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), line1_hbox, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), line2_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), line3_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), line4_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), line5_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), line6_hbox, FALSE, FALSE, 0);
    gtk_widget_show(vbox);
    return vbox;
}

static  GtkWidget * mk_start_vm_fail_hdmi_err_prompt(char *icon, int lab_font_size, char * btn_text, GCallback callback, void* data)
{
	GtkWidget * line1;
	GtkWidget * line2;

	GtkWidget * line1_hbox;
	GtkWidget * line2_hbox;
	GtkWidget * vbox;
	//btn_apend_t * btn_cnter = NULL;
	GtkWidget * image = NULL;
	GdkPixbuf *src_pixbuf = NULL;
	
	src_pixbuf = gdk_pixbuf_new_from_file(icon, NULL);
	image = gtk_image_new_from_pixbuf(src_pixbuf);
	g_object_unref(src_pixbuf);
	gtk_widget_show(image);

	line1 = gtk_label_new("因本机异常关机，导致云桌面状态异常，请正常关机后，");
	set_widget_font_size(line1, lab_font_size, "Snow");
	gtk_widget_show(line1);

	line1_hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(line1_hbox), image, FALSE, FALSE, 10);
	gtk_box_pack_start(GTK_BOX(line1_hbox), line1, FALSE, FALSE, 8);
	gtk_widget_show(line1_hbox);


	line2 = gtk_label_new("断开电源，再插上电源进行恢复");
	set_widget_font_size(line2, lab_font_size, "Snow");
	gtk_widget_show(line2);

	//btn_cnter = create_button("./icon/btn_setup.png", "./icon/btn_setup_h.png", NULL, btn_text, -1, -1, (GCallback)callback, data);
	//set_widget_font_size(btn_cnter->label, 14, "Snow");

	line2_hbox = gtk_hbox_new(FALSE, 15);
	gtk_box_pack_start(GTK_BOX(line2_hbox), line2, FALSE, FALSE, 0);
    //gtk_box_pack_start(GTK_BOX(line2_hbox), btn_cnter->container, FALSE, FALSE, 0);
	gtk_widget_show(line2_hbox);

	vbox = gtk_vbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(vbox), align_center(line1_hbox), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), align_center(line2_hbox), FALSE, FALSE, 0);
	gtk_widget_show(vbox);
	return vbox;
}

static int ui_win_status_init()
{
	GtkWidget * prompt;

    hbox = gtk_hbox_new(FALSE, 10);
    ui_win_status.height = 31.25;

    g_signal_connect (G_OBJECT (hbox), "realize",
                      G_CALLBACK (ui_compt_realize), &ui_win_status);
    ui_win_btn_all_enable();
    ui_win_wifibtn_enable();
    if (ui_extern_get_wired1xauth_exist() == 0) {
        ui_win_authbtn_enable();
    }
    switch (mode) {
    case UI_WIN_STATUS_UNCONFIG:
        //prompt = mk_prompt_unconfig();
        prompt = create_prompt("./icon/ico_abnormal.png", "还未设置云主机地址，请点击", 27, "设置", (GCallback)ui_win_show_config, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        //ui_win_set_pos(hbox, 1, 30);
        break;
    case UI_WIN_STATUS_CONNECTING:
    	prompt = create_prompt("./icon/ico_loading.png", "正在连接中...", 27, NULL, NULL/*G_CALLBACK(ui_win_status_connecting_cancal)*/, NULL);//mk_prompt_connecting();
    	gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        //ui_win_set_pos(hbox, 30, 30);
        //g_signal_connect (G_OBJECT (hbox), "show", G_CALLBACK (ui_manager_config_disable), NULL);
        //g_signal_connect (G_OBJECT (hbox), "hide",  G_CALLBACK (ui_manager_config_enable), NULL);
        break;
    case UI_WIN_STATUS_CLIENT_UPDATE:
    	//ui_win_btn_all_disable();
    	prompt = create_prompt("./icon/ico_loading.png", "正在更新终端软件版本，请勿断网断电...", 27, NULL, NULL, NULL);//mk_client_update();
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        //ui_win_set_pos(hbox, 30, 30);
        break;
    case UI_WIN_STATUS_CLIENT_UPDATE_SUCCESS:
    	//ui_win_btn_all_enable();
        prompt = create_prompt("./icon/ico_correct.png", "更新终端成功，稍后重启...", 27, NULL, NULL, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        //ui_win_set_pos(hbox, 25, 30);
        break;
    case UI_WIN_STATUS_CLIENT_UPDATE_SUCCESS_RCC:
        prompt = create_prompt("/usr/share/RCC-Client/teacher/ico_correct.png", "更新终端成功，稍后重启...", 27, NULL, NULL, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_PUBLIC_LOGIN:
        prompt = create_prompt("./icon/ico_correct.png", "即将进入公共桌面...", 27, NULL, NULL, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        //ui_win_set_pos(hbox, 35, 30);
        ui_manager_hold_timer();
        g_signal_connect (G_OBJECT (hbox), "show", G_CALLBACK (ui_manager_publogin_timeout_enable), NULL);
        g_signal_connect (G_OBJECT (hbox), "hide",  G_CALLBACK (ui_manager_publogin_timeout_disable), NULL);
        break;
    case UI_WIN_STATUS_LOGIN:
        prompt = create_prompt("./icon/ico_correct.png", "即将登录云办公桌面...", 27, NULL, NULL, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);        
        break;
    case UI_WIN_STATUS_DISCONNECT:
        prompt = mk_disconnect_widget();
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        //ui_win_set_pos(hbox, 40, 30);
        break;
    case UI_WIN_STATUS_NOCONF_IMAGE:
        prompt = create_prompt("./icon/ico_abnormal.png", "服务器未配置镜像", 27, "确认", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_NOCONF_IMAGE_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_NOCONF_POLICY:
        prompt = create_prompt("./icon/ico_abnormal.png", "服务器未配置策略", 27, "确认", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_NOCONF_POLICY_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_RECOVERY_IMAGE:
        prompt = create_prompt("./icon/ico_abnormal.png", "镜像被还原！", 27, "确认", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_RECOVERY_IMAGE_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_SERVER_ERROR:
        prompt = create_prompt("./icon/ico_abnormal.png", "服务器有异常", 27, "确认", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_SERVER_ERROR_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_CLIENT_INIT:
        prompt = create_prompt("./icon/ico_loading.png", "终端即将初始化,确认后继续,完成后自动重启", 27, "确认", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_CLIENT_INIT_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_IMG_STOP:
        prompt = create_prompt("./icon/ico_abnormal.png", "服务器镜像分发服务还未开启", 27, NULL, NULL, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;        
    case UI_WIN_STATUS_IMG_ERR:
        prompt = create_prompt("./icon/ico_abnormal.png", "服务器镜像文件不可用", 27, NULL, NULL, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_IMG_LOST:
        prompt = create_prompt("./icon/ico_abnormal.png", "查找不到终端可用的镜像", 27, NULL, NULL, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break; 
    case UI_WIN_STATUS_LOG_ERR:
        prompt = create_prompt("./icon/ico_abnormal.png", "用户名或密码错误", 27, NULL, NULL, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;         
    case UI_WIN_STATUS_BINDED:
        prompt = create_prompt("./icon/ico_abnormal.png", "用户已在其他终端绑定,无法再绑定", 27, NULL, NULL, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_NO_USER:
        prompt = create_prompt("./icon/ico_abnormal.png", "查询不到该用户", 27, NULL, NULL, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;   
    case UI_WIN_STATUS_DEV_NO_IMG:
        prompt = create_prompt("./icon/ico_abnormal.png", "终端未完成初始化,待连接云主机后再继续使用", 27, "返回", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_DEV_NO_IMG_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_DEV_FORBIDED:
        prompt = create_prompt("./icon/ico_abnormal.png", "终端长时间脱网,因安全原因暂停使用,请联系管理员", 27, "返回", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_DEV_FORBIDED_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_DEV_MODE_CHANGE:
        prompt = create_prompt("./icon/ico_abnormal.png", "终端模式变更，点击确定后重启", 27, "确定", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_DEV_MODE_CHANGE_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;   
        
    case UI_WIN_STATUS_DEV_USER_CHANGE:
        prompt = create_prompt("./icon/ico_abnormal.png", "终端绑定用户变更，点击确定后重启", 27, "确定", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_DEV_USER_CHANGE_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_CLIENT_UPDATE_FAIL:
    	//ui_win_btn_all_enable();
       	prompt = create_prompt("./icon/ico_abnormal.png", "更新终端失败...", 27, "重试", G_CALLBACK(ui_win_status_client_tips), (void *)UI_CLIENT_UPDATE_FAIL_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        //ui_win_set_pos(hbox, 35, 30);
        break;
    case UI_WIN_STATUS_CLIENT_UPDATE_FAIL_DEGRADE:
        prompt = create_prompt("./icon/ico_abnormal.png", "服务器版本过低，请升级服务器", 27, "重试", G_CALLBACK(ui_win_status_client_tips), (void *)UI_CLIENT_UPDATE_FAIL_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_CLIENT_UPDATE_FAIL_PID:
        prompt = create_prompt("./icon/ico_abnormal.png", "终端型号不支持在目标版本使用，请联系管理员", 27, NULL, NULL, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_CLIENT_UPDATE_FAIL_UDISK:
        prompt = create_prompt("./icon/ico_abnormal.png", "更新终端失败，请使用一键还原或U盘刷机后重试", 27, NULL, NULL, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_CLIENT_UPDATE_FAIL_OS:
        prompt = create_prompt("./icon/ico_abnormal.png", "终端系统版本过低，不支持升级至课堂版本", 27, NULL, NULL, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_CLIENT_UPDATE_FAIL_OS_DEGRADE:
        prompt = create_prompt("./icon/ico_abnormal.png", "终端系统版本较高，请选择以下方案解决当前问题：\n（1）升级服务器至标准版3.10及以上版本或增强版4.3及以上版本\n" \
                               "（2）U盘灌装0150以下版本终端系统", 27, "重试", G_CALLBACK(ui_win_status_client_tips), (void *)UI_CLIENT_UPDATE_FAIL_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_CLIENT_UPDATE_FAIL_RCC:
        prompt = create_prompt("./icon/ico_abnormal.png", "检测到终端镜像目录含有课堂版本的残留文件\n如需切换为办公使用，请U盘完整刷机后重试", 27, NULL, NULL, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_NET_DISCONNETCT:
		if(1)
		{
			//ui_win_status_enter_local_mode();
			ui_extern_enter_local_mode();
		}
		else
		{
	    	//ui_win_btn_all_enable();
	       	prompt = mk_net_disconect_prompt("./icon/ico_abnormal.png", 27, "立即登录", G_CALLBACK(ui_win_status_enter_local_mode), NULL);
	        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
	        //ui_win_set_pos(hbox, 35, 30);
	        ui_manager_hold_timer();
	        g_signal_connect (G_OBJECT (hbox), "show", G_CALLBACK (ui_manager_offline_autologin_timer_enable), NULL);
	        g_signal_connect (G_OBJECT (hbox), "hide",  G_CALLBACK (ui_manager_offline_autologin_timer_disable), NULL);
		}
        break;
    case UI_WIN_STATUS_LICENSE_OVERRUN:
        prompt = create_prompt("./icon/ico_abnormal.png", "终端数已达授权上限，请申请新的授权", 27, "确定", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_LICENSE_OVERRUN_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_DEV_OVERRUN:
        prompt = create_prompt("./icon/ico_abnormal.png", "终端数已达服务器支持上限，请新增服务器", 27, "确定", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_DEV_OVERRUN_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_IMG_BIG:
    	prompt = create_prompt("./icon/ico_abnormal.png", "C盘镜像过大，请联系管理员", 27, "重试", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_IMG_BIG_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_DELETE_TEACHERDISK:
        prompt = create_prompt("./icon/ico_abnormal.png", "用户数据盘被清空！", 27, "确定", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_DELETE_TEACHERDISK_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_DEV_LOCKED:
    case UI_WIN_STATUS_LAYER_DOWNGRADE:
        logi("dev locked type: %d\n", val);
        prompt = mk_dev_locked_prompt("./icon/ico_abnormal.png", val, "脱网登录", G_CALLBACK(ui_win_status_enter_local_mode), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        ui_win_status.height = 27;
    	break;
    case UI_WIN_STATUS_IMG_BAD_DRIVER:
        prompt = create_prompt("./icon/ico_abnormal.png", "服务器的镜像因驱动问题不支持本终端，无法下载", 27, "重试", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_IMG_BAD_DRIVER_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_IMG_USER_ERR:
        prompt = create_prompt("./icon/ico_abnormal.png", "该用户无效，或该用户无本终端的使用权限", 27, "重试", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_IMG_USER_ERR_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_IMG_NOT_FOUND:
        prompt = create_prompt("./icon/ico_abnormal.png", "请在服务器上为终端所在组绑定镜像", 27, "重试", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_IMG_NOT_FOUND_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_IMG_ABNORMAL:
        prompt = create_prompt("./icon/ico_abnormal.png", "服务器镜像未准备就绪", 27, "重试", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_IMG_ABNORMAL_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_RECOVER_DISK:
        ui_win_btn_disable_config();
        ui_win_wifibtn_disable();
        logi("recover disk note: %s\n", note);
        prompt = create_prompt("./icon/ico_abnormal.png", note, 27, NULL, NULL, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;        
    case UI_WIN_STATUS_START_VM_FAIL_HDMI_ERR:
        ui_win_btn_disable_config();
        ui_win_wifibtn_disable();
        prompt = mk_start_vm_fail_hdmi_err_prompt("./icon/ico_abnormal.png", 27, NULL, NULL, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_SERVER_NOT_VALID:
        prompt = create_prompt("./icon/ico_abnormal.png", "无法连接云主机，请重新配置云主机IP", 27, "设置", (GCallback)ui_win_show_config, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_SERVER_IS_CLASS:
        prompt = create_prompt("./icon/ico_abnormal.png", "检测到云主机为课堂版本，终端数据将被清空\n请谨慎选择!", 27, "确定", (GCallback)ui_win_show_upgrade, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_SERVER_MAINTAIN:
        prompt = mk_sever_maintain_win("./icon/ico_abnormal.png", "服务器维护中，禁止登录", 24);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
     case UI_WIN_STATUS_IMG_GT_VERSION_OUTDATED:
        prompt = create_prompt("./icon/ico_abnormal.png", "服务器镜像的Guesttool版本过低，请联系管理员升级该镜像的Guesttool", 24, "重试", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_IMG_VERSION_GT_OUTDATED_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
     case UI_WIN_STATUS_DEV_NONSUPPORT_OSTYPE32:
        prompt = create_prompt("./icon/ico_abnormal.png", "该终端不支持Win10 32位操作系统, 请联系管理员更换镜像", 24, "重试", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_DEV_NONSPUPPORT_OSTYPE32_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
     case UI_WIN_STATUS_DEV_NONSUPPORT_OSTYPEXP:
        prompt = create_prompt("./icon/ico_abnormal.png", "该终端不支持WinXP操作系统, 请联系管理员更换镜像", 24, "重试", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_DEV_NONSPUPPORT_OSTYPEXP_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_UPGRADE_FAIL:
        prompt = create_prompt("./icon/ico_abnormal.png", "终端更新失败，请检查网络连接！", 24, "返回", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_UPGRADE_FAIL_RET); 
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_AUTUHOR_NOT_FOUND:
        prompt = create_prompt("./icon/ico_abnormal.png", "终端授权证书不存在，无法接入XC-IDV终端", 27, "确定", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_DEV_OVERRUN_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    case UI_WIN_STATUS_AUTUHOR_OUT_OF_RANGE:
        prompt = create_prompt("./icon/ico_abnormal.png", "无可用授权数，无法接入XC-IDV终端", 27, "确定", G_CALLBACK(ui_win_status_client_tips), (void *)UI_TIPS_DEV_OVERRUN_RET);
        gtk_box_pack_start(GTK_BOX(hbox), prompt, FALSE, FALSE, 10);
        break;
    }
    
    ui_win_set_pos(hbox, 0, 0);

    gtk_widget_set_can_focus(GTK_WIDGET(hbox), TRUE);
    gtk_window_set_focus(GTK_WINDOW(g_win_manager.window), hbox);

    return 0;
}

static int ui_win_status_ctrl(void *p)
{
    ui_msg_t *msg = p;

    mode = msg->sub_obj;

    switch (mode) {
    case UI_WIN_STATUS_RECOVER_DISK:
        note = (gchar *)(((ui_string_arg_t *)(msg->args))->str);
        break;
    case UI_WIN_STATUS_DEV_LOCKED:
    case UI_WIN_STATUS_LAYER_DOWNGRADE:
        val = (int)(((ui_int_arg_t *)(msg->args))->val);
        break;
    }

    if (hbox) {
        reset_cursor(hbox);
        gtk_widget_destroy(hbox);
        hbox = NULL;
    }
    ui_win_status_init();

    return 0;
}


static void ui_win_status_hide()
{
    ui_win_btn_all_enable();
    ui_win_wifibtn_enable();

    if (ui_extern_get_wired1xauth_exist() == 0) {
        ui_win_authbtn_enable();
    }
    gtk_widget_hide(hbox);
}

static void ui_win_status_show()
{
    gtk_widget_show(hbox);
}

static void ui_win_status_adapt(void)
{
    if (hbox) {
            ui_win_status.connect_id = g_signal_connect(G_OBJECT (hbox), "size-allocate", 
                                                       G_CALLBACK(ui_win_put_nice_position_cb), &ui_win_status);

            if ( ui_win_status.status == UI_STATUS_SHOW) {
                gtk_widget_hide(hbox);
                if (mode != UI_WIN_STATUS_PUBLIC_LOGIN) {
                    gtk_widget_show(hbox);
                }
            }
    }
}


static void ui_win_status_destroy(void)
{
    if (hbox) {
        gtk_widget_destroy(hbox);
        hbox =  NULL;
    }
}

struct ui_comp_s ui_win_status = {
    .type = UI_TYPE_WIN_STATUS,
    .subtype = UI_TOPLEVEL_TYPE_WIN,
    .is_init = UI_NEED_INIT,
    .widget = &hbox,
    .init = ui_win_status_init,
    .show = ui_win_status_show,
    .hide = ui_win_status_hide,
    .ctrl = ui_win_status_ctrl,
    .adapt = ui_win_status_adapt,
    .destroy = ui_win_status_destroy,
    .height = 31.25,
};


