#include "device_interface.h"
#include "sysabslayer_linux.h"
#include "rc_json.h"
#include "application.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

DeviceInterface::DeviceInterface(Application* app)
    : _app(app)
{
    int ret = abslayer_InitForClient("client", DeviceInterface::hotplugCallback);
    if (ret != 0) {
        LOG_WARNING("init hotplug failed:%d", ret);
    }

    // init terminal port white ssid
    ret = abslayer_securityInit();
    if (ret != 0) {
        LOG_WARNING("init security failed:%d", ret);
    }
}

DeviceInterface::~DeviceInterface()
{
}

#define STR_SIZE(STRING) (sizeof(STRING)-1)

#define SET_JSON_WRITE_INIT(json, json_string)\
    string json_string;\
    char* json_text;\
    cJSON * json = cJSON_CreateObject();

#define SET_JSON_WRITE_FREE(json)\
    json_text = cJSON_PrintUnformatted(json);\
    json_string = json_text;\
    LOG_DEBUG("create json_text \n%s", json_text);\
    if(json_text)\
    {\
        free(json_text);\
    }\
    cJSON_Delete(json);

#define GET_JSON_JUDGE_NULL_INIT(pMsg, dst, json, key)\
    pMsg = cJSON_GetObjectItem (json, key.c_str());\
    if(pMsg == NULL)\
    {\
        LOG_WARNING("can not find json key %s", key.c_str());\
        return;\
    }

void DeviceInterface::get_json_valuebool(bool& dst, cJSON* json, const string& key)
{
    cJSON* pMsg = NULL;
    GET_JSON_JUDGE_NULL_INIT(pMsg, dst, json, key);
    dst = pMsg->valueint;
}
void DeviceInterface::get_json_valueint(int& dst, cJSON* json, const string& key)
{
    cJSON* pMsg = NULL;
    GET_JSON_JUDGE_NULL_INIT(pMsg, dst, json, key);
    dst = pMsg->valueint;
}

void DeviceInterface::get_json_valuestring(string& dst, cJSON* json, const string& key)
{
    cJSON* pMsg = NULL;
    GET_JSON_JUDGE_NULL_INIT(pMsg, dst, json, key);
    dst = pMsg->valuestring;
}

void DeviceInterface::get_json_valueCstring(char *dst, cJSON* json, const string& key)
{
    cJSON* pMsg = NULL;
    GET_JSON_JUDGE_NULL_INIT(pMsg, dst, json, key);
    strcpy(dst, pMsg->valuestring);
}

void DeviceInterface::get_json_child(string& dst, cJSON* json, const string& key)
{
    cJSON* pMsg = NULL;
    GET_JSON_JUDGE_NULL_INIT(pMsg, dst, json, key);
    dst = cJSON_PrintUnformatted(pMsg);
}
#undef GET_JSON_JUDGE_NULL_INIT

#if 0
void DeviceInterface::audioCallback(rcdev::DevMsg& msg, void *user)
{
    int hdmi_status = 0;
    Application* app = ((DeviceInterface*)user)->_app;

    if (app == NULL) {
        LOG_ERR("app is NULL!");
        return;
    }
    switch (msg.msg_id) {
        case rcdev::DEV_EVENT_PULSE_CONNECTED:
            if (app->get_vm()->get_vmManage()->get_start_vm_is_emulation() && ((DeviceInterface*)user)->isHdmiConnected()) {
                hdmi_status = app->_userinfomgr.get_hdmi_audio_info();
            } else {
                hdmi_status = FALSE;
                LOG_WARNING("the vm is not emulation or hdmi is not connected\n");
            }
            app->_device_interface->setHdmiVoiceStatus(hdmi_status);
            break;
        default:
            break;
    }
}

void DeviceInterface::audioJsonCallback(string& msg, void *user)
{
    Application* app = ((DeviceInterface*)user)->_app;

    if (app == NULL) {
        LOG_ERR("app is NULL!");
        return;
    }
    app->_vm->get_vmMessage()->message_terminal_device_msg(msg);
}

