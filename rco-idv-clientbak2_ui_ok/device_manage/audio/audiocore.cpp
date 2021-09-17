
#include "audiocore.h"
#include "sink.h"
#include "source.h"
#include "sinkinput.h"
#include "sourceoutput.h"

#define RJ_HDMI_AUDIO          "hdmi"
#define RJ_ANALOG_AUDIO        "analog"

AudioCore*           AudioCore::mInstance = NULL;
rcdev::AudioManager* AudioCore::mAm = NULL;
AudioCore::GC        AudioCore::mGc;
pthread_once_t       AudioCore::mOnce;

static rcdev::DevMsg pulseMsg = {
    .msg_id = rcdev::DEV_EVENT_PULSE_CONNECTED,
    .msg = "PulseAudio connect success"
};

AudioCore::AudioCore()
    : defaultSinkIdx(0),
      defaultSourceIdx(0),
      innerCardIdx(0),
      context(NULL),
      bHDMI(true),
      api(NULL),
      m(NULL),
      retry(3),
      retval(0),
      mInited(false),
      minnerState(false) {

}

AudioCore::~AudioCore() {

}

AudioCore* AudioCore::getInstance() {
    pthread_once(&mOnce, init);
    return mInstance;
}

void AudioCore::init() {
    mInstance = new AudioCore();
}

void AudioCore::sink_cb(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
    AudioCore *w = static_cast<AudioCore*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(w->context) == PA_ERR_NOENTITY)
            return;

        DEV_LOG_ERR("Sink callback failure");
        return;
    }

    if (eol > 0) {
        return;
    }

    w->updateSink(*i);
}

void AudioCore::sink_input_cb(pa_context *, const pa_sink_input_info *i, int eol, void *userdata) {
    AudioCore *w = static_cast<AudioCore*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(w->context) == PA_ERR_NOENTITY)
            return;

        DEV_LOG_ERR("Sink input callback failure");
        return;
    }

    if (eol > 0) {
        return;
    }

    w->updateSinkInput(*i);
}

void AudioCore::source_output_cb(pa_context *, const pa_source_output_info *i, int eol, void *userdata) {
    AudioCore *w = static_cast<AudioCore*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(w->context) == PA_ERR_NOENTITY)
            return;

        DEV_LOG_ERR("Source output callback failure");
        return;
    }

    if (eol > 0) {
        return;
    }

    w->updateSourceOutput(*i);
}

void AudioCore::source_cb(pa_context *, const pa_source_info *i, int eol, void *userdata) {
    AudioCore *w = static_cast<AudioCore*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(w->context) == PA_ERR_NOENTITY)
            return;

        DEV_LOG_ERR("Source callback failure");
        return;
    }

    if (eol > 0) {
        return;
    }

    // grep the monitor device
    if (i->monitor_of_sink != PA_INVALID_INDEX) {
        return;
    }

    w->updateSource(*i);
}

void AudioCore::server_info_cb(pa_context *, const pa_server_info *i, void *userdata) {
    AudioCore *w = static_cast<AudioCore*>(userdata);
    if (!i) {
        DEV_LOG_ERR("Server info callback failure");
        return;
    }

    if (strstr(i->default_source_name, "auto_null") != NULL || strstr(i->default_sink_name, "auto_null") != NULL) {
        DEV_LOG_ERR("The system is in Abnormal status.(the snd driver may load fail)");
        w->paStop();
        return;
    }

    w->updateServer(*i);
}

