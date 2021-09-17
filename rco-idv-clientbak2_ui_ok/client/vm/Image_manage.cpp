#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include "rc_json.h"
#include <vector>
#include <regex>

#include "Image_manage.h"
using namespace std;
const string	IMAGE_DIR = "/opt/lessons/";

ImageManage::ImageManage(Application* app):_app(app),_process_loop(this),_analyzeStatusTimer(new AnalyzeStatusTimer(this)),_imageData(this)
{
   //_image_commond_path   = "/usr/local/BitTornado/Images_Client.sh";
   _image_commond_path   = "python /usr/local/BitTornado/bt_proxy.py";
   _bt_torrent_path      = "/opt/lessons";
   _bt_file              = "";
   _bt_torrent_file      = "";
   _bt_share_name        = "baseDownload";
   _download_percent     = "";
   _download_rate        = "";
   _download_downloading = "";
   _download_checking    = "";
   _download_merge_error = "";
   _downloaded_size       = "";
   _download_total_size   = "";
   _md5                  = "";
   _download_quit        = false;//cancel download image base if true
   _analyze_interval     = 1000;//each 1s return download info

   _clear_inst                = "rm -rf /opt/lessons/*.img";
   _clear_layer               = "rm -rf /opt/lessons/*.layer";
   _clear_layer_tmplete       = "rm -f " RCC_LAYER_PATH "layer_templete";
   _clear_layer_tmplete_md5   = "rm -f " RCC_LAYER_PATH "layer_templete.md5";
   _clear_base                = "rm -rf /opt/lessons/*.base";
   _clear_torrent             = "rm -rf /opt/lessons/*.torrent";
   _clear_teacher_disk        = "rm -rf /opt/lessons/data.teacher.disk";
   _clear_print_disk          = "rm -f /opt/lessons/w.disk";
   _clear_print_path          = "rm -rf /opt/lessons/print/";
   _clear_hostname_conf       = "rm -rf /opt/lessons/acpitable.conf";
   _clear_image_info_ini      = "rm -rf " RCC_DATA_PATH "vm_image_info.ini";
   _clear_network_info_ini    = "rm -rf " RCC_DATA_PATH "vm_network_info.ini";
   _clear_bt_log              = "rm -rf /var/log/Images_sharebt_" + _bt_share_name + "_stdout.log";
   _cmd_base_num              = "ls /opt/lessons/*.base | wc -l";
   _cmd_personal_img_num      = "ls /opt/lessons/*.img | grep -v '/opt/lessons/guest.img' | wc -l";
   _cmd_layer_templete_num    = "ls /opt/lessons/layer/layer_templete | wc -l";
   _cmd_layer_templete_unpack = "tar zxvf /etc/RCC_Client/layer/layer_templete.tgz -C /opt/lessons/";
   _cmd_print_templete_unpack = "tar zxvf /etc/RCC_Client/layer/print.tgz -C /opt/lessons/";
   _cmd_have_usedisk          = "ls /opt/lessons/data.teacher.disk | wc -l";
   _cmd_have_layer_disk       = "ls /opt/lessons/*.layer | grep -v '/opt/lessons/guest.layer' | wc -l";
   _time_passed               = 0;
   _TIME_OUT                  = 300;
   _download_percent_latest   = "";
   _download_success_tips     = 1;

   _thread = new UnjoinableThread(ImageManage::thread_main, this);
   _usb_manage = new UsbManage();
   _mount_dev.clear();
   _usb_downloading = false;
}

ImageManage::~ImageManage()
{
    _analyzeStatusTimer->unref();
    _thread->cancel();
    delete _thread;
}

void ImageManage::action()
{
    cout << "test" << endl;
}

/*
**thread main
*/
void* ImageManage::thread_main(void* data)
{
    ImageManage* imageManage = (ImageManage*)data;
	imageManage->_process_loop.run();
	return NULL;
}

/*
**push quit download image event
*/
bool ImageManage::vm_quit_download_image()
{
	int ret;
    QuitDownloadImageEvent* event = new QuitDownloadImageEvent(this);
    ret = _process_loop.push_event(event);
    event->unref();
    if (ret < 0){
    	return false;
    }
    else{
    	return true;
    }
}

/*
**quit download Image
*/
void ImageManage::quit_download_image()
{
	LOG_INFO("req quit download image");
	string cmd_stop_bt = _image_commond_path + " stop " +_bt_torrent_path+"/"+get_new_base_name()+" false";
    if (is_silent_download()){
        cmd_stop_bt = _image_commond_path + " stop " +_bt_torrent_path+"/diff/"+get_new_base_name()+" false";
	}
	else if (DOWNLOAD_IMAGE_MERGING==get_download_mode())
	{
	    LOG_INFO("merging image!"); 
	    return;
	}
	string stop_bt = execute_command(cmd_stop_bt);
    if(stop_bt == "error")
    {
        LOG_WARNING("stop bt fail after quitting downloading the image base!");
	    _app->vm_download_quit_status(QOIT_FAIL);
    }else{
        //clear_base();
        //clear_hostname_conf();

	    LOG_INFO("cancel download image success!");
        _process_loop.deactivate_interval_timer(_analyzeStatusTimer);
		_app->vm_download_quit_status(QUIT_SUCCESS);
	}
}

bool ImageManage::unpack_layer_templete()
{
    string result = execute_command(_cmd_layer_templete_unpack);

    if (result == "error") {
        LOG_ERR("unpack_layer_templete fail!");
        return false;
    } else {
        return true;
    }
}

bool ImageManage::unpack_print_templete()
{
    string result = execute_command(_cmd_print_templete_unpack);

    if (result == "error") {
        LOG_ERR("unpack_print_templete fail!");
        return false;
    } else {
        return true;
    }
}

/*
** clear the /opt/lessons/ *.inst
*/
int ImageManage::clear_inst()
{
    string result = execute_command(_clear_inst);
    if(result == "error")
    {
        LOG_ERR("clear inst of guest fail!")
        return -1;
    }else{
        return 0;
    }
}

int ImageManage::clear_layer()
{
    string result = execute_command(_clear_layer);

    if (result == "error") {
        LOG_ERR("clear layer fail!")
        return -1;
    } else {
        return 0;
    }
}

int ImageManage::clear_layer_templete()
{
    string result_tmplete = execute_command(_clear_layer_tmplete);
    string result_tmplete_md5 = execute_command(_clear_layer_tmplete_md5);

    if (result_tmplete == "error" || result_tmplete_md5 == "error") {
        LOG_ERR("clear_layer_templete fail!")
        return -1;
    } else {
        return 0;
    }
}

int ImageManage::clear_imginfo()
{
	LOG_INFO("clear image ini file!");
    string result = execute_command(_clear_image_info_ini);
    if(result == "error")
    {
        return -1;
    }else{
        return 0;
    }
}

