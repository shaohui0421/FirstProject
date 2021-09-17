#include "rj_ipc.h"

#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <poll.h>
#include <signal.h>
#include <stddef.h>

#include "dev_log.h"

using std::string;
using std::list;

//namespace net
//{

static int setNoneBlock(int fd, bool non_block)
{
    int flags, res;

    flags = fcntl(fd, F_GETFL);
    if (flags == -1) {
        DEV_LOG_ERR("error : cannot get socket flags");
        return -1;
    }

    if (non_block)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    res  = fcntl(fd, F_SETFL, flags);
    if (res == -1) {
        DEV_LOG_ERR("error : cannot set socket flags");
        return res;
    }

    return 0;
}

//==================================================================================================
//  class RjService  =============================================================================
//==================================================================================================

RjService::RjService(const char* name)
    :mName((char *)name),
     mThread(NULL),
     mSockFd(-1),
     mEpollFd(-1)
{

    mClientFd.clear();

    mSockFd = createSocket(mName);
    ASSERT(mSockFd >= 0);

    setNoneBlock(mSockFd, true);

    signal(SIGPIPE, SIG_IGN);
    mEpollFd = epoll_create(MAX_CONNECT_NUM);
    ASSERT(mEpollFd >= 0);

    struct epoll_event event;
    event.events  = EPOLLIN | EPOLLOUT ;
    event.data.fd = mSockFd;

    epoll_ctl(mEpollFd, EPOLL_CTL_ADD, mSockFd, &event);

    mThread = new RjThread(serverThread, (void* )this, mName);
    ASSERT(mThread);

    mThread->start();
}


RjService::~RjService()
{
    if (mThread) {
        if (mThread->get_status() != NET_THREAD_UNINITED) {
            mThread->stop();
        }
        delete mThread;
        mThread = NULL;
    }

    while(!mClientFd.empty()) {
        int fd = mClientFd.front();
        removeClientFd(fd);
        // no need tor pop front for removeClientfd doing remove
        // mClientFd.pop_front();
    }
    mClientFd.clear();

    while(!mClientFd.empty()) {
        int fd = mAttachFd.front();
        detachFd(fd);
        //mAttachFd.pop_front();
    }
    mAttachFd.clear();

    if (mEpollFd >= 0) {
        close(mEpollFd);
        mEpollFd = -1;
    }

    if (mSockFd >= 0) {
        close(mSockFd);
        mSockFd = -1;
    }
}

int RjService::sendMessage(const string& message, int fd)
{
    AutoMutex lock(mLock);

    if (fd >= 0) {
        if (handleWrite(fd, message.c_str(), message.length()) < 0) {
            DEV_LOG_ERR("handle write error! write fd : %d", fd);
            removeClientFd(fd);
            return -1;
        }
        return 0;
    }

    if (mAttachFd.size() <= 0) {
        DEV_LOG_ERR("there is no client connect to service! send nothing to client!");
        return 0;
    }

    for(auto it = mAttachFd.begin(); it != mAttachFd.end(); ++it) {
        int attach_fd = *it;
        if (handleWrite(attach_fd, message.c_str(), message.length()) < 0) {
            DEV_LOG_ERR("handle write error! write fd : %d", attach_fd);
            detachFd(attach_fd);
            removeClientFd(attach_fd);
        }
    }
    return 0;
}

int RjService::sendMessage(const char* data, size_t len, int fd)
{
    AutoMutex lock(mLock);

    if (fd >= 0) {
        if (handleWrite(fd, data, len) < 0) {
            DEV_LOG_ERR("handle write error! write fd : %d", fd);
            removeClientFd(fd);
            return -1;
        }
        return 0;
    }

    if (mAttachFd.size() <= 0) {
        DEV_LOG_ERR("there is no client connect to service! send nothing to clinet!");
        return 0;
    }

    for(auto it = mAttachFd.begin(); it != mAttachFd.end(); ++it) {
        int attach_fd = *it;
        if (handleWrite(attach_fd, data, len) < 0) {
            DEV_LOG_ERR("handle write error! write fd : %d", attach_fd);
            detachFd(attach_fd);
            removeClientFd(attach_fd);
        }
    }

    return 0;
}


