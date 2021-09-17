#include <sys/prctl.h>
#include <glib.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include "common.h"
#include "rc_log.h"
#include "logic_event.h"

#define TIME_NULL              (-1)
#define DEBUG_FILE             "/etc/ter_debug"
#define PRIVATE_ACCOUNT        "private_account"
#define NO_ACCOUNT             "no_account"
#define MUTI_ACCOUNT           "muti_account"
#define MSG_CONFIG_TIME        5
#define MSG_COMMON_TIME        2

time_t last_msg_time[MSG_TOTAL_NUM];

struct sockaddr_in localaddr;  //9115
struct sockaddr_in bcastaddr;  //9116
struct sockaddr_in localaddr2; //9117
pthread_t recv_tid;
pthread_t scan_tid;

int common_system(const char * cmdstring)
{
    pid_t pid;
    int status = 0;
    int i = 0;

    if(cmdstring == NULL)
    {
        return (1);
    }

    if((pid = fork())<0)
    {
        status = -1;
    }
    else if(pid == 0)
    {
        for (i = 3; i < 1024; i++)
        {
            close(i);
        }

        execl("/bin/sh", "sh", "-c", cmdstring, (char *)0);
        _exit(127);
    }
    else
    {
        while(waitpid(pid, &status, 0) < 0)
        {
            if(errno != EINTR)
            {
                status = -1;
                break;
            }
        }
    }

    return status;
}

int common_exec_cli_cmd(const char *cmd)
{
    int status = -1;
    char ret = -1;

    if (cmd == NULL) {
        return -1;
    }

    if (strlen(cmd) > 512) {
        LOG_ERR("length of cmd is over");
        return -1;
    }

    status = 0;
    __sighandler_t old_handler;
    old_handler = signal(SIGCHLD, SIG_DFL);

    status = common_system(cmd);
    signal(SIGCHLD, old_handler);

    if (WIFEXITED(status)) {
        ret = WEXITSTATUS(status);
        if (ret == -1) {
            LOG_ERR("error cmd: %s", strerror(errno));
        }
    } else if (WIFSIGNALED(status)) {
        LOG_ERR("abnormal termination, signal number =%d", WTERMSIG(status));
    } else if (WIFSTOPPED(status)) {
        LOG_ERR("process stopped, signal number =%d", WSTOPSIG(status));
    }

    return ret;
}

int is_vm_running(void)
{
    int ret;

    ret = common_exec_cli_cmd("ps aux | grep qemu-system-x86 | grep -v grep");
    if (ret) {
        return FALSE;
    }
    return TRUE;
}

void set_thread_name(const char *name)
{
    int ret;

    if (NULL == name || strlen(name) == 0) {
        LOG_ERR("set thread name err, thread name is empty");
        return;
    }

    ret = prctl(PR_SET_NAME, name, NULL, NULL, NULL);
    if (ret < 0) {
        LOG_WARNING("set thread[%lu] name(%s) failed: %s\n",
            pthread_self(), name, strerror(errno));
    }
}

time_t get_monolithic_time(void)
{
    struct timespec time_space;
    clock_gettime(CLOCK_MONOTONIC, &time_space);
    return time_space.tv_sec;
}

int message_time_filter(int type, double interval)
{
    double diff_time;
    time_t current_time;
    time_t last_time;

    if (type >= MSG_TOTAL_NUM || type < 0) {
        return FALSE;
    }

    last_time = last_msg_time[type];
    current_time = get_monolithic_time();
    diff_time = difftime(current_time, last_time);
    if (last_time != TIME_NULL && diff_time < interval) {
        return FALSE;
    }
    last_msg_time[type] = current_time;
    return TRUE;
}

int is_file_exist(char *name)
{
    return (access(name, F_OK)? 0 : 1);
}

int is_debug(void)
{
	return is_file_exist(DEBUG_FILE);
}

int is_local_conf(char *mac)
{
    return is_local_mac(mac);
}

static int empty_str(char *buf)
{
    return buf[0] == '\0' ?  1 : 0;
}

void show_system_info(void)
{
    LOG_INFO("==========SHOW LOCAL INFO==========");
    LOG_INFO("ifver:         %s", local_net.ifVer);
    LOG_INFO("net type:      %s", local_net.ntType);
    LOG_INFO("local ip:      %s", local_net.ip);
    LOG_INFO("ip type:       %s", local_net.ipType);
    LOG_INFO("mask:          %s", local_net.mask);
    LOG_INFO("gateway:       %s", local_net.gateway);
    LOG_INFO("local mac:     %s", local_net.mac_string);
    LOG_INFO("dns type:      %s", local_dns.dnstype);
    LOG_INFO("main dns:      %s", local_dns.dnsaddr1);
    LOG_INFO("vice dns:      %s", local_dns.dnsaddr2);
    LOG_INFO("vm ip:         %s", vm_net.ip);
    LOG_INFO("vm ip type:    %s", vm_net.ipType);
    LOG_INFO("vm mask:       %s", vm_net.mask);
    LOG_INFO("vm gateway:    %s", vm_net.gateway);
    LOG_INFO("vm dns type:   %s", vm_dns.dnstype);
    LOG_INFO("vm main dns:   %s", vm_dns.dnsaddr1);
    LOG_INFO("vm vice dns:   %s", vm_dns.dnsaddr2);
    LOG_INFO("bits pixel:    %d", local_disply.bitspixel);
    LOG_INFO("height:        %d", local_disply.height);
    LOG_INFO("width:         %d", local_disply.width);
    LOG_INFO("refresh rate:  %d", local_disply.refresh);
    LOG_INFO("===================================");
}

