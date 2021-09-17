#include <stdlib.h>
#include "ui_main.h"
#include "ui_dialog_download.h"

static GtkWidget *dialog, *vbox, * label_tips;
static GtkWidget *progress;
static GtkWidget *action_area;
static int mode, sub_mode;
static GtkWidget *label_speed;
static GtkWidget *status_vbox;
static GtkWidget *label_update;
//static btn_apend_t * retry_btn;
static GtkWidget *u_disk_area = NULL;

//static GtkWidget *retry_btn;

static void ui_dialog_download_hide()
{
    //ensure ui_ctrl can execute init2 and refresh title & guidebar
    mode = UI_DIALOG_DOWNLOAD_NONE;
    gtk_widget_hide(dialog);
}


gint ui_dialog_download_click_at_once(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    
    return 0;
}

gint ui_dialog_download_click_later(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    
    return 0;
}

static void ui_dialog_download_retry(GtkWidget *widget,gpointer data)
{
    ui_extern_download_retry();
}

static void ui_dialog_usb_copy_base(GtkWidget *widget,gpointer data)
{
    ui_extern_usb_copy_base();
}

static GtkWidget * mk_prompt(char *icon, char * lab_text, int lab_font_size, GtkWidget * widget)
{
	GtkWidget * align;
	GtkWidget * label = NULL;
	GtkWidget * hbox;
	GtkWidget * image = NULL;
	GdkPixbuf *src_pixbuf = NULL;
	
	if (icon) {
	    src_pixbuf = gdk_pixbuf_new_from_file(icon, NULL);
	    image = gtk_image_new_from_pixbuf(src_pixbuf);
	    g_object_unref(src_pixbuf);
	    gtk_widget_show(image);		
	}

	if (lab_text) {
	    label = gtk_label_new(lab_text);
	    set_widget_font_size(label, lab_font_size, NULL);
	    //gtk_widget_set_size_request(label, set_width, set_height);
	    gtk_widget_show(label);		
	}

    hbox = gtk_hbox_new(FALSE, 5);
    if (image) {
    	gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 10);
    }
    if (label) {
    	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 8);
    }

	if (widget)	{
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 4);
	}
	
	
	gtk_widget_show(hbox);

	align = gtk_alignment_new(0.5,0.5,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

	return align;
}


static GtkWidget * mk_action_area()
{
	GtkWidget * align;
	GtkWidget * btn_hbox;
	btn_apend_t * okay_button;
	btn_apend_t * cancal_button;

    okay_button = create_button("./icon/btn_green.png", "./icon/btn_green_h.png", NULL, "立即更新",
    		-1 ,-1, G_CALLBACK(ui_dialog_download_click_at_once), NULL);

    cancal_button = create_button("./icon/btn_gray.png", "./icon/btn_gray_h.png", NULL, "暂不更新",
    		-1, -1, G_CALLBACK(ui_dialog_download_click_later), NULL);

    btn_hbox = gtk_hbox_new(FALSE,15);
    gtk_box_pack_start(GTK_BOX(btn_hbox), okay_button->container, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(btn_hbox), cancal_button->container, FALSE, FALSE, 0);
    gtk_widget_show(btn_hbox);

	align = gtk_alignment_new(0.5,0.5,0,0);
    gtk_container_add(GTK_CONTAINER(align), btn_hbox);
    gtk_widget_show(align);
        
    return align;
}