int RjService::onEvent(int fd, const char* buf, size_t len)
{
    ASSERT(fd >= 0);
    ASSERT(buf);

    string str_ping("PING");
    string str_attach("ATTACH");
    string str_detach("DETACH");

    DEV_LOG_INFO("Service on Event! event is %s,  len = %d", buf, len);

    if (strstr(buf, str_attach.c_str())) {
        DEV_LOG_INFO("attach fd : %d",  fd);
        attachFd(fd);
        return ATTACH;
    }

    if (strstr(buf, str_detach.c_str())) {
        DEV_LOG_INFO("detach fd : %d",  fd);
        sendMessage(string("EXIT"), fd);
        detachFd(fd);
        return DETACH;
    }

    if (strstr(buf, str_ping.c_str())) {
        DEV_LOG_INFO("ping fd : %d",  fd);
        sendMessage(string("PONG"), fd);
    }

    return 0;
}

int RjService::createSocket(const char* name)
{
    AutoMutex lock(mFdLock);

    int sockfd;

    struct sockaddr_un un;

    memset(&un, 0 ,sizeof(struct sockaddr_un));

    un.sun_family = AF_UNIX;

    sprintf(un.sun_path, "/tmp/%s", name);
    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        DEV_LOG_ERR("domain socket create failed! please check!");
        return -1;
    }

    unlink(un.sun_path);

    int size = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);
    if (bind(sockfd, (struct sockaddr *)&un, size) < 0) {
        DEV_LOG_ERR("domain socket bind failed");
        return -1;
    }

    if (listen(sockfd, MAX_CONNECT_NUM) < 0) {
        DEV_LOG_ERR("domain socket listen failed");
        return -1;
    }

    int time_out = 2000;
    setsockopt(sockfd, SO_SNDTIMEO, SO_SNDBUF, (const char *)&time_out, sizeof(int));

    int buf_size = 1024 * 1024;
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (const char*)&buf_size, sizeof(int));

    int option_value = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&option_value, sizeof(int));

    //int no_delay = 1;
    //setsockopt(sockfd, IPPRO_TCP, TCP_NODELAY, (const char*)&no_delay, sizeof(int));

    return sockfd;
}

int RjService::attachFd(int fd)
{
    AutoMutex autoLock(mFdLock);

    mAttachFd.push_back(fd);

    return 0;
}

int RjService::detachFd(int fd)
{
    AutoMutex Autolock(mFdLock);

    for (auto it = mAttachFd.begin(); it != mAttachFd.end(); ) {
        list<int>::iterator fdp = it++;
        if (*fdp == fd) {
            mAttachFd.erase(fdp);
            return 0;
        }
    }

    return -1;
}


int RjService::addClientFd(int fd)
{
    AutoMutex autoLock(mFdLock);

    struct epoll_event event;
    event.data.fd = fd;
    event.events  =  EPOLLIN;

    if (epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd, &event) < 0 ) {
        DEV_LOG_ERR("add client fd failed!");
        return -1;
    }

    setNoneBlock(fd, true);
    mClientFd.push_back(fd);

    return 0;
}

int RjService::removeClientFd(int fd)
{
    AutoMutex autoLock(mFdLock);

    struct epoll_event event;

    event.data.fd = fd;
    event.events  =  EPOLLIN;

    if (epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd, &event) < 0 ) {
        DEV_LOG_ERR("del client fd failed!");
    }

    for (auto it = mClientFd.begin(); it != mClientFd.end(); ) {
        list<int>::iterator fdp = it++;
        if (*fdp == fd) {
            mClientFd.erase(fdp);
        }
    }

    close(fd);

    return 0;
}

int RjService::handleAccept(int sfd)
{
    struct sockaddr client_addr;
    int cli_fd;
    socklen_t addrlen = sizeof(struct sockaddr);

    DEV_LOG_INFO("handle accept start");
    cli_fd = accept(sfd, &client_addr, &addrlen);

    if (cli_fd < 0) {
        DEV_LOG_ERR("accept fd failed : %d, %s", errno, strerror(errno));
        return cli_fd;
    }

    DEV_LOG_INFO("accept client connect! cli_fd = %d", cli_fd);

    setNoneBlock(cli_fd, true);
    addClientFd(cli_fd);

    return 0;
}