void system_info_init(void)
{
    memset(&local_net, 0, sizeof(local_net));
    memset(&local_dns, 0, sizeof(local_dns));
    memset(&vm_net, 0, sizeof(vm_net));
    memset(&vm_dns, 0, sizeof(vm_dns));

    local_disply.bitspixel = 32;
    local_disply.height = 768;
    local_disply.refresh = 60;
    local_disply.width = 1366;

    strncpy(local_net.ifVer, "wired", sizeof(local_net.ifVer) - 1);

    UpdateNetworkInfo(&local_net, &local_dns);

    get_disply_info(&(local_disply.width), &(local_disply.height), &(local_disply.refresh));
    updata_vm_network(&vm_net, &vm_dns);
    show_system_info();
}

void msg_timer_init(void)
{
    int index;

    for (index = 0; index < MSG_TOTAL_NUM; index++) {
        last_msg_time[index] = TIME_NULL;
    }
    return;
}

int group_set_conf(EASYDEPLOY * m_easy_deploy)
{

    IniFile* ini;
    ini = ini_file_new();

    if (empty_str(m_easy_deploy->b_serverip)) {
    	ini_file_write_string(ini,"easy_deploy","serverip","");
    } else {
    	ini_file_write_string(ini,"easy_deploy","serverip",m_easy_deploy->b_serverip);
    }


    if (empty_str(m_easy_deploy->b_hostname)) {
    	ini_file_write_string(ini,"easy_deploy","hostname","");
    } else {
    	ini_file_write_string(ini,"easy_deploy","hostname",m_easy_deploy->b_hostname);
    }

    if (empty_str(m_easy_deploy->b_resolution)) {
    	ini_file_write_string(ini,"easy_deploy","resolution","");
    } else {
    	ini_file_write_string(ini,"easy_deploy","resolution",m_easy_deploy->b_resolution);
    }

    if (empty_str(m_easy_deploy->b_ip)) {
    	ini_file_write_string(ini,"easy_deploy","ip","");
    } else {
    	ini_file_write_string(ini,"easy_deploy","ip",m_easy_deploy->b_ip);
    }

    if (empty_str(m_easy_deploy->b_netmask)) {
    	ini_file_write_string(ini,"easy_deploy","netmask","");
    } else {
    	ini_file_write_string(ini,"easy_deploy","netmask",m_easy_deploy->b_netmask);
    }

    if (empty_str(m_easy_deploy->b_gateway)) {
    	ini_file_write_string(ini,"easy_deploy","gateway","");
    } else {
    	ini_file_write_string(ini,"easy_deploy","gateway",m_easy_deploy->b_gateway);
    }

    if (empty_str(m_easy_deploy->b_dns)) {
    	ini_file_write_string(ini,"easy_deploy","dns","");
    } else {
    	ini_file_write_string(ini,"easy_deploy","dns",m_easy_deploy->b_dns);
    }

    if (empty_str(m_easy_deploy->b_backdns)) {
    	ini_file_write_string(ini,"easy_deploy","backdns","");
    } else {
    	ini_file_write_string(ini,"easy_deploy","backdns",m_easy_deploy->b_backdns);
    }

    if (m_easy_deploy->b_dhcpip != -1) {
    	ini_file_write_boolean(ini,"easy_deploy","dhcpip",m_easy_deploy->b_dhcpip);
    } else {
    	ini_file_write_int(ini,"easy_deploy","dhcpip",-1);
    }

    if (m_easy_deploy->b_dhcpdns != -1) {
    	ini_file_write_boolean(ini,"easy_deploy","dhcpdns",m_easy_deploy->b_dhcpdns);
    } else {
    	ini_file_write_int(ini,"easy_deploy","dhcpdns",-1);
    }

    if (m_easy_deploy->b_autopower_enable != -1) {
    	ini_file_write_boolean(ini,"easy_deploy","autopower",m_easy_deploy->b_autopower_enable);
    } else {
    	ini_file_write_int(ini,"easy_deploy","autopower",-1);
    }

    if (m_easy_deploy->b_autopower_need_deploy != -1) {
        ini_file_write_boolean(ini,"easy_deploy","power_need_deploy",m_easy_deploy->b_autopower_need_deploy);
    } else {
    	ini_file_write_int(ini,"easy_deploy","power_need_deploy",-1);
    }

    if (m_easy_deploy->b_mode != -1) {
    	ini_file_write_int(ini,"easy_deploy","mode",m_easy_deploy->b_mode);
    } else {
    	ini_file_write_int(ini,"easy_deploy","mode",-1);
    }

    if (empty_str(m_easy_deploy->b_vmip)) {
    	ini_file_write_string(ini,"easy_deploy","vm_ip","");
    } else {
    	ini_file_write_string(ini,"easy_deploy","vm_ip",m_easy_deploy->b_vmip);
    }

    if (empty_str(m_easy_deploy->b_vmmask)) {
    	ini_file_write_string(ini,"easy_deploy","vm_netmask","");
    } else {
    	ini_file_write_string(ini,"easy_deploy","vm_netmask",m_easy_deploy->b_vmmask);
    }

    if (empty_str(m_easy_deploy->b_vmgateway)) {
    	ini_file_write_string(ini,"easy_deploy","vm_gateway","");
    } else {
    	ini_file_write_string(ini,"easy_deploy","vm_gateway",m_easy_deploy->b_vmgateway);
    }

    if (empty_str(m_easy_deploy->b_vmdns)) {
    	ini_file_write_string(ini,"easy_deploy","vm_dns","");
    } else {
    	ini_file_write_string(ini,"easy_deploy","vm_dns",m_easy_deploy->b_vmdns);
    }

    if (empty_str(m_easy_deploy->b_vmbackdns)) {
    	ini_file_write_string(ini,"easy_deploy","vm_backdns","");
    } else {
    	ini_file_write_string(ini,"easy_deploy","vm_backdns",m_easy_deploy->b_vmbackdns);
    }


    if (m_easy_deploy->b_vmdhcpip != -1) {
    	ini_file_write_boolean(ini,"easy_deploy","vm_dhcpip",m_easy_deploy->b_vmdhcpip);
    } else {
    	ini_file_write_int(ini,"easy_deploy","vm_dhcpip",-1);
    }

    if (m_easy_deploy->b_vmdhcpdns != -1) {
    	ini_file_write_boolean(ini,"easy_deploy","vm_dhcpdns",m_easy_deploy->b_vmdhcpdns);
    } else {
    	ini_file_write_int(ini,"easy_deploy","vm_dhcpdns",-1);
    }

    ini_file_write_file(ini,EASY_DEPLOY_CONFIG);
    ini_file_free(ini);

    return system("kill -10 `pidof IDV_Client`");
}

