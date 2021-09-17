#include <unistd.h>

#include "dev_common.h"
#include "dev_api.h"
#include "video_core.h"

namespace rcdev
{

#define HPD_EXE_NAME     "hotplug_daemon"
#define HPD_EXE_PATH     "/usr/local/bin/system/"
#define HPD_SCRIPT_NAME  "0910B_HotplugDaemon.bash"
#define HPD_SCRIPT_PATH  "/etc/AutoStart.xinit/"
#define HPD_VERSION_FILE "/etc/RainOScfg/version_rainos"
#define HPD_MIN_VERSION  100

VideoManager::VideoManager()
    : mCb(NULL)
    , mJcb(NULL)
    , mUser(NULL)
{

}

VideoManager::~VideoManager()
{

}

void VideoManager::registerCallback(onDevEvent cb, onJsonEvent jcb, void* user)
{
    mCb = cb;
    mJcb = jcb;
    mUser = user;
}

void VideoManager::doEvent(DevMsg& msg)
{
    if (mCb == NULL) {
        DEV_LOG_ERR("mCb is NULL!");
        return;
    }
    DEV_LOG_INFO("send DevEvent, id: %d, msg: %s", msg.msg_id, msg.msg.c_str());
    mCb(msg, mUser);
}

void VideoManager::doJsonEvent(string& json)
{
    if (mJcb == NULL) {
        DEV_LOG_ERR("mJcb is NULL!");
        return;
    }
    DEV_LOG_INFO("send json: %s", json.c_str());
    mJcb(json, mUser);
}

void VideoManager::doJsonCommand(int handle, const string& json)
{
    DevMsg out_msg;
    string out_json;

    DEV_LOG_INFO("recv json: %s", json.c_str());
    switch (handle) {
        case DEV_HANDLE_REQUEST_BACKLIGHT:
            out_json = "{\"handle\": 211}";
            doJsonEvent(out_json);
            break;
        case DEV_HANDLE_SET_BACKLIGHT:
            out_json = "{\"handle\": 212}";
            doJsonEvent(out_json);
            break;
        case DEV_HANDLE_SAVE_BACKLIGHT:
            out_msg.msg_id = DEV_EVENT_SAVE_BACKLIGHT;
            out_msg.msg = "save backlight value";
            out_json = "{\"handle\": 212}";
            doEvent(out_msg);
            doJsonEvent(out_json);
            break;
        default:
            break;
    }
}

int VideoManager::getBrightness()
{
    return 0;
}

int VideoManager::setBrightness()
{
    return 0;
}

int VideoManager::getResolutionList(vector<DevResInfo>& resList)
{
    int ret = 0;
    vector<DevInfo> devList;

    resList.clear();

    DEV_LOG_INFO("============== call get res list ==============");
    ret = VideoCore::getInstance()->getResolutionList_Lock(devList);
    if (ret  < 0) {
        DEV_LOG_ERR("get dev list failed, ret %d", ret);
        return ret;
    }

    if (devList.size() > 2) {
        DEV_LOG_ERR("more than 2 monitors are connected");
        return -1;
    }

    if (devList.size() == 0) {
        DEV_LOG_INFO("no monitor is connected");
        return 0;
    }

    int finalFlag = 0;

    /* merge 1368x768 to 1366x768 */
    for (auto devInfo = devList.begin(); devInfo != devList.end(); ++devInfo) {
        bool alreadyExist = false;
        vector<ResInfo>::iterator specialRes;
        for (auto resInfo = devInfo->resList.begin(); resInfo != devInfo->resList.end(); ) {
            int width = 0;
            int height = 0;
            vector<ResInfo>::iterator tmpRes = resInfo++;
            if (tmpRes->res == "1368x768" || tmpRes->res == "1366x768") {
                if (!alreadyExist) {
                    /* change to 1366x768, and mark the resolution */
                    DEV_LOG_INFO("change %s res %s flag %d to 1366x768",
                                 devInfo->devType.c_str(), tmpRes->res.c_str(), tmpRes->flag);
                    tmpRes->res.assign("1366x768");
                    specialRes = tmpRes;
                    alreadyExist = true;
                } else {
                    /* merge flag, and remove the resolution */
                    specialRes->flag |= tmpRes->flag;
                    DEV_LOG_INFO("merge %s res %s flag %d, updated flag %d",
                                 devInfo->devType.c_str(), tmpRes->res.c_str(),
                                 tmpRes->flag, specialRes->flag);
                    devInfo->resList.erase(tmpRes);
                    resInfo--;
                    continue;
                }
            }

            /* remove unsupported resolutions */
            sscanf(tmpRes->res.c_str(), "%dx%d", &width, &height);
            if (!IS_SUPPORTED_RES(width, height)) {
                devInfo->resList.erase(tmpRes);
                resInfo--;
                continue;
            }
        }
    }

    /* single monitor */
    if (devList.size() == 1) {
        vector<ResInfo> srcList;
        DevResInfo dst;

        DEV_LOG_INFO("single monitor");

        srcList = devList.at(0).resList;

        for (auto it = srcList.begin(); it != srcList.end(); ++it) {
            dst.res = it->res;
            dst.flag = it->flag;
            finalFlag |= dst.flag;

            resList.push_back(dst);

            DEV_LOG_INFO("res: %s, flag %d", dst.res.c_str(), dst.flag);
        }
    } else if (devList.size() == 2) {
       /* in dual monitor scene, select the intersection of the resolutions */
        vector<ResInfo> srcList0;
        vector<ResInfo> srcList1;
        DevResInfo dst;

        DEV_LOG_INFO("dual monitor");

        srcList0 = devList.at(0).resList;
        srcList1 = devList.at(1).resList;
        for (auto it0 = srcList0.begin(); it0 != srcList0.end(); ++it0) {
            for (auto it1 = srcList1.begin(); it1 != srcList1.end(); ++it1) {
                if (it0->res.compare(it1->res) == 0) {
                    dst.res = it0->res;
                    dst.flag = it0->flag & it1->flag;
                    finalFlag |= dst.flag;

                    resList.push_back(dst);
                    DEV_LOG_INFO("res: %s, flag %d", dst.res.c_str(), dst.flag);
                }
            }
        }
    }

    /* if there is no BEST or CURRENT resolution,
     * choose the first resolution in the list by default.
     */
    if (!resList.empty()) {
        if (!(finalFlag & DEV_RES_BEST)) {
            auto it = resList.begin();
            it->flag |= DEV_RES_BEST;
        }
        if (!(finalFlag & DEV_RES_CUR)) {
            auto it = resList.begin();
            it->flag |= DEV_RES_CUR;
        }

        DEV_LOG_INFO("----- final output res list -----");
        for (auto it = resList.begin(); it != resList.end(); ++it) {
            DEV_LOG_INFO("res: %s, flag %d", it->res.c_str(), it->flag);
        }
    } else {
        DEV_LOG_WARNING("no effective resolution!");
    }

    return 0;
}

int VideoManager::setResolution(string res, bool force)
{
    int ret = 0;

    if (res.empty()) {
        DEV_LOG_ERR("input res is empty");
        return -1;
    }

    DEV_LOG_INFO("============== call set res (%d): %s ==============", force, res.c_str());

    ret = VideoCore::getInstance()->setResolution_Lock(res, force);
    if (ret < 0) {
        DEV_LOG_ERR("set resolution failed, ret %d", ret);
        return ret;
    }

    return 0;
}

int VideoManager::getDispCardStatus(vector<DispCardStatus> &statusList)
{
    DEV_LOG_INFO("============== call get status ==============");

    return VideoCore::getInstance()->getDispCardStatus(statusList);
}


int VideoManager::updateDisplayInfo()
{
    int ret = 0;

    DEV_LOG_INFO("============== call update DisplayInfo ==============");

    ret = VideoCore::getInstance()->updateDisplayInfo_Lock();

    if (ret < 0) {
        DEV_LOG_ERR("set resolution failed, ret %d", ret);
        return ret;
    }

    return 0;
}

int VideoManager::startHotplugDaemon()
{
    DEV_LOG_INFO("============== call start hotplug_daemon ==============");

    vector<string> cmdRes;
    if (dev_exec("ps -ef | grep " HPD_EXE_NAME " | grep -v \"grep\"", cmdRes) > 0) {
        DEV_LOG_WARNING(HPD_SCRIPT_NAME " is already running");
        return 0;
    }

    if (access(HPD_SCRIPT_PATH HPD_SCRIPT_NAME, F_OK) != 0) {
        DEV_LOG_ERR(HPD_SCRIPT_NAME " not exists");
        return -1;
    }

    if (dev_exec(HPD_SCRIPT_PATH HPD_SCRIPT_NAME " &") < 0) {
        DEV_LOG_ERR("start hotplug daemon script failed");
        return -1;
    }

    return 0;
}

int VideoManager::stopHotplugDaemon()
{
    DEV_LOG_INFO("============== call stop hotplug_daemon ==============");

    if (dev_exec("kill -9 `ps -ef | grep " HPD_SCRIPT_NAME " | grep -v \"grep\" | awk '{print $2}'`") < 0) {
        DEV_LOG_ERR("kill failed: " HPD_SCRIPT_NAME);
        return -1;
    }

    if (dev_exec("pkill " HPD_EXE_NAME) < 0) {
        DEV_LOG_ERR("kill failed: " HPD_EXE_NAME);
        return -1;
    }

    return 0;
}

bool VideoManager::isHotplugDaemonRunning()
{
    DEV_LOG_INFO("============== call isHotlugDaemonRunning ==============");

    /* rainos version file exists, and version >= 100 */
    if (access(HPD_VERSION_FILE, F_OK) == 0) {
        string cmdRes;

        if (dev_exec("cat " HPD_VERSION_FILE, cmdRes) > 0) {
            int version = atoi(cmdRes.substr(0, 4).c_str());

            if (version >= HPD_MIN_VERSION) {
                DEV_LOG_INFO("version %d >= %d", version, HPD_MIN_VERSION);
                return true;
            }
        }
    }

    return false;
}

} //namespace