static GtkWidget *mk_action_area2(int usb_copy)
{
	GtkWidget * vbox;
	GtkWidget * btn_hbox;
	GdkPixbuf * src_pixbuf = NULL;
	GdkPixbuf * dst_pixbuf = NULL;
	GtkWidget * image;
	GtkWidget * align;
    btn_apend_t  *usb_button;

    if(usb_copy == FALSE) {
        usb_button = create_button("./icon/btn_green.png", "./icon/btn_green_h.png", NULL, "U盘导入",
        		-1, -1, G_CALLBACK(ui_dialog_usb_copy_base), dialog);
        set_widget_font_size(usb_button->label, -1, "Snow");
        set_btn_effect(usb_button, TRUE);
    } else {
        usb_button = create_button("./icon/btn_gray.png", "./icon/btn_gray.png", NULL, "U盘导入",
        		-1, -1, NULL, dialog);
        set_widget_font_size(usb_button->label, -1, "#666666");
        set_btn_effect(usb_button, FALSE);
    }

    btn_hbox = gtk_hbox_new(FALSE, 30);
    gtk_box_pack_start(GTK_BOX(btn_hbox), usb_button->container, FALSE, FALSE, 5);
    gtk_widget_show(btn_hbox);

	align = gtk_alignment_new(0.5, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), btn_hbox);
    gtk_widget_show(align);

	src_pixbuf = gdk_pixbuf_new_from_file("./icon/shadow.png", NULL);
    dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, 830, 10, GDK_INTERP_BILINEAR);
	image = gtk_image_new_from_pixbuf(dst_pixbuf);
	g_object_unref(src_pixbuf);
	g_object_unref(dst_pixbuf);
	gtk_widget_show(image);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), image, FALSE, FALSE, 6);
	gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 5);
	gtk_widget_show(vbox);

    return vbox;
}

GtkWidget * mk_hbox_cntner(GtkWidget * widget, gboolean expand, gboolean fill)
{
	GtkWidget *hbox;
	//GtkWidget * align;
	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(hbox), widget, expand, fill, 40);
	gtk_widget_show(hbox);
	
	//align = gtk_alignment_new(0.0,0,0,0);
    //gtk_container_add(GTK_CONTAINER(align), hbox);
    //gtk_widget_show(align);
    	
	return hbox;
}

static void ui_dialog_download_init3()
{
	//GtkWidget * ctner;

	btn_apend_t * retry_btn;
	
    switch (sub_mode) {
    case UI_DIALOG_DOWNLOAD_ST_NORMAL:
        label_tips = mk_prompt("./icon/ico_clock.png", "正在下载，请耐心等待...", 20, NULL);
        break;
    case UI_DIALOG_DOWNLOAD_ST_CHECKING:
        label_tips = mk_prompt("./icon/ico_clock.png", "检查文件，请耐心等待...", 20, NULL);
        break;
    case UI_DIALOG_DOWNLOAD_ST_MERGE:
        label_tips = mk_prompt("./icon/ico_clock.png", "正在更新，请耐心等待...", 20, NULL);
        break;
    case UI_DIALOG_DOWNLOAD_ST_ERROR:
        retry_btn = create_button("./icon/btn_retry.png", "./icon/btn_retry_h.png", NULL,
					"重试", -1, -1, G_CALLBACK(ui_dialog_download_retry), NULL);
        label_tips = mk_prompt("./icon/ico_abnormal_m.png", "下载出错，请重试...", 20, retry_btn->container);
        break;
    case UI_DIALOG_DOWNLOAD_ST_INITING:
        label_tips = mk_prompt("./icon/ico_clock.png", "检查文件，请耐心等待...", 20, NULL);
        break;
    case UI_DIALOG_CP_BASE_ST_NORMAL:
        label_tips = mk_prompt("./icon/ico_clock.png", "拷贝镜像中，请耐心等待...", 20, NULL);
        break;
    case UI_DIALOG_CP_BASE_ST_ERROR:
        //retry_btn = create_button("./icon/btn_retry.png", "./icon/btn_retry_h.png", NULL,
		//			"重试", -1, -1, G_CALLBACK(ui_dialog_download_retry), NULL);
        label_tips = mk_prompt("./icon/ico_abnormal_m.png", "拷贝镜像出错...", 20, NULL);
        break;
    }
    gtk_widget_show(label_tips);
    gtk_box_pack_start(GTK_BOX(status_vbox), label_tips, FALSE, FALSE, 0);
}

static GtkWidget * mk_updating_gif(void)
{
	GtkWidget * vbox;
	GtkWidget * label;
	GtkWidget * image;
	GtkWidget * align;
	GdkPixbufAnimation* anim_pixbuf;
	
	label = gtk_label_new("正在更新...");
	set_widget_font_size(label, 15, NULL);
	gtk_widget_show(label);
	
	
	anim_pixbuf = gdk_pixbuf_animation_new_from_file("./icon/update.gif", NULL);
	image = gtk_image_new_from_animation(anim_pixbuf);
	g_object_unref(anim_pixbuf);
	gtk_widget_show(image);
	
	vbox = gtk_vbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), image, FALSE, FALSE, 5);
	
	gtk_widget_show(vbox);


	align = gtk_alignment_new(0.5,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), vbox);
    gtk_widget_show(align);
    
    return 	align;
}