/*
**clear /opt/lessons/ *.base
*/
int ImageManage::clear_base()
{
	LOG_INFO("clear base!");
    string result = execute_command(_clear_base);
    int result_clear_ini = clear_imginfo();
    if(result == "error" || result_clear_ini == -1)
    {
        LOG_ERR("clear base fail!")
        return -1;
    }else{
        return 0;
    }
}


/*
** after production, there maybe two build-in golden images, remove one
*/
int ImageManage::remove_unnecessary_base(string exclude)
{
    string all;
    string result;
    std::regex ws_re("\\s+");

    /* such as 'Removing base exclude RCO_Win7_64_V004.base' */
    LOG_INFO("Removing base exclude %s!", exclude.c_str());

    all = execute_command("ls " + IMAGE_DIR + "*.base");

    LOG_INFO("all base file are <%s>.\n", all.c_str());

    if(all == "error" || all.empty()){
        LOG_INFO("No any base images found\n");
        return -1;
    }

    std::vector<std::string> v(std::sregex_token_iterator(all.begin(), all.end(), ws_re, -1), std::sregex_token_iterator() );

    if (v.size() != 2) {
        LOG_INFO("only process the situation where two golden images exist, count is %d.\n", v.size());
        return -1;
    }

    /* s is '/opt/lessons/xx.base' */
    for(auto&& s: v) {
        LOG_INFO("iterator: current images is %s\n", s.c_str());
        if (s != (IMAGE_DIR + exclude)) {
            result = execute_command("rm " + s);
            if(result == "error"){
                LOG_ERR("Removing base %s meets error\n", s.c_str());
            }else{
                LOG_INFO("Base %s is removed with result %s\n", s.c_str(), result.c_str());
            }
        } else {
            LOG_INFO("Image name is the same, keep %s\n", s.c_str());
        }
    }

    return 0;
}



int ImageManage::rename_base(string oldname, string newname)
{
	string ret;
	string local_base;
	if (newname.empty()) {
		LOG_ERR("new name is null!")
		ret = "error";
		goto end;
	}

	if (!oldname.empty()) {
		ret = execute_command("mv " + IMAGE_DIR + oldname + " " + IMAGE_DIR + newname);

	} else {
		if (check_local_base_exist()) {
			local_base = execute_command("ls " + IMAGE_DIR + "*.base");

			ret = execute_command("mv " + local_base + " " + IMAGE_DIR + newname);
		} else {
			ret = "error";
		}
	}
end:
    if(ret == "error")
    {
        return -1;
    }else{
        return 0;
    }
}

/*
**clear /opt/lessons/data.teacher.disk
*/
int ImageManage::clear_teacher_disk()
{
	LOG_INFO("clear disk!");
    string result = execute_command(_clear_teacher_disk);
    if(result == "error")
    {
        LOG_ERR("clear disk fail!")
        return -1;
    }else{
        return 0;
    }
}

/*
**clear /opt/lessons/xxx.disk
*/
int ImageManage::clear_print_disk(const string &disk_name)
{
    LOG_INFO("clear_print_disk!");
    string result;
    string cmd = "rm -f /opt/lessons/" + disk_name;
    string cmd_print = "rm -rf /opt/lessons/print";

    result = execute_command(cmd);
    if (result == "error"){
        LOG_ERR("clear_print_disk fail!")
        return -1;
    } else {
        return 0;
    }

    result = execute_command(cmd_print);
    if (result == "error"){
        LOG_ERR("clear_print_disk fail!")
        return -1;
    } else {
        return 0;
    }
}


/*
**claer hostname conf
*/
int ImageManage::clear_hostname_conf()
{
	LOG_INFO("clear hostname conf!");
	string result = execute_command(_clear_hostname_conf);
	if(result == "error")
	{
		LOG_ERR("clear hostname conf fail!")
		return -1;
	}else{
		return 0;
	}
}

int ImageManage::clear_all()
{
    LOG_INFO("clear all!");
    string result_clear_inst = execute_command(_clear_inst);
    string result_clear_data_disk = execute_command(_clear_teacher_disk);
    string result_clear_print_disk = execute_command(_clear_print_disk);
    string result_clear_print_path = execute_command(_clear_print_path);
    string result_clear_layer = execute_command(_clear_layer);
    string result_clear_hostname_conf = execute_command(_clear_hostname_conf);

    if (result_clear_inst == "error" || result_clear_data_disk == "error" 
        || result_clear_hostname_conf == "error" || result_clear_print_disk == "error" 
        || result_clear_layer == "error" || result_clear_print_path == "error") {
        LOG_ERR("clear all fail!")
        return -1;
    } else {
        return 0;
    }
}

int ImageManage::stop_download_image()
{
    //FIXME:we do not need do this if we are not downloading
	string cmd_stop_bt = _image_commond_path + " stop " + _bt_torrent_path+"/"+get_new_base_name()+" false";
	if (is_silent_download()){
        cmd_stop_bt = _image_commond_path + " stop " +_bt_torrent_path+"/diff/"+get_new_base_name()+" false";
	}
	string stop_bt = execute_command(cmd_stop_bt);
	if(stop_bt == "error")
	{
		LOG_WARNING("stop bt fail!");
		return -1;
	}else{
		LOG_INFO("stop bt success!");
		//deactivate _analyzeStatusTimer after stopping download
		_process_loop.deactivate_interval_timer(_analyzeStatusTimer);
		return 0;
	}
}

/*
**calculte file md5
*/
string ImageManage::calculte_file_md5()
{
	LOG_INFO("CHECK IMAGE MD5...");
    string cmd_calculte_file_md5 = "md5sum " + _bt_torrent_path + "/" + _bt_file + ".base";
	string full_md5_info = execute_command(cmd_calculte_file_md5);
    if(full_md5_info == "error")
    {
        LOG_WARNING("calculte file md5 fail!");
	    return "error";	
    }
	int whitespaceLocation = full_md5_info.find(" ");
	string file_md5 = full_md5_info.substr(0,whitespaceLocation);
	LOG_INFO("file_md5:%s",file_md5.c_str());
	return file_md5;
}

/**
* calcute application file md5 is ok or not
* @ layer_file: layer templete file absolute path 
*
* return true: check md5 is ok; false: check md5 fail
*/
bool ImageManage::check_file_md5(const string &file)
{
    LOG_INFO("CHECK MD5...");
    string cmd_file_md5, cmd_md5;
    string file_md5, file_md5_value;

    cmd_file_md5 = "md5sum -c " + file + ".md5";
    file_md5 = execute_command(cmd_file_md5);
    if (file_md5 == "error") {
        LOG_ERR("error full_md5_info:%s", file.c_str());
        return false;
    }

    return true;
}


