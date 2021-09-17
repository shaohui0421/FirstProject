#ifndef _HOTPLUG_SERVICE_H_
#define _HOTPLUG_SERVICE_H_

#include "rj_thread.h"
#include "rj_ipc.h"
#include "dev_api.h"

using namespace rcdev;

class HotplugService : RjService {
    public:
        HotplugService();
        ~HotplugService();

    private:
        void start();
        void stop();
        int  createNetlinkSocket(int type, int group);
        void closeNetlinkSocket();
        void *threadLoop();
        void processEvent(char *buffer, int len);
        int getDispCardStatus(vector<DispCardStatus> &statusList);
        void getStatusChangeInfo(string& changeInfo, vector<DispCardStatus> statusList);
        void getHotplugInfo(string &hpdInfo, vector<DispCardStatus> statusList, string plugInfo);
        string getCurrentResolution();
        int mSocket;
        int mNumDispConnected;
        vector<DispCardStatus> mStatusList;

        VideoManager *mVideo;

        RjThread *mThreadNetlink;
        static void* threadFunc(void* ctx);
};

#endif /* _HOTPLUG_SERVICE_H_ */
