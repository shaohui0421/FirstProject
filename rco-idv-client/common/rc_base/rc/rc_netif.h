/*
 * rc_netif.h
 *
 *  Created on: Feb 7, 2017
 *      Author: zhf
 */

/*
 * rcc_public.h
 *
 *  Created on: Feb 4, 2017
 *      Author: zhf
 */

#ifndef _RC_NETIF_H_
#define _RC_NETIF_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RC_IPTYPE_UNKNOW	0
#define RC_IPTYPE_STATIC	1
#define RC_IPTYPE_DHCP		2
#define RC_IPTYPE_LOOPBACK	3

//dev status
#define RC_DNSTYPE_UNKNOW	0
#define RC_DNSTYPE_STATIC	1
#define RC_DNSTYPE_DHCP		2

#define RC_IFDEV_STATUS_UNKNOW	0
#define RC_IFDEV_STATUS_OFF	1
#define RC_IFDEV_STATUS_ON	2

#define RC_IFDEV_LINKSTATUS_UNKNOW		0
#define RC_IFDEV_LINKSTATUS_OFF			1
#define RC_IFDEV_LINKSTATUS_ON			2

#define RC_IFDEV_SPEED_UNKNOW		0
#define RC_IFDEV_SPEED_10M			10
#define RC_IFDEV_SPEED_100M			100
#define RC_IFDEV_SPEED_1000M		1000


struct rc_netifdev_t{
	char dev[32];	//devname eg.ethx brx ...
	int status; 	//dev status(defined in micro RC_IFDEV_STATUS_XXX) eg.  RC_IFDEV_STATUS_ON
	int link_speed; //dev speed(defined in micro RC_IFDEV_SPEED_XXX) eg.  RC_IFDEV_SPEED_1000M
	int link_status; //dev link status(defined in micro RC_IFDEV_LINKSTATUS_XXX) eg.  RC_IFDEV_LINKSTATUS_OFF
	char mac[18];
	int iptype; 	//dev iptype(defined in micro RC_IPTYPE_XXX) eg.  RC_IPTYPE_STATIC
	int dnstype;	//dev dnstype(defined in micro RC_DNSTYPE_XXX) eg.  RC_DNSTYPE_DHCP
	struct in_addr ip;
	struct in_addr netmask;
	struct in_addr gateway;
	struct in_addr dns1;
	struct in_addr dns2;
};

int rc_getnetifnum(void);
int rc_getnetifnamelist(char *data, int size);
int rc_getdefaultnetifname(char *data, int size);
int rc_getnetifdevinfo(struct rc_netifdev_t *dev);

int rc_setnetifdevip(struct rc_netifdev_t *dev);
int rc_setnetifdevip_noblock(struct rc_netifdev_t *dev,int (*callback)(int, void *), void *data);
int rc_setdns(struct in_addr dns1, struct in_addr dns2);
int rc_sethostname(const char *hostname);
int rc_gethostname(char *hostname, int size);

int rc_detectipconflicts(struct in_addr ip);

enum cb_opration_data_opration {
	cb_opration_no_response = -2,
	cb_opration_exit = -1,
	cb_opration_retmsg = 0,
};

struct cb_opration_data {
	int opration;
	int size;
	void *data;
};

void* rc_request_ping(char* addr, int callback(void *private_data, struct cb_opration_data *data), void *private_data);
int rc_cancle_ping(void *handle);
void* rc_request_arping(char* addr, int callback(void *private_data, struct cb_opration_data *data), void *private_data);
int rc_cancle_arping(void *handle);

#ifdef __cplusplus
}
#endif

#endif

