#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
#include "rc_log.h"
#include "common.h"
#include "easydeployfixed.h"
#include "ui_thread.h"
#include "logic_event.h"
#include "logic_status.h"

#define WAITING_TIME 150

gboolean close_button_handle(GtkWidget *widget, GdkEventButton *event, gpointer* data)
{
    switch (event->button) {
    case 1:
        //TODO:CLOSE UI
        logic_event_handle(UI_SHOW_SERIAL_CLOSE, NULL);
        break;
    default:
        break;
    }
    return FALSE;
}

gboolean enter_button_handle(GtkWidget *widget, GdkEventButton *event, gpointer *data)
{
    EasydeployFixed *easydeployfixed;

    if (data == NULL) {
        LOG_ERR("data is NULL");
        return FALSE;
    }
    easydeployfixed = (EasydeployFixed *)data;

    if (easydeployfixed->beginSerialFixed == NULL) {
        LOG_WARNING("beginSerialFixed is NULL");
        return FALSE;
    }

    switch (event->type) {
    case GDK_BUTTON_RELEASE:
        //TODO:SHOW WAIT UI
        logic_event_handle(UI_SHOW_SERIAL_WAIT, NULL);
        //TODO:SEND MACADDR
        g_idle_add((GSourceFunc)send_thread_init, NULL);
        break;
    default:
        break;
    }

    return FALSE;
}

gboolean enter_key_handle(GtkWidget *widget, GdkEventKey *event, gpointer *data)
{
    EasydeployFixed *easydeployfixed;

    if (data == NULL) {
        LOG_ERR("data is NULL");
        return FALSE;
    }
    easydeployfixed = (EasydeployFixed *)data;

    if (easydeployfixed->beginSerialFixed == NULL) {
        LOG_WARNING("beginSerialFixed is NULL");
        return FALSE;
    }

    switch (event->keyval) {
    case GDK_KP_Enter:
    case GDK_Return:
        //TODO:SHOW WAIT UI
        logic_event_handle(UI_SHOW_SERIAL_WAIT, NULL);
        //TODO:SEND MACADDR
        g_idle_add((GSourceFunc)send_thread_init, NULL);
        break;
    default:
        break;
    }

    return FALSE;
}

gboolean wait_timeout_handle(gpointer* data)
{
    EasydeployFixed *easydeployfixed;

    if (data == NULL) {
        LOG_ERR("data is NULL");
        return FALSE;
    }
    easydeployfixed = (EasydeployFixed *)data;

    easydeployfixed->wait_time    = 0;
    easydeployfixed->serial_state = 0;
    easydeployfixed->serial_num   = 0;
    easydeployfixed->timer1 = 0;
    //TODO:SHOW ENTER
    logic_event_handle(UI_SHOW_SERIAL_RESET, NULL);
    return FALSE;
}

gboolean wait_success_handle(gpointer* data)
{
    EasydeployFixed *easydeployfixed;

    if (data == NULL) {
        LOG_ERR("data is NULL");
        return FALSE;
    }
    easydeployfixed = (EasydeployFixed *)data;
    easydeployfixed->timer2 = 0;
    //TODO:SHOW END UI
    logic_event_handle(UI_SHOW_SERIAL_SHOW, NULL);
    return FALSE;
}

gboolean wait_error_handle(gpointer* data)
{
    EasydeployFixed *easydeployfixed;

    if (data == NULL) {
        LOG_ERR("data is NULL");
        return FALSE;
    }
    easydeployfixed = (EasydeployFixed *)data;

    easydeployfixed->wait_time    = 0;
    easydeployfixed->serial_state = 0;
    easydeployfixed->serial_num   = 0;
    easydeployfixed->timer3 = 0;
    //TODO:SHOW ENTER
    logic_event_handle(UI_SHOW_SERIAL_RESET, NULL);
    return FALSE;
}

