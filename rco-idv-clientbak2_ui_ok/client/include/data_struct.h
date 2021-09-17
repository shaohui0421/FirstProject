#ifndef _DATA_STRUCT_H
#define _DATA_STRUCT_H

#include <string>
#include <sstream>
#include <iostream>

using std::string;
using std::stringstream;

enum NetcardSpeed
{
    NETCARD_OFF = 0,
    NETCARD_10M = 10,
    NETCARD_100M = 100,
    NETCARD_1000M = 1000,
};

enum Mode 
{
   SPECIAL_MODE,
   MULTIUSER_MODE,
   PUBLIC_MODE,
};

enum UnknownDev_Data_Structure
{
    UNKNOWN_IDV_LEN = 4,
    UNKNOWN_PID_LEN = 4,
    UNKNOWN_BCD_LEN = 4,
    UNKNOWN_DATA_LEN = 2,
    MANUFACTURER_POS = 0,
    PRODUCT_POS = 1,
    SERIAL_POS =2,
    POS_MAX = 3,
};

struct VersionInfo
{
    VersionInfo()
        :main_version (0)
        ,minor_version (0)
        ,third_version (0)
        ,fourth_version (0)
        ,extra_first ("")
        ,extra_second ("")
        ,build_date ("")
    {
    }
    
    int main_version;
    int minor_version;
    int third_version;
    int fourth_version;
    string extra_first;
    string extra_second;
    string build_date;

    void set_version(const int main_ver, const int minor_ver, const int fourth_ver)
    {
        main_version = main_ver;
        minor_version = minor_ver;
        fourth_version = fourth_ver;
    }

