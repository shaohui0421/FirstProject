#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ui_main.h"
#include "ui_dialog_config.h"
#include "ui_test.h"
#ifdef IDV_CLIENT
#include "../../include/application_c_interfaces.h"
#include "rc/rc_netif.h"
#include "rc/rc_checknetifval.h"

#endif


//GtkWidget * ping_server_ctner;

#define		PING_STR	         "开始Ping"
#define		CANCEL_STR	         "取消"
#define		CHECK_STR	         "开始检测"
#define     DOMAIN_NAME_REGEX    "^[a-zA-Z0-9][-a-zA-Z0-9]{0,62}(\\.[a-zA-Z0-9][-a-zA-Z0-9]{0,62})+$"

static btn_apend_t  * ping_btn = NULL;
static btn_apend_t  * check_btn = NULL;

static GtkWidget * text_view_ping;
static GtkWidget * text_view_check;
static GtkWidget * ping_entry;
static GtkWidget * check_entry;

static GtkWidget * ping_err;
static GtkWidget * ping_err_hide;

#ifdef IDV_CLIENT
static void * ping_handle;
static void * check_handle;
#endif

static void clean_text_view(GtkWidget *text_view)
{
    GtkTextIter start, end;
    GtkTextBuffer *buffer;

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_get_bounds (buffer, &start, &end );
    gtk_text_buffer_delete (buffer, &start, &end);
}

#ifdef IDV_CLIENT
static void add_text_view(GtkWidget *text_view, const char *str, int size)
{
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_get_end_iter (buffer, &iter);
    gtk_text_buffer_insert (buffer, &iter, str, size);
}

static void err_box_show(GtkWidget * err_prompt, GtkWidget * err_prompt_hide)
{
    gtk_widget_show(err_prompt);
    gtk_widget_hide(err_prompt_hide);
}

static void err_box_hide(GtkWidget * err_prompt, GtkWidget * err_prompt_hide)
{
    gtk_widget_show(err_prompt_hide);
    gtk_widget_hide(err_prompt);
}

static GtkWidget * vbox_pack_err_prompt(GtkWidget * item, GtkWidget * err_prompt, GtkWidget ** expand_widget)
{
    GtkWidget *vbox;
    GtkWidget * hid_label;

    if (expand_widget) {
        hid_label = gtk_label_new("");
        gtk_widget_show(hid_label);
        *expand_widget = hid_label;
    }

    gtk_widget_hide(err_prompt);
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), item, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), err_prompt, FALSE, FALSE, 0);
    if (expand_widget) {
        gtk_box_pack_start(GTK_BOX(vbox), hid_label, FALSE, FALSE, 0);
    }

    gtk_widget_show(vbox);
    return vbox;
}

static void reset_ping_btn()
{
	if (g_strcmp0(gtk_label_get_text(GTK_LABEL(ping_btn->label)), CANCEL_STR) == 0 ) {
		gtk_label_set_text(GTK_LABEL(ping_btn->label), PING_STR);
	}
}

static void reset_check_btn()
{
	if (g_strcmp0(gtk_label_get_text(GTK_LABEL(check_btn->label)), CANCEL_STR) == 0 ) {
		gtk_label_set_text(GTK_LABEL(check_btn->label), CHECK_STR);
	}
}

typedef struct {
    struct cb_opration_data data;
    GtkWidget * text_view;
} text_msg_t;


static gboolean text_handle_msg_idle(gpointer user_data)
{
    text_msg_t *text_msg = user_data;
    struct cb_opration_data *result = &text_msg->data;
    GtkWidget * text_view = text_msg->text_view;

    if (text_view != text_view_ping && text_view != text_view_check) {
        logi("text_view not match!\n");
        return FALSE;
    }

    if (text_view == text_view_ping && ping_handle == NULL) {
        return FALSE;
    }
    
    if (text_view == text_view_check && check_handle == NULL) {
        return FALSE;
    }

    switch (result->opration) {
    case cb_opration_exit:
        if (text_view == text_view_ping) {
            reset_ping_btn();
        } else {
            reset_check_btn();
        }
        break;
    case cb_opration_retmsg:
        add_text_view(text_view, result->data, strlen(result->data));
        break;
    }

    if (result->data) {
        free(result->data);
    }
    free(text_msg);
    

    return FALSE;

}