// PA_SUBSCRIPTION_EVENT_NEW    - 0x0000U
// PA_SUBSCRIPTION_EVENT_CHANGE - 0x0010U
// PA_SUBSCRIPTION_EVENT_REMOVE - 0x0020U
void AudioCore::subscribe_cb(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata) {
    AudioCore *w = static_cast<AudioCore*>(userdata);
    DEV_LOG_INFO("case:%#x type:%#x", t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK, t & PA_SUBSCRIPTION_EVENT_TYPE_MASK);
    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
        case PA_SUBSCRIPTION_EVENT_SINK:     //0x0000U
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
                w->removeSink(index);
                Sink::autoDefault(w);
            } else if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_NEW) {
                pa_operation *o;
                if (!(o = pa_context_get_sink_info_by_index(c, index, sink_cb, w))) {
                    DEV_LOG_ERR("pa_context_get_sink_info_by_index() failed");
                    return;
                }
                pa_operation_unref(o);
            } else {
                DEV_LOG_INFO("[sink]ignore this event type:%#x", t & PA_SUBSCRIPTION_EVENT_TYPE_MASK);
                return;
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SOURCE:     //0x0001U
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
                w->removeSource(index);
                Source::autoDefault(w);
            } else if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_NEW) {
                pa_operation *o;
                if (!(o = pa_context_get_source_info_by_index(c, index, source_cb, w))) {
                    DEV_LOG_ERR("pa_context_get_source_info_by_index() failed");
                    return;
                }
                pa_operation_unref(o);
            } else {
                DEV_LOG_INFO("[source]ignore this event type:%#x", t & PA_SUBSCRIPTION_EVENT_TYPE_MASK);
                return;
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SINK_INPUT:     //0x0002U
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
                w->removeSinkInput(index);
            } else {
                pa_operation *o;
                if (!(o = pa_context_get_sink_input_info(c, index, sink_input_cb, w))) {
                    DEV_LOG_ERR("pa_context_get_sink_input_info() failed");
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:     //0x0003U
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
                w->removeSourceOutput(index);
            } else {
                pa_operation *o;
                if (!(o = pa_context_get_source_output_info(c, index, source_output_cb, w))) {
                    DEV_LOG_ERR("pa_context_get_sink_input_info() failed");
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SERVER: {     //0x0007U
                pa_operation *o;
                if (!(o = pa_context_get_server_info(c, server_info_cb, w))) {
                    DEV_LOG_ERR("pa_context_get_server_info() failed");
                    return;
                }
                pa_operation_unref(o);
            }
            break;
    }
}

void AudioCore::context_state_callback(pa_context *c, void *userdata) {
    AudioCore *w = static_cast<AudioCore*>(userdata);

    DEV_LOG_INFO(" state:%d", pa_context_get_state(c));
    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;

        case PA_CONTEXT_READY: {
            pa_operation *o;
            w->mInited = true;

            pa_context_set_subscribe_callback(c, subscribe_cb, w);
            if (!(o = pa_context_subscribe(c, (pa_subscription_mask_t)
                                           (PA_SUBSCRIPTION_MASK_SINK|
                                            PA_SUBSCRIPTION_MASK_SOURCE|
                                            PA_SUBSCRIPTION_MASK_SINK_INPUT|
                                            PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT), NULL, NULL))) {
                DEV_LOG_ERR("pa_context_subscribe() failed");
                return;
            }
            pa_operation_unref(o);

            if (!(o = pa_context_get_server_info(c, server_info_cb, w))) {
                DEV_LOG_ERR("pa_context_get_server_info() failed");
                return;
            }
            pa_operation_unref(o);

            if (!(o = pa_context_get_sink_info_list(c, sink_cb, w))) {
                DEV_LOG_ERR("pa_context_get_sink_info_list() failed");
                return;
            }
            pa_operation_unref(o);

            if (!(o = pa_context_get_source_info_list(c, source_cb, w))) {
                DEV_LOG_ERR("pa_context_get_source_info_list() failed");
                return;
            }
            pa_operation_unref(o);

            if (!(o = pa_context_get_sink_input_info_list(c, sink_input_cb, w))) {
                DEV_LOG_ERR("pa_context_get_sink_input_info_list() failed");
                return;
            }
            pa_operation_unref(o);

            if (!(o = pa_context_get_source_output_info_list(c, source_output_cb, w))) {
                DEV_LOG_ERR("pa_context_get_source_output_info_list() failed");
                return;
            }
            pa_operation_unref(o);

            break;
        }

        //the pulseaudio interrupt.
        case PA_CONTEXT_FAILED: {
            DEV_LOG_ERR("reconnect the PulseAudio...");
            w->removeAll();
            w->mInited = false;
            w->minnerState = false;
            w->innerCardName.clear();
            if (w->context) {
                pa_context_unref(w->context);
                w->context = NULL;
            }
            w->paConnect(w);
            break;
        }

        case PA_CONTEXT_TERMINATED:
        default:
            w->removeAll();
            w->mInited = false;
            w->minnerState = false;
            w->innerCardName.clear();
            DEV_LOG_INFO("exit the audio module.");
            return;
    }
}