/*
**analyze download status:download speed,download percent
*/
int ImageManage::update_download_status()
{
    cJSON* json_out ;
    //LOG_INFO(" get_new_base_name:%s ",get_new_base_name().c_str());
    string cmd_download_status = _image_commond_path + " status "+_bt_torrent_path+"/" + get_new_base_name().c_str();
    if  (is_silent_download())
    {
        cmd_download_status = _image_commond_path + " status "+_bt_torrent_path+"/diff/" + get_new_base_name().c_str();
    }
    else if (is_need_merge())
    {
        cmd_download_status = _image_commond_path + " status "+_bt_torrent_path+"/" + _imageData.get().name.c_str();
    }
    string full_download_status = execute_command(cmd_download_status);
    LOG_DEBUG("bt_proxy status := %s  ",full_download_status.c_str());
    //string full_download_status = "{}";
    if(full_download_status == "error")
    {
        LOG_WARNING("get download status error!");
        return -1;
    }
	_nospace_flag                         = 0;
    string status_flag ;
    
    if (2==full_download_status.find("status",0))   //return json correct format
    {
        cJSON* json = cJSON_Parse(full_download_status.c_str());
        
        json_out = cJSON_GetObjectItem(json, "status");
        if (json_out == NULL) {
            cJSON_Delete(json);
            LOG_ERR("get cJSON status failed");
        }
        status_flag = json_out->valuestring;
        json_out = cJSON_GetObjectItem(json, "download_speed");
        if (json_out == NULL) {
            cJSON_Delete(json);
            LOG_ERR("get cJSON download_speed failed");
        }
        _download_rate = json_out->valuestring;
        
        json_out = cJSON_GetObjectItem(json, "progress");
        if (json_out == NULL) {
            cJSON_Delete(json);
            LOG_ERR("get cJSON _download_percent failed");
        }
        _download_percent = json_out->valuestring;
        
        json_out = cJSON_GetObjectItem(json, "download_size");
        if (json_out == NULL) {
            cJSON_Delete(json);
            LOG_ERR("get cJSON download_size failed");
        }
        _downloaded_size = json_out->valuestring;
    }
    
   
    if (is_silent_download() && ((strcmp(status_flag.c_str(),"downloading")==0)||(strcmp(status_flag.c_str(),"finished")==0)))
    {
		_app->send_download_progress_info_to_zhjsgt(_downloaded_size,_download_total_size,_download_percent,_download_rate);
    }    
    if (strcmp(status_flag.c_str(),"merge_error") == 0)
    {
        _download_merge_error = "1";
    }
    else
    {
        _download_merge_error = " ";
    }
    
    if (strcmp(status_flag.c_str(),"checking") == 0)
    {
        _download_checking = "1";
    }
    else
    {
        _download_checking = " ";
    }

    LOG_DEBUG("status = %s  _download_rate = %s _download_percent = %s",status_flag.c_str(), _download_rate.c_str(), _download_percent.c_str());

   

    //fix _download_rate,the last . should be removed such as 123.MB/s
    int len = _download_rate.length();
    if(len > 3 && _download_rate[3] == '.')
    {
        _download_rate.erase(3,1);
    }
    //replace kB/s to KB/s
    if(len >= 4 && _download_rate[len - 4] == 'k')
    {
        _download_rate[len - 4] = 'K';
    }
    
    /*if (_download_percent.length()>5)
    {
       _download_percent = _download_percent.substr(0,5);
    }*/
	
	_downloadInfo.percent_double = strtod(_download_percent.c_str(), NULL);

    _downloadInfo.percent = _download_percent;
    _downloadInfo.speed   = _download_rate;

	LOG_INFO("_download_percent:%s",_download_percent.c_str());
	LOG_INFO("_download_rate:%s",_download_rate.c_str());
	return 0;
}

/*
**compare download percent latest with current
**if _time_passed > _TIME_OUT, time out, download fail
*/
bool ImageManage::download_time_out()
{
    if(_download_percent_latest != _download_percent){
    	_download_percent_latest = _download_percent;
    	_time_passed = 0;
    }else{
    	_time_passed = _time_passed + 1;
    	LOG_INFO("download _time_passed:%d!",_time_passed);
    	if((_time_passed > _TIME_OUT)&&(get_download_mode()==DOWNLOAD_IMAGE_FORCE_DOWNLOAD)){
    		_time_passed = 0;
    		LOG_INFO("download time out!");
    		return true;
    	}
    }
    return false;
}

/*
**get download status:download percent
*/
string ImageManage::get_download_percent()
{
    return _download_percent;
}

/*
**get download status:download rate
*/
string ImageManage::get_download_rate()
{
    return _download_rate;
}

/*
**update torrent file
*/
void ImageManage::update_torrent_file(string btfile)
{
    _bt_torrent_file = btfile;
}

/*
**push start download image event
*/
bool ImageManage::vm_start_download_image(ImageInfo* imageInfo,string baseName, string torrentName)
{
	_bt_file         = baseName;
	_bt_torrent_file = torrentName;
	_imageInfo = *imageInfo;

    StartDownloadImageEvent* event = new StartDownloadImageEvent(this, &_imageInfo);

    int ret = _process_loop.push_event(event);
    event->unref();
    if (ret < 0){
    	return false;
    }
    else{
    	return true;
    }
}

int ImageManage::get_user_disk_size()
{
    return get_image_virt_size("data.teacher.disk");
}

int ImageManage::get_expand_disk_size(const string &disk_name)
{
    return get_image_virt_size(disk_name);
}
/*
int ImageManage::get_c_disk_size()
{
	int size = 0;
	int nPos;
	string image;
	string basename;

	image = _imageData.get().name;

	nPos = image.find_first_of(".");
	basename = image.substr( 0 , nPos);

	size = get_image_virt_size(basename + ".img");
	if (size > 0) {
		return size;
	}

	return get_image_virt_size(image);
}
*/
bool ImageManage::create_c_disk(string image_name, int size)
{
    string base_image_path;
    string image_path;
    string result;

    image_path = IMAGE_DIR + image_name;
    base_image_path = IMAGE_DIR + _imageData.get().name;

    if (!get_file_exist(image_path.c_str())) {
        // vmmanager --create_base_diff qcow2 backingfile.base diff.img
        result = execute_command("vmmanager --create_base_diff qcow2 " + base_image_path + " " + image_path);
        if (result == "error") {
            return false;
        }
    }

    if (get_image_virt_size(image_name) <= 0) {
        return false;
    }

    // local size > set size
    if (get_image_virt_size(image_name) > size) {
        return true;
    }

    return resize_image(image_name, size, "qcow2");
}