void DeviceInterface::videoCallback(rcdev::DevMsg& msg, void *user)
{
    Application* app = ((DeviceInterface*)user)->_app;

    if (app == NULL) {
        LOG_ERR("app is NULL!");
        return;
    }
    switch (msg.msg_id) {
        case rcdev::DEV_EVENT_SAVE_BACKLIGHT:
            //TODO save backlight to ini
            break;
        default:
            break;
    }
}

void DeviceInterface::videoJsonCallback(string& msg, void *user)
{
    Application* app = ((DeviceInterface*)user)->_app;

    if (app == NULL) {
        LOG_ERR("app is NULL!");
        return;
    }
    app->get_vm()->get_vmMessage()->message_terminal_device_msg(msg);
}
#endif

int DeviceInterface::hotplugCallback(const char *eventName, int eventType, const char *eventContent)
{
    Application *app = Application::get_application();
    if (app == NULL) {
        LOG_ERR("app is NULL!");
        return -1;
    }

    if (app->get_vm()->vm_is_vm_running()) {
        LOG_DEBUG("vm is running, so ignore the hotplug callback");
        return 0;
    }

    DeviceInterface *m_user = app->get_device_interface();

    DevHotplugInfo info;
    info.eventName = eventName;
    info.eventType = eventType;

    cJSON *json = cJSON_Parse(eventContent);
    get_json_valuestring(info.devType, json, "devType");
    get_json_valuestring(info.plugInfo, json, "plugInfo");
    cJSON_Delete(json);

    LOG_INFO("Hot plug plugInfo: %s, eventName: %s, eventType %d, devType %s", \
                info.plugInfo.c_str(), \
                info.eventName.c_str(), \
                info.eventType, \
                info.devType.c_str());

    switch (info.eventType) {
    case HPD_EVENT_MONITOR_CHANGE: {
        int ret = 0;
        string width;
        string height;
        char res[128] = {0};

        /* get resolution info */
        ret = m_user->getCurResolution(width, height, IS_RES_BEST);
        if (ret != 0) {
            LOG_ERR("get best resolution failed");
            return -1;
        }
        sprintf(res, "%sx%s", width.c_str(), height.c_str());

        /* set resolution */
        ret = m_user->setResolution(res);
        if (ret != 0) {
            LOG_ERR("setResolution fail, ret %d", ret);
            return -1;
        }
        if (app->get_UsrUserInfoMgr()->delete_resolution_info_section() == 0) {
            LOG_ERR("delete_resolution_info_section succ, ret ");
        }
        l2u_redisplay_adapt_control(atoi(width.c_str()), atoi(height.c_str()));
        // when public mode adapt ui and stop the timer, so start the timer again
        app->l2u_hotplug_show_public_login();
    } break;
    default:
        break;
    }

    return 0;
}

#if 0
void DeviceInterface::doJsonCommand(int handle, const string& json)
{
    switch (handle) {
    case rcdev::DEV_HANDLE_REQUEST_SND_DEV_LIST:
    case rcdev::DEV_HANDLE_SWITCH_SND_DEV:
        if (_audio_manager != NULL) {
            _audio_manager->doJsonCommand(handle, json);
        }
        break;
    case rcdev::DEV_HANDLE_REQUEST_BACKLIGHT:
    case rcdev::DEV_HANDLE_SET_BACKLIGHT:
        if (_video_manager != NULL) {
            _video_manager->doJsonCommand(handle, json);
        }
        break;
    default:
        break;
    }
}
#endif

/********** AudioManager **********/
int DeviceInterface::startPaService()
{
    int ret = abslayer_audioInit();
    LOG_INFO("start pulse audio service, ret = %d", ret);
    return ret;
}

int DeviceInterface::stopPaService()
{
    int ret = abslayer_audioDeInit();
    LOG_INFO("stop pulse audio service, ret = %d", ret);
    return ret;
}

