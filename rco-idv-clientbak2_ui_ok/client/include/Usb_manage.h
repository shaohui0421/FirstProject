#ifndef _USB_MANAGE_H
#define _USB_MANAGE_H

#include <string.h>
#include <iostream>
#include "vm_common.h"

enum CP_BASE_STATUS
{
    CP_BASE_MOUNTED_DEV_NOT_EXIST = 0,
    CP_BASE_UPDATE_STATUS_BASE_READY,
    CP_BASE_UPDATE_STATUS_ERROR,
    CP_BASE_UPDATE_STATUS_SUCCESS,
};



class UsbManage:public VM_Common
{
const string USB_CP_BASE_MOUNT_DIR =       "/mnt/usb_cp_base/";
const string USB_CP_BASE_DIR       =       "/opt/lessons/";
const string USB_CP_BASE_LOG       =       "/tmp/usb_cp_base.log";

public:
    UsbManage();
    int if_usb_contain_same_base(const string& basename, string& dev_name);
    void start_cp_base(const string& basename);
    void stop_cp_base();
    void mount_dev(const string& dev_name);
    void umount_dev();
    int update_status(string& cp_percent, string& cp_rate, const string& dev_name);
    void start_cp_base2(const string& basename);
    void stop_cp_base2();
    int update_status2(string& cp_percent, string& cp_rate, const string& dev_name);
    int get_base_size(const string& basename);
    virtual void action(){}
private:
    bool if_mounted_dev_exist(const string& dev_name);
    string _mount_dir;
    string _cp_log;
    string _base_dir;
};

#endif

