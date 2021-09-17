#ifndef _DEV_COMMON_H
#define _DEV_COMMON_H

#include <string>
#include <vector>
#include <pthread.h>
#include "dev_log.h"
#include "dev_thread.h"


using std::string;
using std::vector;

namespace rcdev
{

#define DEV_LOG_FILEPATH              "/var/log/rcdev.log"
#define DEV_LOG_FILE_MAXSIZE          (16 << 20)
#define DEV_LOG_DEFAULT_LOGLEVEL      (DEV_LOG_LEVEL_DEBUG)

#define HPD_LOG_FILEPATH              "/var/log/hotplug_service.log"
#define HPD_LOG_FILE_MAXSIZE          (8 << 20)
#define HPD_LOG_DEFAULT_LOGLEVEL      (DEV_LOG_LEVEL_DEBUG)


#define HPD_BUFFER_SIZE 128
#define HOTPLUG_SERVICE_NAME "HotplugService"

#define DEV_OK       0
#define DEV_FAILED  -1


void init_dev_log();
void init_hotplug_log();
void init_hotplug_client_log(const char *logfilepath_in);
void set_thread_name(const char *name);
int dev_exec(const char *cmd, vector<string> &resvec);
int dev_exec(const char *cmd);
int dev_exec(const char *cmd, string &resstr);
int dev_open_filelock(const char *lckname);
int dev_close_filelock(int fd);
int dev_filelock_ex(int fd);
int dev_filelock_unlock(int fd);

} //namespace

#endif //_DEV_COMMON_H

