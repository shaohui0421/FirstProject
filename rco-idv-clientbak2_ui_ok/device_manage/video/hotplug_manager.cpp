#include "rj_ipc.h"
#include "dev_api.h"
#include "dev_common.h"
#include "cJSON.h"

namespace rcdev
{

#define HPD_IPC_TRY_TIMES 2

HotplugManager::HotplugManager(int hpdType, onHpdEvent cb, void *user)
    :mMonitor(NULL),
     mHpdType(hpdType),
     mCb(cb),
     mUser(user)
{
    RjClient *client = NULL;

    client = new RjClient(HOTPLUG_SERVICE_NAME);
    /* XXX: may block client ui, cause black srceen */

    if (!client->getConnected()) {
        reconnectService((void **)&client, false, true);
    }
    client->attach(&HotplugManager::onEvent, (void*)this);

    mMonitor = (void *)client;
}

HotplugManager::~HotplugManager()
{
    if (mMonitor) {
        RjClient *client = (RjClient *)mMonitor;
        client->detach();
        delete client;
        mMonitor = NULL;
    }
}

int HotplugManager::onEvent(void* data, size_t len, void* user)
{
    HotplugManager *manager = static_cast<HotplugManager *> (user);

    int hpdEventType = -1;
    string eventSrc;
    string eventName;
    string eventContent;
    string json;

    eventSrc.assign((const char*)data, len);

    DEV_LOG_INFO("get event: %s", eventSrc.c_str());

    eventName = eventSrc.substr(0, eventSrc.find(";"));

    if (eventName == "TERMINATING") {
        DEV_LOG_WARNING("hotplug service terminate!");
        manager->reconnectService(&manager->mMonitor, true, true);
        return DEV_OK;
    } else {
        if (eventName == HPD_EVENT_NAME_MONITOR) {
            hpdEventType = HPD_EVENT_MONITOR_CHANGE;
            eventContent = eventSrc.substr(eventSrc.find(";") + 1);
        } else {
            DEV_LOG_ERR("unknown event name: %s", eventName.c_str());
            return DEV_FAILED;
        }
    }

    DEV_LOG_INFO("event [name] %s, [type] %d, [content] %s",
                eventName.c_str(), hpdEventType, eventContent.c_str());

    if (hpdEventType != manager->mHpdType) {
        DEV_LOG_INFO("event type %d is not required %d", hpdEventType, manager->mHpdType);
        return DEV_OK;
    }

    if (manager->formatJson(json, hpdEventType, eventName, eventContent) != DEV_OK) {
        DEV_LOG_ERR("format json failed");
        return DEV_FAILED;
    }

    if (manager->mCb) {
        manager->mCb(json, manager->mUser);
    } else {
        DEV_LOG_WARNING("callback does not exist");
    }

    return DEV_OK;
}

void HotplugManager::reconnectService(void** monitor, bool attach, bool finite)
{
    int tryTimes = HPD_IPC_TRY_TIMES;
    if (!monitor || !(*monitor)) {
        DEV_LOG_ERR("hotplug socket client is NULL");
    }

    RjClient *client = (RjClient *)(*monitor);

    DEV_LOG_INFO("reconnect client : %p,  attach : %d", client, attach);

    /* XXX: use "do while", rather than "while do" */
    do {
        sleep(1);

        if (client) {
            delete client;
            client = NULL;
        }
        client = new RjClient(HOTPLUG_SERVICE_NAME);

        if (finite) {
            tryTimes--;
            DEV_LOG_INFO("tryTimes remain: %d", tryTimes);
            if (tryTimes == 0) {
                break;
            }
        }
    } while (!client->getConnected());

    if (attach) {
        client->attach(&onEvent, (void *)this);
    }

    DEV_LOG_INFO("reconnect process end");
}

int HotplugManager::formatJson(string &json, int evenType, string eventName, string info)
{
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        DEV_LOG_ERR("event json create failed");
        return DEV_FAILED;
    }

    cJSON_AddNumberToObject(root, "eventType", evenType);
    cJSON_AddStringToObject(root, "eventName", eventName.c_str());

    string devType = "NA";
    string plugInfo = "NA";
    string lastRes = "NA";
    if (!info.empty()) {
        string tmp;
        size_t pos;

        tmp = info;
        pos = tmp.find(";");
        devType = tmp.substr(0, pos);

        tmp = tmp.substr(pos + 1);
        pos = tmp.find(";");
        plugInfo = tmp.substr(0, pos);
    }
    cJSON_AddStringToObject(root, "devType", devType.c_str());
    cJSON_AddStringToObject(root, "plugInfo", plugInfo.c_str());

    char *p = cJSON_Print(root);
    if (p == NULL) {
        cJSON_Delete(root);
        return DEV_FAILED;
    }

    cJSON_Delete(root);
    json.assign(p);
    free(p);

    DEV_LOG_INFO("json: %s", json.c_str());

    return  DEV_OK;
}

} /* namespace rcdev */

