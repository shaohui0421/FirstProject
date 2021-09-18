#include <dirent.h>
#include <regex.h>
#include "common.h"
#include "net_api.h"
#include "application.h"

static int exec_cmd(char *cmd, char *line_buf, int size)
{
    FILE *fp;
    char *last_char;

    fp = popen(cmd,"r");
    if(fp == NULL)
    {
    	LOG_ERR("cmd %s err(%s).\n", cmd, strerror(errno));
        return -1;
    }

    if (line_buf != NULL) {
    	memset(line_buf, 0, size);
    	if (fgets(line_buf, size-1, fp) == NULL) {
    		goto end;
    	}
    	if (strlen(line_buf) > 0) {
    		last_char = line_buf + strlen(line_buf) - 1;
    		if (*last_char == '\n' || *last_char == '\r') {
    			*last_char = '\0';
    		}
    	}
    }
end:
	pclose(fp);
	return 0;
}

int get_random()
{
	srand(time(0));
	return rand();
}

/*
 * function:regex_match
 * Description: Match regex string
 * Input:  input_str    input string
 *           regex_str   regex string
 * Return: 0     Match OK
 * 		  else Match Fail
 */
int regex_match(const char *input_str, const char *regex_str)
{
    int status;
    int cflags = REG_EXTENDED;
    regmatch_t pmatch[1];
    const size_t nmatch = 1;
    regex_t reg;

    if (input_str == NULL || regex_str == NULL) {
        return -1;
    }
      
    regcomp(&reg, regex_str, cflags);
    status = regexec(&reg, input_str, nmatch, pmatch, 0);
    regfree(&reg);
    return status;
}

void get_product_id(char *buf, int size)
{
	int ret;
	char cmd[64];
	memset(buf, 0, size);

	snprintf(cmd, sizeof(cmd), "%s", "dmidecode -s baseboard-product-name");
	ret = exec_cmd(cmd, buf, size);
	if (ret < 0) {
		return;
	}
	return;
}

static int check_data_disk_path()
{
    char command[256];

    sprintf(command, "[ ! -d %s ] && mkdir -p %s", RCC_DATA_PATH, RCC_DATA_PATH);
    rc_system(command);
    sprintf(command, "[ ! -d %s ] && mkdir -p %s", RCC_DATA_PATH_DIFF, RCC_DATA_PATH_DIFF);
    return rc_system(command);
}
static int check_usb_policy_path()
{
    char command[256];

    sprintf(command, "[ ! -d %s ] && mkdir -p %s", USB_POLICY_PATH, USB_POLICY_PATH);
    return rc_system(command);
}
/**
 * Backup and rename ini file
 */
static void ini_bak(const char * ininame)
{
    char *ini_file_name = NULL;
    char *ini_bak_name = NULL;
    char *ini_bak_cmd = NULL;
    int len;

    LOG_DEBUG("backup ini file: %s", ininame);

    len = strlen(ininame);
    ini_file_name = (char*)malloc(len+1);
    if (!ini_file_name) {
        LOG_ERR("ini_file_name:memory allocation failure");
        return;
    }

    ini_bak_name  = (char*)malloc(len+30);
    if (!ini_bak_name) {
        free(ini_file_name);
        LOG_ERR("ini bak: memory allocation failure");
        return;
    }

    ini_bak_cmd   = (char*)malloc(len*2+40);
    if (!ini_bak_cmd) {
        free(ini_file_name);
        free(ini_bak_name);
        LOG_ERR("ini_bak_cmd: memory allocation failure");
        return;
    }

    // get ini bak file name
    strncpy(ini_file_name, ininame, strlen(ininame)-4);
    ini_file_name[strlen(ininame)-4] = '\0';
    sprintf(ini_bak_name, "%s_bak.ini", ini_file_name);
    
    // backup ini file
    LOG_WARNING("create ini bak file: %s", ini_bak_name);
    sprintf(ini_bak_cmd, "cp %s %s", ininame, ini_bak_name);
    rc_system(ini_bak_cmd);

    free(ini_file_name);
    free(ini_bak_name);
    free(ini_bak_cmd);
}

/**
 * Remove blanks at the beginning and the end of a string.
 */
static unsigned strstrip(char * s)
{
    char *last = NULL ;
    char *dest = s;

    if (s==NULL) return 0;

    last = s + strlen(s);
    while (isspace((int)*s) && *s) s++;
    while (last > s) {
        if (!isspace((int)*(last-1)))
            break ;
        last -- ;
    }
    *last = (char)0;

    memmove(dest,s,last - s + 1);
    return last - s;
}

