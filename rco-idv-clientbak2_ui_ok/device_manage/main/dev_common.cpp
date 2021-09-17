#include <sys/prctl.h>
#include <sys/file.h>
#include <unistd.h>
#include "dev_common.h"

namespace rcdev
{


#define EXEC_MAX_CMD_SIZE 1024

/**
 * Function:    init_dev_log
 * Description: init device log
 */
void init_dev_log()
{
    _init_dev_log(DEV_LOG_FILEPATH, DEV_LOG_FILE_MAXSIZE, DEV_LOG_DEFAULT_LOGLEVEL);
}


void init_hotplug_log()
{
    _init_dev_log(HPD_LOG_FILEPATH, HPD_LOG_FILE_MAXSIZE, HPD_LOG_DEFAULT_LOGLEVEL);
}

void init_hotplug_client_log(const char *logfilepath_in)
{
    _init_dev_log(logfilepath_in, HPD_LOG_FILE_MAXSIZE, HPD_LOG_DEFAULT_LOGLEVEL);
}

/**
 * Function:    set_thread_name
 * Description: set thread name
 * Input:       @thread_name
 */
void set_thread_name(const char *name)
{
    int ret;

    if (NULL == name || strlen(name) == 0) {
        DEV_LOG_ERR("set thread name err, thread name is empty");
        return;
    }
    ret = prctl(PR_SET_NAME, name, NULL, NULL, NULL);
    if (ret < 0) {
        DEV_LOG_WARNING("Set thread[%lu] name(%s) failed: %s\n",
            pthread_self(), name, strerror(errno));
    }
}

int dev_exec(const char *cmd, vector<string> &resvec)
{

    FILE *pp = popen(cmd, "r");

    if (!pp) {
        DEV_LOG_ERR("popen failed");
        return -1;
    }

    char tmp[EXEC_MAX_CMD_SIZE];
    resvec.clear();

    while (fgets(tmp, sizeof(tmp), pp) != NULL) {
        if (tmp[strlen(tmp) - 1] == '\n') {
            tmp[strlen(tmp) - 1] = '\0';
        }
        resvec.push_back(tmp);
    }

    pclose(pp);

    return resvec.size();
}

int dev_exec(const char *cmd)
{

    FILE *pp = popen(cmd, "r");

    if (!pp) {
        DEV_LOG_ERR("popen failed");
        return -1;
    }

    pclose(pp);

    return 0;
}

int dev_exec(const char *cmd, string &resstr)
{

    FILE *pp = popen(cmd, "r");

    if (!pp) {
        DEV_LOG_ERR("popen failed");
        return -1;
    }

    int num;
    char tmp[1024];
    resstr.clear();

    /* only output the first string in result */
    if (fgets(tmp, sizeof(tmp), pp) != NULL) {
        if (tmp[strlen(tmp) - 1] == '\n') {
            tmp[strlen(tmp) - 1] = '\0';
        }
        resstr.append(tmp);
        num = 1;
    } else {
        resstr = '\0';
        num = 0;
    }

    pclose(pp);

    return num;
}


int dev_open_filelock(const char *lckname)
{
    char lckpath[EXEC_MAX_CMD_SIZE] = {0};
    int fd;

    snprintf(lckpath, EXEC_MAX_CMD_SIZE, "%s/%s", "/tmp", lckname);

    fd = open(lckpath, O_WRONLY|O_CREAT, S_IRWXU);
    if (fd < 0) {
        DEV_LOG_ERR("open file lock fd failed: %d", fd);
        return -1;
    }

    return fd;
}

int dev_close_filelock(int fd)
{
    if (fd < 0) {
        DEV_LOG_ERR("fd is error: %d", fd);
        return -1;
    }

    close(fd);

    return 0;
}

int dev_filelock_ex(int fd)
{
    int retry = 30000;
    int ret = -1;

    if (fd < 0) {
        DEV_LOG_ERR("fd is error: %d", fd);
    }

    while (retry--) {
        ret = flock(fd, LOCK_EX | LOCK_NB);
        if (ret == 0) {
            return 0;
        } else if (ret == -1) {
            if (errno != 0) {
                ;//DEV_LOG_ERR("errno:%d: %s", errno, strerror(errno));
            }
        }

        usleep(1000);
    }
    return ret;
}

int dev_filelock_unlock(int fd)
{
    int ret = 0;

    if (fd < 0) {
        DEV_LOG_ERR("fd is error: %d", fd);
        return -1;
    }

    ret = flock(fd, LOCK_UN);
    if (ret == -1) {
        if (errno != 0) {
            DEV_LOG_ERR("errno:%d: %s", errno, strerror(errno));
        }
        return -1;
    }

    return 0;
}

} //namespace