static void ui_dialog_download_init2()
{    
    GtkWidget *title_bar = NULL;
    GtkWidget *guide_bar = NULL;
    GtkWidget *hide_label[2];
	//GtkWidget * item_hbox;
    int is_new_deploy = 0;

    vbox = gtk_vbox_new(FALSE,10);

    ui_extern_is_new_deploy(&is_new_deploy);
    logi("get ui_extern_is_new_deploy: %d\n", is_new_deploy);
    title_bar = title_bar_init(is_new_deploy ? "配置向导" : "系统更新", NULL, 1);
    gtk_box_pack_start(GTK_BOX(vbox), title_bar, FALSE, FALSE, 0);
    
    switch (mode) {
    case UI_DIALOG_DOWNLOAD_CHOICE:
        //XXX: it's not neccesary work?? wuyu
        hide_label[0] = gtk_label_new("");
        gtk_widget_show(hide_label[0]);
        gtk_box_pack_start(GTK_BOX(vbox), hide_label[0], FALSE, FALSE, 70);

        label_tips = mk_prompt("./icon/ico_prompt_m.png", "有新的系统需要更新", 20, NULL); 
        gtk_widget_show(label_tips);
        gtk_box_pack_start(GTK_BOX(vbox), label_tips, FALSE, FALSE, 10);

        hide_label[1] = gtk_label_new("");
        gtk_widget_show(hide_label[1]);
        gtk_box_pack_start(GTK_BOX(vbox), hide_label[1], FALSE, FALSE, 30);

        action_area = mk_action_area();
        gtk_box_pack_start (GTK_BOX(vbox), action_area, FALSE, FALSE, 0);
        break;
    case UI_DIALOG_DOWNLOAD_PROGRESS:
        if (is_new_deploy) {
            guide_bar = create_newdeploy_guide_bar(4);
            gtk_box_pack_start(GTK_BOX(vbox), guide_bar, FALSE, FALSE, 30);
        } else {
            hide_label[0] = gtk_label_new("");
            gtk_widget_show(hide_label[0]);
            gtk_box_pack_start(GTK_BOX(vbox), hide_label[0], FALSE, FALSE, 50);
        }
        status_vbox = gtk_vbox_new(FALSE, 0);
        ui_dialog_download_init3();
        gtk_widget_show(status_vbox);
        gtk_box_pack_start(GTK_BOX(vbox), status_vbox, FALSE, FALSE, 10);

    	//GtkWidget *hide_label[1];
        hide_label[1] = gtk_label_new("");
        gtk_widget_show(hide_label[1]);
        gtk_box_pack_start(GTK_BOX(vbox), hide_label[1], FALSE, FALSE, 10);

        label_speed = gtk_label_new("下载速度:未知");
        gtk_widget_show(label_speed);
        gtk_box_pack_start(GTK_BOX(vbox), mk_hbox_cntner(label_speed, FALSE, FALSE), FALSE, FALSE, 0);

        progress = gtk_progress_bar_new();
        gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(progress), 0.00);
        gtk_widget_show(progress);
        gtk_box_pack_start(GTK_BOX(vbox), mk_hbox_cntner(progress, TRUE, TRUE), FALSE, FALSE, 0);

        u_disk_area = mk_action_area2(FALSE);
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area), u_disk_area);

#if 0
        static GtkWidget *label_time;
        label_time = gtk_label_new("估计剩余时间");
        gtk_widget_show(label_time);
        gtk_box_pack_start(GTK_BOX(vbox), mk_hbox_cntner(label_time, FALSE, FALSE), FALSE, FALSE, 0);
