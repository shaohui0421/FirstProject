#ifndef _IMAGE_MANAGE_H
#define _IMAGE_MANAGE_H

#include <iostream>
#include <string.h>
#include <pthread.h>
#include <vector>
#include <string.h>

#include "common.h"
#include "process_loop.h"
#include "application.h"
#include "vm_common.h"
#include "Usb_manage.h"

enum QUIT_DOWNLOAD_IMAGE
{
    QUIT_SUCCESS    = 0,
    QOIT_FAIL,
};

enum DOWNLOAD_IMAGE_STATUS
{
    DOWNLOAD_IMAGE_SUCCESS = 0,
    DOWNLOAD_IMAGE_ERROR,
	DOWNLOAD_IMAGE_TIMEOUT,
    DOWNLOAD_IMAGE_DOWNLOADING,
    DOWNLOAD_IMAGE_CHECKING_MD5,
	DOWNLOAD_IMAGE_CHECKED_MD5_FAIL,
	DOWNLOAD_IMAGE_CHECKING,
	DOWNLOAD_IMAGE_NOSPACE,

    DOWNLOAD_COPING_USB_IMAGE,
    DOWNLOAD_CP_BASE_MOUNTED_DEV_NOT_EXIST,
    DOWNLOAD_CP_BASE_UPDATE_STATUS_ERROR,
    DOWNLOAD_CP_BASE_UPDATE_STATUS_BASE_READY,
	DOWNLOAD_IMAGE_MERGE,
};

enum DOWNLOAD_IMAGE_MODE
{
    DOWNLOAD_IMAGE_NO_OPS = 0,
    DOWNLOAD_IMAGE_FORCE_DOWNLOAD,
    DOWNLOAD_IMAGE_SILENT_DOWNLOAD,
    DOWNLOAD_IMAGE_SILENT_DOWNLOADING,
    DOWNLOAD_IMAGE_NEED_MERGE,
    DOWNLOAD_IMAGE_MERGING,
};
class Application;
class ImageManage;
class VmManage;

class ImageManage:public VM_Common
{
class StartDownloadImageEvent;
class QuitDownloadImageEvent;
class AnalyzeStatusTimer;

public:
    ImageManage(Application* app);
    virtual ~ImageManage();

    virtual void action();

    bool start_download_image(ImageInfo* imageInfo);
    void analyze_download_status();    
    void quit_download_image();
    bool unpack_layer_templete();
    bool unpack_print_templete();

    bool vm_start_download_image(ImageInfo* imageInfo, string baseName, string torrentName);
    bool download_torrent(const string torrent_file, const string url);
    bool resize_image(const string image, const int size, string fmt, string unit = "G");
    void update_image_info(ImageInfo* imageInfo);
    void update_image_ostype(string *ostype);
    int get_image_ostype(string &ostype);
    int get_image_real_size(const string image);
    int get_image_virt_size(const string image);
    int get_image_size(const string image, bool is_real = false);
    int get_image_default_virt_size(const string image_name);
    int get_user_disk_size();
    int get_expand_disk_size(const string &disk_name);
    //int get_c_disk_size();
    bool create_user_disk(int size);
    bool create_c_disk(string image, int size);
    bool create_layer_disk(string layer_name, int size);
    bool create_expand_disk(const string &disk_name, const int &resize);
    bool copy_print_disk();
    bool vm_quit_download_image();
    void set_download_mode(int mode);
    int  get_download_mode();
    int get_image_diff_size( string new_torrent, string old_torrent);
    void set_new_base_name(string name);
    string  get_new_base_name();
    void clean_download_success_tips();
    void set_new_base_version(string version);
    string  get_new_base_version();
    void set_new_base_id(int id);
    int  get_new_base_id();
    
    bool is_silent_download();
    bool is_need_merge();
    ImageInfo get_image_info();
    void get_layerdisk_info(LayerDiskInfo & layer_info);
    bool get_image_download_status();
    string get_local_image_base_name();

