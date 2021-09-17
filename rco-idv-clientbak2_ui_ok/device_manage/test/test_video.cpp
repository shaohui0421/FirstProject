#include <unistd.h>

#include "dev_common.h"
#include "dev_api.h"

using namespace rcdev;

static Mutex g_lock_getres;

static void videoCallback(DevMsg& msg, void *user)
{
    DEV_LOG_INFO("** recv video callback **");
    DEV_LOG_INFO("** id : %d", msg.msg_id);
    DEV_LOG_INFO("** msg: %s", msg.msg.c_str());
    DEV_LOG_INFO("** recv video callback **");
}

static void videoJsonCallback(string& msg, void *user)
{
    DEV_LOG_INFO("** recv video json callback **");
    DEV_LOG_INFO("** json: %s", msg.c_str());
    DEV_LOG_INFO("** recv video json callback **");
}

static void getResList(VideoManager *vm)
{
    vector<DevResInfo> resList;

    //Lock lock(g_lock_getres);

    //DEV_LOG_INFO("vm: %p", vm);
    if (vm->getResolutionList(resList) == -1) {
        DEV_LOG_ERR("####### get resolution failed #######");
    }

    #if 0
    for (auto resInfo = resList.begin(); resInfo != resList.end(); ++resInfo) {
        DEV_LOG_INFO("res %s, flag %d", resInfo->res.c_str(), resInfo->flag);
    }
    #endif
}

static void setResList(VideoManager *vm, string res)
{
    //Lock lock(g_lock_getres);

    //DEV_LOG_INFO("vm: %p", vm);
    if (vm->setResolution(res, true) == -1) {
        DEV_LOG_ERR("####### get resolution failed #######");
    }

    #if 0
    for (auto resInfo = resList.begin(); resInfo != resList.end(); ++resInfo) {
        DEV_LOG_INFO("res %s, flag %d", resInfo->res.c_str(), resInfo->flag);
    }
    #endif
}


static void *getResListLoop0(void *arg)
{
    VideoManager *vm = static_cast<VideoManager*>(arg);

    pthread_detach(pthread_self());
    
    do {
        DEV_LOG_INFO("##### GET 0 (start) #####");
        getResList(vm);
        DEV_LOG_INFO("##### GET 0 (end) #####");
    } while(1);

    return NULL;
}

static void *getResListLoop1(void *arg)
{
    VideoManager *vm = static_cast<VideoManager*>(arg);

    pthread_detach(pthread_self());

    do {
        DEV_LOG_INFO("##### GET 1 (start) #####");
        getResList(vm);
        DEV_LOG_INFO("##### GET 1 (end) #####");
    } while(1);

    return NULL;
} 

static void *setResListLoop0(void *arg)
{
    VideoManager *vm = static_cast<VideoManager*>(arg);

    pthread_detach(pthread_self());

    do {
        DEV_LOG_INFO("***** SET 0 (start) *****");
        setResList(vm, "1920x1080");
        DEV_LOG_INFO("***** SET 0 (end) *****");
    } while(1);

    return NULL;
}

static void *setResListLoop1(void *arg)
{
    VideoManager *vm = static_cast<VideoManager*>(arg);

    pthread_detach(pthread_self());

    do {
        DEV_LOG_INFO("***** SET 1 (start) *****");
        setResList(vm, "1920x1080");
        DEV_LOG_INFO("***** SET 1 (end) *****");
    } while(1);

    return NULL;
}

static void *resLoop0(void *arg)
{
    VideoManager *vm = static_cast<VideoManager*>(arg);

    pthread_detach(pthread_self());

    getResList(vm);

    do {
        DEV_LOG_INFO("***** RES 0 (start) *****");
        usleep(5000);
        getResList(vm);
        usleep(5000);
        setResList(vm, "1920x1080");
        usleep(5000);
        vm->updateDisplayInfo();
        DEV_LOG_INFO("***** RES 0 (end) *****");
    } while(1);

    return NULL;
}

static void *resLoop1(void *arg)
{
    VideoManager *vm = static_cast<VideoManager*>(arg);

    pthread_detach(pthread_self());

    getResList(vm);

    do {
        DEV_LOG_INFO("***** RES 1 (start) *****");
        usleep(3000);
        getResList(vm);
        usleep(3000);
        setResList(vm, "1680x1050");
        usleep(3000);
        vm->updateDisplayInfo();
        DEV_LOG_INFO("***** RES 1 (end) *****");
    } while(1);

    return NULL;
}


static void writeFeature(VideoManager *vm)
{
    vm->updateDisplayInfo();
}


static void *updateFeature0(void *arg)
{
    VideoManager *vm = static_cast<VideoManager*>(arg);

    pthread_detach(pthread_self());

    do {
        DEV_LOG_INFO("### WRITE FEATURE 0 ###");
        writeFeature(vm);
    } while(1);

    return NULL;
}

