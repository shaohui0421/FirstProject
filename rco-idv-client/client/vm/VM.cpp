#include <string>
#include <math.h>
#include "VM.h"
#include "vmmessage.h"
#include "rc_json.h"

using namespace RcJson;
using namespace std;
VM *VM::_vmself = NULL;
VM *VM::get_vmself()
{
	Application* app = Application::get_application();
	return app->get_vm();
}

VmManage* VM::get_vmManage(){
	return _vmManage;
}

VM::VM(Application* app)
    : _app(app)
    , _vmManage(new VmManage(app))
    , _vmmessage(new Vmmessage())
    , _imageManage(new ImageManage(app))
    , _est_sock(new ESTSock(app))
	, _vm_mac_dict(NULL)
{
    _imageName   = "";
    //_recovery    = true;
    _mac_inifile = "/opt/lessons/mac.ini";
    _ip_inifile  = "/opt/lessons/ip.ini";
    _VM_SN       = "";
    _network_info_inifile = "/opt/lessons/RCC-Client/vm_network_info.ini";
    _replace_mac = "52:54:00";
    _VM_MAC2 = "";

    _is_local_mode = false;
  	struct BasicInfo basicInfo;
	_app->vm_get_basic_info(&basicInfo);
    string basicinfo_mac = basicInfo.mac;
    //create vm mac by replacing basic mac
    string tempMAC = basicinfo_mac.erase(0,8);
    _VM_MAC = _replace_mac + tempMAC;
    string basicinfo_mac2 = basicInfo.mac2;
    if (basicinfo_mac2 != ""){
        //create vm mac by replacing basic mac
        string tempMAC2 = basicinfo_mac2.erase(0,8);
        _VM_MAC2 = _replace_mac + tempMAC2;
    }
    _VM_SN  = basicInfo.serial_number;
}

VM::~VM()
{
    delete _vmManage;
    delete _imageManage;
    delete _vmmessage;
    delete _est_sock;
}

bool VM::is_netdisk_enable()
{
	_is_local_mode = _app->is_localmode_startvm();
    if (_is_local_mode == true || _modeInfo.mode == PUBLIC_MODE
         || _userInfo.username == "guest" || _userInfo.netdisk_info.netdisk_enable == false) {
        // local mode & public mode & guest login, disable net disk.
    	return false;
    } else {
    	return true;
    }
}

bool VM::get_is_netdisk()
{
	return _is_netdisk;
}

string VM::get_usb_policy()
{
	return _usbPolicy;
}

bool VM::get_net_policy()
{
	return _netPolicy;
}

NetdiskInfo VM::get_netdisk_info()
{
	return _netDiskInfo;
}

void VM::vm_start_vm(ModeInfo* modeInfo, UserInfo* userInfo, bool recovery, int allow_userdisk, const string& driver_install_parament)
{

    DiskInfo w_disk;
    //int size = 0;

    _modeInfo = *modeInfo;
    _userInfo = *userInfo;
    _is_netdisk = is_netdisk_enable();

#if 0
    if (_modeInfo.mode == PUBLIC_MODE || _userInfo.username == "guest") {
        _usbPolicy = _app->get_public_policy().usb_policy;
        _netPolicy = _app->get_public_policy().net_policy;
    } else {
        _usbPolicy = userInfo->policy_info.usb_policy;
        _netPolicy = userInfo->policy_info.net_policy;
    }
#else
    //public policy will be stored into userinfo struct while public or guest loginning.
    _usbPolicy = userInfo->policy_info.usb_policy;
    if (driver_install_parament.empty()) {
        _netPolicy = userInfo->policy_info.net_policy;
    } else {
        _netPolicy = true;
    }
#endif

    LOG_DEBUG("create usb_policy %s", _usbPolicy.c_str());
    _app->create_usbfilter(_usbPolicy);


    if (_is_netdisk)
        _netDiskInfo = userInfo->netdisk_info;

    string imageName = _imageManage->get_image_info().name;
    LOG_DEBUG("imageName(read ini):%s", imageName.c_str());
    int nPos = imageName.find_first_of(".");
    string baseName = imageName.substr( 0 , nPos );

    if (!vm_is_vm_running()) {
        if (allow_userdisk == false) {
            _imageManage->clear_teacher_disk();
        } else {
            ImageInfo local_image_info;
            int d_disk_size;
            local_image_info = vm_get_vm_imageinfo();

            cal_d_disk_size(local_image_info.real_size, local_image_info.virt_size, &d_disk_size);
            //_app->_mina->mina_l2w_notify_partition(local_image_info.virt_size, d_disk_size);
            if (driver_install_parament.empty()) {
                _imageManage->create_user_disk(d_disk_size);
            }
        }

        if (!_app->get_UsrUserInfoMgr()->get_disk_info(PRINTER_DISK_NAME, w_disk)) {
            if (w_disk.is_enable) {
                if (cal_expand_disk()) {
                    if (driver_install_parament.empty()) {
                        if (!_imageManage->check_expand_disk_exist(PRINTER_DISK_NAME)) {
                            vm_print_unpack_handle(RCC_PRINT_FILE);
                        }
                    }
                }
            }
        }

        _vmManage->vm_set_VM(&_modeInfo, &_userInfo, baseName,
                recovery, _VM_MAC, _VM_SN, allow_userdisk, driver_install_parament, _VM_MAC2);
    }
}

