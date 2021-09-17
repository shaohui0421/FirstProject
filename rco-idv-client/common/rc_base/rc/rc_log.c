/*
 * rc_log.c
 *
 *  Created on: Feb 4, 2017
 *      Author: zhf
 */

#include "rc_log.h"
#include "rc_public.h"
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "iniparser/iniparser.h"


#define LOG_PATH_NAME_MAX_LEN		0x400

#define LOGFILE_PATH_DEF		"/var/log/rc_log.log"
#define LOGFILE_SIZE_MIN		(8*1024)
#define LOGFILE_SIZE_DEF		(100*1024*1024)
#define LOGFILE_BAK_MAX_COUNT	(7)
#define LOGFILE_LEVEL_DEF		USER_NOTICE
#define LOGFILE_INIPATH_DEF		"/etc/logconf.ini"

#define LOGCONF_INI_PATH		"logconf:path"
#define LOGCONF_INI_SIZE		"logconf:size"
#define LOGCONF_INI_PRIO		"logconf:prio"

static int log_inited = 0;
static char *log_path_name;
static int log_size;
static int log_level;
static int bak_log_lock = 0;

#define LOG_INITED_FLAG		1
#define LOG_DBG(format, args...)	printf("%s:%s:%d : " format, __FUNCTION__, __FILE__, __LINE__, ##args)

