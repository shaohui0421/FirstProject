#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <signal.h>

#include "dev_common.h"
#include "hotplug_service.h"

using namespace std;

static bool hotplug_running = true;

static void sig_handler(int signum)
{
    switch (signum) {
    case SIGINT:
        DEV_LOG_INFO("Get signal SIGINT. EXIT PROCESS.");
        hotplug_running = false;
        break;
    default:
        DEV_LOG_WARNING("Unknown signal %d.", signum);
        break;
    }

    return;
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    HotplugService *hs = NULL;

    rcdev::init_hotplug_log();

    signal(SIGINT, sig_handler);

    hs = new HotplugService();
    if (!hs) {
        DEV_LOG_ERR("create hotplug service failed");
        return DEV_FAILED;
    }

    DEV_LOG_INFO("HOTPLUG DAEMON START");

    while(hotplug_running) {
        sleep(1);
    }

    DEV_LOG_INFO("HOTPLUG DAEMON END");

    if (hs) {
        delete hs;
        hs = NULL;
    }

    return DEV_OK;
}
