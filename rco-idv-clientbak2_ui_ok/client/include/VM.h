#ifndef _VM_H
#define _VM_H

#include "Image_manage.h"
#include "VM_Manage.h"
#include "application.h"
#include "vmmessage.h"
#include "user_db.h"
#include "est_sock.h"

class Application;
class ImageManage;
class VmManage;
class Vmmessage;
class ESTSock;

#define READ_PIPE_LENGTH                            256

class VM
{
public:
    VM(Application* app);
    virtual ~VM();
    //vm_manage event
    void vm_start_vm(ModeInfo* modeInfo, UserInfo* userInfo, bool recovery, int allow_userdisk, const string& driver_install_parament);

    //shutdown vm normal
    //if normal shutdown fail,shutdown vm force,like power down
    void vm_shutdown_VM_normal();
    void vm_shutdown_VM();

    void vm_check_base_exist();
    int  vm_check_local_base_exist();
    bool vm_check_personal_img_exist();
    bool vm_check_layerdisk_exist();
    bool vm_layer_templete_handle(const string &layer_file);
    bool vm_check_usedisk_exist();
    bool vm_check_expand_disk_exist(const string &disk_name);
    bool vm_check_file_md5(const string &file);
    bool vm_print_unpack_handle(const string &print_file);
    bool vm_is_vm_running();
    void vm_set_image_downmode(int mode);
    int vm_get_image_downmode();
    string vm_get_new_base_name();
    string vm_get_new_base_version();
    int vm_get_new_base_id();
    int vm_get_image_diff_size(string new_torrent, string old_torrent);
    void vm_set_guest_merge_tips();
    void vm_send_download_progress_info_to_zhjsgt(string downloaded_size,string total_size,string percent,string rate);
    ImageInfo vm_get_vm_imageinfo();
    void vm_get_vm_layerdisk_info(LayerDiskInfo &layer_info);
    bool vm_get_vm_download_status();
    void vm_set_vm_netinfo_to_ini(NetworkInfo& networkInfo);
    void vm_clean_download_success_tips();
    
    //image_manage event
    void vm_start_download_image(ImageInfo* imageInfo);
    bool vm_download_torrent(string torrent_name, string url);
    int vm_stop_download_image();
    void vm_quit_download_image();

    int vm_start_copy_usb_image();
    int vm_stop_copy_usb_image();
    bool vm_is_usb_downloading();
    bool usb_copy_space_enough(const string& basename);
    float cal_real_disk_space();
    int vm_check_local_base_status();
    int vm_clear_inst();
    int vm_clear_layer();
    int vm_clear_print_disk(const string &disk_name);
    int vm_clear_base();
    int vm_remove_one_unnecessary_golden_image(string used);
    int vm_clear_teacher_disk();
    int vm_clear_all();
    bool space_enough(const int real_size, const int virt_size, int * clean_use_disk);
    bool space_enough_silent(const int real_size, const int virt_size, const int diff_size);
    int  cal_expand_disk();
    bool cal_d_disk_size(const int real_size, const int virt_size,  int *disk_size);
    int cal_max_c_disk_size(const int real_size, const int c_disk_size, const int d_disk_size);
    bool judge_disk_isfull(const string basename);

    void vm_update_image_info(ImageInfo * imageInfo);
    void vm_update_image_ostype(string *ostype);
    int vm_get_image_ostype(string &ostype);
    bool vm_resize_image(string image, int size, string fmt);
    bool vm_create_c_disk(string image);
    bool vm_create_layer_disk(string layer_name);
    int	vm_get_image_virt_size(const string image);
    int vm_get_image_size(const string image, bool is_real_size);
    int vm_get_user_disk_size();
    int vm_get_expand_disk_size(const string& disk_name);
    int vm_create_user_disk(int size);
    //int vm_get_c_disk_size();
    bool vm_is_vm_recovery();
    bool vm_is_xserver_exited();
    bool vm_start_bt_service();
    bool vm_stop_bt_service();
    void vm_start_est_sock();
    void vm_stop_est_sock();

    //vm_message event
    void vm_get_vm_info();
	void vm_set_net_policy(bool netpolicy);
	void vm_set_usb_policy(string usbpolicy);
	void vm_set_vm_netinfo(NetworkInfo *networkInfo, int status);
    void vm_set_netinfo_on_running();
	NetworkInfo vm_get_vm_netinfo();
	void vm_set_vm_diskinfo(NetdiskInfo &netdiskinfo);
    void vm_send_web_info(const string &web_msg);
    void vm_set_guest_disconnected();

    //vm operations
    void vm_disable_netuse();
    void vm_enable_netuse();

    //vm operations 2
    void vm_disable_net2use();
    void vm_enable_net2use();

    void restart_nat();
    
    void switch_nat();

    void switch_bridge();


	bool get_is_netdisk();
	string get_usb_policy();
	bool get_net_policy();
	NetdiskInfo get_netdisk_info();
    static VM *_vmself;
    static VM *get_vmself();
    VmManage* get_vmManage();
    Vmmessage* get_vmMessage(){return _vmmessage;}
    const string & get_vm_mac(){return _VM_MAC;}
friend class Vmmessage;
private:
	bool is_netdisk_enable();

private:
    Application* _app;
    VmManage* _vmManage;
    Vmmessage* _vmmessage;
    ImageManage* _imageManage;
    ESTSock* _est_sock;
    dictionary* _vm_mac_dict;

    //vm event
	void get_vm_ipinfo();//netinfo
	void get_vm_hostname();
    
    dictionary* _vm_ip_dict;

    string _mac_inifile;
    string _ip_inifile;
    string _network_info_inifile;
    NetworkInfo _networkInfo;

    BasicInfo _basicInfo;
    ModeInfo _modeInfo;
    UserInfo _userInfo;
    ImageInfo _imageInfo;
    bool _isLocal;
    //bool _recovery;
    string _imageName;

    bool _is_local_mode;
    bool _is_netdisk;
    string _usbPolicy;
    bool _netPolicy;
    NetdiskInfo _netDiskInfo;
    
    string _VM_MAC;
    string _VM_MAC2;
    string _replace_mac;
    string _VM_SN;
    string _VM_IP;
    string _VM_hostname;

    VmNetworkInfoDB _networkData;

};

#endif
