#include <pthread.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "ui_thread.h"
#include "rc_log.h"
#include "common.h"
#include "easydeployfixed.h"

static void set_widget_background(GtkWidget *widget, const char *img_file, int s_width, int s_height)
{
    GdkPixbuf *src_pixbuf;
    GdkPixbuf *dst_pixbuf;
    GdkPixmap *pixmap;
    GtkStyle *style;

    src_pixbuf = gdk_pixbuf_new_from_file(img_file, NULL);
    dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, s_width, s_height, GDK_INTERP_BILINEAR);
    pixmap = gdk_pixmap_new(NULL, s_width, s_height, 24);
    gdk_pixbuf_render_pixmap_and_mask(dst_pixbuf, &pixmap, NULL, 0); 

    style = gtk_style_copy(GTK_WIDGET(widget)->style);
    if (style->bg_pixmap[GTK_STATE_NORMAL])
        g_object_unref(style->bg_pixmap[GTK_STATE_NORMAL]);
    style->bg_pixmap[GTK_STATE_NORMAL]      = g_object_ref(pixmap);
    style->bg_pixmap[GTK_STATE_ACTIVE]      = g_object_ref(pixmap);
    style->bg_pixmap[GTK_STATE_PRELIGHT]    = g_object_ref(pixmap);
    style->bg_pixmap[GTK_STATE_SELECTED]    = g_object_ref(pixmap);
    style->bg_pixmap[GTK_STATE_INSENSITIVE] = g_object_ref(pixmap);
    gtk_widget_set_style(GTK_WIDGET(widget), style);

    g_object_unref(src_pixbuf);
    g_object_unref(dst_pixbuf);
    g_object_unref(pixmap);
    g_object_unref(style);
    return;
}

static void ui_screen_init(void)
{
    get_disply_info(&(g_window.r_x), &(g_window.r_y), &(g_window.r_refresh));
    if (g_window.r_x <= 0 || g_window.r_y <= 0) {
        g_window.r_x = gdk_screen_width();
        g_window.r_y = gdk_screen_height();
    }

    LOG_INFO("g_window.r_x:%d", g_window.r_x);
    LOG_INFO("g_window.r_y:%d", g_window.r_y);

    if (g_window.r_x == 1280 && g_window.r_y == 720) {
    } else {
        if (g_window.r_x < 1024) {
            g_window.r_x = 1024;
        }
        if (g_window.r_y < 768) {
            g_window.r_y = 768;
        }
    }

    return;
}

static void ui_window_init(void)
{
    g_window.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    set_widget_background(g_window.window, ICONPATH "background.png", g_window.r_x, g_window.r_y);
    gtk_window_set_position(GTK_WINDOW(g_window.window), GTK_WIN_POS_CENTER);
    gtk_widget_set_size_request(GTK_WIDGET(g_window.window), g_window.r_x, g_window.r_x);
    gtk_window_set_resizable(GTK_WINDOW(g_window.window), TRUE);
    gtk_window_set_decorated(GTK_WINDOW(g_window.window), FALSE);
    gtk_widget_show(g_window.window);
    return;
}

static void ui_widget_init(void)
{
    g_window.fixed = easydeploy_fixed_new();
    gtk_container_add(GTK_CONTAINER(g_window.window), g_window.fixed);
    easydeploy_fixed_show(g_window.fixed);
    return;
}

static void ui_main( int argc, char *argv[])
{
    LOG_INFO("ui show");
    gtk_init(&argc, &argv);
    ui_screen_init();
    ui_window_init();
    ui_widget_init();
    gtk_main();
    LOG_INFO("ui quit");
    return;
}

static void* ui_thread(void* data)
{
    pthread_detach(pthread_self());
    set_thread_name("ui monitor");

    struct arg_t *arg = NULL;

    memset(&g_window, 0, sizeof(ui_main_t));
    arg = (struct arg_t *)data;
    ui_main(arg->argc, arg->argv);

    return NULL;
}

gboolean ui_thread_init(void)
{
    pthread_t thread_id;
    int ret;

    ret = pthread_create(&thread_id, NULL, ui_thread, (void *)&arg);
    if (ret) {
        LOG_ERR("pthread create failed, error:%d(%s)", errno, strerror(errno));
        return FALSE;
    }
    return TRUE;
}

