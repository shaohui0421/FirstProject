#include "ui_main.h"

static GtkWidget *dialog;
static int mode;
static ui_callback_t *cb;
static int cb_num;
static guint dialog_tips_timer;

static guint dialog_tips_timer_dpi = 0;
static int time_sec_dpi;
static GtkWidget * times_label_dpi;

static void ui_dialog_tips_dpi_timer_start(GtkWidget *widget, gpointer data);
static void ui_dialog_tips_dpi_timer_end(GtkWidget *widget, gpointer data);
extern void restore_last_dpi();

static void ui_dialog_tips_hide()
{
    //gtk_widget_hide(dialog);
	if(dialog != NULL) {
		gtk_widget_destroy(dialog);
		dialog = NULL;
	}
}


static void mk_dialog_tips_bg(int type)
{
    switch (type) {
    case UI_DIALOG_TIPS_BG_400_140:
        update_widget_irregular_bg(dialog, "./icon/box_prompt.png", 400, 140);
        gtk_widget_set_size_request(dialog, 400, 140);
        break;
    case UI_DIALOG_TIPS_BG_400_110:
        update_widget_irregular_bg(dialog, "./icon/box_prompt_400x110.png", 400, 110);
        gtk_widget_set_size_request(dialog, 400, 110);
        break;
    case UI_DIALOG_TIPS_BG_400_200:
        update_widget_irregular_bg(dialog, "./icon/box_prompt_400x200.png", 400, 200);
        gtk_widget_set_size_request(dialog, 400, 200);
        break;        
    }
}

static GtkWidget * mk_title(char * text, int size)
{
	GtkWidget * align;
	GtkWidget * hbox;
	GtkWidget * label;

	label = gtk_label_new(text);
    set_widget_font_size(label, size, "Snow");
	gtk_widget_show(label);

	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(hbox);
	align = gtk_alignment_new(0.5,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

	return align;
}

static void do_btn_deal(gpointer data)
{
    static void (*func)(void *);
    static void *func_args;

	switch (mode) {
  	case UI_DIALOG_TIPS_CP_BASE_INSERT_U_DISK:
        func = cb[0].callback;
        func_args = cb[0].args;
		if (func) {
			(*func)(func_args);
		}
		break;
	case UI_DIALOG_TIPS_CP_BASE:
        if (strcmp((char*)data, "确定") == 0) {
            func = cb[0].callback;
            func_args = cb[0].args;
    		if (func) {
    			(*func)(func_args);
    		}
        } else if (strcmp((char*)data, "取消") == 0) {
            func = cb[1].callback;
            func_args = cb[1].args;
    		if (func) {
    			(*func)(func_args);
    		}
        }
        break;
    case UI_DIALOG_TIPS_CP_BASE_FAIL:
    case UI_DIALOG_TIPS_CP_BASE_FAIL_NO_SPACE:
    case UI_DIALOG_TIPS_CP_BASE_FAIL_NO_DEV:
        func = cb[0].callback;
        func_args = cb[0].args;
		if (func) {
			(*func)(func_args);
		}
        break;
	case UI_DIALOG_TIPS_SAVE_DPI:
        if (strcmp((char*)data, "保存") == 0) {
            func = cb[0].callback;
            func_args = cb[0].args;
    		if (func) {
    			(*func)(func_args);
    		}
        } else if (strcmp((char*)data, "还原") == 0) {
            func = cb[1].callback;
            func_args = cb[1].args;
    		if (func) {
    			(*func)(func_args);
    		}
        }
        break;
    case UI_DIALOG_TIPS_LAYER_MANAGE:
        if (strcmp((char*)data, "确定") == 0) {
            func = cb[0].callback;
            func_args = cb[0].args;
            if (func) {
                (*func)(func_args);
            }
        } else if (strcmp((char*)data, "取消") == 0) {
            func = cb[1].callback;
            func_args = cb[1].args;
            if (func) {
                (*func)(func_args);
            }
        }
        break;
	}
}


static gint btn_common_deal(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	do_btn_deal(data);
	if (dialog) {
		gtk_widget_destroy(dialog);
		dialog = NULL;
	}
    return FALSE;
}

static GtkWidget * mk_action_area(char * btn_text)
{
	GtkWidget * hbox;
	GtkWidget * align;
	btn_apend_t * btn;

	btn = create_button("./icon/btn_green_80x26.png", "./icon/btn_green_80x26_h.png", NULL, btn_text, -1, -1, G_CALLBACK(btn_common_deal), NULL);
	set_widget_font_size(btn->label, 10, "Snow");

    hbox = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), btn->container, FALSE, FALSE, 0);
 	gtk_widget_show(hbox);


	align = gtk_alignment_new(0.5,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

	return align;
}