gboolean wait_for_handle(gpointer* data)
{
    EasydeployFixed *easydeployfixed;
    int serial_state;
    char msg[256];

    if (data == NULL) {
        LOG_ERR("data is NULL");
        return FALSE;
    }
    easydeployfixed = (EasydeployFixed *)data;

    if (easydeployfixed->wait_time > WAITING_TIME) {
        //TODO:TIMEOUT UI
        logic_event_handle(UI_SHOW_SERIAL_TIMEOUT, NULL);
        easydeployfixed->wait_time = 0;
        return TRUE;
    }

    serial_state = easydeployfixed->serial_state;
    switch (serial_state) {
    case SERIAL_NUM_WAITING:
        sprintf(msg, ICONPATH "notice-icon-loading-%d.png", ((easydeployfixed->wait_time)%8)+1);
        gtk_image_set_from_file(GTK_IMAGE(easydeployfixed->waitImage), msg);
        gtk_widget_show(easydeployfixed->waitImage);
        easydeployfixed->wait_time++;
        LOG_INFO("serial num wait:%d", easydeployfixed->wait_time);
        return TRUE;
    case SERIAL_NUM_TIMEOUT:
        gtk_image_set_from_file(GTK_IMAGE(easydeployfixed->waitImage), ICONPATH "notice-icon-light.png");
        gtk_label_set_markup(GTK_LABEL(easydeployfixed->waitLable),
                             "<span foreground='#FF0000' font_desc='15' >连接超时，请稍后重试</span>");
        gtk_widget_set_size_request(easydeployfixed->waitLable, 220, 60);
        gtk_fixed_move(GTK_FIXED(easydeployfixed->mainFixed), easydeployfixed->waitSerialFixed,
                      (easydeployfixed->r_x-(60+10+220))/2, (easydeployfixed->r_y-60)/2);
        gtk_widget_show(easydeployfixed->waitImage);
        gtk_widget_show(easydeployfixed->waitLable);
        easydeployfixed->timer1 = g_timeout_add(1000, (GSourceFunc)wait_timeout_handle, (gpointer *)data);
        LOG_INFO("serial num timeout");
        break;
    case SERIAL_NUM_SUCCESS:
        gtk_image_set_from_file(GTK_IMAGE(easydeployfixed->waitImage), ICONPATH "notice-icon-ok.png");
        gtk_label_set_markup(GTK_LABEL(easydeployfixed->waitLable),
                             "<span foreground='#FFFFFF' font_desc='15' >编号成功</span>");
        gtk_widget_set_size_request(easydeployfixed->waitLable, 100, 60);
        gtk_fixed_move(GTK_FIXED(easydeployfixed->mainFixed), easydeployfixed->waitSerialFixed,
                      (easydeployfixed->r_x-(60+10+100))/2, (easydeployfixed->r_y-60)/2);
        gtk_widget_show(easydeployfixed->waitImage);
        gtk_widget_show(easydeployfixed->waitLable);
        easydeployfixed->timer2 = g_timeout_add(1000, (GSourceFunc)wait_success_handle, (gpointer *)data);
        LOG_INFO("serial num success");
        break;
    case SERIAL_NUM_ERROR:
        gtk_image_set_from_file(GTK_IMAGE(easydeployfixed->waitImage), ICONPATH "notice-icon-fail.png");
        gtk_label_set_markup(GTK_LABEL(easydeployfixed->waitLable),
                             "<span foreground='#FF0000' font_desc='15' >编号异常</span>");
        gtk_widget_set_size_request(easydeployfixed->waitLable, 100, 60);
        gtk_fixed_move(GTK_FIXED(easydeployfixed->mainFixed), easydeployfixed->waitSerialFixed,
                      (easydeployfixed->r_x-(60+10+100))/2, (easydeployfixed->r_y-60)/2);
        gtk_widget_show(easydeployfixed->waitImage);
        gtk_widget_show(easydeployfixed->waitLable);
        easydeployfixed->timer3 = g_timeout_add(1000, (GSourceFunc)wait_error_handle, (gpointer *)data);
        LOG_INFO("serial num error");
        break;
    default:
        break;
    }

    easydeployfixed->timer = 0;
    return FALSE;
}