int createsndsock(int port)
{
    int sockfd;
    int opt = 1;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0){
        LOG_ERR("socket error:%s", strerror(errno));
        return -1;
    }

    bcastaddr.sin_family = AF_INET;
    bcastaddr.sin_addr.s_addr = INADDR_BROADCAST;
    bcastaddr.sin_port = htons(port);

    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (const char *)&opt, sizeof(opt)) < 0) {
        LOG_ERR("setsockopt error:%s", strerror(errno));
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int send_socket_init(void)
{
    sendfd = createsndsock(BRO_SND_PORT);
    if (sendfd < 0) {
        LOG_ERR("send socket create failed");
        return FALSE;
    }
    return TRUE;
}

int createrecvsock(int port, struct sockaddr_in *addr)
{
    int fd = -1;
    int val;
    int brodopt;

    memset(addr, 0, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = 0;
    addr->sin_port = htons(port);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        LOG_ERR("socket error:%s", strerror(errno));
        goto END;
    }

    val = fcntl(fd, F_GETFD);
    val |= FD_CLOEXEC;
    if(fcntl(fd, F_SETFD, val) < 0) {
        LOG_ERR("fcntl error:%s", strerror(errno));
        goto END;
    }

    brodopt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (const char*)&brodopt, sizeof(brodopt)) < 0) {
        LOG_ERR("setsockopt error:%s", strerror(errno));
        goto END;
    }

    if (bind(fd, (struct sockaddr *)addr, sizeof(struct sockaddr_in)) < 0) {
        LOG_ERR("bind error:%s", strerror(errno));
        goto END;
    }

    return fd;
END:
    if (fd >= 0) {
        close(fd);
    }
    return -1;
}

int recv_msg(int fd, char *buffer, int size, struct sockaddr_in *addr)
{
    int addrlen;
    int count;

    addrlen = sizeof(struct sockaddr_in);
    count = recvfrom(fd, buffer, size, 0, (struct sockaddr*)addr, (socklen_t *)&addrlen);
    if (count < 0) {
        LOG_ERR("recvfrom error:%s", strerror(errno));
        return -1;
    }

    return count;
}