bool DeviceInterface::isHdmiConnected()
{
    bool ret;
    if (abslayer_getHdmiVoiceStatus() == -11001) {
        ret = false;
    } else {
        //compatible HDMI config file
        if (get_file_exist(USER_HDMI_AUDIO_INI)) {
            Application::get_application()->_userinfomgr.init_hdmi_audio_info();
        }
        ret = true;
    }
    LOG_INFO("is HDMI Connected, ret is %d", ret);
    return ret;
}

bool DeviceInterface::getHdmiVoiceEnable()
{
    int ret = abslayer_getHdmiVoiceStatus();
    LOG_INFO("get HDMI status, ret is %d", ret);
    if (ret == 1) {
        return true;
    }
    return false;
}

int DeviceInterface::setHdmiVoiceEnable(int enable)
{
    int ret = abslayer_setHdmiVoiceStatus(enable);
    LOG_INFO("set HDMI enable:%d, ret is %d", enable, ret);
    return ret;
}

/********** VideoManager **********/
int DeviceInterface::getTermBrightness()
{
    int ret = abslayer_getBrightness();
    LOG_INFO("get brightness, ret = %d", ret);
    return ret;
}

int DeviceInterface::setTermBrightness(int brightness)
{
    int ret = abslayer_setBrightness(brightness);
    LOG_INFO("set brightness, ret = %d", ret);
    return ret;
}

int DeviceInterface::getResolutionList(vector<DevResInfo>& resList)
{
    return getDisplayResolutionList(0, resList);
}

int DeviceInterface::setResolution(const string &res)
{
    int ret = 0;

    if (findDviDevice() == false) {
        //not found DVI device
        ret = setDisplayResolution(0, true, res, DEFAULT_DISPLAY_POS, DEFAULT_DISPLAY_ROTATE);
        LOG_INFO("setDeviceResolution, res:%s, ret = %d", res.c_str(), ret);
    } else {
        //rc_system("xrandr --output HDMI1 --mode 1024x768 --dpi 96 --pos 0x0 --rotate normal --reflect normal --output DVI-I-1-1 --off --output DP1 --off 2>&1");
        ret = setResolutionDvi(res);
    }
    return ret;
}

int DeviceInterface::setResolution(const int width, const int height)
{
    char res_buf[16];
    sprintf(res_buf, "%dx%d", width, height);
    return setResolution(res_buf);
}

/**
* function: get current resolution 
* param: width : display width  height : display height
*
* return: ret -1 error 0 ok
*/
int DeviceInterface::getCurResolution(string &width, string &height, int flag)
{
    int ret = 0;

    if (flag & IS_RES_CURRENT) {
        ret = getDisplayCurResolution(0, width, height);
    } else {
        ret = getDisplayBestResolution(0, width, height);
    }

    return ret;
}

int DeviceInterface::setBestResolution(const int flag)
{
    string str_w, str_h;
    int ret = 0;

    ret = getCurResolution(str_w, str_h, flag);
    if (ret != 0) {
        LOG_WARNING("get optimum resolution failed, ret=%d", ret);
        return -1;
    }

    ret = setResolution(atoi(str_w.c_str()), atoi(str_h.c_str()));
    return ret;
}

