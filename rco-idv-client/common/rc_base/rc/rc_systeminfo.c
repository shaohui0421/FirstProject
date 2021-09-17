/*
 * rc_systeminfo.c
 *
 *  Created on: Feb 7, 2017
 *      Author: zhf
 */

#include "rc_systeminfo.h"

#define GETSYSTEMINFO_SERIALNUM	"serialnum="
#define GETSYSTEMINFO_PRODUCTID	"productid="
#define GETSYSTEMINFO_PRODUCTNAME	"productname="
#define GETSYSTEMINFO_HDVERSION	"hdversion="
#define GETSYSTEMINFO_OSVERSION	"osversion="
#define GETSYSTEMINFO_CPU		"cpu="
#define GETSYSTEMINFO_MEMORY	"memory="
#define GETSYSTEMINFO_STORAGE	"storage="
#define GETSYSTEMINFO_BIOSVERSION	"biosversion="

/**
 * Function: rc_getsysteminfo
 * Description: read system information
 * Return:	-1 read failed
 * 			 0 read ok. maybe some field cann't read as permission, and this field will return null string
 */
int rc_getsysteminfo(struct rc_systeminfo_t *system_info){
	char cmd[0x200], data[0x200], *pt;
	int n, datalen = sizeof(data);

	if (NULL == system_info) {
		LOG_ERR("invalid system_info pt");
		return -1;
	}

	n = sizeof(cmd);
	if (0 >= (n -= snprintf(&cmd[sizeof(cmd) - n], n, "bash /etc/rc_script/getsysteminfo.sh "))){
		LOG_ERR("command too long");
		return -1;
	}

	LOG_DEBUG("run \"%s\"", cmd);
	if (0 != rc_system_rw(cmd, (unsigned char *)data, &datalen, "r")){
		LOG_ERR("run \"%s\" error", cmd);
		return -1;
	}

	if (datalen >= sizeof(data)) {
		datalen = sizeof(data) -1;
	}
	data[datalen] = '\0';

	LOG_DEBUG("get info:%s\n", data);

#define GETSYSTEMINFO_STRXXX(tag, to)									\
if (NULL != (pt = strstr(data, tag))){									\
	pt = pt + strlen(tag);												\
	n = rc_strncp_line(system_info->to, pt, sizeof(system_info->to));	\
	if (n < sizeof(system_info->to)) system_info->to[n] = '\0';			\
	else system_info->to[sizeof(system_info->to)-1] = '\0';				\
}else {																	\
	system_info->to[0] = '\0';												\
}

	GETSYSTEMINFO_STRXXX(GETSYSTEMINFO_SERIALNUM, serial_number);
	GETSYSTEMINFO_STRXXX(GETSYSTEMINFO_PRODUCTID, product_id);
	GETSYSTEMINFO_STRXXX(GETSYSTEMINFO_PRODUCTNAME, product_name);
	GETSYSTEMINFO_STRXXX(GETSYSTEMINFO_HDVERSION, hardware_version);
	GETSYSTEMINFO_STRXXX(GETSYSTEMINFO_OSVERSION, os_version);
	GETSYSTEMINFO_STRXXX(GETSYSTEMINFO_CPU, cpu);
	GETSYSTEMINFO_STRXXX(GETSYSTEMINFO_MEMORY, memory);
	GETSYSTEMINFO_STRXXX(GETSYSTEMINFO_STORAGE, storage);
	GETSYSTEMINFO_STRXXX(GETSYSTEMINFO_BIOSVERSION, bios_version);
#undef GETSYSTEMINFO_STRXXX

	return 0;
}

