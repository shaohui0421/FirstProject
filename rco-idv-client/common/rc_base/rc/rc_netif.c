/*
 * rc_netif.c
 *
 *  Created on: Feb 7, 2017
 *      Author: zhf
 */
#include <pthread.h>
#include "rc_netif.h"
#include "rc_public.h"
#include <fcntl.h>

/**
 * Function: rc_getnetifnum
 * Description: get net interface number
 * return: -1 get error
 * 			>=0 interface number
 */
int rc_getnetifnum(void) {
	unsigned char data[0x20];
	int size;
	size =  sizeof(data);
	if (0 == rc_system_rw("bash /etc/rc_script/getnetifnum.sh", data, &size, "r")){
		if (size >= sizeof(data)) return -1;
		data[size] = 0;
		return atoi((char *)data);
	}
	LOG_ERR("getnetifnum error");
	return -1;
}

/**
 * Function: rc_getnetifnamelist
 * Description: get net interface name list
 * return: -1 get error
 * 			=0 interface name list
 */
int rc_getnetifnamelist(char *data, int size){
	int rdsize = size;
	if ((NULL == data) || (size < 2)){
		LOG_ERR("invalid param");
		return -1;
	}

	if (0 == rc_system_rw("bash /etc/rc_script/getnetifnamelist.sh", (unsigned char *)data, &rdsize, "r")){
		if (rdsize >= size) rdsize = size - 1;
		data[rdsize] = 0;
		return 0;
	}
	LOG_ERR("getnetifnamelist error");
	return -1;
}

/**
 * Function: rc_getdefaultnetifname
 * Description: get default net interface name
 * return: =-1 get error
 * 		   =0 default interface name
 */
int rc_getdefaultnetifname(char *data, int size){
	int rdsize = size;
	if ((NULL == data) || (size < 2)){
		LOG_ERR("invalid param");
		return -1;
	}

	if (0 == rc_system_rw("bash /etc/rc_script/getdefaultnetifname.sh", (unsigned char *)data, &rdsize, "r")){
		if (rdsize >= size) rdsize = size - 1;
		data[rdsize] = 0;
		return 0;
	}
	LOG_ERR("rc_getdefaultnetifname error");
	return -1;
}

#define GETIFINFO_STATUS	"status="
#define GETIFINFO_IPTYPE	"iptype="
#define GETIFINFO_DNSTYPE	"dnstype="
#define GETIFINFO_IP		"ip="
#define GETIFINFO_NETMASK	"netmask="
#define GETIFINFO_GATEWAY	"gateway="
#define GETIFINFO_MAC		"mac="
#define GETIFINFO_DNS1		"dns1="
#define GETIFINFO_DNS2		"dns2="
#define GETIFINFO_SPEED		"speed="
#define GETIFINFO_LINKSTATUS	"linkstatus="

/**
 * Function: rc_getnetifdevinfo
 * Description: read the net interface(dev->dev = "ethx") information
 * Input:	dev 	dev->dev must set be set
 * Return:	-1 read failed
 * 			 0 read ok. maybe some field cann't read as permission, and this field will return null string or invalid ip address
 */