#endif
        break;
    case UI_DIALOG_DOWNLOAD_UPDATEING:
    	
    	label_update = mk_updating_gif();
        //label_update =  gtk_label_new("updating");
        //gtk_widget_show(label_update);
        gtk_box_pack_start(GTK_BOX(vbox), label_update, TRUE, FALSE, 100);
        break;

    case UI_DIALOG_DOWNLOAD_INITING:
        if (is_new_deploy) {
            guide_bar = create_newdeploy_guide_bar(4);
            gtk_box_pack_start(GTK_BOX(vbox), guide_bar, FALSE, FALSE, 30);
        } else {
            hide_label[0] = gtk_label_new("");
            gtk_widget_show(hide_label[0]);
            gtk_box_pack_start(GTK_BOX(vbox), hide_label[0], FALSE, FALSE, 50);
        }
        status_vbox = gtk_vbox_new(FALSE, 0);
        ui_dialog_download_init3();
        gtk_widget_show(status_vbox);
        gtk_box_pack_start(GTK_BOX(vbox), status_vbox, FALSE, FALSE, 10);

        //GtkWidget *hide_label[1];
        hide_label[1] = gtk_label_new("");
        gtk_widget_show(hide_label[1]);
        gtk_box_pack_start(GTK_BOX(vbox), hide_label[1], FALSE, FALSE, 10);

        label_speed = gtk_label_new(" ");
        gtk_widget_show(label_speed);
        gtk_box_pack_start(GTK_BOX(vbox), mk_hbox_cntner(label_speed, FALSE, FALSE), FALSE, FALSE, 0);

        progress = gtk_progress_bar_new();
        gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(progress), 0.00);
        gtk_widget_show(progress);
        gtk_box_pack_start(GTK_BOX(vbox), mk_hbox_cntner(progress, TRUE, TRUE), FALSE, FALSE, 0);

        u_disk_area = mk_action_area2(TRUE);
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area), u_disk_area);

        break;

    case UI_DIALOG_USB_COPY_PROGRESS:
        if (is_new_deploy) {
            guide_bar = create_newdeploy_guide_bar(4);
            gtk_box_pack_start(GTK_BOX(vbox), guide_bar, FALSE, FALSE, 30);
        } else {
            hide_label[0] = gtk_label_new("");
            gtk_widget_show(hide_label[0]);
            gtk_box_pack_start(GTK_BOX(vbox), hide_label[0], FALSE, FALSE, 50);
        }
        status_vbox = gtk_vbox_new(FALSE, 0);
        ui_dialog_download_init3();
        gtk_widget_show(status_vbox);
        gtk_box_pack_start(GTK_BOX(vbox), status_vbox, FALSE, FALSE, 10);

    	//GtkWidget *hide_label[1];
        hide_label[1] = gtk_label_new("");
        gtk_widget_show(hide_label[1]);
        gtk_box_pack_start(GTK_BOX(vbox), hide_label[1], FALSE, FALSE, 10);

        label_speed = gtk_label_new("拷贝速度:未知");
        gtk_widget_show(label_speed);
        gtk_box_pack_start(GTK_BOX(vbox), mk_hbox_cntner(label_speed, FALSE, FALSE), FALSE, FALSE, 0);

        progress = gtk_progress_bar_new();
        gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(progress), 0.00);
        gtk_widget_show(progress);
        gtk_box_pack_start(GTK_BOX(vbox), mk_hbox_cntner(progress, TRUE, TRUE), FALSE, FALSE, 0);

        u_disk_area = mk_action_area2(TRUE);
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area), u_disk_area);
        break;
    }
    
    gtk_widget_show(vbox);
    gtk_box_pack_start(GTK_BOX (GTK_DIALOG (dialog)->vbox), vbox, FALSE, FALSE, 0);
}

static int ui_dialog_download_init(void)
{
     /* Create the widgets */
    dialog = gtk_dialog_new();
    
    g_signal_connect (G_OBJECT (dialog), "realize",
                      G_CALLBACK (ui_compt_realize), &ui_dialog_download);

    gtk_widget_set_size_request(dialog, 850, 580);

    ui_dialog_white_background(dialog);

    ui_dialog_download_init2();

    gtk_window_set_decorated(GTK_WINDOW(dialog),FALSE);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
    return 0;
}

static void ui_dialog_download_show(void)
{
    if (dialog == NULL) {
        ui_dialog_download_init();
        
    }
    gtk_widget_show(dialog);

}