bool ImageManage::create_layer_disk(string layer_name, int size)
{
    string layer_file;
    string base_layer_path;
    string result;

    layer_file = IMAGE_DIR + layer_name;
    base_layer_path = RCC_LAYER_FILE;

    // vmmanager --create_base_diff qcow2 backingfile.base diff.img
    if (!get_file_exist(layer_file.c_str())) {
        result = execute_command("vmmanager --create_base_diff qcow2 " + base_layer_path + " " + layer_file);
        if (result == "error") {
            return false;
        }
    }

    if (get_image_virt_size(layer_name) <= 0) {
        return false;
    }

    // local size > set size
    if (get_image_virt_size(layer_name) > size) {
        return true;
    }

    return resize_image(layer_name, size, "qcow2");
}


bool ImageManage::create_user_disk(int resize)
{
    int size;
    //string cmd;
    string name = "data.teacher.disk";
    string result;
    string path = IMAGE_DIR + name;

    if (resize <= 0) {
        return true;
    }

    size = get_user_disk_size();
    if (size == 0) {
        // vmmanager --create_disk_file raw /opt/lessons/data.teacher.disk 25G true (ntfs) 
        result = execute_command("vmmanager --create_disk_file raw " + path + " " + to_string(resize) + "G" + " true");
        if (result == "error") {
            return false;
        }
        return true;
    } else if(resize > size) {
        return resize_image(name, resize, "raw");
    }
    return true;
}

/**
* function : create expand disk for raw fmt 
* disk_name: the name of disk 
* resize: the size of disk you want to set
*
* return: false: create or resize disk failed  true: create or resize disk success
*/
bool ImageManage::create_expand_disk(const string &disk_name, const int &resize)
{
    int size = 0;
    string path = IMAGE_DIR + disk_name;
    string result;

    if (resize <= 0) {
        return true;
    }

    size = get_expand_disk_size(disk_name);
    if (size == 0) {
        // vmmanager --create_disk_file raw /opt/lessons/data.teacher.disk 25G 
        result = execute_command("vmmanager --create_disk_file raw " + path + " " + to_string(resize) + "M" + " false");
        if (result == "error") {
            return false;
        }
    } else if (resize > size) {
        return resize_image(disk_name, resize, "raw", "M");
    }

    return true;
}

int ImageManage::get_image_real_size(const string image)
{
    string path;
    string cmd;
    int real_size = 0;
    double size = 0.0;
    char unit;

    path = IMAGE_DIR + image;

    if (!get_file_exist(path.c_str())) {
        return 0;
    }

    cmd = "ls -lh " + path + " | awk '{print $5}'";

    string result = execute_command(cmd);
    LOG_INFO("get image %s real size is %s", path.c_str(), result.c_str());
    if(result == "error")
    {
        LOG_INFO("get disk real size fail!");
        return 0;
    } else {
        if (result.size() <= 1) {
            return 0;
        }
        unit = result.back();
        if (tolower(unit) == 'k') {
            result.erase(result.size()-1);
            size = atof(result.c_str());
            real_size = (int)ceil(size / (1024*1024));
        } else if (tolower(unit) == 'm') {
            result.erase(result.size()-1);
            size = atof(result.c_str());
            real_size = (int)ceil(size / 1024);
        } else if (tolower(unit) == 'g') {
            result.erase(result.size()-1);
            real_size = (int)ceil(atof(result.c_str()));
        } else {
            real_size = 0;
        }
        return real_size;
    }

    return 0;
}

/**
* function : get real size of disk 
* image : the name of disk 
* 
* return: the real size of disk
*/
int ImageManage::get_image_size(const string image, bool is_real_size)
{
    string path;
    string cmd;
    string result;
    char unit;
    double size = 0.0;
    int real_size = 0;

    path = IMAGE_DIR + image;
    if (!get_file_exist(path.c_str())) {
        return 0;
    }

    if (is_real_size) {
        cmd = "vmmanager --get_image_real_size " + path;
    } else {
        cmd = "vmmanager --get_image_virtual_size " + path;
    }

    result = execute_command(cmd);
    LOG_INFO("get image %s disk size is %s", path.c_str(), result.c_str());
    if (result == "error") {
        LOG_INFO("get disk size fail!");
        return 0;
    } else {
        if (result.size() <= 1) {
            return 0;
        }

        unit = result.back();
        result.erase(result.size()-1);
        size = atof(result.c_str());

        if (tolower(unit) == 'k') {
           real_size = (int)ceil(size / (1024*1024));
        } else if (tolower(unit) == 'm') {
           real_size = (int)ceil(size / 1024);
        } else if (tolower(unit) == 'g') {
           real_size = (int)ceil(size);
        } else {
           real_size = 0;
        }
        LOG_DEBUG("real_size:%d", real_size);
        return real_size;
    }

    return 0;
}

int ImageManage::get_image_virt_size(const string image)
{
    string path;
    string cmd;
    string result;

    path = IMAGE_DIR + image;

    if (!get_file_exist(path.c_str())) {
        return 0;
    }

#if 0
// XXX: if file is xxx.disk 格式 下面语句执行都是失败的
    cmd = "qemu-img check " + path;
    result = execute_command(cmd);

    if (result == "error") {
        LOG_INFO("disk check is failed!");
        return 0;
    }
#endif

    cmd = "";
    // vmmanager --get_image_virtual_size xxx.img/xxx.base/xxx.disk
    cmd = "vmmanager --get_image_virtual_size " + path;

    result = ""; 
    result = execute_command(cmd);
    LOG_INFO("get image %s size is %s", path.c_str(), result.c_str());
    if(result == "error")
    {
        LOG_INFO("get disk size fail!");
        return 0;
    } else {
        if (result.size() > 0) {
            result.erase(result.size()-1);
            return atoi(result.c_str());
        }
    }

    return 0;
}

int ImageManage::get_image_default_virt_size(const string image_name)
{
    if (check_personal_img_exist())
    {
        //get virt size of personal img
        int nPos = image_name.find_first_of(".");
        string img_name = image_name.substr(0 , nPos) + ".img";
        return get_image_virt_size(img_name);
    }
    else
    {
        //get virt size of personal base
        return get_image_virt_size(image_name);
    }
}

void ImageManage::update_image_info(ImageInfo* imageInfo)
{
	bool status;
	status = _imageData.get_download_status();
	_imageData.set(*imageInfo, status);
}

void ImageManage::update_image_ostype(string *ostype)
{
    _imageData.set_ostype(*ostype);
}

/**
*function: found if ostype is exsit and get it value
*
*return 0:not exsit 1:exsit
*/
int ImageManage::get_image_ostype(string &ostype)
{
    return _imageData.get_ostype(ostype);
}

void ImageManage::clean_download_success_tips()
{
    _download_success_tips = 0;
}

void ImageManage::set_download_mode(int mode)
{
    _imageData.set_download_mode(mode);
}

