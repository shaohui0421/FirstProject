#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

#define EST_SOCK_ADDR                "127.0.0.1"
#define EST_SOCK_PORT                (8900)
#define INVALID_SOCKET               (-1)

static void est_msg_write_int(void *dest, const int src)
{
    int tmp = htonl(src);
    memcpy(dest, &tmp, sizeof(int));
}

static void est_msg_write_char(void *dest, const char *msg, const int msg_length)
{
    strncpy(dest, msg, msg_length);
}

static void est_msg_read_int(int *dest, void *src)
{
    memcpy(dest, (int *)src, sizeof(int));
    *dest = ntohl(*dest);
}

//static void est_msg_read_char(char *dest, void *src, int size)
//{
//    strncpy(dest, (char *)src, size);
//}

/**
 * est_send_msg - 发送消息到服务器
 * @sk:与服务器连接的sk
 * @msg:消息缓存
 *
 * 返回值：成功返回0，失败返回-1
 */
int est_send_msg(int sk, const char *msg, const int msg_length)
{
    if (sk == INVALID_SOCKET || msg == NULL || msg_length <= 0) {
        return -1;
    }

    int return_value = 0;
    int send_length = 0;
    int pos = 0;
    int total_length = sizeof(short) + sizeof(int) + msg_length;
    char *send_msg = NULL;
    send_msg = (char *)malloc(total_length);
    if (send_msg == NULL) {
        printf("no memory to send est msg\n");
        return -1;
    }
    char *pointer = send_msg;

    memset(send_msg, 0, sizeof(short) + sizeof(int));
    pointer += sizeof(short);
    est_msg_write_int(pointer, msg_length);
    pointer += sizeof(int);
    est_msg_write_char(pointer, msg, msg_length);

    while (pos < total_length) {
        send_length = send(sk, send_msg + pos, total_length - pos, 0);
        if (send_length < 0) {
            switch (errno) {
            case EPIPE:
                printf("socket is shut down\n");
                return_value = -1;
                goto quit;
            case EINTR:
            case EAGAIN:
                continue;
            default:
                return_value = -1;
                goto quit;
            }
        }
        pos += send_length;
    }
    return_value = 0;

quit:
    if (send_msg) {
        free(send_msg);
        send_msg = NULL;
    }
    return return_value;
}

#define EST_MSG_RECV(fd, recv_buffer, length, error_action)\
    ret = recv(fd, recv_buffer, length, MSG_WAITALL);\
    if (ret < 0) {\
        printf("recv err, errno=%d\n", errno);\
        error_action;\
    } else if (ret == 0) {\
        error_action;\
    }

/**
 * est_recv_msg -接收消息的内容
 * sk - socket号
 * msg - 消息缓存
 *
 * 返回值：成功返回0，失败返回-1
 */
static int est_recv_msg(int sk, char **msg)
{
#define EST_RECV_BUF_LEN    (32)

    if (sk == INVALID_SOCKET || msg == NULL) {
        return -1;
    }

    int ret;
    char recv_buffer[EST_RECV_BUF_LEN];
    int msg_length = 0;
    
    EST_MSG_RECV(sk, recv_buffer, sizeof(short), goto quit); //version
    EST_MSG_RECV(sk, recv_buffer, sizeof(int), goto quit);   //length
    est_msg_read_int(&msg_length, recv_buffer);
    if (msg_length < 0) {
        printf("est msg length < 0\n");
        return -1;
    }
    *msg = (char *)malloc(msg_length + 1);
    if (*msg == NULL) {
        printf("no memory to recv est msg\n");
        return -1;
    }
    memset(*msg, 0, msg_length + 1);
    EST_MSG_RECV(sk, *msg, msg_length, goto quit);   //content    

    return 0;

quit:
    printf("EST_MSG_RECV err\n");
    return -1;

#undef     EST_RECV_BUF_LEN
}

static void est_free_msg(char **msg)
{
    if (*msg) {
        free(*msg);
        *msg = NULL;
    }
}

static void connect_server(int sockfd)
{
    int ret;
    char *msg = NULL;
    fd_set sockset;
    char sendbuf[2048];
    char specialdata[] = "abc\ndef\0ghi\0";
    char bigdata[80000 + 1] = {0};
    int length;

    memset(bigdata, '1', sizeof(bigdata) - 1);

    //初始化集合  
    FD_ZERO(&sockset);
    FD_SET(sockfd,&sockset);
    FD_SET(0,&sockset);

    while(1)
    {
        memset(sendbuf,0,sizeof(sendbuf));
        select(sockfd+1,&sockset,NULL,NULL,NULL);
        if(FD_ISSET(sockfd,&sockset))
        {
            //处理：如果是套接字被激活，表示服务器有信息传过来，进行接收处理
            ret = est_recv_msg(sockfd, &msg);
            if (ret == -1) {
                printf("recv msg err!\n");
                est_free_msg(&msg);
                break;
            } else {
                printf("recv: %s\n", msg);
                est_free_msg(&msg);
            }
        }
        if(FD_ISSET(0,&sockset))
        {
            //处理：如果是I/O被激活，表示客户有消息要发送出去，进行发送处理  
            fgets(sendbuf,sizeof(sendbuf),stdin);
            length = strlen(sendbuf);
            sendbuf[length - 1] = '\0';

            if (strlen(sendbuf) == 0) {
                continue;
            } else if (strcmp(sendbuf, "quit") == 0 || strcmp(sendbuf,"quit\n") == 0) {
                //处理，客户想要退出，关闭套接字，用户程序退出 
                break;
            } else if (strcmp(sendbuf, "specialdata") == 0) {
                est_send_msg(sockfd, specialdata, strlen(specialdata));
            } else if (strcmp(sendbuf, "bigdata") == 0) {
                est_send_msg(sockfd, bigdata, strlen(bigdata));
            } else {
                est_send_msg(sockfd, sendbuf, strlen(sendbuf));
            }
        }
        FD_ZERO(&sockset);
        FD_SET(sockfd,&sockset);
        FD_SET(0,&sockset);
    }
}

static void est_client_sock(void)
{
    int ret = 0;
    int client_fd;
    int client_len;
    struct sockaddr_in client_addr;
    
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0){
        printf("est socket establish fail, errno=%d\n", errno);
        return;
    }
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(EST_SOCK_PORT);
    client_addr.sin_addr.s_addr = inet_addr(EST_SOCK_ADDR);
    client_len = sizeof(client_addr);

    struct timeval time_out = {2, 0};
    ret = setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, &time_out, sizeof(time_out));
    if (ret < 0) {
        printf("set socket opt failed, errno=%d\n", errno);
        close(client_fd);
        return;
    }
    int option_value = 1;
    ret = setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(int));
    if (ret < 0) {
        printf("set socket opt failed, errno=%d\n", errno);
        close(client_fd);
        return;
    }
    
    printf("connect...\n");
    ret = connect(client_fd, (struct sockaddr *)&client_addr, client_len);
    if (ret == -1) {
        printf("est socket connect fail, errno=%d\n", errno);
        close(client_fd);
        return;
    }

    connect_server(client_fd);
    close(client_fd);    
}

int main()
{
    while (1) {
        printf("====== start est_client_sock ======\n");
        est_client_sock();
        printf("====== close est_client_sock ======\n");
        sleep(10);
    }
    return 0;
}