int RjService::handleRead(int fd)
{
    int n = 0;
    char buf[RECV_BUF_SIZE];
    string event;

    while ((n = recv(fd, buf, RECV_BUF_SIZE, 0)) > 0) {
        event.append((const char *)buf, n);
    }

    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {

        if (event.length() <= 0) {
            DEV_LOG_ERR("recv result 0, connect closed, fd : %d", fd);
            return 0;
        }

        DEV_LOG_INFO("%s read message : %s", mName, event.c_str());
        return onEvent(fd, event.c_str(), event.length());
    }

    if (n == 0) {
        DEV_LOG_ERR("recv result 0, connect closed, fd : %d", fd);
    }

    if (n < 0) {
        DEV_LOG_ERR("read %d error: %d %s", fd, errno, strerror(errno));
    }

    return -1;
}


int RjService::handleWrite(int fd, const char* buf, size_t len)
{
    int n = 0;
    int total_length = len;
    int pos = 0;

    DEV_LOG_INFO("%s send message! ,fd = %d, buf = %.*s", mName, fd, 128, buf);

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    fd_set fset;

    while (pos < total_length) {

        FD_ZERO(&fset);
        FD_SET(fd, &fset);

        if (select(fd + 1, NULL, &fset, NULL, &tv) > 0) {

            signal(SIGPIPE, SIG_IGN);
            n = send(fd, buf + pos, len - pos, 0);

            if ( n < 0 ) {
                switch(errno) {
                case EPIPE :
                    DEV_LOG_ERR("write error for %d: %d %s", fd, errno, strerror(errno));
                    goto error;
                case EINTR:
                case EAGAIN:
                    continue;
                default:
                    goto error;
                }
            }
            pos += n;
        } else {
            DEV_LOG_ERR("select fd timeout to write , please check the client recv!");
            goto error;
        }
    }

    return 0;

error:
    return -1;
}

void*  RjService::threadLoop()
{
    while (mThread->get_status() == NET_THREAD_RUNNING) {

        struct epoll_event epollEvent[MAX_EVENTS];

        DEV_LOG_INFO("epoll wait for client event! ");
        int cnt = epoll_wait(mEpollFd, epollEvent, MAX_EVENTS, -1);
        DEV_LOG_INFO("epoll wait for client event success, cnt = %d ", cnt);

        for (int i = 0; i< cnt ; i++) {
            //DEV_LOG_INFO("epoll wait event %d  : 0x%x , fd : %d, sfd : %d",
            //         i, epollEvent[i].events, epollEvent[i].data.fd, mSockFd);

            if (epollEvent[i].data.fd == mSockFd) {
                if (handleAccept(mSockFd) < 0) {
                    DEV_LOG_ERR("accept fd failed!");
                }
            } else if ((epollEvent[i].events & EPOLLERR) \
                       || (epollEvent[i].events & EPOLLHUP) \
                       || !(epollEvent[i].events & EPOLLIN)) {
                DEV_LOG_ERR("error : socket fd error! event : 0x%x", epollEvent[i].events);
                removeClientFd(epollEvent[i].data.fd);
                detachFd(epollEvent[i].data.fd);
                continue;
            } else {
                //DEV_LOG_INFO("client write socket case, read data and process.");
                if (handleRead(epollEvent[i].data.fd) < 0) {
                    removeClientFd(epollEvent[i].data.fd);
                    detachFd(epollEvent[i].data.fd);
                }
            }
        }
    }
    return NULL;
}

void* RjService::serverThread(void* thread)
{
    RjService* mService = static_cast<RjService *>(thread);

    ASSERT(mService);

    return mService->threadLoop();
}


//==========================================================================================================
// class RjClient =========================================================================================
//==========================================================================================================

RjClient::RjClient(const char* name)
    :mAttach(false),
     mConnected(false),
     mThread(NULL),
     mCb(NULL),
     mUser(NULL),
     mSockFd(-1),
     mName((char *)name),
     mRunning(false)
{
    AutoMutex lock(mLock);

    mSockFd = createSocket();
    ASSERT(mSockFd >= 0);

    mExitSockets[0] = mExitSockets[1] = -1;

    if (!connectService(mSockFd, name, TRY_CONNECT_TIME)) {
        DEV_LOG_ERR("cannot connect to server!");
        return;
    }

    mConnected = true;
}

RjClient::~RjClient()
{
    if (mAttach)
        detach();

    if (mSockFd > 0) {
        close(mSockFd);
        unlink(socket_path.c_str());
        mSockFd = -1;
    }
}