static int text_msg_cb(void *private_data, struct cb_opration_data *result)
{
    text_msg_t *text_msg;

    text_msg = malloc(sizeof(text_msg_t));
    if (text_msg == NULL) {
        return -1;
    }
    memset(text_msg, 0, sizeof(text_msg_t));
    text_msg->data.opration = result->opration;
    if (result->data != NULL) {
        text_msg->data.data = strdup(result->data);
    }
    text_msg->text_view = private_data;

    g_idle_add(text_handle_msg_idle, text_msg);
	return 0;
}
#endif

static void ping_cancal_handle()
{
#ifdef IDV_CLIENT
    if (ping_handle != NULL) {
        rc_cancle_ping(ping_handle);
        ping_handle = NULL;
    }
#endif
}

static void check_cancal_handle()
{
#ifdef IDV_CLIENT
    if (check_handle != NULL) {
        rc_cancle_arping(check_handle);
        check_handle = NULL;
    }
#endif
}

static gint btn_ping_handle(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
    if (g_strcmp0(gtk_label_get_text(GTK_LABEL(ping_btn->label)), PING_STR) == 0) {
        err_box_hide(ping_err, ping_err_hide);
		/* try to do ping  */
		// clean view
		clean_text_view(text_view_ping);
		// do ping
		logi("ping addr:%s\n", gtk_entry_get_text(GTK_ENTRY(ping_entry)));
#ifdef IDV_CLIENT
        if (rc_check_ip_format_valid(gtk_entry_get_text(GTK_ENTRY(ping_entry))) < 0
                && ui_regex_match(gtk_entry_get_text(GTK_ENTRY(ping_entry)), DOMAIN_NAME_REGEX) != 0) {
            err_box_show(ping_err, ping_err_hide);
        } else {
            gtk_label_set_text(GTK_LABEL(ping_btn->label), CANCEL_STR);
            ping_handle = rc_request_ping((char *)gtk_entry_get_text(GTK_ENTRY(ping_entry)), text_msg_cb, text_view_ping);
        }
#endif
	} else {
		gtk_label_set_text(GTK_LABEL(ping_btn->label), PING_STR);

        /*  try to cancel check */
		ping_cancal_handle();

	}
	
    return FALSE;
}


static void cancal_ping(GtkWidget *widget, gpointer data)
{
    // if ping is doing , please stop it.
    printf("cancal_ping\n");
#ifdef IDV_CLIENT
    ping_cancal_handle();
#endif
}

static gint btn_check_handle(GtkWidget *widget,GdkEventButton *event, gpointer data)
{
	if (g_strcmp0(gtk_label_get_text(GTK_LABEL(check_btn->label)), CHECK_STR) == 0 ) {
		gtk_label_set_text(GTK_LABEL(check_btn->label), CANCEL_STR);
		// clean view
		clean_text_view(text_view_check);
		logi("check addr:%s\n", gtk_entry_get_text(GTK_ENTRY(check_entry)));
#ifdef IDV_CLIENT
		check_handle = rc_request_arping((char *)gtk_entry_get_text(GTK_ENTRY(check_entry)), text_msg_cb, text_view_check);
#endif		
	} else {
		gtk_label_set_text(GTK_LABEL(check_btn->label), CHECK_STR);

		check_cancal_handle();
	}
	
    return FALSE;
}


static void cancal_check(GtkWidget *widget, gpointer data)
{
    // if ping is doing , please stop it.
    printf("cancal_check\n");
#ifdef IDV_CLIENT
    check_cancal_handle();
#endif
}

static GtkWidget *mk_ping_prompt(char *icon, char * lab_text, int lab_font_size, gfloat x_align)
{
    GtkWidget * align;
    GtkWidget * label = NULL;
    GtkWidget * hbox;
    GtkWidget * image = NULL;
    GdkPixbuf * src_pixbuf = NULL;
    if(icon) {
        src_pixbuf = gdk_pixbuf_new_from_file(icon, NULL);
        image = gtk_image_new_from_pixbuf(src_pixbuf);
        g_object_unref(src_pixbuf);
        gtk_widget_show(image);
    }
    if(lab_text) {
        label = gtk_label_new(lab_text);
        set_widget_font_size(label, lab_font_size, NULL);
        gtk_widget_show(label);
    }
    hbox = gtk_hbox_new(FALSE, 5);
    if(image) {
        gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
    }
    if(label) {
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    }
    gtk_widget_show(hbox);

    align = gtk_alignment_new(x_align, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);

    return align;
}