static void *updateFeature1(void *arg)
{
    VideoManager *vm = static_cast<VideoManager*>(arg);

    pthread_detach(pthread_self());

    do {
        DEV_LOG_INFO("+++ WRITE FEATURE 1 +++");
        writeFeature(vm);
    } while(1);

    return NULL;
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
    vector<string> customResList = {
        //"1920x1080",
        //"1680x1050",
        //"1600x900",
        //"1440x900",
        "1366x768",
        //"1360x768",
        //"1280x1024",
        //"1280x720",
        //"1024x768"
    };

    vector<string> abnormalResList = {
        "1x1",
        "100x100",
        "1000x1000",
        "1024x720",
        "8192x8192"
    };

    int testCnt = 0;
    int testTimes = 1;//12*64*64;
    pthread_t pid_0;
    pthread_t pid_1;

    init_dev_log();

    VideoManager* vm = new VideoManager();
    vm->registerCallback(videoCallback, videoJsonCallback, NULL);

    //VideoManager* vm1 = new VideoManager();
    //vm1->registerCallback(videoCallback, videoJsonCallback, NULL);

    DEV_LOG_INFO("==================== START VIDEO TEST ====================");

    bool ret;
    dev_exec("export DISPLAY=:0");    
    pthread_create(&pid_0, NULL, resLoop0, (void *)vm);
    pthread_create(&pid_1, NULL, resLoop1, (void *)vm);

    sleep(2);

    do {
        ;
    } while (1);
    //ret = vm->isHotplugDaemonRunning();
    //DEV_LOG_INFO("isrunning %d", ret);

    #if 0
    sleep(5);

    vm->startHotplugDaemon();
    ret = vm->isHotplugDaemonRunning();
    DEV_LOG_INFO("isrunning %d", ret);


    sleep(5);

    vm->stopHotplugDaemon();
    ret = vm->isHotplugDaemonRunning();
    DEV_LOG_INFO("isrunning %d", ret);

    sleep(5);

    vm->startHotplugDaemon();
    ret = vm->isHotplugDaemonRunning();
    DEV_LOG_INFO("isrunning %d", ret);
    #endif

    #if 0
    do {
        DEV_LOG_INFO("test cnt %d", testCnt);

        getResList(vm);
        getResList(vm1);

        pthread_create(&pid, NULL, getResListLoop, (void *)vm);
        sleep(2);


        DEV_LOG_INFO("wait seconds for modify tmp file ...");
        sleep(10);

        DEV_LOG_INFO("******* get resolution list start (2) *******");
        if (vm->getResolutionList(resList) == -1) {
            DEV_LOG_ERR("####### get resolution failed #######");
        }

        DEV_LOG_INFO("~~~~~~ resolution list ~~~~~~");
        for (auto resInfo = resList.begin(); resInfo != resList.end(); ++resInfo) {
            DEV_LOG_INFO("res %s, flag %d", resInfo->res.c_str(), resInfo->flag);
        }

        DEV_LOG_INFO("******* get resolution list end (2) *******");


        pthread_create(&pid, NULL, getResListLoop, (void *)vm);
        sleep(2);

        #if 0
        /* set resolution in supported list */
        DEV_LOG_INFO("******* test resolution in supported list start *******");
        for (auto it = resList.begin(); it != resList.end(); ++it) {
            DEV_LOG_INFO("set res %s", it->res.c_str());
            if(vm->setResolution(it->res) == -1) {
                DEV_LOG_ERR("####### set resolution failed #######");
            }
        }
        DEV_LOG_INFO("******* test resolution in supported list end *******");
        #endif

        #if 0
        DEV_LOG_INFO("******* test resolution in custom list start *******");
        /* set resolution in supported list */
        for (auto it = customResList.begin(); it != customResList.end(); ++it) {
            DEV_LOG_INFO("set res %s", it->c_str());
            if(vm->setResolution(*it) == -1) {
                DEV_LOG_ERR("####### set resolution failed #######");
            }
        }
        DEV_LOG_INFO("******* test resolution in custom list end *******");
        #endif

        #if 0
        DEV_LOG_INFO("******* test resolution in abnormal list start *******");
        /* set resolution in supported list */
        for (auto it = abnormalResList.begin(); it != abnormalResList.end(); ++it) {
            DEV_LOG_INFO("set res %s", it->c_str());
            if(vm->setResolution(*it) == -1) {
                DEV_LOG_ERR("####### set resolution failed #######");
            }
        }
        DEV_LOG_INFO("******* test resolution in abnormal list end *******");
        #endif

        testCnt++;
    } while (testCnt < testTimes);
    //doJsonTest(vm);

    if (vm) {
        delete vm;
        vm = NULL;
    }
    DEV_LOG_INFO("==================== STOP VIDEO TEST =====================");

    #endif

    return 0;
}