    int clear_inst();
    int clear_layer();
    int clear_layer_templete();
    int clear_imginfo();
    int clear_base();
    int remove_unnecessary_base(string name);
    int rename_base(string oldname, string newname);
    int clear_teacher_disk();
    int clear_print_disk(const string &disk_name);
    int clear_hostname_conf();
    int clear_all();
    int stop_download_image();

    int check_local_base_exist();
    bool check_personal_img_exist();
    bool check_layer_disk_exist();
    bool check_layer_tmplete_exist();
    bool check_file_md5(const string &file);
    bool check_usedisk_exist();
    bool check_expand_disk_exist(const string &disk_name);
    bool start_bt_service();
    bool stop_bt_service();
    int check_local_base_status();
    int start_usb_image();
    int stop_usb_image();
    bool is_usb_downloading();
    int get_usb_base_size(const string& basename);

private:
    static void* thread_main(void* data);
    bool download_time_out();
    
    int update_download_status();
    void update_torrent_file(string btfile);
    string get_download_percent();
    string get_download_rate();
    string calculte_file_md5();
    void get_json_valueint(int& dst, cJSON* json, const string& key);
    void get_json_valuestring(string& dst, cJSON* json, const string& key);
    
private:
    Application* _app;
    ProcessLoop _process_loop;
    AnalyzeStatusTimer* _analyzeStatusTimer;
    UnjoinableThread* _thread;
    
    string _image_commond_path;
    string _bt_torrent_path;
    string _bt_file;
    string _bt_torrent_file;
    string _bt_share_name;//torrent share name

    string _clear_inst;
    string _clear_layer;
    string _clear_layer_tmplete;
    string _clear_layer_tmplete_md5;
    string _clear_base;
    string _clear_torrent;
    string _clear_teacher_disk;
    string _clear_print_disk;
    string _clear_print_path;
    string _clear_hostname_conf;
    string _clear_image_info_ini;
    string _clear_network_info_ini;
    string _clear_bt_log;

    //base download info
    string _download_percent_latest;
    int _time_passed;
    int _TIME_OUT;
    string _download_percent;
    string _download_rate;
    string _download_downloading;
    string _download_checking;
    string _download_merge_error;
    string _download_total_size;
    string _downloaded_size;
    int _nospace_flag;
    string _md5;
    int _analyze_interval;
    bool _download_quit;
    DownloadInfo _downloadInfo;
    string _cmd_base_num;
    string _cmd_personal_img_num;
    string _cmd_layer_templete_num;
    string _cmd_layer_templete_unpack;
    string _cmd_print_templete_unpack;
    string _cmd_have_usedisk;
    string _cmd_have_layer_disk;
    ImageInfoDB _imageData;
    ImageInfo _imageInfo;

    UsbManage* _usb_manage;
    string _mount_dev;
    bool _usb_downloading;
    bool _first_silent;
    int _download_success_tips;

friend class ImageManage::StartDownloadImageEvent;
friend class ImageManage::QuitDownloadImageEvent;
friend class ImageManage::AnalyzeStatusTimer;
    class StartDownloadImageEvent:public Event
    {
        public:
            StartDownloadImageEvent(ImageManage* imageManage,ImageInfo* imageInfo):_imageManage(imageManage),_imageInfo(imageInfo)
                {
                }
            virtual ~StartDownloadImageEvent(){}
            virtual void response()
            {

            	LOG_INFO("===============================");
                _imageManage->start_download_image(_imageInfo);
            }
        private:
            ImageManage* _imageManage;
            ImageInfo* _imageInfo;
    }; 

    class QuitDownloadImageEvent:public Event
    {
        public:
            QuitDownloadImageEvent(ImageManage* imageManage):_imageManage(imageManage){}
            virtual ~QuitDownloadImageEvent(){}
            virtual void response()
            {
                _imageManage->quit_download_image();
            }
        private:
           ImageManage* _imageManage; 
    };

    class AnalyzeStatusTimer:public Timer
    {
        public:
            AnalyzeStatusTimer(ImageManage* imageManage):_imageManage(imageManage) {}
            virtual ~AnalyzeStatusTimer(){}
            virtual void response()
            {
                _imageManage->analyze_download_status();
            }

        private:
            ImageManage* _imageManage;
    };
};

#endif