/**
 * Check validity of ini line.
 */
static int ini_check_line(const char * input_line, char * key, char * value)
{
    char * line = NULL;
    size_t len;
    int ret = 0;

    line = strdup(input_line);
    len = strstrip(line);

    if (len<1) {
        // Empty line
    } else if (line[0]=='#' || line[0]==';') {
        // Comment line
    } else if (line[0]=='[' && line[len-1]==']') {
        // Section line
    } else if (sscanf(line, "%[^=] = %[^;#]", key, value) == 2) {
        // key = value
        //LOG_DEBUG("ini check line: key = %s\n", key);
        //LOG_DEBUG("ini check line: val = %s\n", value);
        if (strchr(key, ':') != NULL) {
            ret = -1;
        }
    } else if (sscanf(line, "%[^=] = %[;#]", key, value) == 2
           ||  sscanf(line, "%[^=] %[=]", key, value) == 2) {
        // Special cases:
        // key = ;
        // key = #
        // key =    
        //LOG_DEBUG("ini check line: key = %s", key);
        if (strchr(key, ':') != NULL) {
            ret = -1;
        }
    } else {
        // syntax error
        ret = -1;
    }

    free(line);
    return ret;
}

/**
 * Get rid of invalid line in ini file.
 */
static void ini_check(const char * ininame)
{
#define LINE_MAX_SZ      (2048)
#define __INI_TEMP_PATH  ".ini_temp"
    FILE* fp_in;
    FILE* fp_temp;
    char line[LINE_MAX_SZ+1];
    char key[LINE_MAX_SZ+1];
    char value[LINE_MAX_SZ+1];
    char cmd[64];

    int last = 0;
    int len;
    int lineno = 0;
    int errs = 0;
    int toolong_flag = 0;

    //LOG_DEBUG("ini check: %s", ininame);
    if ((fp_in = fopen(ininame, "r"))==NULL) {
        LOG_ERR("ini check: cannot open %s !", ininame);
        return ;
    }
    if ((fp_temp = fopen(__INI_TEMP_PATH, "w"))==NULL) {
        fclose(fp_in);
        LOG_ERR("ini check: cannot open %s !", __INI_TEMP_PATH);
        return ;
    }

    memset(line, 0, LINE_MAX_SZ);
    last=0;

    while (fgets(line+last, LINE_MAX_SZ-last, fp_in)!=NULL) {
        lineno++;
        len = (int)strlen(line)-1;
        if (len<0) {
            continue;
        }
        // Safety check against buffer overflows
        if (line[len]!='\n' && !feof(fp_in)) {
            toolong_flag = 1;
            lineno--;
            continue;
        }
        if (toolong_flag == 1) {
            LOG_ERR("ini check: input line too long in %s (%d)", ininame, lineno);
            toolong_flag = 0;
            errs++;
            memset(line, 0, LINE_MAX_SZ);
            last=0;
            continue;
        }
        // Get rid of \n and spaces at end of line
        while ((len>=0) && ((line[len]=='\n') || (isspace(line[len])))) {
            line[len]=0 ;
            len--;
        }
        if (len < 0) { // Line was entirely \n and/or spaces
            len = 0;
        }
        // Detect multi-line
        if (line[len]=='\\') {
            last=len;
            continue ;
        } else {
            last=0;
        }
        // Check ini line
        //LOG_DEBUG("check ini line: %s", line);
        if (ini_check_line(line, key, value) < 0) {
            LOG_ERR("ini check: syntax error in %s (%d)", ininame, lineno);
            errs++;
        } else {
            fputs(line, fp_temp);
            fputc('\n', fp_temp);
        }
        memset(line, 0, LINE_MAX_SZ);
        last=0;
    }

    fflush(fp_temp);
    fsync(fileno(fp_temp));
    fclose(fp_in);
    fclose(fp_temp);
    
    //LOG_DEBUG("ini check errs: %d", errs);
    if (errs > 0) {
        ini_bak(ininame);
        sprintf(cmd, "mv %s %s", __INI_TEMP_PATH, ininame);
        rc_system(cmd);
    } else {
        sprintf(cmd, "rm %s", __INI_TEMP_PATH);
        rc_system(cmd); 
    }
#undef LINE_MAX_SZ
#undef __INI_TEMP_PATH
}

