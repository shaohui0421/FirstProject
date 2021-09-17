#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "rc_log.h"
#include "rc_checknetifval.h"

//#undef LOG_ERR
//#define LOG_ERR(fmt, args...) printf("====%s %d " fmt "\n", __FUNCTION__, __LINE__, ##args)

#define INVALID_IP_ADDR_MIN 0
#define INVALID_IP_ADDR_MAX 0xffffffff

typedef struct
{
	unsigned char count;
	unsigned char head;
	unsigned char mask;
	unsigned char mask2;
}UTF8_HEAD_t;

const static UTF8_HEAD_t utf8Head[]={
	{2, 0xC0, 0xe0, 0x1f},
	{3, 0xE0, 0xf0, 0x0f},
	{4, 0xF0, 0xf8, 0x07},
	{5, 0xF8, 0xfc, 0x03},
	{6, 0xFC, 0xfe, 0x01},
};

/**
*utf-8
1bytes0xxxxxxx
2bytes110xxxxx10xxxxxx
3bytes1110xxxx10xxxxxx10xxxxxx
4bytes11110xxx10xxxxxx10xxxxxx10xxxxxx
5bytes111110xx10xxxxxx10xxxxxx10xxxxxx10xxxxxx
6bytes1111110x10xxxxxx10xxxxxx10xxxxxx10xxxxxx10xxxxxx
*/
int rc_get_one_utf8code(const char*data, int len, int *code)
{
	if ((NULL == data) || (len < 1))
	{
		LOG_ERR("invaled input data");
		return -1;
	}

	if(0 == (*data & 0x80)) {
		*code = *data;
		return 1;
	}

	unsigned int i;
	unsigned int c;
	for (i=0; i<sizeof(utf8Head)/sizeof(UTF8_HEAD_t); ++i) {
		if(utf8Head[i].head == (*data & utf8Head[i].mask)) {
			c = utf8Head[i].count;
			if (len < c){
				return -1;
			}
			*code = *data++ & utf8Head[i].mask2;
			while (--c) {
				if (0x80 == ((*data) & 0xc0)) {
					*code <<= 6;
					*code += (*data++ & 0x3f);
				}else {
					return -1;
				}
			}
			return utf8Head[i].count;
		}
	}
	return -1;
}

/*
 * function:rc_check_hostname
 * Description: check hostname value use utf8 is ok
 * 			    hostname only containt 0-9 a-z A-Z _ and Chinese
 * Input:  hostname 	utf8 string
 * 		   hostnamemaxlen 	max char len, Chinese is one char
 * Return: 0 OK
 * 		   -1 Error
 */
int rc_check_hostname(const char *hostname, int hostnamemaxlen)
{
    int len, ccount = 0;

    if (!hostname) {
    	LOG_ERR("hostname invaled(null)");
        return -1;
    }

    len = strlen(hostname);
    if (len == 0) {
    	LOG_ERR("hostname length invaled(0)");
        return -1;
    }

    while (len > 0) {
    	int code, utf8len;
    	utf8len = rc_get_one_utf8code(hostname, len, &code);
    	if (utf8len < 0) {
    		LOG_ERR("invaled utf8");
    		return -1;
    	}
    	if (++ccount > hostnamemaxlen){
    		LOG_ERR("hostname too long");
    		return -1;
    	}
    	if (((code >= 0x30) && (code <= 0x39))
    		|| ((code >= 0x61) && (code <= 0x7a))
			|| ((code >= 0x41) && (code <= 0x5a))
			|| (code == 0x5f)
			|| ((code >= 0x4e00) && (code <= 0x9fa5))
    		) {
    		len -= utf8len;
    		hostname += utf8len;
    	}else {
    		LOG_ERR("hostname invaled");
    		return -1;
    	}
    }
    if (0 != len) {
    	LOG_ERR("invaled input length or invaled");
    	return -1;
    }
    LOG_INFO("hostname length:%d", ccount);
    return 0;
}

/*
 * function:rc_check_ip_format_valid
 * Description: check ip format is valid
 * Input:  ip 	the ip to check
  * Return: 0 OK
 * 		   -1 Error
 */