bool AudioCore::paConnect(void *userdata) {
    AudioCore *w = static_cast<AudioCore*>(userdata);

    if (w->context) {
        return false;
    }

    pa_proplist *proplist = pa_proplist_new();
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, "ruijie sound control");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "ruijie.com.snd.ctl");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ICON_NAME, "audio-card");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_VERSION, "V1.0");

    w->context = pa_context_new_with_proplist(w->api, NULL, proplist);
    pa_proplist_free(proplist);
    if (!w->context) {
        DEV_LOG_ERR("pa_context_new fail");
        return false;
    }

    pa_context_set_state_callback(w->context, context_state_callback, w);
    if (pa_context_connect(w->context, NULL, PA_CONTEXT_NOFAIL, NULL) < 0) {
        if(!w->retry) {
            DEV_LOG_ERR("Connection to PulseAudio failed. Automatic retry in 5s");
            sleep(5);
            w->paConnect(w);
            w->retry--;
        } else {
            DEV_LOG_ERR("Unable connect to PulseAudio exit!");
            pa_context_unref(w->context);
            w->context = NULL;
            return false;
        }
    }
    return true;
}

void* AudioCore::main_loop(void *arg) {
    AudioCore *w = static_cast<AudioCore*>(arg);
    pthread_detach(pthread_self());

    //load the snd module
    if (system("modprobe snd_hda_intel") < 0) {
        DEV_LOG_ERR("load snd_hda_intel err:%s", strerror(errno));
        goto out;
    }

    //start the pulseaudio service
    if (system("pulse-daemon &") < 0) {
        DEV_LOG_ERR("start pulseaudio err:%s", strerror(errno));
        goto out;
    }
    usleep(5000);    //wait the service stable.

    //connect
    w->m = pa_mainloop_new();
    if (!(w->m)) {
        DEV_LOG_ERR("pa_mainloop_new fail:%s", strerror(errno));
        goto out;
    }

    w->api = pa_mainloop_get_api(w->m);
    if(!(w->api)) {
        DEV_LOG_ERR("pa_mainloop_get_api fail:%s", strerror(errno));
        goto out;
    }

    if (w->paConnect(w) == false) {
        DEV_LOG_ERR("connect_to_pulse fail:%s", strerror(errno));
        goto out;
    }

    if (pa_mainloop_run(w->m, &(w->retval)) < 0) {
        DEV_LOG_ERR("pa_mainloop_run fail:%s", strerror(errno));
        goto out;
    }

    DEV_LOG_INFO("exit the mainloop.");
out:
    if (w->context) {
        pa_context_unref(w->context);
        w->context = NULL;
    }
    if (w->m) {
        pa_mainloop_free(w->m);
        w->m = NULL;
    }

    return NULL;
}

bool AudioCore::paStart(void *userdata) {
    if (userdata == NULL) {
        DEV_LOG_ERR("please register the event callback");
        return false;
    }

    pthread_t pid_t;

    mAm = static_cast<rcdev::AudioManager *>(userdata);
    if (pthread_create(&pid_t, NULL, main_loop, this) != 0) {
        DEV_LOG_ERR("pthread_create err:%s", strerror(errno));
        return false;
    }

    return true;
}

bool AudioCore::paStop() {
    bool ret = true;

    //disconnect
    if (context) {
        pa_context_disconnect(context);
        pa_mainloop_quit(m, retval);
    }

    //stop the pulseaudio service
    if (system("pkill pulse-daemon") < 0) {
        DEV_LOG_ERR("stop pulseaudio err:%s", strerror(errno));
        ret = false;
    }
    if (system("pkill pulseaudio") < 0) {
        DEV_LOG_ERR("stop pulseaudio err:%s", strerror(errno));
        ret = false;
    }
    usleep(5000);    //wait the service complete quit.

    //unload the snd module
    if (system("modprobe -r snd_hda_intel") < 0) {
        DEV_LOG_ERR("unload snd_hda_intel err:%s", strerror(errno));
        ret = false;
    }

    return ret;
}