static int ends_with(const char *str, const char *suffix)
{
    if (!str || !suffix)
    {
        return 0;
    }
    size_t len_str = strlen(str);
    size_t len_suffix = strlen(suffix);
    if (len_suffix > len_str)
    {
        return 0;
    }
    return (strncmp(str + len_str - len_suffix, suffix, len_suffix) == 0);
}

/**
 * check validity of all ini file
 */
static void check_ini_file()
{
    DIR* pdir;
    struct dirent *pfile;
    char filename[PATH_MAX];

    pdir = opendir(RCC_DATA_PATH);
    if (!pdir)
    {
        LOG_ERR("cannot open %s, errno = %d, strerror: %s", RCC_DATA_PATH, errno, strerror(errno));
        return;
    }
    while ((pfile = readdir(pdir)) != NULL)
    {
        sprintf(filename, "%s%s", RCC_DATA_PATH, pfile->d_name);
        if (ends_with(filename, "_bak.ini"))
        {
            // backup of ini file
            continue;
        }
        else if (ends_with(filename, ".ini"))
        {
            ini_check(filename);
        }
    }
    closedir(pdir);
}

static void clear_net_socket()
{
    //FIXME: according to pid
    rc_system("rm /tmp/socket_client* -rf");
}

static void clean_wlan_roamconfig()
{
    if(!get_file_exist("/opt/lessons/wpa_supplicant/roamconfig.ini_"))
    {

        LOG_INFO("clean_wlan_roamconfig");
        rc_system("sed -i 's/roamlevel=90/roamlevel=0/g' /etc/wpa_supplicant/roamconfig.ini");
        rc_system("cp -rf /etc/wpa_supplicant/roamconfig.ini /opt/lessons/wpa_supplicant/roamconfig.ini_");
        rc_system("sync");
    }
}
static void start_halo()
{
    rc_system("test -x /usr/local/bin/oaHalo/halo_module_daemon.sh && bash /usr/local/bin/oaHalo/halo_module_daemon.sh &");
}
int get_dir_all_space(const char * dir, long * size, const char * unit)
{
	char kblock[15];
	char cmd[64];
	int ret;
	memset(kblock, 0, sizeof(kblock));

	snprintf(cmd, sizeof(cmd)-1, "df %s | grep /dev/ | awk '{print $2}'", dir);
	ret = exec_cmd(cmd, kblock, sizeof(kblock));
	if (ret < 0) {
		return -1;
	}


	if (unit && (strcasecmp(unit, "k") == 0)) {
		*size = atol(kblock);
	} else if (unit && (strcasecmp(unit, "m") == 0)) {
		*size = atol(kblock) / 1024;
	} else {
		*size = atol(kblock) / (1024*1024);
	}
	return 0;
}

int get_data_part_all_space(long * size)
{
    DiskPartInfo disk_part_info;

    Application *app = Application::get_application();
    if (app == NULL) {
        LOG_ERR("app is NULL!");
        return -1;
    }

    app->get_device_interface()->getDiskPartInfo("data", disk_part_info);
    *size = disk_part_info.total / (1024*1024*1024);
    LOG_INFO("get_data_part_all_space size = %d", *size);

    return 0;
}

int get_disk_status(string * status)
{
    char command[128];
    char result[128];
    int result_size = sizeof(result);
    int ret = 0;

    if(!get_file_exist("/etc/diskstatus_mgr.bash"))
    {
        LOG_INFO("cancel disk recover function if /etc/diskstatus_mgr.bash not existed");
        return 0;
    }

    memset(result, 0, sizeof(result));
    sprintf(command, "bash /etc/diskstatus_mgr.bash --status");
    ret = rc_system_rw(command, (unsigned char*)result, &result_size, "r");

    // del linefeeds at end of result
    if (strlen(result) > 0 && result[strlen(result)-1] == '\n')
    {
        result[strlen(result)-1] = '\0';
    }
    LOG_INFO("get disk status: ret = %d, result = %s", ret, result);
    *status = result;
    return ret;
}

bool get_file_exist(const char * filename)
{
    struct stat file_stat;
    int ret = 0;
    
    errno = 0;
    ret = stat(filename, &file_stat);
    if(ret == 0)
    {
        return true;
    }
    else if(errno == ENOENT)
    {
        return false;
    }
    else
    {
        LOG_WARNING("stat return != 0, but errno != ENOENT, errno = %d, strerror:%s", errno, strerror(errno));
        return false;
    }
}