void VM::vm_shutdown_VM_normal()
{
    _vmManage->vm_shutdown_VM_normal();
}

void VM::vm_shutdown_VM()
{
    _vmManage->vm_shutdown_VM();
}

/**
* vm_layer_templete_handle
*
* return: false : error / true: success
*/
bool VM::vm_layer_templete_handle(const string &layer_file)
{
    bool result = false;

    if (_imageManage->check_layer_tmplete_exist()) {
        result = _imageManage->check_file_md5(layer_file);
        if (!result) {
            _imageManage->clear_layer_templete();
        } else {
            return true;
        }
    }

    result = _imageManage->unpack_layer_templete();
    if (result) {
        result = _imageManage->check_file_md5(layer_file);
    }
    return result;
}

/**
* vm_print_unpack_handle
*
* return: false : error / true: success
*/
bool VM::vm_print_unpack_handle(const string &print_file)
{
    bool result = false;

    if (_imageManage->check_expand_disk_exist(PRINTER_DISK_NAME)) {
        LOG_INFO("printer disk exsit");
        return true;
    }

    result = _imageManage->unpack_print_templete();
    if (result) {
        result = _imageManage->check_file_md5(print_file);
        if (result) {
            _imageManage->copy_print_disk();
        }
    }
    return result;
}


void VM::vm_check_base_exist()
{
    _vmManage->vm_check_base_exist();
}

int VM::vm_check_local_base_exist()
{
	return _imageManage->check_local_base_exist();
}

bool VM::vm_check_personal_img_exist()
{
    return _imageManage->check_personal_img_exist();
}

bool VM::vm_check_usedisk_exist()
{
    return _imageManage->check_usedisk_exist();
}

bool VM::vm_check_file_md5(const string &file)
{
    return _imageManage->check_file_md5(file);
}

bool VM::vm_check_expand_disk_exist(const string &disk_name)
{
    return _imageManage->check_expand_disk_exist(disk_name);
}

bool VM::vm_check_layerdisk_exist()
{
    return _imageManage->check_layer_disk_exist();
}

bool VM::vm_is_vm_running()
{
	return _vmManage->is_vm_running();
}

void VM::vm_start_download_image(ImageInfo * imageInfo)
{
	_imageInfo = *imageInfo;
    //_recovery  = imageInfo->recovery;

    string torrent_name = _imageManage->execute_command("basename " + imageInfo->torrent_url);
    string imageName = imageInfo->name;
    int nPos = imageName.find_first_of(".");
    string baseName = imageName.substr( 0 , nPos );

    _imageManage->vm_start_download_image(&_imageInfo,baseName,torrent_name);
}

bool VM::vm_download_torrent(string torrent_name, string url)
{
	return _imageManage->download_torrent(torrent_name, url);
}
int VM::vm_get_image_diff_size(string new_torrent, string old_torrent)
{
	return _imageManage->get_image_diff_size(new_torrent,old_torrent);
}

