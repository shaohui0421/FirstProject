#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include "test_est_sock.h"

ESTSock::ESTSock()
    : _server_fd(-1)
    , _maxfd(-1)
{
    for (int i = 0; i < EST_SOCK_MAX_CONN; i++) {
        _client_fds[i] = -1;
    }

    set_handler("HEARTBEAT", &ESTSock::est_handle_heartbeat);
    set_handler("SPECIAL_USB", &ESTSock::est_handle_special_usb);

    create_sock();
}

ESTSock::~ESTSock()
{
    for (int i = 0; i < EST_SOCK_MAX_CONN; i++) {
        if (_client_fds[i] != -1) {
            close(_client_fds[i]);
            _client_fds[i] = -1;
        }
    }
    if (_server_fd != -1) {
        close(_server_fd);
        _server_fd = -1;
    }
}

void ESTSock::set_handler(const string &key, const onEstEvent event)
{
    _handlers.insert(std::pair<string, onEstEvent>(key, event));
}

void ESTSock::create_sock()
{
    int ret;
    struct sockaddr_in server_addr;
    int server_len;

    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd < 0){
        printf("est socket establish fail, errno=%d\n", errno);
        return;
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(EST_SOCK_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_len = sizeof(server_addr);
    
    struct timeval time_out = {2, 0};
    ret = setsockopt(_server_fd, SOL_SOCKET, SO_SNDTIMEO, &time_out, sizeof(time_out));
    if (ret < 0) {
        printf("set socket opt failed, errno=%d\n", errno);
        close(_server_fd);
        return;
    }
    int option_value = 1;
    ret = setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(int));
    if (ret < 0) {
        printf("set socket opt failed, errno=%d\n", errno);
        close(_server_fd);
        return;
    }

    //bind...
    ret = bind(_server_fd, (struct sockaddr *)&server_addr, server_len);
    if (ret < 0) {
        printf("est socket bind fail, errno=%d\n", errno);
        close(_server_fd);
        return;
    }

    //listen...
    ret = listen(_server_fd, EST_SOCK_MAX_CONN);
    if (ret < 0) {
        printf("est socket listen fail, errno=%d\n", errno);
        close(_server_fd);
        return;
    }
}

/**
 * est_send_msg - 发送消息到服务器
 * @sk:与服务器连接的sk
 * @msg:消息缓存
 *
 * 返回值：成功返回true，失败返回false
 */
bool ESTSock::est_send_msg(int sk, const string &msg)
{
    if (sk == INVALID_SOCKET) {
        return false;
    }

    printf("est send sk = %d\n", sk);
    printf("est send msg = %s\n", msg.c_str());

    bool return_value = false;
    int send_length = 0;
    int pos = 0;
    int total_length = sizeof(short) + sizeof(int) + msg.length();
    char* send_msg = new char[total_length];
    char *pointer = send_msg;

    memset(pointer, 0, sizeof(short) + sizeof(int));
    pointer += sizeof(short);
    est_msg_write_int(pointer, (int)msg.length());
    pointer += sizeof(int);
    est_msg_write_char(pointer, msg.c_str(), msg.length());

    while (pos < total_length) {
        send_length = send(sk, send_msg + pos, total_length - pos, 0);
        if (send_length < 0) {
            switch (errno) {
            case EPIPE:
                printf("socket is shut down\n");
                return_value = false;
                goto quit;
            case EINTR:
            case EAGAIN:
                continue;
            default:
                return_value = false;
                goto quit;
            }
        }
        pos += send_length;
    }
    return_value = true;

quit:
    delete[] send_msg;
    return return_value;
}

#define EST_MSG_RECV(fd, recv_buffer, length, error_action)\
    if (recv(fd, recv_buffer, length, MSG_WAITALL) <= 0) {\
        printf("recv err, errno=%d\n", errno);\
        error_action;\
    }

/**
 * est_recv_msg -接收消息的内容
 * sk - socket号
 * msg - 消息缓存，小于4096字节
 *
 * 返回值：成功返回true，失败返回false
 */
bool ESTSock::est_recv_msg(int sk, string &msg)
{
#define EST_RECV_BUF_LEN    (32)

    if (sk == INVALID_SOCKET) {
        printf("invalid params\n");
        return -1;
    }

    char recv_buffer[EST_RECV_BUF_LEN] = {0};
    int msg_length = 0;
    char *data = NULL;

    EST_MSG_RECV(sk, recv_buffer, sizeof(short), return false); //version
    EST_MSG_RECV(sk, recv_buffer, sizeof(int), return false);   //length
    est_msg_read_int(&msg_length, recv_buffer);
    if (msg_length < 0) {
        printf("est msg length < 0\n");
        return -1;
    }
    data = new char[msg_length + 1];
    memset(data, 0, msg_length + 1);
    EST_MSG_RECV(sk, data, msg_length, goto quit);   //content
    msg = data;
    delete[] data;
    return true;

quit:
    delete[] data;
    return false;

#undef     EST_RECV_BUF_LEN
}

/**
 * send_msg_handle -发送消息处理，向client发送消息
 * sockfd - socket文件描述符
 * send_msg - 消息缓存
 */
