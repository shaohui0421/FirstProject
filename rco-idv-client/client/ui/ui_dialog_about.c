#include "ui_main.h"
#ifdef IDV_CLIENT
#include "../../include/application_c_interfaces.h"
#endif

static GtkWidget *dialog;


#if 0
GtkWidget * mk_prompt(char *icon, char * lab_text, int lab_font_size)
{
	GtkWidget * align;
	GtkWidget * label = NULL;
	GtkWidget * hbox;
	btn_apend_t * btn_cnter = NULL;
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
#endif

GtkWidget * mk_logo()
{
	GtkWidget * align;
	GtkWidget *image;
    GdkPixbuf *src_pixbuf = NULL;

	src_pixbuf = gdk_pixbuf_new_from_file("./icon/about_logo.png", NULL);

    image = gtk_image_new_from_pixbuf(src_pixbuf);
    g_object_unref(src_pixbuf);
    gtk_widget_show(image);

	align = gtk_alignment_new(0.5,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), image);
    gtk_widget_show(align);
	return align;
}

static GtkWidget * mk_public_num(void)
{
    GtkWidget * vbox;
    GtkWidget * label;
    GtkWidget *image;
    GdkPixbuf *src_pixbuf = NULL;

    src_pixbuf = gdk_pixbuf_new_from_file("./icon/public_num.png", NULL);

    image = gtk_image_new_from_pixbuf(src_pixbuf);
    g_object_unref(src_pixbuf);
    gtk_widget_show(image);

    label = gtk_label_new("");
    //set_widget_font_size(label, lab_font_size, NULL);
    gtk_widget_show(label);

    vbox = gtk_vbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), image, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
    gtk_widget_show(vbox);
    return vbox;
}

static GtkWidget * mk_label_info(char *text)
{
	GtkWidget * label;
	GtkWidget * hbox;
    label = gtk_label_new(text);
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
    set_widget_font_size(label, 12, NULL);
    gtk_widget_show(label);
    
 	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);   
	gtk_widget_show(hbox);
	return hbox;
}

int str_replace(char strRes[],char from[], char to[]) {
    int i,flag = 0;
    char *p,*q,*ts;
    for(i = 0; strRes[i]; ++i) {
        if(strRes[i] == from[0]) {
            p = strRes + i;
            q = from;
            while(*q && (*p++ == *q++));
            if(*q == '\0') {
                ts = (char *)malloc(strlen(strRes) + 1);
                strcpy(ts,p);
                strRes[i] = '\0';
                strcat(strRes,to);
                strcat(strRes,ts);
                free(ts);
                flag = 1;
            }
        }
    }
    return flag;
}

static GtkWidget * mk_product_info(void)
{
	GtkWidget * vbox;
	GtkWidget * label;
    char str[9][100] = {0};
    int  i;
    unsigned int count = 0;
    int wifi_terminal = 0;
    GtkWidget * hbox;

    GtkWidget *labelLicense;
    GtkWidget *labelLicensePassed;
    GtkWidget *image;
    GdkPixbuf *pixbuf;
    

	vbox = gtk_vbox_new(FALSE, 10);
    ui_extern_is_wifi_terminal(&wifi_terminal);

#ifdef IDV_CLIENT
    struct about_info* info = NULL;
    info = malloc(sizeof(struct about_info));
    if (!info) {
        return NULL;
    }

    ui_get_about_info(info);
    snprintf(str[0], 100, "产品型号：%s", info->model);
    snprintf(str[1], 100, "硬件版本：%s", info->hw_ver);
    snprintf(str[2], 100, "软件版本：%s", info->sw_ver);
    snprintf(str[3], 100, "系统版本：%s", info->os_ver);
    str_replace(str[3],"RainOS","RainOS_OPS");
    snprintf(str[4], 100, "SN序列号：%s", info->sn);    
    if(wifi_terminal == 0) {
        snprintf(str[5], 100, "MAC地址：%s", info->mac);
        //strcpy(str[6], "");
    } else {
        snprintf(str[5], 100, "有线MAC：%s", info->mac);
        snprintf(str[6], 100, "无线MAC：%s", info->wlan_mac);
    }
    snprintf(str[7], 100, "终端名称：%s", info->hostname);
    snprintf(str[8], 100, "终端IP地址：%s", info->ip);
    free(info);
#else
    snprintf(str[0], 100, "产品型号：XXXXX");
    snprintf(str[1], 100, "硬件版本：XXXXX");
    snprintf(str[2], 100, "软件版本：!!!!!");
    snprintf(str[3], 100, "系统版本：XXXXX");
    snprintf(str[4], 100, "SN序列号：XXXXX");
    if(wifi_terminal == 0) {
        snprintf(str[5], 100, "MAC地址：XXXXX");
        //strcpy(str[6], "");
    } else {
        snprintf(str[5], 100, "有线MAC：XXXXX");
        snprintf(str[6], 100, "无线MAC：XXXXX");
    }
    snprintf(str[7], 100, "终端名称：XXXXX");
    snprintf(str[8], 100, "终端IP地址：XXXXX");

#endif
    //display about infos
    //ignore display wlan_mac if it is not a wifi terminal
    count = sizeof(str) / sizeof(str[0]);
    for (i = 0; i < count; i++) {
        if (strlen(str[i]) > 0) {
            label = mk_label_info(str[i]);
            gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
            /* after sn, display license status */
            if (i == 4) {
                pixbuf = gdk_pixbuf_new_from_file("./icon/client-licensed.png", NULL);
                image = gtk_image_new_from_pixbuf(pixbuf);
                g_object_unref(pixbuf);
                gtk_widget_show(image);

                labelLicense = gtk_label_new("授权信息：");
                set_widget_font_size(labelLicense, 12, NULL);
                gtk_widget_show(labelLicense);

                labelLicensePassed = gtk_label_new(" RG-CML-DESKTOP-IDV(已授权)");
                set_widget_font_size(labelLicensePassed, 12, NULL);
                gtk_widget_show(labelLicensePassed);

                hbox = gtk_hbox_new(FALSE, 0);
                gtk_box_pack_start(GTK_BOX(hbox), labelLicense, FALSE, FALSE, 0);
                gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
                gtk_box_pack_start(GTK_BOX(hbox), labelLicensePassed, FALSE, FALSE, 0);
                gtk_widget_show(hbox);

                gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
            }
        }
    }

	gtk_widget_show(vbox);
	return vbox;
}