int RjClient::attach(onDataReached cb, void* user)
{
    AutoMutex lock(mLock);

    mCb = cb;
    mUser = user;

    if (!mConnected) {
        return -1;
    }

    sendMessage(string("ATTACH"));

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, mExitSockets) < 0) {
        DEV_LOG_ERR("cannot pare exit socket");
    }

    if (!mThread) {
        mThread = new RjThread(ClientThread, (void*)this, "Client");
        ASSERT(mThread);
        mRunning = true;
        mThread->start();
    }

    mAttach = true;

    return 0;
}

int RjClient::detach()
{
    AutoMutex lock(mLock);

    if (mThread) {
        mRunning = false;
        TEMP_FAILURE_RETRY(write(mExitSockets[0], "T", 1));
        mThread->stop();
        delete mThread;
        mThread = NULL;
    }
    if (mSockFd >= 0) {
        close(mSockFd);
        unlink(socket_path.c_str());
    }
    if (mExitSockets[0] > 0) {
        close(mExitSockets[0]);
        mExitSockets[0] = -1;
    }
    if (mExitSockets[1] > 0) {
        close(mExitSockets[1]);
        mExitSockets[1] = -1;
    }

    mAttach = false;

    if (mConnected) {
        sendMessage(string("DETACH"));
    }

    return 0;
}

int RjClient::createSocket()
{
    int fd, len;
    struct sockaddr_un un;
    static int counter = 0;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        DEV_LOG_ERR("socket fd failed!");
        return fd;
    }

    memset(&un, 0, sizeof(un));

    un.sun_family = AF_UNIX;
    sprintf(un.sun_path, "/tmp/socket_client_%05d_%d", getpid(), counter++);
    len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);

    unlink(un.sun_path);
    socket_path.assign(un.sun_path);

    if (bind(fd, (struct sockaddr *)&un, len) < 0) {
        DEV_LOG_ERR("socket bind fd failed!");
        return -1;
    }

    setNoneBlock(fd, true);
    return fd;
}

bool RjClient::connectService(int sfd ,const char* name, int timeout)
{
    struct sockaddr_un un;
    int len;
    int fd = sfd;
    string str;

    memset(&un, 0, sizeof(un));
    un.sun_family = AF_UNIX;
    int try_time = timeout;

    sprintf(un.sun_path, "/tmp/%s", name);

    len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);

    while (1) {
        DEV_LOG_INFO("try to connect server! name : %s", name);
        if (connect(fd, (struct sockaddr *)&un, len) == 0) {
            sendMessage_l(string("PING"));
            recvMessage_l(str, 1);
            if (str.find("PONG") != string::npos) {
                DEV_LOG_INFO("client transfer test success!");
                break;
            } else {
                DEV_LOG_ERR("client transfer test failed, try again!");
                continue;
            }
        }
        sleep(1);
        if (timeout > 0) {
            if (--try_time <= 0) {
                DEV_LOG_INFO("client connect service failed");
                return false;
            }
        }
    }

    DEV_LOG_INFO("%s client connect success", mName);

    return true;
}



int RjClient::sendMessage(const string& message)
{
    AutoMutex lock(mLock);

    return sendMessage_l(message);
}

int RjClient::sendMessage_l(const string& message)
{
    DEV_LOG_INFO("%s net client send Message! messsage : %s, length: %d", mName, message.c_str(), message.length());

    if (handleWrite(mSockFd, message.c_str(), message.length()) < 0) {
        DEV_LOG_ERR("net client write data error!");
        return -1;
    }

    return 0;
}

int RjClient::sendMessage(const char* data, size_t len)
{
    AutoMutex lock(mLock);

    DEV_LOG_INFO("%s net client send data %s, len = %d", mName, data, len);
    ASSERT(data);

    if (handleWrite(mSockFd, data, len) < 0) {
        DEV_LOG_ERR("net client write data error!");
        return -1;
    }

    return 0;
}

int RjClient::recvMessage(string& message, int timeout)
{
    AutoMutex lock(mLock);

    return recvMessage_l(message);
}

int RjClient::recvMessage_l(string& message, int timeout)
{
    fd_set readfd;
    FD_ZERO(&readfd);
    FD_SET(mSockFd, &readfd);

    struct timeval tv;

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    int n = select(mSockFd + 1, &readfd, NULL, NULL, &tv);

    if (n <= 0) {
        DEV_LOG_ERR("select error");
        return -1;
    }

    if (!FD_ISSET(mSockFd, &readfd)) {
        DEV_LOG_ERR("not fd set!");
        return -1;
    }
    if (handleRead(mSockFd, message) < 0) {
        DEV_LOG_ERR("handle read error! try to reconnect");
        return -1;
    }

    DEV_LOG_INFO("%s client receive message, buf = %.*s", mName, 128, message.c_str());

    return 0;
}


