#ifndef _DEV_API_H
#define _DEV_API_H

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace rcdev
{

#define DEV_RES_BEST 0x1
#define DEV_RES_CUR  0x2
#define DISPCARD_DISCONNECT 0
#define DISPCARD_CONNECT    1

enum DevJsonHandle
{
    DEV_HANDLE_REQUEST_SND_DEV_LIST      = 101,
    DEV_HANDLE_SWITCH_SND_DEV            = 102,
    DEV_HANDLE_REQUEST_BACKLIGHT         = 111,
    DEV_HANDLE_SET_BACKLIGHT             = 112,
    DEV_HANDLE_SAVE_BACKLIGHT            = 113,

    DEV_HANDLE_REQUEST_SND_DEV_LIST_RET  = 201,
    DEV_HANDLE_SEND_SND_DEV_LIST         = 202,
    DEV_HANDLE_SWITCH_SND_DEV_RET        = 203,
    DEV_HANDLE_REQUEST_BACKLIGHT_RET     = 211,
    DEV_HANDLE_SET_BACKLIGHT_RET         = 212,
};

enum DevEventId
{
    DEV_EVENT_SAVE_BACKLIGHT             = 1,
    DEV_EVENT_PULSE_CONNECTED,
};

typedef struct _DevMsg
{
    int msg_id;
    string msg;
} DevMsg;

typedef struct _DevResInfo {
    string res;
    int flag;
} DevResInfo;


typedef struct _DispCardStatus {
    string devType;
    int    status;
} DispCardStatus;

//function declaration
void init_dev_log();
typedef void (*onDevEvent)(DevMsg &msg, void *user);
typedef void (*onJsonEvent)(string &msg, void *user);

class AudioManager
{
public:
    AudioManager();
    AudioManager(const AudioManager& );
    AudioManager& operator=(const AudioManager& );
    ~AudioManager();

    void registerCallback(onDevEvent cb, onJsonEvent jcb, void* user);
    void doJsonCommand(int handle, const string& json);
    void doEvent(DevMsg& msg);
    void doJsonEvent(string& json);

    int startPaService();
    int stopPaService();
    int setHdmiVoiceStatus(int status);
    bool isHdmiConnected();
    int getSoundDeviceList();
    int setSoundDevicePath();

private:
    onDevEvent    mCb;
    onJsonEvent   mJcb;
    void*         mUser;
};

class VideoManager
{
public:
    VideoManager();
    VideoManager(const VideoManager&);
    VideoManager& operator=(const VideoManager&);
    ~VideoManager();

    void registerCallback(onDevEvent cb, onJsonEvent jcb, void* user);
    void doJsonCommand(int handle, const string& json);

    int getBrightness();
    int setBrightness();
    int getResolutionList(vector<DevResInfo>& resList);
    int setResolution(string res, bool force);
    int updateDisplayInfo();
    int getDispCardStatus(vector<DispCardStatus> &statusList);
    int startHotplugDaemon();
    int stopHotplugDaemon();
    bool isHotplugDaemonRunning();

private:
    void doEvent(DevMsg& msg);
    void doJsonEvent(string& json);

    onDevEvent    mCb;
    onJsonEvent   mJcb;
    void*         mUser;
};

#define HPD_EVENT_NAME_MONITOR "MONITOR_HOTPLUG"

enum HpdType {
    HPD_NONE = 0,
    HPD_MONITOR,
    HPD_BUTT
};

enum HpdEventType {
    HPD_EVENT_MONITOR_CHANGE = 100,
};

typedef int(*onHpdEvent)(string &msg, void *user);

class HotplugManager {
    public:
        HotplugManager(int hpdType, onHpdEvent cb, void *user);
        ~HotplugManager();
    private:
        static int onEvent(void* data, size_t len, void* user);
        void reconnectService(void** monitor, bool attach, bool finite);
        int formatJson(string &json, int evenType, string eventName, string info);
        void *mMonitor;

        int mHpdType;
        onHpdEvent mCb;
        void*     mUser;
};


} //namespace

#endif //_DEV_API_H
