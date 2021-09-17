#ifndef _LOG_H
#define _LOG_H

#include <string.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

void log_init(void);
int _init_log(const char *logfilepath_in, int logfilesize, int prio);
void _log(int prio, const char* filename, const char* func, int line, const char* format, ...);

enum LOG_LEVEL
{
    LOG_LEVEL_EMERG = 0,
    LOG_LEVEL_ALERT,
    LOG_LEVEL_CRIT,
    LOG_LEVEL_ERR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_NOTICE,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_MAX,
};

/*
 * LOG_PERROR : log errno whith error message
 */
#define LOG_DEBUG(format,   args...) _log(LOG_LEVEL_DEBUG,   __FILE__, __FUNCTION__, __LINE__ , format, ##args);
#define LOG_INFO(format,    args...) _log(LOG_LEVEL_INFO,    __FILE__, __FUNCTION__, __LINE__ , format, ##args);
#define LOG_NOTICE(format,  args...) _log(LOG_LEVEL_NOTICE,  __FILE__, __FUNCTION__, __LINE__ , format, ##args);
#define LOG_WARNING(format, args...) _log(LOG_LEVEL_WARNING, __FILE__, __FUNCTION__, __LINE__ , format, ##args);
#define LOG_ERR(format,     args...) _log(LOG_LEVEL_ERR,     __FILE__, __FUNCTION__, __LINE__ , format, ##args);
#define LOG_PERROR(format,  args...) _log(LOG_LEVEL_ERR,     __FILE__, __FUNCTION__, __LINE__ , format " errno:%d: %s", ##args, errno, strerror(errno));
#define LOG_CRIT(format,    args...) _log(LOG_LEVEL_CRIT,    __FILE__,  __FUNCTION__,__LINE__ , format, ##args);
#define LOG_ALERT(format,   args...) _log(LOG_LEVEL_ALERT,   __FILE__, __FUNCTION__, __LINE__ , format, ##args);
#define LOG_EMERG(format,   args...) _log(LOG_LEVEL_EMERG,   __FILE__, __FUNCTION__, __LINE__ , format, ##args);

#define ASSERT(x) if (!(x)) {                               		\
    LOG_ERR("%s:%d ASSERT %s failed\n", __FUNCTION__, __LINE__, #x);     		\
    abort();                                            			\
}

#ifdef __cplusplus
}
#endif

#endif //_LOG_H