int rc_check_ip_format_valid(const char *ip)
{
    struct sockaddr_in sa_ip;
    int result;

    if (NULL == ip) {
        return -1;
    }
    result = inet_pton(AF_INET, ip, &(sa_ip.sin_addr));
    if (result <= 0) {
    	LOG_ERR("invaled ip");
        return -1;
    }
    return 0;
}

/*
 * function:rc_check_input_ip
 * Description: check ip is ok
 * Input:  ip 	the ip to check
  * Return: 0 OK
 * 		   -1 Error
 */
int rc_check_input_ip(const char *ip)
{
    struct sockaddr_in sa_ip;
    int result;

    if (NULL == ip) {
        return -1;
    }
    LOG_DEBUG("%s", ip);
    result = inet_pton(AF_INET, ip, &(sa_ip.sin_addr));
    if (result <= 0 || sa_ip.sin_addr.s_addr == INVALID_IP_ADDR_MIN ||
        sa_ip.sin_addr.s_addr == INVALID_IP_ADDR_MAX) {
    	LOG_ERR("invaled ip");
        return -1;
    }
    result = atoi(ip);
    if ((result >= 1) && (result <= 126)){
    	//A address
    	LOG_INFO("A Class Address");
    	return 0;
    }
    if ((result >= 128) && (result <= 191)){
    	//B address
    	LOG_INFO("B Class Address");
    	return 0;
    }
    if ((result >= 192) && (result <= 223)){
    	//C address
    	LOG_INFO("C Class Address");
    	return 0;
    }
    if ((result >= 224) && (result <= 239)){
    	//D address
    	LOG_INFO("D Class Address");
    	LOG_ERR("invaled ip");
    	return -1;
    }
    if ((result >= 240) && (result <= 254)){
    	//E address
    	LOG_INFO("E Class Address");
    	LOG_ERR("invaled ip");
    	return -1;
    }
    LOG_ERR("invaled ip");
    return -1;
}


/*
 * function:rc_check_input_network
 * Description: check the input ip netmask gateway is ok
 * Input:  ip, netmask, gateway
 * Return: rc_ipretcode
 */
rc_ipretcode rc_check_input_network(const char *ip, const char *netmask, const char *gateway)
{
    struct sockaddr_in sa_ip;
    struct sockaddr_in sa_netmask;
    struct sockaddr_in sa_gateway;
    int result;
    unsigned int n[4];
    unsigned i;
    unsigned int x;
    unsigned int y;
    unsigned int z;

    if (!ip || !netmask || !gateway) {
    	LOG_ERR("maybe one of ip netmask gateway is null");
        return RC_IP_ERR_INVALID;
    }

    /* check netmask */
    result = inet_pton(AF_INET, netmask, &(sa_netmask.sin_addr));
    if (result <= 0 || sa_netmask.sin_addr.s_addr == 0) {
    	LOG_ERR("netmask is invalid");
        return RC_IP_ERR_INVALID_NETMASK;
    }
    sscanf(netmask, "%u.%u.%u.%u", &n[3], &n[2], &n[1], &n[0]);
    x = 0;
    for (i = 0; i < 4; i++) {
        x += n[i] << (i * 8);
    }
    y = ~x;
    z = y + 1;
    LOG_ERR("x 0x%08x, y 0x%08x, z 0x%08x", x, y, z);
    if ((y & z) != 0) {
    	LOG_ERR("netmask is invalid");
        return RC_IP_ERR_INVALID_NETMASK;
    }

    result = inet_pton(AF_INET, ip, &(sa_ip.sin_addr));
    if (result <= 0
    || (rc_check_input_ip(ip) < 0)
	|| ((sa_ip.sin_addr.s_addr & (~sa_netmask.sin_addr.s_addr)) == 0)
	|| ((sa_ip.sin_addr.s_addr & (~sa_netmask.sin_addr.s_addr)) == ((typeof(sa_netmask.sin_addr.s_addr))-1 & (~sa_netmask.sin_addr.s_addr)))
	) {
    	LOG_ERR("ip is invalid");
        return RC_IP_ERR_INVALID_IP;
    }

    result = inet_pton(AF_INET, gateway, &(sa_gateway.sin_addr));
    if (result <= 0
	|| (rc_check_input_ip(ip) < 0)
	|| ((sa_gateway.sin_addr.s_addr & (~sa_netmask.sin_addr.s_addr)) == 0)
	|| ((sa_gateway.sin_addr.s_addr & (~sa_netmask.sin_addr.s_addr)) == ((typeof(sa_netmask.sin_addr.s_addr))-1 & (~sa_netmask.sin_addr.s_addr)))
	) {
    	LOG_ERR("gateway is invalid");
        return RC_IP_ERR_INVALID_GATEWAY;
    }

    if ((sa_gateway.sin_addr.s_addr & sa_netmask.sin_addr.s_addr)
     != (sa_ip.sin_addr.s_addr & sa_netmask.sin_addr.s_addr)
	) {
    	LOG_ERR("ip gateway is in diff network");
        return RC_IP_ERR_INVALID_GATEWAY;
    }

    if (sa_ip.sin_addr.s_addr == sa_gateway.sin_addr.s_addr) {
    	LOG_ERR("ip gateway is conelict");
        return RC_IP_ERR_CONFLICT_IP_GATEWAY;
    }

    return RC_IP_OK;
}