void AudioCore::updateSink(const pa_sink_info &info) {
    Sink *w;
    DEV_LOG_INFO(" name:%s index:%d", info.name, info.index);

    if (sinks.count(info.index)) {
        w = sinks[info.index];
    } else {
        sinks[info.index] = w = new Sink();

        w->index = info.index;
        w->name = info.name;
        w->description = info.description;
        // hook the inner CardIdx
        if (!minnerState) {
            if (strstr(info.name, "pci-0000_00_1f.3") != NULL) {
                innerCardIdx = info.card;
                innerCardName = "IDV inner alc662";
                minnerState = true;
                mAm->doEvent(pulseMsg);        //notify the connected event
            }
        }
    }

    Sink::autoDefault(this);

    return;
}

void AudioCore::updateSource(const pa_source_info &info) {
    Source *w;
    DEV_LOG_INFO(" name:%s index:%d", info.name, info.index);

    if (sources.count(info.index)) {
        w = sources[info.index];
    } else {
        sources[info.index] = w = new Source();

        w->index = info.index;
        w->name = info.name;
        w->description = info.description;
    }

    Source::autoDefault(this);

    return;
}

void AudioCore::updateSinkInput(const pa_sink_input_info &info) {
    SinkInput *w;
    DEV_LOG_INFO(" name:%s index:%d", info.name, info.index);

    if (sinkInputs.count(info.index)) {
        w = sinkInputs[info.index];
    } else {
        sinkInputs[info.index] = w = new SinkInput();
        w->index = info.index;
        w->name = info.name;
    }
    w->moveSinkInput(defaultSinkName.c_str());

    return;
}

void AudioCore::updateSourceOutput(const pa_source_output_info &info) {
    SourceOutput *w;
    DEV_LOG_INFO(" name:%s index:%d", info.name, info.index);

    if (sourceOutputs.count(info.index)) {
        w = sourceOutputs[info.index];
    } else {
        sourceOutputs[info.index] = w = new SourceOutput();

        w->index = info.index;
        w->name = info.name;
    }
    w->moveSourceOutput(defaultSourceName.c_str());

    return;
}

void AudioCore::updateServer(const pa_server_info &info) {

    defaultSourceName = info.default_source_name ? info.default_source_name : "";
    defaultSinkName = info.default_sink_name ? info.default_sink_name : "";

    DEV_LOG_INFO(" defSource:%s defSink:%s", defaultSourceName.c_str(), defaultSinkName.c_str());
}

void AudioCore::removeSink(uint32_t index) {
    DEV_LOG_INFO(" index:%d", index);
    if (!sinks.count(index)) {
        return;
    }

    delete sinks[index];
    sinks.erase(index);
}

void AudioCore::removeSource(uint32_t index) {
    DEV_LOG_INFO(" index:%d", index);
    if (!sources.count(index)) {
        return;
    }

    delete sources[index];
    sources.erase(index);
}

void AudioCore::removeSinkInput(uint32_t index) {
    DEV_LOG_INFO(" index:%d", index);
    if (!sinkInputs.count(index)) {
        return;
    }

    delete sinkInputs[index];
    sinkInputs.erase(index);
}

void AudioCore::removeSourceOutput(uint32_t index) {
    DEV_LOG_INFO(" index:%d", index);
    if (!sourceOutputs.count(index)) {
        return;
    }

    delete sourceOutputs[index];
    sourceOutputs.erase(index);
}

void AudioCore::removeAll() {
    DEV_LOG_INFO("remove all snd device");
    for (std::map<uint32_t, SinkInput*>::iterator it = sinkInputs.begin(); it != sinkInputs.end(); ++it) {
        removeSinkInput(it->first);
    }

    for (std::map<uint32_t, SourceOutput*>::iterator it = sourceOutputs.begin(); it != sourceOutputs.end(); ++it) {
        removeSourceOutput(it->first);
    }

    for (std::map<uint32_t, Sink*>::iterator it = sinks.begin(); it != sinks.end(); ++it) {
        removeSink(it->first);
    }

    for (std::map<uint32_t, Source*>::iterator it = sources.begin(); it != sources.end(); ++it) {
        removeSource(it->first);
    }
}