static GtkWidget * mk_action_area2(char * btn1_text, char * btn2_text)
{
    GtkWidget * hbox;
    GtkWidget * align;
    btn_apend_t * btn1;
    btn_apend_t * btn2;
    
    btn1 = create_button("./icon/btn_green_80x26.png", "./icon/btn_green_80x26_h.png", NULL, btn1_text, -1, -1, G_CALLBACK(btn_common_deal), btn1_text);
    set_widget_font_size(btn1->label, 10, "Snow");
    
    btn2 = create_button("./icon/btn_gray_80x26.png", "./icon/btn_gray_80x26_h.png", NULL, btn2_text, -1, -1, G_CALLBACK(btn_common_deal), btn2_text);
    set_widget_font_size(btn2->label, 10, "Snow");

    hbox = gtk_hbox_new(FALSE, 20);
    gtk_box_pack_start(GTK_BOX(hbox), btn1->container, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), btn2->container, FALSE, FALSE, 0);
 	gtk_widget_show(hbox);


	align = gtk_alignment_new(0.5,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

	return align;
}

static GtkWidget * mk_save_dpi_prompt()
{
#define SAVE_DPI_TIMEOUT        (15)
	GtkWidget * align;
	GtkWidget * vbox = NULL;
    GtkWidget * quest_save;
    GtkWidget * times_label_dpi_tips;
    GtkWidget * time_hbox_dpi;
    char timeout_buf[16];

    quest_save = create_prompt("./icon/ico_prompt_y.png", "是否保存当前分辨率设置 ", 13, NULL, NULL, NULL);
    gtk_widget_show(quest_save);
	gtk_box_pack_start(GTK_BOX(vbox), quest_save, FALSE, FALSE, 0);

    time_sec_dpi = SAVE_DPI_TIMEOUT;
    g_snprintf(timeout_buf, 16, "%d秒", time_sec_dpi);
    times_label_dpi = gtk_label_new(timeout_buf);
    set_widget_font_size(times_label_dpi, 18, "LightSeaGreen");
    gtk_widget_show(times_label_dpi);
    times_label_dpi_tips = create_prompt(NULL, "后恢复以前的分辨率设置", 13, NULL, NULL, NULL);

	time_hbox_dpi = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_end(GTK_BOX(time_hbox_dpi), times_label_dpi_tips, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(time_hbox_dpi), times_label_dpi, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (time_hbox_dpi), "show", G_CALLBACK (ui_dialog_tips_dpi_timer_start), NULL);
 	g_signal_connect (G_OBJECT (time_hbox_dpi), "hide", G_CALLBACK (ui_dialog_tips_dpi_timer_end), NULL);
 	g_signal_connect (G_OBJECT (time_hbox_dpi), "destroy", G_CALLBACK (ui_dialog_tips_dpi_timer_end), NULL);
	gtk_widget_show(time_hbox_dpi);

	vbox = gtk_vbox_new(FALSE, 20);
	gtk_box_pack_start(GTK_BOX(vbox), quest_save, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), time_hbox_dpi, FALSE, FALSE, 0);
	gtk_widget_show(vbox);
    
	align = gtk_alignment_new(0.5,0.5,0,0);
	gtk_container_add(GTK_CONTAINER(align), vbox);
	gtk_widget_show(align);

    return align;
#undef SAVE_DPI_TIMEOUT
}

