#ifndef _COMMON_H
#define _COMMON_H

#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <endian.h>
#include <sys/socket.h>
#include <memory>
#include "rc/rc_public.h"
#include "cJSON.h"
#include "data_struct.h"
#include "iniparser/iniparser.h"
#include "rc/rc_systeminfo.h"
#include "rc/rc_netif.h"
#include "thread.h"
#define INFINITE -1

#undef	MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#undef	MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#undef	ABS
#define ABS(a)     (((a) < 0) ? -(a) : (a))

using std::string;

#define RCC_DEFAULT_IFNAME "br0"
#define RCC_DEFAULT_IFNAME2 "br1"

int get_random();
int regex_match(const char *input_str, const char *regex_str);
void pre_check();
int get_disk_status(string * status);
bool get_file_exist(const char* filename);
void set_non_blocking(int fd);
void set_blocking(int fd);
unsigned long long get_monolithic_time();
unsigned long long ntohll(unsigned long long val);
unsigned long long htonll(unsigned long long val);
int connect_with_timeout(int sock_fd, const string& ip, int port, struct timeval *timeout);
int get_submask_bitcount(const char *input);
int get_dir_all_space(const char * dir, long * size, const char * unit);
int get_data_part_all_space(long * size);
void get_product_id(char *buf, int size);
bool is_wifi_terminal();
bool is_support_dot1x();
int get_power_boot(int* power_boot);
int set_power_boot(int power_boot);

unsigned short GetHexStrVal(const string &hex_str);
int get_file_md5_key(const char *file_path, char *md5_key);

#define RCO_POOL_NAME                           "pool_idv"
#define RCC_POOL_NAME                           "pool_fat"

#define RCC_LOG_FILEPATH                        "/var/log/rcc_client.log"
#define RCC_LOG_FILE_MAXSIZE                    (32 << 20)
#define RCC_LOG_DEFAULT_LOGLEVEL                (USER_INFO)

#define PRINTER_DISK_NAME                       "w.disk"
#define PRINTER_DISK_SIZE                       (1024) // 1G

#define RCC_SYS_PATH                            "/etc/RCC_Client/"
#define RCC_DATA_PATH                           "/opt/lessons/RCC_Client/"
#define RCC_DATA_PATH_DIFF                      "/opt/lessons/diff"
#define RCC_LAYER_PATH                          "/opt/lessons/layer/"
#define RCC_PRINT_PATH                          "/opt/lessons/print/"
#define RCC_SCRIPT_PATH                         RCC_SYS_PATH "scripts/"
#define RCC_LAYER_FILE                          RCC_LAYER_PATH "layer_templete"
#define RCC_PRINT_FILE                          RCC_PRINT_PATH "printtemplete"
#define RCC_DEB_CACHE_PATH                      "/var/cache/apt/archives/"

#define RCC_VERSION_INI_FILENAME                RCC_SYS_PATH "version_client_idv.ini"

#define RCC_SERVERIP_INI_FILENAME               RCC_DATA_PATH "mina.ini"
#define RCC_SERVERIP_DEFAULT                    ""
#define RCC_LOGIC_INI_FILENAME                  RCC_DATA_PATH "logic.ini"
#define RCC_LOGIC_CONFIGURED_INI_FILENAME       RCC_DATA_PATH "logic_configured.ini"
#define RCC_OS_UPGRADE_POLICY_INI               "/etc/RCC_Client_os_upgrade_rule.ini"
#define RCC_DEV_POLICY_INI_FILENAME             RCC_DATA_PATH "dev_policy.ini"
#define RCC_DEV_STATUS_INI_FILENAME             RCC_DATA_PATH "dev_status.ini"
#define RCC_RESERVED_MEMORY_INI_FILENAME        RCC_DATA_PATH "reserved_memory.ini"
#define RCC_NEWDEPLOY_CTRL_INI_FILENAME         RCC_DATA_PATH "newdeploy_ctrl.ini"
#define RCC_AUTH_MGR_INI_FILENAME               RCC_DATA_PATH "auth_mgr.ini"
#define RCC_RESERVED_MEMORY_PASS_DEFAULT        (600)
#define RCC_RESERVED_MEMORY_EMULATION_DEFAULT   (1024)
#define EASY_DEPLOY_INI_FILENAME                "/tmp/easy_deploy.ini"
#define RCC_UPGRADE_LOCK                        RCC_SYS_PATH "upgrade.lock"

