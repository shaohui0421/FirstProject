/*
 * netutil.c
 *
 *  Created on: 2017-1-19
 *      Author: chenw
 */
#include "common.h"
#include "rc/rc_netif.h"

typedef uint32_t u_int;

netinfo_t	local_net;
dns_info_t	local_dns;

netinfo_t	vm_net;
dns_info_t	vm_dns;


#define	VM_CONFIG_FILE				"/opt/lessons/RCC_Client/vm_network_info.ini"

int is_local_mac(char *mac)
{
	return !strcasecmp(local_net.mac_string, mac);
}


int getnetname(char *eth_name, int size)
{
	return rc_getdefaultnetifname(eth_name, size);
}


static void replay_char(void *str, char c, char play)
{
    char *p;
    p = str;

    if (c == play) {
    	return;
    }

    while ((p = strchr(p, c)))  {
        *p = play;
    }

    return;
}

void capital_char(char *str)
{
    char *p;

    if (str == NULL) {
        return;
    }

    for (p = str; *p != '\0'; p++) {
        if (*p >= 'a' && *p <= 'z') {
            *p = *p - 32;
        }
    }

    return;
}


int UpdateNetworkInfo(netinfo_t *net, dns_info_t *dns)
{
	char net_name[32];
	struct rc_netifdev_t netdev;
	int ret;

	ret = 0;

	memset(&netdev, 0, sizeof(netdev));
	ret = getnetname(net_name, sizeof(net_name));
	if (ret < 0) {
		return ret;
	}

	strncpy(netdev.dev, net_name, sizeof(netdev.dev) -1);

	ret = rc_getnetifdevinfo(&netdev);
	if (ret < 0) {
		//log_err("rc_getnetifdevinfo err\n");
		return ret;
	}

	strncpy(net->ip, inet_ntoa(netdev.ip), sizeof(net->ip) - 1);
	strncpy(net->mask, inet_ntoa(netdev.netmask), sizeof(net->mask) - 1);

	if (netdev.iptype == RC_IPTYPE_STATIC) {
		strncpy(net->ipType, "Static", sizeof(net->ipType) - 1);
	} else if (netdev.iptype == RC_IPTYPE_DHCP) {
		strncpy(net->ipType, "Auto", sizeof(net->ipType) - 1);
	} else {
		memset(net->ipType, 0, sizeof(net->ipType));
	}

	strncpy(net->mac_string, netdev.mac, sizeof(net->mac_string) - 1);
	replay_char(net->mac_string, ':', '-');
    capital_char(net->mac_string);

	strncpy(net->gateway, inet_ntoa(netdev.gateway), sizeof(net->gateway) - 1);

	strncpy(dns->dnsaddr1, inet_ntoa(netdev.dns1), sizeof(dns->dnsaddr1) - 1);
	strncpy(dns->dnsaddr2, inet_ntoa(netdev.dns2), sizeof(dns->dnsaddr2) - 1);
	if ((strcasecmp(dns->dnsaddr1, "0.0.0.0") == 0) || (strcasecmp(dns->dnsaddr1, "255.255.255.255")) == 0) {
		memset(dns->dnsaddr1, 0, sizeof(dns->dnsaddr1));
	}
	if ((strcasecmp(dns->dnsaddr2, "0.0.0.0") == 0) || (strcasecmp(dns->dnsaddr2, "255.255.255.255")) == 0) {
		memset(dns->dnsaddr2, 0, sizeof(dns->dnsaddr2));
	}

	if (netdev.dnstype == RC_DNSTYPE_STATIC) {
		strncpy(dns->dnstype, "Static", sizeof(dns->dnstype) - 1);
	} else if (netdev.dnstype == RC_DNSTYPE_DHCP) {
		strncpy(dns->dnstype, "Auto", sizeof(dns->dnstype) - 1);
	} else {
		memset(dns->dnstype, 0, sizeof(dns->dnstype));
	}

    return ret;
}


int updata_vm_network(netinfo_t *net, dns_info_t *dns)
{
	IniFile* ini;
	char *tmp;

	if(access(VM_CONFIG_FILE, F_OK) != 0) {
		return -1;
	}

	ini = ini_file_open_file(VM_CONFIG_FILE);
	if (ini == NULL) {
		return -1;
	}

	ini_file_read_string(ini, "network-info", "dhcp", &tmp);
	if (tmp) {
		if (strcasecmp(tmp, "true") == 0) {
			strncpy(net->ipType, "Auto", sizeof(net->ipType) - 1);
		} else {
			strncpy(net->ipType,"Static", sizeof(net->ipType) - 1);
		}
		g_free(tmp);
	}

	ini_file_read_string(ini, "network-info", "ip", &tmp);
	if (tmp) {
		strncpy(net->ip, tmp, sizeof(net->ip) - 1);
		g_free(tmp);
	}

	ini_file_read_string(ini, "network-info", "submask", &tmp);
	if (tmp) {
		strncpy(net->mask, tmp, sizeof(net->mask) - 1);
		g_free(tmp);
	}

	ini_file_read_string(ini, "network-info", "gateway", &tmp);
	if (tmp) {
		strncpy(net->gateway, tmp, sizeof(net->gateway) - 1);
		g_free(tmp);
	}

	ini_file_read_string(ini, "network-info", "auto_dns", &tmp);
	if (tmp) {
		if (strcasecmp(tmp, "true") == 0) {
			strncpy(dns->dnstype, "Auto", sizeof(dns->dnstype) - 1);
		} else {
			strncpy(dns->dnstype,"Static", sizeof(dns->dnstype) - 1);
		}
		g_free(tmp);
	}

	ini_file_read_string(ini, "network-info", "main_dns", &tmp);
	if (tmp) {
		strncpy(dns->dnsaddr1, tmp, sizeof(dns->dnsaddr1) - 1);
		g_free(tmp);
	}

	ini_file_read_string(ini, "network-info", "back_dns", &tmp);
	if (tmp) {
		strncpy(dns->dnsaddr2, tmp, sizeof(dns->dnsaddr2) - 1);
		g_free(tmp);
	}

	ini_file_free(ini);
	return 0;
}