int log_ini_parser(char *logfilepath, int *logfilesize, int *prio, const char *inifile){
	dictionary *d;
	int creatini=0;

	if((NULL == inifile) || (0 == strlen(inifile) || (LOG_PATH_NAME_MAX_LEN < strlen(inifile)))){
		inifile = LOGFILE_INIPATH_DEF;
		LOG_DBG("inifile path invalid, set to default val:%s\n\r", inifile);
	}

	if (NULL == (d = iniparser_load(inifile))){
		d =dictionary_new(0);
	    if (!d) {
	    	LOG_DBG("can't open and create log config file:%s\n", inifile);
	        return -1;
	    };
    	LOG_DBG("can't open and will create log config file:%s\n", inifile);
	    creatini = 1;
	}

	if (0 == iniparser_find_entry(d, LOGCONF_INI_PATH)){
		if((NULL == logfilepath) || (0 == strlen(logfilepath) || (LOG_PATH_NAME_MAX_LEN < strlen(logfilepath)))){
			logfilepath[0] = 0;
			strncat(logfilepath, LOGFILE_PATH_DEF, LOG_PATH_NAME_MAX_LEN);
			LOG_DBG("logfilepath invalid, set to default:%s\n", logfilepath);
		}
		LOG_DBG("create %s:%s\n", LOGCONF_INI_PATH, logfilepath);
		iniparser_set(d, LOGCONF_INI_PATH, logfilepath);
		creatini = 1;
	}else {
		const char *tmp;
		tmp = iniparser_getstring(d, LOGCONF_INI_PATH, LOGFILE_PATH_DEF);
		if((NULL == tmp) || (0 == strlen(tmp) || (LOG_PATH_NAME_MAX_LEN < strlen(tmp)))){
			logfilepath[0] = 0;
			strncat(logfilepath, LOGFILE_PATH_DEF, LOG_PATH_NAME_MAX_LEN);
			LOG_DBG("iniconf invalid path val, set to default:%s\n", logfilepath);
			iniparser_set(d, LOGCONF_INI_PATH, logfilepath);
			LOG_DBG("create %s:%s\n", LOGCONF_INI_PATH, logfilepath);
			creatini = 1;
		}else {
			logfilepath[0] = 0;
			strncat(logfilepath, tmp, LOG_PATH_NAME_MAX_LEN);
			LOG_DBG("get %s:%s\n", LOGCONF_INI_PATH, logfilepath);
		}
	}

	if (0 == iniparser_find_entry(d, LOGCONF_INI_SIZE)){
		char tmp[0x20];
		if (LOGFILE_SIZE_MIN > *logfilesize){
			LOG_DBG("logfilesize:%d too small, set to default size:%d\n", *logfilesize, LOGFILE_SIZE_DEF);
			*logfilesize = LOGFILE_SIZE_DEF;
		}
		LOG_DBG("create %s:%d\n", LOGCONF_INI_SIZE, *logfilesize);
		snprintf(tmp, sizeof(tmp), "%d", *logfilesize);
		iniparser_set(d, LOGCONF_INI_SIZE, tmp);
		creatini = 1;
	}else {
		*logfilesize = iniparser_getint(d, LOGCONF_INI_SIZE, LOGFILE_SIZE_DEF);
		if (LOGFILE_SIZE_MIN > *logfilesize){
			char tmp[0x20];
			*logfilesize = LOGFILE_SIZE_DEF;
			LOG_DBG("iniconf invalid logfilesize too small, set to default size:%d\n", *logfilesize);
			snprintf(tmp, sizeof(tmp), "%d", *logfilesize);
			iniparser_set(d, LOGCONF_INI_SIZE, tmp);
			LOG_DBG("create %s:%d\n", LOGCONF_INI_SIZE, *logfilesize);
			creatini = 1;
		}else {
			LOG_DBG("get %s:%d\n", LOGCONF_INI_SIZE, *logfilesize);
		}
	}

	if (0 == iniparser_find_entry(d, LOGCONF_INI_PRIO)){
		char tmp[0x20];
		if ((*prio < 0) || (*prio > USER_MAX)){
			LOG_DBG("prio:%d invalid, set to default prio:%d\n", *prio, LOGFILE_LEVEL_DEF);
			*prio = LOGFILE_LEVEL_DEF;
		}
		LOG_DBG("create %s:%d\n", LOGCONF_INI_PRIO, *prio);
		snprintf(tmp, sizeof(tmp), "%d", *prio);
		iniparser_set(d, LOGCONF_INI_PRIO, tmp);
		creatini = 1;
	}else {
		*prio = iniparser_getint(d, LOGCONF_INI_PRIO, LOGFILE_LEVEL_DEF);
		if ((*prio < 0) || (*prio > USER_MAX)){
			char tmp[0x20];
			*prio = LOGFILE_LEVEL_DEF;
			LOG_DBG("iniconf prio val invalid, set to default prio:%d\n", *prio);
			snprintf(tmp, sizeof(tmp), "%d", *prio);
			iniparser_set(d, LOGCONF_INI_PRIO, tmp);
			LOG_DBG("create %s:%d\n", LOGCONF_INI_PRIO, *prio);
			creatini = 1;
		}else {
			LOG_DBG("get %s:%d\n", LOGCONF_INI_PRIO, *prio);
		}
	}

	if (creatini && (0 != iniparser_dump_ini(d, inifile))){
		LOG_DBG("save log config file %s error\n", inifile);
	}else if (creatini){
		LOG_DBG("save log config file %s ok\n", inifile);
	}
	iniparser_freedict(d);
	return 0;
}

int init_log(const char *logfilepath_in, int logfilesize, int prio, const char *inifile){
	FILE *fd;
	char *logfilepath;

	logfilepath = malloc(LOG_PATH_NAME_MAX_LEN + 1);
	if (NULL == logfilepath){
		LOG_DBG("log_path_name malloc error\n\r");
		return -1;
	}
	logfilepath[0] = 0;
	if ((NULL == logfilepath_in) || (0 == strlen(logfilepath_in) || (LOG_PATH_NAME_MAX_LEN < strlen(logfilepath_in)))){
		LOG_DBG("log file name invalid\n");
	}else {
		strncat(logfilepath, logfilepath_in, LOG_PATH_NAME_MAX_LEN);
	}

//	log_ini_parser(logfilepath, &logfilesize, &prio, inifile);

	if ((NULL == logfilepath) || (0 == strlen(logfilepath) || (LOG_PATH_NAME_MAX_LEN < strlen(logfilepath)))){
		LOG_DBG("logfilepath invalid\n\r");
		return -1;
	}
    fd = fopen(logfilepath, "a+");
    if (NULL == fd){
    	LOG_DBG("failed to open log file, errno:%d %s\n\r", errno, strerror(errno));
    	return -1;
    }
    fclose(fd);
	if (LOGFILE_SIZE_MIN > logfilesize){
		LOG_DBG("logfilesize too small\n\r");
		return -1;
	}
	if ((prio < 0) || (prio > USER_MAX)){
		LOG_DBG("prio invalid\n\r");
		return -1;
	}
	log_path_name = malloc(strlen(logfilepath) + 1);
	if (NULL == log_path_name){
		LOG_DBG("log_path_name malloc error\n\r");
		return -1;
	}
	strcpy(log_path_name, logfilepath);

	log_size = logfilesize;
	log_level = prio;
	log_inited = LOG_INITED_FLAG;
	free(logfilepath);
	return 0;
}

