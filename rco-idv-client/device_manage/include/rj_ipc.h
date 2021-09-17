/*
 *
 */

#ifndef RJ_IPC_H
#define RJ_IPC_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/epoll.h>

#include <string>
#include <list>

#include "rj_thread.h"

using std::string;
using std::list;


#define MAX_CONNECT_NUM  (1024)
#define MAX_EVENTS       (1024)
#define RECV_BUF_SIZE    (1024)
#define TRY_CONNECT_TIME (5)

enum event_e {
    NORMAL = 0,
    ATTACH = 1,
    DETACH = 2,
    PING   = 3,
};

class RjService
{
public:
    explicit RjService(const char* name);
    virtual ~RjService();

    int sendMessage(const string& message, int fd = -1);
    int sendMessage(const char* data, size_t len, int fd = -1);

    virtual int onEvent(int cli_fd, const char* buf, size_t len);

private:
    int    createSocket(const char* name);

    int    handleWrite(int fd, const char* buf, size_t len);
    int    handleRead(int fd);
    int    handleAccept(int sfd);

    int    addClientFd(int fd);
    int    removeClientFd(int fd);

    int    attachFd(int fd);
    int    detachFd(int fd);

    static void* serverThread(void* thread);
    void*  threadLoop();

private:
    char* mName;  // service name

    RjThread* mThread;

    int mSockFd;
    list<int> mClientFd;
    list<int> mAttachFd;
    int mEpollFd;

    Mutex mLock;
    Mutex mFdLock;

    RjService(const RjService&);
    RjService& operator=(const RjService& );
};

typedef int (*onDataReached)(void* data, size_t len, void* user);
#define DEFAULT_RECEIVE_TIMEOUT (5)
class RjClient
{
public:
    explicit RjClient(const char* name);
    virtual ~RjClient();

    int sendMessage(const string& message);
    int sendMessage(const char* data, size_t len);

    int recvMessage(string& message, int timeout = DEFAULT_RECEIVE_TIMEOUT);

    int attach(onDataReached cb, void* user);
    int detach();

    bool getConnected() const
    {
        return mConnected;
    }

    bool reconnect(int time);
    string socket_path;

private:
    int handleRead(int fd, string& data);
    int handleWrite(int fd, const char* data, size_t length);

    int recvMessage_l(string& message, int timeout = DEFAULT_RECEIVE_TIMEOUT);
    int sendMessage_l(const string& message);
    int  createSocket();
    bool connectService(int fd ,const char* name, int timeout);


    static void* ClientThread(void* p);
    void*        ThreadLoop();

    RjClient(const RjClient& );
    RjClient& operator=(const RjClient& );

private:
    bool mAttach;
    bool mConnected;
    RjThread* mThread;

    onDataReached mCb;
    void* mUser;

    Mutex mLock;

    int mSockFd;
    int mExitSockets[2];
    char* mName;

    bool mRunning;
};
#endif

