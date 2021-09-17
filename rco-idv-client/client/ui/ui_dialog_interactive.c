#include "ui_main.h"

static GtkWidget *dialog = NULL;
//static int mode;

static guint common_timer;
static GtkWidget * time_label;
GtkWidget * timer_hbox;
static int time_sec = 0;
static int mode;
static void (*func)(void *);
static void *func_args;
static void *data;

static void ui_dialog_interactive_hide()
{
/*
	if (dialog) {
		gtk_widget_destroy(dialog);
		dialog = NULL;
	}
*/
}

static void do_btn_deal1(void){
        if (data) {
            *((gboolean *)data) = TRUE;
        }

        if (func) {
            (*func)(func_args);
        }
    }

static void do_btn_deal(void)
{
    switch (mode) {
    case UI_DIALOG_INTERACTIVE_SHOW_AUTOSHUTDOWN:
        ui_extern_cancal_auto_shutdown();
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_EASYDEPLOY:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_VMERR:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_VMLASTERR:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_IMGBIG:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_BADDRIVER:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_IMGABNORMAL:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_UPGRADEERR:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_DOWNLOAD_CONFIRM:
    case UI_DIALOG_INTERACTIVE_SHOW_NEED_MERGE:
        if (data) {
            *((gboolean *)data) = TRUE;
        }

        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_DOWNLOAD_MODE_SELECT:
        if (data) {
            *((gboolean *)data) = FALSE;
        }

        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_OSTYPERR:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_OSTYPENOTSPORT:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_CPUNOTSPORT:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_VMMODEINILOST:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_DRIVER_ERR:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_CDISK_ERR:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_LAYER_TEMPLETE_ERR:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_INTEL_NO_AUDIO_DEVICE:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_EASYDEPLOY_NOT_WIFI:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_NOTSUPPORT_XP:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_NOTSUPPORT_WIN10_32:
        if (func) {
            (*func)(func_args);
        }
        break;
    }
}


static void do_timeout_deal(void)
{
    switch (mode) {
    case UI_DIALOG_INTERACTIVE_SHOW_AUTOSHUTDOWN:
        ui_extern_shutdown(1);
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_EASYDEPLOY:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_VMERR:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_VMLASTERR:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_IMGBIG:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_BADDRIVER:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_IMGABNORMAL:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_UPGRADEERR:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_DOWNLOAD_CONFIRM:
    case UI_DIALOG_INTERACTIVE_SHOW_DOWNLOAD_MODE_SELECT:
    case UI_DIALOG_INTERACTIVE_SHOW_NEED_MERGE:
        if (data) {
            *((gboolean *)data) = FALSE;
        }

        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_OSTYPERR:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_OSTYPENOTSPORT:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_CPUNOTSPORT:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_VMMODEINILOST:
        if (func) {
            (*func)(func_args);
        }
    case UI_DIALOG_INTERACTIVE_SHOW_DRIVER_ERR:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_CDISK_ERR:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_LAYER_TEMPLETE_ERR:
        if (func) {
            (*func)(func_args);
        }
    case UI_DIALOG_INTERACTIVE_SHOW_INTEL_NO_AUDIO_DEVICE:
        if (func) {
            (*func)(func_args);
        }
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_EASYDEPLOY_NOT_WIFI:
        if (func) {
            (*func)(func_args);
        }
        break;
    }
}

static gint btn_common_deal(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    do_btn_deal();
    if (dialog) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
    return FALSE;
}
static gint btn_common_deal1(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    //ui_
    do_btn_deal1();
    if (dialog) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
    return TRUE;
}

#if 0
static GtkWidget * mk_action_area(char * btn_text)
{
    GtkWidget * vbox;
    GtkWidget * align;
    btn_apend_t * btn;
    GdkPixbuf * src_pixbuf = NULL, *dst_pixbuf = NULL;
    GtkWidget * image;

    btn = create_button("./icon/btn_green.png", "./icon/btn_green_h.png", NULL, btn_text, -1, -1, G_CALLBACK(btn_common_deal), NULL);
    set_widget_font_size(btn->label, 13, "Snow");

    src_pixbuf = gdk_pixbuf_new_from_file("./icon/shadow.png", NULL);
    dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, 500, 1, GDK_INTERP_BILINEAR);
    image = gtk_image_new_from_pixbuf(dst_pixbuf);
    g_object_unref(src_pixbuf);
    g_object_unref(dst_pixbuf);
    gtk_widget_show(image);

    vbox = gtk_vbox_new(FALSE, 20);
    gtk_box_pack_start(GTK_BOX(vbox), image, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn->container, FALSE, FALSE, 0);
    gtk_widget_show(vbox);

    align = gtk_alignment_new(0.5, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), vbox);
    gtk_widget_show(align);
    
    return align;
}
#endif

