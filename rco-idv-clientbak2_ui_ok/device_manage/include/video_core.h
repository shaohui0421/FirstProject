#ifndef _VIDEO_CORE_H_
#define _VIDEO_CORE_H_

#include <string>
#include <vector>
#include <dev_thread.h>

#include "dev_api.h"

using namespace std;

namespace rcdev
{

#define IS_SUPPORTED_RES(w, h) ((w) >= 1024 && (h) >= 720 && ((w) * (h) >= 1024 * 768))
#define STR_NULL "str_null"

typedef struct _RateInfo {
    string rate;
    int flag;
} RateInfo;

typedef struct _ResInfo {
    string res;
    int flag;
    vector<RateInfo> rateList;
} ResInfo;

typedef struct _DevInfo {
    string devType;
    vector<ResInfo> resList;
} DevInfo;

typedef struct _DevFeature {
    string devType;
    string maxRes;
    string bestRes;
} DevFeature;

class VideoCore {
public:
    ~VideoCore();
    static VideoCore* getInstance();

    int getResolutionList_Lock(vector<DevInfo>& devList, bool original = false);
    int setResolution_Lock(string res, bool force);
    int updateDisplayInfo_Lock();
    int getDispCardStatus(vector<DispCardStatus> &statusList);

private:
    VideoCore();
    VideoCore(const VideoCore& );
    VideoCore& operator=(const VideoCore& );
    static VideoCore* mInstance;
    static Mutex mInstanceLock;

    vector<DevFeature> mFeatList;

    class VideoCoreGarbo   
    {
    public:
        ~VideoCoreGarbo()
        {
            if (VideoCore::mInstance) {
                delete VideoCore::mInstance;
                VideoCore::mInstance = NULL;
            }
        }
    };
    static VideoCoreGarbo Garbo;

    int getResolutionList(vector<DevInfo>& devList, bool original = false);
    int setResolution(string res, bool force);
    int updateDisplayInfo();

    void clearRateInfo(RateInfo& rateInfo);
    void clearResInfo(ResInfo& resInfo);
    void clearDevInfo(DevInfo& devInfo);
    void writeDevFeature();
    int readDevFeature();
    int updateDevFeature();
    int writeXrandrFile();
    int readXrandrFile(vector<string> &cmdRes);
    int validateDevInfo(DevInfo& devInfo, size_t idx);
    void generateExtraConfig(string &cfg, vector<DispCardStatus> statusList);

    int mFlockVideo;
    Mutex mLockVideo;
};

} // namespace rcdev

#endif