static int ui_dialog_tips_init(void)
{
     /* Create the widgets */
    dialog = gtk_dialog_new();
	GtkWidget * vbox;
    GtkWidget * title = NULL;
	GtkWidget * prompt = NULL;
    GtkWidget * prompt_dpi = NULL;
	GtkWidget * action_area = NULL;

    g_signal_connect (G_OBJECT (dialog), "realize",
                  G_CALLBACK (ui_compt_realize), &ui_dialog_tips);
    
    gtk_window_set_decorated (GTK_WINDOW(dialog), FALSE);
    //gtk_window_set_opacity (GTK_WINDOW(dialog), FALSE);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

    switch (mode) {
    case UI_DIALOG_TIPS_CONFIG_OK:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_140);
    	prompt = create_prompt("./icon/ico_correct32.png", "保存成功", 15, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(2);
        break;
    case UI_DIALOG_TIPS_CONFIG_OK_REBOOT:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_140);
        prompt = create_prompt("./icon/ico_correct32.png", "保存成功，正在重启", 15, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(2);
        break;
    case UI_DIALOG_TIPS_CONFIG_ERROR:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_140);
        prompt = create_prompt("./icon/ico_abnormal_m.png", "保存失败", 15, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(2);
        break;
    case UI_DIALOG_TIPS_CONFIG_IP_OK:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_140);
    	prompt = create_prompt("./icon/ico_correct32.png", "云主机地址和终端网络配置成功", 15, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(2);
        break;
    case UI_DIALOG_TIPS_CONFIG_WLAN_SAVE_IP:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_140);
    	prompt = create_prompt("./icon/ico_abnormal_m.png", "无线连接时无法保存IP", 15, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(2);
        break;
    case UI_DIALOG_TIPS_CONNECTING:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_110);
        prompt = create_prompt("./icon/ico_loading.gif", "正在连接网络", 15, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(30);
        break;
    case UI_DIALOG_TIPS_CONNECT_SUCCESS:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_110);
        prompt = create_prompt("./icon/ico_correct32.png", "网络连接成功", 15, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(2);
        break;
    case UI_DIALOG_TIPS_CONNECT_FAIL:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_110);
        prompt = create_prompt("./icon/ico_abnormal_m.png", "网络连接失败", 15, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(2);
        break;
    case UI_DIALOG_TIPS_AUTHENTICE_FAIL:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_110);
        prompt = create_prompt("./icon/ico_abnormal_m.png", "网络认证失败", 15, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(2);
        break;
    case UI_DIALOG_TIPS_CONNECT_TIMEOUT:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_110);
        prompt = create_prompt("./icon/ico_abnormal_m.png", "网络连接超时", 15, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(2);
        break;
    case UI_DIALOG_TIPS_CP_BASE_INSERT_U_DISK:        
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_200);
        title = mk_title("提示", 13);
        prompt = create_prompt(NULL, "请插入U盘", 13, NULL, NULL, NULL);
        action_area = mk_action_area("确定");
        break;
    case UI_DIALOG_TIPS_CP_BASE:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_200);
        title = mk_title("提示", 13);
        prompt = create_prompt(NULL, "确定从U盘导入? (仅支持NTFS格式U盘)", 13, NULL, NULL, NULL);
        action_area = mk_action_area2("确定", "取消");
        break;
    case UI_DIALOG_TIPS_CP_BASE_FAIL:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_200);
        title = mk_title("提示", 13);
        prompt = create_prompt(NULL, "镜像文件识别失败，请检查镜像文件", 13, NULL, NULL, NULL);
        action_area = mk_action_area("确定");
        break;
    case UI_DIALOG_TIPS_CP_BASE_FAIL_NO_SPACE:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_200);
        title = mk_title("提示", 13);
        prompt = create_prompt(NULL, "镜像文件过大，无法导入", 13, NULL, NULL, NULL);
        action_area = mk_action_area("确定");
        break;
    case UI_DIALOG_TIPS_CP_BASE_FAIL_NO_DEV:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_200);
        title = mk_title("提示", 13);
        prompt = create_prompt(NULL, "请插入U盘", 13, NULL, NULL, NULL);
        action_area = mk_action_area("确定");
        break;
    case UI_DIALOG_TIPS_AUTH_USER_EMPTY:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_110);
    	prompt = create_prompt("./icon/ico_abnormal_m.png", "用户名输入为空", 15, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(1);
        break;
    case UI_DIALOG_TIPS_AUTH_PWD_EMPTY:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_110);
    	prompt = create_prompt("./icon/ico_abnormal_m.png", "密码输入为空", 15, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(1);
        break;
    case UI_DIALOG_TIPS_AUTH_PWD_TOO_SHORT:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_110);
        prompt = create_prompt("./icon/ico_abnormal_m.png", "密码长度不能少于8个字符", 15, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(1);
        break;
    case UI_DIALOG_TIPS_AUTH_SSID_EMPTY:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_110);
    	prompt = create_prompt("./icon/ico_abnormal_m.png", "SSID输入为空", 15, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(1);
        break;
    case UI_DIALOG_TIPS_OTHRE_AUTHENTICE_TYPE:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_110);
    	prompt = create_prompt("./icon/ico_abnormal_m.png", "不支持此认证类型", 15, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(2);
        break;
    case UI_DIALOG_TIPS_NOT_SUPPORT_WEB_HOTSPOT:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_110);
    	prompt = create_prompt("./icon/ico_abnormal_m.png", "不支持WEB热点，已自动断开连接", 13, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(2);
        break;
    case UI_DIALOG_TIPS_CONNECT_NOT_WHITESSID:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_110);
    	prompt = create_prompt("./icon/ico_abnormal_m.png", "无法连接非白名单内的WIFI", 13, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(2);
        break;
	case UI_DIALOG_TIPS_SAVE_DPI:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_200);
        title = mk_title("", 2);
        prompt_dpi = mk_save_dpi_prompt();
        action_area = mk_action_area2("保存", "还原");
        break;
   case UI_DIALOG_TIPS_NET_AUTHING:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_110);
        prompt = create_prompt("./icon/ico_loading.gif", "认证处理中...", 15, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(90);  //see auth_manager.h  AUTHENTICATE_TIMEOUT
        break;
    case UI_DIALOG_TIPS_NET_AUTH_SUCCESS:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_140);
    	prompt = create_prompt("./icon/ico_correct32.png", "认证成功", 15, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(2);
        break;
    case UI_DIALOG_TIPS_NET_AUTH_FAIL:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_140);
    	prompt = create_prompt("./icon/ico_abnormal_m.png", "认证失败", 15, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(2);
        break;
    case UI_DIALOG_TIPS_NET_AUTH_FAIL_USER_ERR:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_140);
    	prompt = create_prompt("./icon/ico_abnormal_m.png", "用户名或密码为空", 15, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(2);
        break;
    case UI_DIALOG_TIPS_NET_AUTH_TIMEOUT:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_140);
    	prompt = create_prompt("./icon/ico_abnormal_m.png", "认证超时", 15, NULL, NULL, NULL);
        ui_dialog_tips_set_timer(2);
        break;
   case UI_DIALOG_TIPS_LAYER_MANAGE:
        mk_dialog_tips_bg(UI_DIALOG_TIPS_BG_400_200);
        title = mk_title("提示", 13);
        prompt = create_prompt(NULL, "请确认是否关闭应用集中更新模式？\n注意：关闭后无法再次开启", 13, NULL, NULL, NULL);
        action_area = mk_action_area2("确定", "取消");
        break;

    default:
        break;
    }

    if (title && prompt && action_area) {
        vbox = gtk_vbox_new(FALSE, 40);
        gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE,0);
        gtk_box_pack_start(GTK_BOX(vbox), prompt, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), action_area, FALSE, FALSE, 0);
        gtk_widget_show(vbox);
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), vbox, TRUE, FALSE, 0);
    }
    else if (title &&prompt_dpi && action_area) {
        vbox = gtk_vbox_new(FALSE, 30);
        //gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE,0);
        gtk_box_pack_start(GTK_BOX(vbox), prompt_dpi, FALSE, FALSE, 0);
        gtk_box_pack_end(GTK_BOX(vbox), action_area, FALSE, FALSE, 0);
        gtk_widget_show(vbox);
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), vbox, TRUE, FALSE, 0);
    }
    else if (prompt) {
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), prompt, TRUE, FALSE, 0);
    }

    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
    return 0;
}


