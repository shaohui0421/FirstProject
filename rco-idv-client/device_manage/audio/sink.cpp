

#include "sink.h"
#include "sinkinput.h"

Sink::Sink() {

}

Sink::~Sink() {

}

void Sink::updateVolume(pa_volume_t v) {
    updateChannelVolume(0, v, true);
    pa_operation *o;

    if (!(o = pa_context_set_sink_volume_by_index(AudioCore::getInstance()->context, index, &volume, NULL, NULL))) {
        DEV_LOG_ERR("pa_context_set_sink_volume_by_index() failed");
        return;
    }
    pa_operation_unref(o);
}

void Sink::autoDefault(AudioCore *ac) {
    std::string hdmi_name, ana_name, echo_cancel, def_name;

    for (std::map<uint32_t, Sink*>::iterator i = ac->sinks.begin(); i != ac->sinks.end(); ++i) {
        Sink *w = i->second;
        if (!w) {
            continue;
        }

        if (strstr(w->name.c_str(), "USB") != NULL || strstr(w->name.c_str(), "usb") != NULL) {
            def_name = w->name;
            break;
        } else if (strstr(w->name.c_str(), "HDMI") != NULL || strstr(w->name.c_str(), "hdmi") != NULL) {
            if (!ac->bHDMI) {
                continue;
            }

            if (hdmi_name.empty()) {
                hdmi_name = w->name;
            }
            continue;
        } else if (strstr(w->name.c_str(), "analog") != NULL || strstr(w->name.c_str(), "Analog") != NULL) {
            if (ac->bHDMI) {
                continue;
            }

            if (ana_name.empty()) {
                ana_name = w->name;
            }
            continue;
        // virturl Echo/Noise-Cancellation device
        } else if (strstr(w->name.c_str(), "echo_cancel") != NULL) {
            if (ac->bHDMI) {
                continue;
            }

            if (echo_cancel.empty()) {
                echo_cancel = w->name;
            }
            continue;
        } else {
            DEV_LOG_NOTICE("ignore this sink:%s", w->name.c_str());
            continue;
        }
    }

    // the echo_cancel priority to analog
    if (!echo_cancel.empty()) {
        ana_name = echo_cancel;
    }

    // ensure the hdmi status
    if (ac->bHDMI && hdmi_name.empty()) {
        DEV_LOG_INFO("self-checking the hdmi");
        ac->changeProfile("hdmi");
    }

    if (def_name.empty()) {
        def_name = hdmi_name.empty() ? ana_name : hdmi_name;
    }

    // ensure the default sink is not empty.
    if (def_name.empty()) {
        DEV_LOG_WARNING("the default sink is empty");
        return;
    }

    DEV_LOG_INFO("[Sink]:%s", def_name.c_str());
    if (def_name == ac->defaultSinkName) {
        DEV_LOG_INFO("the same as defaultSinkName");
        return;
    }

    updateDefault(def_name.c_str());
    ac->defaultSinkName = def_name;

    //move sinkInput to default
    for (std::map<uint32_t, SinkInput*>::iterator it = ac->sinkInputs.begin(); it != ac->sinkInputs.end(); ++it)
        it->second->moveSinkInput(def_name.c_str());

}

void Sink::updateDefault(const char *name) {
    pa_operation *o;

    if (!(o = pa_context_set_default_sink(AudioCore::getInstance()->context, name, NULL, NULL))) {
        DEV_LOG_ERR("pa_context_set_default_sink() failed");
        return;
    }
    pa_operation_unref(o);
}