int ImageManage::get_download_mode()
{
    return _imageData.get_download_mode();
}

void ImageManage::set_new_base_name(string name)
{
    _imageData.set_new_base_name(name);
}

string ImageManage::get_new_base_name()
{
    return _imageData.get_new_base_name();
}

void ImageManage::set_new_base_version(string version)
{
    _imageData.set_new_base_version(version);
}

string ImageManage::get_new_base_version()
{
    return _imageData.get_new_base_version();
}

void ImageManage::set_new_base_id(int id)
{
    _imageData.set_new_base_id(id);
}

int ImageManage::get_new_base_id()
{
    return _imageData.get_new_base_id();
}

bool ImageManage::is_silent_download()
{
    return _imageData.is_silent_download();
}

bool ImageManage::is_need_merge()
{
    return _imageData.is_need_merge();
}


bool ImageManage::resize_image(const string image, const int new_size, string fmt, string unit)
{
	int size;
	string cmd;
	string path;
	string ret;


	path = IMAGE_DIR + image;

	if (new_size <= 0) {
		return false;
	}

	size = get_image_virt_size(image);

    LOG_INFO("!!!!!! resize_image image %s, old size %d; new size %d; fmt %s", image.c_str(), size, new_size, fmt.c_str());
    if (size > new_size) {
        return false;
    }

    if (size == new_size) {
        return true;
    }
    // vmmanager --resize_image_file qcow2 xxx.img 60G
    cmd = "vmmanager --resize_image_file " + fmt + " " + path + " " + to_string(new_size) + unit;

    ret = execute_command(cmd);
    LOG_DEBUG("resize_image fmt %s cmd %s ret %s", fmt.c_str(), cmd.c_str(), ret.c_str());
    if (ret == "error") {
        return false;
    }
    if (fmt == "raw") {
        cmd = "echo y | /sbin/ntfsresize -f " + path;
        ret = execute_command(cmd);
        if (ret == "error") {
            return false;
        }
    }

	return true;
}

bool ImageManage::download_torrent(const string torrent_file, const string url)
{
	string wget_result;
    string torrent_name;

    if (torrent_file.empty()) {
    	torrent_name = execute_command("basename " + url);
    } else {
    	torrent_name = torrent_file;
    }
    wget_result = " ";


	LOG_INFO("download_torrent, get_download_mode : %d", get_download_mode());
    if (is_silent_download())
    {
        wget_result = execute_command("wget --tries=1 --connect-timeout=3 --read-timeout=40 -O " + _bt_torrent_path +  "/diff/" + torrent_name + " " + url);
    }
    else { 
	    execute_command(_clear_torrent);
        wget_result = execute_command("wget --tries=1 --connect-timeout=3 --read-timeout=40 -O " + _bt_torrent_path + "/" + torrent_name + " " + url);
    
    }
	
	if(wget_result == "error")
	{
		return false;
	}
	return true;
}



int ImageManage::get_image_diff_size( string new_torrent, string old_torrent)
{
	string wget_result;
    string torrent_name;
    string torrent_oldname;
    char total_size[7];
    
    torrent_name = execute_command("basename " + new_torrent);
    torrent_oldname = execute_command("basename " + old_torrent);
	LOG_INFO("download_new_torrent=%s,old_torrent=%s",new_torrent.c_str(),old_torrent.c_str());
    wget_result = execute_command("wget --tries=1 --connect-timeout=3 --read-timeout=40 -O " + _bt_torrent_path +  "/diff/" + torrent_name + " " + new_torrent);
	if(wget_result == "error")
	{
		return 0;
	}
	/*wget_result = execute_command("wget --tries=1 --connect-timeout=3 --read-timeout=40 -O " + _bt_torrent_path +  "/" + torrent_oldname + " " + old_torrent);
	if(wget_result == "error")
	{
		return 0;
	}*/
	
	string cmd_get_diff_size = _image_commond_path + " diff_size " + _bt_torrent_path + "/diff/"+ torrent_name + " " + _bt_torrent_path + "/" + torrent_oldname;
	
	string diff_size = execute_command(cmd_get_diff_size);
	
    if(diff_size == "error")
    {
        LOG_WARNING("get_image_diff_size fail!");
        return 0;
    }
	
	LOG_INFO("execute_command bt_proxy diff_size:%s ",diff_size.c_str());
    
    
    if(diff_size.find("massage")  == string::npos)
    {
        LOG_WARNING("get_image_diff_size return json have not massage!");
        return 0;
    }
    
    if(diff_size.find("size")  == string::npos)
    {
        
        LOG_WARNING("get_image_diff_size return json have not size!");
        return 0;
    }
	
    cJSON* json = cJSON_Parse(diff_size.c_str());
	cJSON* json_out = cJSON_GetObjectItem(json, "massage");
    if (json_out == NULL) {
        cJSON_Delete(json);
        LOG_ERR("get cJSON massage failed");
    }
    string massage_flag = json_out->valuestring;
    if ("success" == massage_flag){
        json_out = cJSON_GetObjectItem(json, "size");
        if (json_out == NULL) {
            cJSON_Delete(json);
            LOG_ERR("get cJSON diff_size failed");
        }
        if (json_out->valueint<1024){
            sprintf(total_size,"%dKB",((int)json_out->valueint));
        }
        else if (json_out->valueint<(1024*1024)) {
            sprintf(total_size,"%dMB",((int)json_out->valueint)/1024);
        }            
        else {
            sprintf(total_size,"%.1fGB",((float)json_out->valueint)/(1024*1024));
        }
        LOG_INFO("total_size=%s",total_size);
        _download_total_size = total_size;
        return json_out->valueint;
    }
    return 0;
}
/*
**download image base from server
*/
bool ImageManage::start_download_image(ImageInfo* imageInfo)
{
	if(imageInfo->id == _imageData.get().id && imageInfo->version == _imageData.get().version)
	{
		LOG_WARNING("Local base is same with the server one, do not need to download. Checked the logic func!");
	}

    LOG_INFO("start download image, image_id: %d, image_name: %s, image_version: %s", imageInfo->id, imageInfo->name.c_str(), imageInfo->version.c_str());
	_downloadInfo.image_id    = imageInfo->id;
    _downloadInfo.image_name  = imageInfo->name;
	_downloadInfo.image_version = imageInfo->version;
	
    string new_torrent_abspath = _bt_torrent_path+"/diff/"+_bt_torrent_file + " ";
    string old_torrent_abspath = _bt_torrent_path + "/" + execute_command("basename " + _imageData.get().torrent_url) + " ";
    string base_abspath = _bt_torrent_path + "/" + _imageData.get().name + " ";
    string diff_dir_abspath = _bt_torrent_path+"/diff/";

	if (!download_torrent(_bt_torrent_file, imageInfo->torrent_url)) {
		LOG_WARNING("download torrent file fail!");
		_downloadInfo.status = DOWNLOAD_IMAGE_ERROR;
		_app->vm_download_progress_status(&_downloadInfo);
        return false;
	}

/*
    //stop last bt service before download new image base
    string cmd_stop_bt  = _image_commond_path + " stop " + _bt_torrent_path+"/"+_imageData.get().name+" false";
    //string cmd_clean_bt = _image_commond_path + " clean";
    string stop_bt      = execute_command(cmd_stop_bt);
    //string clear_bt_log = execute_command(_clear_bt_log);
    //string clean_bt     = execute_command(cmd_clean_bt);
    if(stop_bt == "error")
    {
        LOG_WARNING("stop bt fail before downloading the image base!");
        //return false;
        //curent time
    }

	*/
    //incremental download if there is one base in the path and has the same name with current download base
	//if base name is different,delete base then download
	string cmd_download_base = _image_commond_path + " start " + _bt_torrent_path + " " +_bt_torrent_path + "/" + _bt_torrent_file ;
	
    set_new_base_name(imageInfo->name);
    set_new_base_version(imageInfo->version);
    set_new_base_id(imageInfo->id);
	if (DOWNLOAD_IMAGE_FORCE_DOWNLOAD==get_download_mode())
	{
        //clear inst before download base
        clear_inst();
	    if(_imageData.get().name != imageInfo->name)
        {
        	LOG_INFO("base name local:%s is different from server:%s",_imageData.get().name.c_str(),imageInfo->name.c_str());
        	_imageData.delete_image_info();
        	rename_base(_imageData.get().name, imageInfo->name);
        }
        //set download status to ini
        _imageData.set_download_status(true);
        _imageData.set_download_basename(imageInfo->name);
	}
	else if(is_silent_download())
    {
        LOG_WARNING("diff_start download!");
        //diff_start new_torrent_abspath old_torrent_abspath base_abspath diff_dir_abspath 
	     cmd_download_base = _image_commond_path + " diff_start " + new_torrent_abspath + old_torrent_abspath+ base_abspath+diff_dir_abspath +" 80000 ";
         _first_silent = true;
         _imageData.set_download_status(true);
        //_imageData.set_download_basename(imageInfo->name);
    }
    else if(is_need_merge())
    {
        LOG_WARNING("start merge!");
        set_download_mode(DOWNLOAD_IMAGE_MERGING);
        _imageData.set_download_status(true);
        cmd_download_base = _image_commond_path + " merge " + base_abspath  + diff_dir_abspath + imageInfo->name.c_str() ;
    }

	LOG_INFO("get_new_base_name::%s ",get_new_base_name().c_str());
	string download_base = execute_command(cmd_download_base);
	LOG_INFO("execute_command bt_proxy start:%s ",download_base.c_str());
    sleep(2);
    if(download_base == "error")
    {
        LOG_WARNING("downloading image base fail!");
        _downloadInfo.status = DOWNLOAD_IMAGE_ERROR;
        _app->vm_download_progress_status(&_downloadInfo);
        return false;
    }
    _time_passed = 0;
	//each _analyze_interval update download info
	_process_loop.activate_interval_timer(_analyzeStatusTimer,_analyze_interval);
	return true;
}

