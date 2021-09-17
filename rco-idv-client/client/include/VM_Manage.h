#ifndef _VM_MANAGE_H
#define _VM_MANAGE_H

#include <iostream>
#include <string>
#include <vector>
#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>
#include "common.h"
#include "process_loop.h"
#include "application.h"
#include "vm_common.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED __attribute__((__unused__))
#endif

#define VMMODE_EMULATION_INI            "Emulation"
#define VMMODE_PASS_INI                 "Passthrough"
#define OSTYPE_UNKNOWN                  "Unknown"
#define OSTYPE_WINDOWS_XP               "Windows XP"
#define OSTYPE_WINDOWS_7                "Windows 7"
#define OSTYPE_WINDOWS_10               "Windows 10"

#define XML_VMMODE_WINDOWS_7                "Windows7"
#define XML_VMMODE_EMULATION                "emulation"

struct ImportXMLInfo
{
    int vcpu;
    string base_file;
    string img_file;
    string layer_file;
    string layer_serial;
    string driver_iso;
    bool data_disk;
    string vm_mac;
    string vmmode;
    string ostype;
    bool is_layer_exist;
    bool is_print;
    std::vector<string> passlist;
    string screen_w;
    string screen_h;
    int qxl_num;
    string qxl_screen1_w;
    string qxl_screen1_h;
    string qxl_screen2_w;
    string qxl_screen2_h;
    string inter_info;
    string vm_mac2;
};

enum START_VM
{
    START_SUCCESS = 0,
    START_FAIL,
};

enum START_VM_ERRCODE
{
    START_VM_FAILED = 0,
    START_OSTYPE_UNKNOWN, 
    START_NOT_SUPPORT_CPU,
    START_NOT_SUPPORT_OSTYPE,
    START_VMMODEINT_LOST,
    START_DRIVER_OSTYPE_NOTADAPT,
    START_CREATE_CDISK_ERR,
    START_LAYER_TEMPLETE_ERR,
    START_VM_CHECK_OK,
    START_INTEL_NO_AUDIO_DEVICE,
};

enum SHUTDOWN_VM
{
    SHUTDOWN_SUCCESS = 0,
    SHUTDOWN_FAIL,
};

enum REBOOT_VM
{
    REBOOT_SUCCESS = 0,
    REBOOT_FAIL,
};

enum CHECK_IMAGE
{
    EXIST    = 0,
    NO_EXIST,
};

enum PRINTER_MANAGER_STATUS
{
    PRINTER_MANAGER_CLOSE = 0,
    PRINTER_MANAGER_OPEN = 1,
    PRINTER_MANAGER_SPACE_NOT_ENOUGH = 2,
    PRINTER_MANAGER_SPACE_CREATE_ERROR = 3,
    PRINTER_MANAGER_UNPACK_ERROR = 4,
    PRINTER_MANAGER_MAX,
};

class Application;
class ImageManage;
class VmManage;

class VmManage:public VM_Common
{
class SetVmEvent;
class ShutDownVmNormalEvent;
class ShutDownVmEvent;
class CheckBaseExistEvent;
class VmShutdownMonitorTimer;
class CreateXmlFileEvent;

const string IMAGE_PATH =       "/opt/lessons/";
const string USER_DISK_FILE =   IMAGE_PATH + "data.teacher.disk";
const string LAYER_TEMPLETE_FILE = IMAGE_PATH + "layer/layer_templete";
const string PRINT_DISK_FILE =   IMAGE_PATH + "w.disk";
const string XML_FILE =         IMAGE_PATH + "import.xml";
const string IMG_FORMAT =       ".img";
const string LAYER_FORMAT =      ".layer";
const string BASE_FORMAT =      ".base";
const string GUEST_IMG_NAME =	"guest" + IMG_FORMAT;
const string GUEST_IMG =        IMAGE_PATH + GUEST_IMG_NAME;
const string ACPI_CONF =        IMAGE_PATH + "acpitable.conf";

const string SET_VM =           "vmmanager --setvm /opt/lessons/import.xml";
const string SHUTDOWN_VM_NORMAL= "vmmanager --normal_shutdown";
const string SHUTDOWN_VM       =  "vmmanager --shutdown";
const string CHECK_VIRTIO_FLAG= "vmmanager --checkvflag";
const string CMD_BASE_NUM =     "ls /opt/lessons/*.base | wc -l";
//vmmanager --check_image xxx.img
const string VMMANAGER_CHECK =       "vmmanager --check_image /opt/lessons/";

public:
    VmManage(Application* app);
    virtual ~VmManage();
    virtual void action();
    void create_parament_xml_file(struct ImportXMLInfo &param);
    //void testxml();
    void create_dev_inter_xml(xmlNodePtr &devicesNode, const string& infos, string xmlname, bool isObject = false, bool isList = false);
    void set_VM();
    void set_VM_driver_install();
    void shutdown_VM_normal();
    void shutdown_VM();
    void check_base_exist();
    bool is_vm_running();
    int  get_start_vm_err();
    int get_start_vm_is_emulation(bool is_driver = false, string driver_ostype = "");
    void restart_nat();
    void switch_nat();
    void switch_bridge();
    static int monitor_vm_shutdown();
    static int clear_inst_guest();
    static void execute_cmd_after_vm_shutdown();
    static void execute_cmd_after_vm_reboot();
    static void stop(int sig);
    
