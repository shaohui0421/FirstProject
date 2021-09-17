#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>
#include "logic_status.h"
#include "rc_log.h"
#include "ui_manager.h"
#include "common.h"

#define MSG_RECV(bytes, fd, data, length, ret, END)\
    bytes = read(fd, data, length);\
    if (bytes < 0)\
    {\
        LOG_INFO("bytes<0, error:%d(%s)", errno, strerror(errno));\
        *ret = -1;\
        goto END;\
    }\
    else if (bytes == 0)\
    {\
        LOG_INFO("bytes=0, error:%d(%s)", errno, strerror(errno));\
        *ret = 0;\
        goto END;\
    }

static int _fd;
int num;

int get_logic_fd(void)
{
    return _fd;
}

void set_logic_fd(int fd)
{
    _fd = fd;
    return;
}

void send_event_to_logic(int event, const char *data, int data_size)
{
    event_data_t *head = NULL;
    int total_size;
    int send_size = 0;
    int ret;
    int fd = 0;

    fd = open(LOGIC_FIFO, O_WRONLY);
    if (fd < 0) {
        LOG_ERR("open failed, errno:%d(%s)", errno, strerror(errno));
        return;
    }

    total_size = sizeof(event_data_t) + data_size;
    head = (event_data_t *)malloc(total_size);
    if (head == NULL) {
        goto END;
    }
    memset(head, 0, total_size);
    head->event = event;
    if (data) {
        head->size = data_size;
        memcpy(head->data, data, data_size);
    } else {
        head->size = 0;
    }

    while (send_size < total_size) {
        ret = write(_fd, head + send_size, total_size - send_size);
        LOG_INFO("write ret:%d", ret);
        if (ret <= 0) {
            LOG_ERR("write failed, errno:%d(%s)", errno, strerror(errno));
            goto END;
        }
        send_size += ret;
    }

END:
    close(fd);
    if (head)
        free(head);
    return;
}

char *parse_event_type(int _fd, int *ret)
{
    event_data_t head;
    int bytes;
    char *data = NULL;

    memset(&head, 0, sizeof(event_data_t));
    MSG_RECV(bytes, _fd, &head, sizeof(event_data_t), ret, END);

    data = (char *)malloc(head.size + sizeof(event_data_t));
    if (data == NULL) {
        *ret = 0;
        goto END;
    }
    memset(data, 0, head.size + sizeof(event_data_t));
    memcpy(data, &head, sizeof(event_data_t));
    if (head.size > 0)
        MSG_RECV(bytes, _fd, data + sizeof(event_data_t), head.size, ret, END);

END:
    return data;
}

int logic_event_handle(int _fd)
{
    event_data_t *data = NULL;
    int ret = 0;
    int event;

    data = (event_data_t *)parse_event_type(_fd, &ret);
    if (ret <= 0) {
        LOG_ERR("fifo stop");
        goto END;
    }

    event = data->event;
    LOG_INFO("event type:%d", event);

    switch(event) { 
    case EVENT_MSG_CONF_BEGIN:
        break;
    case EVENT_MSG_CONF_IMPLE:
        break;
    case EVENT_MSG_SERI_PARSE:
        break;
    case EVENT_MSG_SERI_CLOSE:
        break;
    case EVENT_MSG_CONF_CLOSE:
        break;
    case EVENT_UI_PRESS_ENTER:
        break;
    case EVENT_UI_CLICK_CLOSE:
        break;
    default:
        break;
    }

END:
    if (data)
        free(data);
    return ret;
}

void start_pipe_monitor(int _fd)
{
    fd_set rfds;
    int retval;
    int ret;

    LOG_INFO("start pipe monitor");

    FD_ZERO(&rfds);
    FD_CLR(_fd, &rfds);
    FD_SET(_fd, &rfds);

    while (1)
    {
        retval = select(FD_SETSIZE, &rfds, NULL, NULL, NULL);
        if (retval == -1) {
            LOG_ERR("select error:%d(%s)", errno, strerror(errno));
            break;
        } else if (retval) {
            if(FD_ISSET(_fd, &rfds)) {
                ret = logic_event_handle(_fd);
                if (ret == -1) {
                    close(_fd);
                    FD_ZERO(&rfds);
                    break;
                }
            }
        } else {
            LOG_WARNING("## select time out ##\n");
        }
    }

    unlink(LOGIC_FIFO);
    LOG_WARNING("monitor end");

    return;
}

int mk_event_fifo(void)
{
    int ret = 0;
    int fd = -1;

    if (access(LOGIC_FIFO, F_OK) == -1) {
        ret = mkfifo(LOGIC_FIFO, 0777);
        if(ret != 0) {
            LOG_ERR("mkfifo failed, error:%d(%s)", errno, strerror(errno));
            return -1;
        }
    }

    fd = open(LOGIC_FIFO, O_RDONLY);
    if (fd < 0) {
        LOG_ERR("open failed error:%d(%s)", errno, strerror(errno));
        return -1;
    }

    set_logic_fd(fd);

    return fd;
}


static void *logic_thread(void *data)
{
    pthread_detach(pthread_self());
    set_thread_name("logic monitor");
    int fd;

    fd = mk_event_fifo();
    if (fd < 0) {
        return NULL;
    }
    start_pipe_monitor(fd);

    return NULL;
}

void logic_thread_init(void)
{
    pthread_t thread_id;
    int ret;

    ret = pthread_create(&thread_id, NULL, logic_thread, NULL);
    if (ret) {
        LOG_ERR("create failed, error:%d(%s)", errno, strerror(errno));
    }
    return;
}

void logic_handle_event(int event, void *data)
{
    switch (event) {
    case EVENT_MSG_CONF_BEGIN:
        send_event_to_logic(event, NULL, 0);
        break;
    case EVENT_MSG_CONF_IMPLE:
        send_event_to_logic(event, (char *)data, sizeof(EASYDEPLOY));
        break;
    case EVENT_MSG_SERI_PARSE:
        send_event_to_logic(event, (char *)data, sizeof(int));
        break;
    case EVENT_MSG_SERI_CLOSE:
        send_event_to_logic(event, NULL, 0);
        break;
    case EVENT_MSG_CONF_CLOSE:
        send_event_to_logic(event, NULL, 0);
        break;
    case EVENT_UI_PRESS_ENTER:
        send_event_to_logic(event, NULL, 0);
        break;
    case EVENT_UI_CLICK_CLOSE:
        send_event_to_logic(event, NULL, 0);
        break;
    default:
        break;
    }

    return;
}

