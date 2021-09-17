#include "ui_main.h"
#include "ui_win_auth_btnbox.h"

#define AUTH_TIP_CONNECTED  "已认证"
#define AUTH_TIP_DISCONNECT "未认证"
#define AUTH_TIP_CONNETING "认证中"

GtkWidget *auth_btnbox_bg = NULL;
GtkWidget *auth_btnbox = NULL;
btn_apend_t *auth_button = NULL;
GtkWidget *auth_bubble = NULL;
static int auth_status_ctrl = UI_AUTH_STATUS_SUCCESS;

static void ui_win_auth_btnbox_show(void);
static void ui_win_auth_btnbox_destroy(void);
static int ui_win_auth_btnbox_init(void);

void bubble_normal_show(int auth_status)
{
    int status = UI_AUTH_STATUS_SUCCESS;

    if (auth_bubble == NULL) {
        logi("auth_bubble is null\n");
        return;
    }

    if (ui_tab[UI_TYPE_WIN_AUTH_BTNBOX]->status != UI_STATUS_SHOW
        || ui_tab[UI_TYPE_DIALOG_AUTHPWD]->status == UI_STATUS_SHOW) {
        logi("auth_status %d btn status: %d , authpwd status: %d auth_status_ctrl = %d\n",
            auth_status, ui_tab[UI_TYPE_WIN_AUTH_BTNBOX]->status, ui_tab[UI_TYPE_DIALOG_AUTHPWD]->status, auth_status_ctrl);
        return;
    }

    //status = ui_extern_get_auth_status();
    //TODO: 规避设置ip的时候无法调用ui_extern_get_auth_status()
    if (auth_status == UI_AUTH_STATUS_REQUEST) {
        status = auth_status_ctrl;
    } else {
        status = auth_status;
    }

    switch (status) {
    case UI_AUTH_STATUS_SUCCESS:
    case UI_AUTH_STATUS_AUTHING:
    case UI_AUTH_ENV_NOEXIST:
        gtk_widget_hide(auth_bubble);
        break;
    case UI_AUTH_STATUS_UNNECESSARY:
    case UI_AUTH_STATUS_FAILED:
    case UI_AUTH_STATUS_OTHER:
    case UI_AUTH_ENV_EXIST:
    default:
        gtk_widget_show(auth_bubble);
        break;
    }
}

void ui_win_authbtn_disable(void)
{
    if(auth_button) {
        set_btn_effect(auth_button, FALSE);
    }
}

void ui_win_authbtn_enable(void)
{
    if(auth_button) {
        set_btn_effect(auth_button, TRUE);
    }
}

static void reflash_auth_button(btn_apend_t *object, char *text, char *icon, char *icon_mouse_on)
{
    GdkPixbuf *src_pixbuf = NULL;
    GdkPixbufAnimation* anim_pixbuf = NULL;

    if(icon) {
        if (strstr(icon, ".gif") != NULL) {
            anim_pixbuf = gdk_pixbuf_animation_new_from_file(icon, NULL);
            gtk_image_set_from_animation(GTK_IMAGE(object->image), anim_pixbuf);
            g_object_unref(anim_pixbuf);
        } else {
            src_pixbuf = gdk_pixbuf_new_from_file(icon, NULL);
            gtk_image_set_from_pixbuf(GTK_IMAGE(object->image), src_pixbuf);
            g_object_unref(src_pixbuf);

        }
    }

    if (icon_mouse_on) {
        if (strstr(icon_mouse_on, ".gif") != NULL) {
            anim_pixbuf = gdk_pixbuf_animation_new_from_file(icon_mouse_on, NULL);
            gtk_image_set_from_animation(GTK_IMAGE(object->mouse_on), anim_pixbuf);
            g_object_unref(anim_pixbuf);
        } else {
            src_pixbuf = gdk_pixbuf_new_from_file(icon_mouse_on, NULL);
            gtk_image_set_from_pixbuf(GTK_IMAGE(object->mouse_on), src_pixbuf);
            g_object_unref(src_pixbuf);
        }
    }

    if (text) {
        gtk_label_set_text(GTK_LABEL(object->label), text);
    }

}

static void ui_win_auth_btnbox_hide(void)
{

    logi("%s\n", __func__);
    if (auth_btnbox) {
        ui_win_authbtn_enable();
        btn_normal_hide(auth_button);
        gtk_widget_hide(auth_btnbox);
    }

    if(auth_bubble) {
        gtk_widget_hide(auth_bubble);
    }
}