int is_local_info(const char *buf)
{
    cJSON* pMsg = NULL;
    cJSON* tmpMsg = NULL;
    cJSON* tmpJson = NULL;
    cJSON* pJson;
    char b_mac[MAXMAC];
    int term_num;
    int index;
    int ret = 0;

    pJson = cJSON_Parse(buf);
    if (pJson == NULL) {
        LOG_ERR("pJson is NULL");
        return 0;
    }

    pMsg = cJSON_GetObjectItem(pJson, "k");
    if (pMsg == NULL) {
        LOG_ERR("pMsg is NULL");
        cJSON_Delete(pJson);
        return 0;
    }

    term_num = cJSON_GetArraySize(pMsg);
    for (index = 0; index < term_num; index++) {
        tmpJson = cJSON_GetArrayItem(pMsg, index);
        if (tmpJson) {
            tmpMsg = cJSON_GetObjectItem(tmpJson, "c");
            if (tmpMsg) {
                strcpy(b_mac, tmpMsg->valuestring);
                if (is_local_conf(b_mac)) {
                    ret = 1;
                    break;
                }
            }
        }
    }
    cJSON_Delete(pJson);
    LOG_INFO("ret:%d", ret);

    return ret;
}

int parse_conf_info(const char *buf, EASYDEPLOY *m_easy_deploy)
{
    cJSON* pMsg = NULL;
    cJSON* tmpMsg = NULL;
    cJSON* tmpJson = NULL;
    cJSON* pJson;
    int computer_num;
    int i;
    int local_conf;
    char b_mac[MAXMAC];

    local_conf = 0;
    pMsg = NULL;

    pJson = cJSON_Parse (buf);
    if (pJson)
    {
        pMsg =  cJSON_GetObjectItem ( pJson, "k" );
        if(pMsg){
            computer_num = cJSON_GetArraySize ( pMsg );
            for( i = 0; i < computer_num; i++)
            {
                tmpJson = cJSON_GetArrayItem(pMsg, i);
                tmpMsg = cJSON_GetObjectItem(tmpJson,"c");
                if (tmpMsg) {
                	strcpy(b_mac,tmpMsg->valuestring);
                }

                if(is_local_conf(b_mac)){
                    tmpMsg =  cJSON_GetObjectItem ( tmpJson, "p" );
                    if (tmpMsg) {
                    	strcpy(m_easy_deploy->b_ip,tmpMsg->valuestring);
                    }

                    tmpMsg =  cJSON_GetObjectItem ( tmpJson, "vmip" );
                    if (tmpMsg) {
                    	strcpy(m_easy_deploy->b_vmip,tmpMsg->valuestring);
                    }

                    tmpMsg =  cJSON_GetObjectItem ( tmpJson, "a" );
                    if (tmpMsg) {
                    	strcpy(m_easy_deploy->b_hostname,tmpMsg->valuestring);
                    }
                    local_conf = 1;
                    break;
                }

            }
        }

        memset(m_easy_deploy->b_teacherip,'\0',sizeof(m_easy_deploy->b_teacherip));
        pMsg =  cJSON_GetObjectItem ( pJson, "e" );
        if (pMsg) {
        	strcpy(m_easy_deploy->b_serverip,pMsg->valuestring);
        }

        pMsg =  cJSON_GetObjectItem ( pJson, "x" );
        if (pMsg) {
        	strcpy(m_easy_deploy->b_backserverip,pMsg->valuestring);
        }

        pMsg =  cJSON_GetObjectItem ( pJson, "v" );
        if (pMsg) {
        	strcpy(m_easy_deploy->b_resolution,pMsg->valuestring);
        }

        pMsg =  cJSON_GetObjectItem ( pJson, "n" );
        if (pMsg) {
        	strcpy(m_easy_deploy->b_netmask,pMsg->valuestring);
        }

        pMsg =  cJSON_GetObjectItem ( pJson, "g" );
        if (pMsg) {
        	strcpy(m_easy_deploy->b_gateway,pMsg->valuestring);
        }

        pMsg =  cJSON_GetObjectItem ( pJson, "d" );
        if (pMsg) {
        	strcpy(m_easy_deploy->b_dns,pMsg->valuestring);
        }

        pMsg =  cJSON_GetObjectItem ( pJson, "b" );
        if (pMsg) {
        	strcpy(m_easy_deploy->b_backdns,pMsg->valuestring);
        }

        pMsg =  cJSON_GetObjectItem ( pJson, "q" );
        if (pMsg) {
        	m_easy_deploy->b_setip = pMsg->type;
        } else {
        	m_easy_deploy->b_setip =-1;
        }

        pMsg =  cJSON_GetObjectItem ( pJson, "j" );
        if (pMsg) {
        	m_easy_deploy->b_dhcpip = pMsg->type;
        } else {
        	m_easy_deploy->b_dhcpip = -1;
        }

        pMsg =  cJSON_GetObjectItem ( pJson, "m" );
        if (pMsg) {
        	m_easy_deploy->b_dhcpdns = pMsg->type;
        } else {
        	m_easy_deploy->b_dhcpdns = -1;
        }

        pMsg =  cJSON_GetObjectItem ( pJson, "auto" );
        if (pMsg) {
            if(!strcmp(pMsg->valuestring, "1"))
            {
                m_easy_deploy->b_autopower_enable= 1;
                m_easy_deploy->b_autopower_need_deploy= 1;
            }
            else if(!strcmp(pMsg->valuestring, "0"))
            {
                m_easy_deploy->b_autopower_enable= 0;
                m_easy_deploy->b_autopower_need_deploy= 1;
            }
            else
            {
                m_easy_deploy->b_autopower_need_deploy= 0;
            }
        } else {
        	m_easy_deploy->b_autopower_need_deploy= -1;
        }

        pMsg =  cJSON_GetObjectItem ( pJson, "vmnetmask");
        if (pMsg) {
        	strcpy(m_easy_deploy->b_vmmask, pMsg->valuestring);
        }

        pMsg =  cJSON_GetObjectItem ( pJson, "vmgateway");
        if (pMsg) {
        	strcpy(m_easy_deploy->b_vmgateway, pMsg->valuestring);
        }

        pMsg =  cJSON_GetObjectItem ( pJson, "vmdns");
        if (pMsg) {
        	strcpy(m_easy_deploy->b_vmdns, pMsg->valuestring);
        }

        pMsg =  cJSON_GetObjectItem ( pJson, "vmback_dns");
        if (pMsg) {
        	strcpy(m_easy_deploy->b_vmbackdns, pMsg->valuestring);
        }

        pMsg =  cJSON_GetObjectItem ( pJson, "proxyusemode");
        if (pMsg) {
        	if (strcmp(pMsg->valuestring, PRIVATE_ACCOUNT) == 0) {
        		m_easy_deploy->b_mode = PRI_MODE;
        	} else if (strcmp(pMsg->valuestring, MUTI_ACCOUNT) == 0) {
        		m_easy_deploy->b_mode = MUTI_MODE;
        	} else if (strcmp(pMsg->valuestring, NO_ACCOUNT) == 0) {
        		m_easy_deploy->b_mode = CMN_MODE;
        	} else {
        		m_easy_deploy->b_mode = -1;
        	}
        } else {
        	m_easy_deploy->b_mode = -1;
        }

        pMsg =  cJSON_GetObjectItem ( pJson, "vmdhcpip" );
        if (pMsg) {
        	m_easy_deploy->b_vmdhcpip = pMsg->type;
        } else {
        	m_easy_deploy->b_vmdhcpip = -1;
        }

        pMsg =  cJSON_GetObjectItem ( pJson, "vmdhcpdns" );
        if (pMsg) {
        	m_easy_deploy->b_vmdhcpdns = pMsg->type;
        } else {
        	m_easy_deploy->b_vmdhcpdns = -1;
        }
        //cJSON_Delete ( tmpJson );
        cJSON_Delete ( pJson );
    }

    if (local_conf)
        LOG_INFO("easy_deploy info:%s", buf);

    return local_conf;
}