static GtkWidget * mk_action_area(char * btn_text)
{
    GtkWidget * hbox;
    GtkWidget * align;
    btn_apend_t * btn;

    btn = create_button("./icon/btn_green.png", "./icon/btn_green_h.png", NULL, btn_text, -1, -1, G_CALLBACK(btn_common_deal), NULL);
    set_widget_font_size(btn->label, 13, "Snow");

    hbox = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), btn->container, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

    align = gtk_alignment_new(0.5,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

    return align;
}


static gboolean common_timer_timeout(gpointer user_data)
{
    char buf[20];

    if (--time_sec != 0) {
        g_snprintf (buf, 20, "%d秒", time_sec);
        gtk_label_set_text(GTK_LABEL(time_label), buf);
        return TRUE;
    } else {
        g_source_remove(common_timer);
        common_timer = 0;
        do_timeout_deal();
    	if (dialog) {
    		gtk_widget_destroy(dialog);
    		dialog = NULL;
    	}

        //ui_extern_new_deploy(mode, gtk_entry_get_text(GTK_ENTRY(usr_entry)));
        return FALSE;
    }
}



static void commom_timer_end(GtkWidget *widget, gpointer data)
{
    if (common_timer) {
        logi("commom_timer_end\n");
        g_source_remove(common_timer);
        common_timer = 0;
    }
}

static void common_timer_start(GtkWidget *widget, gpointer data)
{
	if (common_timer) {
		commom_timer_end(widget, data);
	}

	logi("common_timer_start\n");
	common_timer = g_timeout_add_seconds(1, common_timer_timeout, data);

}

GtkWidget * mk_timer(int sec, char * text, int font_size)
{
	GtkWidget * tips;
	GtkWidget * align;
	char buf[20];

    time_sec = sec;
    g_snprintf (buf, sizeof(buf), "%d秒", time_sec);
	time_label = gtk_label_new(buf);
	set_widget_font_size(time_label, font_size, "LightSeaGreen");
	gtk_widget_show(time_label);

	tips = gtk_label_new(text);
	set_widget_font_size(tips, font_size, NULL);
	gtk_widget_show(tips);

	timer_hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(timer_hbox), time_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(timer_hbox), tips, FALSE, FALSE, 0);

    g_signal_connect (G_OBJECT (timer_hbox), "show", G_CALLBACK (common_timer_start), NULL);
    g_signal_connect (G_OBJECT (timer_hbox), "hide", G_CALLBACK (commom_timer_end), NULL);
    g_signal_connect (G_OBJECT (timer_hbox), "destroy", G_CALLBACK (commom_timer_end), NULL);
    gtk_widget_hide(timer_hbox);

	align = gtk_alignment_new(0.5,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), timer_hbox);
    gtk_widget_show(align);
    return align;
}

#if 0
static GtkWidget * mk_title(char * text)
{
    GtkWidget * label;
    GtkWidget * event_box;
    GdkColor fg_color, bg_color;

    label = gtk_label_new(text);
    gtk_widget_set_size_request(label, 500, 30);
    gtk_misc_set_alignment(GTK_MISC(label), 0.016, 0.5);

    event_box = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(dialog), event_box);
    gtk_container_add(GTK_CONTAINER(event_box), label);

    gdk_color_parse ("Black", &fg_color);
    gdk_color_parse ("WhiteSmoke", &bg_color);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &fg_color);
    gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &bg_color);

    gtk_widget_show(label);
    gtk_widget_show(event_box);

    return event_box;
}
#endif