void AudioCore::printAll() {
    DEV_LOG_INFO("print all snd device");
    for (std::map<uint32_t, SinkInput*>::iterator it = sinkInputs.begin(); it != sinkInputs.end(); ++it) {
        DEV_LOG_INFO("sinkInput:%s (%d)", it->second->name.c_str(), it->second->index);
    }

    for (std::map<uint32_t, SourceOutput*>::iterator it = sourceOutputs.begin(); it != sourceOutputs.end(); ++it) {
        DEV_LOG_INFO("sourceOutput:%s (%d)", it->second->name.c_str(), it->second->index);
    }

    for (std::map<uint32_t, Sink*>::iterator it = sinks.begin(); it != sinks.end(); ++it) {
        DEV_LOG_INFO("sink:%s (%d)", it->second->name.c_str(), it->second->index);
    }

    for (std::map<uint32_t, Source*>::iterator it = sources.begin(); it != sources.end(); ++it) {
        DEV_LOG_INFO("source:%s (%d)", it->second->name.c_str(), it->second->index);
    }
}

int AudioCore::setSinkVolume(pa_volume_t vol) {

    if (sinks.count(defaultSinkIdx)) {
        Sink *sink = sinks[defaultSinkIdx];
        sink->updateVolume(vol);
    }

    return 0;
}

int AudioCore::setSourceVolume(pa_volume_t vol) {

    if (sources.count(defaultSourceIdx)) {
        Source *source = sources[defaultSourceIdx];
        source->updateVolume(vol);
    }

    return 0;
}

int AudioCore::changeProfile(const char *profileName) {
    if (profileName == NULL) {
        DEV_LOG_ERR("the profile is invalid");
        return -1;
    }

    if (mInited == false) {
        DEV_LOG_ERR("the audio module init fail");
        return -1;
    }

    if (innerCardName.empty()) {
        DEV_LOG_ERR("not found the inner card");
        return -1;
    }
    char cmd[1024] = {0};
    vector<string> cmdRes;

    if (strncmp(profileName, RJ_HDMI_AUDIO, strlen(RJ_HDMI_AUDIO)) == 0) {
        bHDMI = true;

        snprintf(cmd, sizeof(cmd), "pactl set-card-profile %d %s 2>&1", innerCardIdx, HDMI_PROFILE);
        if (rcdev::dev_exec(cmd, cmdRes) < 0) {
            DEV_LOG_ERR("cmd:%s exec fail", cmd);
            return -1;
        }

        if (!cmdRes.empty()) {
            DEV_LOG_ERR("cmd:%s change fail", cmd);
            return -1;
        }
    } else if (strncmp(profileName, RJ_ANALOG_AUDIO, strlen(RJ_ANALOG_AUDIO)) == 0) {
        bHDMI = false;

        snprintf(cmd, sizeof(cmd), "pactl set-card-profile %d %s 2>&1", innerCardIdx, ANA_PROFILE);
        if (rcdev::dev_exec(cmd, cmdRes) < 0) {
            DEV_LOG_ERR("cmd:%s exec fail", cmd);
            return -1;
        }

        if (!cmdRes.empty()) {
            DEV_LOG_ERR("cmd:%s change fail", cmd);
            return -1;
        }
    } else {
        DEV_LOG_ERR("nonsupport this profile:%s ", profileName);
        return -1;
    }

    DEV_LOG_INFO("profileName:%s innerCard:%s index:%d", profileName, innerCardName.c_str(), innerCardIdx);

    return 0;
}

bool AudioCore::isHdmiConnected() {
    int ret = 0;
    vector<string> cmdRes;

    ret = rcdev::dev_exec("cat `find /sys/class/drm/ | grep HDMI | sed 's/$/\\/status/g'` | grep \"^connected\"",
                          cmdRes);
    if (ret < 0) {
        DEV_LOG_ERR("dev_exec /sys/class/drm/ fail");
        return false;
    }

    if (cmdRes.empty()) {
        DEV_LOG_INFO("HDMI device disconnected.");
        return false;
    } else {
        DEV_LOG_INFO("HDMI device connected.");
        return true;
    }
}





