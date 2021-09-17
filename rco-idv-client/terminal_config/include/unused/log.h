/*
 * blog.h
 *
 *  Created on: 2013-8-12
 *      Author: dong
 */

#ifndef __LOG_H__
#define __LOG_H__
#include <syslog.h>

extern void debug(char *fmt, ...);

#define log_info(fmt, argv...) do {                                                        \
                                        syslog(LOG_USER | LOG_INFO, "[%s,%d]"fmt, __func__, __LINE__, ##argv);       \
                                } while(0)

#define log_err(fmt, argv...) do {                                                        \
                                        syslog(LOG_USER | LOG_ERR, "[%s,%d]"fmt, __func__, __LINE__, ##argv);       \
                                } while(0)

#endif /* __LOG_H__ */