static void ui_win_auth_btnbox_bg_show(void)
{
    if (auth_btnbox_bg) {
        gtk_widget_show(auth_btnbox_bg);
    }
}

static void ui_win_auth_btnbox_bg_hide(void)
{
    if (auth_btnbox_bg) {
        gtk_widget_hide(auth_btnbox_bg);
    }
}

static int ui_win_auth_btnbox_ctrl(void *data)
{
    ui_msg_t *msg = NULL;

    msg = (ui_msg_t *)data;
    if (!msg) {
        return -1;
    }

    if (auth_btnbox == NULL) {
        if (msg->sub_obj == UI_AUTH_ENV_EXIST) {
            ui_win_auth_btnbox_init();
        }
    }

    logi("%s auth status %d\n", __func__, msg->sub_obj);
    if (auth_button && auth_btnbox) {
        ui_win_auth_btnbox_bg_hide();
        switch (msg->sub_obj) {
            case UI_WIN_AUTH_BTNBOX_ENABLE:
                ui_win_authbtn_enable();
                return 0;
            case UI_WIN_AUTH_BTNBOX_DISABLE:
                ui_win_authbtn_disable();
                return 0;
            case UI_AUTH_ENV_EXIST:
                reflash_auth_button(auth_button, AUTH_TIP_DISCONNECT, "./icon/auth/unauth_44x44.png", "./icon/auth/unauth_h_44x44.png");
                ui_win_auth_btnbox_show();
                msg->sub_obj = UI_AUTH_STATUS_FAILED;
                break;
            case UI_AUTH_ENV_NOEXIST:
                ui_win_auth_btnbox_destroy();
                break;
            case UI_AUTH_STATUS_SUCCESS:
                reflash_auth_button(auth_button, AUTH_TIP_CONNECTED, "./icon/auth/authenticated_44x44.png", "./icon/auth/authenticated_h_44x44.png");
                break;
            case UI_AUTH_STATUS_AUTHING:
                ui_win_auth_btnbox_bg_show();
                reflash_auth_button(auth_button, AUTH_TIP_CONNETING, "./icon/auth/authing_44x44.gif", "./icon/auth/authing_h_44x44.gif");
                break;
            case UI_AUTH_STATUS_UNNECESSARY:
            case UI_AUTH_STATUS_FAILED:
            case UI_AUTH_STATUS_OTHER:
            default:
                reflash_auth_button(auth_button, AUTH_TIP_DISCONNECT, "./icon/auth/unauth_44x44.png", "./icon/auth/unauth_h_44x44.png");
                break;
        }
       auth_status_ctrl = msg->sub_obj;
       bubble_normal_show(msg->sub_obj);
    }
    return 0;
}

static void ui_win_auth_btnbox_destroy(void)
{
    if (auth_btnbox) {
        gtk_widget_destroy(auth_btnbox);
        auth_btnbox_bg = NULL;
        auth_button = NULL;
        auth_btnbox = NULL;
    }

    if(auth_bubble) {
        gtk_widget_destroy(auth_bubble);
        auth_bubble = NULL;
    }
}

static void ui_win_auth_btnbox_adapt(void)
{
    if (auth_btnbox) {
        ui_win_authbtn_move_greate_pos(auth_btnbox, 60, 11.9, 1);
        if(ui_win_auth_btnbox.status == UI_STATUS_SHOW) {
            gtk_widget_hide(auth_btnbox);
            gtk_widget_show(auth_btnbox);

            if (auth_bubble) {
                ui_win_authbtn_move_greate_pos(auth_bubble, 198, 50, 1);
                gtk_widget_hide(auth_bubble);
                bubble_normal_show(UI_AUTH_STATUS_REQUEST);
            }
        }
    }
}

static void ui_win_auth_btnbox_show(void)
{
    if (auth_button) {
        btn_normal_show(auth_button);
    }

    if (auth_btnbox) {
        gtk_widget_show(auth_btnbox);
    }
}

static gint ui_win_button_click_auth(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    ui_extern_enter_auth_config();
    ui_manager_publogin_timeout_disable();
    ui_manager_offline_autologin_timer_disable();
    ui_settype_timer_disable();
    if (auth_bubble) {
        gtk_widget_hide(auth_bubble);
    }
    ui_tab[UI_TYPE_DIALOG_AUTHPWD]->show();
    return 0;
}