#define RCC_USB_FLITER                          RCC_DATA_PATH "RCDUSBFilter"
#define RCC_USB_CONF                            RCC_DATA_PATH "RCDUSBConf"
#define RCD_SPICE_USBINFO_PATH                  "/tmp/newusbdevicefile"

#define VM_NETWORK_INFO_INI_FILENAME             RCC_DATA_PATH "vm_network_info.ini"
#define VM_IMAGE_INFO_INI_FILENAME               RCC_DATA_PATH "vm_image_info.ini"
#define VM_CONFIGURED_INI_FILENAME               RCC_DATA_PATH "vm_configured.ini"

#define USB_POLICY_PATH                          RCC_DATA_PATH "usb_policy/"
#define USB_PUBLIC_POLICY                        USB_POLICY_PATH "public_policy"

class IniData
{
public:
    IniData(string inifile): _inifile(inifile){}
    virtual ~IniData() {}
    //set();
    //get();

protected:
    virtual void on_load(){}
    virtual void on_save(){}
    
    void load()
    {
        Lock lock(_mutex);
        d = iniparser_load(_inifile.c_str());
        //may load empty data, so on_save() save default data after on_load()
        on_load();
        on_save();
        iniparser_dump_ini(d, _inifile.c_str());
        iniparser_freedict(d);
    }
    
    void save()
    {
        Lock lock(_mutex);
        d = iniparser_load(_inifile.c_str());
        on_save();
        iniparser_dump_ini(d, _inifile.c_str());
        iniparser_freedict(d);
    }
    Mutex _mutex;
    dictionary *d;
private:
    string _inifile;
};


class InitFile
{
public:
    InitFile(string file, string write_mode = "w+", string read_mode = "r"):_file(file), _write_mode(write_mode), _read_mode(read_mode)
    {
        _file_fd = NULL;
    }

    virtual ~InitFile() {}

    int wite_file(const void *data) {
        int nwrite = 0;

        Lock lock(_mutex);
        if (open_file("write")) {
            nwrite = on_wite_file(data);
            if (nwrite == 0) {
                LOG_WARNING("failed to write %s errno = %d, strerror:%s", _file.c_str(), errno, strerror(errno));
            }

            fclose(_file_fd);
            _file_fd = NULL;
        }
        return nwrite;
    }

    int read_file(const void *buf, int len) {
        int nread = 0;

        Lock lock(_mutex);
        if (open_file("read")) {
            nread = on_read_file(buf, len);
            if (nread == 0) {
                LOG_WARNING("failed to read %s errno = %d, strerror:%s", _file.c_str(), errno, strerror(errno));
            }
            fclose(_file_fd);
            _file_fd = NULL;
        }
        return nread;
    }

    int get_file_length(void) 
    {
        int length = 0, ret;
        struct stat statbuf;

        Lock lock(_mutex);
        ret = stat(_file.c_str(),&statbuf);
        if (ret == -1) {
            LOG_DEBUG("get_file_length errno: %d, strerror: %s", errno, strerror(errno));
            return ret;
        }
        length=statbuf.st_size;
        LOG_INFO("get_file_length: %d", length);

        return length;
    }

    Mutex _mutex;
protected:
    virtual int on_wite_file(const void *data) { return 0; }
    virtual int on_read_file(const void *buf, int len) { return 0; }

    FILE * get_file_fd() { return _file_fd; }

    bool open_file(string open_mode) {
        if (_file.empty()) {
            LOG_INFO("InitFile _file is empty");
            _file_fd = NULL;
            return false;
        } 

        if (_file_fd) {
            LOG_INFO("InitFile file is used");
            return false;
        }

        if (open_mode == "write") {
            _file_fd =  fopen( _file.c_str(), _write_mode.c_str());
        } else if (open_mode == "read") {
            _file_fd =  fopen( _file.c_str(), _read_mode.c_str());
        }

        if (_file_fd == NULL) {
            LOG_ERR("op %s file failed , errno = %d, strerror:%s",  _file.c_str(), errno, strerror(errno));
            return false;
        }
         return true;
    }

private:
    string _file;
    string _write_mode;
    string _read_mode;
    FILE *_file_fd;
};
/**
 * Singleton design pattern; only one instance is allowed.
 */
template <class T> class Singleton
{
public:

    /**
     * Constructor
     * @param instance New instance of T.
     */
    Singleton<T>(T *obj)
    {
        instance = obj;
    }

protected:
    
    /** One and only instance. */
    static T *instance;
};

/* Initialize the static member obj. */
template <class T> T* Singleton<T>::instance = 0;



#endif //_COMMON_H