void VM::vm_set_image_downmode(int mode)
{
    _imageManage->set_download_mode(mode);
}
int VM::vm_get_image_downmode()
{
  return  _imageManage->get_download_mode();
}
string VM::vm_get_new_base_name()
{
  return  _imageManage->get_new_base_name();
}

void VM::vm_clean_download_success_tips()
{
	_imageManage->clean_download_success_tips();
}

string VM::vm_get_new_base_version()
{
  return  _imageManage->get_new_base_version();
}

int VM::vm_get_new_base_id()
{
  return  _imageManage->get_new_base_id();
}

int	VM::vm_get_image_virt_size(const string image)
{
	return _imageManage->get_image_virt_size(image);
}

int VM::vm_get_image_size(const string image, bool is_real_size)
{
    return _imageManage->get_image_size(image, is_real_size);
}

int	VM::vm_get_user_disk_size()
{
	return _imageManage->get_user_disk_size();
}

int VM::vm_get_expand_disk_size(const string& disk_name)
{
    return _imageManage->get_expand_disk_size(disk_name);
}

int	VM::vm_create_user_disk(int size)
{
	return _imageManage->create_user_disk(size);
}
/*
int	VM::vm_get_c_disk_size()
{
	return _imageManage->get_c_disk_size();
}
*/
bool VM::vm_resize_image(string image, int size, string fmt)
{
	return _imageManage->resize_image(image, size, fmt);
}

bool VM::vm_create_c_disk(string image)
{
    ImageInfo local_image_info;
    local_image_info = vm_get_vm_imageinfo();
    return _imageManage->create_c_disk(image, local_image_info.virt_size);
}

bool VM::vm_create_layer_disk(string layer_name)
{
    ImageInfo local_image_info;
    local_image_info = vm_get_vm_imageinfo();
    return _imageManage->create_layer_disk(layer_name, local_image_info.virt_size);
}

void VM::vm_update_image_info(ImageInfo * imageInfo)
{
	return _imageManage->update_image_info(imageInfo);
}

void VM::vm_update_image_ostype(string *ostype)
{
    if (ostype) {
        _imageManage->update_image_ostype(ostype);
    }
}

/**
*function: found if ostype is exsit and get it value
*
*return 0:not exsit 1:exsit
*/
int VM::vm_get_image_ostype(string &ostype)
{
    return _imageManage->get_image_ostype(ostype);
}

int VM::vm_stop_download_image()
{
	return _imageManage->stop_download_image();
}

void VM::vm_quit_download_image()
{
    _imageManage->vm_quit_download_image();
}

int VM::vm_start_copy_usb_image()
{
    return _imageManage->start_usb_image();
}

int VM::vm_stop_copy_usb_image()
{
    return _imageManage->stop_usb_image();
}

bool VM::vm_is_usb_downloading()
{
    return _imageManage->is_usb_downloading();
}

int VM::vm_clear_inst()
{
    return _imageManage->clear_inst();
}

int VM::vm_clear_layer()
{
    return _imageManage->clear_layer();
}

int VM::vm_clear_print_disk(const string &disk_name)
{
    return _imageManage->clear_print_disk(disk_name);
}

int VM::vm_clear_base()
{
	return _imageManage->clear_base();
}

int VM::vm_remove_one_unnecessary_golden_image(string used)
{
    return _imageManage->remove_unnecessary_base(used);
}


int VM::vm_clear_teacher_disk()
{
    return _imageManage->clear_teacher_disk();
}

int VM::vm_clear_all()
{
	return _imageManage->clear_all();
}

void VM::get_vm_ipinfo()
{
    _vm_ip_dict = iniparser_load(_ip_inifile.c_str());
    _VM_IP      = iniparser_getstring(_vm_ip_dict,"ip:vm_ip",NULL);
    iniparser_dump_ini(_vm_ip_dict, _ip_inifile.c_str());
    iniparser_freedict(_vm_ip_dict);
}

void VM::vm_set_vm_netinfo_to_ini(NetworkInfo& networkInfo)
{
    _networkData.set_vm_network(networkInfo);
}

ImageInfo VM::vm_get_vm_imageinfo()
{
    return _imageManage->get_image_info();
}

void VM::vm_get_vm_layerdisk_info(LayerDiskInfo &layer_info)
{
    _imageManage->get_layerdisk_info(layer_info);
}