/**
* function: init terminal resolution
* param: width : display width  height : display height
*/
int DeviceInterface::initTermResolution(const int width, const int height)
{
    vector<DevResInfo> res_list;
    char res_buf[16] = {0};
    string str_w, str_h;
    int res_size = 0;
    int ret = 0;

    ret = getDisplayResolutionList(0, res_list);
    if (ret != 0) {
        LOG_ERR("get support resolution list fail!");
        return ret;
    }

    res_size = res_list.size();
    if (res_size == 0) {
        LOG_WARNING("cannot get device resolution info, set resolution: %dx%d", width, height);
        //no need to set resolution if cannot get xrandr list
        ret = setResolution(width, height);
        return ret;
    }

    sprintf(res_buf, "%dx%d", width, height);
    // find current resolution in resList
    for (int i = 0; i < res_size; i++) {
        if (strcmp(res_list[i].res.c_str(), res_buf) == 0) {
            LOG_INFO("find current resolution in resList, set resolution: %dx%d", width, height);
            ret = setResolution(width, height);
            return ret;
        }
    }

    // cannot find current resolution in resList
    LOG_INFO("cannot find current resolution in resList, reset opt resolution");
    ret = getCurResolution(str_w, str_h, IS_RES_BEST);
    if (_app != NULL && _app->get_UsrUserInfoMgr()->delete_resolution_info_section() == 0) {
        sprintf(res_buf, "%sx%s", str_w.c_str(), str_h.c_str());
        ret = setResolution(res_buf);
    }

    return ret;
}

/* Display Info Interface */
int DeviceInterface::getDisplayPortNumber(void)
{
    int ret = abslayer_getDisplayPortNum();
    LOG_INFO("getDisplayPortNum, ret:%d", ret);
    return ret;
}

int DeviceInterface::getDisplayConnectedNumber(void)
{
    int ret = 0;

    for (int i = 0; i < abslayer_getDisplayPortNum(); i++) {
        if (isDisplayConnected(i+1)) {
            ret++;
        }
    }

    if (ret > 2) {
        ret = 2;
    }
    LOG_INFO("getDisplayConnectedNumber, ret:%d", ret);
    return ret;
}

bool DeviceInterface::isDisplayConnected(int port)
{
    char info[32] = {0};
    int isConnected = 0;

    int ret = abslayer_getDisplayInfo(port, DISPLAY_GET_MASK_IS_CONNECTED, info, STR_SIZE(info));
    if (ret < 0) {
        LOG_WARNING("port:%d, getDisplayInfo failed, ret:%d", port, ret);
        return false;
    }

    cJSON *json = cJSON_Parse(info);
    get_json_valueint(isConnected, json, "isConnected");
    cJSON_Delete(json);

    LOG_INFO("port:%d, connected, ret:%d", port, isConnected);
    if (isConnected) {
        return true;
    }
    return false;
}

bool DeviceInterface::isDisplayEnable(int port)
{
    char info[32] = {0};
    int isEnabled;

    int ret = abslayer_getDisplayInfo(port, DISPLAY_GET_MASK_IS_ENABLE, info, STR_SIZE(info));
    if (ret < 0) {
        LOG_WARNING("port:%d, getDisplayInfo failed, ret:%d", port, ret);
        return false;
    }

    cJSON *json = cJSON_Parse(info);
    get_json_valueint(isEnabled, json, "isEnabled");
    cJSON_Delete(json);

    LOG_INFO("port:%d, enable, ret:%d", port, isEnabled);
    if (isEnabled) {
        return true;
    }
    return false;
}

int DeviceInterface::getDisplayCurResolution(int port, string &width, string &height)
{
    char info[64] = {0};
    string _res;
    int w, h;
    int isConnected = 0;

    int ret = abslayer_getDisplayInfo(port, DISPLAY_GET_MASK_CURT_RES | DISPLAY_GET_MASK_IS_CONNECTED, info, STR_SIZE(info));
    if (ret < 0) {
        LOG_WARNING("port:%d, getDisplayInfo failed, ret:%d", port, ret);
        return -1;
    }

    cJSON *json = cJSON_Parse(info);
    get_json_valuestring(_res,     json, "curRes");
    get_json_valueint(isConnected, json, "isConnected");
    cJSON_Delete(json);

    if (isConnected == 0) {
        LOG_WARNING("port:%d is not connected", port);
        return -1;
    }

    sscanf(_res.c_str(), "%dx%d", &w, &h);
    width = std::to_string(w);
    height = std::to_string(h);
    LOG_INFO("port:%d, cur res, width %s height %s", port, width.c_str(), height.c_str());

    return 0;
}