static int ui_dialog_download_ctrl(void *arg)
{
    ui_msg_t                    *msg = arg;
    l2u_download_t              *args = NULL;
    char buf[20], str[100];
    int     old_mode;

    old_mode = mode;
    mode = msg->object;
    

    if (dialog) {
        if (old_mode != mode) {
            if (vbox) {
                gtk_widget_destroy(vbox);
                vbox = NULL;
            }
            /* bug:588699 */
            if (u_disk_area) {
                gtk_widget_destroy(u_disk_area);
                u_disk_area = NULL;
            }
            ui_dialog_download_init2();
        }
    } else {
        ui_dialog_download_init();
    }

    if (msg->object == UI_DIALOG_DOWNLOAD_PROGRESS || msg->object == UI_DIALOG_DOWNLOAD_INITING) {
        if (msg->args) {
            args = msg->args;
            if (sub_mode != args->status) {
                sub_mode = args->status;
                gtk_widget_destroy(label_tips);
                ui_dialog_download_init3();
            }
            snprintf(buf, 20, "%.2f%%", args->process);
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), args->process/100);
            gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), buf);
            if (sub_mode == UI_DIALOG_DOWNLOAD_ST_ERROR) {
                switch (args->err_code) {
                case UI_DIALOG_DOWNLOAD_ERR_UNKNOWN:
                    g_snprintf (str, 100, "出错原因:请联系管理员");
                    break;
                case UI_DIALOG_DOWNLOAD_ERR_TIMEOUT:
                    g_snprintf (str, 100, "出错原因:下载超时");
                    break;
                case UI_DIALOG_DOWNLOAD_ERR_NOSPACE:
                    g_snprintf (str, 100, "出错原因:磁盘空间不足");
                    break;
                case UI_DIALOG_DOWNLOAD_ERR_MD5:
                    g_snprintf (str, 100, "出错原因:MD5值校验失败");
                    break;           
                }
            } else {
                if (args->speed) {
                    if (sub_mode == UI_DIALOG_DOWNLOAD_ST_NORMAL) {
                        g_snprintf (str, 100, "下载速度:%s", args->speed);
                    }
                    free(args->speed);
                } else {
                    memset(str, 0, 100);
                    //g_snprintf (str, 100, "");
                }
            }
            gtk_label_set_text(GTK_LABEL(label_speed), str);
        }
        // EXTRA 
    }
    else if (msg->object == UI_DIALOG_USB_COPY_PROGRESS) {
        if (msg->args) {            
            args = msg->args;
            if (sub_mode != args->status) {
                sub_mode = args->status;
                gtk_widget_destroy(label_tips);
                ui_dialog_download_init3();
            }
            snprintf(buf, 20, "%.2f%%", args->process);
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), args->process/100);
            gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), buf);
            if (sub_mode == UI_DIALOG_DOWNLOAD_ST_ERROR) {
                switch (args->err_code) {
                case UI_DIALOG_CP_BASE_MOUNTED_DEV_NOT_EXIST:
                    g_snprintf (str, 100, "出错原因:USB设备不存在");
                    break;
                case UI_DIALOG_CP_BASE_UPDATE_STATUS_ERROR:
                    g_snprintf (str, 100, "出错原因:拷贝镜像出错");
                    break;                    
                }
            } else {
                if (args->speed) {
                    if (sub_mode == UI_DIALOG_CP_BASE_ST_NORMAL) {
                        g_snprintf (str, 100, "拷贝速度:%s", args->speed);
                    }
                    free(args->speed);
                } else {
                    memset(str, 0, 100);
                    //g_snprintf (str, 100, "");
                }
            }
            gtk_label_set_text(GTK_LABEL(label_speed), str);
        }
    }

    return 0;
}


static void ui_dialog_download_adapt(void)
{
    if (dialog) {
        if (ui_dialog_download.status == UI_STATUS_SHOW) {
            logi("ui_dialog_download_adapt\n");
            gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
        }
    }
}

static void ui_dialog_download_destroy(void)
{
    if (dialog) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
}

struct ui_comp_s ui_dialog_download = {
    .type = UI_TYPE_DIALOG_DOWNLOAD,
    .subtype = UI_TOPLEVEL_TYPE_DIALOG,
    .is_init = UI_NEED_INIT,
    .widget = &dialog,
    .init = ui_dialog_download_init,
    .show = ui_dialog_download_show,
    .hide = ui_dialog_download_hide,
    .ctrl = ui_dialog_download_ctrl,
    .destroy = ui_dialog_download_destroy,
    .adapt = ui_dialog_download_adapt,
};