void set_non_blocking(int fd)
{
    int flags;
    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        LOG_WARNING("failed to set socket non block: errno = %d, strerror:%s", errno, strerror(errno));
    }

    if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        LOG_WARNING("failed to set socket non block: errno = %d, strerror:%s", errno, strerror(errno));
    }
}

void set_blocking(int fd)
{
    int flags;
    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        LOG_WARNING("failed to clear socket non block: errno = %d, strerror:%s", errno, strerror(errno));
    }

    if (fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) == -1) {
        LOG_WARNING("failed to clear socket non block: errno = %d, strerror:%s", errno, strerror(errno));
    }
}

unsigned long long get_monolithic_time()
{
    struct timespec time_space;
    clock_gettime(CLOCK_MONOTONIC, &time_space);
    return uint64_t(time_space.tv_sec) * 1000 * 1000 * 1000 + uint64_t(time_space.tv_nsec);
}

unsigned long long ntohll(unsigned long long val)
{
    if (__BYTE_ORDER == __LITTLE_ENDIAN)
    {
        return (((unsigned long long )htonl((int)((val << 32) >> 32))) << 32) | (unsigned int)htonl((int)(val >> 32));
    }
    else if (__BYTE_ORDER == __BIG_ENDIAN)
    {
        return val;
    }
}

unsigned long long htonll(unsigned long long val)
{
    if (__BYTE_ORDER == __LITTLE_ENDIAN)
    {
        return (((unsigned long long )htonl((int)((val << 32) >> 32))) << 32) | (unsigned int)htonl((int)(val >> 32));
    }
    else if (__BYTE_ORDER == __BIG_ENDIAN)
    {
        return val;
    }
}

int connect_with_timeout(int sock_fd, const string& ip, int port, struct timeval *timeout)
{
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_addr.s_addr = inet_addr(ip.c_str());
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    
    set_non_blocking(sock_fd);
    int connected = connect(sock_fd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr_in));
    int ret = -1;
    if (connected != 0)
    {
        if(errno != EINPROGRESS)
        {
            LOG_PERROR("connect error");
        }
        else
        {
            fd_set wset,rset;
            FD_ZERO(&wset);
            FD_ZERO(&rset);
            FD_SET(sock_fd,&wset);
            FD_SET(sock_fd,&rset);
            int res = select(sock_fd+1,&rset,&wset,NULL,timeout);
            if(res < 0)
            {
                LOG_PERROR("network error in connect");
            }
            else if(res == 0)
            {
                LOG_INFO("connect time out");
            }
            else if (1 == res)
            {
                if(FD_ISSET(sock_fd,&wset))
                {
                    LOG_INFO("connect succeed.");
                    set_blocking(sock_fd);
                    ret = 0;
                }
                else
                {
                    LOG_PERROR("other error when select");
                }  
            }
        }
    }
    return ret; 
}

int get_submask_bitcount(const char *input)
{
    int i, n;
    int bits;
    struct in_addr addr;

    n = 0;
    bits = sizeof(u_int32_t) * 8;
    if (inet_aton(input, &addr) == 0) {
        LOG_WARNING("netmask is invalid!\n");
        return -1;
    }
    for (i = bits - 1; i >= 0; i--) {
        if ( addr.s_addr & (0x01 << i)) {
            n++;
        }
    }
    return n;
}

void pre_check()
{
    check_data_disk_path();
    check_usb_policy_path();
    check_ini_file();
    clear_net_socket();
    clean_wlan_roamconfig();
    start_halo();
}

#if 0
bool is_wifi_terminal()
{
    static std::list<string> wifi_terminal_list;
    std::list<string>::iterator iter;
    static bool inited = false;

    string product_id = _app->get_basic_info().product_id;

    //init wifi_terminal_list
    if(!inited)
    {
        wifi_terminal_list.push_back("0x80060023");  //300W V2
        wifi_terminal_list.push_back("0x80060017");  //310W V2
        wifi_terminal_list.push_back("0x80060042");  //400W V2
        wifi_terminal_list.push_back("0x80060051");  //320W V1.00
        inited = true;
    }

    for(iter = wifi_terminal_list.begin(); iter != wifi_terminal_list.end(); ++iter)
    {
        if (product_id == *iter) {
            return true;
        }
    }
    return false;
}
#endif