bool VM::vm_get_vm_download_status()
{
    return _imageManage->get_image_download_status();
}

void VM::vm_get_vm_info()
{
#ifdef UNIT_TEST
	struct VMInfo vmInfo =
	{
			false,
			4,
			1,
			2,
			3,
			4,
	};
	_app->vm_get_vm_info_status(&vmInfo);
#else
	_vmmessage->message_upload_vm_info();
    if (!vm_is_vm_running()) {
        struct VMInfo vmInfo = {
            .running = false,
        };
        //vmInfo.vm_net = vm_get_vm_netinfo();    //not send network info to web if vm not running 
        _app->vm_get_vm_info_status(&vmInfo);
    }
#endif /* UNIT_TEST */
}

void VM::vm_set_guest_merge_tips()
{
	_vmmessage->message_set_guest_merge_tips();
}
void VM::vm_send_download_progress_info_to_zhjsgt(string downloaded_size,string total_size,string percent,string rate)
{
	_vmmessage->message_upload_download_info_to_zhjsgt(downloaded_size,total_size,percent,rate);
}

void VM::vm_set_net_policy(bool netpolicy)
{
	if (_netPolicy == false)
		_vmmessage->message_set_guest_netdisable();
}

void VM::vm_set_usb_policy(string usbpolicy)
{
	_vmmessage->message_set_guest_usb_polocy(usbpolicy);
}

void VM::vm_set_vm_netinfo(NetworkInfo *networkInfo, int status)
{
    _networkData.set_vm_network(*networkInfo);
    if (status == STATUS_RUNNING_VM) {
	    _vmmessage->message_upload_vm_network_info(*networkInfo);
	    _vmmessage->message_upload_vm_network_info(*networkInfo, GUEST_NET_ID);        
        //todo?sync with guesttool
    }
}

void VM::vm_set_netinfo_on_running()
{
    NetworkInfo networkInfo = _networkData.get_vm_network();
    _vmmessage->message_upload_vm_network_info(networkInfo);
}

NetworkInfo VM::vm_get_vm_netinfo()
{
    return _networkData.get_vm_network();
}

void VM::vm_set_vm_diskinfo(NetdiskInfo &netdiskinfo)
{
	_vmmessage->message_upload_netdisk(netdiskinfo);
}

void VM::vm_disable_netuse()
{
    LOG_NOTICE("Disable VM network");
    rc_system("virsh domif-setlink vm vnet0 down");
    //_vmmessage->message_set_guest_disconnected();
    //TODO: if VM net up, should we re-forward vmmessages?
}

void VM::vm_enable_netuse()
{
    LOG_NOTICE("Enable VM network");
    rc_system("virsh domif-setlink vm vnet0 up");
}

void VM::vm_disable_net2use()
{
    LOG_NOTICE("Disable VM net2work");
    rc_system("virsh domif-setlink vm vnet1 down");
    //_vmmessage->message_set_guest_disconnected();
    //TODO: if VM net up, should we re-forward vmmessages?
}

void VM::vm_enable_net2use()
{
    LOG_NOTICE("Enable VM net2work");
    rc_system("virsh domif-setlink vm vnet1 up");
}

bool VM::vm_is_vm_recovery()
{
    return _vmManage->vm_is_vm_recovery();
}

bool VM::vm_is_xserver_exited()
{
    return _vmManage->vm_is_xserver_exited();
}

void VM::restart_nat()
{
    _vmManage->restart_nat();
}

void VM::switch_nat()
{
    _vmManage->switch_nat();
}

void VM::switch_bridge()
{
    _vmManage->switch_bridge();
}

void VM::vm_send_web_info(const string &web_msg)
{
    int target      = rc_json_get_int(web_msg, "target");
    int module_id   = rc_json_get_int(web_msg, "module_id");
    string msg      = rc_json_get_child(web_msg, "guesttool_msg");
    if((target != ERROR_INT) || (module_id != ERROR_INT))
    {
        _vmmessage->message_upload_web_info(msg, module_id, target);
    }
}

void VM::vm_set_guest_disconnected()
{
    _vmmessage->message_set_guest_disconnected();
}

