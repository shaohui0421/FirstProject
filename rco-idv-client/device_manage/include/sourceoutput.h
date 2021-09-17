
#ifndef sourceoutput_h
#define sourceoutput_h

#include "audiocore.h"

class SourceOutput {
public:
    SourceOutput();
    virtual ~SourceOutput();
    virtual void moveSourceOutput(const char *defName);

    std::string name;
    uint32_t index;
};

#endif