int DeviceInterface::getDisplayBestResolution(int port, string &width, string &height)
{
    char info[64] = {0};
    string _res;
    int w, h;
    int isConnected = 0;

    int ret = abslayer_getDisplayInfo(port, DISPLAY_GET_MASK_BEST_RES | DISPLAY_GET_MASK_IS_CONNECTED, info, STR_SIZE(info));
    if (ret < 0) {
        LOG_WARNING("port:%d, getDisplayInfo failed, ret:%d", port, ret);
        return -1;
    }

    cJSON *json = cJSON_Parse(info);
    get_json_valuestring(_res,     json, "bestRes");
    get_json_valueint(isConnected, json, "isConnected");
    cJSON_Delete(json);

    if (isConnected == 0) {
        LOG_WARNING("port:%d is not connected", port);
        return -1;
    }

    sscanf(_res.c_str(), "%dx%d", &w, &h);
    width = std::to_string(w);
    height = std::to_string(h);
    LOG_INFO("port:%d, best res, width %s height %s", port, width.c_str(), height.c_str());

    return 0;
}

int DeviceInterface::getDisplayResolutionList(int port, char *info, int len)
{
    if (info == NULL) {
        LOG_DEBUG("param err");
        return -1;
    }
    int ret = abslayer_getDisplayInfo(port,
                             DISPLAY_GET_MASK_IS_CONNECTED | DISPLAY_GET_MASK_RES_LIST | DISPLAY_GET_MASK_CURT_RES | DISPLAY_GET_MASK_BEST_RES,
                             info,
                             len);
    LOG_DEBUG("port:%d,info: %s", port, info);
    return ret;
}

int DeviceInterface::getDisplayResolutionList(int port, vector<DevResInfo>& resList)
{
    char info[640] = {0};
    char res_list[512] = {0};
    string best_res;
    string curt_res;
    const char *sep = ";";
    char *_res;
    DevResInfo default_res;
    int isConnected = 0;

    int ret = getDisplayResolutionList(port, info, STR_SIZE(info));
    cJSON *json = cJSON_Parse(info);
    if (json == NULL) {
        return -2;
    }
    get_json_valuestring(best_res, json,  "bestRes");
    get_json_valuestring(curt_res, json,  "curRes");
    get_json_valueCstring(res_list, json, "resList");
    get_json_valueint(isConnected, json, "isConnected");
    cJSON_Delete(json);

    if (isConnected == 0) {
        LOG_WARNING("port:%d is not connected", port);
        return -2;
    }

    if (ret < 0 || strlen(res_list) == 0) {
        LOG_WARNING("port:%d, getDisplayInfo failed, ret:%d", port, ret);
        default_res.res = DEFAULT_DISPLAY_SIZE;
        default_res.flag = IS_RES_CURRENT | IS_RES_BEST;
        resList.push_back(default_res);
        return -1;
    }

    _res = strtok(res_list, sep);
    while (_res) {
        memset(&default_res, 0, sizeof(DevResInfo));
        LOG_DEBUG("strtok _res:%s", _res);
        if (strchr(_res, 'x') != NULL) {
            default_res.res = _res;
            if (strcmp(curt_res.c_str(), _res) == 0) {
                default_res.flag |= IS_RES_CURRENT;
            }
            if (strcmp(best_res.c_str(), _res) == 0) {
                default_res.flag |= IS_RES_BEST;
            }
            resList.push_back(default_res);
        }
        _res = strtok(NULL, sep);
    }

    return 0;
}

int DeviceInterface::setDisplayEnable(int port, int enable)
{
    SET_JSON_WRITE_INIT(json, json_string);
    cJSON_AddNumberToObject(json, "enable", enable);
    SET_JSON_WRITE_FREE(json);

    int ret = abslayer_setDisplayConfig(port, DISPLAY_SET_MASK_ENABLE, json_string.c_str());
    if (ret < 0) {
        LOG_WARNING("port:%d, setDisplayConfig:%d failed, ret:%d", port, enable, ret);
        return -1;
    }
    return 0;
}

