/*
 * rcc_public.c
 *
 *  Created on: Feb 4, 2017
 *      Author: zhf
 */

#include "rc_public.h"

/**
 * Function: rc_popen
 * Description: same as popen, and auto write log
 */
FILE *rc_popen( const char* command, const char* mode ){
	FILE *fp = NULL;
	if (NULL == command){
		LOG_ERR("command is null");
		return NULL;
	}
	if (NULL == mode){
		LOG_ERR("mode is not set");
		return NULL;
	}
	errno = 0;
	fp = popen(command, mode);
	if (NULL == fp){
		LOG_ERR("popen failed");
		if (0 != errno){
			LOG_PERROR();
		}
		return NULL;
	}
	return fp;
}

/**
 * Function: rc_pclose
 * Description: same as pclose, and auto write log
 */
int rc_pclose( FILE *fp ){
	int ret;
	if (NULL == fp){
		LOG_ERR("invalid ptr");
		return -1;
	}
	errno = 0;
	ret = pclose(fp);
	if (ret < 0){
		LOG_PERROR("pclose failed");
		return ret;
	}
	if (errno) LOG_PERROR("pclose failed???");
	if (WIFSIGNALED(ret)){
		LOG_ERR("popen command abnormal,signal number:%d %s", WTERMSIG(ret), strsignal(WTERMSIG(ret)));
		return -1;
	} else if (WIFSTOPPED(ret)){
		LOG_ERR("popen command stop,signal number:%d %s", WSTOPSIG(ret), strsignal(WSTOPSIG(ret)));
		return -1;
	} else if (WIFEXITED(ret)){
		ret = WEXITSTATUS(ret);
		if (ret) LOG_ERR("popen command exit status:%d", ret);
	}
	return ret;
}

/**
 * Function: rc_system_rw
 * Description: run system cmd with only read or write mode
 * Input: 	@command
 * 			@mode	"r" or "w"
 * In&Out:	@data	data input or output, NULL when none data
 * 			@size	the size of data to read or write
 * Return:	-1 	failed to run cmd
 * 			>=0 cmd return value
 */
int rc_system_rw(const char *command, unsigned char *data, int *size, const char* mode){
	int readsize = 0, n;
	FILE *fd;
	if ((NULL != data) &&(NULL != size)){
		readsize = *size;
		*size = 0;
	}
	fd = rc_popen(command,mode);
	if (NULL != fd) {
		if (*mode == 'r'){
			while ((readsize > 0) && ((n = fread(data, 1, readsize, fd)) > 0)){
				*size += n;
				readsize -= n;
				data += n;
			}
		} else {
			while ((readsize > 0) && ((n = fwrite(data, 1, readsize, fd)) > 0)){
				*size += n;
				readsize -= n;
				data += n;
			}
		}
		return rc_pclose(fd);
	}
	LOG_ERR("rc_popen failed");
	return -1;
}

/**
 * Function: rc_system
 * Description: run system cmd without read or write mode
 * Input: 	@command
 * Return:	-1 	failed to run cmd
 * 			>=0 cmd return value
 */
int rc_system(const char *command){
	FILE *fd;
	fd = rc_popen(command,"w");
	if (NULL != fd) {
		return rc_pclose(fd);
	}
	LOG_ERR("rc_popen failed");
	return -1;
}

/**
 * Function: rc_strncp_line
 * Description: copy one line string that not more than size, don't copy '\n' and '\r'
 * Input:	dst copy to
 * 			src copy from
 * 			size copy max size
 * 	Return: the size copied
 */
int rc_strncp_line(char *dst, const char *src, int size){
	int cpsize = 0;
	while (size-- > 0){
		if ((0 == *src) || (*src < 0x20) || (*src == '\n') || (*src == '\r')) break;
		*dst++ = *src++;
		cpsize++;
	}
	return cpsize;
}
