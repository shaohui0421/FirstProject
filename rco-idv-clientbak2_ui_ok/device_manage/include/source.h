
#ifndef source_h
#define source_h

#include "device.h"

class Source : public Device {
public:
    Source();
    virtual ~Source();

    virtual void updateVolume(pa_volume_t v);
    static void updateDefault(const char *name);
    static void autoDefault(AudioCore *ac);
};

#endif
