#include <gtk/gtk.h>
#include "ui_main.h"
#include "ui_win_main.h"

static GtkWidget *window;
static GtkWidget *fixed;
static GtkWidget * image;
static int width;

static GdkPixbuf *get_fill_bg_pixbuf(const GdkPixbuf *src_pixbuf, int _screen_width, int _screen_height)
{
    GdkPixbuf *tmp_pixbuf = NULL;
    GdkPixbuf *dst_pixbuf = NULL;
    int pic_width;
    int pic_height;

    pic_width  = gdk_pixbuf_get_width(src_pixbuf);
    pic_height = gdk_pixbuf_get_height(src_pixbuf);

    logi("src_pixbuf, pic_width=%d, pic_height=%d\n", pic_width, pic_height);

    if ((double)pic_width/pic_height > (double)_screen_width/_screen_height) {
        tmp_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf,
                                             _screen_height*pic_width/pic_height,
                                             _screen_height,
                                             GDK_INTERP_BILINEAR);

        if (tmp_pixbuf == NULL)
            return NULL;

        dst_pixbuf = gdk_pixbuf_new_subpixbuf(tmp_pixbuf,
                                              (_screen_height*pic_width/pic_height-_screen_width)/2,
                                              0,
                                              _screen_width,
                                              _screen_height);
        logi("subpixbuf, src_x = %d\n", (_screen_height*pic_width/pic_height-_screen_width)/2);
        g_object_unref(tmp_pixbuf);
    } else if ((double)pic_width/pic_height < (double)_screen_width/_screen_height) {
        tmp_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf,
                                             _screen_width,
                                             _screen_width*pic_height/pic_width,
                                             GDK_INTERP_BILINEAR);

         if (tmp_pixbuf == NULL)
             return NULL;

        dst_pixbuf = gdk_pixbuf_new_subpixbuf(tmp_pixbuf,
                                              0,
                                              (_screen_width*pic_height/pic_width-_screen_height)/2,
                                              _screen_width,
                                              _screen_height);
        logi("subpixbuf, src_y = %d\n", (_screen_width*pic_height/pic_width-_screen_height)/2);
        g_object_unref(tmp_pixbuf);
    } else {
        dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf,
                                             _screen_width,
                                             _screen_height,
                                             GDK_INTERP_BILINEAR);
        logi("simple as original graph\n");
    }

    return dst_pixbuf;
}

void ui_window_set_bg(GtkWidget *src_widget, int type, int s_width, int s_height)
{
    GdkPixbuf *src_pixbuf = NULL;
    GdkPixbuf *dst_pixbuf = NULL;
    GtkStyle *style;
    GdkPixmap *pixmap;
    char image_path[128] = {0};
    int algorithm = 0;

    if (type) {
        if (!access(UI_USER_BG, F_OK)) {
            strcpy(image_path, UI_USER_BG);
            algorithm = 1;
        } else {
            strcpy(image_path, "./icon/bg.png");
            algorithm = 0;
        }
    } else {
        strcpy(image_path, "./icon/bg.png");
        algorithm = 0;
    }

    logi("algorithm = %d, path = %s\n", algorithm, image_path);
    src_pixbuf = gdk_pixbuf_new_from_file(image_path, NULL);
    if (algorithm) {
        /* Fill-in centering algorithm */
        dst_pixbuf = get_fill_bg_pixbuf(src_pixbuf, s_width, s_height);
    } else {
        dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, s_width, s_height, GDK_INTERP_BILINEAR);
    }

    if (src_pixbuf)
        g_object_unref(src_pixbuf);

    if (dst_pixbuf == NULL) {
        logi("dst_pixbuf is NULL\n");
        return;
    }

    pixmap = gdk_pixmap_new(NULL, s_width, s_height, 24);
    gdk_pixbuf_render_pixmap_and_mask(dst_pixbuf, &pixmap, NULL, 0);
    g_object_unref(dst_pixbuf);
    style = gtk_style_copy(GTK_WIDGET(src_widget)->style);

    if (style->bg_pixmap[GTK_STATE_NORMAL])
        g_object_unref(style->bg_pixmap[GTK_STATE_NORMAL]);

    style->bg_pixmap[GTK_STATE_NORMAL] = g_object_ref(pixmap);
    style->bg_pixmap[GTK_STATE_ACTIVE] = g_object_ref(pixmap);
    style->bg_pixmap[GTK_STATE_PRELIGHT] = g_object_ref(pixmap);
    style->bg_pixmap[GTK_STATE_SELECTED] = g_object_ref(pixmap);
    style->bg_pixmap[GTK_STATE_INSENSITIVE] = g_object_ref(pixmap);

    gtk_widget_set_style(GTK_WIDGET(src_widget), style);
    g_object_unref(pixmap);
    g_object_unref(style);

    return;
}