static GtkWidget * mk_about()
{
    GtkWidget * align;
    GtkWidget * hbox;
    GtkWidget * pulic_num;
    GtkWidget * product_info = NULL;

    pulic_num = mk_public_num();
    product_info = mk_product_info();

    hbox = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), pulic_num, FALSE, FALSE, 10);
    if (product_info) {
        gtk_box_pack_start(GTK_BOX(hbox), product_info, FALSE, FALSE, 10);
    }
    gtk_widget_show(hbox);

    align = gtk_alignment_new(0.5,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);
    return align;
}


static GtkWidget * mk_end()
{
	GtkWidget * align;
	GtkWidget * hbox;

	GtkWidget * label;
	GtkWidget *image;
    GdkPixbuf *src_pixbuf = NULL;
	GdkPixbuf * dst_pixbuf = NULL;
	GtkWidget * vbox;

	src_pixbuf = gdk_pixbuf_new_from_file("./icon/ruijie.png", NULL);

    image = gtk_image_new_from_pixbuf(src_pixbuf);
    g_object_unref(src_pixbuf);
    gtk_widget_show(image);

    label = gtk_label_new("锐捷网络 © 2020");
    //set_widget_font_size(label, lab_font_size, NULL);
    gtk_widget_show(label);

	hbox = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);
	gtk_widget_show(hbox);

	align = gtk_alignment_new(0.5,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);
    
	src_pixbuf = gdk_pixbuf_new_from_file("./icon/shadow.png", NULL);
    dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, 840, 3, GDK_INTERP_BILINEAR);
	image = gtk_image_new_from_pixbuf(dst_pixbuf);
	g_object_unref(src_pixbuf);
	g_object_unref(dst_pixbuf);
	gtk_widget_show(image);    

	vbox = gtk_vbox_new(FALSE, 20);
	gtk_box_pack_start(GTK_BOX(vbox), image, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 0);
	gtk_widget_show(vbox);    
    
    return vbox;
}

static int ui_dialog_about_init(void)
{
	GtkWidget *title_bar;
     /* Create the widgets */
	GtkWidget *logo;
	GtkWidget *about;
	GtkWidget *end;
	GtkWidget * hide_label;
    dialog = gtk_dialog_new();
    
    g_signal_connect (G_OBJECT (dialog), "realize",
                      G_CALLBACK (ui_compt_realize), &ui_dialog_about);

    gtk_widget_set_size_request(dialog, 850, 580);

    ui_dialog_white_background(dialog);
    title_bar = title_bar_init("关于", dialog, 2);

    logo = mk_logo();
    about = mk_about();
    end = mk_end();
	hide_label = gtk_label_new("");
	gtk_widget_show(hide_label);
	
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), title_bar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), logo, FALSE, FALSE, 20);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), about, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hide_label, FALSE, FALSE, 20);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), end, FALSE, FALSE, 10);

    gtk_window_set_decorated(GTK_WINDOW(dialog),FALSE);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
    return 0;
}

static void ui_dialog_about_show(void)
{
    if (dialog == NULL) {
        ui_dialog_about_init();
    }

    gtk_widget_show(dialog);

}

static void ui_dialog_about_hide(void)
{
    if (dialog) {
        gtk_widget_hide(dialog);
    }
}

static void ui_dialog_about_adapt(void)
{
    if (dialog) {
        if(ui_dialog_about.status == UI_STATUS_SHOW) {
            logi("ui_dialog_about_adapt\n");
            gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
        }
    }
}

static void ui_dialog_about_destroy(void)
{
    if (dialog) {
        gtk_widget_destroy(dialog);
        dialog = NULL;
    }
}

struct ui_comp_s ui_dialog_about = {
    .type = UI_TYPE_DIALOG_ABOUT,
    .subtype = UI_TOPLEVEL_TYPE_DIALOG,
    .is_init = UI_NOT_NEED_INIT,
    .widget = &dialog,
    .init = ui_dialog_about_init,
    .show = ui_dialog_about_show,
    .hide = ui_dialog_about_hide,
    .adapt = ui_dialog_about_adapt,
    .destroy = ui_dialog_about_destroy,
};