/**
 * parse serial num from server
 */
int parse_serial_num(const char *buf)
{
    cJSON *pJson = NULL;
    cJSON *pMsg = NULL;
    int serialNum;

    pJson = cJSON_Parse(buf);
    if (pJson == NULL) {
        LOG_ERR("cJSON_Parse, failed");
        return -1;
    }

    pMsg = cJSON_GetObjectItem(pJson, "mac");
    if (pMsg == NULL) {
        LOG_ERR("GetObjectItem, mac failed");
        serialNum = -1;
        goto END;
    }

    if (!strcmp(pMsg->valuestring, local_net.mac_string)) {
        pMsg = cJSON_GetObjectItem(pJson, "num");
        if (pMsg == NULL) {
            LOG_ERR("GetObjectItem, num failed");
            serialNum = 0;
            goto END;
        }
        LOG_INFO("receive serial num :%d", pMsg->valueint);
        serialNum = pMsg->valueint;
    } else {
        serialNum = -1;
    }

END:
    cJSON_Delete(pJson);
    return serialNum;
}

int group_conf(const char *buf)
{
    int ret;
    EASYDEPLOY easy_deploy;

    memset(&easy_deploy, 0, sizeof(easy_deploy));
    ret = parse_conf_info(buf, &easy_deploy);
    if (ret == 1) {
        group_set_conf(&easy_deploy);
        LOG_INFO("conf set buf:%s", buf);
    }

    return 0;
}

