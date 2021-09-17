
#ifndef sink_h
#define sink_h

#include "device.h"

class Sink : public Device {
public:
    Sink();
    virtual ~Sink();

    virtual void updateVolume(pa_volume_t v);
    static void updateDefault(const char *name);
    static void autoDefault(AudioCore *ac);
};

#endif