/*
**analyze download status
*/
void ImageManage::analyze_download_status()
{
    if(_usb_downloading)
    {
        string cp_percent;
        string cp_rate;
        int result = _usb_manage->update_status2(cp_percent, cp_rate, _mount_dev);
        if(result == CP_BASE_UPDATE_STATUS_SUCCESS)
        {
            _downloadInfo.status = DOWNLOAD_COPING_USB_IMAGE;
            _downloadInfo.percent = cp_percent;
            _downloadInfo.speed   = cp_rate;
            _downloadInfo.percent_double = strtod(cp_percent.c_str(), NULL);
            _app->vm_download_progress_status(&_downloadInfo);
            if (_downloadInfo.percent != "100%") {
                //continue copy base
                return;
            }
            LOG_INFO("copy usb image success");
            stop_usb_image();
        }
        else
        {
            if(result == CP_BASE_MOUNTED_DEV_NOT_EXIST)
                _downloadInfo.status = DOWNLOAD_CP_BASE_MOUNTED_DEV_NOT_EXIST;
            if(result == CP_BASE_UPDATE_STATUS_ERROR)
                _downloadInfo.status = DOWNLOAD_CP_BASE_UPDATE_STATUS_ERROR; 
            if(result == CP_BASE_UPDATE_STATUS_BASE_READY)
                _downloadInfo.status = DOWNLOAD_CP_BASE_UPDATE_STATUS_BASE_READY; 
            LOG_INFO("coping usb image gets error, %d", _downloadInfo.status);
            _app->vm_download_progress_status(&_downloadInfo);
            stop_usb_image();
        }
        if (_app->is_dev_network_down()) {
            _app->vm_usb_download_disconnect();
            _process_loop.deactivate_interval_timer(_analyzeStatusTimer);
        } else {
            LOG_INFO("===============");
            LOG_INFO("after copy usb image, restart downloading image base");
            _process_loop.deactivate_interval_timer(_analyzeStatusTimer);
            start_download_image(&_imageInfo);
        }
        return;
    }
	//return error status if update_download_status is fail or download_time_out is true
	if (update_download_status() < 0 || download_time_out()){
		_downloadInfo.status = DOWNLOAD_IMAGE_TIMEOUT;
		_app->vm_download_progress_status(&_downloadInfo);

		_process_loop.deactivate_interval_timer(_analyzeStatusTimer);
		string cmd_stop_bt = _image_commond_path + " stop " + _bt_torrent_path+"/"+get_new_base_name()+" false";
		if (is_silent_download()){
            cmd_stop_bt = _image_commond_path + " stop " +_bt_torrent_path+"/diff/"+get_new_base_name()+" false";
	    }
		string stop_bt = execute_command(cmd_stop_bt);

		LOG_INFO("update_download_status < 0,_time_passed:%d",_time_passed);
		return;
	}

	//return nospace status if device is not space
	if (_nospace_flag > 0 ) {
		_downloadInfo.status = DOWNLOAD_IMAGE_NOSPACE;
		_app->vm_download_progress_status(&_downloadInfo);

		_process_loop.deactivate_interval_timer(_analyzeStatusTimer);
		string cmd_stop_bt = _image_commond_path + " stop " +_bt_torrent_path+"/"+get_new_base_name()+" true";
		if (is_silent_download()){
            cmd_stop_bt = _image_commond_path + " stop " +_bt_torrent_path+"/diff/"+get_new_base_name()+" true";
	    }
   
		string stop_bt = execute_command(cmd_stop_bt);

		LOG_INFO("DEVICE NO SPACE");
		return;
	}

	//if local base has same name with server one,bt will checking base first
	//if _download_checking is 1, return checking status
    
    if (is_silent_download())
    {
        if (_first_silent){
			LOG_INFO("set _downloadInfo.status = DOWNLOAD_IMAGE_SUCCESS");
			_first_silent = false;
            _imageData.set_download_status(false);
			_downloadInfo.status = DOWNLOAD_IMAGE_SUCCESS;
			_app->vm_download_progress_status(&_downloadInfo);
    	}
    	else{
    	    if(strcmp(_download_percent.c_str(),"100.00") == 0)
    		{
		        LOG_INFO("silent_download 100%");
		        execute_command("sync");
                set_download_mode(DOWNLOAD_IMAGE_NEED_MERGE);
                _app->prepare_merge_image();
                _download_success_tips = 1;
                //execute_command("reboot");
				//_imageInfo.md5 = "ok";
				//_imageData.set(_imageInfo,false);
    		}
    	}
    }
    else if (DOWNLOAD_IMAGE_FORCE_DOWNLOAD==get_download_mode())
    {
    	if (strcmp(_download_checking.c_str(),"1") == 0){
    		LOG_DEBUG("Checking image base for incremental download.");
    		_downloadInfo.status = DOWNLOAD_IMAGE_CHECKING;
    		_app->vm_download_progress_status(&_downloadInfo);

    		LOG_INFO("checking base!");
    	}else
    	{
    		if(strcmp(_download_percent.c_str(),"100.00") == 0)
    		{
    			//if _download_percent is 100.00%,stop _analyzeStatusTimer,return checking_md5 status
    			_process_loop.deactivate_interval_timer(_analyzeStatusTimer);
    			//_downloadInfo.status = DOWNLOAD_IMAGE_CHECKING_MD5;
    			//_app->vm_download_progress_status(&_downloadInfo);

    			//ui wait for checking md5
    			LOG_INFO("calculte md5 of download base!");
    			//string file_md5 = calculte_file_md5();
    			string file_md5 = "ok";
    			if(file_md5 == "error")
    			{
    				LOG_WARNING("calculte file md5 fail!");
    				_downloadInfo.status = DOWNLOAD_IMAGE_CHECKED_MD5_FAIL;
    				_app->vm_download_progress_status(&_downloadInfo);
    			}else{
    				LOG_INFO("===================================file_md5=%s====================",file_md5.c_str());
    				//check MD5 success,return success status
    				_imageInfo.md5 = file_md5;
    /*
    				int image_size;
    				image_size = get_image_virt_size(_imageInfo.name);
    				LOG_INFO("download image %s, image size %d, resize %d", _imageInfo.name.c_str(), image_size, _imageInfo.virt_size);
    				if (image_size == 0) {
    					image_size = _imageInfo.virt_size;
    				}

    				if (image_size < _imageInfo.virt_size) {
    					if (resize_image(_imageInfo.name, _imageInfo.virt_size, "qcow2")) {
    						image_size = _imageInfo.virt_size;
    					}
    				}

    				_imageInfo.virt_size = image_size;
    */
    				_imageData.set(_imageInfo,false);
    				 set_download_mode(DOWNLOAD_IMAGE_NO_OPS);
#if 0
                    //app_layer_manage
                    if (_imageInfo.layer_info.layer_on_1 == "Y") {
                        _app->get_UsrUserInfoMgr()->set_app_layer_switch(1);
                    } else {
                        _app->get_UsrUserInfoMgr()->set_app_layer_switch(-1);
                    }
#endif
    				_downloadInfo.status = DOWNLOAD_IMAGE_SUCCESS;
    				_app->vm_download_progress_status(&_downloadInfo);
    			}
    		}else{
    			LOG_INFO("DOWNLOADING...");
    			_downloadInfo.status = DOWNLOAD_IMAGE_DOWNLOADING;
    			_app->vm_download_progress_status(&_downloadInfo);
    		}
    	}
    }
   else if (DOWNLOAD_IMAGE_MERGING==get_download_mode())
    {
    	if (strcmp(_download_merge_error.c_str(),"1") == 0){
    		LOG_DEBUG("merge image error , delete diff base and force download image.");
    		_downloadInfo.status = DOWNLOAD_IMAGE_ERROR;
    		_app->vm_download_progress_status(&_downloadInfo);
    		_process_loop.deactivate_interval_timer(_analyzeStatusTimer);
    		set_download_mode(DOWNLOAD_IMAGE_FORCE_DOWNLOAD);
            execute_command("rm /opt/lessons/diff/* -rf");
    	}else
    	{
    		if(strcmp(_download_percent.c_str(),"100.00") == 0)
    		{
    			_process_loop.deactivate_interval_timer(_analyzeStatusTimer);
   				_imageInfo.md5 = "ok";
   
			    rename_base(_imageData.get().name, _imageInfo.name);
				LOG_INFO("merge success");
				_imageData.set(_imageInfo,false);
				 set_download_mode(DOWNLOAD_IMAGE_NO_OPS);
                 execute_command("rm /opt/lessons/diff/* -rf");
				_downloadInfo.status = DOWNLOAD_IMAGE_SUCCESS;
				_app->vm_download_progress_status(&_downloadInfo);
    		}else{
    			LOG_INFO("DOWNLOADING...");
    			_downloadInfo.status = DOWNLOAD_IMAGE_MERGE;
    			_app->vm_download_progress_status(&_downloadInfo);
    		}
    	}
    }
   else if (DOWNLOAD_IMAGE_NEED_MERGE==get_download_mode())
    {
        if (_download_success_tips==0)
	    {
	        _process_loop.deactivate_interval_timer(_analyzeStatusTimer);
	    }
	    else
	    {
	        if (120==_download_success_tips)
		    {
                _app->prepare_merge_image();
                _download_success_tips = 1;
		    }
		    _download_success_tips++;
	    }
    }
}