int parse_msg_type(const char *parse_msg, int thread_type, const char *buf)
{
    int interval = MSG_COMMON_TIME;
    int msgType = -1;

    if (parse_msg[0] == '\0') {
        return -1;
    }

    switch (thread_type) {
    case THREAD_TYPE_DEAL1:
        if (!strcmp(parse_msg, "si") || !strcmp(parse_msg, "ci")) {
            if (is_local_info(buf)) {
                interval = MSG_CONFIG_TIME;
                msgType  = MSG_CONF_IMPLE;
            }
        } else if (!strcmp(parse_msg, "begin easy deploy")) {
            msgType = MSG_CONF_BEGIN;
        } else if (!strcmp(parse_msg, "serial number")) {
            msgType = MSG_SERI_PARSE;
        } else if (!strcmp(parse_msg, "end serial num")) {
            msgType = MSG_SERI_CLOSE;
        } else if (!strcmp(parse_msg, "end easy deploy")) {
            msgType = MSG_CONF_CLOSE;
        }
        break;
    case THREAD_TYPE_DEAL2:
        if (!strcmp(parse_msg, "scan terminal dev")) {
            msgType = MSG_INFO_UPLOAD;
        }
        break;
    }

    if (msgType != -1) {
        if (!message_time_filter(msgType, interval)) {
            return -1;
        }
        LOG_INFO("parse type:%d, msg:%s", msgType, parse_msg);
    }
    return msgType;
}

int deal_msg(char *buf, int size)
{
    char parse_msg[128];
    cJSON *pJson;
    cJSON *pMsg;
    int type;
    int ret;
    EASYDEPLOY info;

    memset(&info, 0, sizeof(EASYDEPLOY));
    memset(parse_msg, 0, sizeof(parse_msg));
    pJson = cJSON_Parse(buf);
    if (pJson) {
        pMsg = cJSON_GetObjectItem(pJson, "msg");
        if (pMsg == NULL) {
            cJSON_Delete(pJson);
            return -1;
        }
        snprintf(parse_msg, sizeof(parse_msg), "%s", pMsg->valuestring);
        cJSON_Delete(pJson);
    }

    type = parse_msg_type(parse_msg, THREAD_TYPE_DEAL1, buf);
    switch (type) {
    case MSG_CONF_BEGIN:
        logic_event_handle(UI_SHOW_SERIAL_BEGIN, NULL);
        break;
    case MSG_SERI_PARSE:
        ret = parse_serial_num(buf);
        if (ret > 0) {
            logic_event_handle(UI_SHOW_SERIAL_SUCCESS, &ret);
        } else if (ret == 0) {
            logic_event_handle(UI_SHOW_SERIAL_ERROR, NULL);
        }
        break;
    case MSG_SERI_CLOSE:
        logic_event_handle(UI_SHOW_SERIAL_CLOSE, NULL);
        break;
    case MSG_CONF_IMPLE:
        ret = parse_conf_info(buf, &info);
        if (ret) {
            logic_event_handle(LOGIC_SET_TERM_CONF, &info);
        }
        break;
    case MSG_CONF_CLOSE:
        logic_event_handle(LOGIC_CLR_LAST_TIME, NULL);
        break;
    default:
        break;
    }

    return 0;
}

int deal_msg2(char *buf, int size)
{
    char parse_msg[128];
    cJSON* pJson;
    cJSON* pMsg;
    int type;

    memset(parse_msg, 0, sizeof(parse_msg));
    pJson = cJSON_Parse(buf);
    if (pJson) {
        pMsg = cJSON_GetObjectItem (pJson, "msg");
        if (pMsg == NULL) {
            cJSON_Delete(pJson);
            return -1;
        }
        snprintf(parse_msg, sizeof(parse_msg), "%s", pMsg->valuestring);
        cJSON_Delete(pJson);
    }

    type = parse_msg_type(parse_msg, THREAD_TYPE_DEAL2, buf);
    switch (type) {
    case MSG_INFO_UPLOAD:
        send_config_info();
        break;
    default:
        break;
    }

    return 0;
}

gpointer recvbcast_thread()
{
    int ret;
    int recbcast_sk;
    char *buf;
    int count;

    set_thread_name("conf monitor");

    buf = calloc(MSG_LEN, 1);
    if (buf == NULL) {
        LOG_ERR("calloc error:%s", strerror(errno));
        return NULL;
    }

    recbcast_sk = createrecvsock(BRO_RECV_PORT, &localaddr);
    if (recbcast_sk < 0) {
        LOG_ERR("createrecvsock failed", recbcast_sk);
        free(buf);
        return NULL;
    }

    while (1) {
        memset(buf, 0, MSG_LEN);
        count = recv_msg(recbcast_sk, buf, MSG_LEN, &localaddr);
        if (count <= 0) {
            sleep(2);
            continue;
        }
        //LOG_INFO("receive buf:%s", buf);
        ret = deal_msg(buf, MSG_LEN);
        if (ret < 0) {
            LOG_ERR("deal msg error");
        }
    }

    close(recbcast_sk);
    free(buf);

    return NULL;
}

/**
 * thread for receive scan signal
 */
