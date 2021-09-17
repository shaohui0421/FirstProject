#ifndef _DEVICE_INTERFACE_H
#define _DEVICE_INTERFACE_H

#include "common.h"
#include "application.h"

using namespace std;

#define IS_RES_BEST               (1<<0)
#define IS_RES_CURRENT            (1<<1)
#define DEFAULT_DISPLAY_SIZE      "1024x768"
#define DEFAULT_DISPLAY_POS       "0x0"
#define DEFAULT_DISPLAY_ROTATE    ""

//#define HPD_MONITOR                 100

#define DISPLAY_GET_MASK_PORT_NAME         0x1
#define DISPLAY_GET_MASK_IS_CONNECTED      0x2
#define DISPLAY_GET_MASK_IS_ENABLE         0x4
#define DISPLAY_GET_MASK_CURT_RES          0x8
#define DISPLAY_GET_MASK_BEST_RES          0x10
#define DISPLAY_GET_MASK_RES_LIST          0x20
#define DISPLAY_SET_MASK_ENABLE            0x1
#define DISPLAY_SET_MASK_RES               0xf

typedef struct _DevResInfo {
    string res;
    int flag;
} DevResInfo;

typedef struct _DevHotplugInfo {
    int eventType;
    string eventName;
    string devType;
    string plugInfo;
} DevHotplugInfo;

class DeviceInterface
{
public:
    DeviceInterface(Application *app);
    ~DeviceInterface();

    //common
    //void doJsonCommand(int handle, const string& json);

    //AudioManager
    int startPaService();
    int stopPaService();
    int setHdmiVoiceEnable(int enable);
    bool isHdmiConnected();
    bool getHdmiVoiceEnable();

    //VideoManager
    int getTermBrightness();
    int setTermBrightness(int brightness);
    int getResolutionList(vector<DevResInfo>& resList);
    int setResolution(const string &res);
    int setResolution(const int width, const int height);
    int getCurResolution(string &width, string &height, int flag = IS_RES_CURRENT);
    int setBestResolution(int flag = IS_RES_BEST);
    int initTermResolution(const int width, const int height);
    int getDisplayPortNumber();
    int getDisplayConnectedNumber();
    bool isDisplayConnected(int port);
    bool isDisplayEnable(int port);
    int getDisplayCurResolution(int port, string &width, string &height);
    int getDisplayBestResolution(int port, string &width, string &height);
    int getDisplayResolutionList(int port, vector<DevResInfo>& resList);
    int getDisplayResolutionList(int port, char *info, int len);
    int setDisplayEnable(int port, int enable);
    int setDisplayResolution(int port, const bool &enable, const string &res, const string &pos, const string &rotate);
    int getExtDisplayResolutionList(map<string, int> *resList);
    void getDisplayPortNameList(vector<string> &list);

    // terminal passwd
    int set_terminal_passwd(const string &pwd);
    int getPowerBootState();
    
    // system info
    int getDiskPartInfo(const char *part, DiskPartInfo &disk_part_info);
    int getAllDiskInfo(char *info, int len);
    int getAllDisksize(string &info);
    int getCpuInfo(char *info, int len);
    int getMemoryInfo(char *info, int len);
    int getSystemHardInfo(SystemHardwareInfo &system_hard_info);
private:
    //callback
    static int hotplugCallback(const char *eventName, int eventType, const char *eventContent);

    static void get_json_valuebool(bool& dst, cJSON* json, const string& key);
    static void get_json_valueint(int& dst, cJSON* json, const string& key);
    static void get_json_valuestring(string& dst, cJSON* json, const string& key);
    static void get_json_valueCstring(char *dst, cJSON* json, const string& key);
    static void get_json_child(string& dst, cJSON* json, const string& key);

    bool findDviDevice();
    int setResolutionDvi(const string &res);

    Application* _app;
};

#endif //_DEVICE_INTERFACE_H
