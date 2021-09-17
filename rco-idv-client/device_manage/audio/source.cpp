
#include "source.h"
#include "sourceoutput.h"

Source::Source() {

}

Source::~Source() {

}

void Source::updateVolume(pa_volume_t v) {
    updateChannelVolume(0, v, true);
    pa_operation* o;

    if (!(o = pa_context_set_source_volume_by_index(AudioCore::getInstance()->context, index, &volume, NULL, NULL))) {
        DEV_LOG_ERR("pa_context_set_source_volume_by_index() failed");
        return;
    }
    pa_operation_unref(o);
}

void Source::autoDefault(AudioCore *ac) {
    std::string hdmi_name, ana_name, echo_cancel, def_name;

    for (std::map<uint32_t, Source*>::iterator i = ac->sources.begin(); i != ac->sources.end(); ++i) {
        Source *w = i->second;
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
            if (ana_name.empty()) {
                ana_name = w->name;
            }
            continue;
        // virturl Echo/Noise-Cancellation device
        } else if (strstr(w->name.c_str(), "echo_cancel") != NULL) {
            if (echo_cancel.empty()) {
                echo_cancel = w->name;
            }
            continue;
        } else {
            DEV_LOG_NOTICE("ignore this source:%s", w->name.c_str());
            continue;
        }
    }

    if (!echo_cancel.empty()) {
        ana_name = echo_cancel;    // the echo_cancel priority to analog
    }

    if (def_name.empty()) {
        def_name = hdmi_name.empty() ? ana_name : hdmi_name;
    }

    // ensure the default source is not empty.
    if (def_name.empty()) {
        DEV_LOG_WARNING("the default source is empty");
        return;
    }

    DEV_LOG_INFO("[Source]:%s", def_name.c_str());
    if (def_name == ac->defaultSourceName) {
        DEV_LOG_INFO("the same as the defaultSourceName");
        return;
    }

    updateDefault(def_name.c_str());
    ac->defaultSourceName = def_name;

    //move sourceOutput to default
    for (std::map<uint32_t, SourceOutput*>::iterator it = ac->sourceOutputs.begin();
         it != ac->sourceOutputs.end(); ++it) {
        it->second->moveSourceOutput(def_name.c_str());
    }

}

void Source::updateDefault(const char *name) {
    pa_operation* o;

    if (!(o = pa_context_set_default_source(AudioCore::getInstance()->context, name, NULL, NULL))) {
        DEV_LOG_ERR("pa_context_set_default_source() failed");
        return;
    }
    pa_operation_unref(o);
}