static gint button_mouse_handle(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    btn_apend_t *btn_apend = (btn_apend_t *)data;

    switch(event->type)
    {
    case GDK_ENTER_NOTIFY:
        if (btn_apend->mouse_on) {
            gtk_widget_show(btn_apend->mouse_on);
        }

        if (btn_apend->image) {
            gtk_widget_hide(btn_apend->image);
        }
        break;
    case GDK_LEAVE_NOTIFY:
        if (btn_apend->mouse_on) {
            gtk_widget_hide(btn_apend->mouse_on);
        }

        if (btn_apend->image) {
            gtk_widget_show(btn_apend->image);
        }
        break;
    default:
        break;
    }

    return FALSE;
}

static void btn_free_event(GtkWidget *object, gpointer user_data)
{
    btn_apend_t *btn = user_data;
    if (btn != NULL) {
        g_free(btn);
    }
}

static void btn_normal_show(btn_apend_t *btn_apend)
{
    if (btn_apend->image) {
        gtk_widget_show(btn_apend->image);
    }

    if(btn_apend->label) {
        gtk_widget_show(btn_apend->label);
    }

    if (btn_apend->mouse_on) {
        gtk_widget_hide(btn_apend->mouse_on);
    }

    gtk_widget_show(btn_apend->container);
}

GtkWidget *create_image(const char *path, int width, int height, GdkInterpType type)
{
    GdkPixbuf *src_pixbuf;
    GdkPixbuf *dst_pixbuf;
    GtkWidget *image;
    int _width;
    int _height;

    _width = width;
    _height = height;

    src_pixbuf = gdk_pixbuf_new_from_file(path, NULL);
    if (width < 0 || height < 0) {
        _width = gdk_pixbuf_get_width(src_pixbuf);
        _height = gdk_pixbuf_get_height(src_pixbuf);
    }
    dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, _width, _height, type);
    image = gtk_image_new_from_pixbuf(dst_pixbuf);
    g_object_unref(src_pixbuf);
    g_object_unref(dst_pixbuf);
    gtk_widget_show(image);

    return image;
}


btn_apend_t *create_button(const char *icon, const char *icon_mouse_on,
                               const char *text, GCallback callback, void *data)
{
    btn_apend_t *btn_apend;
    GdkPixbuf *src_pixbuf;
    GtkWidget *eventbox;
    GtkWidget *fixed;
    GtkWidget *align;
    int set_width, set_height;

    btn_apend = g_malloc(sizeof(btn_apend_t));
    memset(btn_apend, 0, sizeof(btn_apend_t));

    fixed = gtk_fixed_new();

    src_pixbuf = gdk_pixbuf_new_from_file(icon, NULL);
    set_width = gdk_pixbuf_get_width(src_pixbuf);
    set_height = gdk_pixbuf_get_height(src_pixbuf);
    g_object_unref(src_pixbuf);

    btn_apend->image = gtk_image_new_from_file(icon);
    btn_apend->mouse_on = gtk_image_new_from_file(icon_mouse_on);

    if (text) {
        btn_apend->label = gtk_label_new(text);
        gtk_widget_set_size_request(btn_apend->label, set_width, set_height);
        gtk_label_set_justify(GTK_LABEL(btn_apend->label), GTK_JUSTIFY_CENTER);
    }

    gtk_widget_set_size_request(fixed, set_width, set_height);
    gtk_fixed_put(GTK_FIXED(fixed), btn_apend->image, 0, 0);
    gtk_fixed_put(GTK_FIXED(fixed), btn_apend->mouse_on, 0, 0);
    if (btn_apend->label) {
        gtk_fixed_put(GTK_FIXED(fixed), btn_apend->label, 0, 0);
    }

    eventbox = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(eventbox), FALSE);
    gtk_widget_set_size_request(eventbox, set_width, set_height);
    gtk_container_add(GTK_CONTAINER(eventbox), fixed);

    align = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), eventbox);
    btn_apend->container = align;

    gtk_widget_show(fixed);
    gtk_widget_show(eventbox);
    btn_normal_show(btn_apend);

    btn_apend->event = eventbox;
    if (callback) {
        g_signal_connect(G_OBJECT(eventbox), "button_press_event", G_CALLBACK(callback), (void*)data);
    }
    g_signal_connect(G_OBJECT(eventbox), "enter_notify_event", G_CALLBACK(button_mouse_handle), btn_apend);
    g_signal_connect(G_OBJECT(eventbox), "leave_notify_event", G_CALLBACK(button_mouse_handle), btn_apend);
    g_signal_connect(G_OBJECT(btn_apend->container), "destroy", G_CALLBACK(btn_free_event), btn_apend);

    return btn_apend;
}