    bool vm_set_VM(ModeInfo* modeInfo,UserInfo* userInfo,string imageName,bool recovery, string mac, \
    string sn, int allow_userdisk, const string& driver_install_parament, string mac2);
    bool vm_shutdown_VM_normal();
    bool vm_shutdown_VM();
    bool vm_check_base_exist();
    bool vm_is_vm_recovery();
    bool vm_is_xserver_exited();

private:
    void _create_acpi_file(const string& filename, bool edit,const string& isE1000);
    bool _has_user_disk();
    bool _has_create_layer();
    void _get_printer_manager_code(const DiskInfo_t &disk_info, string& printer_manager);
    //int _get_vm_cpu_num();
    void _get_cpu_codename(string &codename);
    int _check_vmmodeini_exist();
    int _get_vmmodeini_param(string &vmmode, string &cpu);
    int _get_image_ostype_vmmode(string &vmmode, bool is_driver, string driver_ostype="");
    int _get_vmmode_passlist(std::vector<string> &passlist, bool is_driver, string driver_ostype="");
    //int _calc_vm_memory_size(const string &vmmode);
    void _vm_check_virtio_flag(string base_file, string img_file, bool* virtio_flag, bool* extend_mem);
    //int _vm_extend_mem_mode(bool* extend_mem);
   // int _vm_get_user_disk_clear(bool &extend_mem);
    //int _vm_get_vm_start_quickly_mem(bool &extend_mem);
    static void* start_monitor(void *data);
    bool create_hostname_conf(string SN);
    static void* thread_main(void* data);
    static void connectClose(virConnectPtr conn ATTRIBUTE_UNUSED,int reason,void *opaque ATTRIBUTE_UNUSED);
    static void freeFunc(void *opaque);

    static int domainEventLifeCycleCallback(virConnectPtr conn ATTRIBUTE_UNUSED,virDomainPtr dom,int event,int detail,void *opaque ATTRIBUTE_UNUSED);
    static int domainEventRebootCallback(virConnectPtr conn ATTRIBUTE_UNUSED,virDomainPtr dom,void *opaque ATTRIBUTE_UNUSED);
    static string eventToString(int event);
    static string eventDetailToString(int event, int detail);

    // added by yejx: start & stop X server
    void _start_xserver();
    void _stop_xserver();

private:
    Application* _app;
    ProcessLoop _process_loop;
    VmShutdownMonitorTimer* _vmShutdownMonitorTimer;
    bool _xserver_exited;
    
    string _clear_data_disk;
    string _base_name;//use which base to start vm

    ModeInfo* _modeInfo;
    UserInfo* _userInfo;
    string _imageName;
    bool   _recovery;
    bool   _vm_recovery;
    string _vm_mac;
    string _vm_mac2;
    string _vm_sn;
    LayerDiskInfo _layer_info;
    DiskInfo_t _printerdisk_info;
    static int _mode;
    static int _run;
    int _allow_userdisk;
    //int _reserved_memory;
    int _startvm_errcode;
    string _vmmode;
    string _driver_install_parament;
    UnjoinableThread* _main_thread;
    UnjoinableThread* _monitor_thread;    

friend class VmManage::SetVmEvent;
friend class VmManage::ShutDownVmNormalEvent;
friend class VmManage::ShutDownVmEvent;
friend class VmManage::CheckBaseExistEvent;
friend class VmManage::VmShutdownMonitorTimer;
    class SetVmEvent:public Event
    {
        public:
            SetVmEvent(VmManage* vmManage):_vmManage(vmManage){}
            virtual ~SetVmEvent(){}
            virtual void response()
            {
                if(_vmManage->_driver_install_parament.empty())
                {
                    _vmManage->set_VM();
                }
                else
                {
                    _vmManage->set_VM_driver_install();

                }
            }
        private:
            VmManage* _vmManage;
    };
        class ShutDownVmNormalEvent:public Event
    {
        public:
            ShutDownVmNormalEvent(VmManage* vmManage):_vmManage(vmManage){}
            virtual ~ShutDownVmNormalEvent(){}
            virtual void response()
            {
                _vmManage->shutdown_VM_normal();
            }
        private:
            VmManage* _vmManage;
    };

    class ShutDownVmEvent:public Event
    {
        public:
            ShutDownVmEvent(VmManage* vmManage):_vmManage(vmManage){}
            virtual ~ShutDownVmEvent(){}
            virtual void response()
            {
                _vmManage->shutdown_VM();
            }
        private:
            VmManage* _vmManage;
    };

    class CheckBaseExistEvent:public Event
    {
        public:
            CheckBaseExistEvent(VmManage* vmManage):_vmManage(vmManage){}
            virtual ~CheckBaseExistEvent(){}
            virtual void response()
            {
                _vmManage->check_base_exist();
            }
        private:
            VmManage* _vmManage;
    };

    class VmShutdownMonitorTimer:public Timer
    {
        public:
            VmShutdownMonitorTimer(VmManage* vmManage):_vmManage(vmManage){}
            virtual ~VmShutdownMonitorTimer(){}
            virtual void response()
            {
                _vmManage->monitor_vm_shutdown();
            }
        private:
            VmManage* _vmManage;
    };
    
};

#endif