void ESTSock::send_msg_handle(int sockfd, const string &send_msg)
{
    bool ret;

    ret = est_send_msg(sockfd, send_msg);
    if (ret == false) {
        printf("send msg err\n");
        for (int i = 0; i < EST_SOCK_MAX_CONN; i++) {
            if (_client_fds[i] == -1) {
                continue;
            }
            if (FD_ISSET(sockfd, &_readfds)) {
                FD_CLR(sockfd, &_allfds);
                close(sockfd);
                _client_fds[i] = -1;
            }
        }
    }
}

/**
 * send_all_msg_handle -发送消息处理，向所有client发送消息
 * send_msg - 消息缓存
 */
void ESTSock::send_all_msg_handle(const string &send_msg)
{
    bool ret;
    int client_fd;

    for (int i = 0; i < EST_SOCK_MAX_CONN; i++) {
        client_fd = _client_fds[i];
        if (client_fd == -1) {
            continue;
        }
        ret = est_send_msg(client_fd, send_msg);
        if (ret == false) {
            printf("est send msg err! errno=%d\n", errno);
            FD_CLR(client_fd, &_allfds);
            close(client_fd);
            _client_fds[i] = -1;
        }
    }
}

/**
 * distribute_msg_handle -分发接收的消息
 * sk - 客户端socket文件描述符
 * recv_msg - 接收的客户端消息
 *
 */
void ESTSock::distribute_msg_handle(int sk, const string &recv_msg)
{
    string::size_type sep_pos;
    string msg_key;
    est_msg_data msg_data = { sk, "" };

    printf("recv msg len: %d\n", (int)recv_msg.length());

    //separate recv_msg
    sep_pos = recv_msg.find(":");
    if (sep_pos == string::npos) {
        msg_key = recv_msg;
    } else {
        msg_key = recv_msg.substr(0, sep_pos);
        msg_data.content = recv_msg.substr(sep_pos + 1);
    }

    printf("msg key: %s\n", msg_key.c_str());
    printf("content: %s\n", msg_data.content.c_str());

    //distribute ms
    if (_handlers.count(msg_key) > 0) {
        (this->*_handlers[msg_key])(&msg_data);
    }
}

/**
 * recv_msg_handle -接收所有客户端消息处理
 */
void ESTSock::recv_msg_handle(void)
{
    int client_fd;
    string msg;
    bool ret;

    for (int i = 0; i < EST_SOCK_MAX_CONN; i++) {
        client_fd = _client_fds[i];
        if (client_fd == -1) {
            continue;
        }
        if (FD_ISSET(client_fd, &_readfds)) {
            ret = est_recv_msg(client_fd, msg);
            if (ret == false) {
                printf("est recv msg err, errno=%d\n", errno);
                FD_CLR(client_fd, &_allfds);
                close(client_fd);
                _client_fds[i] = -1;
            } else {
                distribute_msg_handle(client_fd, msg);
            }
        }
    }
}

/**
 * accept_new_sock_handle -新客户端连接处理
 */
void ESTSock::accept_new_sock_handle(void)
{
    int i;
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len;

    client_len = sizeof(client_addr);
    client_fd = accept(_server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd == -1) {
        printf("est accept error, errno=%d\n", errno);
        return;
    }
    printf("accept new socket\n");
    for (i = 0; i < EST_SOCK_MAX_CONN; i++) {
        if (_client_fds[i] == -1) {
            _client_fds[i] = client_fd;
            printf("accept fd[%d] = %d\n", i, client_fd);
            break;
        }
    }
    if (i == EST_SOCK_MAX_CONN) {
        //客户端连接数达到最大，释放_client_fds[0]
        if (_client_fds[0] != -1) {
            FD_CLR(_client_fds[0], &_allfds);
            close(_client_fds[0]);
            _client_fds[0] = client_fd;
            printf("release client fd[0] and accpet new socket %d\n", client_fd);
        } else {
            printf("release client fd[0] fail\n");
        }
    }
    FD_SET(client_fd, &_allfds);
    if (_maxfd < client_fd) {
        _maxfd = client_fd;
    }
}

void ESTSock::connect(void)
{
    int nready;
    struct timeval timeout;

    if (_server_fd == -1) {
        printf("unknown server fd\n");
        return;
    }

    _maxfd = _server_fd;
    FD_ZERO(&_readfds);
    FD_ZERO(&_allfds);
    FD_SET(_server_fd, &_allfds);

    while (1) {
        _readfds = _allfds;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        nready = select(_maxfd + 1, &_readfds, NULL, NULL, &timeout);
        if (nready == -1) {
            //select 出错
            printf("est select error, errno=%d\n", errno);
            break;
        } else if (nready == 0){
            //select 超时
            //printf("est select timeout\n");
            continue;
        }

        if (FD_ISSET(_server_fd, &_readfds)) {
            //有新的连接
            accept_new_sock_handle();
        } else {
            //处理客户端连接
            recv_msg_handle();
        }
    }
}

void ESTSock::est_handle_heartbeat(void *data)
{
    //do nothing
    printf("recv est heartbeat\n");
}

void ESTSock::est_handle_special_usb(void *data)
{
    est_msg_data *msg_data = (est_msg_data *)data;

    if (msg_data) {
        printf("recv est special_usb: %s\n", msg_data->content.c_str());
        send_msg_handle(msg_data->sk, "recv special usb");
    }
}

int main()
{
    ESTSock est_sock;
    est_sock.connect();
    return 0;
}