bool VM::vm_start_bt_service()
{
    return _imageManage->start_bt_service();
}

bool VM::vm_stop_bt_service()
{
    return _imageManage->stop_bt_service();
}
int VM::vm_check_local_base_status()
{
    return _imageManage->check_local_base_status();
}

void VM::vm_start_est_sock()
{
    _est_sock->start_sock();
}

void VM::vm_stop_est_sock()
{
    //close all sock fd
    _est_sock->close_sock();
}

/**
* function: calcute expand disk whether can be create or not and the reserve size for expand disk
*
* return: > 0: the size to reserve for create  expand disk  0: not enough to create or not need to create
*/
int VM::cal_expand_disk()
{
    ImageInfo local_image_info;
    DiskInfo_t diskinfo;
    int expand_disk = 0;
    long total_size = 0;
    int user_disk = 0;
    int ret = 0;

    if (get_data_part_all_space(&total_size) < 0) {
        return 0;
    }

    if (_imageManage->check_expand_disk_exist(PRINTER_DISK_NAME)) {
        // PRINTER_DISK_NAME it size is 1G, then not need to change unit
        expand_disk = _imageManage->get_expand_disk_size(PRINTER_DISK_NAME);
        return expand_disk;
    }

    ret = _app->get_UsrUserInfoMgr()->get_disk_info(PRINTER_DISK_NAME, diskinfo);
    if (ret != 0) {
        LOG_ERR("read diskini file failed %d", ret);
        return 0;
    } else {
        if (diskinfo.is_enable) {
            expand_disk = diskinfo.size;
            expand_disk = ceil((double)expand_disk / 1024.0);
        } else {
            return 0;
        }
    }

    local_image_info = vm_get_vm_imageinfo();
    if (local_image_info.virt_size + local_image_info.real_size > total_size - expand_disk) {
       LOG_DEBUG("not enough space to create expand disk");
       return 0;
    }

    if (_imageManage->check_usedisk_exist()) {
        user_disk = _imageManage->get_user_disk_size();
    }

    if (local_image_info.virt_size + local_image_info.real_size + user_disk > total_size - expand_disk) {
        LOG_DEBUG("not enough cal_expand_disk:%d total_size:%d virt_size:%d real_size:%d user_disk:%d", expand_disk, total_size,local_image_info.virt_size, local_image_info.real_size, user_disk);
        return 0;
    }

    LOG_DEBUG("cal_expand_disk %d", expand_disk);
    return expand_disk;
}

bool VM::space_enough(const int real_size, const int virt_size, int * clean_use_disk)
{
    long total_size;
    int user_disk = 0;
    int need_clean = 0;
    int expand_disk = 0;

	if (get_data_part_all_space(&total_size) < 0) {
		return false;
	}

    // if expand open then expend disk Priority is higher than C disk Capacity expansion
    expand_disk = cal_expand_disk();
    if (real_size + virt_size > total_size - expand_disk) {
        return false;
    }

	if (_imageManage->check_usedisk_exist()) {
		user_disk = _imageManage->get_user_disk_size();
	}

    if (user_disk + real_size + virt_size > total_size - expand_disk) {
        need_clean = 1;
    }

	if (clean_use_disk) {
		*clean_use_disk = need_clean;
	}

    return true;
}
bool VM::space_enough_silent(const int real_size, const int virt_size, const int diff_size)
{
    long total_size;
    int user_disk = 0;
    int expand_disk = 0;

	if (get_dir_all_space("/opt/lessons", &total_size, "g") < 0) {
		return false;
	}

    // if expand open then expend disk Priority is higher than C disk Capacity expansion
    expand_disk = cal_expand_disk();

	if (_imageManage->check_usedisk_exist()) {
		user_disk = _imageManage->get_user_disk_size();
	}

    LOG_DEBUG("real_size %d virt_size %d total_size %d user_disk %d diff_size %d", real_size, virt_size, total_size, user_disk,diff_size);
    if ((user_disk + real_size + virt_size + diff_size*2) > (total_size - expand_disk)) {
        return false;
    }
    
    return true;
}

