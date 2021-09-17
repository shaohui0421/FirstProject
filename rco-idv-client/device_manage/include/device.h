
#ifndef device_h
#define device_h

#include "audiocore.h"

class AudioCore;

class Device {
public:
    Device();
    virtual ~Device();
    virtual void updateChannelVolume(int channel, pa_volume_t v, bool isAll);

    std::string name;
    std::string description;
    uint32_t index;

protected:
    pa_cvolume volume;
};

#endif