static GtkWidget * mk_ping_area(void)
{
	GtkWidget * align;
    GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * label;
	GtkWidget * container;
	
	label = gtk_label_new("Ping服务");
	set_widget_font_size(label, 14, NULL);
	gtk_widget_show(label);

	ping_entry = gtk_entry_new_with_max_length(20);
	container = add_entry_style(ping_entry,  NULL,  "输入地址", 350, 30, TRUE);

    g_signal_connect(G_OBJECT(ping_entry),"focus",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    g_signal_connect(G_OBJECT(ping_entry),"button_press_event",G_CALLBACK(ui_win_pop_up_keyboard), NULL);
    ping_btn = create_button("./icon/btn_ping.png", "./icon/btn_ping_h.png", NULL, PING_STR,
    		-1, -1, G_CALLBACK (btn_ping_handle), ping_btn);

    ping_err = mk_ping_prompt("./icon/ico_abnormal_mm.png", "您输入的IP格式或者域名不正确！", 8, 0.25);

    set_widget_font_size(ping_btn->label, 12, "Snow");
    hbox = gtk_hbox_new(FALSE, 20);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), container, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), ping_btn->container, FALSE, FALSE, 0);
    
    gtk_widget_show(hbox);
    g_signal_connect (G_OBJECT (hbox), "destroy",
                      G_CALLBACK (cancal_ping), NULL);

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), vbox_pack_err_prompt(hbox, ping_err, &ping_err_hide), FALSE, FALSE, 0);
    gtk_widget_show(vbox);

	align = gtk_alignment_new(0.5,0, 0,0);
    gtk_container_add(GTK_CONTAINER(align), vbox);
    gtk_widget_show(align);
	return align;
}

static GtkWidget * mk_show_area(GtkWidget * text_view)
{
	GtkWidget * align;
	//GtkWidget * view;
	GtkWidget * scrolled_window;
	GtkWidget * hbox;

    gtk_text_view_set_editable (GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    //view = gtk_viewport_new(NULL, NULL);
    //gtk_viewport_set_shadow_type(GTK_VIEWPORT(view), GTK_SHADOW_NONE);
    //ui_dialog_white_background(view);

    gtk_widget_show(text_view);
    //gtk_container_add(GTK_CONTAINER (view), align);

    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    //ui_dialog_white_background(scrolled_window);
    //gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), GTK_SHADOW_NONE);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), GTK_SHADOW_IN);
    gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 0);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    
    gtk_widget_set_size_request(scrolled_window, 800, 400);
    gtk_container_add(GTK_CONTAINER (scrolled_window), text_view);
	
	gtk_widget_show(scrolled_window);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), scrolled_window, FALSE, FALSE, 0);
	gtk_widget_show(hbox);
    
	align = gtk_alignment_new(0.5,0, 0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
	gtk_widget_show(align);  

	return align;
}

GtkWidget *ui_config_ping_init(void)
{
	GtkWidget * vbox;
	GtkWidget * ping_area;
	GtkWidget * show_area;

	ping_area = mk_ping_area();
	text_view_ping = gtk_text_view_new();	
	show_area = mk_show_area(text_view_ping);

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), ping_area, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), show_area, FALSE, FALSE, 0);
    
    gtk_widget_show(vbox);
	
	return vbox;
}



static GtkWidget * mk_check_area(void)
{
	GtkWidget * align;
	GtkWidget * hbox;
	GtkWidget * label;
	GtkWidget * container;
	
	label = gtk_label_new("IP冲突检测服务");
	set_widget_font_size(label, 14, NULL);
	gtk_widget_show(label);

	check_entry = gtk_entry_new_with_max_length(20);
	container = add_entry_style(check_entry,  NULL,  "输入地址", 350, 30, TRUE);
	
    check_btn = create_button("./icon/btn_ping.png", "./icon/btn_ping_h.png", NULL, CHECK_STR,
    		-1, -1, G_CALLBACK (btn_check_handle), NULL);	
    
    set_widget_font_size(check_btn->label, 12, "Snow");	
    hbox = gtk_hbox_new(FALSE, 20);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), container, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), check_btn->container, FALSE, FALSE, 0);
    
    gtk_widget_show(hbox);
    g_signal_connect (G_OBJECT (hbox), "destroy",
                      G_CALLBACK (cancal_check), NULL);

	align = gtk_alignment_new(0.5,0, 0,0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
	gtk_widget_show(align);  
	return align;
}



GtkWidget *ui_config_ip_err_init(void)
{
	GtkWidget * vbox;
	GtkWidget * check_area;
	GtkWidget * show_area;

	check_area = mk_check_area();

	text_view_check = gtk_text_view_new();	
	show_area = mk_show_area(text_view_check);

    vbox = gtk_vbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), check_area, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), show_area, FALSE, FALSE, 5);
    
    gtk_widget_show(vbox);
	
	return vbox;
}