int get_log_prio(void){
	return log_level;
}

/**
 * Function: set_log_prio
 * Description: set log debug level
 * Input:	prio	new debug level
 * Return:	last log debug level, you can save, and restore it later
 */
int set_log_prio(int prio){
	int last_level = log_level;
	if ((prio < 0) || (prio > USER_MAX)){
		LOG_DBG("prio invalid\n\r");
		return -1;
	}
	log_level = prio;
	return last_level;
}

int get_log_size(void){
	return log_size;
}

/**
 * Function: set_log_size
 * Description: set log file size
 * Input:	prio	new file size
 * Return:	last log file size, you can save, and restore it later
 */
int set_log_size(int size){
	int last_size = log_size;
	if (LOGFILE_SIZE_MIN > size){
		LOG_DBG("size too small\n\r");
		return -1;
	}
	log_size = size;
	return last_size;
}

static int get_file_exist(const char *filename)
{
    struct stat file_stat;
    int ret = 0;
    
    errno = 0;
    ret = stat(filename, &file_stat);
    if(ret == 0)
    {
        return 1;
    }
    else if(errno == ENOENT)
    {
        return 0;
    }
    else
    {
        LOG_DBG("stat return != 0, but errno != ENOENT, errno = %d, strerror:%s", errno, strerror(errno));
        return 0;
    }
}

static void bak_log()
{
    char cmd[LOG_PATH_NAME_MAX_LEN*2 + 30];
    char tar_name[LOG_PATH_NAME_MAX_LEN + 10];
    char log_path_name_bk[LOG_PATH_NAME_MAX_LEN + 10];
    int i;

    sprintf(log_path_name_bk, "%s_bak", log_path_name);

    //mv bak file
    for (i = LOGFILE_BAK_MAX_COUNT - 1; i > 0; i--)
    {
        sprintf(tar_name, "%s.%d.gz", log_path_name, i);
        if (get_file_exist(tar_name)) {
            sprintf(cmd, "mv %s.%d.gz %s.%d.gz", log_path_name, i, log_path_name, i+1);
            LOG_DBG("run \"%s\"", cmd);
            if (0 != rc_system(cmd)){
                LOG_DBG("run \"%s\" error", cmd);
                return;
            }
        }
    }

    //gzip log
    if (get_file_exist(log_path_name_bk)) {
        sprintf(cmd, "gzip -c %s > %s.1.gz", log_path_name_bk, log_path_name);
        LOG_DBG("run \"%s\"", cmd);
        if (0 != rc_system(cmd)){
            LOG_DBG("run \"%s\" error", cmd);
        }
    }

    //rename bak
    rename(log_path_name, log_path_name_bk);
}

void write_log(int prio, const char* filename, const char* func, int line, const char* format, ...)
{
    FILE *file_fd = NULL;
    char *log_prio[USER_MAX] = { "EMERG", "ALERT", "CRIT", "ERR", "WARNING", "NOTICE", "INFO", "DEBUG", };
    time_t now;
    struct tm *pnowtime;
    char tmpbuf[128] = {0, };
    va_list args;
    long file_size = 0;

    if ((LOG_INITED_FLAG != log_inited) || (prio > log_level))
    {
        return;
    }

    file_fd = fopen(log_path_name, "a+");

    if (NULL != file_fd)
    {
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

    if ((!bak_log_lock) && (file_size > log_size))
    {
        bak_log_lock = 1;
        bak_log();
        bak_log_lock = 0;
    }
}