int DeviceInterface::setDisplayResolution(int port, const bool &enable, const string &res, const string &pos, const string &rotate)
{
    SET_JSON_WRITE_INIT(json, json_string);
    cJSON_AddNumberToObject(json, "enable", enable);
    cJSON_AddStringToObject(json, "res",    res.c_str());
    cJSON_AddStringToObject(json, "pos",    pos.c_str());
    cJSON_AddStringToObject(json, "rotate", rotate.c_str());
    SET_JSON_WRITE_FREE(json);

    int ret = abslayer_setDisplayConfig(port, DISPLAY_SET_MASK_RES, json_string.c_str());
    if (ret < 0) {
        LOG_WARNING("port:%d, enable:%d, res:%s, pos:%s, rotate:%s, ret:%d", port, enable, res.c_str(), pos.c_str(), rotate.c_str(), ret);
        return -1;
    }

    return 0;
}

int DeviceInterface::getExtDisplayResolutionList(map<string, int> *resList)
{
    char info[640] = {0};
    char res_list[512] = {0};
    string best_res;
    string curt_res;
    const char *sep = ";";
    char *_res;
    DevResInfo default_res;
    int isConnected = 0;
    int i = 0, num = 0, ret;

    for (i = 1; i <= getDisplayPortNumber(); i++) {
        ret = getDisplayResolutionList(i, info, STR_SIZE(info));

        cJSON *json = cJSON_Parse(info);
        if (json == NULL) {
            LOG_WARNING("json is null");
            continue;
        }
        get_json_valuestring(best_res, json,  "bestRes");
        get_json_valuestring(curt_res, json,  "curRes");
        get_json_valueCstring(res_list, json, "resList");
        get_json_valueint(isConnected, json, "isConnected");
        cJSON_Delete(json);

        if (isConnected == 0) {
            LOG_WARNING("port:%d is not connected", i);
            continue;
        }

        if (ret < 0 || strlen(res_list) == 0) {
            LOG_WARNING("port:%d, getDisplayInfo failed, ret:%d", i, ret);
            resList[num].insert(map<string, int>::value_type(DEFAULT_DISPLAY_SIZE, IS_RES_CURRENT | IS_RES_BEST));
            num++;
            continue;
        }

        _res = strtok(res_list, sep);
        while (_res) {
            memset(&default_res, 0, sizeof(DevResInfo));
            LOG_DEBUG("strtok _res:%s", _res);
            if (strchr(_res, 'x') != NULL) {
                default_res.res = _res;
                if (strcmp(curt_res.c_str(), _res) == 0) {
                    default_res.flag |= IS_RES_CURRENT;
                }
                if (strcmp(best_res.c_str(), _res) == 0) {
                    default_res.flag |= IS_RES_BEST;
                }
                resList[num].insert(map<string, int>::value_type(default_res.res, default_res.flag));
            }
            _res = strtok(NULL, sep);
        }
        num++;
    }
    LOG_DEBUG("getExtDisplayResolutionList ret: %d", num);
    return num;
}

void DeviceInterface::getDisplayPortNameList(vector<string> &list)
{
    char info[256] = {0};
    string portName;
    int isConnected = 0;
    int ret = 0;

    for (int port = 1; port <= getDisplayPortNumber(); port++) {
        ret = abslayer_getDisplayInfo(port,
                             DISPLAY_GET_MASK_PORT_NAME | DISPLAY_GET_MASK_IS_CONNECTED,
                             info,
                             STR_SIZE(info));

        if (ret < 0) {
            LOG_WARNING("port:%d, getDisplayInfo failed, ret:%d", port, ret);
            continue;
        }
        cJSON *json = cJSON_Parse(info);
        if (json == NULL) {
            LOG_WARNING("json is null");
            continue;
        }
        get_json_valuestring(portName, json,  "portName");
        get_json_valueint(isConnected, json, "isConnected");

        if (isConnected == 0) {
            LOG_WARNING("port:%d is not connected", port);
            continue;
        }
        list.push_back(portName);
    }    
}