gpointer scanbcast_thread()
{
    int ret;
    int recbcast_sk;
    char *buf;
    int count;

    set_thread_name("scan monitor");

    buf = calloc(MSG_LEN, 1);
    if (buf == NULL) {
        LOG_ERR("calloc error:%s", strerror(errno));
        return NULL;
    }

    recbcast_sk = createrecvsock(BRO_SCAN_PORT, &localaddr2);
    if (recbcast_sk < 0) {
        LOG_ERR("createrecvsock failed", recbcast_sk);
        free(buf);
        return NULL;
    }

    while (1) {
        memset(buf, 0, MSG_LEN);
        count = recv_msg(recbcast_sk, buf, MSG_LEN, &localaddr2);
        if (count <= 0) {
            sleep(2);
            continue;
        }
        //LOG_INFO("receive buf:%s", buf);
        ret = deal_msg2(buf, MSG_LEN);
        if (ret < 0) {
            LOG_ERR("deal msg error");
        }
    }

    close(recbcast_sk);
    free(buf);

    return NULL;
}

int recv_thread_init(void)
{
    int ret;

    ret = pthread_create(&recv_tid, NULL, recvbcast_thread, NULL);
    if(ret != 0) {
        LOG_ERR("pthread create failed:%s", ret);
        return FALSE;
    }
    return TRUE;
}

int scan_thread_init(void)
{
    int ret;

    ret = pthread_create(&scan_tid, NULL, scanbcast_thread, NULL);
    if(ret != 0) {
        LOG_ERR("pthread create failed:%s", ret);
        return FALSE;
    }
    return TRUE;
}

int send_msg(int sockfd, const char* buf, int len)
{
    int return_value = 0;
    int pos = 0;
    int send_length = 0;

    while (pos < len) 
    {
        send_length = sendto(sockfd, buf + pos, len - pos, 0, (struct sockaddr*)&bcastaddr, sizeof(bcastaddr));
        if(send_length < 0)
        {
            LOG_WARNING("mina send data ret <= 0, ret = %d, errno = %d", send_length, errno);
            switch(errno)
            {
                case EPIPE:
                {
                    return_value = FALSE;
                    goto quit;
                }
                case EINTR:
                {
                    continue;
                }
                case EAGAIN:
                {
                    continue;
                }
                default:
                {
                    return_value = FALSE;
                    goto quit;
                }
            }
        }
        pos += send_length;
    }
    return_value = TRUE;

quit:
    return return_value;
}

void create_jsonmsg(send_msg_info_t *msg_info, char **buf)
{
    cJSON *fmt;
    fmt=cJSON_CreateObject();
    cJSON_AddStringToObject(fmt, "DNSAddr1",     msg_info->dns.dnsaddr1);
    cJSON_AddStringToObject(fmt, "DNSAddr2",     msg_info->dns.dnsaddr2);
    cJSON_AddStringToObject(fmt, "DNSType",      msg_info->dns.dnstype);

    cJSON_AddStringToObject(fmt, "RCDSerIp",     msg_info->rcdserip);
    cJSON_AddStringToObject(fmt, "RCDSlaveSerIp", msg_info->rcdslaveserip);
    cJSON_AddNumberToObject(fmt, "ScBitsPixel",  msg_info->display.bitspixel);
    cJSON_AddNumberToObject(fmt, "ScHeight",     msg_info->display.height);
    cJSON_AddNumberToObject(fmt, "ScVRefresh",   msg_info->display.refresh);
    cJSON_AddNumberToObject(fmt, "ScWidth",      msg_info->display.width);
    cJSON_AddStringToObject(fmt, "gateway",      msg_info->net.gateway);

    cJSON_AddStringToObject(fmt, "ifVer",        msg_info->net.ifVer);
    cJSON_AddStringToObject(fmt, "ipAddr",       msg_info->net.ip);

    cJSON_AddStringToObject(fmt, "ipMask",       msg_info->net.mask);
    cJSON_AddStringToObject(fmt, "ipType",       msg_info->net.ipType);
    cJSON_AddStringToObject(fmt, "macAddr",      msg_info->net.mac_string);

    cJSON_AddStringToObject(fmt, "msg",          msg_info->msg);
    cJSON_AddStringToObject(fmt, "msgId",        msg_info->msgid);
    cJSON_AddStringToObject(fmt, "ntMacAddr",    msg_info->net.mac_string);

    cJSON_AddStringToObject(fmt, "ntType",       msg_info->net.ntType);

    cJSON_AddNumberToObject(fmt, "seqNum",       msg_info->seqnum);
    cJSON_AddStringToObject(fmt, "winHostName",  msg_info->hostname);
    cJSON_AddStringToObject(fmt, "RCCTeacherIp", msg_info->rccteacherip);
    cJSON_AddNumberToObject(fmt, "role",         msg_info->role);

    cJSON_AddStringToObject(fmt,"SN",  msg_info->sn);
    switch(msg_info->devmod) {
        case PRI_MODE:
            cJSON_AddStringToObject(fmt, "proxyusemode", PRIVATE_ACCOUNT);
            break;
        case MUTI_MODE:
            cJSON_AddStringToObject(fmt, "proxyusemode", MUTI_ACCOUNT);
            break;
        case CMN_MODE:
            cJSON_AddStringToObject(fmt, "proxyusemode", NO_ACCOUNT);
            break;
        default:
            break;
    }

    cJSON_AddStringToObject(fmt, "vmip",       msg_info->vmnet.ip);
    cJSON_AddStringToObject(fmt, "vmnetmask",  msg_info->vmnet.mask);
    cJSON_AddStringToObject(fmt, "vmgateway",  msg_info->vmnet.gateway);
    cJSON_AddStringToObject(fmt, "vmdns",      msg_info->vmdns.dnsaddr1);
    cJSON_AddStringToObject(fmt, "vmback_dns", msg_info->vmdns.dnsaddr2);
    cJSON_AddStringToObject(fmt, "vmdhcpip",   msg_info->vmnet.ipType);
    cJSON_AddStringToObject(fmt, "vmdhcpdns",  msg_info->vmdns.dnstype);

    *buf = cJSON_Print(fmt);
    cJSON_Delete(fmt);
    return;
}

