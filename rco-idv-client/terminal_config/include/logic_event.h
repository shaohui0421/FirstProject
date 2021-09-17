#ifndef _LOGICEVENT_H_
#define _LOGICEVENT_H_
#include <gtk/gtk.h>

enum EVENT_TYPE {
    UI_SHOW_SERIAL_BEGIN,        /*显示编号开始事件*/
    UI_SHOW_SERIAL_WAIT,         /*显示编号等待事件*/
    UI_SHOW_SERIAL_SUCCESS,      /*设置编号成功事件*/
    UI_SHOW_SERIAL_ERROR,        /*设置编号异常事件*/
    UI_SHOW_SERIAL_SHOW,         /*显示编号数值事件*/
    UI_SHOW_SERIAL_CLOSE,        /*执行编号结束事件*/
    UI_SHOW_SERIAL_RESET,        /*执行编号重置事件*/
    UI_SHOW_SERIAL_TIMEOUT,      /*显示编号超时事件*/
    LOGIC_SET_TERM_CONF,         /*终端执行配置事件*/
    LOGIC_CLR_LAST_TIME,         /*计时器清除事件*/
    LOGIC_SEND_DEV_MAC,          /*发送本地MAC事件*/
    LOGIC_EVENTS_UNKNOW,
};

int logic_event_handle(int event_type, void *data);
const char *logic_event_desc(int event);

#endif