/*
 * get image info
 */
ImageInfo ImageManage::get_image_info()
{
	return _imageData.get();
}

/**
* get layer info
*/
void ImageManage::get_layerdisk_info(LayerDiskInfo & layer_info)
{
   return _imageData.get_layerdisk_info(layer_info);
}


/*
 * set imageinfo recovery
 */


/*
 * get download status
 */
bool ImageManage::get_image_download_status()
{
	return _imageData.get_download_status();
}
/*
 *
 */
string ImageManage::get_local_image_base_name()
{
	string torrent_url = get_image_info().torrent_url;
	string torrent_name = execute_command("basename " + torrent_url);
	int nPos = torrent_name.find_first_of(".");
	string baseName = torrent_name.substr( 0 , nPos );
	return baseName;
}

/*
 * check /opt/lessons has base file or not
 */
int ImageManage::check_local_base_exist()
{
	string result = execute_command(_cmd_base_num);
	if(result == "error")
	{
		LOG_INFO("check base exist fail!");
		return -1;
	}else{
		if(atoi(result.c_str()) == 0)
		{
			LOG_INFO("base file not exist!");
			//base not exist,rm vm_image_info.ini
			string clear_ini = execute_command("rm -rf " RCC_DATA_PATH "vm_image_info.ini");
			return 0;
		}else{
			if(_imageData.get_download_status() == true){
				LOG_INFO("base is downloading!");
				return 0;
			}else{
				LOG_INFO("base file exist!");
				return 1;
			}
		}
	}
}
int ImageManage::check_local_base_status()
{
	string result = execute_command(_cmd_base_num);
	if(result == "error")
	{
		LOG_INFO("check base exist fail!");
		return -1;
	}else{
		if(atoi(result.c_str()) == 0)
		{
			LOG_INFO("base file not exist!");
			return 0;
		}else{
			LOG_INFO("base file exist!");
			return 1;
		}
	}
}
/*
 * check /opt/lessons has personal img file or not
 */