void ui_widget_set_logo(GtkWidget *src_widget, int type)
{
    GdkPixbuf *src_pixbuf;
    char image_path[128] = {0};

    if (type) {
        if (!access(UI_USER_LOGO, F_OK)) {
            strcpy(image_path, UI_USER_LOGO);
        } else {
            strcpy(image_path, "./icon/logo_a.png");
        }
    } else {
        strcpy(image_path, "./icon/logo_a.png");
    }

    src_pixbuf = gdk_pixbuf_new_from_file(image_path, NULL);
    if (type == UI_CALL_TYPE_INIT) {
        width = gdk_pixbuf_get_width(src_pixbuf);
        image = gtk_image_new_from_pixbuf(src_pixbuf);
        ui_win_put_nice_position(image, 0, width, 4.167);
        gtk_widget_show(image);
    } else {
        int _width = gdk_pixbuf_get_width(src_pixbuf);
        gtk_image_set_from_pixbuf(GTK_IMAGE(src_widget), src_pixbuf);
        ui_win_put_nice_position(src_widget, 1, _width, 4.167);
        gtk_widget_show(src_widget);
    }
    g_object_unref(src_pixbuf);

    return;
}

static int ui_win_main_init()
{
	//GdkPixbuf * src_pixbuf = NULL;
	//GtkWidget * image;
	//GtkWidget * label;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    //gtk_window_set_title(GTK_WINDOW(window), "Center");
    gtk_window_set_decorated(GTK_WINDOW(window),FALSE);
    gtk_window_resize(GTK_WINDOW(window), g_win_manager.r_x, g_win_manager.r_y); //FIX ME
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect_swapped(G_OBJECT(window), "destroy",
                                G_CALLBACK(gtk_main_quit), NULL);
    // set background pic , feixiushi
    ui_window_set_bg(window, UI_CALL_TYPE_INIT, g_win_manager.r_x, g_win_manager.r_y);
    // fix me: g_signal_connect_swapped(G_OBJECT(windows), "show",  G_CALLBACK(ui_widget_show));

    fixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(window), fixed);

    g_win_manager.window = window;
    g_win_manager.win_fixed = fixed;

    ui_widget_set_logo(NULL, UI_CALL_TYPE_INIT);

    return 0;
}

static int ui_win_main_ctrl(void *data)
{
    ui_msg_t *msg = data;

    if (msg == NULL)
        return 0;

    switch (msg->sub_obj) {
    case UI_WIN_MAIN_SET_LOGO:
        ui_widget_set_logo(image, UI_CALL_TYPE_SET);
        break;
    case UI_WIN_MAIN_RESET_LOGO:
        ui_widget_set_logo(image, UI_CALL_TYPE_RESET);
        break;
    case UI_WIN_MAIN_SET_BG:
        ui_window_set_bg(window, UI_CALL_TYPE_SET, g_win_manager.r_x, g_win_manager.r_y);
        break;
    case UI_WIN_MAIN_RESET_BG:
        ui_window_set_bg(window, UI_CALL_TYPE_RESET, g_win_manager.r_x, g_win_manager.r_y);
        break;
    }

    return 0;
}

void ui_win_main_show()
{
    gtk_widget_show(fixed);
    gtk_widget_show(window);
}

void ui_win_main_destroy(void)
{
    if (fixed) {
        gtk_widget_destroy(fixed);
        fixed = NULL;
    }

    if (window) {
        gtk_widget_destroy(window);
        window = NULL;
    }
}

void ui_win_main_adapt()
{
    if (window) {
        gtk_window_resize(GTK_WINDOW(window), g_win_manager.r_x, g_win_manager.r_y); 
        gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER_ALWAYS);
        //update_widget_bg(window, "./icon/bg.png", g_win_manager.r_x, g_win_manager.r_y);
        ui_window_set_bg(window, UI_CALL_TYPE_SET, g_win_manager.r_x, g_win_manager.r_y);
    }

    if (image) {
        ui_win_put_nice_position(image, 1, width, 4.167);
    }
}

struct ui_comp_s ui_win_main = {
    .type = UI_TYPE_WIN_MAIN,
    .subtype = UI_TOPLEVEL_TYPE_WIN,
    .is_init = UI_NEED_INIT,
    .widget = &window,
    .init = ui_win_main_init,
    .show = ui_win_main_show,
    .destroy = ui_win_main_destroy,
    .ctrl = ui_win_main_ctrl,
    .adapt = ui_win_main_adapt,
};

