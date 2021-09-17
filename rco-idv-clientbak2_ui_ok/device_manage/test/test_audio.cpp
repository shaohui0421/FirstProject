
#include <unistd.h>
#include "dev_common.h"
#include "dev_api.h"
#include "dev_log.h"

using namespace rcdev;

static void audioCallback(DevMsg& msg, void *user)
{
    DEV_LOG_INFO("** recv audio callback **");
    DEV_LOG_INFO("** id : %d", msg.msg_id);
    DEV_LOG_INFO("** msg: %s", msg.msg.c_str());
    DEV_LOG_INFO("** recv audio callback **");
}

static void audioJsonCallback(string& msg, void *user)
{
    DEV_LOG_INFO("** recv audio json callback **");
    DEV_LOG_INFO("** json: %s", msg.c_str());
    DEV_LOG_INFO("** recv audio json callback **");
}

static void doJsonTest(AudioManager* am)
{
    am->doJsonCommand(DEV_HANDLE_REQUEST_SND_DEV_LIST, "{\"handle\": 101}");
    am->doJsonCommand(DEV_HANDLE_SWITCH_SND_DEV, "{\"handle\": 102}");
}

int main(int argc, char* argv[])
{
    int ret;

    init_dev_log();
    AudioManager* am = new AudioManager();
    am->registerCallback(audioCallback, audioJsonCallback, NULL);

    DEV_LOG_INFO("==================== START AUDIO TEST ====================");

    ret = am->startPaService();
    DEV_LOG_INFO("start pulse audio, ret = %d", ret);
    sleep(20);
    DEV_LOG_INFO("== recv json test ==");
    doJsonTest(am);
    DEV_LOG_INFO("== recv json test end ==");
    ret = am->stopPaService();
    DEV_LOG_INFO("==================== STOP AUDIO TEST =====================");
    return 0;
}