bool is_wifi_terminal()
{
    return net::NetworkManager::checkWifiRequirement();
}

bool is_support_dot1x()
{
    return true;
}

/**
 * get auto boot command: rw_bios_nv_user.
 * ac:0  means auto boot if power up
 * ac:1  means not auto boot
 */
int get_power_boot(int* power_boot)
{
    static bool inited = false;
    char cmd[128], data[128];
	int datalen = sizeof(data);
    int ret = 0;

    if (!inited)
    {
        memset(cmd, 0, sizeof(cmd));
        memset(data, 0, sizeof(data));
        sprintf(cmd, "cd /usr/local/bin; rw_bios_nv_user | awk '{print $1}' | awk -F ':' '{print $2}'");
    	ret = rc_system_rw(cmd, (unsigned char *)data, &datalen, "r");
        if (ret != 0)
        {
            LOG_ERR("execute rw_bios_nv_user err!");
            return ret;
        }
        // del linefeeds at end of result
        if (strlen(data) > 0 && data[strlen(data)-1] == '\n')
        {
            data[strlen(data)-1] = '\0';
        }

        LOG_INFO("get_power_boot data: %s", data);
        if (strcmp(data, "0") == 0) {
            *power_boot = 1;
        } else {
            *power_boot = 0;
        }
        inited = true;
    }
    
    LOG_INFO("get_power_boot: %d", *power_boot);
    return ret;
}

/**
 * set auto boot
 * auto boot:      rw_bios_nv_user 0 0 0 0 0 1
 * not auto boot: rw_bios_nv_user 1 0 0 0 0 1
 */
int set_power_boot(int power_boot)
{
    char cmd[128], data[128];
	int datalen = sizeof(data);
    int ret;
    int power[6] = {0};

    memset(cmd, 0, sizeof(cmd));
    memset(data, 0, sizeof(data));
    sprintf(cmd, "cd /usr/local/bin; rw_bios_nv_user | sed 's/[a-z:]//g'");
    ret = rc_system_rw(cmd, (unsigned char *)data, &datalen, "r");
    if (ret != 0)
    {
        LOG_ERR("execute rw_bios_nv_user err!");
        return ret;
    }
    sscanf(data, "%d %d %d %d %d %d", &power[0], &power[1], &power[2], &power[3], &power[4], &power[5]);
    power[0] = ((power_boot == 1) ? 0 : 1);

    LOG_INFO("rw_bios_nv_user %d %d %d %d %d %d", power[0], power[1], power[2], power[3], power[4], power[5]);    

    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "cd /usr/local/bin; rw_bios_nv_user %d %d %d %d %d %d", power[0], power[1], power[2], power[3], power[4], power[5]);    
    ret = rc_system(cmd);
    if (ret != 0)
    {
        LOG_ERR("execute rw_bios_nv_user err!");
        return ret;
    }

    LOG_INFO("set_power_boot: %d", power_boot);
    return ret;
}

/**
* function : change str val hex to  dec
* pChar：hex str lenth: str lenth most for 2 byte
*
* return: change value
*/
unsigned short GetHexStrVal(const string &hex_str)
{
    unsigned short val = 0;
    int i = 0;
    int length = hex_str.length();
    char c;

    for (i = 0; i < length; i++) {
        c = hex_str[i];
        if ((c >= '0' && c <= '9')) {
            val *= 16;
            val += (c - '0');
        } else if (c >= 'a' && c <= 'f') {
            val *= 16;
            val += (c - 'a') + 10;
        } else if ( c >= 'A' && c <= 'F' ) {
            val *= 16;
            val += (c - 'A') + 10;
        }
    }
    return val;
}

int get_file_md5_key(const char *file_path, char *md5_key)
{
    char cmd[128] = {0};
    int datalen = 32;
    int ret = -1;

    if (md5_key == NULL) {
        LOG_ERR("md5_key error!");
        return ret;
    }

    if (!get_file_exist(file_path)) {
        LOG_INFO("file:%s is not exist!", file_path);
        return ret;
    }

    sprintf(cmd, "md5sum %s | awk '{print $1}'", file_path);
    ret = rc_system_rw(cmd, (unsigned char *)md5_key, &datalen, "r");
    if (ret != 0) {
        LOG_ERR("execute md5sum error!");
        return ret;
    }

    LOG_INFO("file:%s, md5:%s", file_path, md5_key);

    return 0;
}