GtkWidget *mk_begin_serial_bg(EasydeployFixed *easydeployfixed)
{
    GtkWidget *image;
    GtkWidget *fixed;
    GtkWidget *eventbox;
    GtkWidget *label;

    fixed = gtk_fixed_new();
    image = create_image(ICONPATH "set-num.jpg", easydeployfixed->r_x, easydeployfixed->r_y, GDK_INTERP_BILINEAR);

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<span foreground='#000000' font_desc='30' >请敲击回车键或单击鼠标对电脑编号</span>");
    gtk_widget_set_size_request(label, easydeployfixed->r_x, 40);
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
    gtk_widget_show(label);

    easydeployfixed->closebtn1 = create_button(ICONPATH "header-btn-close.png",
                                               ICONPATH "header-btn-close-hover.png",
                                               NULL,
                                               G_CALLBACK(close_button_handle),
                                               (void *)easydeployfixed);

    gtk_fixed_put(GTK_FIXED(fixed), image, 0, 0);
    gtk_fixed_put(GTK_FIXED(fixed), easydeployfixed->closebtn1->container, easydeployfixed->r_x-30, 0);
    gtk_fixed_put(GTK_FIXED(fixed), label, 0, (easydeployfixed->r_y-40)/2);

    eventbox = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(eventbox), FALSE);
    gtk_widget_set_size_request(eventbox, easydeployfixed->r_x, easydeployfixed->r_y);
    gtk_container_add(GTK_CONTAINER(eventbox), fixed);

    g_signal_connect(G_OBJECT(eventbox), "button_release_event", G_CALLBACK(enter_button_handle), (void*)easydeployfixed);

    gtk_widget_show(fixed);
    gtk_widget_show(eventbox);

    return eventbox;
}

GtkWidget *mk_wait_serial_bg(EasydeployFixed *easydeployfixed)
{
    GtkWidget *fixed;

    easydeployfixed->serial_state = SERIAL_NUM_WAITING;

    fixed = gtk_fixed_new();
    easydeployfixed->waitImage = create_image(ICONPATH "notice-icon-loading-1.png", -1, -1, GDK_INTERP_BILINEAR);

    easydeployfixed->waitLable = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(easydeployfixed->waitLable), "<span foreground='#FFFFFF' font_desc='15' >正在编号，请稍候...</span>");
    gtk_widget_set_size_request(easydeployfixed->waitLable, 200, 60);
    gtk_misc_set_alignment(GTK_MISC(easydeployfixed->waitLable), 0, 0.5);
    gtk_label_set_justify(GTK_LABEL(easydeployfixed->waitLable), GTK_JUSTIFY_LEFT);
    gtk_widget_show(easydeployfixed->waitLable);

    gtk_fixed_put(GTK_FIXED(fixed), easydeployfixed->waitImage, 0, 0);
    gtk_fixed_put(GTK_FIXED(fixed), easydeployfixed->waitLable, 60+10, 0);
    easydeployfixed->timer = g_timeout_add(100, (GSourceFunc)wait_for_handle, (gpointer *)easydeployfixed);
    gtk_widget_show(fixed);

    return fixed;
}