static int ui_dialog_tips_ctrl(void *p)
{
    ui_msg_t *msg = p;

    mode = msg->sub_obj;
    if (msg->args)
    {
        if (cb) {
            free(cb);
        }
        cb = ((ui_callbacks_t *)(msg->args))->cb;
        cb_num = ((ui_callbacks_t *)(msg->args))->cb_num;
    }

    return 0;
}

static void ui_dialog_tips_show(void)
{
/*	logi("chenw ui_dialog_tips_show\n");
    if (dialog == NULL) {
        ui_dialog_tips_init();
    }
    gtk_widget_show_all(dialog);
*/
    if (dialog) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }

    ui_dialog_tips_init();
    gtk_widget_show(dialog);

}

static gboolean ui_dialog_tips_timer_callback(gpointer user_data)
{
    ui_dialog_tips_hide();
    return FALSE;
}

static gboolean ui_dialog_tips_timer_callback_connecting(gpointer user_data) {
    int net_status = ui_extern_net_status_query_result();
    if (net_status == 2) {
        //wifi up
        mode = UI_DIALOG_TIPS_CONNECT_SUCCESS;
        ui_dialog_tips_show();
    } else {
        mode = UI_DIALOG_TIPS_CONNECT_TIMEOUT;
        ui_dialog_tips_show();
        ui_extern_cancel_show_wifipwd();
    }
    return FALSE;
}