bool VM::cal_d_disk_size(const int real_size, const int virt_size,  int *disk_size)
{
    long total_size;
    int user_disk = 0;
    int left_size = 0;
    int reserver_space = 0;
    int expand_disk = 0;

	*disk_size = 0;
	if (get_data_part_all_space(&total_size) < 0) {
		return false;
	}

    expand_disk = cal_expand_disk();
    if (real_size + virt_size > total_size - expand_disk) {
        return false;
    }

	//for ssd 128 terminal dev
/*	if (total_size == 108) {
		total_size = 110;
	}
*/
	if (_imageManage->check_usedisk_exist()) {
		user_disk = _imageManage->get_user_disk_size();
	}

	reserver_space = (virt_size/2)  + real_size;
	if (reserver_space >  virt_size) {
		reserver_space = virt_size;
	}

    left_size = total_size - virt_size - reserver_space - expand_disk;

    LOG_DEBUG("reserver_space %d left_size %d", reserver_space, left_size);
    if (left_size < 5) {    
        left_size = 0;
    } else {
        left_size = left_size - (left_size %5 );
    }

//	if (disk_size) {
		*disk_size = left_size;

        if (user_disk > 0) {
            *disk_size = user_disk;
        }
//  }
    LOG_NOTICE("total_size %d, real size %d; virt size %d, user_disk %d, cal d disk size %d expand_disk %d",
        total_size, real_size, virt_size, user_disk, *disk_size, expand_disk);
    return true;
}

int VM::cal_max_c_disk_size(const int real_size, const int c_disk_size, const int d_disk_size)
{
    long total_size;
    int user_disk = 0;
    int max_c_disk_size = 0;
    int expand_disk = 0;

    if (get_data_part_all_space(&total_size) < 0) {
        LOG_ERR("cal max c disk size err: total size < 0");
        return c_disk_size;
    }

    expand_disk = cal_expand_disk();
    if (_imageManage->check_usedisk_exist()) {
        user_disk = _imageManage->get_user_disk_size();
        max_c_disk_size = total_size - real_size - user_disk - expand_disk;
    } else {
        max_c_disk_size = total_size - real_size - expand_disk;
    }
    if (max_c_disk_size < c_disk_size) {
        LOG_ERR("cal max c disk size err: total size %d, real_size %d, c_disk_size %d, d_disk_size %d, cal max_c_disk_size %d expand_disk %d",
            total_size, real_size, c_disk_size, d_disk_size, max_c_disk_size, expand_disk);
        max_c_disk_size = c_disk_size;
    }
    LOG_NOTICE("cal max c disk size: %d", max_c_disk_size);
    return max_c_disk_size;
}

bool VM::usb_copy_space_enough(const string& basename)
{
    long total_size;
    int usb_base_size = 0;
    int user_disk = 0;
    int expand_disk = 0;

    if (get_data_part_all_space(&total_size) < 0) {
        return false;
    }

    expand_disk = cal_expand_disk();
    if (_imageManage->check_usedisk_exist()) {
        user_disk = _imageManage->get_user_disk_size();
    }

    usb_base_size = _imageManage->get_usb_base_size(basename);

    LOG_NOTICE("total_size %d, user_disk %d, usb base size %d expand_disk %d", total_size, user_disk, usb_base_size, expand_disk);

    if (user_disk + usb_base_size > total_size - expand_disk) {
        return false;
    }
    return true;
}

/**
* function: judget layer disk and img disk total real size is full
* basename : image name
*
* return: true: is full / false
*/
bool VM::judge_disk_isfull(const string basename)
{
    bool ret = false;
    int layer_size = 0, img_size = 0;
    string imgae_name;
    ImageInfo local_image_info;

    LOG_DEBUG("enter %s", __func__);

    if (!_imageManage->check_layer_disk_exist()) {
       return false;
    }

    imgae_name = basename + ".layer";
    layer_size = vm_get_image_size(imgae_name, true);

    if (!_imageManage->check_personal_img_exist()) {
        LOG_INFO("personal_img not exist");
        return false;
    }

    imgae_name = basename + ".img";
    img_size = vm_get_image_size(imgae_name, true);

    local_image_info = vm_get_vm_imageinfo();
    if (layer_size + img_size > local_image_info.virt_size + 2) {
        ret = true;
    }

    LOG_DEBUG("judge_disk_isfull %d", ret);
    return ret;
}

