/*
 * rc_log.h
 *
 *  Created on: Feb 4, 2017
 *      Author: zhf
 */

#ifndef _RC_LOG_H_
#define _RC_LOG_H_

#include <string.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

int init_log(const char *logfilepath_in, int logfilesize, int prio, const char *inifile);
int get_log_prio(void);
int set_log_prio(int prio);
int get_log_size(void);
int set_log_size(int size);
void write_log(int prio, const char* filename, const char* func, int line, const char* format, ...);

enum LOG_LEVEL{
	USER_EMERG = 0,
	USER_ALERT,
	USER_CRIT,
    USER_ERR,
    USER_WARNING,
    USER_NOTICE,
    USER_INFO,
    USER_DEBUG,
	USER_MAX,
};

/*
 * LOG_PERROR : log errno whith error message
 */
#define LOG_DEBUG(format, args...) write_log(USER_DEBUG, __FILE__, __FUNCTION__, __LINE__ , format, ##args);
#define LOG_INFO(format, args...) write_log(USER_INFO, __FILE__, __FUNCTION__, __LINE__ , format, ##args);
#define LOG_NOTICE(format, args...) write_log(USER_NOTICE, __FILE__, __FUNCTION__, __LINE__ , format, ##args);
#define LOG_WARNING(format, args...)  write_log(USER_WARNING, __FILE__, __FUNCTION__, __LINE__ , format, ##args);
#define LOG_ERR(format, args...) write_log(USER_ERR, __FILE__, __FUNCTION__, __LINE__ , format, ##args);
#define LOG_PERROR(format, args...) write_log(USER_ERR, __FILE__, __FUNCTION__, __LINE__ , format " errno:%d: %s", ##args, errno, strerror(errno));
#define LOG_CRIT(format, args...) write_log(USER_CRIT, __FILE__,  __FUNCTION__,__LINE__ , format, ##args);
#define LOG_ALERT(format, args...) write_log(USER_ALERT, __FILE__, __FUNCTION__, __LINE__ , format, ##args);
#define LOG_EMERG(format, args...) write_log(USER_EMERG, __FILE__, __FUNCTION__, __LINE__ , format, ##args);

#define ASSERT(x) if (!(x)) {                               		\
    LOG_ERR("%s:%d ASSERT %s failed\n", __FUNCTION__, __LINE__, #x);     		\
    abort();                                            			\
}

#ifdef __cplusplus
}
#endif

#endif
