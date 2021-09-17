#include "logic_status.h"
#include "rc_log.h"
#include "logic_event.h"
#include "common.h"

static int logic_status = 0;

const static char *LogicStatusDesc[] = {
    "SERIAL INIT STATUS",
    "SERIAL BEGIN STATUS",
    "SERIAL WAITING STATUS",
    "SERIAL SUCCESS STATUS",
    "SERIAL ERROR STATUS",
    "SERIAL TIMEOUT STATUS",
    "SERIAL SHOW STATUS",
    "UNKNOW LOGIC STATUS",
};

const char *logic_state_desc(int status)
{
    if (status >= LOGIC_STATE_INIT && status < LOGIC_STATE_UNKNOW)
        return LogicStatusDesc[status];
    else
        return LogicStatusDesc[LOGIC_STATE_UNKNOW];
}

void set_logic_status(int status)
{
    LOG_INFO("CHANGE  STATUS:[%s]", logic_state_desc(status));
    logic_status = status;
}

int get_logic_status(void)
{
    LOG_INFO("CURRENT STATUS:[%s]", logic_state_desc(logic_status));
    return logic_status;
}

void logic_status_change(int event)
{
    switch (event) {
    case UI_SHOW_SERIAL_BEGIN:
        set_logic_status(LOGIC_STATE_BEGIN);
        break;
    case UI_SHOW_SERIAL_WAIT:
        set_logic_status(LOGIC_STATE_WAITING);
        break;
    case UI_SHOW_SERIAL_SHOW:
        set_logic_status(LOGIC_STATE_SHOW);
        break;
    case UI_SHOW_SERIAL_CLOSE:
        set_logic_status(LOGIC_STATE_INIT);
        break;
    case UI_SHOW_SERIAL_RESET:
        set_logic_status(LOGIC_STATE_BEGIN);
        break;
    case UI_SHOW_SERIAL_SUCCESS:
        set_logic_status(LOGIC_STATE_SUCCESS);
        break;
    case UI_SHOW_SERIAL_ERROR:
        set_logic_status(LOGIC_STATE_ERROR);
        break;
    case UI_SHOW_SERIAL_TIMEOUT:
        set_logic_status(LOGIC_STATE_TIMEOUT);
        break;
    default:
        break;
    }

    return;
}

int logic_status_handle(int event)
{
    int current_state;
    int ret = -1;

    LOG_DEBUG("TRIG EVENT:[%s]", logic_event_desc(event));
    current_state = get_logic_status();
    switch (current_state) {
    case LOGIC_STATE_INIT:
        {
            switch (event) {
            case UI_SHOW_SERIAL_BEGIN:
            case UI_SHOW_SERIAL_CLOSE:
            case LOGIC_SET_TERM_CONF:
            case LOGIC_CLR_LAST_TIME:
                ret = event;
                break;
            default:
                break;
            }
        }
        break;
    case LOGIC_STATE_BEGIN:
        {
            switch (event) {
            case LOGIC_SET_TERM_CONF:
            case LOGIC_CLR_LAST_TIME:
            case UI_SHOW_SERIAL_WAIT:
            case UI_SHOW_SERIAL_CLOSE:
                ret = event;
                break;
            default:
                break;
            }
        }
        break;
    case LOGIC_STATE_WAITING:
        {
            switch (event) {
            case UI_SHOW_SERIAL_SUCCESS:
            case UI_SHOW_SERIAL_ERROR:
            case UI_SHOW_SERIAL_TIMEOUT:
            case UI_SHOW_SERIAL_CLOSE:
            case LOGIC_SEND_DEV_MAC:
                ret = event;
                break;
            default:
                break;
            }
        }
        break;
    case LOGIC_STATE_SUCCESS:
        {
            switch (event) {
            case UI_SHOW_SERIAL_SHOW:
            case UI_SHOW_SERIAL_CLOSE:
                ret = event;
                break;
            default:
                break;
            }
        }
        break;
    case LOGIC_STATE_TIMEOUT:
    case LOGIC_STATE_ERROR:
        {
            switch (event) {
            case UI_SHOW_SERIAL_RESET:
            case UI_SHOW_SERIAL_CLOSE:
                ret = event;
                break;
            default:
                break;
            }
        }
        break;
    case LOGIC_STATE_SHOW:
        {
            switch (event) {
            case UI_SHOW_SERIAL_BEGIN:
            case UI_SHOW_SERIAL_CLOSE:
            case LOGIC_SET_TERM_CONF:
            case LOGIC_CLR_LAST_TIME:
                ret = event;
                break;
            default:
                break;
            }
        }
        break;
    default:
        break;
    }

    if (ret != -1)
        LOG_INFO("EXEC EVENT:[%s]", logic_event_desc(ret));

    return ret;
}

