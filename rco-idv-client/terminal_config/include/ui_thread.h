#ifndef UITHREAD_H
#define UITHREAD_H
#include <gtk/gtk.h>

typedef struct ui_main {
    int       r_x;
    int       r_y;
    GtkWidget *window;
    GtkWidget *fixed;
    int r_refresh;
} ui_main_t;

ui_main_t g_window;

gboolean ui_thread_init(void);

#endif