GtkWidget *mk_wait_serial_btn(EasydeployFixed *easydeployfixed)
{
    easydeployfixed->closebtn1 = create_button(ICONPATH "header-btn-close.png",
                                               ICONPATH "header-btn-close-hover.png",
                                               NULL,
                                               G_CALLBACK(close_button_handle),
                                               (void *)easydeployfixed);
    return easydeployfixed->closebtn1->container;
}

GtkWidget *mk_end_serial_bg(EasydeployFixed *easydeployfixed, int serial_num)
{
    GtkWidget *image;
    GtkWidget *fixed;
    GtkWidget *label;
    GtkWidget *number;
    char info[256];
    int font_desc = 200;

    fixed = gtk_fixed_new();
    image = create_image(ICONPATH "begin_serial.png", -1, -1, GDK_INTERP_BILINEAR);

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<span foreground='#FFFFFF' font_desc='46' >编号成功,这台电脑编号是</span>");
    gtk_widget_set_size_request(label, easydeployfixed->r_x, 60);
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
    gtk_widget_show(label);

    number = gtk_label_new(NULL);
    if (serial_num > 99)
        font_desc = 150;
    sprintf(info, "<span foreground='#1b7ee0' font_desc='%d' >%d</span>", font_desc, serial_num);
    gtk_label_set_markup(GTK_LABEL(number), info);
    gtk_widget_set_usize(number, 500, 500);
    gtk_label_set_justify(GTK_LABEL(number), GTK_JUSTIFY_CENTER);
    gtk_widget_show(number);

    easydeployfixed->closebtn2 = create_button(ICONPATH "header-btn-close.png",
                                               ICONPATH "header-btn-close-hover.png",
                                               NULL,
                                               G_CALLBACK(close_button_handle),
                                               (void *)easydeployfixed);

    gtk_fixed_put(GTK_FIXED(fixed), image,  (easydeployfixed->r_x-500)/2, 100+180*easydeployfixed->r_x*easydeployfixed->r_y/(1920*1080));
    gtk_fixed_put(GTK_FIXED(fixed), number, (easydeployfixed->r_x-500)/2, 100+180*easydeployfixed->r_x*easydeployfixed->r_y/(1920*1080));
    gtk_fixed_put(GTK_FIXED(fixed), easydeployfixed->closebtn2->container, easydeployfixed->r_x-30, 0);
    gtk_fixed_put(GTK_FIXED(fixed), label, 0, 120*easydeployfixed->r_x*easydeployfixed->r_y/(1920*1080));
    gtk_widget_show(fixed);

    return fixed;
}

static void easydeploy_fixed_data_init(EasydeployFixed *easydeployfixed)
{
    easydeployfixed->r_x = g_window.r_x;
    easydeployfixed->r_y = g_window.r_y;
    easydeployfixed->wait_time    = 0;
    easydeployfixed->serial_num   = 0;
    easydeployfixed->serial_state = 0;
    easydeployfixed->timer  = 0;
    easydeployfixed->timer1 = 0;
    easydeployfixed->timer2 = 0;
    easydeployfixed->timer3 = 0;
    easydeployfixed->mainFixed = NULL;
    easydeployfixed->beginSerialFixed = NULL;
    easydeployfixed->waitSerialFixed = NULL;
    return;
}

static void easydeploy_fixed_class_init(EasydeployFixedClass *easydeployfixedclass)
{
    return;
}