bool ImageManage::check_personal_img_exist()
{
	string result = execute_command(_cmd_personal_img_num);
	if(result == "error")
	{
		LOG_INFO("check personal img fail!");
		return true;
	}else{
		if(atoi(result.c_str()) == 0)
		{
			LOG_INFO("personal img is not exist!");
			return false;
		}else{
			LOG_INFO("personal img is exist!");
			return true;
		}
	}
}

bool ImageManage::check_layer_disk_exist()
{
    if (_app->get_UsrUserInfoMgr()->get_app_layer_switch() != 1) {
        LOG_DEBUG("layer manage switch off, layer_disk is not exist!");
        return false;
    }

    string result = execute_command(_cmd_have_layer_disk);
    if (result == "error") {
        LOG_INFO("check layer disk fail!");
        return true;
    } else {
        if (atoi(result.c_str()) == 0) {
            LOG_INFO("layer_disk is not exist!");
            return false;
        } else {
            LOG_INFO("layer_disk is exist!");
            return true;
        }
    }
}

bool ImageManage::check_layer_tmplete_exist()
{
    string result = execute_command(_cmd_layer_templete_num);
    if (result == "error") {
        LOG_INFO("check layer tmplete fail!");
        return true;
    } else {
        if(atoi(result.c_str()) == 0)
        {
            LOG_INFO("layer_tmplete is not exist!");
            return false;
        }else{
            LOG_INFO("layer_tmplete is exist!");
            return true;
        }
    }
}

bool ImageManage::check_usedisk_exist()
{
	string result = execute_command(_cmd_have_usedisk);
	if(result == "error")
	{
		LOG_INFO("check usedisk fail!");
		return true;
	} else {
		if(atoi(result.c_str()) == 0)
		{
			LOG_INFO("usedisk is not exist!");
			return false;
		}else{
			LOG_INFO("usedisk is exist!");
			return true;
		}
	}
}

bool ImageManage::check_expand_disk_exist(const string &disk_name)
{
    string cmd_have_print_disk = "ls /opt/lessons/" + disk_name + " " + "| wc -l";
    string result;

    result = execute_command(cmd_have_print_disk);
    if (result == "error") {
        LOG_INFO("check %s fail!", disk_name.c_str());
        return true;
    } else {
        if (atoi(result.c_str()) == 0) {
            LOG_INFO("%s is not exist!", disk_name.c_str());
            return false;
        } else {
            LOG_INFO("%s is exist!", disk_name.c_str());
            return true;
        }
    }
}

bool ImageManage::copy_print_disk( )
{
    string copy_cmd = "cp /opt/lessons/print/printtemplete /opt/lessons/w.disk";
    string result;

    result = execute_command(copy_cmd);
    if (result == "error") {
        LOG_INFO("copy_print_disk error");
        return false;
    }

    return true;
}


bool ImageManage::start_bt_service()
{
    string torrent_url = get_image_info().torrent_url;
    if(torrent_url == "")
    {
        LOG_ERR("torrent file is not exist!");
        return false;
	}if (DOWNLOAD_IMAGE_NO_OPS!=get_download_mode())
	{
        return false;
	}
    string torrent_name = execute_command("basename " + torrent_url);
    string cmd = _image_commond_path + " start_in_seeding " + _bt_torrent_path + " " +_bt_torrent_path + "/" + torrent_name;
    //rc_system(cmd.c_str());
    string result = execute_command(cmd);
    return true;
}

bool ImageManage::stop_bt_service()
{
    if (DOWNLOAD_IMAGE_NO_OPS!=get_download_mode())
	{
        return false;
	}
    string cmd = _image_commond_path + " stop " +  _bt_torrent_path+"/"+_imageData.get().name+" false";
    string result = execute_command(cmd);
    return true;
}

int ImageManage::start_usb_image()
{
    //deactive download timer, or switch usb download will be locked for a moment.
    _process_loop.deactivate_interval_timer(_analyzeStatusTimer);
    string cmd = _image_commond_path + " stop " +  _bt_torrent_path+"/"+get_new_base_name()+" false";
    if (is_silent_download()){
        cmd = _image_commond_path + " stop " +_bt_torrent_path+"/diff/"+get_new_base_name()+" false";
	}
    const string usb_basename = _downloadInfo.image_name;
    int ret = _usb_manage->if_usb_contain_same_base(usb_basename, _mount_dev);
    if(0 == ret)
    {
        execute_command(cmd);
        _usb_manage->stop_cp_base2();
        _usb_manage->mount_dev(_mount_dev);

        //space enough
        if(_app->get_vm()->usb_copy_space_enough(usb_basename) == false)
        {
            LOG_ERR("start usb copy err, space not enough");
            _mount_dev.clear();
            _usb_downloading = false;
            ret = -5;
        }
        else
        {
            _usb_manage->start_cp_base2(usb_basename);
            _usb_downloading = true;
        }
    }
    else
    {
        LOG_ERR("start usb copy err");
        _mount_dev.clear();
        _usb_downloading = false;
    }
    if (_usb_downloading == true) {
        _app->vm_stop_redownload_timer();
    }
    _process_loop.activate_interval_timer(_analyzeStatusTimer,_analyze_interval);
    return ret;
}

int ImageManage::stop_usb_image()
{
    if(_usb_downloading)
    {
        _usb_manage->umount_dev();
        _usb_manage->stop_cp_base2();
        _usb_downloading = false;
    }
    return 0;
}

bool ImageManage::is_usb_downloading()
{
    return _usb_downloading;
}

int ImageManage::get_usb_base_size(const string& basename)
{
    return _usb_manage->get_base_size(basename);
}

