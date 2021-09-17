#include <string.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include "dev_common.h"
#include "hotplug_service.h"


HotplugService::HotplugService()
    :RjService(HOTPLUG_SERVICE_NAME),
     mSocket(-1),
     mNumDispConnected(0),
     mVideo(NULL),
     mThreadNetlink(NULL)
{
    mVideo = new VideoManager();

    if (!mVideo) {
        DEV_LOG_ERR("new VideoManager failed");
        return;
    }
    mVideo->getDispCardStatus(mStatusList);

    mVideo->updateDisplayInfo();

    start();
}

HotplugService::~HotplugService()
{
    if (mVideo) {
        delete mVideo;
        mVideo = NULL;
    }

    stop();
}

void HotplugService::start()
{
    int type = NETLINK_KOBJECT_UEVENT;
    int group = 1;

    if (createNetlinkSocket(type, group) != DEV_OK) {
        DEV_LOG_ERR("create netlink socket failed");
        return;
    }

    if (!mThreadNetlink) {
        DEV_LOG_INFO("start hotplug thread");
        mThreadNetlink = new RjThread(threadFunc, (void*)this, "netlink");
        if (!mThreadNetlink) {
            DEV_LOG_ERR("create thread netlink failed");
            closeNetlinkSocket();
            return;
        }
        mThreadNetlink->start();
    }
}

void HotplugService::stop()
{
    if (mThreadNetlink) {
        if (mThreadNetlink->get_status() != NET_THREAD_UNINITED) {
            mThreadNetlink->stop();
        }
    }

    closeNetlinkSocket();
}

int HotplugService::createNetlinkSocket(int type, int group)
{
    struct sockaddr_nl addr;
    int sock = 0;

    mSocket = -1;

    if ((sock = socket(PF_NETLINK, SOCK_RAW, type)) == -1) {
        DEV_LOG_ERR("netlink socket create failed");
        return DEV_FAILED;
    }

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = group;

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        DEV_LOG_ERR("netlink socket bind failed");
        return DEV_FAILED;
    }

    mSocket = sock;

    return DEV_OK;
}

void HotplugService::closeNetlinkSocket()
{
    if (mSocket >= 0) {
        close(mSocket);
        mSocket = -1;
    }
}

void *HotplugService::threadLoop()
{
    char buffer[HPD_BUFFER_SIZE] = {'\0'};

    while(mThreadNetlink->get_status() != NET_THREAD_UNINITED) {
        int len = recv(mSocket, buffer, HPD_BUFFER_SIZE, 0);

        if (len < 0) {
            DEV_LOG_ERR("netlink message read error");
            break;
        }
        if (len == 0) {
            DEV_LOG_WARNING("netlink read length 0");
            continue;
        }

        processEvent(buffer, len);
    }

    return NULL;
}

void *HotplugService::threadFunc(void * ctx)
{
    HotplugService *service = static_cast<HotplugService *>(ctx);

    return service->threadLoop();
}

void HotplugService::processEvent(char *buffer, int len)
{
    string src(buffer, len);
    string info;

    if (src.find("change")  != string::npos &&
        src.find("devices") != string::npos &&
        src.find("drm")     != string::npos) {

        int numConnected = 0;
        string changeInfo;
        vector<DispCardStatus> statusList;

        DEV_LOG_INFO("monitor hotplug happens: %s", buffer);

        numConnected = mVideo->getDispCardStatus(statusList);

        getStatusChangeInfo(changeInfo, statusList);

        mStatusList = statusList;

        if (changeInfo.empty()) {
            DEV_LOG_WARNING("no connect status changed, not a valid event");
            return;
        }

        /* sleep 50ms to wait for available xrandr -q result */
        usleep(50000);
        mVideo->updateDisplayInfo();

        /* if no monitor is connected, do not send message to client */
        if (numConnected == 0) {
            DEV_LOG_INFO("no monitor is connected, no events sent to client");
            return;
        }

        getHotplugInfo(info, statusList, changeInfo);

        sendMessage(info);
    }
}

string HotplugService::getCurrentResolution()
{
    string curRes;
    vector<DevResInfo> resList;

    mVideo->getResolutionList(resList);
    if (resList.size() == 0) {
        DEV_LOG_WARNING("the support res is empty");
        return string("");
    }

    for (auto it = resList.begin(); it != resList.end(); ++it) {
        if (it->flag & DEV_RES_CUR) {
            curRes = it->res;
            break;
        }
    }

    if (curRes.empty()) {
        DEV_LOG_WARNING("current res not found");
        return string("");
    }

    DEV_LOG_INFO("current res: %s", curRes.c_str());

    return curRes;
}

void HotplugService::getStatusChangeInfo(string& changeInfo, vector<DispCardStatus> statusList)
{
    int cnt = 0;

    changeInfo.clear();
    /* find out devType of monitor hotpluged */
    for (auto it = statusList.begin(); it != statusList.end(); it++) {
        DispCardStatus lastStatus = mStatusList.at(cnt);
        if (it->status != lastStatus.status) {
            changeInfo = it->devType + ";";
            if (it->status == DISPCARD_CONNECT && lastStatus.status == DISPCARD_DISCONNECT) {
                changeInfo += "PLUGIN;";
            } else {
                changeInfo += "PLUGOUT;";
            }
            break;
        }
        cnt++;
    }
}

void HotplugService::getHotplugInfo(string &hpdInfo, vector<DispCardStatus> statusList, string plugInfo)
{
    string lastRes;
    hpdInfo = "MONITOR_HOTPLUG;";

    hpdInfo += plugInfo;

    DEV_LOG_INFO("event info: %s", hpdInfo.c_str());
}

