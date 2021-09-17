
#ifndef sinkinput_h
#define sinkinput_h

#include "audiocore.h"

class SinkInput {
public:
    SinkInput();
    virtual ~SinkInput();
    virtual void moveSinkInput(const char *defName);

    std::string name;
    uint32_t index;
};

#endif