bool DeviceInterface::findDviDevice()
{
    vector<string> portNameList;
    vector<string>::iterator iter;
    string portName;

    getDisplayPortNameList(portNameList);
    for (iter = portNameList.begin(); iter != portNameList.end(); ++iter) {
        portName = *iter;
        if (portName.find("DVI") != string::npos) {
            return true;
        }
    }
    return false;
}

int DeviceInterface::setResolutionDvi(const string &res)
{
    Application *app = Application::get_application();
    char info[256] = {0};
    string portName;
    int isConnected = 0;
    int ret = 0;

    LOG_INFO("enter setResolutionDvi");

    if (app == NULL) {
        LOG_ERR("app is NULL!");
        return -1;
    }
    if (app->get_vm()->vm_is_vm_running()) {
        LOG_DEBUG("vm is running, so ignore setResolutionDvi");
        return -1;
    }
    for (int port = 1; port <= getDisplayPortNumber(); port++) {
        ret = abslayer_getDisplayInfo(port,
                             DISPLAY_GET_MASK_PORT_NAME | DISPLAY_GET_MASK_IS_CONNECTED,
                             info,
                             STR_SIZE(info));

        if (ret < 0) {
            LOG_WARNING("port:%d, getDisplayInfo failed, ret:%d", port, ret);
            continue;
        }
        cJSON *json = cJSON_Parse(info);
        if (json == NULL) {
            LOG_WARNING("json is null");
            continue;
        }
        get_json_valuestring(portName, json,  "portName");
        get_json_valueint(isConnected, json, "isConnected");
        cJSON_Delete(json);

        if (isConnected == 0) {
            LOG_WARNING("port:%d is not connected", port);
            continue;
        }
        if (portName.find("DVI") != string::npos) {
            //DVI device port
            ret = setDisplayResolution(port, false, res, DEFAULT_DISPLAY_POS, DEFAULT_DISPLAY_ROTATE);
        } else {
            //normal display device port
            ret = setDisplayResolution(port, true, res, DEFAULT_DISPLAY_POS, DEFAULT_DISPLAY_ROTATE);
        }
        if (ret < 0) {
            LOG_DEBUG("setDisplayResolution err, portName = %s", portName.c_str());
            break;
        }
    }

    return ret;
}

int DeviceInterface::set_terminal_passwd(const string &pwd)
{
    cJSON * json;
    string in_json;
    int ret = 0;

    LOG_DEBUG("enter %s ", __func__);
    json = cJSON_CreateObject();
    if (json == NULL) {
        LOG_ERR("set_terminal_passwd create json error");
        return -1;
    }

    cJSON_AddStringToObject(json, "passwd", pwd.c_str());
    in_json = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    LOG_INFO("in_json = %s", in_json.c_str());
    ret = abslayer_setPasswdByCipher(in_json.c_str());
    if (ret != 0) {
        LOG_ERR("abslayer_setPasswdByCipher err %d", ret);
    }
    return ret;
}

int DeviceInterface::getPowerBootState()
{
    int ret = abslayer_getPowerState();
    LOG_INFO("get power state, ret = %d", ret);
    if(ret == -11001) {
        LOG_INFO("not support power boot, ret = %d", ret);
        return -1;
    }
    return ret;
}

int DeviceInterface::getAllDiskInfo(char *info, int len)
{
    if (info == NULL) {
        LOG_DEBUG("param err");
        return -1;
    }
    int ret = abslayer_getAllDiskInfo(info, len);
    LOG_DEBUG("len:%d,info: %s", len, info);
    return ret;
}

