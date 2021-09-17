/*
 * rc_checknetifval.h
 *
 *  Created on: Mar 1, 2017
 *      Author: zhf
 */

#ifndef __RC_CHECKNETIFVAL_H__
#define __RC_CHECKNETIFVAL_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	RC_IP_OK = 0,
	RC_IP_ERR_INVALID = -1,
	RC_IP_ERR_INVALID_HOSTNAME = -2,
	RC_IP_ERR_INVALID_SERVERIP = -3,
	RC_IP_ERR_INVALID_IP = -4,
	RC_IP_ERR_INVALID_NETMASK = -5,
	RC_IP_ERR_INVALID_GATEWAY = -6,
	RC_IP_ERR_CONFLICT_IP_GATEWAY = -7,
	RC_IP_ERR_CONFLICT_IP_SERVERIP = -8,
} rc_ipretcode;

#define RC_HOSTNAME_MAXLEN		20


int rc_check_hostname(const char *hostname, int hostnamemaxlen);
int rc_check_ip_format_valid(const char *ip);
int rc_check_input_ip(const char *ip);
int rc_check_password(const char *password, int passwordmaxlen);
rc_ipretcode rc_check_input_network(const char *ip, const char *netmask, const char *gateway);
rc_ipretcode rc_check_input_valid(const char *hostname, const char *serverip, const char *ip, const char *netmask, const char *gateway);

#ifdef __cplusplus
}
#endif

#endif