static GtkWidget * mk_title1(char * text)
{
    GtkWidget * hbox;
    GtkWidget * label;
    GtkWidget * hide_label;
    btn_apend_t *button_quit;
    
    hide_label = gtk_label_new("");
    gtk_widget_set_size_request(hide_label, 8, 1);
    gtk_widget_show(hide_label);

    label = gtk_label_new(text);
    set_widget_font_size(label, 12, NULL);
    gtk_widget_show(label);
    


    hbox = gtk_hbox_new(FALSE,10);
    gtk_box_pack_start(GTK_BOX(hbox), hide_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    
    button_quit = create_button("./icon/btn_close.png", "./icon/btn_close_h.png", NULL, NULL, -1, -1, G_CALLBACK(btn_common_deal1), NULL);
	gtk_box_pack_end(GTK_BOX(hbox), button_quit->container, FALSE, FALSE, 5);
	
   // gtk_box_pack_end(GTK_BOX(hbox), image, FALSE, FALSE, 0);
    gtk_widget_show(hbox);
    return hbox;
}


static GtkWidget * mk_title(char * text)
{
    GtkWidget * hbox;
    GtkWidget * label;
    GtkWidget * hide_label;

    hide_label = gtk_label_new("");
    gtk_widget_set_size_request(hide_label, 8, 1);
    gtk_widget_show(hide_label);

    label = gtk_label_new(text);
    set_widget_font_size(label, 12, NULL);
    gtk_widget_show(label);

    hbox = gtk_hbox_new(FALSE,10);
    gtk_box_pack_start(GTK_BOX(hbox), hide_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_widget_show(hbox);
    return hbox;
}

static GtkWidget * mk_tips(char * text, int font_size)
{
    GtkWidget * hbox;
    GtkWidget * label;
    GtkWidget * align;
    GtkWidget * image;
    GdkPixbuf *src_pixbuf = NULL;

    label = gtk_label_new(text);
    set_widget_font_size(label, font_size, NULL);
    gtk_widget_show(label);

    src_pixbuf = gdk_pixbuf_new_from_file("./icon/ico_prompt_y.png", NULL);
    image = gtk_image_new_from_pixbuf(src_pixbuf);
    g_object_unref(src_pixbuf);
    gtk_widget_show(image);

    hbox = gtk_hbox_new(FALSE,10);
    gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

    align = gtk_alignment_new(0.5,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

    return align;
}

static GtkWidget * mk_vmerr_tips(char *top_txt, char *font_name, char * text, int font_size) //是否加粗
{
    GtkWidget * hbox;
    GtkWidget * vbox;
    GtkWidget * label;
    GtkWidget * top_label;
     GtkWidget *hide_label;
    GtkWidget * align;
    GtkWidget * image;
    GdkPixbuf * src_pixbuf = NULL;

    src_pixbuf = gdk_pixbuf_new_from_file("./icon/vm_failed.png", NULL);
    image = gtk_image_new_from_pixbuf(src_pixbuf);
    g_object_unref(src_pixbuf);
    gtk_misc_set_alignment(GTK_MISC(image), 0, 0);
    gtk_widget_show(image);

    hide_label = gtk_label_new("");
    gtk_widget_set_size_request(hide_label, 400, 1);
    gtk_widget_show(hide_label);

    top_label = gtk_label_new(top_txt);
    set_widget_font_type_size(top_label, font_name, NULL);
    gtk_misc_set_alignment(GTK_MISC(top_label), 0, 0);
    gtk_widget_show(top_label);

    label = gtk_label_new(text);
    gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    gtk_widget_set_size_request(label, 388, 110);
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
    set_widget_font_size(label, font_size, NULL);
    gtk_widget_show(label);

    vbox = gtk_vbox_new(FALSE,10);
    gtk_box_pack_start(GTK_BOX(vbox), hide_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), top_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
    gtk_widget_show(vbox);

    hbox = gtk_hbox_new(FALSE,10);
    gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

    align = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);
    return align;

}


static int ui_dialog_interactive_init(void)
{
     /* Create the widgets */

    dialog = gtk_dialog_new();
    //gtk_widget_set_size_request(dialog, 450 ,300);
    gtk_widget_set_size_request(dialog, 500 ,300);
    gtk_window_set_decorated (GTK_WINDOW(dialog), FALSE);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    ui_dialog_white_background(dialog);
    g_signal_connect (G_OBJECT (dialog), "realize",
                      G_CALLBACK (ui_compt_realize), &ui_dialog_interactive);

    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
    return 0;
}


static int ui_dialog_interactive_ctrl(void *arg)
{
    ui_msg_t *msg = arg;
    GtkWidget * title = NULL;
    GtkWidget * title1 = NULL;
    GtkWidget * tips = NULL;
    GtkWidget * timer_cnter = NULL;
    GtkWidget * action_area = NULL;
    GtkWidget * vbox;
    char buf_tips[512] = {0};

    if (dialog == NULL) {
        ui_dialog_interactive_init();
    }

    mode = msg->object;
    if (msg->args) {
        func = ((ui_callback_t *)(msg->args))->callback;
        func_args = ((ui_callback_t *)(msg->args))->args;
        data = ((ui_callback_t *)(msg->args))->data;
    } else {
        func = NULL;
        func_args = NULL;
        data = NULL;
    }

    switch(mode) {
    case UI_DIALOG_INTERACTIVE_SHOW_AUTOSHUTDOWN:
        title = mk_title("自动重启");
        timer_cnter = mk_timer(60, "后自动重启系统", 20);
        action_area = mk_action_area("取消");
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_EASYDEPLOY:
        title = mk_title("提示");
        tips = mk_tips("一键部署工具修改配置生效", 17);
        timer_cnter = mk_timer(5, "后返回", 17);
        action_area = mk_action_area("返回");
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_VMERR:
        title = mk_title("提示");
        tips = mk_tips("虚机启动失败,即将返回", 17);
        timer_cnter = mk_timer(5, "后返回", 17);
        action_area = mk_action_area("返回");
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_VMLASTERR:
        title = mk_title("提示");
        tips = mk_tips("虚机启动异常，终端已自动重启", 16);
        timer_cnter = mk_timer(5, "后返回", 16);
        action_area = mk_action_area("返回");
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_IMGBIG:
        title = mk_title("提示");
        tips = mk_tips("终端镜像文件过大，不支持静默下载", 16);
        timer_cnter = mk_timer(5, "后返回", 16);
        action_area = mk_action_area("返回");
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_BADDRIVER:
        title = mk_title("提示");
        tips = mk_tips("镜像内驱动有误，无法下载使用", 16);
        timer_cnter = mk_timer(5, "后返回", 16);
        action_area = mk_action_area("返回");
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_IMGABNORMAL:
        title = mk_title("提示");
        tips = mk_tips("服务器镜像未准备就绪，无法下载使用", 16);
        timer_cnter = mk_timer(5, "后返回", 16);
        action_area = mk_action_area("返回");
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_UPGRADEERR:
        title = mk_title("提示");
        tips = mk_tips("ISO版本不满足要求，无法升级", 16);
        timer_cnter = mk_timer(5, "后返回", 16);
        action_area = mk_action_area("返回");
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_DOWNLOAD_CONFIRM:
        title = mk_title("镜像替换");
        tips = mk_tips("是否替换当前镜像，替换后个人镜像将还原", 16);
        timer_cnter = mk_timer(5, "后取消替换", 16);
        action_area = mk_action_area("替换");
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_DOWNLOAD_MODE_SELECT:
        title1 = mk_title1("下载镜像");
        sprintf(buf_tips, "终端镜像文件已更新，文件大小：%s", update_image_size);
        tips =  mk_tips(buf_tips, 16);
        timer_cnter = mk_timer(10, "后自动进入后台下载", 16);
        action_area = mk_action_area("确定");
        break;     
   case UI_DIALOG_INTERACTIVE_SHOW_NEED_MERGE:
        title = mk_title("系统更新");
        tips =  mk_tips("系统将在", 16);
        timer_cnter = mk_timer(5, "后重启更新", 16);
        action_area = mk_action_area("立即重启");
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_OSTYPENOTSPORT:
    case UI_DIALOG_INTERACTIVE_SHOW_OSTYPERR:
        title = mk_title("提示");
        if (data) {
            sprintf(buf_tips, "当前下发的镜像操作系统为%s，终端无法识别“Windows XP” “Windows 7” “Windows 10”以外的镜像操作系统，请在服务器上重新绑定一个终端可识别的镜像。", (char *)data);
        } else {
            strcpy(buf_tips, "终端无法识别“Windows XP” “Windows 7” “Windows 10”以外的镜像操作系统，请在服务器上重新绑定一个终端可识别的镜像。");
        }

        tips = mk_vmerr_tips("启动虚机失败！", "Bold 13", buf_tips, 13);
        action_area = mk_action_area("确定");
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_CPUNOTSPORT:
        title = mk_title("提示");
        if (data) {
            sprintf(buf_tips, "终端处理器%s在当前版本不支持，请联系管理员升级到支持该处理器的终端版本。", (char *)data);
        } else {
            strcpy(buf_tips, "当前版本不支持该终端处理器，请联系管理员升级到支持该处理器的终端版本。");
        }

        tips = mk_vmerr_tips("启动虚机失败！", "Bold 13", buf_tips, 13);
        action_area = mk_action_area("确定");
        break;
     case UI_DIALOG_INTERACTIVE_SHOW_VMMODEINILOST:
        title = mk_title("提示");
        tips = mk_vmerr_tips("启动虚机失败！", "Bold 13", "虚机启动配置文件丢失，请联系锐捷400售后维修热线寻求人工帮助。", 13);
        action_area = mk_action_area("确定");
        break;
     case UI_DIALOG_INTERACTIVE_SHOW_DRIVER_ERR:
        title = mk_title("提示");
        if (data) {
            sprintf(buf_tips, "当前选定的镜像操作系统为%s，当前终端不支持“Windows 7” “Windows 10”以外的镜像进行驱动安装，请联系管理员确认WEB上的绑定镜像是否正确。", (char *)data);
        } else {
            strcpy(buf_tips, "当前终端不支持“Windows 7” “Windows 10”以外的镜像进行驱动安装，请联系管理员确认WEB上的绑定镜像是否正确。");
        }
        tips = mk_vmerr_tips("启动虚机失败！", "Bold 13", buf_tips, 13);
        action_area = mk_action_area("确定");
        break;
     case UI_DIALOG_INTERACTIVE_SHOW_CDISK_ERR:
        title = mk_title("提示");
        tips = mk_tips("系统错误！请联系管理员", 17);
        timer_cnter = mk_timer(5, "后返回", 17);
        action_area = mk_action_area("返回");
        break;
     case UI_DIALOG_INTERACTIVE_SHOW_LAYER_TEMPLETE_ERR:
        title = mk_title("提示");
        tips = mk_tips("应用集中更新模式开启失败，请联系管理员。", 16);
        action_area = mk_action_area("确定");
        break;
     case UI_DIALOG_INTERACTIVE_SHOW_INTEL_NO_AUDIO_DEVICE:
        title = mk_title("提示");
        tips = mk_tips("音频设备未识别到，请尝试断电重启。", 16);
        action_area = mk_action_area("确定");
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_EASYDEPLOY_NOT_WIFI:
        title = mk_title("提示");
        tips = mk_tips("WIFI连接下不支持本地网络配置", 17);
        timer_cnter = mk_timer(5, "后返回", 17);
        action_area = mk_action_area("返回");
        break;
    case UI_DIALOG_INTERACTIVE_SHOW_NOTSUPPORT_XP:
        title = mk_title("提示");
        tips = mk_tips("该终端不支持WinXP操作系统", 16);
        action_area = mk_action_area("确定");
        break;
     case UI_DIALOG_INTERACTIVE_SHOW_NOTSUPPORT_WIN10_32:
        title = mk_title("提示");
        tips = mk_tips("该终端不支持Win10 32位操作系统", 16);
        action_area = mk_action_area("确定");
        break;
    }
    if (title1){
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), title1, FALSE, FALSE, 0);
    }
    else if (title) {
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), title, FALSE, FALSE, 0);
    }

    if (tips && timer_cnter) {
        vbox = gtk_vbox_new(FALSE, 40);
        gtk_box_pack_start(GTK_BOX(vbox), tips, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), timer_cnter, FALSE, FALSE, 0);
        gtk_widget_show(vbox);
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), vbox, TRUE, FALSE, 0);
    } else if (tips) {
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), tips, TRUE, FALSE, 0);
    } else if (timer_cnter) {
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), timer_cnter, TRUE, FALSE, 0);
    }

    if (action_area) {
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), action_area, FALSE, FALSE, 20);
    }

    return 0;
}

static void ui_dialog_interactive_show(void)
{
    if (dialog == NULL) {
        ui_dialog_interactive_init();
    }
    gtk_widget_show(timer_hbox);
    gtk_widget_show(dialog);
}

static void ui_dialog_interactive_adapt(void)
{
    if (dialog) {
        if (ui_dialog_interactive.status == UI_STATUS_SHOW) {
            logi("ui_dialog_interactive_adapt\n");
            gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
        }
    }
}

static void ui_dialog_interactive_destroy(void)
{
    if (dialog) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
}


struct ui_comp_s ui_dialog_interactive = {
    .type = UI_TYPE_DIALOG_INTERACTIVE,
    .subtype = UI_TOPLEVEL_TYPE_DIALOG,
    .is_init = UI_NOT_NEED_INIT,
    .widget = &dialog,
    .init = ui_dialog_interactive_init,
    .show = ui_dialog_interactive_show,
    .hide = ui_dialog_interactive_hide,
    .ctrl = ui_dialog_interactive_ctrl,
    .adapt = ui_dialog_interactive_adapt,
    .destroy = ui_dialog_interactive_destroy,
};



