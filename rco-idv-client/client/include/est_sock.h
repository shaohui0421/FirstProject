#ifndef EST_SOCK_H
#define EST_SOCK_H

#include <map>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include "application.h"

#define EST_SOCK_PORT                (8900)
#define EST_SOCK_MAX_CONN            (1)

using std::string;

typedef struct {
    int sk;                //client sockfd
    string content;        //msg content
} est_msg_data;

class Application;

class ESTSock
{
public:
    typedef void (ESTSock::*onEstEvent)(void *data);
    typedef std::map<string, onEstEvent> EstHandlers;

    ESTSock(Application *app);
    virtual ~ESTSock();
    void start_sock(void);
    void close_sock(void);

private:
    void set_handler(const string &key, const onEstEvent handle);
    static void *thread_main(void* data);
    void connect_sock(void);
    bool create_sock();

    bool est_send_msg(int sk, const string &msg);
    bool est_recv_msg(int sk, string &msg);
    void send_msg_handle(int sockfd, const string &send_msg);
    //void send_all_msg_handle(const string &send_msg);
    void distribute_msg_handle(int sk, const string &recv_msg);
    void recv_msg_handle(void);
    void accept_new_sock_handle(void);

    void est_handle_heartbeat(void *data);
    void est_handle_special_usb(void *data);

    Application *_app;
    UnjoinableThread* _thread;
    EstHandlers _handlers;
    fd_set _readfds;
    fd_set _allfds;
    int _client_fds[EST_SOCK_MAX_CONN];
    int _server_fd;
    int _maxfd;
};

static inline void est_msg_write_int(void *dest, const int src)
{
    int tmp = htonl(src);
    memcpy(dest, &tmp, sizeof(int));
}

static inline void est_msg_write_char(char *dest, const char *src, const int len)
{
    strncpy(dest, src, len);
}

static inline void est_msg_read_int(int *dest, void *src)
{
    memcpy(dest, (int *)src, sizeof(int));
    *dest = ntohl(*dest);
}

//static inline void est_msg_read_char(char *dest, const char *src, const int len)
//{
//    strncpy(dest, src, len);
//}

#endif //EST_SOCK_H
