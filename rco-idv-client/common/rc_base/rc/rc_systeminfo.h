/*
 * rc_systeminfo.h
 *
 *  Created on: Feb 7, 2017
 *      Author: zhf
 */

#ifndef _RC_SYSTEMINFO_H_
#define _RC_SYSTEMINFO_H_

#include "rc_public.h"

#ifdef __cplusplus
extern "C" {
#endif

struct rc_systeminfo_t {
	char serial_number[64]; 	//eg. xxxxxxx
	char product_id[64];		//eg. 80060011
	char product_name[64];		//eg. RG-Rain400W(128)
	char hardware_version[64];	//eg. V1.00
	char bios_version[64];	//eg. RJ622XXX
	char os_version[64];	//eg. RCC_RainOS_V3.3_R0.6
	char cpu[64];			//eg .Inte(R) Core(TM) i3-6100U CPU @ 2.30GHz
	char memory[64];		//eg. 4024232 kB
	char storage[64];		//eg. 128 GB
};

int rc_getsysteminfo(struct rc_systeminfo_t *system_info);

#ifdef __cplusplus
}
#endif

#endif
