#ifndef __COMMON_H__
#define __COMMON_H__

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
//#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <net/if.h>
#include "inifile.h"
#include "cJSON.h"

#define		MAXMAC				50
#define		MAXIP				20
#define		MAXTYPE				10
#define		MAX_PATH			160
#define		STATENUM			10
#define		MSG_LEN				20480
#define 	BUFSIZE 			8192

#define		BRO_RECV_PORT		9115
#define		BRO_SND_PORT		9116
#define		BRO_SCAN_PORT		9117

//#define	RCC_CONFIG	"/opt/lessons/RCC_Client/RCC_Client_Config.ini"
#define		MINA_CONFIG	"/opt/lessons/RCC_Client/mina.ini"
#define EASY_DEPLOY_CONFIG "/tmp/easy_deploy.ini"

struct arg_t {
    int argc;
    char** argv;
};

typedef enum bool_t
{
	false = 0,
	true = 1
}
bool;

typedef enum dev_mode
{
	PRI_MODE = 0,
	MUTI_MODE,
	CMN_MODE
} dev_mode_t;

typedef void * gpointer;

typedef struct easydeploy{
	char b_teacherip[MAXIP];
	char b_serverip[MAXIP];
	char b_backserverip[MAXIP];
	char b_hostname[50];
	char b_resolution[50];
	char b_ip[MAXIP];
	char b_netmask[MAXIP];
	char b_gateway[32];
	char b_dns[MAXIP];
	char b_backdns[MAXIP];

	int	 b_mode;
	char b_vmip[MAXIP];
	char b_vmmask[MAXIP];
	char b_vmgateway[MAXIP];
	char b_vmdns[MAXIP];
	char b_vmbackdns[MAXIP];
	int b_vmdhcpip;
	int b_vmdhcpdns;
	int b_setip;
	int b_dhcpip;
	int b_dhcpdns;
	int b_autopower_enable;
	int b_autopower_need_deploy;
}EASYDEPLOY;

typedef struct netinfo {
	char ifVer[10];
	char ntType[MAXTYPE];
	char mac_string[MAXMAC];
	char mac_value[MAXMAC];
	char ip[MAXIP];
	char mask[MAXIP];
//	char submask[MAXIP];
	char gateway[MAXIP];
	char ipType[MAXTYPE];
}netinfo_t;

typedef struct display_info {
	int height;
	int width;
	int bitspixel;
	int refresh;
}display_info_t;

typedef struct dns_info {
	char dnsaddr1[MAXIP];
	char dnsaddr2[MAXIP];
	char dnstype[MAXTYPE];
} dns_info_t;


typedef struct send_msg_info {
	int seqnum;
	int role;
	int	devmod;
	char msgid[20]; //    strcpy(msgId,"0xdeaddead");
	char msg[20];
	char hostname[50];
	char sn[128];
	char rcdserip[MAXIP];
	char rcdslaveserip[MAXIP];
	char rccteacherip[MAXIP];
	netinfo_t net;
	dns_info_t dns;
	netinfo_t	vmnet;
	dns_info_t	vmdns;
	display_info_t	display;
} send_msg_info_t;

enum thread_type_s
{
    THREAD_TYPE_DEAL1,
    THREAD_TYPE_DEAL2,
};

enum msg_type_s
{
    MSG_CONF_BEGIN,
    MSG_CONF_IMPLE,
    MSG_SERI_PARSE,
    MSG_SERI_CLOSE,
    MSG_CONF_CLOSE,
    MSG_INFO_UPLOAD,
    MSG_TOTAL_NUM,
};

struct arg_t arg;
int sendfd;

extern netinfo_t	local_net;
extern dns_info_t	local_dns;
extern netinfo_t	vm_net;
extern dns_info_t	vm_dns;

extern display_info_t local_disply;

extern int get_disply_info(int *width,int *height,int *refresh);
extern void update_display_info(display_info_t *disply);
extern int UpdateNetworkInfo(netinfo_t *net, dns_info_t *dns);
extern int getnetname(char *name, int size);
extern int is_local_mac(char *mac);
extern int get_serialnum(char * serialNumber, int size);
extern int updata_vm_network(netinfo_t *net, dns_info_t *dns);
extern int get_dev_mode(int * mode);
extern int get_hostname(char * buf, int size);
extern int is_debug(void);
void set_thread_name(const char *name);
int send_msg(int sockfd, const char* buf, int len);
void *send_config_info(void);
void *send_mac_info(void *data);
int send_thread_init(void);
int is_vm_running(void);
int group_set_conf(EASYDEPLOY * m_easy_deploy);
void msg_timer_init(void);

#endif
