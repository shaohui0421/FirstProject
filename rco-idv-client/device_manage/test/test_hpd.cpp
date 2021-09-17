#include <unistd.h>

#include "dev_common.h"
#include "dev_api.h"

using namespace rcdev;

static VideoManager vm;

static int onHpdEventTest(const char *eventName, int eventType, const char *eventContent, void *user)
{
    DEV_LOG_INFO("=== callback enter ===");

    DEV_LOG_INFO("event name %s, type: %d, content: %s", eventName, eventType, eventContent);

    string strName(eventName);
    string strContent(eventContent);

    if(strName == HPD_EVENT_NAME_MONITOR) {
        if(eventType == HPD_EVENT_MONITOR_CHANGE) {
            /* get info */
            string devType, plugInfo, lastRes, tmp;
            size_t pos;

            tmp = strContent;
            pos = tmp.find(";");
            devType = tmp.substr(0, pos);

            tmp = tmp.substr(pos + 1);
            pos = tmp.find(";");
            plugInfo = tmp.substr(0, pos);

            tmp = tmp.substr(pos + 1);
            pos = tmp.find(";");
            lastRes = tmp.substr(0, pos);

            DEV_LOG_INFO("devType %s, pluginfo %s, lastRes %s",
                         devType.c_str(), plugInfo.c_str(), lastRes.c_str());

            /* set resolution */
            int ret = 0;
            bool lastResExist = false;
            string res;
            string bestRes;
            vector <DevResInfo> resList;
            ret = vm.getResolutionList(resList);
            if (ret != 0) {
                DEV_LOG_ERR("getSupportResolution failed, ret %d", ret);
                return -1;
            }
            for (auto it = resList.begin(); it != resList.end(); it++) {
                if (it->flag & DEV_RES_BEST) {
                    bestRes = it->res;
                }
                if (it->res == lastRes) {
                    lastResExist = true;
                }
            }

            if (lastRes != "NA") {

                DEV_LOG_INFO("get last res %s", lastRes);

                if (lastResExist) {
                    res = lastRes;
                    DEV_LOG_INFO("set last resolution: %s", res.c_str());
                } else {
                    DEV_LOG_WARNING("res conf %s is not in res list", lastRes.c_str());
                    res = bestRes;
                    DEV_LOG_INFO("set default resolution: %s", res.c_str());
                }
            } else {
                DEV_LOG_WARNING("last res is empty");
                res = bestRes;
                DEV_LOG_INFO("set default resolution: %s", res.c_str());
            }

            /* set resolution */
            ret = vm.setResolution(res, true);
            if (ret != 0) {
                DEV_LOG_ERR("setResolution fail, ret %d", res.c_str());
                return -1;
            }
        }
    }
    DEV_LOG_INFO("=== callback leave ===");

    return 0;
}




#if 0
static void doJsonTest(VideoManager* vm)
{
    vm->doJsonCommand(DEV_HANDLE_REQUEST_BACKLIGHT, "{\"handle\": 111}");
    vm->doJsonCommand(DEV_HANDLE_SET_BACKLIGHT, "{\"handle\": 112}");
    vm->doJsonCommand(DEV_HANDLE_SAVE_BACKLIGHT, "{\"handle\": 113}");
}
#endif

int main(int argc, char* argv[])
{
    init_hotplug_client_log("/var/log/hotplug_client_0.log");

    DEV_LOG_INFO("==================== START HOTPLUG TEST ====================");

    dev_exec("export DISPLAY=:0");

    HotplugManager *hm = new HotplugManager(100, &onHpdEventTest, NULL);

    while(1) {
        ;
    }

    DEV_LOG_INFO("==================== STOP VIDEO TEST =====================");

    if (hm) {
        delete hm;
        hm = NULL;
    }

    return 0;
}
