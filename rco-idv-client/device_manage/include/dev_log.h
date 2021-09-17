#ifndef _DEV_LOG_H
#define _DEV_LOG_H

#include <string.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

int _init_dev_log(const char *logfilepath_in, int logfilesize, int prio);
void _dev_log(int prio, const char* filename, const char* func, int line, const char* format, ...);

enum DEV_LOG_LEVEL
{
	DEV_LOG_LEVEL_EMERG = 0,
	DEV_LOG_LEVEL_ALERT,
	DEV_LOG_LEVEL_CRIT,
    DEV_LOG_LEVEL_ERR,
    DEV_LOG_LEVEL_WARNING,
    DEV_LOG_LEVEL_NOTICE,
    DEV_LOG_LEVEL_INFO,
    DEV_LOG_LEVEL_DEBUG,
	DEV_LOG_LEVEL_MAX,
};

/*
 * DEV_LOG_PERROR : log errno whith error message
 */
#define DEV_LOG_DEBUG(format, args...)   _dev_log(DEV_LOG_LEVEL_DEBUG, __FILE__, __FUNCTION__, __LINE__ , format, ##args);
#define DEV_LOG_INFO(format, args...)    _dev_log(DEV_LOG_LEVEL_INFO, __FILE__, __FUNCTION__, __LINE__ , format, ##args);
#define DEV_LOG_NOTICE(format, args...)  _dev_log(DEV_LOG_LEVEL_NOTICE, __FILE__, __FUNCTION__, __LINE__ , format, ##args);
#define DEV_LOG_WARNING(format, args...) _dev_log(DEV_LOG_LEVEL_WARNING, __FILE__, __FUNCTION__, __LINE__ , format, ##args);
#define DEV_LOG_ERR(format, args...)     _dev_log(DEV_LOG_LEVEL_ERR, __FILE__, __FUNCTION__, __LINE__ , format, ##args);
#define DEV_LOG_PERROR(format, args...)  _dev_log(DEV_LOG_LEVEL_ERR, __FILE__, __FUNCTION__, __LINE__ , format " errno:%d: %s", ##args, errno, strerror(errno));
#define DEV_LOG_CRIT(format, args...)    _dev_log(DEV_LOG_LEVEL_CRIT, __FILE__,  __FUNCTION__,__LINE__ , format, ##args);
#define DEV_LOG_ALERT(format, args...)   _dev_log(DEV_LOG_LEVEL_ALERT, __FILE__, __FUNCTION__, __LINE__ , format, ##args);
#define DEV_LOG_EMERG(format, args...)   _dev_log(DEV_LOG_LEVEL_EMERG, __FILE__, __FUNCTION__, __LINE__ , format, ##args);

#define ASSERT(x) if (!(x)) {                               		\
    DEV_LOG_ERR("%s:%d ASSERT %s failed\n", __FUNCTION__, __LINE__, #x);     		\
    abort();                                            			\
}

#ifdef __cplusplus
}
#endif

#endif //_DEV_LOG_H