bool RjClient::reconnect(int time)
{
    AutoMutex lock(mLock);

    mConnected = false;

    DEV_LOG_INFO("%s reconnect to service!", mName);

    if (mSockFd > 0) {
        close(mSockFd);
        mSockFd = -1;
    }

    mSockFd = createSocket();

    if (!connectService(mSockFd, mName, time)) {
        DEV_LOG_ERR("reconnect to sevice failed");
        return false;
    }

    DEV_LOG_INFO("%s reconnect to service success!", mName);

    mConnected = true;

    return true;
}

int RjClient::handleWrite(int fd, const char* buf, size_t length)
{
    int n = 0;
    int total_length = length;
    int pos = 0;

    DEV_LOG_INFO("%s client send message, fd = %d, buf = %s, len = %d", mName, fd, buf, length);

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    fd_set fset;

    while (pos < total_length) {

        FD_ZERO(&fset);
        FD_SET(fd, &fset);

        if (select(fd + 1, NULL, &fset, NULL, &tv) > 0) {

            signal(SIGPIPE, SIG_IGN);
            n = send(fd, buf + pos, length - pos, 0);
            if ( n < 0 ) {
                switch(errno) {
                case EPIPE :
                    DEV_LOG_ERR("write error for %d: %d %s", fd, errno, strerror(errno));
                    goto error;
                case EINTR:
                case EAGAIN:
                    continue;
                default:
                    goto error;
                }
            }
            pos += n;
        } else {
            DEV_LOG_ERR("client cannot select fd to send, please check the service");
            goto error;
        }
    }

    return 0;

error:
    return -1;
}

int RjClient::handleRead(int fd, string& data)
{
    int n = 0;
    char buf[RECV_BUF_SIZE];

    DEV_LOG_INFO("%s handle read data, fd : %d", mName, fd);

    while ( (n = recv(fd,  buf, RECV_BUF_SIZE, 0)) > 0) {
        data.append(buf, n);
    }

    DEV_LOG_INFO("%s handle read data end", mName);

    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
        return 0;
    }

    if (n < 0) {
        DEV_LOG_ERR("read %d error: %d %s", fd, errno, strerror(errno));
    }

    return -1;
}

void* RjClient::ClientThread(void* p)
{
    RjClient* mClient = static_cast<RjClient *>(p);

    ASSERT(mClient);

    return mClient->ThreadLoop();
}

void* RjClient::ThreadLoop()
{
    fd_set readfd;
    FD_ZERO(&readfd);
    FD_SET(mSockFd, &readfd);

    while (mRunning) {

        FD_ZERO(&readfd);
        FD_SET(mSockFd, &readfd);
        //FD_SET(mExitSockets[1], &readfd);

        int max_fd = mSockFd;// > mExitSockets[1] ? mSockFd : mExitSockets[1];

        //DEV_LOG_INFO("%s client wait for event", mName);
        int n = select(max_fd + 1, &readfd, NULL, NULL, NULL);
        //DEV_LOG_INFO("%s client wait for event : n : %d", mName, n);

        if (n <= 0) {
            DEV_LOG_ERR("select error");
            if ((mAttach) && (mCb)) {
                string message("TERMINATING");
                mCb((void *)message.c_str(), message.length(), mUser);
            }
            break;
        }

        if (!FD_ISSET(mSockFd, &readfd)) {
            DEV_LOG_ERR("not fd set!");
            continue;
        }

        string recv_data;

        if (handleRead(mSockFd, recv_data) < 0) {
            DEV_LOG_ERR("handle read error! try to reconnect");
            if ((mAttach) && (mCb)) {
                string message("TERMINATING");
                mCb((void *)message.c_str(), message.length(), mUser);
            }
            break;
        } else {

            if (recv_data.find("EXIT") != string::npos) {
                DEV_LOG_INFO("thread exist");
                break;
            }

            if ((mAttach) && (mCb)) {
                DEV_LOG_INFO("%s callback data to process : %s", mName, recv_data.c_str());
                mCb((void *)recv_data.c_str(), recv_data.length(), mUser);
                DEV_LOG_INFO("%s callback data to process end", mName);
            }
        }
    }
    return NULL;
}


//}
