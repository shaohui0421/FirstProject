#include <fstream>
#include <sstream>
#include <thread>

#include "video_core.h"
#include "dev_log.h"
#include "dev_common.h"
#include "dev_api.h"

namespace rcdev
{

#define XRANDR_OUTFILE "/tmp/xrandr_outfile"
#define FEATURE_FILE "/tmp/monitor_feature"
#define FLOCK_VIDEO "flock_video"

#define XR_DEFAULT_DPI 96

VideoCore::VideoCore()
{
    mFlockVideo = dev_open_filelock(FLOCK_VIDEO);
    if (mFlockVideo == -1) {
        DEV_LOG_ERR("open file lock video failed");
    }

    /* update tmp file while video core being created,
      to fix dual-monitor display issue in passthrough mode */
    if (writeXrandrFile() < 0) {
        DEV_LOG_ERR("write xrandr file failed");
    }

    ifstream fin1(FEATURE_FILE);
    if (!fin1) {
        if (updateDevFeature() < 0) {
            DEV_LOG_ERR("update dev feature failed");
        }
    } else {
        if (readDevFeature() < 0) {
            DEV_LOG_ERR("read dev feature failed");
        }
    }
}

VideoCore::~VideoCore()
{
    dev_close_filelock(mFlockVideo);
    mFlockVideo = -1;
}

Mutex VideoCore::mInstanceLock;
VideoCore* VideoCore::mInstance = NULL;
VideoCore* VideoCore::getInstance()
{
    Lock lock(mInstanceLock);
    if (!mInstance) {
        mInstance = new VideoCore();
    }
    stringstream  ss;
    ss << std::this_thread::get_id();
    DEV_LOG_INFO("instance %p, thread_id %s", mInstance, ss.str().c_str());

    return mInstance;
}


void VideoCore::clearRateInfo(RateInfo& rateInfo)
{
    rateInfo.rate.clear();
    rateInfo.flag = 0;
}

void VideoCore::clearResInfo(ResInfo& resInfo)
{
    resInfo.res.clear();
    resInfo.flag = 0;
    for (auto it = resInfo.rateList.begin(); it != resInfo.rateList.end(); ++it) {
        clearRateInfo(*it);
    }
    resInfo.rateList.clear();
}

void VideoCore::clearDevInfo(DevInfo& devInfo)
{
    devInfo.devType.clear();
    for (auto it = devInfo.resList.begin(); it != devInfo.resList.end(); ++it) {
        clearResInfo(*it);
    }
    devInfo.resList.clear();
}

/* read feature for file, and store in mFeatList */
int VideoCore::readDevFeature()
{
    ifstream fin(FEATURE_FILE);

    if (fin) {
        int idx = 0;

        mFeatList.clear();
        while (!fin.eof()) {
            DevFeature devFeat;

            fin >> devFeat.devType >> devFeat.maxRes >> devFeat.bestRes;
            if (devFeat.maxRes.empty() || devFeat.bestRes.empty())
                break;

            mFeatList.push_back(devFeat);
            DEV_LOG_INFO("read feature[%d]: max res %s, best res %s",
                         idx, devFeat.maxRes.c_str(), devFeat.bestRes.c_str());

            idx++;
        }
    } else {
        DEV_LOG_ERR("feature file %s does not exist", FEATURE_FILE);
        return -1;
    }

    return 0;
}

/* write feature to file */
void VideoCore::writeDevFeature()
{
    ofstream fout(FEATURE_FILE);

    string split("\t");
    string endLine("\n");

    for (auto it = mFeatList.begin(); it != mFeatList.end(); ++it) {
        fout << it->devType << split << it->maxRes << split << it->bestRes << endLine;
    }
}

int VideoCore::updateDevFeature()
{
    int ret = 0;
    vector<DevInfo> devList;

    mFeatList.clear();

    ret = getResolutionList(devList, true);
    if (ret < 0) {
        DEV_LOG_ERR("get res list failed, ret %d", ret);
        return -1;
    }

    int idx = 0;
    int width = 0;
    int height = 0;
    int maxWidth = 0;
    int maxHeight = 0;
    string maxRes;
    string bestRes;
    vector<ResInfo>::iterator maxIt;
    vector<ResInfo>::iterator bestIt;

    for (auto devInfo = devList.begin(); devInfo != devList.end(); ++devInfo) {
        width  = 0;
        height = 0;
        maxWidth = 0;
        maxHeight = 0;
        maxIt = devInfo->resList.end();
        bestIt = devInfo->resList.end();

        for (auto resInfo = devInfo->resList.begin(); resInfo != devInfo->resList.end(); ++resInfo) {
            sscanf(resInfo->res.c_str(), "%dx%d", &width, &height);
            if (width * height > maxWidth * maxHeight) {
                maxWidth = width;
                maxHeight = height;
                maxIt = resInfo;
            }
            if (resInfo->flag & DEV_RES_BEST) {
                bestIt = resInfo;
            }
        }

        DevFeature devFeat;

        devFeat.devType = devInfo->devType;
        devFeat.maxRes = maxIt->res;

        if (bestIt != devInfo->resList.end()) {
            devFeat.bestRes = bestIt->res;
        } else {
            devFeat.bestRes.assign(STR_NULL);
        }
        mFeatList.push_back(devFeat);
        DEV_LOG_INFO("set feature[%d]: max res %s, best res %s",
                     idx, devFeat.maxRes.c_str(), devFeat.bestRes.c_str());
        idx++;
    }

    writeDevFeature();

    return 0;
}

int VideoCore::validateDevInfo(DevInfo& devInfo, size_t idx)
{
    string oriDevType;
    string oriMaxRes;
    string oriBestRes;
    vector<ResInfo>::iterator bestIt = devInfo.resList.end();
    vector<ResInfo>::iterator maxIt = devInfo.resList.end();

    /* get info from feature statistics */
    readDevFeature();
    if (mFeatList.size() <= idx) {
        DEV_LOG_ERR("get feature[%d] %s failed: size %d <= idx %d",
                    idx, devInfo.devType.c_str(), mFeatList.size(), idx);
        return -1;
    }
    oriDevType = mFeatList.at(idx).devType;
    oriMaxRes = mFeatList.at(idx).maxRes;
    oriBestRes = mFeatList.at(idx).bestRes;
    if (devInfo.devType != oriDevType) {
        DEV_LOG_WARNING("monitor hotplug may happen, devType ori: %s, cur %s",
                   oriDevType.c_str(), devInfo.devType.c_str());
        //return -1;
    }
    if (oriMaxRes.empty()) {
        DEV_LOG_ERR("get feature[%d] %s failed: max res empty",
                    idx, devInfo.devType.c_str());
        return -1;
    }
    if (oriBestRes.empty()) {
        DEV_LOG_ERR("get feature[%d] %s failed: best res empty",
                    idx, devInfo.devType.c_str());
        return -1;
    }

    DEV_LOG_INFO("get feature[%d] %s: max res %s, best res %s",
                idx, devInfo.devType.c_str(), oriMaxRes.c_str(), oriBestRes.c_str());

    /* get max res position in current list */
    for (auto it = devInfo.resList.begin(); it != devInfo.resList.end(); ++it) {
        if (it->res == oriMaxRes) {
            maxIt = it;
            break;
        }
    }
    if (maxIt == devInfo.resList.end()) {
        DEV_LOG_ERR("max res not matched ori %s", oriMaxRes.c_str());
        return -1;
    }

    /* get best res position in current list */
    for (auto it = devInfo.resList.begin(); it != devInfo.resList.end(); ++it) {
        if (it->flag & DEV_RES_BEST) {
            bestIt = it;
        }
    }

    /* validate best res */
    if (bestIt == devInfo.resList.end()) {
        /* while best res not found */
        DEV_LOG_WARNING("best res not found in current list");

        if (oriBestRes != STR_NULL) {
            for (auto it = devInfo.resList.begin(); it != devInfo.resList.end(); ++it) {
                if (it->res  == oriBestRes) {
                    bestIt = it;
                    break;
                }
            }
        }

        /* while best res not matched */
        if (bestIt == devInfo.resList.end()) {
            DEV_LOG_WARNING("best res not matched ori %s", oriBestRes.c_str());
            DEV_LOG_INFO("set max res %s to best res", maxIt->res.c_str());

            maxIt->flag |= DEV_RES_BEST;
            bestIt = maxIt;

            DEV_LOG_INFO("modified res: %s, flag %d", maxIt->res.c_str(), maxIt->flag);
        }
    }

    /* while max res is not best res */
    if (bestIt != maxIt) {
        DEV_LOG_WARNING("best res %s is not max res %s", bestIt->res.c_str(), maxIt->res.c_str());

        if (oriBestRes == STR_NULL) {
            DEV_LOG_INFO("change best res from %s to %s",
                         bestIt->res.c_str(), maxIt->res.c_str());

            maxIt->flag |= DEV_RES_BEST;
            bestIt->flag &= ~DEV_RES_BEST;

            DEV_LOG_INFO("modified res: %s, flag %d", bestIt->res.c_str(), bestIt->flag);
            DEV_LOG_INFO("modified res: %s, flag %d", maxIt->res.c_str(), maxIt->flag);
        }
    }

    DEV_LOG_INFO("----- validated res list -----");
    for (auto it = devInfo.resList.begin(); it != devInfo.resList.end(); ++it) {
        DEV_LOG_INFO("res: %s, flag %d", it->res.c_str(), it->flag);
    }

    return 0;
}

int VideoCore::getResolutionList_Lock(vector<DevInfo>& devList, bool original)
{
    Lock lock(mLockVideo);
    dev_filelock_ex(mFlockVideo);

    int ret = getResolutionList(devList, original);

    dev_filelock_unlock(mFlockVideo);

    return ret;
}

int VideoCore::getResolutionList(vector<DevInfo>& devList, bool original)
{
    vector<string> cmdRes;

    if (readXrandrFile(cmdRes) < 0) {
        DEV_LOG_WARNING("read xrandr file failed, write again");
        if (writeXrandrFile() < 0) {
            DEV_LOG_ERR("write xrandr file failed");
            return -1;
        }
        if (readXrandrFile(cmdRes) < 0) {
            DEV_LOG_ERR("read xrandr file failed *again*");
            return -1;
        }
    }

    if (cmdRes.empty()) {
        DEV_LOG_ERR("xrandr is not ready");
        return -2;
    }

    int devCnt = 0;
    bool isConnected = false;
    bool bInterlaced;
    size_t strPos0 = 0;
    size_t strPos1 = 0;
    string str0;
    string str1;
    string tmpStr;
    string extraConfig;

    ResInfo resInfo;
    RateInfo rateInfo;
    DevInfo devInfo;

    clearDevInfo(devInfo);

    for (auto it = cmdRes.begin(); it != cmdRes.end(); ++it) {
        /* device type */
        if (it->find("VIRTUAL1") != string::npos) {
            DEV_LOG_INFO("ending of actual monitors");
            break;
        } else if (it->find(" disconnected") != string::npos) {
            isConnected = false;
        } else if (it->find(" connected") != string::npos) {
            isConnected = true;
            devCnt++;
            if (devCnt > 1 && !devInfo.resList.empty()) {
                devList.push_back(devInfo);
            }
            clearDevInfo(devInfo);

            DEV_LOG_INFO("----- monitor info -----");
            strPos0 = it->find(" connected");
            devInfo.devType = it->substr(0, strPos0);
            DEV_LOG_INFO("devType: %s", devInfo.devType.c_str());
        } else if (isConnected &&
                   it->find("x")   != string::npos &&
                   it->find(" x ") == string::npos) {
            clearResInfo(resInfo);

            /* width and height */
            strPos0 = it->find("x");
            strPos1 = it->rfind(" ", strPos0);
            str0 = it->substr(strPos1 + 1, strPos0 - (strPos1 + 1));

            /* NOTE: may including 'i', such as 1920x1080i */
            strPos1 = it->find(" ", strPos0);
            str1 = it->substr(strPos0 + 1, strPos1 - (strPos0 + 1));

            resInfo.res = str0.append("x").append(str1);

            //width = atoi(str0.c_str());
            bInterlaced = false;
            if (str1.find("i") != string::npos) {
                str1.erase(str1.find("i"), 1);
                bInterlaced = true;
            }
            //height = atoi(str1.c_str());

            //DEV_LOG_INFO("res: %s, width %d, height %d, interlaced %d",
            //             resInfo.res.c_str(), width, height, bInterlaced);

            if (!bInterlaced) {

                /* refresh rate */
                size_t skipNum = 0;
                tmpStr = (*it);
                do {
                    strPos0 = tmpStr.find(".");
                    if (strPos0 == string::npos) {
                        break;
                    }
                    clearRateInfo(rateInfo);

                    strPos1 = tmpStr.rfind(" ", strPos0);
                    str0    = tmpStr.substr(strPos1 + 1, strPos0 - (strPos1 + 1));

                    if (tmpStr.find("*+", strPos0) != string::npos) {
                        strPos1 = tmpStr.find("*+", strPos0);
                        rateInfo.flag = DEV_RES_BEST | DEV_RES_CUR;
                        skipNum = strlen("*+");
                    } else if (tmpStr.find(" +", strPos0) != string::npos) {
                        strPos1 = tmpStr.find(" +", strPos0);
                        rateInfo.flag = DEV_RES_BEST;
                        skipNum = strlen(" +");
                    } else if (tmpStr.find("*", strPos0) != string::npos) {
                        strPos1 = tmpStr.find("*", strPos0);
                        rateInfo.flag = DEV_RES_CUR;
                        skipNum = strlen("*");
                    } else if (tmpStr.find(" ", strPos0) != string::npos) {
                        strPos1 = tmpStr.find(" ", strPos0);
                        skipNum = strlen(" ");
                    } else {
                        skipNum = tmpStr.length() - strPos1;
                    }
                    str1    = tmpStr.substr(strPos0 + 1, strPos1 - (strPos0 + 1));

                    tmpStr  = tmpStr.substr(strPos1 + skipNum);

                    rateInfo.rate = str0.append(".").append(str1);

                    //DEV_LOG_INFO("     rate: %s, flag %d", rateInfo.rate.c_str(), rateInfo.flag);

                    resInfo.flag |= rateInfo.flag;
                    resInfo.rateList.push_back(rateInfo);

                } while (!tmpStr.empty());

                devInfo.resList.push_back(resInfo);
            }
        }
    }


    if (devCnt > 0 && !devInfo.resList.empty()) {
        devList.push_back(devInfo);
    }

    /* validate dev info */
    if (!original && !devList.empty()) {
        size_t idx = 0;
        for (auto it = devList.begin(); it != devList.end(); ++it) {
            if (validateDevInfo(*it, idx) < 0) {
                DEV_LOG_ERR("validate dev[%d] %s failed", idx, it->devType.c_str());
                return -1;
            }
            idx++;
        }
    }

    return 0;
}

int VideoCore::setResolution_Lock(string res, bool force)
{
    Lock lock(mLockVideo);
    dev_filelock_ex(mFlockVideo);

    int ret = setResolution(res, force);

    dev_filelock_unlock(mFlockVideo);

    return ret;
}

int VideoCore::setResolution(string res, bool force)
{
    int ret = 0;
    string baseConfig;
    vector<DevInfo> devList;

    DEV_LOG_INFO("get res list first");

    ret = getResolutionList(devList);
    if (ret < 0) {
        DEV_LOG_ERR("get resolution list failed, ret %d", ret);
        return ret;
    }

    if (devList.empty()) {
        DEV_LOG_WARNING("no monitor, no need to set res");
        return -1;
    }

    bool isModeCreated = false;
    char cmd[256];
    string str0;
    string str1;
    string mode;
    string modeLine;
    size_t strPos0;
    size_t strPos1;
    vector<int> isSameResList;
    vector<string> cmdRes;


    if (res == "1368x768") {
        DEV_LOG_WARNING("1368x768 is not supported, changed to 1366x768");
        res.assign("1366x768");
    }

    for (auto devInfo = devList.begin(); devInfo != devList.end(); ++devInfo) {
        bool newMode = true;
        int  isSameRes = 0;

        /* if res is in the list, output directly */
        if (res == "1366x768") {
            vector<ResInfo>::iterator tmpRes;

            for (auto resInfo = devInfo->resList.begin(); resInfo != devInfo->resList.end(); ++resInfo) {
                if (resInfo->res == "1366x768" || resInfo->res == "1368x768") {

                    newMode = false;
                    tmpRes = resInfo;

                    if (resInfo->flag & DEV_RES_CUR) {
                        isSameRes = 1;
                        DEV_LOG_INFO("same as current res: %s", resInfo->res.c_str());
                        break;
                    }
                }
            }

            if (!newMode && (force || !isSameRes)) {
                sprintf(cmd, " --output %s --mode %s --dpi %d",
                        devInfo->devType.c_str(), tmpRes->res.c_str(), XR_DEFAULT_DPI);
                baseConfig += cmd;
            }

        } else {

            for (auto resInfo = devInfo->resList.begin(); resInfo != devInfo->resList.end(); ++resInfo) {
                if (resInfo->res == res) {
                    newMode = false;

                    if (resInfo->flag & DEV_RES_CUR) {
                        isSameRes = 1;
                        DEV_LOG_INFO("same as current res: %s", resInfo->res.c_str());
                    }

                    if (force || !isSameRes) {
                        sprintf(cmd, " --output %s --mode %s --dpi %d",
                                devInfo->devType.c_str(), resInfo->res.c_str(), XR_DEFAULT_DPI);
                        baseConfig += cmd;
                    }

                    break;
                }
            }
        }

        /* new mode */
        if (newMode) {

            /* create the same mode twice may cause X Error,
             * leading to xrandr --newmode failed.
             */
            if (!isModeCreated) {
                /* cvt */
                str0 = "cvt ";
                str1 = res;
                str0 = str0.append(str1.replace(str1.find("x"), 1, " "));
                sprintf(cmd, "%s", str0.c_str());
                DEV_LOG_INFO("exec: %s", cmd);
                if (dev_exec(cmd, cmdRes) < 0 || cmdRes.size() != 2) {
                    DEV_LOG_ERR("exec failed: %s", cmd);
                    ret = -1;
                    break;
                }

                /* new mode */
                str0 = cmdRes.at(1);
                strPos0 = str0.find("\"");
                strPos1 = str0.find("\"", strPos0 + 1);
                modeLine = str0.substr(strPos1 + 1);

                mode = res;

                DEV_LOG_INFO("mode: %s, modeLine: %s", mode.c_str(), modeLine.c_str());
                sprintf(cmd, "xrandr --newmode \"%s\" %s", mode.c_str(), modeLine.c_str());
                DEV_LOG_INFO("exec: %s", cmd);
                if (dev_exec(cmd) < 0) {
                    DEV_LOG_ERR("exec failed: %s", cmd);
                    ret = -1;
                    break;
                }

                isModeCreated = true;
            }

            /* add mode */
            sprintf(cmd, "xrandr --addmode %s %s", devInfo->devType.c_str(), mode.c_str());
            DEV_LOG_INFO("exec: %s", cmd);
            if (dev_exec(cmd) < 0) {
                DEV_LOG_ERR("exec failed: %s", cmd);
                ret = -1;
                break;
            }

            /* output */
            sprintf(cmd, " --output %s --mode %s --dpi %d",
                    devInfo->devType.c_str(), mode.c_str(), XR_DEFAULT_DPI);
            baseConfig += cmd;
        }

        isSameResList.push_back(isSameRes);
    }

    bool isSameAsCurRes = true;
    for (auto it = isSameResList.begin(); it != isSameResList.end(); ++it) {
        if (!(*it)) {
            isSameAsCurRes = false;
            break;
        }
    }

    if (!baseConfig.empty()) {
        string config;
        string extraConfig;
        vector<DispCardStatus> statusList;

        getDispCardStatus(statusList);
        generateExtraConfig(extraConfig, statusList);

        config = "xrandr" + baseConfig + extraConfig;

        DEV_LOG_INFO("exec: %s", config.c_str());

        if (dev_exec(config.c_str()) < 0) {
            DEV_LOG_ERR("exec failed: %s", config.c_str());
            ret = -1;
        }
    }

    /* update xrandr -q output to file */
    if (force || !isSameAsCurRes) {
        if (writeXrandrFile() < 0) {
            DEV_LOG_ERR("write xrandr file failed");
            return -1;
        }
    }

    if (ret < 0) {
        DEV_LOG_ERR("set resolution failed");
        return -1;
    }

    return 0;
}

int VideoCore::getDispCardStatus(vector<DispCardStatus> &statusList)
{
    int cnt = 0;
    int numConnected = 0;
    DispCardStatus ele;
    vector<string> cmdRes0;
    vector<string> cmdRes1;

    statusList.clear();

    if (dev_exec("find /sys/class/drm/*/status", cmdRes0) <= 0) {
        DEV_LOG_ERR("disp card path not found");
        return 0;
    }
    if (dev_exec("cat /sys/class/drm/*/status", cmdRes1) <= 0) {
        DEV_LOG_ERR("disp card status not found");
        return 0;
    }
    if (cmdRes0.size() != cmdRes1.size()) {
        DEV_LOG_ERR("disp card info is invalid");
        return 0;
    }

    for (auto it = cmdRes0.begin(); it != cmdRes0.end(); it++) {
        string tmp, type, idx, status;

        tmp = it->substr(it->find("-") + 1);
        type = tmp.substr(0, tmp.find("-"));

        tmp = it->substr(it->rfind("-") + 1);
        idx = tmp.substr(0,tmp.find("/"));

        ele.devType = type + idx;

        status = cmdRes1.at(cnt);

        if (status == "disconnected") {
            ele.status = DISPCARD_DISCONNECT;
        } else if (status == "connected") {
            ele.status = DISPCARD_CONNECT;
            numConnected++;
        } else {
            DEV_LOG_WARNING("invalid status: %s", it->c_str());
        }
        DEV_LOG_INFO("devType: %5s, status: %s (%d)", ele.devType.c_str(), status.c_str(), ele.status);

        statusList.push_back(ele);
        cnt++;
    }

    return numConnected;
}

void VideoCore::generateExtraConfig(string &cfg, vector<DispCardStatus> statusList)
{
    cfg.clear();

    for (auto it = statusList.begin(); it != statusList.end(); it++) {
        if (it->status == DISPCARD_DISCONNECT) {
            cfg += " --output " + it->devType + " --off";
        }
    }
}

int VideoCore::writeXrandrFile()
{
    char cmd[128];

   /* update tmp file while video core being created,
      to fix dual-monitor display issue in passthrough mode */
    sprintf(cmd, "xrandr -q > %s", XRANDR_OUTFILE);
    DEV_LOG_INFO("exec: %s", cmd);
    if (dev_exec(cmd) < 0) {
        DEV_LOG_ERR("exec failed: %s", cmd);
        return -1;
    }

    return 0;
}

int VideoCore::readXrandrFile(vector<string> &cmdRes)
{
    char cmd[128];

    cmdRes.clear();

    ifstream fin(XRANDR_OUTFILE);

    if (!fin) {
        DEV_LOG_ERR("xrandr file does not exist");
        return -1;
    }

    fin.get(); /* read one char first */
    if (fin.eof()) {
        DEV_LOG_ERR("xrandr file is empty", XRANDR_OUTFILE);
        return -1;
    }

    sprintf(cmd, "cat %s", XRANDR_OUTFILE);
    DEV_LOG_INFO("exec: %s", cmd);
    if (dev_exec(cmd, cmdRes) < 0) {
        DEV_LOG_ERR("exec failed: %s", cmd);
        return -1;

    }

    return 0;
}

int VideoCore::updateDisplayInfo_Lock()
{
    Lock lock(mLockVideo);
    dev_filelock_ex(mFlockVideo);

    int ret = updateDisplayInfo();

    dev_filelock_unlock(mFlockVideo);

    return ret;
}

int VideoCore::updateDisplayInfo()
{
    if (writeXrandrFile() < 0) {
        DEV_LOG_ERR("write xrandr file failed");
        return -1;
    }

    if (updateDevFeature() < 0) {
        DEV_LOG_ERR("update feature failed");
        return -1;
    }

    return 0;
}

} // namespace rcdev

