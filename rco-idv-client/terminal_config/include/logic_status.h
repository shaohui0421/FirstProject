#ifndef _LOGICSTATUS_H_
#define _LOGICSTATUS_H_
#include <glib.h>

enum LOGIC_STATE {
    LOGIC_STATE_INIT,          /*编号初始:无状态*/
    LOGIC_STATE_BEGIN,         /*编号开始:ENTER界面等待*/
    LOGIC_STATE_WAITING,       /*编号等待:等待编号消息*/
    LOGIC_STATE_SUCCESS,       /*编号成功:显示编号成功1s*/
    LOGIC_STATE_ERROR,         /*编号异常:显示编号异常1s*/
    LOGIC_STATE_TIMEOUT,       /*编号超时:显示编号超时1s*/
    LOGIC_STATE_SHOW,          /*编号显示:显示获取的编号*/
    LOGIC_STATE_UNKNOW,
};

void set_logic_status(int status);
int get_logic_status(void);
int logic_status_handle(int event);
void logic_status_change(int event);

#endif
