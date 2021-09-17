#include "dev_common.h"
#include "dev_api.h"
#include "audiocore.h"

namespace rcdev
{

AudioManager::AudioManager()
    : mCb(NULL)
    , mJcb(NULL)
    , mUser(NULL)
{
}

AudioManager::~AudioManager()
{
}

void AudioManager::registerCallback(onDevEvent cb, onJsonEvent jcb, void* user)
{
    mCb = cb;
    mJcb = jcb;
    mUser = user;
}

void AudioManager::doEvent(DevMsg& msg)
{
    if (mCb == NULL) {
        DEV_LOG_ERR("mCb is NULL!");
        return;
    }
    DEV_LOG_INFO("send DevEvent, id: %d, msg: %s", msg.msg_id, msg.msg.c_str());
    mCb(msg, mUser);
}

void AudioManager::doJsonEvent(string& json)
{
    if (mJcb == NULL) {
        DEV_LOG_ERR("mJcb is NULL!");
        return;
    }
    DEV_LOG_INFO("send json: %s", json.c_str());
    mJcb(json, mUser);
}

void AudioManager::doJsonCommand(int handle, const string& json)
{
    string out_json;

    DEV_LOG_INFO("recv json: %s", json.c_str());
    switch (handle) {
        case DEV_HANDLE_REQUEST_SND_DEV_LIST:
            out_json = "{\"handle\": 201}";
            doJsonEvent(out_json);
            break;
        case DEV_HANDLE_SWITCH_SND_DEV:
            out_json = "{\"handle\": 201}";
            doJsonEvent(out_json);
            break;
        default:
            break;
    }
}

int AudioManager::startPaService()
{
    return (AudioCore::getInstance()->paStart(this) == true) ? 0 : -1;
}
int AudioManager::stopPaService()
{
    return (AudioCore::getInstance()->paStop() == true) ? 0 : -1;
}

int AudioManager::setHdmiVoiceStatus(int status)
{
    return AudioCore::getInstance()->changeProfile((bool)status ? "hdmi" : "analog");
}

bool AudioManager::isHdmiConnected()
{
    return AudioCore::getInstance()->isHdmiConnected();
}

int AudioManager::getSoundDeviceList()
{
    //no use
    return 0;
}
int AudioManager::setSoundDevicePath()
{
    //no use
    return 0;
}

} //namespace
