#ifndef TEST_EST_SOCK_H
#define TEST_EST_SOCK_H

#include <map>
#include <string>
#include <string.h>
#include <sys/socket.h>

#define EST_SOCK_PORT                (8900)
#define EST_SOCK_MAX_CONN            (1)
#define INVALID_SOCKET               (-1)

using std::string;

typedef struct {
    int sk;                //client sockfd
    string content;        //msg content
} est_msg_data;

class ESTSock
{
public:
    typedef void (ESTSock::*onEstEvent)(void *data);
    typedef std::map<string, onEstEvent> EstHandlers;

    ESTSock();
    virtual ~ESTSock();
    void create_sock(void);
    void connect(void);

private:
    void set_handler(const string &key, const onEstEvent handle);

    bool est_send_msg(int sk, const string &msg);
    bool est_recv_msg(int sk, string &msg);
    void send_msg_handle(int sockfd, const string &send_msg);
    void send_all_msg_handle(const string &send_msg);
    void distribute_msg_handle(int sk, const string &recv_msg);
    void recv_msg_handle(void);
    void accept_new_sock_handle(void);

    void est_handle_heartbeat(void *data);
    void est_handle_special_usb(void *data);

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


#endif //TEST_EST_SOCK_H