    string print_data() const
    {
        string print;
        stringstream software_stream;
        software_stream << "main_version:"    << main_version     << "|"\
        << "minor_version:"   << minor_version    << "|"\
        << "third_version:"   << third_version    << "|"\
        << "fourth_version:"  << fourth_version   << "|"\
        << "extra_first:"     << main_version     << "|"\
        << "extra_second:"    << extra_second     << "|"\
        << "build_date:"      << build_date;
        software_stream >> print;
        return print;
    }
    bool operator == (const VersionInfo & input) const
    {
        if((main_version    != input.main_version)      ||
           (minor_version   != input.minor_version)     ||
           (fourth_version  != input.fourth_version)    )
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    bool operator != (const VersionInfo & input) const
    {
        if((main_version    != input.main_version)      ||
           (minor_version   != input.minor_version)     ||
           (fourth_version  != input.fourth_version)    )
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    bool operator > (const VersionInfo & input) const
    {
        if (main_version > input.main_version)
        {
            return true;
        }
        else if (main_version < input.main_version)
        {
            return false;
        }
        else
        {
            if (minor_version > input.minor_version)
            {
                return true;
            }
            else if (minor_version < input.minor_version)
            {
                return false;
            }
            else
            {
                if (fourth_version > input.fourth_version)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
    }
};

struct BasicInfo
{
    string serial_number;
    string product_id;
    string product_name;
    string mac;
    string software_version;
    string hardware_version;
    string bios_version;
    string os_version;
    string cpu;
    string memory;
    string storage;
    string mac2;
};

#pragma pack(1)

typedef struct UsbConfigHead_T
{
    char default_allow;
    unsigned short allow_len;
    unsigned short unallow_len;
}UsbConfigHead;

typedef struct UsbConfigInfo_T
{
    UsbConfigHead head;
    unsigned short allow_len;
    unsigned short unallow_len;
    string allow_data;
    string unallow_data;
}UsbConfigInfo;

#pragma pack()

struct NetworkInfo
{
    int netcard_speed;
    
    bool dhcp;
    string ip;
    string submask;
    string gateway;

    bool auto_dns;
    string main_dns;
    string back_dns;
    bool operator == (const NetworkInfo & input) const
    {
        if((dhcp    != input.dhcp)     ||
           (ip      != input.ip)       ||
           (submask != input.submask)  ||
           (gateway != input.gateway)  ||
           (auto_dns!= input.auto_dns) ||
           (main_dns!= input.main_dns) ||
           (back_dns!= input.back_dns)   )
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    bool operator != (const NetworkInfo & input) const
    {
        if((dhcp    != input.dhcp)     ||
           (ip      != input.ip)       ||
           (submask != input.submask)  ||
           (gateway != input.gateway)  ||
           (auto_dns!= input.auto_dns) ||
           (main_dns!= input.main_dns) ||
           (back_dns!= input.back_dns)   )
        {
            return true;
        }
        else
        {
            return false;
        }
    }
};

struct LayerDiskInfo
{
    int layer_disk_number;
    string layer_disk_serial_1;
    string layer_on_1;
    string layer_x64_1;
};

struct DevInterfaceInfo
{
    string interface_info;
    int net_passthrough;
};
struct ImageInfo
{
    //bool recovery;
    int id;
    string name;
    string version;
    string torrent_url;
    string md5;
    string ostype;
    int  real_size;
    int  virt_size;
    int  set_size;
    LayerDiskInfo layer_info;
};

struct PublicInfo
{
    int  outctrl_day;
};

struct PolicyInfo
{
    string usb_policy;
    bool net_policy;
    PublicInfo pub;
};


struct NetdiskInfo
{
    bool netdisk_enable;
    string netdisk_username;
    string netdisk_password;
    string netdisk_ip;
    string netdisk_path;
};

struct UserAuthInfo
{
    string user_auth_type;    //"LOCAL" for normal user, "LDAP" for ldap user, "AD" for ad user
    string ad_domain;         //ad domain address
};

struct UserInfo
{
    string username;
    string password;
    string new_password;
    string pcname;
    string desktop_redir;
    int remember_flag;
    NetdiskInfo netdisk_info;
    int group_id;
    PolicyInfo policy_info;
    UserAuthInfo user_auth_info;
};

struct MainWindowInfo
{
    string pictureMD5;
    string pictureUrl;
    string type;
};

struct PortMappingInfo
{
    int src_port;
    int dst_port;
};

/* idv over wan */
struct HttpPortInfo
{
    int public_port;
    int private_port;
    string public_ip;
    string private_ip;
    /* bt ports range, in the form "xxxx:xxxx" */
    string  btPortsRange;
};

struct ModeInfo
{
	Mode mode;
    UserInfo bind_user;

    bool operator == (const ModeInfo & input) const
    {
        if((mode                != input.mode)              ||
           (bind_user.username  != input.bind_user.username))
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    bool operator != (const ModeInfo & input) const
    {
        if((mode                != input.mode)              ||
           (bind_user.username  != input.bind_user.username))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
};

struct DevPolicyInfo
{
	int allow_recovery;
	int allow_userdisk;
	int allow_guestlogin;
	int allow_netdisk;
	int allow_otherslogin;
	int id;
    bool operator == (const DevPolicyInfo & input) const
    {
        if(input.allow_guestlogin != allow_guestlogin
        || input.allow_netdisk != allow_netdisk
        || input.allow_otherslogin != allow_otherslogin
        || input.allow_recovery != allow_recovery
        || input.allow_userdisk != allow_userdisk
        )
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    bool operator != (const DevPolicyInfo & input) const
    {
    	if(input.allow_guestlogin != allow_guestlogin
    			|| input.allow_netdisk != allow_netdisk
    	        || input.allow_otherslogin != allow_otherslogin
    	        || input.allow_recovery != allow_recovery
    	        || input.allow_userdisk != allow_userdisk
    	        )
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    DevPolicyInfo& operator=(const DevPolicyInfo & other)
    {
        if (this == &other) return *this;

        allow_recovery = other.allow_recovery;
        allow_userdisk = other.allow_userdisk;
        allow_guestlogin = other.allow_guestlogin;
        allow_netdisk = other.allow_netdisk;
        allow_otherslogin = other.allow_otherslogin;
        id = other.id;
        return *this;
    }

};

struct VMInfo
{
    bool running;
    int cpu;    // percentage
    int memory_total;//unit: M
    int memory_use;//unit: M
    int storage_total;//unit: M
    int storage_use;//unit: M
    NetworkInfo vm_net;
};

struct DownloadInfo
{
    int status;
    int image_id;
    string image_name;
    string image_version;
    string percent;
    double percent_double;
    string speed;
};


enum EasyDeployInvalid
{
    EASYDEPLOY_INVALID = -1,
};

struct EasyDeploy
{
    //EASYDEPLOY_INVALID means not set for int
    //empty means not set for string

    bool set_local_net;
    NetworkInfo local_net;

    bool set_vm_net;
    NetworkInfo vm_net;
    
    string server_ip;
    int mode;
    string hostname;

    bool set_autopower;
    int autopower;
};

struct IpxeInfo
{
	string iso_name;
	string iso_version;
};

/* displayinfo information */
struct DisplayInfo
{
    int width;
    int height;
    int flag;    //Resolution level, bit 0 :RES_BEST, bit 1:RES_CURRENT
    int custom;
};

struct ExtDisplayInfo
{
    int width[2];
    int height[2];
    int flag[2];    //Resolution level, bit 0 :RES_BEST, bit 1:RES_CURRENT
    int custom[2];
};

/* diskinfo information */
typedef struct DiskInfo
{
    int size;       //unit : MB
    int is_enable;  // 1: need create expand disk  0: not need create expand disk
    string disk_name;
}DiskInfo_t;

/* unkonw usb dev info */
struct Unknown_UsbInfo
{
    unsigned short idv;
    unsigned short pid;
    unsigned short bcd;
    string manufacturer;
    string product;
    string serial;
};

struct UpgradeInfo
{
    VersionInfo versionInfo;
    int serverType;

    void clear(void)
    {
        VersionInfo initInfo;
        versionInfo = initInfo;
        serverType = 0;
    }
};

struct AuthInfo
{
    int auth_type;
    int auto_connect;
    string auth_user;
    string auth_passwd;
};

struct FtpLogInfo
{
    string ftpuser;
    string ftppwd;
};

struct SystemHardwareInfo
{
    bool wifi_function;
    string cpu;
    string disk_size;
    string memory;
};

struct DiskPartInfo
{
    long long total;
    long long available;
};

#endif//_DATA_STRUCT_H

