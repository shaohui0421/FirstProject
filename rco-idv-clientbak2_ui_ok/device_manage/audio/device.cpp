
#include "audiocore.h"
#include "device.h"

Device::Device()
    : index(0) {

}

Device::~Device() {

}

//reserved interface
void Device::updateChannelVolume(int channel, pa_volume_t v, bool isAll) {
    pa_cvolume n = volume;

    if (isAll)
        pa_cvolume_set(&n, n.channels, v);
    else
        n.values[channel] = v;
}