int DeviceInterface::getAllDisksize(string &info)
{
    info = "";
    char disk_info[1024] = {0};
    //child node
    cJSON *json_child = NULL;
    string dev_form = "";
    string dev_media = "";
    string dev_totalsize = "";
    int ret = getAllDiskInfo(disk_info, STR_SIZE(disk_info));
    if (ret != 0)
    {
        return ret;
    }
    cJSON *json = cJSON_Parse(disk_info);
    if (json == NULL) {
        return -2;
    }
    int size = cJSON_GetArraySize(json);
    for (int i = 0; i < size; i++) {
        json_child = cJSON_GetArrayItem(json, i);
        if(cJSON_Object == json_child->type) {
            get_json_valuestring(dev_form, json_child,  "dev_form");
            get_json_valuestring(dev_media, json_child,  "dev_media");
            get_json_valuestring(dev_totalsize, json_child,  "dev_totalsize");
            if (dev_form == "virtual" || dev_media == "EMMC"){
                continue ;
            }
            double total_size_d = stod(dev_totalsize.c_str()) / 1024 / 1024 / 1024;
            //int total_size = (int)ceil(total_size_d);
            int total_size = (int)round(total_size_d);
            dev_totalsize = std::to_string(total_size) + "G";
            string dev_result = dev_media + " " + dev_totalsize;
            LOG_DEBUG("len:%d,info: %.3f,result:%s", total_size, total_size_d, dev_result.c_str());
            if (info != ""){
                info = info + ",";
            }
            info = info + dev_result;
        }
    }
    cJSON_Delete(json);
    return 0;
}


int DeviceInterface::getCpuInfo(char *info, int len)
{
    if (info == NULL) {
        LOG_DEBUG("param err");
        return -1;
    }
    // info = Inte(R) Core(TM) i3-6100U CPU @ 2.30GHz
    int ret = abslayer_getCpuInfo(info, len);
    LOG_DEBUG("len:%d,info: %s", len, info);
    return ret;
}

int DeviceInterface::getMemoryInfo(char *info, int len)
{
    if (info == NULL) {
        LOG_DEBUG("param err");
        return -1;
    }
    //info = 2147483648
    int ret = abslayer_getMemoryInfo(info, len);
    LOG_DEBUG("len:%d,info: %s", len, info);
    return ret;
}


int DeviceInterface::getSystemHardInfo(SystemHardwareInfo &system_hard_info)
{
    char cpu_info[512] = {0};
    //char memory_info[512] = {0};
    string disk_size = "";
    getAllDisksize(disk_size);
    getCpuInfo(cpu_info, STR_SIZE(cpu_info));
    system_hard_info.disk_size = disk_size;
    system_hard_info.cpu = cpu_info;
    system_hard_info.memory = "";
    return 0;
}

int DeviceInterface::getDiskPartInfo(const char *part, DiskPartInfo &disk_part_info)
{
    char info[256] = {0};
    string data_info;
    string total;
    string available;
    cJSON *json = NULL;
    cJSON *json_child = NULL;

    int ret = abslayer_getDiskPartitionInfo(info, STR_SIZE(info));

    if (ret < 0) {
        LOG_INFO("abslayer_getDiskPartitionInfo failed, ret = %d", ret);
        return -1;
    }
    LOG_INFO("getDiskPartInfo = %s", info);

    json = cJSON_Parse(info);
    if (json == NULL) {
        LOG_ERR("json is null");
        return -1;
    }

    get_json_child(data_info, json, part);
    json_child = cJSON_Parse(data_info.c_str());
    if (json_child == NULL) {
        LOG_ERR("json_child is null");
        ret = -1;
        goto END;
    }

    get_json_valuestring(total, json_child, "total");
    get_json_valuestring(available, json_child, "available");
    disk_part_info.total = atoll(total.c_str());
    disk_part_info.available = atoll(available.c_str());

    LOG_INFO("getDiskPartInfo part = %s, total = %lld, available = %lld", part, disk_part_info.total, disk_part_info.available);

END:
    if (json) {
        cJSON_Delete(json);
    }

    if (json_child) {
        cJSON_Delete(json_child);
    }

    return ret;
}
