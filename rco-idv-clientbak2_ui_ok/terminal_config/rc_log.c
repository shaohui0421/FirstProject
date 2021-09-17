#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "rc_log.h"

#define LOG_PATH_NAME_MAX_LEN       0x400
#define LOGFILE_SIZE_MIN            (8*1024)
#define LOG_FILEPATH                "/var/log/terminal_conf.log"
#define LOG_FILE_MAXSIZE            (16 << 20)
#define LOG_DEFAULT_LOGLEVEL        (LOG_LEVEL_DEBUG)

static int log_inited = 0;
static char *log_path_name;
static char *log_path_name_bk;
static int log_size;
static int log_level;

#define LOG_INITED_FLAG		1
#define LOG_DBG(format, args...)	printf("%s:%s:%d : " format, __FUNCTION__, __FILE__, __LINE__, ##args)

/**
 * Function:    init_dev_log
 * Description: init device log
 */
void log_init(void)
{
    _init_log(LOG_FILEPATH, LOG_FILE_MAXSIZE, LOG_DEFAULT_LOGLEVEL);
}

int _init_log(const char *logfilepath_in, int logfilesize, int prio)
{
	FILE *fd;
	char logfilepath[LOG_PATH_NAME_MAX_LEN + 1] = {'\0'};

	if ((NULL == logfilepath_in) || (0 == strlen(logfilepath_in) || (LOG_PATH_NAME_MAX_LEN < strlen(logfilepath_in)))) {
		LOG_DBG("log file name invalid\n");
	}else {
		strncat(logfilepath, logfilepath_in, LOG_PATH_NAME_MAX_LEN);
	}
	if (0 == strlen(logfilepath) || (LOG_PATH_NAME_MAX_LEN < strlen(logfilepath))) {
		LOG_DBG("logfilepath invalid\n\r");
		return -1;
	}
    fd = fopen(logfilepath, "a+");
    if (NULL == fd){
    	LOG_DBG("failed to open log file, errno:%d %s\n\r", errno, strerror(errno));
    	return -1;
    }
    fclose(fd);
	if (LOGFILE_SIZE_MIN > logfilesize) {
		LOG_DBG("logfilesize too small\n\r");
		return -1;
	}
	if ((prio < 0) || (prio > LOG_LEVEL_MAX)) {
		LOG_DBG("prio invalid\n\r");
		return -1;
	}
	log_path_name = malloc(strlen(logfilepath) + 1);
	if (NULL == log_path_name) {
		LOG_DBG("log_path_name malloc error\n\r");
		return -1;
	}
	log_path_name_bk = malloc(strlen(logfilepath) + 20);
	if (NULL == log_path_name_bk) {
		free(log_path_name);
		LOG_DBG("log_path_name_bk malloc error\n\r");
		return -1;
	}
	strcpy(log_path_name, logfilepath);

	strcpy(log_path_name_bk, logfilepath);
	strncat(log_path_name_bk,"_bak",20);
	log_size = logfilesize;
	log_level = prio;
	log_inited = LOG_INITED_FLAG;

	return 0;
}

void _log(int prio, const char* filename, const char* func, int line, const char* format, ...)
{
    FILE *file_fd = NULL;
    time_t now;
    long file_size = 0;

    if ((LOG_INITED_FLAG != log_inited) || (prio > log_level)) {
        return;
    }

    file_fd = fopen(log_path_name, "a+");

    if (NULL != file_fd) {
        va_list args;
        char tmpbuf[128] = {0, };
        struct tm *pnowtime;
        char *log_prio[LOG_LEVEL_MAX] = { "EMERG", "ALERT", "CRIT", "ERR", "WARNING", "NOTICE", "INFO", "DEBUG", };

        time(&now);
        pnowtime = localtime(&now);
        strftime(tmpbuf, sizeof(tmpbuf), "%Y-%m-%d %H:%M:%S", pnowtime);

        fprintf(file_fd, "%s [%s %s %d]<%s>: ", tmpbuf, filename, func, line, log_prio[prio]);
        va_start(args, format);
        vfprintf(file_fd, format, args);
        fprintf(file_fd, "\n");
        va_end(args);
        fflush(file_fd);
        file_size = ftell(file_fd);
        fclose(file_fd);
    } else {
    	LOG_DBG("failed to open log file, errno:%d %s\n\r", errno, strerror(errno));
    }

    if (file_size > log_size)
    {
        rename(log_path_name, log_path_name_bk);
    }
}
