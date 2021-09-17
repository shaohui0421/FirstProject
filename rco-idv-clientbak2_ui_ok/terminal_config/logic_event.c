#include <gtk/gtk.h>
#include "logic_event.h"
#include "ui_thread.h"
#include "easydeployfixed.h"
#include "rc_log.h"
#include "logic_status.h"
#include "common.h"

const static char *LogicEventDesc[] = {
    "SERIAL BEGIN EVENT",
    "SERIAL WAIT EVENT",
    "SERIAL SUCCESS EVENT",
    "SERIAL ERROR EVENT",
    "SERIAL SHOW EVENT",
    "SERIAL CLOSE EVENT",
    "SERIAL RESET EVENT",
    "SERIAL TIMEOUT EVENT",
    "CONFIG INFO EVENT",
    "CLEAR TIME EVENT",
    "SEND MACADDR EVENT",
    "UNKNOW LOGIC EVENT",
};

const char *logic_event_desc(int event)
{
    if (event >= UI_SHOW_SERIAL_BEGIN && event < LOGIC_EVENTS_UNKNOW)
        return LogicEventDesc[event];
    else
        return LogicEventDesc[LOGIC_STATE_UNKNOW];
}

gboolean ui_thread_quit(void)
{
    if (g_window.fixed) {
        easydeploy_fixed_destory(g_window.fixed);
        g_window.fixed = NULL;
    }

    if (g_window.window) {
        gtk_widget_destroy(g_window.window);
        g_window.window = NULL;
    }

    while(gtk_events_pending()) {
        gtk_main_iteration();
    }
    gtk_main_quit();

    return FALSE;
}

gboolean ui_serial_reset(void)
{
    easydeploy_fixed_wait2begin(g_window.fixed);
    return FALSE;
}

gboolean ui_serial_wait(void)
{
    easydeploy_fixed_begin2wait(g_window.fixed);
    return FALSE;
}

gboolean ui_serial_show(void)
{
    easydeploy_fixed_wait2end(g_window.fixed);
    return FALSE;
}

gboolean ui_serial_success(void)
{
    easydeploy_fixed_set_state(g_window.fixed, SERIAL_NUM_SUCCESS);
    return FALSE;
}

gboolean ui_serial_error(void)
{
    easydeploy_fixed_set_state(g_window.fixed, SERIAL_NUM_ERROR);
    return FALSE;
}

gboolean ui_serial_timeout(void)
{
    easydeploy_fixed_set_state(g_window.fixed, SERIAL_NUM_TIMEOUT);
    return FALSE;
}

void ui_serial_set(int *num)
{
    easydeploy_fixed_set_num(g_window.fixed, *num);
    return;
}

void logic_set_config(EASYDEPLOY *easy_deploy)
{
    if (gtk_main_level()) {
        set_logic_status(LOGIC_STATE_INIT);
        g_idle_add((GSourceFunc)ui_thread_quit, NULL);
        sleep(1);
    }
    group_set_conf(easy_deploy);
    return;
}

void logic_clear_timer(void)
{
    msg_timer_init();
    return;
}

void logic_send_mac(char *info)
{
    send_msg(sendfd, info, strlen(info));
    return;
}

void logic_ui_init(void)
{
    if (gtk_main_level()) {
        g_idle_add((GSourceFunc)ui_thread_quit, NULL);
        sleep(1);
    }
    ui_thread_init();
    return;
}

int logic_event_handle(int event_type, void *data)
{
    int _event_type;

    /* do ui event depend on gtk main */
    if (event_type != UI_SHOW_SERIAL_BEGIN && \
        event_type != LOGIC_SET_TERM_CONF  && \
        event_type != LOGIC_CLR_LAST_TIME  && \
        event_type != LOGIC_SEND_DEV_MAC) {
        if (!gtk_main_level()) {
            LOG_WARNING("event is ignored, gtk main quit");
            return FALSE;
        }
    }

    /* if vm running do not handle events */
    if (is_vm_running()) {
        LOG_WARNING("event is ignored, vm is running");
        return FALSE;
    }

    _event_type = logic_status_handle(event_type);
    logic_status_change(_event_type);
    switch (_event_type) {
    case UI_SHOW_SERIAL_BEGIN:
        logic_ui_init();
        break;
    case UI_SHOW_SERIAL_WAIT:
        g_idle_add((GSourceFunc)ui_serial_wait, NULL);
        break;
    case UI_SHOW_SERIAL_SHOW:
        g_idle_add((GSourceFunc)ui_serial_show, NULL);
        break;
    case UI_SHOW_SERIAL_CLOSE:
        g_idle_add((GSourceFunc)ui_thread_quit, NULL);
        break;
    case UI_SHOW_SERIAL_RESET:
        g_idle_add((GSourceFunc)ui_serial_reset, NULL);
        break;
    case UI_SHOW_SERIAL_SUCCESS:
        ui_serial_set(data);
        g_idle_add((GSourceFunc)ui_serial_success, NULL);
        break;
    case UI_SHOW_SERIAL_ERROR:
        g_idle_add((GSourceFunc)ui_serial_error, NULL);
        break;
    case UI_SHOW_SERIAL_TIMEOUT:
        g_idle_add((GSourceFunc)ui_serial_timeout, NULL);
        break;
    case LOGIC_SET_TERM_CONF:
        logic_set_config(data);
        break;
    case LOGIC_CLR_LAST_TIME:
        logic_clear_timer();
        break;
    case LOGIC_SEND_DEV_MAC:
        logic_send_mac(data);
        break;
    default:
        return FALSE;
    }

    return TRUE;
}