static GtkWidget * mk_auth_btnbox_bg(char * icon)
{
    GtkWidget * align;
    GtkWidget *image;
    GdkPixbuf *src_pixbuf = NULL;

    src_pixbuf = gdk_pixbuf_new_from_file(icon, NULL);
    image = gtk_image_new_from_pixbuf(src_pixbuf);
    g_object_unref(src_pixbuf);
    gtk_widget_show(image);

    align = gtk_alignment_new(0.5,0.5,0,0);
    gtk_container_add(GTK_CONTAINER(align), image);

    return align;
}

static GtkWidget * mk_bubble_tip(char * icon, char * icon_h, int width, int height)
{
    GtkWidget * align;
    GtkWidget *image;
    GdkPixbuf *src_pixbuf = NULL;

    src_pixbuf = gdk_pixbuf_new_from_file(icon, NULL);
    image = gtk_image_new_from_pixbuf(src_pixbuf);
    g_object_unref(src_pixbuf);
    gtk_widget_show(image);

    align = gtk_alignment_new(0.5,0,0,0);
    gtk_container_add(GTK_CONTAINER(align), image);

    return align;
}
#if 0
static void ui_win_authbtn_load(int auth_status)
{
    ui_win_auth_btnbox_bg_hide();
    switch (auth_status) {
    case UI_AUTH_STATUS_SUCCESS:
        auth_button = create_button_above_label("./icon/auth/authenticated_44x44.png", "./icon/auth/authenticated_h_44x44.png", NULL, AUTH_TIP_CONNECTED, GTK_SIGNAL_FUNC(ui_win_button_click_auth), NULL);
        break;
    case UI_AUTH_STATUS_AUTHING:
        ui_win_auth_btnbox_bg_show();
        auth_button = create_button_above_label("./icon/auth/authing_h_44x44.gif", "./icon/auth/authing_h_44x44.gif", NULL, AUTH_TIP_CONNETING, GTK_SIGNAL_FUNC(ui_win_button_click_auth), NULL);
        break;
    case UI_AUTH_STATUS_UNNECESSARY:
    case UI_AUTH_STATUS_FAILED:
    case UI_AUTH_STATUS_OTHER:
    default:
        auth_button = create_button_above_label("./icon/auth/unauth_44x44.png", "./icon/auth/unauth_h_44x44.png", NULL, AUTH_TIP_DISCONNECT, GTK_SIGNAL_FUNC(ui_win_button_click_auth), NULL);
        break;
    }

}
#endif

static int ui_win_auth_btnbox_init()
{
    auth_btnbox = gtk_fixed_new();
    g_signal_connect(G_OBJECT(auth_btnbox), "realize", 
                        G_CALLBACK(ui_compt_realize), &ui_win_auth_btnbox);

    auth_btnbox_bg = mk_auth_btnbox_bg("./icon/auth/auth_btnbox_bg_44x44.png");
    auth_button = create_button_above_label("./icon/auth/unauth_44x44.png", "./icon/auth/unauth_h_44x44.png", NULL, AUTH_TIP_DISCONNECT, GTK_SIGNAL_FUNC(ui_win_button_click_auth), NULL);
    set_widget_font_size(auth_button->label, 13, "Snow");
    gtk_fixed_put(GTK_FIXED(auth_btnbox), auth_button->container, 0, 0);
    gtk_fixed_put(GTK_FIXED(auth_btnbox), auth_btnbox_bg, 3, 0);

    auth_bubble = mk_bubble_tip("./icon/auth/auth_bubble_187x36.png", "./icon/auth/auth_bubble_h_187x36.png", -1, -1 );
    ui_win_authbtn_move_greate_pos(auth_bubble, 198, 50, 0);
    ui_win_authbtn_move_greate_pos(auth_btnbox, 60, 11.9, 0);
    return 0;
}

struct ui_comp_s ui_win_auth_btnbox = {
    .type = UI_TYPE_WIN_AUTH_BTNBOX,
    .subtype = UI_TOPLEVEL_TYPE_DIALOG,
    .is_init = UI_NOT_NEED_INIT,
    .widget = &auth_btnbox,
    .init = ui_win_auth_btnbox_init,
    .show = ui_win_auth_btnbox_show,
    .adapt = ui_win_auth_btnbox_adapt,
    .destroy = ui_win_auth_btnbox_destroy,
    .ctrl = ui_win_auth_btnbox_ctrl,
    .hide = ui_win_auth_btnbox_hide,

    .height = 88.67,
};