int create_info_msg(char **msg)
{
    IniFile* ini;
    char *tmp;
    send_msg_info_t msg_info;

    memset(&msg_info, 0, sizeof(msg_info));
    if (UpdateNetworkInfo(&local_net, &local_dns) < 0) {
        LOG_ERR("get netwoking fail");
    }
    updata_vm_network(&vm_net, &vm_dns);
    get_disply_info(&(local_disply.width), &(local_disply.height), &(local_disply.refresh));
    get_hostname(msg_info.hostname, sizeof(msg_info.hostname));
    get_serialnum(msg_info.sn, sizeof(msg_info.sn)-1);
    get_dev_mode(&(msg_info.devmod));
    strncpy(msg_info.msg, "rain info", sizeof(msg_info.msg) - 1);
    strncpy(msg_info.msgid, "0xdeaddead", sizeof(msg_info.msgid) - 1);

    msg_info.role = 0;
    msg_info.seqnum = 1;
    msg_info.net = local_net;
    msg_info.dns = local_dns;
    msg_info.display = local_disply;
    msg_info.vmnet = vm_net;
    msg_info.vmdns = vm_dns;

    if(access(MINA_CONFIG, F_OK) == 0)
    {
        ini = ini_file_open_file(MINA_CONFIG);
        if (ini != NULL) {
            ini_file_read_string(ini, "default", "server_ip", &tmp);
            if (tmp) {
                strncpy(msg_info.rcdserip, tmp, sizeof(msg_info.rcdserip)-1);
                g_free(tmp);
            }
            ini_file_free(ini);
        }
    }
    msg_info.role = 0;
    create_jsonmsg(&msg_info, msg);
    //LOG_INFO("json msg:%s", *msg);

    return 0;
}

void create_mac_msg(char **msg)
{
    cJSON *fmt;

    fmt = cJSON_CreateObject();
    cJSON_AddStringToObject(fmt, "msg", "stu mac");
    cJSON_AddStringToObject(fmt, "mac", local_net.mac_string);
    *msg = cJSON_Print(fmt);
    cJSON_Delete(fmt);
    LOG_INFO("send json msg:%s", *msg);

    return;
}

void *send_config_info(void)
{
    char *msg_buf = NULL;
    create_info_msg(&msg_buf);
    send_msg(sendfd, msg_buf, strlen(msg_buf));
    if (msg_buf)
        free(msg_buf);
    return NULL;
}

void *send_mac_info(void *data)
{
    pthread_detach(pthread_self());

    char *msg_buf = NULL;
    int index;
    int ret;

    create_mac_msg(&msg_buf);
    for (index = 0; index < 3; index++) {
        ret = logic_event_handle(LOGIC_SEND_DEV_MAC, msg_buf);
        if (!ret) {
            break;
        }
        sleep(0.2);
    }

    if (msg_buf)
        free(msg_buf);

    return NULL;
}

int send_thread_init(void)
{
    pthread_t send_tid;
    int ret;

    ret = pthread_create(&send_tid, NULL, send_mac_info, NULL);
    if(ret != 0) {
        LOG_ERR("pthread create failed:%s", ret);
    }
    return FALSE;
}

int main(int argc, char *argv[])
{
    arg.argc = argc;
    arg.argv = argv;

    log_init();
    msg_timer_init();
    system_info_init();

    if (!send_socket_init())
        goto END;

    if (!recv_thread_init())
        goto END;

    if (!scan_thread_init())
        goto END;

    pthread_join(recv_tid, NULL);
    pthread_join(scan_tid, NULL);

END:
    sleep(2);
    return 0;
}
