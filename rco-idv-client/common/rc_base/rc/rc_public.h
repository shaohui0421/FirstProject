/*
 * rcc_public.h
 *
 *  Created on: Feb 4, 2017
 *      Author: zhf
 */

#ifndef _RC_PUBLIC_H_
#define _RC_PUBLIC_H_

#include "rc_log.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

FILE *rc_popen(const char* command, const char* mode);
int rc_pclose( FILE *fp );
int rc_system_rw(const char *command, unsigned char *data, int *size, const char* mode);
int rc_system(const char *command);
int rc_strncp_line(char *dst, const char *src, int size);

#ifdef __cplusplus
}
#endif

#endif