int rc_getnetifdevinfo(struct rc_netifdev_t *dev){
	char cmd[0x200], data[0x200], *pt;
	int n, datalen = sizeof(data);

	if (NULL == dev){
		LOG_ERR("hostname is null");
		return -1;
	}

	if (strlen(dev->dev) > sizeof(dev->dev)){
		LOG_ERR("netif devname too long");
		return -1;
	}
	n = sizeof(cmd);
	if (0 >= (n -= snprintf(&cmd[sizeof(cmd) - n], n, "bash /etc/rc_script/getnetifdevinfo.sh %s ", dev->dev))){
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
	data[datalen] = 0;

	if (NULL != (pt = strstr(data, GETIFINFO_STATUS))){
			pt = pt + strlen(GETIFINFO_STATUS);
			switch(atoi(pt)){
				case 1:
					dev->status = RC_IFDEV_STATUS_OFF;
					break;
				case 2:
					dev->status = RC_IFDEV_STATUS_ON;
					break;
				default:
					dev->status = RC_IFDEV_STATUS_UNKNOW;
					break;
			}
	}else {
		dev->status = RC_IFDEV_STATUS_UNKNOW;
	}
	if (NULL != (pt = strstr(data, GETIFINFO_IPTYPE))){
			pt = pt + strlen(GETIFINFO_IPTYPE);
			if (0 == strncmp(pt, "dhcp", 4)){
				dev->iptype = RC_IPTYPE_DHCP;
			}else if (0 == strncmp(pt, "static", 6)){
				dev->iptype = RC_IPTYPE_STATIC;
			}else if (0 == strncmp(pt, "loopback", 8)){
				dev->iptype = RC_IPTYPE_LOOPBACK;
			}else {
				dev->iptype = RC_IPTYPE_UNKNOW;
			}
	}else {
		dev->iptype = RC_IPTYPE_UNKNOW;
	}
	if (NULL != (pt = strstr(data, GETIFINFO_DNSTYPE))){
			pt = pt + strlen(GETIFINFO_DNSTYPE);
			if (0 == strncmp(pt, "dhcp", 4)){
				dev->dnstype = RC_DNSTYPE_DHCP;
			}else if (0 == strncmp(pt, "static", 6)){
				dev->dnstype = RC_DNSTYPE_STATIC;
			}else {
				dev->dnstype = RC_DNSTYPE_UNKNOW;
			}
	}else {
		dev->dnstype = RC_IPTYPE_UNKNOW;
	}
	if (NULL != (pt = strstr(data, GETIFINFO_SPEED))){
			pt = pt + strlen(GETIFINFO_SPEED);
			switch(atoi(pt)){
				case 10:
					dev->link_speed = RC_IFDEV_SPEED_10M;
					break;
				case 100:
					dev->link_speed = RC_IFDEV_SPEED_100M;
					break;
				case 1000:
					dev->link_speed = RC_IFDEV_SPEED_1000M;
					break;
				default:
					dev->link_speed = RC_IFDEV_SPEED_UNKNOW;
					break;
			}
	}else {
		dev->link_speed = RC_IFDEV_SPEED_UNKNOW;
	}
	if (NULL != (pt = strstr(data, GETIFINFO_LINKSTATUS))){
			pt = pt + strlen(GETIFINFO_LINKSTATUS);
			if (0 == memcmp(pt, "yes", 3)){
				dev->link_status = RC_IFDEV_LINKSTATUS_ON;
			}else if (0 == memcmp(pt, "no", 2)){
				dev->link_status = RC_IFDEV_LINKSTATUS_OFF;
			}else {
				dev->link_status = RC_IFDEV_LINKSTATUS_UNKNOW;
			}
	}else {
		dev->link_status = RC_IFDEV_LINKSTATUS_UNKNOW;
	}
	if (NULL != (pt = strstr(data, GETIFINFO_IP))){
		pt = pt + strlen(GETIFINFO_IP);
		inet_aton((const char *)pt, &dev->ip);
	}else {
		dev->ip.s_addr = 0;
	}
	if (NULL != (pt = strstr(data, GETIFINFO_NETMASK))){
		pt = pt + strlen(GETIFINFO_NETMASK);
		inet_aton((const char *)pt, &dev->netmask);
	}else {
		dev->netmask.s_addr = 0;
	}
	if (NULL != (pt = strstr(data, GETIFINFO_GATEWAY))){
		pt = pt + strlen(GETIFINFO_GATEWAY);
		inet_aton((const char *)pt, &dev->gateway);
	}else {
		dev->gateway.s_addr = 0;
	}
	if (NULL != (pt = strstr(data, GETIFINFO_DNS1))){
		pt = pt + strlen(GETIFINFO_DNS1);
		inet_aton((const char *)pt, &dev->dns1);
	}else {
		dev->dns1.s_addr = 0;
	}
	if (NULL != (pt = strstr(data, GETIFINFO_DNS2))){
		pt = pt + strlen(GETIFINFO_DNS2);
		inet_aton((const char *)pt, &dev->dns2);
	}else {
		dev->dns2.s_addr = 0;
	}
	if (NULL != (pt = strstr(data, GETIFINFO_MAC))){
		pt = pt + strlen(GETIFINFO_MAC);
		strncpy(dev->mac, pt, 17);
		dev->mac[17] = 0;
	}else {
		dev->mac[0] = 0;
	}
	return 0;
}

/**
 * Function: rc_setnetifdevip
 * Description: set the net interface(dev->dev = "ethx") ip netmask gateway, the function will block before set finish.
 * Input:	dev 	dev->dev must be set
 * 					dev->iptype must be set static or dhcp
 * 					dev->ip must be set when iptype is static
 * 					dev->netmask must be set when iptype is static
 * 					dev->gateway must be set when iptype is static
 * Return:	-1 ip set failed
 * 			 0 ip set ok.
 */
int rc_setnetifdevip(struct rc_netifdev_t *dev){
	char cmd[0x200];
	int n;
	if (NULL == dev){
		LOG_ERR("dev is null");
		return -1;
	}

	if (strlen(dev->dev) > sizeof(dev->dev)){
		LOG_ERR("netif devname too long");
		return -1;
	}

	n = sizeof(cmd);
	if (0 >= (n -= snprintf(&cmd[sizeof(cmd) - n], n, "bash /etc/rc_script/setnetifdevip.sh %s ", dev->dev))){
		LOG_ERR("command too long");
		return -1;
	}
	switch (dev->iptype){
	case RC_IPTYPE_STATIC:
			if ((INADDR_NONE == dev->ip.s_addr) || (INADDR_ANY == dev->ip.s_addr)) {
				LOG_ERR("invalid ip");
				return -1;
			}
			if (0 >= (n -= snprintf(&cmd[sizeof(cmd) - n], n, " %s ", inet_ntoa(dev->ip)))){
				LOG_ERR("command too long");
				return -1;
			}
			if ((INADDR_NONE == dev->netmask.s_addr) || (INADDR_ANY == dev->netmask.s_addr)) {
				LOG_ERR("invalid netmask");
				return -1;
			}
			if (0 >= (n -= snprintf(&cmd[sizeof(cmd) - n], n, " %s ", inet_ntoa(dev->netmask)))){
				LOG_ERR("command too long");
				return -1;
			}
			if ((INADDR_NONE == dev->gateway.s_addr) || (INADDR_ANY == dev->gateway.s_addr)) {
				LOG_ERR("invalid gateway");
				return -1;
			}
			if (0 >= (n -= snprintf(&cmd[sizeof(cmd) - n], n, " %s ", inet_ntoa(dev->gateway)))){
				LOG_ERR("command too long");
				return -1;
			}
			break;
		case RC_IPTYPE_DHCP:
			break;
		default:
			LOG_ERR("invalid iptype");
			return -1;
	}

	LOG_DEBUG("run \"%s\"", cmd);
	if (0 != rc_system(cmd)){
		LOG_ERR("run \"%s\" error", cmd);
		return -1;
	}
    LOG_DEBUG("run \"%s\" finished", cmd);
	return 0;
}

struct rc_setnetifdevip_thread_t {
	struct rc_netifdev_t *dev;
	int (*callback)(int ret, void *data);
	void *data;
};

void *rc_setnetifdevip_thread(void *data){
	struct rc_setnetifdevip_thread_t *arg = data;
	int ret;

	ret = rc_setnetifdevip(arg->dev);
	if (arg->callback){
		arg->callback(ret, arg->data);
	}
	free(data);
	return NULL;
}

/**
 * Function: rc_setnetifdevip_noblock
 * Description: set the net interface(dev->dev = "ethx") ip netmask gateway, the function will return before set finish.
 * 				when ip set finish will call the function which int (*callback)(int ret, void *app_privdata).
 * 				the callback function'param ret is the return val of ip set,
 * 				and the param app_privdata is the param of the function rc_setnetifdevip_noblock
 * Input:	dev 	dev->dev must be set
 * 					dev->iptype must be set static or dhcp
 * 					dev->ip must be set when iptype is static
 * 					dev->netmask must be set when iptype is static
 * 					dev->gateway must be set when iptype is static
 * Return:	-1 ip set failed
 * 			 0 ip set ok.
 */
int rc_setnetifdevip_noblock(struct rc_netifdev_t *dev,int (*callback)(int, void *), void *app_privdata){
	pthread_t pthread_id = 0;
	struct rc_setnetifdevip_thread_t *arg;
	pthread_attr_t attr;

	arg = malloc(sizeof(struct rc_setnetifdevip_thread_t));
	if (NULL == arg){
		LOG_PERROR("malloc memory error");
		return -1;
	}
	arg->dev = dev;
	arg->callback = callback;
	arg->data = app_privdata;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (-1 == pthread_create(&pthread_id, &attr, rc_setnetifdevip_thread, arg)){
		LOG_PERROR("pthread_create error");
		free(arg);
		return -1;
	}
	LOG_DEBUG("rc_setnetifdevip_noblock create thhread ok");
	return 0;
}

/**
 * Function: rc_setdns
 * Description: set system dns, if dns1 and dns2 is invalid, will auto set by system
 * Input:	dns1
 * 			dns2
 * Return:	-1  set failed
 * 			 0  set ok.
 */
int rc_setdns(struct in_addr dns1, struct in_addr dns2){
	char cmd[0x200];
	int n, dns1_invalid = 0, dns2_invalid = 0;
	if ((INADDR_NONE == dns1.s_addr) || (INADDR_ANY == dns1.s_addr)){
		LOG_ERR("invaled dns1");
		dns1_invalid = 1;
	}
	if ((INADDR_NONE == dns2.s_addr) || (INADDR_ANY == dns2.s_addr)){
		LOG_ERR("invaled dns2");
		dns2_invalid = 1;
	}

	n = sizeof(cmd);
	if (0 >= (n -= snprintf(&cmd[sizeof(cmd) - n], n, "bash /etc/rc_script/setdns.sh "))){
		LOG_ERR("command too long");
		return -1;
	}
	if ((0 == dns1_invalid) && (0 >= (n -= snprintf(&cmd[sizeof(cmd) - n], n, " %s", inet_ntoa(dns1))))){
		LOG_ERR("command too long");
		return -1;
	}
	if ((0 == dns2_invalid) && (0 >= (n -= snprintf(&cmd[sizeof(cmd) - n], n, " %s", inet_ntoa(dns2))))){
		LOG_ERR("command too long");
		return -1;
	}
	LOG_DEBUG("run \"%s\"", cmd);
	if (0 != rc_system(cmd)){
		LOG_ERR("run \"%s\" error", cmd);
		return -1;
	}
	return 0;
}

/**
 * Function: rc_sethostname
 * Description: set system hostname
 * Input:	hostname
 * Return:	-1  set failed
 * 			 0  set ok.
 */
int rc_sethostname(const char *hostname){
	char cmd[0x200];
	int n;
	if (NULL == hostname){
		LOG_ERR("hostname is null");
		return -1;
	}

	if (strlen(hostname) > 0x80){
			LOG_ERR("hostname too long");
			return -1;
	}

	n = sizeof(cmd);
	if (0 >= (n -= snprintf(&cmd[sizeof(cmd) - n], n, "bash /etc/rc_script/sethostname.sh %s", hostname))){
		LOG_ERR("command too long");
		return -1;
	}
	LOG_DEBUG("run \"%s\"", cmd);
	if (0 != rc_system(cmd)){
		LOG_ERR("run \"%s\" error", cmd);
		return -1;
	}
	return 0;
}

/**
 * Function: rc_gethostname
 * Description: get system hostname
 * Input:	hostname buf and buf size
 * Return:	-1  get failed
 * 			 0  get ok.
 */
#define GET_HOSTNAME		"hostname="
int rc_gethostname(char *hostname, int size){
	char cmd[0x200], data[0x200];
	int n, datalen = sizeof(data);
	char *pt = NULL;
	if (NULL == hostname){
		LOG_ERR("hostname is null");
		return -1;
	}

	if (size < 2){
			LOG_ERR("hostname too long");
			return -1;
	}

	n = sizeof(cmd);
	if (0 >= (n -= snprintf(&cmd[sizeof(cmd) - n], n, "bash /etc/rc_script/gethostname.sh "))){
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
	data[datalen] = 0;

	if (NULL != (pt = strstr(data, GET_HOSTNAME))){
		pt = pt + strlen(GET_HOSTNAME);
		strncpy(hostname, pt, size - 1);
		hostname[size - 1] = '\0';
		if (NULL != (pt = strstr(hostname, "\n"))) *pt = '\0';
		if (NULL != (pt = strstr(hostname, "\r"))) *pt = '\0';
	}else {
		hostname[0] = '\0';
		return -1;
	}

	return 0;
}

/**
 * Function: rc_detectipconflicts
 * Description: detect the ip is conflicts on the internet
 * Input:	ip
 * Return:	-1  detect failed
 * 			 0  the ip isn't conflicts.
 * 			 1  the ip is conflict.
 */
int rc_detectipconflicts(struct in_addr ip){
	char cmd[0x200];
	int n, ret;
	if ((INADDR_NONE == ip.s_addr) || (INADDR_ANY == ip.s_addr)){
		LOG_ERR("invaled ip");
		return -1;
	}

	n = sizeof(cmd);
	if (0 >= (n -= snprintf(&cmd[sizeof(cmd) - n], n, "bash /etc/rc_script/detectipconflicts.sh %s ", inet_ntoa(ip)))){
		LOG_ERR("command too long");
		return -1;
	}
	if (0 < (ret = rc_system(cmd))){
		LOG_ERR("run \"%s\" error", cmd);
		return -1;
	}
	return ret;
}

typedef struct {
	pthread_t pthread_id;
	volatile int running;
	FILE *fd;
	char addr[0x100];
	int (*callback)(void *private_data, struct cb_opration_data *data);
	void *private_data;
}rc_request_ping_thread_t;

#define RC_NO_RESPONSE_TIMEOUT_10MS		300

void *rc_request_ping_thread(void *data){
	rc_request_ping_thread_t *arg = data;
	struct cb_opration_data op_data;
	FILE *fd;
	char cmd[0x200], buf[0x400];
	int n, flags, timeout;

	arg->running = 1;
	op_data.opration = cb_opration_exit;
	op_data.data = NULL;
	n = sizeof(cmd);
	if (0 >= (n -= snprintf(&cmd[sizeof(cmd) - n], n, "ping -O %s  \n", arg->addr))){ // ping  -D -O
		LOG_ERR("command too long");
		arg->callback(arg->private_data, &op_data);
		goto thread_end;
	}

	fd = rc_popen(cmd,"r");
	if (NULL == fd){
		LOG_ERR("run \"%s\" error", cmd);
		arg->callback(arg->private_data, &op_data);
		goto thread_end;
	}

	flags = fcntl(fileno(fd), F_GETFL, 0);
	if (flags < 0) {
		LOG_PERROR("fcntl ping fd error");
		arg->callback(arg->private_data, &op_data);
		goto thread_end;
	}
	flags |= O_NONBLOCK;
	if (fcntl(fileno(fd), F_SETFL, flags) < 0){
		LOG_PERROR("fcntl ping fd error");
		arg->callback(arg->private_data, &op_data);
		goto thread_end;
	}

	arg->fd = fd;
	LOG_INFO("ping start to loop fgets, fdno:%d", fileno(fd));
	timeout = 0;

    op_data.opration = cb_opration_retmsg;//cb_opration_no_response
    op_data.data = cmd;
    arg->callback(arg->private_data,&op_data);
	while (arg->running){
		if (fgets(buf, sizeof(buf) - 1, fd)) {
			LOG_DEBUG("ping msg:%s", buf);
			op_data.opration = cb_opration_retmsg;
			op_data.data = buf;
			arg->callback(arg->private_data, &op_data);
			timeout = 0;
		}
		if (timeout++ > RC_NO_RESPONSE_TIMEOUT_10MS) {
			LOG_INFO("ping opration is no response");
            op_data.opration = cb_opration_retmsg;//cb_opration_no_response
            op_data.data = "no response\n";
			arg->callback(arg->private_data, &op_data);
			timeout = 0;
		}
		usleep(10000);
	}

    LOG_DEBUG("pkill -SIGINT ping");
    rc_system("pkill -SIGINT ping");

	LOG_INFO("ping opration is closing");
	rc_pclose(fd);
	LOG_INFO("ping opration cancled");
thread_end:
	while (arg->running){
		sleep(1);
	}
	LOG_INFO("close ping free memory");
	free(data);
	return NULL;
}

void* rc_request_ping(char* addr, int callback(void *private_data, struct cb_opration_data *data), void *private_data){
	rc_request_ping_thread_t *arg;
	pthread_attr_t attr;

	LOG_DEBUG("rc_request_ping to ping %s", addr);
	arg = malloc(sizeof(rc_request_ping_thread_t));
	if (NULL == arg){
		LOG_PERROR("malloc memory error");
		return NULL;
	}
	if ((NULL == addr) || (strlen(addr) > (sizeof(arg->addr) - 1))){
		LOG_ERR("addr too long");
		return NULL;
	}
	strcpy(arg->addr, addr);
	arg->callback = callback;
	arg->private_data = private_data;
	arg->fd = NULL;
	arg->running = 0;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (-1 == pthread_create(&arg->pthread_id, &attr, rc_request_ping_thread, arg)){
		LOG_PERROR("pthread_create error");
		free(arg);
		return NULL;
	}
	LOG_DEBUG("rc_request_ping create thhread ok, ping %s", addr);
	return arg;
}

int rc_cancle_ping(void *handle){
	rc_request_ping_thread_t *arg = handle;
	if (NULL == arg){
		LOG_ERR("invaled handle(null)");
		return -1;
	}
	if (arg->running) {
		LOG_INFO("close ping opration");
		arg->running = 0;
//		rc_pclose(arg->fd);
//		LOG_INFO("close ping fd (pclose)");
//		pthread_cancel(arg->pthread_id);
//		LOG_INFO("cancle ping thread");
//		free(arg);
		LOG_INFO("close ping done");
		return 0;
	} else {
		LOG_ERR("invaled status");
	}
	return -1;
}

void *rc_request_arping_thread(void *data){
	rc_request_ping_thread_t *arg = data;
	struct cb_opration_data op_data;
	FILE *fd;
	char cmd[0x200], buf[0x400];
	int n, flags, timeout;
	struct rc_netifdev_t dev;

	arg->running = 1;
	op_data.opration = cb_opration_exit;
	op_data.data = NULL;

	if (rc_getdefaultnetifname(dev.dev, sizeof(dev.dev) - 1)){
		LOG_ERR("rc_getdefaultnetifname error");
		arg->callback(arg->private_data, &op_data);
		goto thread_end;
	}
	if (rc_getnetifdevinfo(&dev)){
		LOG_ERR("rc_getnetifdevinfo error");
		arg->callback(arg->private_data, &op_data);
		goto thread_end;
	}

	n = sizeof(cmd);
	if (0 >= (n -= snprintf(&cmd[sizeof(cmd) - n], n, "arping -0 %s ", arg->addr))){
		LOG_ERR("command too long");
		arg->callback(arg->private_data, &op_data);
		goto thread_end;
	}

	fd = rc_popen(cmd,"r");
	if (NULL == fd){
		LOG_ERR("run \"%s\" error", cmd);
		arg->callback(arg->private_data, &op_data);
		goto thread_end;
	}

	flags = fcntl(fileno(fd), F_GETFL, 0);
	if (flags < 0) {
		LOG_PERROR("fcntl arping fd error");
		arg->callback(arg->private_data, &op_data);
		goto thread_end;
	}
	flags |= O_NONBLOCK;
	if (fcntl(fileno(fd), F_SETFL, flags) < 0){
		LOG_PERROR("fcntl arping fd error");
		arg->callback(arg->private_data, &op_data);
		goto thread_end;
	}

	arg->fd = fd;
	op_data.opration = cb_opration_retmsg;
	op_data.data = buf;
	LOG_INFO("arping start to loop fgets, fdno:%d", fileno(fd));
	timeout = 0;
	while (arg->running){
		if (fgets(buf, sizeof(buf) - 1, fd)) {
			LOG_DEBUG("arping msg:%s", buf);
			if (strstr(buf, dev.mac)){
				//my netif mac skip
				LOG_INFO("myself mac");
				op_data.opration = cb_opration_retmsg;
				op_data.data = "Timeout";
			}else {
				LOG_INFO("rcv other mac or other msg");
				op_data.opration = cb_opration_retmsg;
				op_data.data = buf;
			}
			arg->callback(arg->private_data, &op_data);
			timeout = 0;
		}
		if (timeout++ > RC_NO_RESPONSE_TIMEOUT_10MS){
			LOG_INFO("arping opration is no response");
			op_data.opration = cb_opration_no_response;
			op_data.data = "Timeout";
			arg->callback(arg->private_data, &op_data);
			timeout = 0;
		}
		usleep(10000);
	}

	LOG_INFO("arping opration is closing");
	rc_pclose(fd);
	LOG_INFO("arping opration cancled");
thread_end:
	while (arg->running){
		sleep(1);
	}
	LOG_INFO("close arping free memory");
	free(data);
	return NULL;
}

void* rc_request_arping(char* addr, int callback(void *private_data, struct cb_opration_data *data), void *private_data){
	rc_request_ping_thread_t *arg;
	pthread_attr_t attr;

	LOG_DEBUG("rc_request_arping to arping %s", addr);
	arg = malloc(sizeof(rc_request_ping_thread_t));
	if (NULL == arg){
		LOG_PERROR("malloc memory error");
		return NULL;
	}
	if ((NULL == addr) || (strlen(addr) > (sizeof(arg->addr) - 1))){
		LOG_ERR("addr too long");
		return NULL;
	}
	strcpy(arg->addr, addr);
	arg->callback = callback;
	arg->private_data = private_data;
	arg->fd = NULL;
	arg->running = 0;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (-1 == pthread_create(&arg->pthread_id, &attr, rc_request_arping_thread, arg)){
		LOG_PERROR("pthread_create error");
		free(arg);
		return NULL;
	}
	LOG_DEBUG("rc_request_arping create thhread ok, arping %s", addr);
	return arg;
}

int rc_cancle_arping(void *handle){
	rc_request_ping_thread_t *arg = handle;
	if (NULL == arg){
		LOG_ERR("invaled handle(null)");
		return -1;
	}
	if (arg->running) {
		LOG_INFO("close arping opration");
		arg->running = 0;
//		pthread_cancel(arg->pthread_id);
//		LOG_INFO("cancle arping thread");
//		rc_pclose(arg->fd);
//		LOG_INFO("close arping fd (pclose)");
//		free(arg);
//		LOG_INFO("close arping free memory");
		LOG_INFO("close arping done");
		return 0;
	} else {
		LOG_ERR("invaled status");
	}
	return -1;
}