void easydeploy_fixed_init(EasydeployFixed *easydeployfixed)
{
    easydeploy_fixed_data_init(easydeployfixed);
    gtk_widget_set_size_request(GTK_WIDGET(easydeployfixed), easydeployfixed->r_x, easydeployfixed->r_y);
    easydeployfixed->mainFixed = gtk_fixed_new();
    easydeployfixed->beginSerialFixed = mk_begin_serial_bg(easydeployfixed);
    gtk_fixed_put(GTK_FIXED(easydeployfixed->mainFixed), easydeployfixed->beginSerialFixed, 0, 0);
    gtk_container_add(GTK_CONTAINER(easydeployfixed), easydeployfixed->mainFixed);
    gtk_widget_show(easydeployfixed->mainFixed);

    easydeployfixed->signal_id = g_signal_connect(G_OBJECT(g_window.window), "key_release_event",
                                                  G_CALLBACK(enter_key_handle), (void*)easydeployfixed);
    return;
}

GtkType easydeploy_fixed_get_type(void)
{
    static GType easydeploy_fixed_type = 0;
    if (!easydeploy_fixed_type)
    {
        const GTypeInfo easydeploy_fixed_info = {
            sizeof(EasydeployFixedClass),
            NULL,
            NULL,
            (GClassInitFunc)easydeploy_fixed_class_init,
            NULL,
            NULL,
            sizeof(EasydeployFixed),
            0,
            (GInstanceInitFunc)easydeploy_fixed_init,
        };
        easydeploy_fixed_type = g_type_register_static(GTK_TYPE_FIXED,
                                                       "EasydeployFixed",
                                                       &easydeploy_fixed_info,
                                                       0);
    }

    return easydeploy_fixed_type;
}

GtkWidget* easydeploy_fixed_new(void)
{
    return GTK_WIDGET(g_object_new(easydeploy_fixed_get_type(), 0));
}

void easydeploy_fixed_show(GtkWidget *widget)
{
    if (widget == NULL) {
        LOG_ERR("widget is NULL");
        return;
    }
    gtk_widget_show(widget);
    return;
}

void easydeploy_fixed_destory(GtkWidget *widget)
{
    EasydeployFixed *easydeployfixed;

    if (widget == NULL) {
        LOG_ERR("widget is NULL");
        return;
    }
    easydeployfixed = EASYDEPLOY_FIXED(widget);

    if (easydeployfixed != NULL) {
        if ((easydeployfixed->signal_id)) {
            g_signal_handler_disconnect(G_OBJECT(g_window.window), easydeployfixed->signal_id);
            easydeployfixed->signal_id = 0;
        }
        if (easydeployfixed->timer) {
            g_source_remove(easydeployfixed->timer);
            easydeployfixed->timer = 0;
        }
        if (easydeployfixed->timer1) {
            g_source_remove(easydeployfixed->timer1);
            easydeployfixed->timer1 = 0;
        }
        if (easydeployfixed->timer2) {
            g_source_remove(easydeployfixed->timer2);
            easydeployfixed->timer2 = 0;
        }
        if (easydeployfixed->timer3) {
            g_source_remove(easydeployfixed->timer3);
            easydeployfixed->timer3 = 0;
        }
        if (easydeployfixed->mainFixed) {
            gtk_widget_destroy(easydeployfixed->mainFixed);
            easydeployfixed->mainFixed = NULL;
        }
    }
    gtk_widget_destroy(widget);
    return;
}

void easydeploy_fixed_begin2wait(GtkWidget *widget)
{
    EasydeployFixed *easydeployfixed;

    if (widget == NULL) {
        LOG_ERR("widget is NULL");
        return;
    }

    easydeployfixed = EASYDEPLOY_FIXED(widget);
    if (easydeployfixed == NULL) {
        LOG_ERR("easydeployfixed is NULL");
        return;
    }

    if (easydeployfixed->beginSerialFixed == NULL) {
        LOG_ERR("beginSerialFixed is NULL");
        return;
    }

    gtk_widget_destroy(easydeployfixed->beginSerialFixed);
    easydeployfixed->beginSerialFixed = NULL;
    easydeployfixed->waitSerialFixed  = mk_wait_serial_bg(easydeployfixed);
    easydeployfixed->waitSerialButton = mk_wait_serial_btn(easydeployfixed);
    gtk_fixed_put(GTK_FIXED(easydeployfixed->mainFixed), easydeployfixed->waitSerialButton,
                   easydeployfixed->r_x-30, 0);
    gtk_fixed_put(GTK_FIXED(easydeployfixed->mainFixed), easydeployfixed->waitSerialFixed,
                  (easydeployfixed->r_x-(60+10+200))/2, (easydeployfixed->r_y-60)/2);
    return;
}

