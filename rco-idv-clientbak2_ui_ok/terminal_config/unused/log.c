/*
 * Copyright(C) 2013 Ruijie Network. All rights reserved.
 */
/*
 * blog.c

 *
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdarg.h>
#include <pthread.h>


static char sprint_buf[1024];
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void debug(char *fmt, ...)
{
    va_list args;

    pthread_mutex_lock(&log_mutex);
    va_start(args, fmt);
    vsprintf(sprint_buf, fmt, args);
    va_end(args);
	syslog(LOG_USER | LOG_INFO, "%s", sprint_buf);
	pthread_mutex_unlock(&log_mutex);
	return;
}
/*
void log_err(char *fmt, ...)
{
    va_list args;

    pthread_mutex_lock(&log_mutex);
    va_start(args, fmt);
    vsprintf(sprint_buf, fmt, args);
    va_end(args);
	syslog(LOG_USER | LOG_ERR, "%s", sprint_buf);
	pthread_mutex_unlock(&log_mutex);
	return;
}
*/
