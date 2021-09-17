#include <math.h>
#include "common.h"
#include "Usb_manage.h"

using namespace std;

UsbManage::UsbManage()
{
    char command_buf[256];
    if(!get_file_exist(USB_CP_BASE_MOUNT_DIR.c_str()))
    {
        sprintf(command_buf, "mkdir -p %s", USB_CP_BASE_MOUNT_DIR.c_str());
        rc_system(command_buf);
    }
}

//if usb contain same base,return true
//basename:the name of base
//dev_name:return the dev name which contains the base
int UsbManage::if_usb_contain_same_base(const string& basename, string& dev_name)
{
    string cmd = RCC_SCRIPT_PATH"if_usb_contain_same_base.sh " + basename + " " + USB_CP_BASE_MOUNT_DIR + " &";
    string result = execute_command(cmd);
    if ("DEV NOT EXIST" == result) {
        LOG_INFO("usb dev not exist!");
        return 1;
    }else if ("NOT EXIST" == result || "error" == result) {
        LOG_INFO("usb does not contain same base:%s!",basename.c_str());
        return 2;
    }else {
        LOG_INFO("usb contain same base:%s!",basename.c_str());
        dev_name = result;
        return 0;
    }
}

void UsbManage::mount_dev(const string& dev_name)
{
    string cmd = "mount " + dev_name + " " + USB_CP_BASE_MOUNT_DIR;
    string result = execute_command(cmd);
    if ("error" == result) {
        LOG_INFO("mount dev:%s  error!",dev_name.c_str());
    }else {
        LOG_INFO("mount dev:%s  success!",dev_name.c_str());
    }
}

void UsbManage::umount_dev()
{
    string cmd = "umount -fl " + USB_CP_BASE_MOUNT_DIR;
	string result = execute_command(cmd);
    if ("error" == result) {
        LOG_INFO("umount error!");
    }else {
        LOG_INFO("umount success!");
    }
}

void UsbManage::start_cp_base(const string& basename)
{
    LOG_INFO("enter start_cp_base, basename = %s", basename.c_str());
    string cmd = "rsync -avzP " + USB_CP_BASE_MOUNT_DIR + basename +  " " + USB_CP_BASE_DIR + " > " + USB_CP_BASE_LOG + "&";
    string result = execute_command(cmd);
	if ("error" == result) {
        LOG_INFO("start cp base error!");
    }else {
        LOG_INFO("start cp base success!");
    }
}

void UsbManage::stop_cp_base()
{
    LOG_INFO("enter stop_cp_base");
    string cmd = "pkill -9 rsync &";
    string result = execute_command(cmd);
	if ("error" == result) {
        LOG_INFO("stop cp base error!");
    }else {
        LOG_INFO("stop cp base success!");
    }
}

//update cp status
//if dev_name is not exist,return
//if get status error,return
int UsbManage::update_status(string& cp_percent, string& cp_rate, const string& dev_name)
{

    string cmd_percent = "sed \'s/\\r/\\n/g\' " + USB_CP_BASE_LOG + " | awk \'/%/ {print $2}\' | tail -1";
    string cmd_rate = "sed \'s/\\r/\\n/g\' " + USB_CP_BASE_LOG + " | awk \'/%/ {print $3}\' | tail -1";
    cp_percent = execute_command(cmd_percent);
    cp_rate = execute_command(cmd_rate);

    if (!if_mounted_dev_exist(dev_name))
    {
        return CP_BASE_MOUNTED_DEV_NOT_EXIST;
    }else if ("" == cp_percent || "" == cp_rate) {
        return CP_BASE_UPDATE_STATUS_BASE_READY;
    }else if ("error" == cp_percent || "error" == cp_rate) {
        return CP_BASE_UPDATE_STATUS_ERROR;
    }else {
        return CP_BASE_UPDATE_STATUS_SUCCESS;
    }
}

void UsbManage::start_cp_base2(const string& basename)
{
    LOG_INFO("enter start_cp_base, basename = %s", basename.c_str());
    string cmd = RCC_SCRIPT_PATH"usb_copy_base.sh " + USB_CP_BASE_MOUNT_DIR + basename +  " " + USB_CP_BASE_DIR + basename + " > " + USB_CP_BASE_LOG + " &";
    string result = execute_command(cmd);
    if ("error" == result) {
        LOG_INFO("start cp base error!");
    }else {
        LOG_INFO("start cp base success!");
    }
}

void UsbManage::stop_cp_base2()
{
    LOG_INFO("enter stop_cp_base");
    string cmd = "pkill -9 cp &";
    string result = execute_command(cmd);
    if ("error" == result) {
        LOG_INFO("stop cp base error!");
    }else {
        LOG_INFO("stop cp base success!");
    }
}

//update cp status
//if dev_name is not exist,return
//if get status error,return
int UsbManage::update_status2(string& cp_percent, string& cp_rate, const string& dev_name)
{
    string cmd_percent = "tail -1 " + USB_CP_BASE_LOG + " | awk \'{print $1}\'";
    string cmd_rate = "tail -1 " + USB_CP_BASE_LOG + " | awk \'{print $2}\'";
    cp_percent = execute_command(cmd_percent);
    cp_rate = execute_command(cmd_rate);

    if (!if_mounted_dev_exist(dev_name))
    {
        return CP_BASE_MOUNTED_DEV_NOT_EXIST;
    }else if ("error" == cp_percent || "error" == cp_rate) {
        return CP_BASE_UPDATE_STATUS_ERROR;
    }else {
        return CP_BASE_UPDATE_STATUS_SUCCESS;
    }
}
int UsbManage::get_base_size(const string& basename)
{
    int base_size = 0;
    double size = 0.0;
    char unit;
    string cmd = "ls -lh " + USB_CP_BASE_MOUNT_DIR + basename + " | awk '{print $5}'";
    string result = execute_command(cmd);
    LOG_INFO("get usb base size: %s", result.c_str());
    if("error" == result)
    {
        LOG_INFO("get usb base size fail!");
        return 0;
    } else {
        if (result.size() <= 1) {
            return 0;
        }
        unit = result.back();
        if (tolower(unit) == 'k') {
            result.erase(result.size()-1);
            size = atof(result.c_str());
            base_size = (int)ceil(size / (1024*1024));
        } else if (tolower(unit) == 'm') {
            result.erase(result.size()-1);
            size = atof(result.c_str());
            base_size = (int)ceil(size / 1024);
        } else if (tolower(unit) == 'g') {
            result.erase(result.size()-1);
            base_size = (int)ceil(atof(result.c_str()));
        } else {
            base_size = 0;
        }
        return base_size;
    }
}

bool UsbManage::if_mounted_dev_exist(const string& dev_name)
{
    string cmd = "fdisk -l | grep " + dev_name + " | wc -l";
	string result = execute_command(cmd);
	if (result == "error") {
        LOG_INFO("check dev fail!");
        return false;
    } else {
        if (atoi(result.c_str()) == 0) {
           LOG_INFO("dev:%s is not exist!",dev_name.c_str());
           return false; 
		} else {
           LOG_INFO("dev:%s is exist!",dev_name.c_str());
           return true;
		} 
    }
}