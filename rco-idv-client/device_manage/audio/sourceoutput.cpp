
#include "sourceoutput.h"
#include "audiocore.h"
#include "source.h"

SourceOutput::SourceOutput()
    : index(0) {

}

SourceOutput::~SourceOutput() {

}

void SourceOutput::moveSourceOutput(const char *defName) {
    pa_operation* o;

    if (!(o = pa_context_move_source_output_by_name(AudioCore::getInstance()->context, index, defName, NULL, NULL))) {
        DEV_LOG_ERR("pa_context_move_source_output_by_index() failed");
        return;
    }
    pa_operation_unref(o);
}