/*
 * function:rc_check_input_valid
 * Description: check the input value is ok
 * Input:  hostname 	containt 0-9 a-z A-Z _ and Chinese, maxlen RC_HOSTNAME_MAXLEN
 * 		   serverip, ip, netmask, gateway
 * Return: rc_ipretcode
 */
rc_ipretcode rc_check_input_valid(const char *hostname, const char *serverip, const char *ip, const char *netmask, const char *gateway)
{
    struct sockaddr_in sa_serverip;
    struct sockaddr_in sa_ip;
    int result;

    if (!hostname || !serverip || !ip || !netmask || !gateway) {
    	LOG_ERR("maybe one of hostname serverip ip netmask gateway is null");
        return RC_IP_ERR_INVALID;
    }

    if (rc_check_hostname(hostname, RC_HOSTNAME_MAXLEN) != 0) {
    	LOG_ERR("hostname is invalid");
        return RC_IP_ERR_INVALID_HOSTNAME;
    }

    result = inet_pton(AF_INET, serverip, &(sa_serverip.sin_addr));
    if (result <= 0
	|| (rc_check_input_ip(ip) < 0)
	) {
    	LOG_ERR("serverip is invalid");
        return RC_IP_ERR_INVALID_SERVERIP;
    }

    result = rc_check_input_network(ip, netmask, gateway);
    if (result < 0){
    	LOG_ERR("ip netmask gateway is invalid");
    	return result;
    }

    result = inet_pton(AF_INET, ip, &(sa_ip.sin_addr));
    if (result <= 0) {
    	LOG_ERR("ip is invalid");
        return RC_IP_ERR_INVALID_IP;
    }

    if (sa_ip.sin_addr.s_addr == sa_serverip.sin_addr.s_addr) {
    	LOG_ERR("ip serverip is conelict");
        return RC_IP_ERR_CONFLICT_IP_SERVERIP;
    }
    return RC_IP_OK;
}

/*
 * function:rc_check_password
 * Description: check the password value use utf8 is ok
 * 		           password not containt special characters
 * Input:  password string
 * 		   passwordmaxlen 	max char len
 * Return: 0 OK
 * 		   -1 Error
 */
int rc_check_password(const char *password, int passwordmaxlen)
{
    int len, ccount = 0;

    if (!password) {
        LOG_ERR("password invaled(null)");
        return -1;
    }

    len = strlen(password);
    if (len == 0) {
        LOG_ERR("password length invaled(0)");
        return -1;
    }

    while (len > 0) {
        int code, utf8len;
        utf8len = rc_get_one_utf8code(password, len, &code);
        if (utf8len < 0) {
            LOG_ERR("invaled utf8");
            return -1;
        }
        if (++ccount > passwordmaxlen){
            LOG_ERR("password too long");
            return -1;
        }
        if (((code >= 0x20) && (code <= 0x7e))) {
            len -= utf8len;
            password += utf8len;
        }else {
            LOG_ERR("password invaled");
            return -1;
        }
    }
    if (0 != len) {
        LOG_ERR("invaled input length or invaled");
        return -1;
    }
    LOG_INFO("password length:%d", ccount);
    return 0;
}