static gboolean ui_dialog_tips_timer_net_auth_timeout(gpointer user_data) {
    mode = UI_DIALOG_TIPS_NET_AUTH_TIMEOUT;
    ui_dialog_tips_show();
    return FALSE;
}

static void ui_dialog_tips_timer_end(GtkWidget *widget, gpointer data)
{
    if (dialog_tips_timer) {
        logi("dialog_tips_timer_end\n");
        g_source_remove(dialog_tips_timer);
        dialog_tips_timer = 0;
    }
}

static void ui_dialog_tips_timer_start(GtkWidget *widget, gpointer data)
{
    guint interval = *((guint*)data);
    ui_dialog_tips_timer_end(widget, data);
    logi("dialog_tips_timer_start\n");
    if (mode == UI_DIALOG_TIPS_CONNECTING) {
        dialog_tips_timer = g_timeout_add_seconds(interval, ui_dialog_tips_timer_callback_connecting, NULL);
    } else if (mode == UI_DIALOG_TIPS_NET_AUTHING) {
        dialog_tips_timer = g_timeout_add_seconds(interval, ui_dialog_tips_timer_net_auth_timeout, NULL);
    } else {
        dialog_tips_timer = g_timeout_add_seconds(interval, ui_dialog_tips_timer_callback, NULL);
    }
}

/**
 * Function:    ui_dialog_tips_set_timer
 * Description: start dialog_tips timer
 * Input:        seconds:   displayed time of dialog_tips(seconds). 0 means infinite.
 */
void ui_dialog_tips_set_timer(guint seconds)
{
    if (seconds == 0) {
        return;
    }
    ui_dialog_tips_timer_start(dialog, (gpointer)(&seconds));        
    g_signal_connect (G_OBJECT (dialog), "hide", G_CALLBACK (ui_dialog_tips_timer_end), NULL);
    g_signal_connect (G_OBJECT (dialog), "destroy", G_CALLBACK (ui_dialog_tips_timer_end), NULL);
}

static void ui_dialog_tips_dpi_timer_end(GtkWidget *widget, gpointer data)
{
    if (dialog_tips_timer_dpi) {
        logi("ui_dialog_tips_dpi_timer_end\n");
        g_source_remove(dialog_tips_timer_dpi);
        dialog_tips_timer_dpi = 0;
    }
}

static gboolean ui_dialog_tips_dpi_timer_callback(gpointer user_data)
{
    char buf[16];

    if (time_sec_dpi > 0) {
        g_snprintf(buf, 16, "%d秒", time_sec_dpi);
        gtk_label_set_text(GTK_LABEL(times_label_dpi), buf);
        time_sec_dpi--;
        return TRUE;
    } else {
        dialog_tips_timer_dpi = 0;
        restore_last_dpi();
        ui_dialog_tips_hide();
        return FALSE;
    }
}

static void ui_dialog_tips_dpi_timer_start(GtkWidget *widget, gpointer data)
{
    if (dialog_tips_timer_dpi) {
        ui_dialog_tips_dpi_timer_end(widget, data);
    }
    logi("ui_dialog_tips_dpi_timer_start\n");
    dialog_tips_timer_dpi = g_timeout_add_seconds(1, ui_dialog_tips_dpi_timer_callback, NULL);
}

static void ui_dialog_tpis_adapt(void)
{
    if (dialog) {
        if (ui_dialog_tips.status == UI_STATUS_SHOW) {
            logi("ui_dialog_tpis_adapt\n");
            gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
        }
    }
}

static void ui_dialog_tpis_destroy(void)
{
    if (dialog) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
}

struct ui_comp_s ui_dialog_tips = {
    .type = UI_TYPE_DIALOG_TIPS,
    .subtype = UI_TOPLEVEL_TYPE_DIALOG,
    .is_init = UI_NEED_INIT,
    .widget = &dialog,
    .init = ui_dialog_tips_init,
    .show = ui_dialog_tips_show,
    .hide = ui_dialog_tips_hide,
    .ctrl = ui_dialog_tips_ctrl,
    .destroy = ui_dialog_tpis_destroy,
    .adapt = ui_dialog_tpis_adapt,
};