void easydeploy_fixed_wait2begin(GtkWidget *widget)
{
    EasydeployFixed *easydeployfixed;

    if (widget == NULL) {
        LOG_ERR("widget is NULL");
        return;
    }

    easydeployfixed = EASYDEPLOY_FIXED(widget);
    if (easydeployfixed == NULL) {
        LOG_ERR("easydeployfixed is NULL");
        return;
    }

    if (easydeployfixed->waitSerialFixed == NULL) {
        LOG_ERR("waitSerialFixed is NULL");
        return;
    }

    if (easydeployfixed->waitSerialButton == NULL) {
        LOG_ERR("waitSerialButton is NULL");
        return;
    }

    gtk_widget_destroy(easydeployfixed->waitSerialFixed);
    easydeployfixed->waitSerialFixed = NULL;
    gtk_widget_destroy(easydeployfixed->waitSerialButton);
    easydeployfixed->waitSerialButton = NULL;
    easydeployfixed->beginSerialFixed = mk_begin_serial_bg(easydeployfixed);
    gtk_fixed_put(GTK_FIXED(easydeployfixed->mainFixed), easydeployfixed->beginSerialFixed, 0, 0);

    return;
}

void easydeploy_fixed_wait2end(GtkWidget *widget)
{
    EasydeployFixed *easydeployfixed;

    if (widget == NULL) {
        LOG_ERR("widget is NULL");
        return;
    }

    easydeployfixed = EASYDEPLOY_FIXED(widget);
    if (easydeployfixed == NULL) {
        LOG_ERR("easydeployfixed is NULL");
        return;
    }

    if (easydeployfixed->waitSerialFixed == NULL) {
        LOG_ERR("waitSerialFixed is NULL");
        return;
    }

    if (easydeployfixed->waitSerialButton == NULL) {
        LOG_ERR("waitSerialButton is NULL");
        return;
    }

    gtk_widget_destroy(easydeployfixed->waitSerialFixed);
    easydeployfixed->waitSerialFixed = NULL;
    gtk_widget_destroy(easydeployfixed->waitSerialButton);
    easydeployfixed->waitSerialButton = NULL;
    easydeployfixed->endSerialFixed = mk_end_serial_bg(easydeployfixed, easydeployfixed->serial_num);
    gtk_fixed_put(GTK_FIXED(easydeployfixed->mainFixed), easydeployfixed->endSerialFixed, 0, 0);
    return;
}

void easydeploy_fixed_set_num(GtkWidget *widget, int num)
{
    EasydeployFixed *easydeployfixed;

    if (widget == NULL) {
        LOG_ERR("widget is NULL");
        return;
    }
    easydeployfixed = (EasydeployFixed *)widget;
    if (easydeployfixed)
        easydeployfixed->serial_num = num;
    return;
}

void easydeploy_fixed_set_state(GtkWidget *widget, int state)
{
    EasydeployFixed *easydeployfixed;

    if (widget == NULL) {
        LOG_ERR("widget is NULL");
        return;
    }
    easydeployfixed = (EasydeployFixed *)widget;
    if (easydeployfixed)
        easydeployfixed->serial_state = state;
    return;
}

int easydeploy_fixed_get_state(GtkWidget *widget)
{
    EasydeployFixed *easydeployfixed;
    int state = -1;

    if (widget == NULL) {
        LOG_ERR("widget is NULL");
        return -1;
    }
    easydeployfixed = (EasydeployFixed *)widget;
    if (easydeployfixed)
        state = easydeployfixed->serial_state;
    return state;
}


