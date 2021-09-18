#include "application.h"
#include<iostream>
#include "application_c_interfaces.h"
#include "user_mgr.h"
#include "VM.h"
#include <signal.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <linux/netlink.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/rtnetlink.h>
#include <list>
#include "base64.h"
#include "rc_json.h"
#ifdef IDV_CLIENT
#include "rc/rc_checknetifval.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif
#include "dhcp_option.h"
#ifdef __cplusplus
}
#endif

using namespace std;
using namespace RcJson;

enum RcoPackageCount
{
    RCO_PACKAGE_COUNT = 4,
};
const static char RCO_PACKAGE_NAMES[RCO_PACKAGE_COUNT][16] =
{
    "rcd",
    "vmmanager",
    "qemu",
    "bittornado",
};
enum RcoDependsCount
{
    RCO_DEPENDS_COUNT = 2,
};
const static char RCO_DEPENDS_NAMES[RCO_DEPENDS_COUNT][16] =
{
    "rjsyscore",
    "rcos-abslayer",
};

/* class resource file */
#define RCC_PACKAGE_COUNT    6
#define RCC_DAEMON_COUNT     4
#define RCC_BAK_PATH         "/etc/rcc-bak/"
#define RCC_CLIENT_LOCK      "/opt/lessons/UPGRADE_FROM_RCO"

const static char RCC_PACKAGE_NAMES[RCC_PACKAGE_COUNT][16] =
{
    "bittornado",
    "qemu",
    "rainconfig",
    "spice.rain",
    "vmmanager",
    "rcd",
};

const static char RCC_DAEMON_NAMES[RCC_DAEMON_COUNT][32] =
{
    "0800B_RCC_CLIENT.bash",
    "0800B_RCC_BACK.bash",
    "tch_client_daemon.sh",
    "rcc_client_daemon.sh",
};

static void save_ipxe_configure()
{
    rc_system("rm -rf /boot/efi/RCC_Client_bak");
    rc_system("mkdir -p /boot/efi/RCC_Client");
    rc_system("cp -f " RCC_DATA_PATH "*.ini /boot/efi/RCC_Client");
    rc_system("rm -f /boot/efi/RCC_Client/version_client_idv.ini");
    rc_system("touch /boot/efi/RCC_Client/running_ipxe");
    rc_system("cp -rf /boot/efi/RCC_Client /boot/efi/RCC_Client_bak");

    //save network
    Application* app = Application::get_application();
    //NetworkData network_data("/boot/efi/network.ini");
    VmNetworkInfoDB network_data("/boot/efi/network.ini");
    network_data.set_vm_network(app->get_local_networkinfo());
}
static void load_ipxe_configure()
{
    if(!get_file_exist("/boot/efi/RCC_Client/running_ipxe"))
        return;
    rc_system("sed -i 's/roamlevel=90/roamlevel=0/g' /etc/wpa_supplicant/roamconfig.ini"); //clean_wlan_roamconfig
    rc_system("cp -f /boot/efi/RCC_Client/*.ini " RCC_DATA_PATH);

    /*
     *load network
     */
    LOG_INFO("local network data load_ipxe_configure");
    LocalNetworkData local_network_data(NULL, NULL, NULL);
    //NetworkData network_data("/boot/efi/network.ini");
    VmNetworkInfoDB network_data("/boot/efi/network.ini");
    local_network_data.set(network_data.get_vm_network());

    rc_system("rm -f /boot/efi/RCC_Client/running_ipxe");
    rc_system("rm -rf /boot/efi/RCC_Client");
}

Mutex Application::_singleton_mutex;
Application* Application::_application = NULL;
Application* Application::get_application()
{
    Lock singleton_lock(_singleton_mutex);
    if (!_application) {
        LOG_INFO("\n");
        LOG_INFO("****************************************************************");
        LOG_INFO("======================= Application init =======================");
        LOG_INFO("****************************************************************\n");
        LOG_INFO("***********************ZHJS OPS*******************************\n");
        ::load_ipxe_configure();
        _application = new Application();
    }
    return _application;
}

VM* Application::get_vm()
{
	Application* app = Application::get_application();
	return app->_vm;
}

WifiInteractive* Application::get_wifi_interactive()
{
	Application* app = Application::get_application();
	return app->_wifi_interactive;
}

DeviceInterface* Application::get_device_interface()
{
    Application *app = Application::get_application();
    return app->_device_interface;
}

UserInfoMgr* Application::get_UsrUserInfoMgr()
{
    Application* app = Application::get_application();
	return &(app->_userinfomgr);
}

ModeInfoDB& Application::get_mode_data()
{
	Application* app = Application::get_application();
	return app->_mode_data;
}

AuthManager* Application::get_auth_manager()
{
	Application* app = Application::get_application();
	return app->_net_auth;
}

Application::Application()
    :_status_machine (this)
    ,_process_loop (this)
    ,_mina (new Mina(this))
    ,_vm (new VM(this))
    ,localmode(NULL)
    ,_newdeploy_manage(new NewDeployManage(this))
	,_localmode_startvm(false)
	,_on_easy_deploy(false)
    ,_wifi_interactive(new WifiInteractive())
    ,_device_interface(new DeviceInterface(this))
	,_net_auth(new AuthManager())
    ,_local_network_data(this, set_local_network_callback, NULL)
    ,_ui_locking(false)
	,_ui_has_sync_flag(0)
	,_ui_need_sync_flag(0)
	,_ui_setting_error(0)
	,_ui_setting_tip(0)
	,_default_mode(-1)
	,_allow_newdeploy(true)
	,_logic_state (STATUS_NONE)
	,_logined (false)
	,_admin_passwd ("ruijie.com")
	,_bt_service(0)
//	,_NoNetwireConnect(1)
	,_hide_guest_login(0)
	,_configure_serverip_sync_flag (false)
    ,_configure_hostname("")
    ,_configure_mode(-1)
    ,_vmsync_event(&_userinfomgr)
    ,_redownload_timer (new ReDownloadTimer(this))
    ,_config_event_mgr (new ConfigEventMgr(this))
    ,_error_tips(false)
    ,_shutdown_flag(0)
    ,_new_deploy_processing(false)
    ,_init_sync_processing(false)
    ,_wait_tips_ok(false)
    ,_driver_install_parament("")
{
    //register signal for easy deploy
    signal(SIGUSR1 , &Application::easy_deploy);
    
    //init variables
    var_init();
	
    //set event handlers
    SetFuncArray();

    //send init event to self, ensure other events handled after ProcessLoop::run called
    logic_init_self();
}

Application::~Application()
{  
    //DestroyStateTable();
    _redownload_timer->unref();
    if (localmode) {
    	delete localmode;
    }
    delete _mina;
    delete _vm;
    delete _redownload_timer;
    delete _newdeploy_manage;
    delete _wifi_interactive;
    delete _device_interface;
    delete _config_event_mgr;
    delete _net_auth;
}

void LogicUISyncEvent::do_response()
{
    int ret;

    LOG_DEBUG("Wait until X server started ...\n");
    ret = rc_system(RCC_SCRIPT_PATH"wait_until_xstarted.sh");
    if (ret != 0) {
        LOG_WARNING("Error occured while waiting X server to start.\n");
        // Failed to wait X server, just wait a moment.
        sleep(5);
    }
    LOG_INFO("X server has started, UI interfaces could be shown.\n");

    _ui_started = true;
}

void VMUISyncEvent::do_response()
{
    // X server stopping action moves into class VmManage.
#if 0
    rc_system("touch /tmp/.xserver_forbidden");
    sleep(1);
    rc_system("pkill xinit");
    rc_system("pkill Xorg");
	//usleep(50000);
#endif
    _ui_stopped = true;
    _usermgr->permitUI(false);
    LOG_INFO("Client UI thread has stopped.\n");
}

void UIWaitSyncEvent::do_response()
{
    LOG_DEBUG("Client UI thread has waited to destory.");
}


struct arg_t {
	int argc;
	char** argv;
};

void * Application::ui_thread(void *data) {
	struct arg_t *arg = (struct arg_t *)data;
    Application* app = Application::get_application();

    LOG_DEBUG("=========argc:%d argv[0]:%s==========\n", arg->argc, arg->argv[0]);
    // Notify that UI thread has started.
    app->_uisync_event.response();

    ui_main(arg->argc, arg->argv);

    delete arg;
    LOG_DEBUG("ui_thread quit!");
    // Notify that UI thread has stopped.
    app->_vmsync_event.response();
	return NULL;
}

void * Application::ui_top_thread(void *data) {
    struct arg_t *arg = (struct arg_t *)data;
    Application* app = Application::get_application();

    LOG_DEBUG("=========ui_top_thread argc:%d argv[0]:%s==========\n", arg->argc, arg->argv[0]);
    // Notify that UI thread has started.
    app->_uisync_event.response();
    ui_thread_init(arg->argc, arg->argv);

    delete arg;
    LOG_DEBUG("ui_thread quit!");
    // Notify that UI thread has stopped.
    app->_ui_waitsync_event.response();
    return NULL;
}



void Application::create_usbfilter(string &usb_policy)
{
    UsbConfigInfo usbinfo = {0};
    RccUsbFliterData usbfile;
    //RccUsbFliterInfoDB usbfile;

    cJSON* json = NULL;
    cJSON* json_out = NULL;
    int nwrite = 0;

    LOG_INFO("begin create_usbfilter");
    json = cJSON_Parse(usb_policy.c_str());
    if (json == NULL) {
        LOG_ERR("usb_policy cJSON_Parse failed");
        return;
    }

    json_out = cJSON_GetObjectItem(json, "allow_len");
    if (json_out == NULL) {
        cJSON_Delete(json);
        LOG_ERR("get cJSON allow_len failed");
        return;
    }
    usbinfo.allow_len= (short)json_out->valueint;
    usbinfo.head.allow_len =  htons(usbinfo.allow_len);

    json_out = cJSON_GetObjectItem(json, "default_allow");
    if (json_out == NULL) {
        cJSON_Delete(json);
        LOG_ERR("get cJSON default_allow failed");
        return;
    }
    usbinfo.head.default_allow = (char)json_out->valueint;
    
    json_out = cJSON_GetObjectItem(json, "unallow_len");
    if (json_out == NULL) {
        cJSON_Delete(json);
        LOG_ERR("get cJSON unallow_len failed");
        return;
    }
    usbinfo.unallow_len = (short)json_out->valueint;
    usbinfo.head.unallow_len = htons(usbinfo.unallow_len);
    
    json_out = cJSON_GetObjectItem(json, "allow_str");
    if (json_out == NULL) {
         cJSON_Delete(json);
        LOG_ERR("get cJSON allow_str failed");
        return;
    }
    usbinfo.allow_data = json_out->valuestring;
    
    json_out = cJSON_GetObjectItem(json, "unallow_str");
    if (json_out == NULL) {
         cJSON_Delete(json);
        LOG_ERR("get cJSON unallow_str failed");
        return;
    }
    usbinfo.unallow_data = json_out->valuestring;
    
    LOG_DEBUG("usbinfo.allow_len %x usbinfo.unallow_len %x default_allow : %x allow_len : %x unallow_len:%x  allow_data %s unallow_data%s",
              usbinfo.allow_len, usbinfo.unallow_len, usbinfo.head.default_allow,
              usbinfo.head.allow_len, usbinfo.head.unallow_len, usbinfo.allow_data.c_str(),              usbinfo.unallow_data.c_str());

    nwrite = usbfile.wite_file(&usbinfo);
    if (nwrite == 0) {
        LOG_ERR("w2l_sync_public_policy write RCDUSBFilter failed");
    }

    cJSON_Delete(json);
    LOG_INFO("create_usbfilter ok");
}

bool Application::pre_software_upgrade(const char* version, const char* pool)
{
    const char script_path[256] = RCC_SCRIPT_PATH"rcc_pre_upgrade.sh";
    char command_buf[512];
    int ret;

    if (!get_file_exist(script_path)) {
        LOG_INFO("%s is not exist!", script_path);
        return true;
    }
    sprintf(command_buf, "bash " RCC_SCRIPT_PATH "rcc_pre_upgrade.sh %s %s %s %d"\
        , pool\
        , _mina->mina_get_server_ip().c_str()\
        , version\
        , 0);
    LOG_INFO(command_buf);
    ret = rc_system(command_buf);
    if (ret == 0) {
        return true;
    } else {
        return false;
    }
}

bool Application::post_software_upgrade(const char* version, const char* pool)
{
    const char script_path[256] = RCC_SCRIPT_PATH"rcc_post_upgrade.sh";
    char command_buf[512];
    int ret;

    if (!get_file_exist(script_path)) {
        LOG_INFO("%s is not exist!", script_path);
        return true;
    }
    sprintf(command_buf, "bash " RCC_SCRIPT_PATH "rcc_post_upgrade.sh %s %s %s %d"\
        , pool\
        , _mina->mina_get_server_ip().c_str()\
        , version\
        , 0);
    LOG_INFO(command_buf);
    ret = rc_system(command_buf);
    if (ret == 0) {
        return true;
    } else {
        return false;
    }
}

bool Application::post_rcc_software_upgrade(const char* version, const char* pool)
{
    const char script_path[256] = RCC_BAK_PATH"rcc_post_up2class.sh";
    char command_buf[512];
    int ret;

    if (!get_file_exist(script_path)) {
        LOG_INFO("%s is not exist!", script_path);
        return true;
    }
    sprintf(command_buf, "bash " RCC_BAK_PATH "rcc_post_up2class.sh %s %s %s %d"\
        , pool\
        , _mina->mina_get_server_ip().c_str()\
        , version\
        , 0);
    LOG_INFO(command_buf);
    ret = rc_system(command_buf);
    if (ret == 0) {
        return true;
    } else {
        return false;
    }
}

bool Application::upgrade_software_package(const char* first_package_name, const char* package_name,
                                                  const char* version, const char* pool)
{
    char command_buf[512];
    int ret = 0;
    bool single_package_success = false;//get the success value of each package

    /*
     *parament 5 is the flag to update the server's pool
     *we just do it before we upgrade the first bed
     *FIXME: we can do it before the upgrade loop, but it should modify the script
     */
    sprintf(command_buf, "bash " RCC_SCRIPT_PATH "rcc_upgrade.sh %s %s %s %s %d"\
        , pool\
        , _mina->mina_get_server_ip().c_str()\
        , package_name\
        , version\
        , (strcmp(package_name, first_package_name) == 0) ? 1 : 0);
    LOG_INFO(command_buf);

    //excute the shell
    ret = rc_system(command_buf);
    LOG_DEBUG("system ret = %d", ret);

    //check the result
    if(ret == -1)
    {
        LOG_PERROR("error while calling system, system ret is %d", ret);
        single_package_success = false;
    }
    else
    {
        switch(ret)
        {
            case 0://success
            {
                LOG_INFO("upgrade successful");
                single_package_success = true;
                break;
            }
            case 1://already latest
            {
                /*
                 *if we upgrade some package
                 *the result will be already latest for this time
                 *but we should regard it successful for this time
                 */
                LOG_WARNING("already latest");
                single_package_success = true;
                break;
            }
            case 2://server has no pool
            {
                LOG_ERR("server has no pool");
                single_package_success = false;
                break;
            }
            case 3://upgrade error
            {
                LOG_ERR("upgrade error");
                single_package_success = false;
                break;
            }
            default:
            {
                LOG_ERR("unknown error, ret = %d", ret);
                single_package_success = false;
                break;
            }
        }
    }
    return single_package_success;
}

bool Application::upgrade_software_version(char* data)
{
    char* version_buf = static_cast<char*>(data);
    bool success = true;
    bool single_package_success = false;
    unsigned int i = 0;
    int ret = 0;
    int main_version;
    int minor_version;
    char cmd_buf[512];

    //touch upgrade lock file
    sprintf(cmd_buf, "touch %s", RCC_UPGRADE_LOCK);
    rc_system(cmd_buf);
    rc_system("sync");

    //show ui we are downloading
    l2u_show_updateclient_wrap();
    l2u_ctrl_winbtn(false);
    l2u_show_wifi_btnbox(2, -1, -1);

    if (_net_auth->getAuthExist() == 0) {
        l2u_show_net_auth_winbtn(UI_AUTHBTN_DISABLED);
    }

    sscanf(version_buf, "%d.%d.", &main_version, &minor_version);
    LOG_DEBUG("sscanf main_version:%d, minor_version:%d", main_version, minor_version);

    //remove cache deb
    sprintf(cmd_buf, "rm -f %s*.deb", RCC_DEB_CACHE_PATH);
    rc_system(cmd_buf);

    //do pre upgrade
    if (success) {
        success = pre_software_upgrade(version_buf, RCO_POOL_NAME);
    }

    //match os version
    if (success) {
        if (main_version < 4) {
            success = match_old_system_version(TYPE_OLD_RCO_SERVER);
        }
    }

    //upgrade packages
    for(i = 0; i < RCO_PACKAGE_COUNT; i++)
    {
        if(!success)
        {
            break;
        }
        single_package_success = upgrade_software_package(RCO_PACKAGE_NAMES[0], RCO_PACKAGE_NAMES[i], version_buf, RCO_POOL_NAME);
        if(single_package_success == false)
        {
            success = false;
        }
    }

    //upgrade depend packages of rcd.deb
    for (i = 0; i < RCO_DEPENDS_COUNT; i++)
    {
        if (!success) {
            break;
        }

        if (strncmp(RCO_DEPENDS_NAMES[i], "rjsyscore", 9) == 0) {
            if (strncmp(version_buf, "2.0", 3) == 0 || strncmp(version_buf, "1.0", 3) == 0) {
                ret = rc_system("apt-get remove -y --purge rjsyscore");
                if (ret != 0) {
                    LOG_PERROR("error while calling system, system ret is %d", ret);
                    //success = false;    //rjsyscore has removed, may ret 100
                }
            } else {
                single_package_success = upgrade_software_package(RCO_PACKAGE_NAMES[0], RCO_DEPENDS_NAMES[i], version_buf, RCO_POOL_NAME);
                if(single_package_success == false) {
                    success = false;
                }
            }
        } else if (strcmp(RCO_DEPENDS_NAMES[i], "rcos-abslayer") == 0) {
            if (main_version < 4 || (main_version == 4 && minor_version < 1)) {
                ret = rc_system("apt-get remove -y --purge rcos-abslayer");
                if(ret != 0) {
                    LOG_PERROR("error while calling system, system ret is %d", ret);
                    //success = false;    //rcos-abslayer has removed, may ret 100
                }
            }
        }
    }

    //do post upgrade
    if (success) {
        success = post_software_upgrade(version_buf, RCO_POOL_NAME);
    }

    // delete PRINTER disk when version downgrade to lower 4.0
    if (success) {
        //if (_version_data.get().minor_version >= 2 && minor_version < 2) {
        if (main_version < 4 && _version_data.get_version_info().main_version >= 4) {
            _userinfomgr.delete_diskinfo_ini();
            if (_vm->vm_check_expand_disk_exist(PRINTER_DISK_NAME)) {
                _vm->vm_clear_print_disk(PRINTER_DISK_NAME);
            }
         }
     }

    //show ui
    if(success)
    {
        l2u_result_updateclient_wrap(0);
        rc_system("sync");
        //remove upgrade lock file
        sprintf(cmd_buf, "rm -f %s", RCC_UPGRADE_LOCK);
        rc_system(cmd_buf);
        rc_system("sync");
        //reboot
        sleep(5);
        rc_system("reboot");
    }
    else
    {
        l2u_result_updateclient_wrap(1);
    }
    
    return success;
}

void Application::check_software_upgrade(const char* version)
{
    const char script_path[256] = RCC_SCRIPT_PATH"rcc_check_upgrade.sh";
    char command_buf[512];

    if (!get_file_exist(script_path)) {
        LOG_INFO("%s is not exist!", script_path);
        return ;
    }
    sprintf(command_buf, "bash " RCC_SCRIPT_PATH "rcc_check_upgrade.sh %s %s %s %d"\
        , RCO_POOL_NAME\
        , _mina->mina_get_server_ip().c_str()\
        , version\
        , 0);
    LOG_INFO(command_buf);
    rc_system(command_buf);
}

bool Application::upgrade_class_software_version(UpgradeInfo info)
{
    bool success = true;
    bool single_package_success = false;
    unsigned int i = 0;
    char version_buf[128];
    int serverType;

    VersionInfo *ver = &(info.versionInfo);
    serverType = info.serverType;
    sprintf(version_buf, "%d%d%d%d-1", ver->main_version,
            ver->minor_version, 0, ver->fourth_version);

    LOG_DEBUG("upgrade class, version_buf:%s, serverType:%d", version_buf, serverType);

    //show ui we are downloading
    l2u_show_updateclient_wrap();
    l2u_ctrl_winbtn(false);
    l2u_show_wifi_btnbox(2, -1, -1);

    if (_net_auth->getAuthExist() == 0) {
        l2u_show_net_auth_winbtn(UI_AUTHBTN_DISABLED);
    }

    if (!get_file_exist(RCC_SCRIPT_PATH "rcc_upgrade.sh")) {
        l2u_result_updateclient_wrap(4);
        return false;
    }

    //do pre upgrade
    if (success) {
        success = pre_software_upgrade(version_buf, RCC_POOL_NAME);
    }

    //upgrade packages
    for (i = 0; i < RCC_PACKAGE_COUNT; i++)
    {
        if (!success)
        {
            break;
        }
        single_package_success = upgrade_software_package(RCC_PACKAGE_NAMES[0], RCC_PACKAGE_NAMES[i], version_buf, RCC_POOL_NAME);
        if ((single_package_success == false) && (strcmp(RCC_PACKAGE_NAMES[i], "spice.rain")))
        {
            success = false;
        }
    }

    //do post upgrade
    if (success) {
        success = post_rcc_software_upgrade(version_buf, RCC_POOL_NAME);
    }

    //match version
    //if (success) {
    //    success = match_old_system_version(serverType);
    //}

    //make tab file
    if (success) {
        rc_system("touch " RCC_CLIENT_LOCK);
    }

    //show ui
    if(success)
    {
        l2u_result_updateclient_wrap(7);
        rc_system("sync");
        sleep(5);
        rc_system("reboot");
    }
    else
    {
        l2u_result_updateclient_wrap(1);
    }

    return success;
}

/*
bool Application::copy_rcc_daemon_script(int serverType)
{
    if (serverType == TYPE_OLD_RCC_SERVER) {
        char cmd_buf[128];
        int i;

        for (i = 0; i < RCC_DAEMON_COUNT; i++) {
            sprintf(cmd_buf, RCC_BAK_PATH "%s", RCC_DAEMON_NAMES[i]);
            if(!get_file_exist(cmd_buf)) {
                return false;
            }
        }
        sprintf(cmd_buf, "cp -f %s%s /usr/local/bin/", RCC_BAK_PATH, RCC_DAEMON_NAMES[3]);
        rc_system(cmd_buf);
        sprintf(cmd_buf, "cp -f %s%s /usr/local/bin/", RCC_BAK_PATH, RCC_DAEMON_NAMES[2]);
        rc_system(cmd_buf);
        sprintf(cmd_buf, "cp -f %s%s /etc/AutoStart.rclocal/", RCC_BAK_PATH, RCC_DAEMON_NAMES[1]);
        rc_system(cmd_buf);
        sprintf(cmd_buf, "cp -f %s%s /etc/AutoStart.xinit/", RCC_BAK_PATH, RCC_DAEMON_NAMES[0]);
        rc_system(cmd_buf);
        rc_system("mkdir -p /etc/rc_config/default/");
        sprintf(cmd_buf, "ln -sf /etc/AutoStart.rclocal/%s /etc/rc_config/default/rc.local_priv", RCC_DAEMON_NAMES[1]);
        rc_system(cmd_buf);
    }

    rc_system("cp -f " RCC_BAK_PATH "0900B_RCC_VM_DATA_DISK_CREATE.bash /etc/AutoStart.rclocal/");
    rc_system("rm -f /etc/AutoStart.rclocal/0900B_RCO_VMMESSAGE.bash");
    rc_system("rm -f /etc/AutoStart.login/0800B_RCO_CLIENT.bash");
    rc_system("rm -f /etc/AutoStart.xinit/0900F_rco_hotplug.bash");
    rc_system("rm -f /etc/AutoStart.xinit/0910B_HotplugDaemon.bash");

    return true;
}
*/

bool Application::match_old_system_version(int serverType)
{
    char cmd_buf[128] = {0};
    char ret_buf[64] = {0};
    char os_version[8] = {0};
    char te_version[8] = {0};
    int ret_size = sizeof(ret_buf);
    int ret;

    if (!check_old_system_version(ret_buf, ret_size)) {
        sscanf(ret_buf, "IDV-RainOS_%4s%7s", os_version, te_version);
        LOG_DEBUG("version num:%s, te num:%s", os_version, te_version);
        if (serverType == TYPE_OLD_RCC_SERVER) {
            sprintf(cmd_buf, "echo \"RCC_RainOS_V6.0_R0%s.%d_NS\" >/etc/issue", te_version, atoi(os_version));
        } else if (serverType == TYPE_OLD_RCO_SERVER) {
            sprintf(cmd_buf, "echo \"RCO-IDV-Rain_V6.0_R0%s.%d_NS\" >/etc/issue", te_version, atoi(os_version));
        } else {
            return true;
        }
        ret = rc_system(cmd_buf);
        if(ret != 0) {
            LOG_PERROR("error while calling system, system ret is %d", ret);
            return false;
        }
    }

    return true;
}

bool Application::check_old_system_version(const char *os_version, int size)
{
    char cmd_buf[128] = {0};
    int ret;

    strcpy(cmd_buf, "cat /etc/issue | head -n 1 | grep '^IDV-RainOS_*'");
    ret = rc_system_rw(cmd_buf, (unsigned char*)os_version, &size, "r");
    LOG_DEBUG("check os version, ret:%d, version:%s", ret, os_version);
    if (ret == 0) {
        return false;
    }
    return true;
}

int Application::get_system_version(void)
{
    char ret_buf[64] = {0};
    char os_version[8] = {0};
    char te_version[8] = {0};
    int ret_size = sizeof(ret_buf);
    int ret;

    if (check_old_system_version(ret_buf, ret_size)) {
        return -1;
    }

    sscanf(ret_buf, "IDV-RainOS_%4s%7s", os_version, te_version);
    LOG_DEBUG("version num:%s, te num:%s", os_version, te_version);
    ret = atoi(os_version);

    return ret;
}

void* Application::collect_log(void* data)
{
    int ret = 0;
    char logname_buf[512];
    char cmd_buf[1024];
    FtpLogInfo *info = (FtpLogInfo *)data;

    LOG_INFO("enter %s", __func__);
    ret = rc_system("date > /var/log/collect_log.log");
    if(ret != 0)
    {
        LOG_ERR("record collect log time error");
    }

    if (data == NULL) {
        return NULL;
    }

    sprintf(cmd_buf, RCC_SCRIPT_PATH"collet_log.sh %s %s %s %s %s > /dev/null 2>&1"\
        , Application::get_application()->_hostname_data.get_hostname_info().c_str()\
        , Application::get_application()->_local_network_data.get_ip_info().ip.c_str()\
        , Application::get_application()->_mina->mina_get_server_ip().c_str()\
        , (info->ftpuser).c_str(), (info->ftppwd).c_str());

    //LOG_INFO("cmd_buf: %s", cmd_buf);
    sprintf(logname_buf, "%s@%s.tar.gz"\
        , Application::get_application()->_hostname_data.get_hostname_info().c_str()\
        , Application::get_application()->_local_network_data.get_ip_info().ip.c_str());

    ret = rc_system(cmd_buf);
    if(ret != 0)
    {
        LOG_ERR("collect log error");
    }

    LOG_INFO("end collect log");

    delete(info);
    Application::get_application()->_mina->mina_web_collect_log_complete(0, logname_buf);
    return NULL;
}

static inline void set_thread_name(const char *name)
{
    int ret;

    ret = prctl(PR_SET_NAME, name, NULL, NULL, NULL);
    if (ret < 0) {
        LOG_WARNING("Set thread[%lu] name(%s) failed: %s\n",
            pthread_self(), name, strerror(errno));
    }
}

static inline void del_newline(char *s, int size)
{
    //int len = strlen(s);
    char *p = s + size;
    while (*(--p) == '\n')
        *p = '\0';
}

bool Application::is_dev_network_unknown()
{
    return _local_network_data.is_dev_network_unknown();
}

bool Application::is_dev_network_up()
{
    return _local_network_data.is_dev_network_up();
}

bool Application::is_dev_eth_up()
{
    return _local_network_data.is_dev_eth_up();
}

bool Application::is_dev_wlan_up()
{
    return _local_network_data.is_dev_wlan_up();
}

bool Application::is_dev_wired_dot1x_up()
{
    return (_local_network_data.get_wired_auth_exist() == 0);
}

bool Application::is_dev_network_down()
{
    return _local_network_data.is_dev_network_down();
}

bool Application::is_wired_plugged()
{
    return _local_network_data.check_wired_plugged();
}

bool Application::get_net2_status()
{
    return _local_network_data.get_net2_status();
}

bool Application::get_nat_policy()
{
    if(_local_network_data.get_net_status() == NET_STATUS_WLAN_UP)
        return true;
    else
        return false;
}


#if 0
static inline char *get_netif_default_name()
{
#define IFNAME_LEN 32
    static char ifname[IFNAME_LEN];
    static char *ifp = NULL;
    int ret, name_size;

    if (ifp == NULL) {
        name_size = IFNAME_LEN;
#if 0
        ret = rc_system_rw(
            "ifconfig -a | sed -n '/^eth[0-9][A-Za-z0-9 ]*:Ethernet/p' | cut -d' ' -f1",
            (unsigned char *)ifname, &name_size, "r");
#else
        ret = rc_system_rw(RCC_SCRIPT_PATH"get_devifname.sh",
            (unsigned char *)ifname, &name_size, "r");
#endif
        if (ret == 0) {
            if (name_size >= IFNAME_LEN)
                name_size = IFNAME_LEN - 1;
            ifname[name_size] = '\0';
            del_newline(ifname, name_size);
            ifp = ifname;
            LOG_INFO("Device net interface physical name: [%s]", ifp);
        } else {
            /**
             * XXX: IDV default netif is 'br0'
             * but it's a virtual netcard, which really bridge to 'eth0'
             * so we return the real physical netcard 'eth0'
             */
            strcpy(ifname, "eth0");
            ifp = ifname;
            LOG_WARNING("Use device net interface default name: [%s] (for error %d)", ifp, ret);
        }
    }
    return ifp;
}

int Application::create_network_monitor_socket()
{
    int fd, ret;
    struct sockaddr_nl addr;

retry:
    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (fd < 0) {
        LOG_PERROR("Failed to create netlink socket.");
        sleep(1);
        goto retry;
    }

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_LINK;
    ret = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        LOG_PERROR("Failed to bind netlink socket to link group.");
        close(fd);
        sleep(1);
        goto retry;
    }
    return fd;
}

/**
 * query current network status to init net status variable.
 */
void Application::network_status_request(int sd)
{
    int ret = 0;
    struct {
        struct nlmsghdr nh;
        struct ifinfomsg ifimsg;
    } req;

    memset(&req, 0, sizeof(req));
    req.nh.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    req.nh.nlmsg_type = RTM_GETLINK;
    req.ifimsg.ifi_family = AF_UNSPEC;
    req.ifimsg.ifi_index = 0;
    req.ifimsg.ifi_change = 0xFFFFFFFF;
    ret = send(sd, &req, req.nh.nlmsg_len, 0);
    if (ret < 0) {
        LOG_PERROR("Failed to request network current status.");
    }
    //LOG_DEBUG("Request network current status.");
}

void Application::network_status_process(int sd)
{
#define BUF_LEN             4096
    int rc;
    char buf[BUF_LEN];
    struct iovec iov = { buf, sizeof(buf) };
    struct sockaddr_nl sa;
    struct msghdr msg;
    struct nlmsghdr *nh;
    struct ifinfomsg *ifimsg;
    struct rtattr *rta;
    int attrlen;
    char *dev_ifname = get_netif_default_name();
    char *recv_ifname;

    msg = { &sa, sizeof(sa), &iov, 1, NULL, 0, 0 };
    rc = recvmsg(sd, &msg, 0);
    if (rc < 0) {
        LOG_PERROR("Error occured while reading netlink messages.");
        return;
    }
    for (nh = (struct nlmsghdr *)buf; NLMSG_OK(nh, rc); nh = NLMSG_NEXT(nh, rc)) {
        bool is_def_if = false;
        switch (nh->nlmsg_type) {
        case NLMSG_DONE:
            //LOG_DEBUG("Finish processing network netlink messages.\n");
            return;
        case NLMSG_ERROR:
            LOG_ERR("NLMSG ERROR while processing network netlink messages.\n");
            return;
        case RTM_NEWLINK:
        case RTM_DELLINK:
            //LOG_DEBUG("Network link changing event detected.\n");
            ifimsg = (struct ifinfomsg *)NLMSG_DATA(nh);
            if (ifimsg->ifi_type != ARPHRD_LOOPBACK) {
                // ignore loopback device
                attrlen = nh->nlmsg_len - NLMSG_LENGTH(sizeof(struct ifinfomsg));
                for (rta = IFLA_RTA(ifimsg);
                    RTA_OK(rta, attrlen) && rta->rta_type <= IFLA_MAX;
                    rta = RTA_NEXT(rta, attrlen)) {
                    if (rta->rta_type == IFLA_IFNAME) {
                        recv_ifname = (char *)RTA_DATA(rta);
                        LOG_DEBUG("%s: ", recv_ifname);
                        if (!strcmp(recv_ifname, dev_ifname))
                            is_def_if = true;
                    }
                }
                if (ifimsg->ifi_flags & IFF_RUNNING) {
                    LOG_DEBUG("  link up\n");
                    if (is_def_if) {
                        if (is_dev_network_down()) {
                            LOG_NOTICE("link changed: down -> up.");
                            set_dev_network(NET_STATUS_UP);
                            if (!_userinfomgr.on_local_mode() && !_mina->mina_get_server_ip().empty()) {
                                //on local mode, we'll not change the connection
                                //if server ip is empty, no need to consider the case of link up
                                if(_ui_locking == false)
                                {
                                    //if in setting, should not show UI
                                    l2u_show_connecting_wrap();
                                }
                                _mina->mina_establish_connection();
                            }
                            if (_vm->vm_is_vm_running() && _vm->get_net_policy())
                                _vm->vm_enable_netuse();
                        } else {
                            set_dev_network(NET_STATUS_UP);
                        }
                    }
                } else {
                    LOG_DEBUG("  link down\n");
                    if (is_def_if) {
                        if (is_dev_network_up()) {
                            LOG_NOTICE("link changed: up -> down.");
                            set_dev_network(NET_STATUS_DOWN);
                            if (!_userinfomgr.on_local_mode()) {
                                //on local mode, we'll not change the connection
                                if(_vm->vm_get_vm_download_status() == true){
                                    LOG_DEBUG("stop download image first");
                                    _vm->vm_stop_download_image();
                                }

                                if (_ui_locking == false && !_mina->mina_get_server_ip().empty()) {
                                    /**
                                     * other status would change to STATUS_INITING after re-connected,
                                     * so should not show disconnect UI
                                     * if in setting, should not show UI
                                     * if server ip is empty, should not change UI
                                     */
                                    if(_status_machine.get_status() == STATUS_INITING)
                                    {
                                        l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT);
                                        _newdeploy_manage->reset_status();
                                    }
                                    else if(_newdeploy_manage->is_new_terminal())
                                    {
                                        l2u_show_newdeploy_connect(0);
                                    }
                                }

                                //wait until setting done, or change to STATUS_INITING _ui_need_sync_flag & _ui_has_sync_flag would be cleared!                              
                                while (_ui_need_sync_flag != 0) {
                                    sleep(1);
                                }
                                if (_on_easy_deploy) {
                                    //we do not destroy connection until easy deploy finished.
                                    LOG_NOTICE("web would be destroyed after easy deploy.");
                                } else {
                                    _mina->mina_destroy_connection();
                                }
                            }
                            if (_vm->vm_is_vm_running() && _vm->get_net_policy())
                                _vm->vm_disable_netuse();
                            //TODO: zjj
                            if (_status_machine.get_status() == STATUS_NEW_DEPLOY) {
                                _newdeploy_manage->quit_new_deploy_abnormal();
                            }
                        } else {
                            set_dev_network(NET_STATUS_DOWN);
                        }
                    }
                }
            }
        default:
            //LOG_DEBUG("NLMSG event '%d' does not need to do.\n", nh->nlmsg_type);
            break;
        }
    }
}

void *Application::monitor_system_status(void *data)
{
    Application *app = (Application *)data;
    int nready;
    int net_sd;
    fd_set rd_set;
    int max_fd = -1;

    pthread_detach(pthread_self());
    set_thread_name("sys_monitor");

    // create network monitor socket.
    net_sd = app->create_network_monitor_socket();
    FD_ZERO(&rd_set);
    FD_SET(net_sd, &rd_set);
    max_fd = (net_sd > max_fd ? net_sd : max_fd);
    app->network_status_request(net_sd);

    // start monitor.
    while (1) {
        nready = select(max_fd + 1, &rd_set, NULL, NULL, NULL);
        if (nready < 0) {
            LOG_PERROR("Netlink messages select error!");
            sleep(1);
            continue;
        } else if (nready == 0) {
            LOG_WARNING("Never come here!");
            continue;
        }

        if (FD_ISSET(net_sd, &rd_set)) {
            app->network_status_process(net_sd);
            if (--nready <= 0)
                continue;
        }
    }
    
    close(net_sd);
    return NULL;
}
#endif
void Application::handle_vendor_encapsulated_options(const char* input, void* data)
{
    string option_server_ip = input;
    //Application* app = Application::get_application();
    Application* app =((LocalNetworkData*)data)->get_application();
    
    if(app->_local_network_data.get().dhcp == false)
    {
        LOG_INFO("handle_vendor_encapsulated_options _local_network_data not dhcp");
        return;
    }
    if(option_server_ip == app->_mina->mina_get_server_ip())
    {
        LOG_INFO("handle_vendor_encapsulated_options option_server_ip same as mina_server_ip");
        return;
    }
    if(app->_mina->mina_check_web_alive(app->_mina->mina_get_server_ip()) == SUCCESS)
    {
        LOG_INFO("handle_vendor_encapsulated_options mina_server_ip is accessable");
        return;
    }
    if(app->_mina->mina_check_web_alive(option_server_ip) != SUCCESS)
    {
        LOG_INFO("handle_vendor_encapsulated_options option_server_ip not accessable");
        return;
    }
    LOG_INFO("handle_vendor_encapsulated_options modify server_ip to %s", option_server_ip.c_str());
    app->_mina->mina_save_server_ip(option_server_ip);
    app->logic_reset_status_machine();
}

void Application::main(int argc, char* argv[])
{ 
#ifdef UNIT_TEST
    UnjoinableThread *unitetest_thread = new UnjoinableThread(Application::unittest, this);
#endif
    char ifname[16];
    UnjoinableThread *ui_pthread;
    
    string str_w, str_h;
    //UnjoinableThread *sys_monitor_thread;
	struct arg_t *arg = new (struct arg_t);
	arg->argc = _argc = argc;
	arg->argv = _argv = argv;

    if (rc_getdefaultnetifname(ifname, sizeof(ifname)) != 0) {
        LOG_NOTICE("get defaultnetifname fail, so use RCC_DEFAULT_IFNAME");
        strcpy(ifname, RCC_DEFAULT_IFNAME);
    }

    ui_thread_pre_init();
    ui_pthread = new UnjoinableThread(Application::ui_thread, (void *)arg);
    if (_userinfomgr.UI_permitted() && !_uisync_event.ui_check_started()) {
        // UI thread has not started, wait a moment.
        LOG_INFO("Wait until UI started.\n");
        _uisync_event.wait();

        // adjust resolution
        //LOG_INFO("adjust resolution");
        //if(ui_thread_pre_init() == 0) {
        //    if(_device_interface->getCurResolution(str_w, str_h, IS_RES_CURRENT) == 0) {
        //        l2u_redisplay_adapt_control(atoi(str_w.c_str()), atoi(str_h.c_str()));
        //    }
        //}
    }
    // start device & system monitor.
    //TODO,zjj
    //sys_monitor_thread = new UnjoinableThread(Application::monitor_system_status, this);
//	u2l_thread_quit();
    _process_loop.run();
//	ui_pthread->cancel();

    delete ui_pthread;


    //delete sys_monitor_thread;
#ifdef UNIT_TEST
    delete unitetest_thread;
#endif
}

void Application::var_init()
{
    // check if logined, and get saved state.
    _userinfomgr.loadSavedStatus();

    // IDV:4.1R1T1 network security then need init some configure files
    _userinfomgr.UserInfoInitconfig();
    _userinfomgr.AuthIniInitconfig();
}

int Application::ui_thread_pre_init()
{
    int width, height, custom;
    char res_buf[16] = {0};
    string str_w, str_h;
    int ret = 0;

    //adjust resolution without adjust widgets
    if (_vm->get_vmManage()->get_start_vm_is_emulation()) {
        if (_userinfomgr.is_display_info_section_exist("resolution_info")) {
            _userinfomgr.get_display_info(&width, &height, &custom);
            ret = _device_interface->initTermResolution(width, height);
        } else {
            ret = _device_interface->setBestResolution(); 
        }
    } else {
            LOG_INFO("vm start not with emulation");
            if (_userinfomgr.is_display_info_ini_exist()) {
                if (_userinfomgr.delete_display_info_ini() < 0) {
                    LOG_ERR("ui_thread_pre_init delete displayinfo ini err");
                }
            }
            _device_interface->getCurResolution(str_w, str_h, IS_RES_BEST);
            sprintf(res_buf, "%sx%s", str_w.c_str(), str_h.c_str());
            ret = _device_interface->setResolution(res_buf);
    }
    
    return ret;
}

int Application::Req_PushEvent(Event *_event){
	return _process_loop.push_event(_event);
}

#define DEFINE_APPLICATION_API_WITH_PARAM(func_name, input_param_type, tmp_param_type, event_type)\
    int Application::func_name(input_param_type info)\
    {\
        int ret = 0;\
        tmp_param_type* buf = new(tmp_param_type);\
        *buf = info;\
        LogicEvent* event = new LogicEvent(this, event_type, static_cast<void*>(buf), 0);\
        ret = _process_loop.push_event(event);\
        if(ret < SUCCESS)\
        {\
            delete buf;\
        }\
        event->unref();\
        return ((ret >= 0) ? SUCCESS : ret);\
    }

#define DEFINE_APPLICATION_API_WITHOUT_PARAM(func_name, event_type)\
    int Application::func_name()\
    {\
        int ret = 0;\
        LogicEvent* event = new LogicEvent(this, event_type, NULL, 0);\
        ret = _process_loop.push_event(event);\
        event->unref();\
        return ((ret >= 0) ? SUCCESS : ret);\
    }

DEFINE_APPLICATION_API_WITHOUT_PARAM(logic_init_self,                                               L2L_INIT_SELF)
DEFINE_APPLICATION_API_WITHOUT_PARAM(logic_link_up,                                                 L2L_LINK_UP)
DEFINE_APPLICATION_API_WITHOUT_PARAM(logic_link_down,                                               L2L_LINK_DOWN)
DEFINE_APPLICATION_API_WITHOUT_PARAM(logic_link1_up,                                                L2L_LINK1_UP)
DEFINE_APPLICATION_API_WITHOUT_PARAM(logic_link1_down,                                              L2L_LINK1_DOWN)
DEFINE_APPLICATION_API_WITHOUT_PARAM(logic_download_success_tips,                                   L2L_DOWNLOAD_SUCCESS_TIPS)

DEFINE_APPLICATION_API_WITH_PARAM(web_ready,                    const int,          int,            W2L_MINA_READY)

DEFINE_APPLICATION_API_WITHOUT_PARAM(web_upgrade_for_office,                                        W2L_UPGRADE_FOR_OFFICE)
DEFINE_APPLICATION_API_WITH_PARAM(web_upgrade_for_class,        const UpgradeInfo&, UpgradeInfo,    W2L_UPGRADE_FOR_CLASS)
DEFINE_APPLICATION_API_WITHOUT_PARAM(web_sync_deb_path,                                             W2L_SYNC_DEB_PATH)

DEFINE_APPLICATION_API_WITH_PARAM(web_sync_software_version,    const VersionInfo&, VersionInfo,    W2L_SYNC_SOFTWARE_VERSION)
DEFINE_APPLICATION_API_WITH_PARAM(web_sync_public_policy,       const PolicyInfo&,  PolicyInfo,     W2L_SYNC_PUBLIC_POLICY)
DEFINE_APPLICATION_API_WITH_PARAM(web_sync_hostname,            const string&,      string,         W2L_SYNC_HOSTNAME)
DEFINE_APPLICATION_API_WITH_PARAM(web_sync_mode,                const ModeInfo&,    ModeInfo,       W2L_SYNC_MODE)
DEFINE_APPLICATION_API_WITH_PARAM(web_sync_printer_switch,      const int&,         int,            W2L_SYNC_PRINTER_SWITCH)
DEFINE_APPLICATION_API_WITH_PARAM(web_sync_desktop_redir,       const string&,      string,         W2L_SYNC_DESKTOP_REDIR)
DEFINE_APPLICATION_API_WITH_PARAM(web_sync_image,               const ImageInfo&,   ImageInfo,      W2L_SYNC_IMAGE)
DEFINE_APPLICATION_API_WITH_PARAM(web_sync_ipxe,                const IpxeInfo&,    IpxeInfo,       W2L_SYNC_IPXE)
DEFINE_APPLICATION_API_WITH_PARAM(web_sync_recover_image,       const int,          int,            W2L_SYNC_RECOVER_IMAGE)
DEFINE_APPLICATION_API_WITH_PARAM(web_sync_reset_terminal,      const int,          int,            W2L_SYNC_RESET_TERMINAL)
DEFINE_APPLICATION_API_WITH_PARAM(web_sync_server_ip,           const int,          int,            W2L_SYNC_SERVER_IP)
DEFINE_APPLICATION_API_WITH_PARAM(web_sync_all_username,        const UserInfos&,   UserInfos,      W2L_SYNC_ALL_USERINFO)
DEFINE_APPLICATION_API_WITH_PARAM(web_get_dev_interface_info,   const DevInterfaceInfo&, DevInterfaceInfo, W2L_GET_VM_DEV_INTERFACE_INFO)
DEFINE_APPLICATION_API_WITH_PARAM(web_sync_dev_policy,			const DevPolicyInfo&,DevPolicyInfo, W2L_SYNC_DEV_POLICY)
DEFINE_APPLICATION_API_WITH_PARAM(web_sync_driver_install,      const string&,      string,         W2L_SYNC_DRIVER_INSTALL)
//DEFINE_APPLICATION_API_WITH_PARAM(web_sync_reload_image,        const int,          int,            W2L_SYNC_RELOAD_IMAGE)
DEFINE_APPLICATION_API_WITH_PARAM(web_sync_delete_teacherdisk,  const int,          int,            W2L_SYNC_DELETE_TEACHERDISK)
DEFINE_APPLICATION_API_WITH_PARAM(web_sync_port_mapping,        const PortMappingInfos&,            PortMappingInfos,      W2L_SYNC_PORT_MAPPING)
DEFINE_APPLICATION_API_WITH_PARAM(web_sync_ssid_whitelist,      const WhiteList&,   WhiteList,      W2L_SYNC_SSID_WHITELIST)
DEFINE_APPLICATION_API_WITH_PARAM(web_sync_terminal_password,   const string&,      string,         W2L_SYNC_TERMINAL_PSW)
DEFINE_APPLICATION_API_WITH_PARAM(web_sync_rcdusbconf_info,     const string&,      string,         W2L_SYNC_RCDUSBCONF_INFO)
DEFINE_APPLICATION_API_WITH_PARAM(web_sync_main_window,         const MainWindowInfos&, MainWindowInfos, W2L_SYNC_MAIN_WINDOW)

DEFINE_APPLICATION_API_WITH_PARAM(web_init_sync_error,          const int,          int,            W2L_INIT_SYNC_ERROR)
DEFINE_APPLICATION_API_WITH_PARAM(web_image_sync_error,         const int,          int,            W2L_IMAGE_SYNC_ERROR)


DEFINE_APPLICATION_API_WITH_PARAM(web_login_success,            const UserInfo&,    UserInfo,       W2L_LOGIN_SUCCESS)
DEFINE_APPLICATION_API_WITH_PARAM(web_login_fail,               const int,          int,            W2L_LOGIN_FAIL)
DEFINE_APPLICATION_API_WITHOUT_PARAM(web_modify_password_success,                                   W2L_MODIFY_PASSWORD_SUCCESS)
DEFINE_APPLICATION_API_WITH_PARAM(web_modify_password_fail,     const int,          int,            W2L_MODIFY_PASSWORD_FAIL)
DEFINE_APPLICATION_API_WITH_PARAM(web_bind_success,             const UserInfo&,    UserInfo,       W2L_BIND_SECCESS)
DEFINE_APPLICATION_API_WITH_PARAM(web_bind_fail,                const int,          int,            W2L_BIND_FAIL)

DEFINE_APPLICATION_API_WITHOUT_PARAM(web_notify_check_vm_status,                                    W2L_NOTIFY_CHECK_VM_STATUS)
DEFINE_APPLICATION_API_WITHOUT_PARAM(web_notify_shutdown,                                           W2L_NOTIFY_SHUTDOWN)
DEFINE_APPLICATION_API_WITHOUT_PARAM(web_notify_reboot,                                             W2L_NOTIFY_REBOOT)
DEFINE_APPLICATION_API_WITHOUT_PARAM(web_notify_recorver_image,                                     W2L_NOTIFY_RECORVER_IMAGE)
DEFINE_APPLICATION_API_WITH_PARAM(web_notify_modify_password,   const UserInfo&,    UserInfo,       W2L_NOTIFY_MODIFY_PASSWORD)
DEFINE_APPLICATION_API_WITH_PARAM(web_sync_server_time,          const string&,      string,         W2L_SYNC_SERVER_TIME)
DEFINE_APPLICATION_API_WITH_PARAM(web_notify_modify_local_network,const NetworkInfo&,NetworkInfo,   W2L_NOTIFY_MODIFY_LOCAL_NETWORK)
DEFINE_APPLICATION_API_WITH_PARAM(web_notify_modify_vm_network, const NetworkInfo&, NetworkInfo,    W2L_NOTIFY_MODIFY_VM_NETWORK)
DEFINE_APPLICATION_API_WITH_PARAM(web_notify_collect_log,       const FtpLogInfo&, FtpLogInfo,       W2L_NOTIFY_COLLECT_LOG)
DEFINE_APPLICATION_API_WITH_PARAM(web_notify_modify_hostname,   const string&,      string,         W2L_NOTIFY_MODIFY_HOSTNAME)
DEFINE_APPLICATION_API_WITHOUT_PARAM(web_notify_reset_to_initial,                                   W2L_NOTIFY_RESET_TO_INITIAL)
DEFINE_APPLICATION_API_WITH_PARAM(web_notify_resync,            const int,          int,            W2L_NOTIFY_RESYNC)
DEFINE_APPLICATION_API_WITH_PARAM(web_notify_do_ipxe,           const IpxeInfo&,    IpxeInfo,       W2L_NOTIFY_DO_IPXE)
DEFINE_APPLICATION_API_WITH_PARAM(web_notify_ssid_whitelist,    const std::vector<string>&, std::vector<string>,W2L_NODIFY_SSID_WHITELIST)
DEFINE_APPLICATION_API_WITH_PARAM(web_notify_reset_netdisk,     const NetdiskInfo&, NetdiskInfo,    W2L_NODIFY_RESET_NETDISK)

//DEFINE_APPLICATION_API_WITHOUT_PARAM(web_notify_reload_image,                                       W2L_NOTIFY_RELOAD_IMAGE)
DEFINE_APPLICATION_API_WITHOUT_PARAM(web_notify_delete_teacherdisk,                                 W2L_NOTIFY_DELETE_TEACHERDISK)

DEFINE_APPLICATION_API_WITH_PARAM(web_notify_http_port,         const HttpPortInfo&, HttpPortInfo,   W2L_NOTIFY_HTTP_PORT)

DEFINE_APPLICATION_API_WITH_PARAM(web_recv_guesttool_msg,       const string&,      string,         W2L_RECV_GUESTTOOL_MSG)

    

DEFINE_APPLICATION_API_WITHOUT_PARAM(web_mina_connection_established,                               W2L_MINA_CONNECTION_ESTABLISHED)
DEFINE_APPLICATION_API_WITHOUT_PARAM(web_mina_connection_destroyed,                                 W2L_MINA_CONNECTION_DESTROYED)


DEFINE_APPLICATION_API_WITH_PARAM(ui_pass_new_deploy_modeinfo,  const ModeInfo&,    ModeInfo,       U2L_PASS_NEW_DEPLOY_MODEINFO)

DEFINE_APPLICATION_API_WITH_PARAM(ui_set_net_info,              const NetworkInfo&, NetworkInfo,    U2L_SET_NET_INFO)
DEFINE_APPLICATION_API_WITH_PARAM(ui_set_vm_net_info,           const NetworkInfo&, NetworkInfo,    U2L_SET_VM_NET_INFO)
DEFINE_APPLICATION_API_WITH_PARAM(ui_set_server_ip,             const string&,      string,         U2L_SET_SERVER_IP)
DEFINE_APPLICATION_API_WITH_PARAM(ui_set_host_name,             const string&,      string,         U2L_SET_HOST_NAME)
DEFINE_APPLICATION_API_WITH_PARAM(ui_set_wifi_mode,             const wifi_switch_t&,wifi_switch_t, U2L_SET_WIFI_STATUS)
DEFINE_APPLICATION_API_WITH_PARAM(ui_localmode_quit,     		int,  				int,       		U2L_LOCALMODE_QUIT)
DEFINE_APPLICATION_API_WITH_PARAM(ui_localmode_checkerror,     	int,  				int,       		U2L_LOCALMODE_CHECKERROR)
DEFINE_APPLICATION_API_WITH_PARAM(ui_account_login,             const UserInfo&,    UserInfo,       U2L_ACCOUNT_LOGIN)
DEFINE_APPLICATION_API_WITH_PARAM(ui_bind_user,                 int,                int,            U2L_BIND_USER)
DEFINE_APPLICATION_API_WITH_PARAM(ui_set_terminal_mode,         int,                int,            U2L_SET_TERMINAL_MODE)
DEFINE_APPLICATION_API_WITH_PARAM(ui_set_auth_info,             const AuthInfo&,    AuthInfo,       U2L_SET_AUTH_INFO)

DEFINE_APPLICATION_API_WITHOUT_PARAM(ui_shutdown_terminal,                                          U2L_SHUTDOWN_TERMINAL)
DEFINE_APPLICATION_API_WITHOUT_PARAM(ui_reboot_terminal,                                            U2L_REBOOT_TERMINAL)
DEFINE_APPLICATION_API_WITHOUT_PARAM(ui_guest_login,                                                U2L_GUEST_LOGIN)
DEFINE_APPLICATION_API_WITHOUT_PARAM(ui_public_login,                                               U2L_PUBLIC_LOGIN)
DEFINE_APPLICATION_API_WITHOUT_PARAM(ui_establish_connection,                                       U2L_ESTABLISH_CONNECTION)
DEFINE_APPLICATION_API_WITHOUT_PARAM(ui_destroy_connection,                                         U2L_CANCEL_CONNECTION)
DEFINE_APPLICATION_API_WITH_PARAM(ui_modify_passwd,             const UserInfo&,    UserInfo,       U2L_MODIFY_PASSWORD)
DEFINE_APPLICATION_API_WITHOUT_PARAM(ui_enter_local_mode,                                           U2L_ENTER_LOCAL_MODE)
DEFINE_APPLICATION_API_WITHOUT_PARAM(ui_enter_settings,                                             U2L_ENTER_SETTINGS)
DEFINE_APPLICATION_API_WITHOUT_PARAM(ui_enter_wifi_config,                                          U2L_ENTER_WIFI_CONFIG)
DEFINE_APPLICATION_API_WITHOUT_PARAM(ui_enter_auth_config,                                          U2L_ENTER_AUTH_CONFIG)
DEFINE_APPLICATION_API_WITHOUT_PARAM(ui_notify_setting_result,                                      U2L_NOTIFY_SETTING_RESULT)
DEFINE_APPLICATION_API_WITHOUT_PARAM(ui_notify_wifi_config_result,                                  U2L_NOTIFY_WIFI_CONFIG_RESULT)
DEFINE_APPLICATION_API_WITHOUT_PARAM(ui_notify_auth_config_result,                                  U2L_NOTIFY_AUTH_CONFIG_RESULT)
        

DEFINE_APPLICATION_API_WITH_PARAM(ui_save_power_boot,           const int,          int,            U2L_SAVE_POWER_BOOT)
DEFINE_APPLICATION_API_WITHOUT_PARAM(ui_usb_copy_base,                                              U2L_USB_COPY_BASE)
DEFINE_APPLICATION_API_WITHOUT_PARAM(ui_upgrade_for_class,                                          U2L_UPGRADE_FOR_CLASS)


#undef PUSH_EVENT_WITH_PARAM
#undef PUSH_EVENT_WITHOUT_PARAM


int Application::reset_terminal()
{
    int ret;
    
    ret = delete_VM_all();
    if(ret != 0)
    {
        LOG_ERR("delete_VM_all fail!");
        return ERROR_PROCESSING;
    }

    //todo
    //delete all local data;
    return SUCCESS;
}


int Application::delete_VM_all()
{
    //todo
    //delete image
    //delete diff
    //delete data
    int ret = 0;
    LOG_DEBUG("call vm_clear all");
    //ret = _vm->clear_teacher_disk();
    return ret;
}


//web interface
int Application::web_show_ui_disconnect()
{
    if (_ui_locking == false && _status_machine.get_status() != STATUS_CHECKING_LOCAL && !_vm->vm_is_usb_downloading())
    {
        if (_newdeploy_manage->is_new_terminal())
        {
            LOG_INFO("call show_newdeploy_disconnect()");
            _newdeploy_manage->show_newdeploy_disconnect();
        }
        else
        {
            LOG_INFO("call l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT)11111");
            l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT);
        }
    }
    return SUCCESS;
}

int Application::web_sync_dev_policy_complete()
{
    _dev_policy.update_finish();
    if (_init_sync_processing)
    {
        //ensure event L2L_DATA_PROTECT after event W2L_SYNC_DEV_POLICY
        sync_next_event(W2L_SYNC_DEV_POLICY);
    }
    return SUCCESS;
}

int Application::web_do_ipxe_check_env()
{
    if (is_dev_wlan_up()) {
        return IPXE_ACK_WIFI_ENV;
    }
    //if (is_dev_wired_dot1x_up()) {
    //    return IPXE_ACK_DOT1X_ENV;
    //}
    return IPXE_ACK_NONE;
}

//ui interface
int Application::ui_get_basic_info(BasicInfo* info)
{
    if(!info)
        return ERROR_INPUT;
    *info = _basic_data.get();
    
    LOG_DEBUG("unittest basicinfo product name:%s, SN:%s, id=%s, mac=%s, sw ver:%s, hw ver:%s, os ver:%s, cpu:%s, mem:%s, storage:%s", 
                info->product_name.c_str(), info->serial_number.c_str(), info->product_id.c_str(), info->mac.c_str(), info->software_version.c_str(), info->hardware_version.c_str(), 
                info->os_version.c_str(), info->cpu.c_str(), info->memory.c_str(), info->storage.c_str());
    return SUCCESS;
}

int Application::ui_get_net_info(NetworkInfo* info)
{
    if(!info)
        return ERROR_INPUT;
    *info = _local_network_data.get();
    return SUCCESS;
}

int Application::ui_get_vm_net_info(NetworkInfo* info)
{
    if(!info)
        return ERROR_INPUT;

    NetworkInfo netinfo;
    netinfo = _vm->vm_get_vm_netinfo();
    *info = netinfo;
    LOG_DEBUG("ui_get_vm_net_info DHCP:%d, auto dns:%d, ip:%s, summask:%s, gateway:%s, main_dns:%s, back_dns:%s, speed:%d", 
                           info->dhcp, info->auto_dns, info->ip.c_str(), info->submask.c_str(), info->gateway.c_str(), info->main_dns.c_str(), info->back_dns.c_str(), info->netcard_speed);
    return SUCCESS;
}

int Application::ui_get_server_ip(char* ip)
{
    if(!ip)
        return ERROR_INPUT;
    strcpy(ip, _mina->mina_get_server_ip().c_str());
    LOG_DEBUG("server ip is %s", ip);
    return SUCCESS;
}

int Application::ui_get_host_name(char* name)
{
    if(!name)
        return ERROR_INPUT;
    strcpy(name, _hostname_data.get_hostname_info().c_str());
    return SUCCESS;
}


int Application::ui_get_terminal_mode(int* mode, int* readonly)
{
    if(!mode || !readonly)
        return ERROR_INPUT;
    *mode = _mode_data.get().mode;

    /*
        1 new deplay status, we can set mode in l2u_show_settype()
        2 other status, only special mode & unbound can set mode
        3 if disconnected with server, can not set mode
    */
    if (!_mina->mina_get_connected()) {
    	*readonly = 1;
    } else {
    	//if (*mode==SPECIAL_MODE && _vm->vm_check_personal_img_exist()) {
        if (_vm->vm_check_personal_img_exist() || _vm->vm_check_usedisk_exist()) {
    		*readonly = 1;
    	} else {
    		*readonly = 0;
    	}
    }


/*
    if(*mode==SPECIAL_MODE && _mode_data.get().bind_user.username.empty() && _status_machine.get_status()!=STATUS_NEW_DEPLOY && _mina->mina_get_connected()) 
    {
        *readonly = 0;
    }
    else
    {
        *readonly = 1;
    }
*/
    LOG_DEBUG("mode=%d, readonly=%d", *mode, *readonly);
    return SUCCESS;
}


int Application::ui_get_connection_info(int* connected)
{
    if(!connected)
        return ERROR_INPUT;
    if(_mina->mina_get_connected())
    {
        *connected = 1;
    }
    else
    {
        *connected = 0;
    }
    return SUCCESS;
}

int Application::ui_get_last_logined_user(struct UserInfo *user_info)
{
    if (user_info == NULL)
        return -1;
    _userinfomgr.getLastLoginedUser(&user_info->username, &user_info->password,
        &user_info->remember_flag);
    return SUCCESS;
}

int Application::ui_get_bt_service(int* bt_service)
{
    if (bt_service == NULL)
        return -1;
	*bt_service = _bt_service;
	LOG_DEBUG("bt_service=%d", *bt_service);
    return SUCCESS;
}

int Application::ui_get_power_boot(int* power_boot)
{
    if (power_boot == NULL) {
        return -1;
    }
    int ret = get_power_boot(&_power_boot);
    if (ret != 0) {
        return -1;
    } else {
        *power_boot = _power_boot;
        return SUCCESS;
    }
}

int Application::ui_is_new_deploy(int *is_new_deploy)
{
    if (is_new_deploy == NULL)
        return -1;
	*is_new_deploy = (_status_machine.get_status() == STATUS_NEW_DEPLOY);
	LOG_DEBUG("is new deploy=%d", *is_new_deploy);
    return SUCCESS;
}

int Application::ui_is_wifi_terminal(int *wifi_terminal)
{
    if (wifi_terminal == NULL)
        return -1;
    *wifi_terminal = is_wifi_terminal();
    LOG_DEBUG("ui get wifi terminal=%d", *wifi_terminal);
    return SUCCESS;
}

int Application::ui_check_admin_passwd(char* password)
{
    if(strcmp(password, _admin_passwd.c_str()) == 0)
        return SUCCESS;
    else
    {
        LOG_DEBUG("password %s error!", password);
        return -1;
    }
}

void Application::ui_show_next_picture()
{
    //l2u_notify_setting_result();
    ui_notify_setting_result();
}

void Application::ui_show_wifi_next_picture()
{
    //l2u_notify_wifi_config_result();
    ui_notify_wifi_config_result();
}
void Application::ui_show_auth_next_picture()
{
    //l2u_notify_auth_config_result();
    ui_notify_auth_config_result();
}

void Application::ui_cancel_auto_shutdown()
{
    _input_user_info.username.clear();
    _input_user_info.password.clear();
    _status_machine.change_status(EVENT_NEW_DEPLOY_SUCCESS);
}

int Application::ui_show_retry_downloading()
{
	_process_loop.deactivate_interval_timer(_redownload_timer);
	_process_loop.activate_interval_timer(_redownload_timer, 1);
	return 0;
}

void Application::ui_show_login()
{
	l2u_show_userlogin();
	if(!_dev_policy.allow_gusetlogin()) {
		l2u_result_user(11);
	}
	return;
}

void Application::ui_tips_ok(int type)
{
    int status;
    int mode;
    
    mode = _mode_data.get().mode;
    status = _status_machine.get_status();
    _error_tips = false;
    _wait_tips_ok = false;

    LOG_DEBUG("type = %d, _error_tips=%d, status=%d", type, _error_tips, status);

    switch(type)
    {
        //mode or bind user change
        case UI_TIPS_DEV_MODE_CHANGE_RET:
        case UI_TIPS_DEV_USER_CHANGE_RET:
            rc_system("reboot");
            break;

        //reset terminal
        case UI_TIPS_CLIENT_INIT_RET:
            //step1 clear all vm message
        	_dev_status.DevLock(false);
            _vm->vm_clear_all();

            //step2 clear all conf ini.
            rc_system(RCC_SCRIPT_PATH"client_reset_init.sh");
            rc_system("rm -f " UI_USER_LOGO);
            rc_system("rm -f " UI_USER_BG);
            rc_system("rm -rf " USB_POLICY_PATH);
            _local_network_data.clear_wpa_conf();
            _local_network_data.clear_wired_wpa_conf();
            //step3 reboot
            rc_system("reboot");
            break;

        //local mode: no image
        case UI_TIPS_DEV_NO_IMG_RET:
        case UI_TIPS_UPGRADE_FAIL_RET:
        case UI_TIPS_DEV_FORBIDED_RET:
            if(status == STATUS_CHECKING_LOCAL)
            {
                ui_localmode_checkerror(type);
            }
            break;

        //sync web error
        case UI_TIPS_NOCONF_IMAGE_RET:
        case UI_TIPS_SERVER_ERROR_RET:
        case UI_CLIENT_UPDATE_FAIL_RET:
        case UI_TIPS_DEV_OVERRUN_RET:
        case UI_TIPS_LICENSE_OVERRUN_RET:
        case UI_TIPS_IMG_BIG_RET:
        case UI_TIPS_IMG_BAD_DRIVER_RET:
        case UI_TIPS_IMG_USER_ERR_RET:
        case UI_TIPS_IMG_NOT_FOUND_RET:
        case UI_TIPS_IMG_ABNORMAL_RET:
        case UI_TIPS_IMG_VERSION_GT_OUTDATED_RET:
        case UI_TIPS_DEV_NONSPUPPORT_OSTYPE32_RET:
        case UI_TIPS_DEV_NONSPUPPORT_OSTYPEXP_RET:
        case UI_TIPS_DEV_SERVER_MAINTAIN_RET:
            //prevent duplicated confirm
            l2u_show_connecting_wrap();
            logic_reset_status_machine();
            break;

        case UI_TIPS_DELETE_TEACHERDISK_RET:
        case UI_TIPS_RECOVERY_IMAGE_RET:
            //prevent confirm button cannot click if ui locking
            _ui_locking = false;
            if (status == STATUS_INITING) {
                //prevent login ui event ignored in INIT status
                LOG_DEBUG("call recovery image, reset status machine");
                logic_reset_status_machine();
            } else {
                if (mode == PUBLIC_MODE) {
                    LOG_DEBUG("call l2u_show_publiclogin");
                    l2u_show_publiclogin();
                } else {
                    LOG_DEBUG("call l2u_show_userlogin");
                    ui_show_login();
                }
            }
            break;
        default:
            LOG_WARNING("unknown type:%d!", type);

    }
}

int Application::ui_save_bt_service(int bt_service)
{
	if(bt_service == _bt_service)
	{
	    return SUCCESS;
	}
    if(bt_service == 1)
    {
        if (_vm->vm_start_bt_service() == true)
        {
            _bt_service = 1;
            LOG_INFO("ui open bt service success, bt_service=%d", _bt_service);
            return SUCCESS;
        }
        else
        {
            LOG_ERR("ui open bt service fail!");
            return -1;
        }
    }
	else
	{
        if (_vm->vm_stop_bt_service() == true)
        {
            _bt_service = 0;
            LOG_INFO("ui stop bt service success, bt_service=%d", _bt_service);
            return SUCCESS;
        }
        else
        {
            LOG_ERR("ui stop bt service fail!");
            return -1;
        }
	}
}

void Application::ui_read_auth_config(struct AuthInfo &info)
{
    _net_auth->getAuthIni(info);
}

void Application::ui_save_auth_config(struct AuthInfo &auth_info )
{
    _net_auth->saveAuthIni(auth_info);
}

//zhf
int Application::lm_web_update_onLineTime(){
	return LocalMode::LM_WEB_Update_OnLineTime();
}

/**
* function : send unknown dev info to web
* data faormat is xxxx xxxx xxxx xx xx xx xxxx(前几位是ID信息，后三位是产品名称字节长度)
*
 * return : 0: ok -1 error
 */
int Application::vm_upload_unknown_device()
{
    struct Unknown_UsbInfo undev_info;
    string usb_data;
    string usbtmp;
    char *data;
    unsigned int i = 0;
    gsize length = 0;
    unsigned short info_len[POS_MAX] = {0};
    unsigned int head_len = 0, num_len = 0, total_len = 0;

    //read from RCD_SPICE_USBINFO_PATH
    if (!g_file_get_contents(RCD_SPICE_USBINFO_PATH, &data, &length, NULL)) {
        return -1;
    }

    if (!data) {
        return -1;
    }

    usb_data = data;
    LOG_DEBUG("usb_data %s", usb_data.c_str());

    head_len = UNKNOWN_IDV_LEN + UNKNOWN_PID_LEN + UNKNOWN_BCD_LEN + UNKNOWN_DATA_LEN * POS_MAX;
    if (usb_data.length() < head_len) {
        LOG_ERR("usb data format len %d", usb_data.length());
        free(data);
        return -1;
    }

    usbtmp = usb_data.substr(0, UNKNOWN_IDV_LEN);
    undev_info.idv = GetHexStrVal(usbtmp);

    usbtmp = usb_data.substr(UNKNOWN_IDV_LEN, UNKNOWN_PID_LEN);
    undev_info.pid = GetHexStrVal(usbtmp);

    usbtmp = usb_data.substr(UNKNOWN_IDV_LEN + UNKNOWN_PID_LEN, UNKNOWN_BCD_LEN);
    undev_info.bcd = GetHexStrVal(usbtmp);

    num_len = UNKNOWN_IDV_LEN + UNKNOWN_PID_LEN + UNKNOWN_BCD_LEN;
    total_len = 0;
    for (i = 0; i < POS_MAX; i++) {
        usbtmp = usb_data.substr(num_len + i * UNKNOWN_DATA_LEN, UNKNOWN_DATA_LEN);
        info_len[i] = GetHexStrVal(usbtmp);
        total_len += info_len[i];
    }

    if (usb_data.length() < head_len + total_len) {
        LOG_ERR("usb data format error %d", usb_data.length());
        free(data);
        return -1;
    }
    undev_info.manufacturer = usb_data.substr(head_len, info_len[MANUFACTURER_POS]);
    undev_info.product = usb_data.substr(head_len + info_len[MANUFACTURER_POS], info_len[PRODUCT_POS]);
    undev_info.serial = usb_data.substr(head_len + info_len[MANUFACTURER_POS] + info_len[PRODUCT_POS], info_len[SERIAL_POS]);

    LOG_DEBUG("vm_upload_unknown_device %d %d %d %s %s %s", undev_info.idv, undev_info.pid, undev_info.bcd, undev_info.manufacturer.c_str(), undev_info.product.c_str(), undev_info.serial.c_str());
    _mina->mina_web_upload_unknow_devinfo(undev_info);
    free(data);
    return 0;
}

//api for vm  start
int Application::vm_image_exist_status(int status){
	int ret;
    LogicEvent* event = new LogicEvent(this, V2L_IMAGE_EXIST_RESULT, NULL, status);
    ret = _process_loop.push_event(event);
    event->unref();
	return ret;
}

int Application::vm_ui_thread_quit()
{ 
    if (_userinfomgr.UI_permitted())
        l2u_ui_thread_quit();
    // start VM
    if (_userinfomgr.UI_permitted() && !_vmsync_event.ui_check_stopped()) {
        LOG_INFO("Wait until UI stopped.\n");
        _vmsync_event.wait();
    }
    return SUCCESS;
}

int Application::vm_start_vm_status(int status){
	int ret;
	LogicEvent *event = new LogicEvent(this, V2L_VM_START_RESULT, NULL, status);
    ret = _process_loop.push_event(event);
    event->unref();
	return ret;
}

int Application::vm_shutdown_vm_status(int status){
	int ret;
	LogicEvent *event = new LogicEvent(this, V2L_VM_SHUTDOWN_RESULT, NULL, status);
    ret = _process_loop.push_event(event);
    event->unref();
	return ret;
}

int Application::vm_reboot_vm_status(int status){
	int ret;
	LogicEvent *event = new LogicEvent(this, V2L_VM_REBOOT_RESULT, NULL, status);
    ret = _process_loop.push_event(event);
    event->unref();
	return ret;
}

int Application::vm_download_progress_status(struct DownloadInfo *download_info){
	int ret;
	if (NULL == download_info){
		LOG_ERR("input invalid(null pt)");
		return -1;
	}
	struct DownloadInfo *buf = new DownloadInfo;
	*buf = *download_info;
	LogicEvent *eve = new LogicEvent(this, V2L_VM_DOWNLOAD_PROGRESS_STATUS, static_cast<void*>(buf), 0);
    ret = _process_loop.push_event(eve);
    eve->unref();
    if (ret < 0){
    	delete buf;
    }
	return ret;
}

int Application::vm_download_quit_status(int status){
	int ret;
    LogicEvent* event = new LogicEvent(this, V2L_VM_DOWNLOAD_QUIT_RESULT, NULL, status);
    ret = _process_loop.push_event(event);
    event->unref();
	return ret;
}

void Application::vm_usb_download_disconnect(){
    LOG_INFO("enter vm_usb_download_disconnect");
    if(_status_machine.get_status() == STATUS_NEW_DEPLOY) {
        l2u_show_newdeploy_connect(0);
    } else {
	    LOG_INFO("call l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT)22222");
        l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT);
    }
    _process_loop.erase_event(V2L_VM_DOWNLOAD_PROGRESS_STATUS);
    //reset status machine. we could not use mina_destroy_connection, because this case connection has been destroyed.
    logic_reset_status_machine();
}

int Application::vm_get_vm_info_status(VMInfo* vminfo){
	int ret;
	if (NULL == vminfo){
		LOG_ERR("input invalid(null pt)");
		return -1;
	}
	struct VMInfo *buf = new VMInfo;
	*buf = *vminfo;
	LogicEvent *eve = new LogicEvent(this, V2L_GET_VM_INFO_RESULT, static_cast<void*>(buf), 0);
    ret = _process_loop.push_event(eve);
    eve->unref();
    if (ret < 0){
    	delete buf;
    }
	return ret;
}

int Application::vm_set_net_policy_status(int status){
	int ret;
    LogicEvent* event = new LogicEvent(this, V2L_SET_VM_NET_POLICY_RESULT, NULL, status);
    ret = _process_loop.push_event(event);
    event->unref();
	return ret;
}

int Application::vm_set_usb_policy_status(int status){
	int ret;
    LogicEvent* event = new LogicEvent(this, V2L_SET_VM_USB_POLICY_RESULT, NULL, status);
    ret = _process_loop.push_event(event);
    event->unref();
	return ret;
}

int Application::vm_set_vm_netinfo_status(int status){
	int ret;
    LogicEvent* event = new LogicEvent(this, V2L_SET_VM_NETINFO_RESULT, NULL, status);
    ret = _process_loop.push_event(event);
    event->unref();
	return ret;
}

int Application::vm_set_local_wired_network_info(const NetworkInfo& networkInfo)
{
    int ret;
    if(_local_network_data.get_net_status() != NET_STATUS_ETH_UP && _local_network_data.get_net_status() != NET_STATUS_LINK_DOWN)
    {
        LOG_WARNING("local network status is error = %d", _local_network_data.get_net_status());
        return -2;
    }
    NetworkInfo *buf = new (NetworkInfo);
    *buf = networkInfo;
    LogicEvent *eve = new LogicEvent(this, V2L_SET_LOCAL_WIRED_NETWORK, static_cast<void*>(buf), 0);
    ret = _process_loop.push_event(eve);
    eve->unref();
    if (ret < 0){
        delete buf;
    }
    return ret;
}



int Application::vm_get_vm_netinfo_status(NetworkInfo* networkInfo){
	int ret;
	if (NULL == networkInfo){
		LOG_ERR("input invalid(null pt)");
		return -1;
	}
	struct NetworkInfo *buf = new NetworkInfo;
	*buf = *networkInfo;
	LogicEvent *eve = new LogicEvent(this, V2L_GET_VM_NETINFO_RESULT, static_cast<void*>(buf), 0);
    ret = _process_loop.push_event(eve);
    eve->unref();
    if (ret < 0){
    	delete buf;
    }
	return ret;
}

int Application::vm_set_vm_diskinfo_status(int status){
	int ret;
    LogicEvent* event = new LogicEvent(this, V2L_SET_VM_DISKINFO_RESULT, NULL, status);
    ret = _process_loop.push_event(event);
    event->unref();
	return ret;
}


int Application::vm_get_basic_info(BasicInfo* info){
	if(NULL == info){
		LOG_ERR("input param invalid");
		return -1;
	}
	*info = _basic_data.get();

	LOG_DEBUG("Get basicinfo: SN:%s, id=%s, mac=%s, sw ver:%s, hw ver:%s, os ver:%s, cpu:%s, mem:%s, storage:%s,mac2=%s",
				info->serial_number.c_str(), info->product_id.c_str(), info->mac.c_str(), info->software_version.c_str(), info->hardware_version.c_str(),
				info->os_version.c_str(), info->cpu.c_str(), info->memory.c_str(), info->storage.c_str(), info->mac2.c_str());
	return 0;
}

int Application::vm_get_sunny_info(bool *recovery, string *server_ip, string *logined_user)
{
    if (recovery == NULL || server_ip == NULL || logined_user == NULL)
        return -1;
    *recovery = _vm->vm_is_vm_recovery();
    *server_ip = _mina->mina_get_server_ip();
    *logined_user = _userinfomgr.getCurrentLoginedUser();
    return 0;
}

int Application::vm_logout(string input)
{
    string action = RcJson::rc_json_get_string(input, "action");
    if(!_vm->vm_is_vm_running())
    {
        LOG_ERR("vm_logout vm is not running, so return false");
        return -1;
    }
    if((action != "shutdown") && (action != "switchuser"))
    {
        LOG_ERR("vm_logout error key value");
        return -2;
    }

    if(action == "shutdown")
    {
        _shutdown_flag = TERMINAL_SHUTDOWN;
    }
    if(action == "switchuser")
    {
        _shutdown_flag = TERMINAL_LOGOUT;
    }
    _vm->vm_shutdown_VM_normal();
    return SUCCESS;
}

void Application::vm_change_shutdown_flag()
{
    unsigned long check_product;
    #define PRODUCT_RG_IIB_N86AV            0x90007005
    check_product = std::stoul(_basic_data.get().product_id, NULL, 16);
    if ((check_product == PRODUCT_RG_IIB_N86AV ) && (_shutdown_flag == TERMINAL_LOGOUT))      //  logout caused usb -71 error on RG_IIB_N86AV
    {
        LOG_DEBUG("RG_IIB_N86AV user logout,  terminal need reboot");
        _shutdown_flag = TERMINAL_REBOOT;
    }
}

int Application::vm_send_guesttool_info(const string &vm_msg, int source, int target, int module_id)
{
    _mina->mina_guesttool_msg(vm_msg, source, target, module_id);
    return SUCCESS;
}

int Application::vm_stop_redownload_timer()
{
    _process_loop.deactivate_interval_timer(_redownload_timer);
    return SUCCESS;
}

void Application::SetFuncArray()
{
    _event_func[L2L_INIT_SELF]                  = &Application::l2l_init_self;
    _event_func[L2L_EASY_DEPLOY]                = &Application::handle_easy_deploy;
    _event_func[L2L_ENTER_NEW_DEPLOY]           = &Application::l2l_enter_new_deploy;
    _event_func[L2L_ENTER_RUNNING_VM]           = &Application::l2l_enter_running_vm;
    _event_func[L2L_LINK_UP]                    = &Application::l2l_link_up;
    _event_func[L2L_LINK_DOWN]                  = &Application::l2l_link_down;
    _event_func[L2L_LINK1_UP]                    = &Application::l2l_link1_up;
    _event_func[L2L_LINK1_DOWN]                  = &Application::l2l_link1_down;
    _event_func[L2L_DATA_PROTECT]               = &Application::l2l_data_protect_process;   
    _event_func[L2L_DOWNLOAD_SUCCESS_TIPS]      = &Application::l2l_download_success_tips;   

/***UI***/
    _event_func[U2L_PASS_NEW_DEPLOY_MODEINFO]   = &Application::u2l_pass_new_deploy_modeinfo;

    _event_func[U2L_SET_NET_INFO]               = &Application::u2l_set_net_info;
    _event_func[U2L_SET_VM_NET_INFO]            = &Application::u2l_set_vm_net_info;
    _event_func[U2L_SET_SERVER_IP]              = &Application::u2l_set_server_ip;
    _event_func[U2L_SET_HOST_NAME]              = &Application::u2l_set_host_name;
    _event_func[U2L_SET_WIFI_STATUS]            = &Application::u2l_set_wifi_mode;
	_event_func[U2L_LOCALMODE_QUIT]       		= &Application::u2l_localmode_quit;
    _event_func[U2L_LOCALMODE_CHECKERROR]       = &Application::u2l_localmode_checkerror;
    _event_func[U2L_ACCOUNT_LOGIN]              = &Application::u2l_account_login;
    _event_func[U2L_BIND_USER]                  = &Application::u2l_bind_user;
    _event_func[U2L_SET_TERMINAL_MODE]          = &Application::u2l_set_terminal_mode;
    _event_func[U2L_SET_AUTH_INFO]              = &Application::u2l_set_auth_info;
    _event_func[U2L_SHUTDOWN_TERMINAL]          = &Application::u2l_shutdown_terminal;
    _event_func[U2L_REBOOT_TERMINAL]            = &Application::u2l_reboot_terminal;
    _event_func[U2L_GUEST_LOGIN]                = &Application::u2l_guest_login;
    _event_func[U2L_CANCEL_CONNECTION]          = &Application::u2l_cancel_connection;
    _event_func[U2L_ESTABLISH_CONNECTION]       = &Application::u2l_establish_connection;
    _event_func[U2L_PUBLIC_LOGIN]               = &Application::u2l_public_login;
    _event_func[U2L_MODIFY_PASSWORD]            = &Application::u2l_modify_passwd;
    _event_func[U2L_ENTER_LOCAL_MODE]           = &Application::u2l_enter_local_mode;
    _event_func[U2L_ENTER_SETTINGS]             = &Application::u2l_enter_settings;
    _event_func[U2L_ENTER_WIFI_CONFIG]          = &Application::u2l_enter_wifi_config;
    _event_func[U2L_ENTER_AUTH_CONFIG]          = &Application::u2l_enter_auth_config;
    _event_func[U2L_NOTIFY_SETTING_RESULT]      = &Application::l2u_notify_setting_result;
    _event_func[U2L_NOTIFY_WIFI_CONFIG_RESULT]  = &Application::l2u_notify_wifi_config_result;
    _event_func[U2L_NOTIFY_AUTH_CONFIG_RESULT]  = &Application::l2u_notify_auth_config_result;
    _event_func[U2L_SAVE_POWER_BOOT]            = &Application::u2l_save_power_boot;
    _event_func[U2L_USB_COPY_BASE]              = &Application::u2l_usb_copy_base;
    _event_func[U2L_UPGRADE_FOR_CLASS]          = &Application::u2l_upgrade_for_class;
/***UI***/

/***WEB***/
    _event_func[W2L_MINA_READY]                 = &Application::w2l_ready;

    //upgrade for class
    _event_func[W2L_SYNC_DEB_PATH]              = &Application::w2l_sync_deb_path;
    _event_func[W2L_UPGRADE_FOR_CLASS]          = &Application::w2l_upgrade_for_class;

    _event_func[W2L_UPGRADE_FOR_OFFICE]         = &Application::w2l_upgrade_for_office;
    _event_func[W2L_SYNC_SOFTWARE_VERSION]      = &Application::w2l_sync_software_version;
    _event_func[W2L_SYNC_TERMINAL_PSW]          = &Application::w2l_sync_terminal_password;
    _event_func[W2L_SYNC_PUBLIC_POLICY]         = &Application::w2l_sync_public_policy;
    _event_func[W2L_SYNC_MODE]                  = &Application::w2l_sync_mode;
    _event_func[W2L_SYNC_HOSTNAME]              = &Application::w2l_sync_hostname;
    _event_func[W2L_SYNC_IMAGE]                 = &Application::w2l_sync_image;
    _event_func[W2L_SYNC_IPXE]                  = &Application::w2l_sync_ipxe;
    _event_func[W2L_SYNC_RECOVER_IMAGE]         = &Application::w2l_sync_recover_image;
    _event_func[W2L_SYNC_RESET_TERMINAL]        = &Application::w2l_sync_reset_terminal;
    _event_func[W2L_SYNC_SERVER_IP]             = &Application::w2l_sync_server_ip;
    _event_func[W2L_SYNC_ALL_USERINFO]          = &Application::w2l_sync_all_userinfo;
    _event_func[W2L_INIT_SYNC_ERROR]            = &Application::w2l_init_sync_error;
    _event_func[W2L_IMAGE_SYNC_ERROR]           = &Application::w2l_image_sync_error;
    _event_func[W2L_SYNC_SERVER_TIME]            = &Application::w2l_sync_server_time;
    _event_func[W2L_SYNC_DEV_POLICY]            = &Application::w2l_sync_dev_policy;
    _event_func[W2L_SYNC_DRIVER_INSTALL]        = &Application::w2l_sync_driver_install;
    //_event_func[W2L_SYNC_RELOAD_IMAGE]          = &Application::w2l_sync_reload_image;
    _event_func[W2L_SYNC_DELETE_TEACHERDISK]    = &Application::w2l_sync_delete_teacherdisk;
    _event_func[W2L_SYNC_PORT_MAPPING]          = &Application::w2l_sync_port_mapping;
    _event_func[W2L_SYNC_SSID_WHITELIST]        = &Application::w2l_sync_ssid_whitelist;
    _event_func[W2L_SYNC_PRINTER_SWITCH]        = &Application::w2l_sync_printer_switch;
    _event_func[W2L_SYNC_DESKTOP_REDIR]         = &Application::w2l_sync_desktop_redir;
    _event_func[W2L_SYNC_RCDUSBCONF_INFO]       = &Application::w2l_sync_rcdusbconf_info;
    _event_func[W2L_SYNC_MAIN_WINDOW]           = &Application::w2l_sync_main_window;

    _event_func[W2L_LOGIN_SUCCESS]              = &Application::w2l_login_success;
    _event_func[W2L_LOGIN_FAIL]                 = &Application::w2l_login_fail;
    _event_func[W2L_MODIFY_PASSWORD_SUCCESS]    = &Application::w2l_modify_password_success;
    _event_func[W2L_MODIFY_PASSWORD_FAIL]       = &Application::w2l_modify_password_fail;
    _event_func[W2L_BIND_SECCESS]               = &Application::w2l_bind_seccess;
    _event_func[W2L_BIND_FAIL]                  = &Application::w2l_bind_fail;

    _event_func[W2L_NOTIFY_CHECK_VM_STATUS]     = &Application::w2l_notify_check_vm_status;
    _event_func[W2L_NOTIFY_SHUTDOWN]            = &Application::w2l_notify_shutdown;
    _event_func[W2L_NOTIFY_REBOOT]              = &Application::w2l_notify_reboot;
    _event_func[W2L_NOTIFY_RECORVER_IMAGE]      = &Application::w2l_notify_recorver_image;
    _event_func[W2L_NOTIFY_MODIFY_PASSWORD]     = &Application::w2l_notify_modify_password;
    _event_func[W2L_NOTIFY_MODIFY_LOCAL_NETWORK]= &Application::w2l_notify_modify_local_network;
    _event_func[W2L_NOTIFY_MODIFY_VM_NETWORK]   = &Application::w2l_notify_modify_vm_network;
    _event_func[W2L_NOTIFY_COLLECT_LOG]         = &Application::w2l_notify_collect_log;
    _event_func[W2L_NOTIFY_MODIFY_HOSTNAME]     = &Application::w2l_notify_modify_hostname;
    _event_func[W2L_NOTIFY_RESET_TO_INITIAL]    = &Application::w2l_notify_reset_to_initial;
    _event_func[W2L_NOTIFY_RESYNC]              = &Application::w2l_notify_resync;
    _event_func[W2L_NOTIFY_DO_IPXE]             = &Application::w2l_notify_do_ipxe;
    _event_func[W2L_NODIFY_SSID_WHITELIST]      = &Application::w2l_notify_ssid_whitelist;
    _event_func[W2L_NODIFY_RESET_NETDISK]       = &Application::w2l_notify_reset_netdisk;
    //_event_func[W2L_NOTIFY_RELOAD_IMAGE]        = &Application::w2l_notify_reload_image;
    _event_func[W2L_NOTIFY_DELETE_TEACHERDISK]  = &Application::w2l_notify_delete_teacherdisk;

    _event_func[W2L_NOTIFY_HTTP_PORT]           = &Application::w2l_notify_http_port;


    _event_func[W2L_RECV_GUESTTOOL_MSG]         = &Application::w2l_recv_guesttool_msg;


    _event_func[W2L_MINA_CONNECTION_ESTABLISHED]= &Application::w2l_mina_connection_established;
    _event_func[W2L_MINA_CONNECTION_DESTROYED]  = &Application::w2l_mina_connection_destroyed;

/***WEB_END***/

    //zhf
    //v2l api for vm
    _event_func[V2L_IMAGE_EXIST_RESULT] = &Application::v2l_image_exist_result;
    _event_func[V2L_VM_START_RESULT] = &Application::v2l_vm_start_result;
    _event_func[V2L_VM_SHUTDOWN_RESULT] = &Application::v2l_vm_shutdown_result;
    _event_func[V2L_VM_REBOOT_RESULT] = &Application::v2l_vm_reboot_result;
    _event_func[V2L_VM_DOWNLOAD_PROGRESS_STATUS] = &Application::v2l_vm_download_progress_status;
    _event_func[V2L_VM_DOWNLOAD_QUIT_RESULT] = &Application::v2l_vm_download_quit_result;
    _event_func[V2L_GET_VM_INFO_RESULT] = &Application::v2l_get_vm_info_result;
    _event_func[V2L_SET_VM_NET_POLICY_RESULT] = &Application::v2l_set_vm_net_policy_result;
    _event_func[V2L_SET_VM_USB_POLICY_RESULT] = &Application::v2l_set_vm_usb_policy_result;
    _event_func[V2L_SET_VM_NETINFO_RESULT] = &Application::v2l_set_vm_netinfo_result;
    _event_func[V2L_GET_VM_NETINFO_RESULT] = &Application::v2l_get_vm_netinfo_result;
    _event_func[V2L_SET_VM_DISKINFO_RESULT] = &Application::v2l_set_vm_diskinfo_result;
    _event_func[V2L_SET_LOCAL_WIRED_NETWORK] = &Application::v2l_set_local_wired_network_info;

    _event_func[W2L_GET_VM_DEV_INTERFACE_INFO] = &Application::w2l_get_vm_dev_interface_info;
}

int Application::HandleEvent(LogicEvent* event)
{
    int ret;
    int event_type = event->_type;
    
    ASSERT(_event_func.count(event_type) > 0);
    LOG_INFO("Handle event '%s'", logic_event_desc(event->_type).c_str());
    ret = (this->*_event_func[event_type])(event);
    return ret;
}

int Application::logic_reset_status_machine()
{
    bool mina_connected = _mina->mina_get_connected();

    LOG_INFO("enter %s, connected = %d", __func__, mina_connected);
    if (mina_connected)
    {
        _mina->mina_destroy_connection();
    }
    else
    {
        if (_status_machine.get_status() == STATUS_NEW_DEPLOY)
        {
            _newdeploy_manage->reset_status();
        }
        _status_machine.reset_status();
        _status_machine.change_status(EVENT_INIT);
    }
    return SUCCESS;
}

int Application::l2l_init_self(LogicEvent* event)
{
    LOG_INFO("enter %s", __func__);
    _status_machine.change_status(EVENT_INIT);
    return SUCCESS;
}

int Application::l2l_enter_new_deploy(LogicEvent* event)
{
    LOG_INFO("enter %s", __func__);
    _status_machine.change_status(EVENT_ENTER_NEW_DEPLOY);
    return SUCCESS;
}

int Application::l2l_enter_running_vm(LogicEvent* event)
{
    LOG_INFO("enter %s", __func__);
    _status_machine.change_status(EVENT_VM_RUNNING);
    return SUCCESS;
}

int Application::l2l_link_up(LogicEvent* event)
{
    LOG_INFO("enter %s", __func__);

    LOG_NOTICE("link changed: down -> up.");
    if(_vm->vm_is_vm_running())
    {
        if(get_nat_policy())
        {
            _vm->switch_nat();
        }
        else
        {
            _vm->switch_bridge();
        }
    }
    if (!_userinfomgr.on_local_mode()) {
        //on local mode, we'll not change the connection
        if(_vm->vm_get_vm_download_status() == true){
            if (_vm->vm_is_usb_downloading()) {
                LOG_INFO("keep usb downloading when link up");
                return SUCCESS;
            }
        }
        if(_ui_locking == false)
        {
            //if in setting, should not show UI
            if (_mina->mina_get_server_ip().empty()) {
                //server ip is empty
                if (_status_machine.get_status() == STATUS_NEW_DEPLOY) {
                    _newdeploy_manage->show_newdeploy_disconnect();
                } else {
                    l2u_show_unconfig_wrap();
                }
            } else {
                //show connecting
                l2u_show_connecting_wrap();
            }
        }
        if (!_mina->mina_get_server_ip().empty()) {
            if (_status_machine.get_status() == STATUS_NEW_DEPLOY) {
                _newdeploy_manage->quit_new_deploy_abnormal();                
            } else {
                _mina->mina_establish_connection();
            }
        }
    }
    if (_vm->vm_is_vm_running() && _vm->get_net_policy())
        _vm->vm_enable_netuse();

    return SUCCESS;
}

int Application::l2l_link_down(LogicEvent* event)
{
    LOG_INFO("enter %s", __func__);

    LOG_NOTICE("link changed: up -> down.");
    if (!_userinfomgr.on_local_mode()) {
        //on local mode, we'll not change the connection
        if(_vm->vm_get_vm_download_status() == true){
            LOG_DEBUG("stop download image first");
            if (_vm->vm_is_usb_downloading()) {
                LOG_INFO("keep usb downloading when link down");
                return SUCCESS;
            }
            _vm->vm_quit_download_image();
            _process_loop.erase_event(V2L_VM_DOWNLOAD_PROGRESS_STATUS);
        }

        if (_ui_locking == false) {
            /**
                     * other status would change to STATUS_INITING after re-connected,
                     * so should not show disconnect UI
                     * if in setting, should not show UI
                     */
            if (_status_machine.get_status() == STATUS_INITING && !_mina->mina_get_server_ip().empty()) {
                LOG_DEBUG("call l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT)");
				LOG_INFO("call l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT)33333");
                l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT);
            }
            else if(_newdeploy_manage->is_new_terminal()) {
                l2u_show_newdeploy_connect(0);
            }
        }

        //wait until setting done, or change to STATUS_INITING _ui_need_sync_flag & _ui_has_sync_flag would be cleared!                              
        while (_ui_need_sync_flag != 0) {
            sleep(1);
        }
        if (_on_easy_deploy) {
            //we do not destroy connection until easy deploy finished.
            LOG_NOTICE("web would be destroyed after easy deploy.");
        } else {
            if (_status_machine.get_status() == STATUS_NEW_DEPLOY) {
                _newdeploy_manage->reset_status();
            }
            _mina->mina_destroy_connection();
        }
    }
    if (_vm->vm_is_vm_running() && _vm->get_net_policy())
        _vm->vm_disable_netuse();

    return SUCCESS;
}

int Application::l2l_link1_up(LogicEvent* event)
{
    LOG_INFO("enter %s", __func__);
    DevInterfaceInfo inter_info;
    get_UsrUserInfoMgr()->get_dev_interface_info(inter_info);
    LOG_NOTICE("link1 changed: down -> up.");
    if (_vm->vm_is_vm_running() && inter_info.net_passthrough == 1)
        _vm->vm_enable_net2use();

    return SUCCESS;
}

int Application::l2l_link1_down(LogicEvent* event)
{
    LOG_INFO("enter %s", __func__);
    DevInterfaceInfo inter_info;
    get_UsrUserInfoMgr()->get_dev_interface_info(inter_info);
    LOG_NOTICE("link1 changed: up -> down.");
    if (_vm->vm_is_vm_running() && inter_info.net_passthrough == 1)
        _vm->vm_disable_net2use();

    return SUCCESS;
}

int Application::l2l_data_protect_process(LogicEvent* event)
{
    LOG_INFO("enter %s", __func__);
    if(!_dev_status.IsLocked()) {
        LOG_DEBUG("no need to process data protect if dev unlocked");
        return SUCCESS;
    }
    ModeInfo      local_mode_info, server_mode_info;
    DevPolicyInfo local_dev_info,  server_dev_info;
    int lock_type = 0;
    
    local_mode_info  = _mode_data.get();
    server_mode_info = _dev_status.get_mode_info();
    local_dev_info   = _dev_policy.get();
    server_dev_info  = _dev_status.get_dev_policy();
    
    /* keep dev locked if bind username changed
     * keep dev locked if devpolicy will remove img or usedisk
     */
    if (local_mode_info.mode != server_mode_info.mode) {
        LOG_INFO("keep dev locked, because mode changed!");
        lock_type |= UI_DEV_LOCK_DIFF_MODE;
    }
    if (local_mode_info.bind_user.username != server_mode_info.bind_user.username) {
        LOG_INFO("keep dev locked, because bind user changed!");
        lock_type |= UI_DEV_LOCK_DIFF_BINDUSER;
    }
    if (_vm->vm_check_personal_img_exist() && local_dev_info.allow_recovery == false && server_dev_info.allow_recovery == true) {
        LOG_INFO("keep dev locked, because server's dev policy will remove personal img!");
        lock_type |= UI_DEV_LOCK_DIFF_RECOVERY;
    }
    if (_vm->vm_check_usedisk_exist() && local_dev_info.allow_userdisk == true && server_dev_info.allow_userdisk == false) {
        LOG_INFO("keep dev locked, because server's dev policy will remove userdisk!");
        lock_type |= UI_DEV_LOCK_DIFF_USERDISK;
    }

    if (lock_type == 0) {
        LOG_INFO("dev unlocked!!!");
		_allow_newdeploy = false;
        _newdeploy_manage->set_new_terminal(false);
        _mina->mina_set_last_server_ip(_mina->mina_get_server_ip());
	    _dev_status.DevLock(false);
	} else {
        _dev_status.set_lock_type(lock_type);
    }

    if (!_dev_status.IsLocked()) {
        if(local_mode_info != server_mode_info) {
            _mode_data.set(_dev_status.get_mode_info());
            _vm->vm_clear_teacher_disk();
            _vm->vm_clear_inst();
            _vm->vm_clear_layer();
        }
        if(local_dev_info != server_dev_info) {
            if (server_dev_info.allow_recovery == true) {
                _vm->vm_clear_inst();
                _vm->vm_clear_layer();
            }    
            if (server_dev_info.allow_userdisk == false) {
            	_vm->vm_clear_teacher_disk();
            }
            _dev_policy.update(server_dev_info);
        }
    }
    return SUCCESS;
}

int Application::l2l_download_success_tips(LogicEvent* event)
{
    bool flag = true;
    LOG_INFO("enter %s", __func__);
    UIPopEvent sync_event("merge tips");
    l2u_show_need_merge_tips((void *)Application::uipop_sync, (void *)&sync_event, (void *)&flag);
    //sync_event.wait();
    sleep(5);
    if(_vm->vm_is_vm_running())
        return SUCCESS;
    rc_system("reboot");
    return SUCCESS;
}

int Application::u2l_pass_new_deploy_modeinfo(LogicEvent* event)
{
    ModeInfo local_modeinfo_buf;
	ModeInfo* modeinfo;
	LOG_DEBUG("enter %s", __func__);
	modeinfo = static_cast<ModeInfo*>(event->_data);
	ASSERT(modeinfo);
	if(event->_error != SUCCESS)
	{
		delete modeinfo;
		return event->_error;
	}

    if (_new_deploy_processing) {
        LOG_WARNING("Duplicated new device deploy request, ignore it!\n");
        return ERROR_DUPLICATED_DEPLOY;
    }
    _new_deploy_processing = true;
    _input_user_info = modeinfo->bind_user;

    local_modeinfo_buf = _mode_data.get();
    local_modeinfo_buf.mode = modeinfo->mode;
    if(local_modeinfo_buf.mode != SPECIAL_MODE)
    {
        local_modeinfo_buf.bind_user.username.clear();
    }
    _mode_data.set(local_modeinfo_buf);
    
	_mina->mina_l2w_change_mode(modeinfo->mode);

	if (modeinfo->mode == MULTIUSER_MODE || modeinfo->mode == PUBLIC_MODE) {
		_mina->mina_web_request_dev_policy(modeinfo->mode, "");
		_dev_policy.wait4update(2*1000*1000*1000);
	}

	if(modeinfo->mode == SPECIAL_MODE && (!(modeinfo->bind_user.username.empty())))
	{
		_mina->mina_web_bind(modeinfo->bind_user);
	}
	else
	{
		_mina->mina_web_request_image(_vm->vm_get_vm_imageinfo());
	}
	delete modeinfo;
	return SUCCESS;
}

int Application::u2l_set_net_info(LogicEvent* event)
{
    int status;
    int ret = SUCCESS;
    NetworkInfo* netinfo;
    
    LOG_DEBUG("enter %s, _ui_need_sync_flag=%d && _ui_has_sync_flag=%d", __func__, _ui_need_sync_flag, _ui_has_sync_flag);
    
    netinfo = static_cast<NetworkInfo *>(event->_data);
    if(netinfo == NULL)
    {
        ret = ERROR_INPUT;
    }
    else
    {
        status = _status_machine.get_status();
        if(status!=STATUS_NONE && status!=STATUS_INITING && status!=STATUS_WAITING_LOGIN && status!=STATUS_CHECKING_LOCAL && status!=STATUS_NEW_DEPLOY && status!=STATUS_PREPARING_IMAGE)
        {
            LOG_ERR("error! state:%d, event:%s!", status, EVENT_TO_STR(U2L_SET_NET_INFO));
            ret = ERROR_EVENT;
        }
        else if(is_dev_wlan_up())
        {            
            LOG_ERR("error! cannot save netinfo if wlan up!");
            ret = ERROR_EVENT;
            //ret = ERROR_EVENT_WLAN_SAVE_IP;
        }
        else
        {
            LOG_DEBUG("u2l_set_net_info DHCP:%d, auto dns:%d, ip:%s, summask:%s, gateway:%s, main_dns:%s, back_dns:%s, speed:%d", 
                           netinfo->dhcp, netinfo->auto_dns, netinfo->ip.c_str(), netinfo->submask.c_str(), netinfo->gateway.c_str(), netinfo->main_dns.c_str(), netinfo->back_dns.c_str(), netinfo->netcard_speed);
            //async call, get result in callback
            _local_network_data.set(*netinfo);
        }
        delete netinfo;
    }

    if(ret != SUCCESS)
    {
        _ui_setting_error |= UI_NET_INFO_ERR;
        _ui_has_sync_flag |= UI_NET_INFO_SYNC;
        if(_ui_need_sync_flag!=0 && _ui_has_sync_flag ==_ui_need_sync_flag)
        {
            //l2u_notify_setting_result();
            ui_notify_setting_result();
        }
    }
    //if success, set _ui_has_sync_flag & l2u_notify_setting_result in set_local_network_callback, for setting net info always cost a lot of time
    
    return ret;
}


int Application::u2l_set_vm_net_info(LogicEvent* event)
{
    int status;
    int ret = SUCCESS;
    NetworkInfo* netinfo;
    
    LOG_DEBUG("enter %s, _ui_need_sync_flag=%d && _ui_has_sync_flag=%d", __func__, _ui_need_sync_flag, _ui_has_sync_flag);
    
    netinfo = static_cast<NetworkInfo *>(event->_data);
    if(netinfo == NULL)
    {
        ret = ERROR_INPUT;
    }
    else
    {
        status = _status_machine.get_status();
        if(status!=STATUS_NONE && status != STATUS_INITING && status != STATUS_WAITING_LOGIN
            && status != STATUS_CHECKING_LOCAL && status!=STATUS_NEW_DEPLOY && status!=STATUS_PREPARING_IMAGE) {
            LOG_ERR("error! state:%d, event:%s!", status, EVENT_TO_STR(U2L_SET_VM_NET_INFO));
            ret = ERROR_EVENT;
        } else if(is_dev_wlan_up()) {            
            LOG_ERR("error! cannot save vmnetinfo if wlan up!");
            ret = ERROR_EVENT;
            //ret = ERROR_EVENT_WLAN_SAVE_IP;
        } else {
            _vm->vm_set_vm_netinfo(netinfo, status);
            _mina->mina_web_upload_vm_network_info(*netinfo, _vm->get_vm_mac());
        }
        delete netinfo;
    }

    if(ret != SUCCESS)
    {
        _ui_setting_error |= UI_VM_NET_INFO_ERR;
    }
    _ui_has_sync_flag |= UI_VM_NET_INFO_SYNC;
    if(_ui_need_sync_flag!=0 && _ui_has_sync_flag ==_ui_need_sync_flag)
    {
        //l2u_notify_setting_result();
        ui_notify_setting_result();
    }
    
    return ret;
}

void Application:: clear_iptables_upon_ui_serverip_changed() {
    HttpPortInfo hpi;

    hpi.public_ip = "";
    hpi.private_ip = "";
    hpi.btPortsRange = "6881:7180";
    hpi.public_port = 80;
    hpi.private_port = 80;

    LOG_DEBUG("UI server ip changed, clear iptables.\n");
    _userinfomgr.set_http_port_map(hpi);
    _local_network_data.set_port_convert();
    _userinfomgr.set_http_last_port_map(hpi);

    return;

}

int Application::u2l_set_server_ip(LogicEvent* event)
{
    int status;
    int ret = SUCCESS;
    string* ip_str;
    
    LOG_DEBUG("enter %s, _ui_need_sync_flag=%d && _ui_has_sync_flag=%d", __func__, _ui_need_sync_flag, _ui_has_sync_flag);
    
    ip_str = static_cast<string*>(event->_data);
    if(ip_str == NULL)
    {
        LOG_ERR("ip_str == NULL, %p", event->_data);
        ret = ERROR_INPUT;
    }
    else
    {
        status = _status_machine.get_status();
        if(status!=STATUS_NONE && status!=STATUS_INITING && status!=STATUS_WAITING_LOGIN && status!=STATUS_CHECKING_LOCAL && status!=STATUS_NEW_DEPLOY && status!=STATUS_PREPARING_IMAGE)
        {
            LOG_ERR("error! state:%d, event:%s!", status, EVENT_TO_STR(U2L_SET_SERVER_IP));
            ret = ERROR_EVENT;
        }
        else
        {
            string server_ip = *ip_str;
            if(server_ip != _mina->mina_get_server_ip())
            {
                _configure_serverip_sync_flag = true;
                clear_iptables_upon_ui_serverip_changed();
            }
            ret = _mina->mina_set_server_ip(server_ip);
            if (ret != 0) 
            {
                LOG_ERR("mina_set_server_ip fail, ret=%d", ret);
                ret = ERROR_PROCESSING;
            }
        }
        delete ip_str;
    }

    if(ret != SUCCESS)
    {
        _ui_setting_error |= UI_SERVER_IP_ERR;
    }
    _ui_has_sync_flag |= UI_SERVER_IP_SYNC;
    if(_ui_need_sync_flag!=0 && _ui_has_sync_flag ==_ui_need_sync_flag)
    {
        //l2u_notify_setting_result();
        ui_notify_setting_result();
    }
    
    return ret;
}

int Application::u2l_set_host_name(LogicEvent* event)
{
    string* hostname;
    int status;
    int ret = SUCCESS;

    LOG_DEBUG("enter %s, _ui_need_sync_flag=%d && _ui_has_sync_flag=%d", __func__, _ui_need_sync_flag, _ui_has_sync_flag);
    
    hostname = static_cast<string*>(event->_data);
    if(hostname == NULL)
    {
        LOG_ERR("hostname is NULL!");
        ret = ERROR_INPUT;
    }
    else
    {
        status = _status_machine.get_status();
        if(status!=STATUS_NONE && status!=STATUS_INITING && status!=STATUS_WAITING_LOGIN && status!=STATUS_CHECKING_LOCAL && status!=STATUS_NEW_DEPLOY && status!=STATUS_PREPARING_IMAGE)
        {
            LOG_ERR("error! state:%d, event:%s!", status, EVENT_TO_STR(U2L_SET_HOST_NAME));
            ret = ERROR_EVENT;
        }
        else
        {
            if(*hostname != _hostname_data.get_hostname_info())
            {
                _configure_hostname = *hostname;
            }
            LOG_INFO("UI set hostname:%s",hostname->c_str());
            //save hostname to local ini
            _hostname_data.set_hostname_info(*hostname);
            //sync hostname to web
            _mina->mina_l2w_change_hostname(*hostname);
            ret = SUCCESS;
        }
        delete hostname;
    }

    if(ret != SUCCESS)
    {
        _ui_setting_error |= UI_HOSTNAME_ERR;
    }
    _ui_has_sync_flag |= UI_HOSTNAME_SYNC;
    if(_ui_need_sync_flag!=0 && _ui_has_sync_flag ==_ui_need_sync_flag)
    {
        //l2u_notify_setting_result();
        ui_notify_setting_result();
    }

    return ret;
}

int Application::_config_auth_cancel_handle()
{
    int auth_result;
    int ret;

    auth_result = _net_auth->stopAuthentication();
    if (auth_result != 0) {
        LOG_INFO("cancel auth fail, interface err");
        ret = ERROR_PROCESSING;
    } else {
        ret = SUCCESS;
    }

    return ret;
}

int Application::_config_auth_success_handle()
{
    int auth_result;

    auth_result = _net_auth->startAuthentication();
    if (auth_result != 0) {
        LOG_INFO("start auth fail, interface err");
        return ERROR_PROCESSING;
    } 

    return SUCCESS;
}

int Application::u2l_set_auth_info(LogicEvent* event)
{
    int status = 0;
    int ret = SUCCESS;
    AuthInfo* authinfo;

    LOG_DEBUG("u2l_set_auth_info, status=%d, _ui_locking!", status);

    authinfo = static_cast<AuthInfo *>(event->_data);
    if(authinfo == NULL) {
        LOG_ERR("auth info is NULL!");
        ret = ERROR_INPUT;
    } else {
        status = _status_machine.get_status();
        if(status != STATUS_NONE && status != STATUS_INITING && status != STATUS_WAITING_LOGIN
                && status != STATUS_CHECKING_LOCAL && status!=STATUS_NEW_DEPLOY && status!=STATUS_PREPARING_IMAGE) {
            LOG_ERR("error! state:%d, event:%s!", status, EVENT_TO_STR(U2L_SET_AUTH_INFO));
            ret = ERROR_EVENT;
        } else {
            switch (authinfo->auth_type) {
            case AUTH_NONE:
                ret = _config_auth_cancel_handle();
                break;
            case AUTH_DOT1X:
                ret = _config_auth_success_handle();
                break;
            default:
                LOG_ERR("not support auth type, type = %d", authinfo->auth_type);
                break;
            }
        }
        delete authinfo;
    }
    return ret;
}

int Application::u2l_localmode_loginauth(UserInfo* userinfo){
	if (STATUS_CHECKING_LOCAL != _status_machine.get_status()){
		LOG_ERR("v2l localmode loginauth ack in invalid mode");
		return -1;
	}
	if (NULL == localmode){
		LOG_ERR("invalid localmode obj");
		return -1;
	}
	if (userinfo) {
		if (0 != localmode->UI_Ack_LM_LoginAuth(userinfo->username, gloox::password_codec_xor(userinfo->password, true),
            userinfo->remember_flag)) {
			LOG_INFO("LoginAuth failed");
			return -1;
		}
	}else {
		LOG_ERR("invalid userinfo obj");
		return -1;
	}
	return 0;
}

int Application::u2l_localmode_quit(LogicEvent* event)
{
    int status;
    int* result;

    LOG_DEBUG("enter %s", __func__);

    _ui_locking = false;//规避解决bug 517408
    result = static_cast<int*>(event->_data);

    status = _status_machine.get_status();
    if((status == STATUS_CHECKING_LOCAL) && localmode){
    	LOG_INFO("quit localmode");
    	if (localmode->LM_LocalMode_Quit(*result) < 0){
    		LOG_ERR("send EVENT_LOCAL_CHECK_FAILED");
            _userinfomgr.leave_local_mode();
    		_status_machine.change_status(EVENT_LOCAL_CHECK_FAILED);
    	}
    } else {
    	LOG_ERR("status error");
    }
    delete result;

    return SUCCESS;
}

int Application::u2l_localmode_checkerror(LogicEvent* event)
{
    int status;
    int* result;

    LOG_DEBUG("enter %s", __func__);

    result = static_cast<int*>(event->_data);

    status = _status_machine.get_status();
    if((status == STATUS_CHECKING_LOCAL) && localmode){
    	if (localmode->UI_Ack_LM_CheckError(*result) < 0){
    		LOG_ERR("send EVENT_LOCAL_CHECK_FAILED");
            _userinfomgr.leave_local_mode();
    		_status_machine.change_status(EVENT_LOCAL_CHECK_FAILED);
    	}
    } else {
    	LOG_ERR("status error");
    }
    delete result;

    return SUCCESS;
}

int Application::u2l_account_login(LogicEvent* event)
{
    UserInfo* userinfo;
    int status;
    int ret;

    LOG_DEBUG("enter %s", __func__);
    userinfo = static_cast<UserInfo*>(event->_data);
    status = _status_machine.get_status();

    if (userinfo == NULL)
    {
        return ERROR_INPUT;
    }

    //if bind_username is not null  and not equal  userinfo->username
    if (!_dev_policy.allow_otherslogin() && _mode_data.get().mode == SPECIAL_MODE) {
    	if (!_mode_data.get().bind_user.username.empty() && userinfo->username != _mode_data.get().bind_user.username) {
            LOG_ERR("username don't allow login!");
            l2u_result_user(4);
            goto end;
    	}
    }

    if (STATUS_CHECKING_LOCAL == status) 
    {
		ret = u2l_localmode_loginauth(userinfo);
		delete userinfo;
		if (ret == 0) {
			return SUCCESS;
		} else {
		    return ERROR_PROCESSING;
		}
    }
    
    if (status != STATUS_WAITING_LOGIN) 
    { 
        LOG_ERR("error! state:%d, event:%s!", status, EVENT_TO_STR(U2L_ACCOUNT_LOGIN));
        delete userinfo;
        return ERROR_EVENT;
    }


    if(userinfo->username.empty() || userinfo->password.empty())
    {
        LOG_ERR("username or password is empty!");
        l2u_result_user(1);
    } else if (rc_check_password(userinfo->password.c_str(), 32)) {
        LOG_ERR("password has special characters!");
        l2u_result_user(1);
    } else
    {
        _input_user_info.username = userinfo->username;
        _input_user_info.password = gloox::password_codec_xor(userinfo->password, true);
        _input_user_info.remember_flag = userinfo->remember_flag;
        _status_machine.change_status(EVENT_LOGIN_BUTTON_ENTERED);
    }
end:
    delete userinfo;
    
    return SUCCESS;
}

int Application::u2l_bind_user(LogicEvent* event)
{
    int status;
    int* bind;
    
    LOG_DEBUG("enter %s", __func__);
    status = _status_machine.get_status();
    if(status != STATUS_CHECKING_LOGIN)
    {
        LOG_ERR("error! state:%d, event:%s!", status, EVENT_TO_STR(U2L_BIND_USER));
        return ERROR_EVENT;
    }

    bind = static_cast<int*>(event->_data);
    //bind: 1: bind, 0: cancel binding
    if(*bind) 
    {
        delete bind;
        
        ModeInfo modeinfo = _mode_data.get();
        modeinfo.mode = SPECIAL_MODE;
        modeinfo.bind_user.username = _input_user_info.username;
        modeinfo.bind_user.password = _input_user_info.password;

        //notify web
        int ret = _mina->mina_web_bind(modeinfo.bind_user);
        if (ret != 0) 
        {
            LOG_ERR("mina_web_bind fail, ret=%d", ret);
            return ERROR_PROCESSING;
        }
    }
    else
    {
        //cancel binding, return login UI
        ui_show_login();
        _status_machine.change_status(EVENT_LOGIN_FAILED);
    }

    return SUCCESS;
}

int Application::u2l_set_wifi_mode(LogicEvent* event)
{
    struct wifi_switch_t* sw_info;
    int status;
    Application* app;
    int ret = SUCCESS;
    std::vector<string> ssid_list;
    list<net::WifiConfig> his_list;
    
    LOG_DEBUG("enter %s", __func__);

    sw_info = static_cast<struct wifi_switch_t *>(event->_data);
    if (sw_info == NULL) {
        ret = ERROR_INPUT;
    } else {
        status = _status_machine.get_status();

        if(status!=STATUS_NONE && status!=STATUS_INITING && status!=STATUS_WAITING_LOGIN && status!=STATUS_NEW_DEPLOY && status!=STATUS_PREPARING_IMAGE)
        {        
            LOG_ERR("error! state:%d, event:%s!", status, EVENT_TO_STR(U2L_SET_TERMINAL_MODE));
            ret = ERROR_EVENT;
        } else {      
            app = Application::get_application();

            if (sw_info->new_wifi_status != sw_info->old_wifi_status) {
                app->get_wifi_interactive()->wifi_enable_button_handle(sw_info->new_wifi_status);
            }
            
            if (sw_info->new_wssid_status == FALSE && sw_info->old_wssid_status == TRUE) {
                ret = app->get_UsrUserInfoMgr()->deletessidWhite();
                if (ret != 0) {
                    LOG_ERR("deletessidWhite fail, ret=%d", ret);
                    ret = ERROR_PROCESSING;
                } else {
                     ret = app->get_wifi_interactive()->set_white_ssid_list(ssid_list);
                    if (ret != 0) {
                        LOG_ERR("set_white_ssid_list fail, ret=%d", ret);
                        ret = ERROR_PROCESSING;
                    }
                }              
            }
        }
        delete sw_info;
    }
    if (ret != SUCCESS) {
        _ui_setting_error |= UI_WIFI_ERR; 
    }
    
    _ui_has_sync_flag |= UI_WIFI_STATUS_SYNC;
    if(_ui_need_sync_flag != 0  && _ui_has_sync_flag == _ui_need_sync_flag)
    {
        //l2u_notify_setting_result();
        ui_notify_setting_result();
    }  
    LOG_DEBUG("leave set wifi mode");
    return ret;
}

int Application::u2l_set_terminal_mode(LogicEvent* event)
{
	int* mode;
    int status;
    int newmode;
    int ret = SUCCESS;
    
    LOG_DEBUG("enter %s", __func__);
    mode = static_cast<int*>(event->_data);
    
    if(mode == NULL)
    {
        ret = ERROR_INPUT;
    }
    else
    {
        status = _status_machine.get_status();

        if(status!=STATUS_NONE && status!=STATUS_INITING && status!=STATUS_WAITING_LOGIN && status!=STATUS_NEW_DEPLOY && status!=STATUS_PREPARING_IMAGE)
        {
            LOG_ERR("error! state:%d, event:%s!", status, EVENT_TO_STR(U2L_SET_TERMINAL_MODE));
            ret = ERROR_EVENT;
        }
        else
        {
            mode = static_cast<int*>(event->_data);
          
            newmode = *mode;
            delete mode;
            LOG_INFO("get mode %d from UI", newmode);
            if(newmode != _mode_data.get().mode)
            { 
                LOG_INFO("change terminal mode from %d to %d", _mode_data.get().mode, newmode);

                // 1 delete data & inst will clear before image downloading
                ret = _vm->vm_clear_teacher_disk();
                ret = _vm->vm_clear_inst();
                ret = _vm->vm_clear_layer();

                // 2 set new mode to local
                ModeInfo modeinfo;
                //modeinfo = _mode_data.get();      //don't need bind user info
                modeinfo.mode = (Mode)newmode;
                _mode_data.set(modeinfo);

                // 3 notify web
                ret = _mina->mina_l2w_change_mode((Mode)newmode);
                _configure_mode = newmode;
                
                if(ret != SUCCESS)
                {
                    _ui_setting_error |= UI_MODE_ERR;
                }
                else
                {
                    _ui_has_sync_flag |= UI_MODE_SYNC;

                    // 4  notify UI
                    if(_ui_need_sync_flag!=0 && _ui_has_sync_flag ==_ui_need_sync_flag)
                    {
                        //l2u_notify_setting_result();
                        ui_notify_setting_result();
                    }

                    //sleep(5);
                    //5 reset status
                    LOG_INFO("reset _status_machine!");
                    logic_reset_status_machine();
                }
            }
        }
    }

    //if set mode fail, notify_setting_result here
    if(ret != SUCCESS)
    {
        _ui_setting_error |= UI_MODE_ERR;
        _ui_has_sync_flag |= UI_MODE_SYNC;
        if(_ui_need_sync_flag!=0 && _ui_has_sync_flag ==_ui_need_sync_flag)
        {
            //l2u_notify_setting_result();
            ui_notify_setting_result();
        }
    }
    
    return ret;
}

#if 0
int Application::u2l_check_admin_passwd(LogicEvent* event)
{
    int status;
    char* passwd;    
    
    LOG_DEBUG("enter %s", __func__);
    status = _status_machine.get_status();
    if(status!=STATUS_INITING && status!=STATUS_WAITING_LOGIN)
    {
        LOG_ERR("error! state:%d, event:%s!", status, EVENT_TO_STR(U2L_CHECK_ADMIN_PASSWD));
        return ERROR_EVENT;
    }

    passwd = static_cast<char*>(event->_data);
    if(passwd != NULL)
    {  
        if(strcmp(passwd, _admin_passwd.c_str()))
        {
            //notify UI login fail
            l2u_result_user(false);
            LOG_INFO("password error! show admin passwd error UI");
        }
        else
        {
            //notify UI login success
            LOG_INFO("password correct! show setting UI");
        }
        return SUCCESS;
    }
    else
    {
        return ERROR_INPUT;
    }
}
#endif

int Application::u2l_shutdown_terminal(LogicEvent* event)
{
    int status;
    
    LOG_DEBUG("%s enter", __func__);
	status = _status_machine.get_status();
    if(status!=STATUS_INITING && status!=STATUS_WAITING_LOGIN && status!=STATUS_CHECKING_LOCAL && status!=STATUS_NEW_DEPLOY && status!=STATUS_PREPARING_IMAGE)
    {
        LOG_ERR("error! state:%d, event:%s!", status, EVENT_TO_STR(U2L_SHUTDOWN_TERMINAL));
        return ERROR_EVENT;
    }
    
    int ret = rc_system("shutdown -h now");
    if (ret != 0) 
    {
        LOG_ERR("%s fail, ret=%d", __func__, ret);
        return ERROR_PROCESSING;
    }

    return SUCCESS;
}

int Application::u2l_reboot_terminal(LogicEvent* event)
{
    int status;
    
    LOG_DEBUG("%s enter", __func__);
    status = _status_machine.get_status();
    if(status!=STATUS_INITING && status!=STATUS_WAITING_LOGIN && status!=STATUS_CHECKING_LOCAL && status!=STATUS_NEW_DEPLOY && status!=STATUS_PREPARING_IMAGE)
    {
        LOG_ERR("error! state:%d, event:%s!", status, EVENT_TO_STR(U2L_REBOOT_TERMINAL));
        return ERROR_EVENT;
    }

    int ret = rc_system("reboot");
    if (ret != 0) 
    {
        LOG_ERR("%s fail, ret=%d", __func__, ret);
        return ERROR_INPUT;
    }

    return SUCCESS;
}


int Application::u2l_guest_login(LogicEvent* event)
{
    int status;
    bool image_exist = false;
        
    LOG_DEBUG("%s enter", __func__);
    status = _status_machine.get_status();
    if (STATUS_CHECKING_LOCAL == status) 
    {
    	int ret;
    	UserInfo userinfo;
    	userinfo.username = "guest";
    	userinfo.password = "guest";
		ret = u2l_localmode_loginauth(&userinfo);
		if (ret == 0) {
			return SUCCESS;
		} else {
		    return ERROR_PROCESSING;
		}
    }
    
    if(status != STATUS_WAITING_LOGIN)
    {
        LOG_ERR("error! state:%d, event:%s!", status, EVENT_TO_STR(U2L_GUEST_LOGIN));
        return ERROR_EVENT;
    }

    if(_vm->vm_check_local_base_exist() == 1)
    {
        image_exist = true;
    }

    /*
        special->multiuser: old image would be removed first, then new image downloaded before guest login
        multiuser->special: old image not removed. if unbound, guest login is forbidden, otherwise, new image would be downloaded
       */
    if(_mode_data.get().mode==SPECIAL_MODE && _mode_data.get().bind_user.username.empty())
    {   
        //if special mode unbound, any user can not login, return to login UI
        LOG_INFO("special mode unbound, return to login UI, call l2u_show_userlogin(6)");
        l2u_result_user(6);    // 6 indicate unbound
    }
    else if(image_exist == false)
    {
        //if special mode unbound, there is no image, return to login UI
        LOG_INFO("no image, return to login UI, call l2u_show_userlogin(2)");
        l2u_result_user(2);    // 2 indicate no image
    }
    else
    {
        //guest, login success
        LOG_INFO("guest mode, login success, _mode_data.get().bind_user.username=%s", _mode_data.get().bind_user.username.c_str());
        _logined_user_info.username = "guest";
        _logined_user_info.password = gloox::password_codec_xor("guest", true);
        _logined_user_info.remember_flag = 0;

        //guest loginning with public policy.
        _logined_user_info.policy_info = get_public_policy();
        _logined = true;

        _userinfomgr.setUserLogined(_logined);
        _userinfomgr.saveLoginedUser(_logined_user_info.username);
        _status_machine.change_status(EVENT_LOGIN_BUTTON_ENTERED);    
    }

    return SUCCESS;

}

int Application::u2l_cancel_connection(LogicEvent* event)
{
    int status;

    LOG_DEBUG("%s enter", __func__);
    status = _status_machine.get_status();
    if(status != STATUS_INITING)
    {
       LOG_ERR("error! state:%d, event:%s!", status, EVENT_TO_STR(U2L_CANCEL_CONNECTION));
       return ERROR_EVENT;
    }

    //show disconnection UI
    l2u_show_disconnect_wrap();

    //destroy connection with web
    _mina->mina_destroy_connection();

    return SUCCESS;
}

int Application::u2l_establish_connection(LogicEvent* event)
{
    int status;

    LOG_DEBUG("%s enter", __func__);
    status = _status_machine.get_status();
    if(status != STATUS_INITING)
    {
       LOG_ERR("error! state:%d, event:%s!", status, EVENT_TO_STR(U2L_ESTABLISH_CONNECTION));
       return ERROR_EVENT;
    }

    //show connecting UI
    l2u_show_connecting_wrap();

    //establish connection with web
    _mina->mina_establish_connection();

    return SUCCESS;
}


int Application::u2l_public_login(LogicEvent* event)
{
    int status;

    LOG_DEBUG("%s enter", __func__);
    status = _status_machine.get_status();
    
    if (STATUS_CHECKING_LOCAL == status) 
    {
    	int ret;
    	UserInfo userinfo;
    	userinfo.username = "public";
    	userinfo.password = "public";
		ret = u2l_localmode_loginauth(&userinfo);
		if (ret == 0) {
			return SUCCESS;
		} else {
		    return ERROR_PROCESSING;
		}
    }
    
    if(status != STATUS_WAITING_LOGIN)
    {
       LOG_ERR("error! state:%d, event:%s!", status, EVENT_TO_STR(U2L_PUBLIC_LOGIN));
       return ERROR_EVENT;
    }

    _logined_user_info.username = "public";
    _logined_user_info.password = gloox::password_codec_xor("public", true);
    _logined_user_info.remember_flag = 0;

    //public loginning with public policy.
    _logined_user_info.policy_info = get_public_policy();
    _logined = true;

    _userinfomgr.setUserLogined(_logined);
    _userinfomgr.saveLoginedUser(_logined_user_info.username);

    _status_machine.change_status(EVENT_LOGIN_BUTTON_ENTERED);
    return SUCCESS;
}


int Application::u2l_modify_passwd(LogicEvent* event)
{
    int status;
    
    LOG_DEBUG("%s enter", __func__);
    UserInfo *userinfo = (UserInfo*)event->_data;
    status = _status_machine.get_status();
    if (status != STATUS_INITING && status != STATUS_WAITING_LOGIN) {
       LOG_ERR("error! state:%d, event:%s!", status, EVENT_TO_STR(U2L_MODIFY_PASSWORD));
       delete userinfo;
       return ERROR_EVENT;
    }

    userinfo->password = gloox::password_codec_xor(userinfo->password, true);
    userinfo->new_password = gloox::password_codec_xor(userinfo->new_password, true);
    LOG_INFO("%s: username:%s, passwd:%s, new passwd:%s", __func__,
        userinfo->username.c_str(), userinfo->password.c_str(), userinfo->new_password.c_str());
    _mina->mina_web_modify_password(*userinfo);
    _input_user_info = *userinfo;
    delete userinfo;
    return SUCCESS; 
}    

int Application::u2l_enter_local_mode(LogicEvent* event)
{
    //destroy connection with web
    _mina->mina_destroy_connection();
    _status_machine.change_status(EVENT_LOCAL_MODE_CHOOSEN);
    return SUCCESS;
}

void Application::confirm_usb_copy_base(void* args) {
    LOG_INFO("enter confirm_usb_copy_base");
    Application* app = (Application*)args;
    int ret = app->get_vm()->vm_start_copy_usb_image();
    if (ret == -5) {
        l2u_show_dialog_copy_base_fail_no_space(NULL, NULL);
    } else if (ret == 1) {
        l2u_show_dialog_copy_base_fail_no_dev(NULL, NULL);
    } else if (ret == 2) {
        l2u_show_dialog_copy_base_fail(NULL, NULL);
    } else if (ret != 0) {
        LOG_ERR("unknown copy base err! ret = %d", ret);
        l2u_show_dialog_copy_base_fail(NULL, NULL);
    }
}

int Application::u2l_usb_copy_base(LogicEvent* event)
{
    LOG_INFO("call l2u_show_dialog_copy_base");
    l2u_show_dialog_copy_base((void*)confirm_usb_copy_base, this, NULL, NULL);
    return SUCCESS;
}

int Application::u2l_save_power_boot(LogicEvent* event)
{
    int* power_boot;
    int ret = 0;
    LOG_DEBUG("%s enter", __func__);
    
    power_boot = static_cast<int*>(event->_data);
    ASSERT(power_boot);
    if(event->_error != SUCCESS)
    {
        delete power_boot;
        return event->_error;
    }

    if (*power_boot != _power_boot) {
        ret = set_power_boot(*power_boot);
    } else {
        LOG_INFO("no need set power boot");
    }

    if(ret != 0)
    {    
        delete power_boot;
        return -1;
    } else {        
        _power_boot = *power_boot;
        delete power_boot;
        return SUCCESS;
    }
}

int Application::u2l_enter_settings(LogicEvent* event)
{
    int status;

    LOG_DEBUG("%s enter", __func__);
    status = _status_machine.get_status();
    if (status == STATUS_NONE || status == STATUS_DOWNLOADING_IMAGE || status == STATUS_RUNNING_VM) {
        return SUCCESS;
    }

    _ui_locking = true;
    LOG_DEBUG("u2l_enter_settings, status=%d, _ui_locking!", status);

    if(status == STATUS_INITING)
    {
        _mina->mina_destroy_connection();
    }
    
    return SUCCESS;
}

int Application::u2l_enter_wifi_config(LogicEvent* event)
{
    int status;
    
    status = _status_machine.get_status();
    if (status == STATUS_NONE || status == STATUS_DOWNLOADING_IMAGE || status == STATUS_RUNNING_VM) {
        return SUCCESS;
    }

    _ui_locking = true;
    LOG_DEBUG("u2l_enter_wifi_config, status=%d, _ui_locking!", status);

    if(status == STATUS_INITING)
    {
        _mina->mina_destroy_connection();
    }
    
    return SUCCESS;
}

int Application::u2l_enter_auth_config(LogicEvent* event)
{
    int status;
    
    status = _status_machine.get_status();
    _ui_locking = true;
    LOG_DEBUG("u2l_enter_auth_config, status=%d, _ui_locking!", status);
    if(status == STATUS_INITING)
    {
        _mina->mina_destroy_connection();
    }
    
    return SUCCESS;
}

void Application::l2u_show_setting_tip(int result)
{
    switch (result) {
    case UI_SETTING_OK:
        l2u_result_config(0);
        break;        
    case UI_SETTING_FAIL:
        l2u_result_config(1);
        break;        
    case UI_SETTING_FAIL_REBOOT:
        l2u_result_config(2);
        break;
    case UI_SETTING_WLAN_SAVE_IP:
        l2u_result_config(3);
        break;
    case UI_SETTING_AUTH_FAIL:
        l2u_result_config(4);          
        break;        
    case UI_SETTING_AUTH_TIMEOUT:
        l2u_result_config(5);
        break;        
    default:
        //unknown setting tip, save fail
        l2u_result_config(1);
        break;
    }
}

void Application::l2u_hotplug_show_public_login()
{
    LOG_INFO("enter l2u_hotplug_show_public_login");
    int status;
    status = _status_machine.get_status();
    if (!_ui_locking) {
        if (status == STATUS_WAITING_LOGIN) {
            if (is_dev_network_up()) {
                if(_mode_data.get().mode == PUBLIC_MODE) {
                    LOG_DEBUG("call l2u_show_publiclogin");
                    l2u_show_publiclogin();
                }
            }
        }
    }
}

int Application::l2u_notify_setting_result(LogicEvent* event)
{
    LOG_INFO("enter l2u_notify_setting_result, cancel _ui_locking");
    int status;
    status = _status_machine.get_status();

    //if settings modify, clear _error_tips
    _error_tips = false;
    _ui_locking = false;
    LOG_DEBUG("l2u_notify_setting_result, _ui_need_sync_flag=%d, _ui_has_sync_flag=%d, _ui_setting_error=%d", _ui_need_sync_flag, _ui_has_sync_flag, _ui_setting_error);

    //set success or set nothing
    if(_ui_setting_error == 0)
    {
        if(_ui_need_sync_flag !=0)
        {
            //has set something, show success UI
            l2u_show_setting_tip(SUCCESS);
            sleep(2);
        }
        switch(status)
        {
            case STATUS_INITING:
                if(_mina->mina_get_server_ip().empty())
                {
                    //no server ip, show unconfig UI
                    if (_newdeploy_manage->is_new_terminal()) {
                        _newdeploy_manage->show_newdeploy_disconnect();
                    } else {
                        l2u_show_unconfig_wrap();
                    }
                }
                else
                {
                    if(is_dev_network_up())
                    {
                        logic_reset_status_machine();
                    }
                    else
                    {
                        if(_newdeploy_manage->is_new_terminal()) {
                            l2u_show_newdeploy_connect(0);
                        } else {
                        	LOG_INFO("call l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT)44444");
                            l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT);
                            _newdeploy_manage->reset_status();
                        }
                    }
                }
                break;

            case STATUS_PREPARING_IMAGE:
                if(is_dev_network_up())
                {
                    logic_reset_status_machine();
                }
                else
                {
                    if(_newdeploy_manage->is_new_terminal()) {
                        l2u_show_newdeploy_connect(0);
                    } else {
                    	LOG_INFO("call l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT)55555");
                        l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT);
                        _newdeploy_manage->reset_status();
                    }
                }
                break;
                
            case STATUS_WAITING_LOGIN:
                if(is_dev_network_up())
                {
                    //if server ip/terminal ip changed, reconnect
                    if((_ui_has_sync_flag&(UI_SERVER_IP_SYNC|UI_NET_INFO_SYNC))!=0)
                    {
                        logic_reset_status_machine();
                    }
                    else
                    {
                        if(_mode_data.get().mode == PUBLIC_MODE)
                        {
                            LOG_DEBUG("call l2u_show_publiclogin");
                            l2u_show_publiclogin();
                        }
                        else
                        {
                            LOG_DEBUG("call l2u_show_userlogin");
                            ui_show_login();
                        }
                    }
                }
                else
                {
                	LOG_INFO("call l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT)66666");
                    l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT);
                }
                break;       
                
            case STATUS_CHECKING_LOCAL:
            	if (localmode) {
            		LOG_INFO("close setting about poweroff bottun");
            		localmode->LM_LocalMode_UpdateUI();
            	}
                break;

            case STATUS_NEW_DEPLOY:
                //always retry to establish connection
                _newdeploy_manage->quit_new_deploy_abnormal();
                break;

            default:
                LOG_ERR("error status %d!", status);      
        }

        //save or cancel setting done, unlock ui 

    } else {
        l2u_show_setting_tip(_ui_setting_tip);  
        sleep(2);
        _status_machine.reset_status();
        _status_machine.change_status(EVENT_INIT);
        //show setting UI
        //l2u_show_config();
    }
    
    #if 0 
    else if( _ui_setting_error == UI_AUTH_ERR) {
          //set auth error
        l2u_result_config(4);
        sleep(2);
        logic_reset_status_machine(); 
    } else {
        //set error
        l2u_result_config(1);
        sleep(2);
        logic_reset_status_machine(); 
        //show setting UI
        //l2u_show_config();
    }
    #endif 
    ui_clean_all_set_flag();
    return SUCCESS;
}
int Application::l2u_notify_wifi_config_result(LogicEvent* event)
{
    LOG_INFO("enter l2u_notify_wifi_config_result, cancel _ui_locking");
    _ui_locking = false;
    
    int status;
    status = _status_machine.get_status();
    
    switch(status)
    {
        case STATUS_INITING:
            if(_mina->mina_get_server_ip().empty())
            {
                //no server ip, show unconfig UI
                if (_newdeploy_manage->is_new_terminal()) {
                    _newdeploy_manage->show_newdeploy_disconnect();
                } else {
                    l2u_show_unconfig_wrap();
                }
            }
            else
            {
                if(is_dev_network_up())
                {
                    logic_reset_status_machine(); 
                }
                else
                {
                    if(_newdeploy_manage->is_new_terminal()) {
                        l2u_show_newdeploy_connect(0);
                    } else {
                    	LOG_INFO("call l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT)77777");
                        l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT);
                        _newdeploy_manage->reset_status();
                    }
                }
            }
            break;

        case STATUS_PREPARING_IMAGE:
            if(is_dev_network_up())
            {
                logic_reset_status_machine(); 
            }
            else
            {
                if(_newdeploy_manage->is_new_terminal()) {
                    l2u_show_newdeploy_connect(0);
                } else {
                	LOG_INFO("call l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT)88888");
                    l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT);
                    _newdeploy_manage->reset_status();
                }
            }
            break;
            
        case STATUS_WAITING_LOGIN:
            if(is_dev_network_up())
            {
                //if server ip/terminal ip changed, reconnect
                if((_ui_has_sync_flag&(UI_SERVER_IP_SYNC|UI_NET_INFO_SYNC))!=0)
                {
                    logic_reset_status_machine(); 
                }
                else
                {
                    if(_mode_data.get().mode == PUBLIC_MODE)
                    {
                        LOG_DEBUG("call l2u_show_publiclogin");
                        l2u_show_publiclogin();
                    }
                    else
                    {
                        LOG_DEBUG("call l2u_show_userlogin");
                        ui_show_login();
                    }
                }
            }
            else
            {
            	LOG_INFO("call l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT)99999");
                l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT);
            }
            break;       
            
        case STATUS_CHECKING_LOCAL:
        	if (localmode) {
        		LOG_INFO("close setting about poweroff bottun");
        		localmode->LM_LocalMode_UpdateUI();
        	}
            break;

        case STATUS_NEW_DEPLOY:
            //always retry to establish connection
            _newdeploy_manage->quit_new_deploy_abnormal();
            break;

        default:
            LOG_ERR("error status %d!", status);      
    }
    return SUCCESS;
}

int Application::l2u_notify_auth_config_result(LogicEvent* event)
{
    LOG_INFO("enter l2u_notify_auth_config_result, cancel _ui_locking");

    int status;

    _ui_locking = false;
    status = _status_machine.get_status();
   
    switch(status)
    {
        case STATUS_INITING:
            if(_mina->mina_get_server_ip().empty())
            {
                //no server ip, show unconfig UI
                if (_newdeploy_manage->is_new_terminal()) {
                    _newdeploy_manage->show_newdeploy_disconnect();
                } else {
                    l2u_show_unconfig_wrap();
                }
            }
            else
            {
                if(is_dev_network_up())
                {
                    logic_reset_status_machine(); 
                }
                else
                {
                    if(_newdeploy_manage->is_new_terminal()) {
                        l2u_show_newdeploy_connect(0);
                    } else {
                    	LOG_INFO("call l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT)1010101010");
                        l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT);
                        _newdeploy_manage->reset_status();
                    }
                }
            }
            break;

        case STATUS_PREPARING_IMAGE:
            if(is_dev_network_up())
            {
                logic_reset_status_machine(); 
            }
            else
            {
                if(_newdeploy_manage->is_new_terminal()) {
                    l2u_show_newdeploy_connect(0);
                } else {
                	LOG_INFO("call l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT)1111111111");
                    l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT);
                    _newdeploy_manage->reset_status();
                }
            }
            break;
            
        case STATUS_WAITING_LOGIN:
            if(is_dev_network_up())
            {
                //if server ip/terminal ip changed, reconnect
                if((_ui_has_sync_flag&(UI_SERVER_IP_SYNC|UI_NET_INFO_SYNC))!=0)
                {
                    logic_reset_status_machine(); 
                }
                else
                {
                    if(_mode_data.get().mode == PUBLIC_MODE)
                    {
                        LOG_DEBUG("call l2u_show_publiclogin");
                        l2u_show_publiclogin();
                    }
                    else
                    {
                        LOG_DEBUG("call l2u_show_userlogin");
                        ui_show_login();
                    }
                }
            }
            else
            {
	            LOG_INFO("call l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT)1212121212");
                l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT);
            }
            break;
            
        case STATUS_CHECKING_LOCAL:
        	if (localmode) {
        		LOG_INFO("close setting about poweroff bottun");
        		localmode->LM_LocalMode_UpdateUI();
        	}
            break;

        case STATUS_NEW_DEPLOY:
            //always retry to establish connection
            _newdeploy_manage->quit_new_deploy_abnormal();
            break;

        default:
            LOG_ERR("error status %d!", status);      
    }
    return SUCCESS;
}

int Application::u2l_upgrade_for_class(LogicEvent* event)
{
    UpgradeInfo info = get_upgrade_server_info();
    upgrade_class_software_version(info);
    return 0;
}

int Application::w2l_ready(LogicEvent* event)
{
    NetworkInfo vmnet_info = _vm->vm_get_vm_netinfo();

    // TODO:we only send one parament to application, so -1 means it is not new terminal
    // TODO: if there be more parament , we should define a datastruct

    int* web_default_mode = static_cast<int*>(event->_data);
    ASSERT(web_default_mode);
    if(event->_error != SUCCESS)
    {
        delete web_default_mode;
        return event->_error;
    }

    //quit new deploy if this is a not a new terminal for server 
    if (_status_machine.get_status() == STATUS_NEW_DEPLOY && *web_default_mode == -1 && _newdeploy_manage->get_download_status() == false)
    {
        _newdeploy_manage->quit_new_deploy();
        return SUCCESS;
    }

    LOG_INFO("server ip: %s, last_server_ip: %s, dev_status: %d", _mina->mina_get_server_ip().c_str(), _mina->mina_get_last_server_ip().c_str(), _dev_status.IsLocked());
    if (*web_default_mode != -1 || _mina->mina_get_server_ip() != _mina->mina_get_last_server_ip()) {
        // if layer disk exsit then this base layer is on, we should first judget layerdisk then judget img(layer is off)
        if (_vm->vm_check_layerdisk_exist() || _vm->vm_check_personal_img_exist() || _vm->vm_check_usedisk_exist()) {
            LOG_INFO("dev locked!!!");
            _dev_status.DevLock(true);
        }
    }

    //acknowlege last setting to new server
    if(_configure_serverip_sync_flag)
    {
        if(!_configure_hostname.empty())
        {
            _mina->mina_l2w_change_hostname(_configure_hostname);
            sleep(2);
        }
        
        if(_configure_mode != -1)
        {
            _mina->mina_l2w_change_mode((Mode)_configure_mode);
            /*
                    *FIXME: we assert web has set the database in 5s
                    */
            sleep(5);
        }
    }
    
    _mina->mina_web_upload_local_network_info(_local_network_data.get_ip_info());
    _mina->mina_web_upload_vm_network_info(vmnet_info, _vm->get_vm_mac());
    sync_next_event(W2L_MINA_READY);

    /*
     *default_mode != -1 means that this is a new terminal for this server
     * _configure_mode != -1 means that easydeploy or ui set mode last time
     * now easydeploy or ui are first piror
     * because web's instruction is non-specific for us
     */
    if(*web_default_mode != -1 && _configure_mode != -1)
    {
        _default_mode = _configure_mode;
    }
    else
    {
        _default_mode = *web_default_mode;
    }


    _configure_hostname.clear();
    _configure_mode = -1;
    _configure_serverip_sync_flag = false;

    delete web_default_mode;
    return SUCCESS;
}

void Application::sync_next_event(EventType current_event)
{
#define PRODUCT_CT5330M                 0x80061204
    //begin with W2L_MINA_READY
    static std::list<EventType> sync_list;
    std::list<EventType>::iterator it;
    std::list<EventType>::iterator it_next;
    static bool inited = false;

    //init
    if(!inited)
    {
        sync_list.clear();
        sync_list.push_back(W2L_MINA_READY);
        sync_list.push_back(W2L_SYNC_IPXE);
        sync_list.push_back(W2L_SYNC_SOFTWARE_VERSION);
        sync_list.push_back(W2L_SYNC_TERMINAL_PSW);
        sync_list.push_back(W2L_SYNC_RESET_TERMINAL);
        sync_list.push_back(W2L_SYNC_MAIN_WINDOW);
        sync_list.push_back(W2L_SYNC_RECOVER_IMAGE);
        sync_list.push_back(W2L_SYNC_SERVER_IP);
        sync_list.push_back(W2L_SYNC_ALL_USERINFO);
        sync_list.push_back(W2L_SYNC_HOSTNAME);
        sync_list.push_back(W2L_SYNC_SERVER_TIME);
        sync_list.push_back(W2L_SYNC_MODE);
        sync_list.push_back(W2L_SYNC_PRINTER_SWITCH);
        sync_list.push_back(W2L_SYNC_DESKTOP_REDIR);
        sync_list.push_back(W2L_SYNC_DEV_POLICY);

        sync_list.push_back(W2L_SYNC_RCDUSBCONF_INFO);
        sync_list.push_back(L2L_DATA_PROTECT);
        sync_list.push_back(W2L_SYNC_PUBLIC_POLICY);
        //sync_list.push_back(W2L_SYNC_RELOAD_IMAGE);
        sync_list.push_back(W2L_SYNC_PORT_MAPPING);
        sync_list.push_back(W2L_SYNC_SSID_WHITELIST);
        sync_list.push_back(W2L_GET_VM_DEV_INTERFACE_INFO);
        sync_list.push_back(W2L_SYNC_DELETE_TEACHERDISK);
        sync_list.push_back(W2L_SYNC_DRIVER_INSTALL);
        inited = true;
    }

    for(it = sync_list.begin(); it != sync_list.end(); it++)
    {
        //get the current iterator
        if(current_event == *it)
        {
            //get next iterator
            it_next = it;
            it_next++;

            //last element complete sync info
            if(it_next == sync_list.end())
            {
                _init_sync_processing = false;
            	ModeInfo modeinfo = _mode_data.get();
            	LOG_INFO("!!! mode %d, name %s", modeinfo.mode, modeinfo.bind_user.username.c_str());
                
                //not allow back to settype if newdeploy and download interrupted
                if (_status_machine.get_status() == STATUS_NEW_DEPLOY && _newdeploy_manage->get_download_status() == true) {
                    if(modeinfo.mode==SPECIAL_MODE && modeinfo.bind_user.username.empty()) {
                        //we do not consider this if special mode unbound
                    } else {
                        _allow_newdeploy = false;
                    }
                }
                
                if(is_installing_driver())
                {
                    _status_machine.change_status(EVENT_DRIVER_INSTALL_RECEIVED);
                }

                else if(_default_mode != -1 && !_dev_status.IsLocked()  && _allow_newdeploy)
                {
                    modeinfo.mode = Mode(_default_mode);
                    if(modeinfo.mode != SPECIAL_MODE)
                    {
                        modeinfo.bind_user.username.clear();
                    }
                    _mode_data.set(modeinfo);
                    //_status_machine.change_status(EVENT_ENTER_NEW_DEPLOY);
                    if(_status_machine.get_status() == STATUS_NEW_DEPLOY)
                    {
                        _newdeploy_manage->enter_set_type();
                    }
                    else
                    {
                        _newdeploy_manage->set_new_terminal(true);
                        _newdeploy_manage->quit_new_deploy_abnormal();
                    }
                }

                else
                {
                	if (_dev_status.IsLocked()) {
                        LOG_INFO("show dev lock, mode = %d, lock_type = %d", modeinfo.mode, _dev_status.get_lock_type());
                        ASSERT(_dev_status.get_lock_type() != 0);
                        if (!_ui_locking) {
                		    l2u_show_dev_lock(modeinfo.mode, _dev_status.get_lock_type());
                        }
                	} else {
                        if (_status_machine.get_status() == STATUS_NEW_DEPLOY && _newdeploy_manage->get_download_status() == true) {
                            _mina->mina_web_request_image(_vm->vm_get_vm_imageinfo());
                        } else {
                		    _status_machine.change_status(EVENT_WEB_INFO_SYNCHRONIZED);
                        }
                	}
                }
                _default_mode = -1;
                _allow_newdeploy = true;
            }
            else
            {
                /*
                 *here we sync info in order
                 *we can only "switch case", for every web api has no property for us to use a table to correspond
                 *but at least we can control sync info order only here
                 */
                _init_sync_processing = true;
                switch(*it_next)
                {
                    case W2L_SYNC_SOFTWARE_VERSION:
                    {
                        _mina->mina_web_request_software_version();
                        break;
                    }
                    case W2L_SYNC_TERMINAL_PSW:
                    {
                        _mina->mina_web_request_terminal_password();
                        break;
                    }
                    case W2L_SYNC_IPXE:
                    {
                        _mina->mina_web_request_ipxe();
                        break;
                    }
                    case W2L_SYNC_RESET_TERMINAL:
                    {
                        _mina->mina_web_request_reset_terminal();
                        break;
                    }
                    case W2L_SYNC_RECOVER_IMAGE:
                    {
                        _mina->mina_web_request_recover_image();
                        break;
                    }
                    case W2L_SYNC_SERVER_IP:
                    {
                        if (_mina->mina_get_server_ip() == _mina->mina_get_last_server_ip())
                        {
                            LOG_INFO("no need to check server ip");
                            if (!_dev_status.IsLocked())
                            {
                                _mina->mina_set_last_server_ip(_mina->mina_get_server_ip());
                            }
                            current_event = *it_next;//ignore this sync
                            continue;
                        }
                        else
                        {
                            _mina->mina_web_request_check_server_ip(_mina->mina_get_server_ip());
                        }
                        break;
                    }
                    case W2L_SYNC_ALL_USERINFO:
                    {
                        //here we get should go to different order by ret of getAllUsers
                        //if databse is empty we do not sync username
                        int ret = 0;
                        vector<string> users;
                        ret = _userinfomgr.getAllUsers(users);
                        if (ret == 0 && !_dev_status.IsLocked())
                        {
                            LOG_INFO("Request web all Users info ...");
                            _mina->mina_web_request_all_userinfo(users);
                        }
                        else
                        {
                            LOG_WARNING("Failed to get all users: %s", _userinfomgr.usrmgr_strerror(ret).c_str());
                            current_event = *it_next;//ignore this sync
                            continue;
                        }
                        break;
                    }
                    case W2L_SYNC_HOSTNAME:
                    {
                        _mina->mina_web_request_hostname(_hostname_data.get_hostname_info().c_str());
                        break;
                    }
                    case W2L_SYNC_SERVER_TIME:
                    {
                        _mina->mina_web_request_server_time();
						break;
                    }
                    case W2L_SYNC_MODE:
                    {
                        _mina->mina_web_request_mode(_mode_data.get());
                        break;
                    }
                    case W2L_SYNC_DEV_POLICY:
                    {
                        ModeInfo server_mode = (_dev_status.IsLocked() ? _dev_status.get_mode_info() : _mode_data.get());
                        LOG_INFO("Request web dev policy, mode = %d, bind_user = %s", server_mode.mode, server_mode.bind_user.username.c_str());
                        _mina->mina_web_request_dev_policy(server_mode.mode, server_mode.bind_user.username);                        
                        _dev_policy.wait4update(2*1000*1000*1000);
                        break;
                    }
                    case W2L_GET_VM_DEV_INTERFACE_INFO:
                    {
                        string product_id = _basic_data.get().product_id;
                        //product_id = "0x80061204";
                        unsigned long check_product;
                        check_product = std::stoul(product_id, NULL, 16);
                        if (check_product == PRODUCT_CT5330M)
                        {
                            _mina->mina_web_request_dev_interface_info(product_id);
                        }
                        else
                        {
                            LOG_WARNING("Failed to dev interface info");
                            current_event = *it_next;//ignore this sync
                            continue;
                        }
                        break; 
                    }
                    case W2L_SYNC_RCDUSBCONF_INFO:
                        _mina->mina_web_request_rcdusbconf();
                        break;
                    case L2L_DATA_PROTECT:
                    {
                        LogicEvent* event = new LogicEvent(this, L2L_DATA_PROTECT, NULL, 0);
                        _process_loop.push_event(event);
                        event->unref();
                        current_event = *it_next;
                        continue;
                    }
                    case W2L_SYNC_PUBLIC_POLICY:
                    {
                        _mina->mina_web_request_public_policy();
                        break;
                    }
                    /*case W2L_SYNC_RELOAD_IMAGE:
                    {
                        _mina->mina_web_request_reload_image();
                        break;
                    }*/
                    case W2L_SYNC_DELETE_TEACHERDISK:
                    {
                        _mina->mina_web_request_delete_teacherdisk();
                        break;
                    }
                    case W2L_SYNC_DRIVER_INSTALL:
                    {
                        if (is_dev_wlan_up()) {
                            LOG_INFO("ignore sync driver install if wlan up!\n");
                            current_event = *it_next;//ignore this sync
                            continue;
                        } else {
                            _mina->mina_web_request_driver_install();
                        }
                        break;
                    }
                    case W2L_SYNC_PORT_MAPPING:
                    {
                        _mina->mina_web_request_port_mapping();
                        break;
                    }
                    case W2L_SYNC_SSID_WHITELIST:
                    { 
                        LOG_DEBUG("W2L_SYNC_SSID_WHITELIST status %d", _local_network_data.get_net_status());
                        _mina->mina_web_request_ssid_whitelist();
                        break;
                    }
                    case W2L_SYNC_PRINTER_SWITCH:
                    {
                        LOG_DEBUG("W2L_SYNC_PRINTER_SWITCH");
                        _mina->mina_web_request_printer_switch();
                        break;
                    }
                    case W2L_SYNC_DESKTOP_REDIR:
                    {
                        _mina->mina_web_request_desktop_redir();
                        break;
                    }
                    case W2L_SYNC_MAIN_WINDOW:
                    {
                        _mina->mina_web_request_main_window();
                        break;
                    }
                    default:
                    {
                        //you must forget adding a case, check the code
                        ASSERT(0);
                        break;
                    }
                }
            }
        }
    }
    return;
}

bool Application::__upgrade_check_product_id(const VersionInfo& sw_ver, const string& product_id)
{
/* RCO PRODUCT ID */
#define PRODUCT_RAIN300W                0x80060022
#define PRODUCT_RAIN310W                0x80060014
#define PRODUCT_RAIN400W                0x80060041
#define PRODUCT_RAIN410W                0x80060033
#define PRODUCT_RAIN300W_V2             0x80060023
#define PRODUCT_RAIN400W_V2             0x80060042
#define PRODUCT_RAIN320W_V1_10          0x80060052
#define PRODUCT_RAIN310W_V2             0x80060017
#define PRODUCT_RAIN320W_V1_00          0x80060051
#define PRODUCT_RAIN305W                0x80060071
#define PRODUCT_RAIN405W                0x80060081
#define PRODUCT_RAIN310W_500HD          0x80060016
#define PRODUCT_RAIN310_256             0x80060019
#define PRODUCT_RAIN410_256             0x80060036
#define PRODUCT_RAIN320_256             0x80060054
#define PRODUCT_RAIN305_256             0x80060072
#define PRODUCT_RAIN405_256             0x80060082
#define PRODUCT_RAIN310_256_V2_00       0x80060091
#define PRODUCT_RAIN310W_500HD_V2_00    0x80060092
#define PRODUCT_RAIN310W_V2_V2_00       0x80060093
#define PRODUCT_RAIN410_256_V2_00       0x80060101
#define PRODUCT_RAIN320_256_V2_00       0x80060111
/* COMMON TERMINAL */
#define PRODUCT_CT5200                  0x80061100
#define PRODUCT_CT5200S                 0x80061101
#define PRODUCT_CT5300                  0x80061200
#define PRODUCT_CT5330S                 0x80061201
#define PRODUCT_CT5330M                 0x80061204
#define PRODUCT_CT5320S                 0x80061202
#define PRODUCT_CT5530S                 0x80061300
#define PRODUCT_CT6200                  0x80061600
#define PRODUCT_CT6300                  0x80061700
#define PRODUCT_CT6300S                 0x80061701
/* CAV TERMINAL */
#define PRODUCT_RG_SC300_256            0x90006001
#define PRODUCT_RG_SC500_256            0x90006002
#define PRODUCT_RG_OPS_C_I5V2           0x90007002
/* added INTEL 3165 */
#define PRODUCT_RG_OPS_C_I5V4           0x90007004

#define PRODUCT_RG_IIB_N86AV            0x90007005

    VersionInfo supportVER;
    unsigned long check_product;

    check_product = std::stoul(product_id, NULL, 16);
    switch (check_product) {
        case PRODUCT_RAIN305W:
            supportVER.set_version(2, 0, 51);
            break;
        case PRODUCT_RAIN405W:
        case PRODUCT_RAIN310W_500HD:
            supportVER.set_version(3, 0, 0);
            break;
        case PRODUCT_RAIN310_256:
        case PRODUCT_RAIN410_256:
        case PRODUCT_RAIN320_256:
        case PRODUCT_RAIN305_256:
        case PRODUCT_RAIN405_256:
            supportVER.set_version(3, 0, 88);
            break;
        case PRODUCT_RAIN310_256_V2_00:
        case PRODUCT_RAIN310W_500HD_V2_00:
        case PRODUCT_RAIN310W_V2_V2_00:
        case PRODUCT_RAIN410_256_V2_00:
        case PRODUCT_RAIN320_256_V2_00:
            supportVER.set_version(3, 0, 102);
            break;
        case PRODUCT_CT5200:
        case PRODUCT_CT5300:
        case PRODUCT_CT6200:
        case PRODUCT_CT6300:
            supportVER.set_version(4, 0, 0);
            break;
        case PRODUCT_CT5330S:
        case PRODUCT_RG_SC300_256:
        case PRODUCT_RG_SC500_256:
        case PRODUCT_RG_OPS_C_I5V2:
        case PRODUCT_RG_OPS_C_I5V4:
        case PRODUCT_RG_IIB_N86AV:
        case PRODUCT_CT5530S:
            supportVER.set_version(4, 0, 150);
            break;
        case PRODUCT_CT5320S:
            supportVER.set_version(4, 1, 250);
            break;
        case PRODUCT_CT5200S:
        case PRODUCT_CT6300S:
            supportVER.set_version(4, 1, 343);
            break;
        case PRODUCT_CT5330M:
            supportVER.set_version(4, 1, 520);
            break;
        default:
            supportVER.set_version(1, 0, 0);
            break;
    }
    if (supportVER > sw_ver)
        return false;
    return true;

#if 0
    VersionInfo minorVer;

    if (product_id == "0x80060071")
    {
        //80060071    Rain305W
        minorVer.set_version(2, 0, 51);
    }
    else if (product_id == "0x80060081")
    {
        //80060081    Rain405W
        minorVer.set_version(3, 0, 0);
    }
    else if (product_id == "0x80060016")
    {    
        //80060016    Rain310W (500HD)
        minorVer.set_version(3, 0, 0);
    }
    
    if (minorVer > sw_ver)
    {
        return false;
    }
    return true;
#endif
}

bool Application::__upgrade_rcc_check_product_id(const VersionInfo& sw_ver, const string& product_id)
{
/* RCC PRODUCT ID */
#define PRODUCT_RAIN310E_128            0x80060013
#define PRODUCT_RAIN310E_256            0x80060012
#define PRODUCT_RAIN310E_500            0x80060015
#define PRODUCT_RAIN410E_128            0x80060032
#define PRODUCT_RAIN410E_256            0x80060031
#define PRODUCT_RAIN410E_500            0x80060034
#define PRODUCT_RAIN310E_500_V2_00      0x80060094
#define PRODUCT_RAIN410E_500_V2_00      0x80060102
#define PRODUCT_RAIN320T                0x80060053
#define PRODUCT_RAIN320T_V2_00          0x80060112
#define PRODUCT_RAIN305E                0x80060073

    VersionInfo supportVER;
    unsigned long check_product;

    check_product = std::stoul(product_id, NULL, 16);
    switch (check_product) {
        case PRODUCT_RAIN310E_128:
        case PRODUCT_RAIN310E_256:
        case PRODUCT_RAIN410E_128:
        case PRODUCT_RAIN410E_256:
        case PRODUCT_RAIN310E_500:
        case PRODUCT_RAIN410E_500:
        case PRODUCT_RAIN320T:
        case PRODUCT_RAIN310E_500_V2_00:
        case PRODUCT_RAIN410E_500_V2_00:
        case PRODUCT_RAIN320T_V2_00:
        case PRODUCT_RAIN305E:
        case PRODUCT_CT5200:
        case PRODUCT_CT5300:
        case PRODUCT_CT6200:
        case PRODUCT_CT6300:
        case PRODUCT_RG_SC300_256:
        case PRODUCT_RG_SC500_256:
        case PRODUCT_RG_OPS_C_I5V2:
        case PRODUCT_RG_OPS_C_I5V4:
        case PRODUCT_RG_IIB_N86AV:
            if (sw_ver.main_version < 4) {
                supportVER.set_version(3, 7, 50);
            } else if (sw_ver.main_version == 4) {
                supportVER.set_version(4, 1, 59);
            }
            break;
        /* 5200s ,5320s, 6300s */
        case PRODUCT_CT5200S:
        case PRODUCT_CT5320S:
        case PRODUCT_CT6300S:
            if (sw_ver.main_version < 4) {
                supportVER.set_version(3, 11, 0);
            } else if (sw_ver.main_version == 4) {
                supportVER.set_version(4, 3, 0);
            }
            break;

        case PRODUCT_CT5330S:
        case PRODUCT_CT5530S:
            if (sw_ver.main_version < 4) {
                supportVER.set_version(3, 8, 28);
            } else if (sw_ver.main_version == 4) {
                supportVER.set_version(4, 1, 113);
            }
            break;
        default:
            if (sw_ver.main_version < 4) {
                supportVER.set_version(3, 7, 50);
            } else if (sw_ver.main_version == 4) {
                supportVER.set_version(4, 1, 59);
            }
            break;
    }
    if (supportVER > sw_ver)
        return false;
    return true;
}

int Application::__upgrade_check_product_type(const string& product_id)
{
    unsigned long check_product;
    int client_type = 0;

    check_product = std::stoul(product_id, NULL, 16);
    switch (check_product) {
        case PRODUCT_RAIN300W:
        case PRODUCT_RAIN310W:
        case PRODUCT_RAIN400W:
        case PRODUCT_RAIN410W:
        case PRODUCT_RAIN300W_V2:
        case PRODUCT_RAIN400W_V2:
        case PRODUCT_RAIN320W_V1_10:
        case PRODUCT_RAIN310W_V2:
        case PRODUCT_RAIN320W_V1_00:
        case PRODUCT_RAIN305W:
        case PRODUCT_RAIN405W:
        case PRODUCT_RAIN310W_500HD:
        case PRODUCT_RAIN310_256:
        case PRODUCT_RAIN410_256:
        case PRODUCT_RAIN320_256:
        case PRODUCT_RAIN305_256:
        case PRODUCT_RAIN405_256:
        case PRODUCT_RAIN310_256_V2_00:
        case PRODUCT_RAIN310W_500HD_V2_00:
        case PRODUCT_RAIN310W_V2_V2_00:
        case PRODUCT_RAIN410_256_V2_00:
        case PRODUCT_RAIN320_256_V2_00:
            client_type = TYPE_OLD_RCO_CLIENT;
            break;
        case PRODUCT_RAIN310E_128:
        case PRODUCT_RAIN310E_256:
        case PRODUCT_RAIN410E_128:
        case PRODUCT_RAIN410E_256:
        case PRODUCT_RAIN310E_500:
        case PRODUCT_RAIN410E_500:
        case PRODUCT_RAIN320T:
        case PRODUCT_RAIN310E_500_V2_00:
        case PRODUCT_RAIN410E_500_V2_00:
        case PRODUCT_RAIN320T_V2_00:
        case PRODUCT_RAIN305E:
            client_type = TYPE_OLD_RCC_CLIENT;
            break;
        case PRODUCT_CT5200:
        case PRODUCT_CT5200S:
        case PRODUCT_CT5300:
        case PRODUCT_CT6200:
        case PRODUCT_CT6300:
        case PRODUCT_CT6300S:
        case PRODUCT_CT5330M:
        case PRODUCT_RG_SC300_256:
        case PRODUCT_RG_SC500_256:
        case PRODUCT_RG_OPS_C_I5V2:
        case PRODUCT_RG_OPS_C_I5V4:
        case PRODUCT_RG_IIB_N86AV:
        case PRODUCT_CT5330S:
        case PRODUCT_CT5320S:
        case PRODUCT_CT5530S:
            client_type = TYPE_NEW_CLIENT;
            break;
        default:
            client_type = TYPE_UNKNOW_CLIENT;
            break;
    }

   LOG_DEBUG("__upgrade_check_product_type:%d", client_type);

    return client_type;
}

bool Application::__upgrade_rcc_check_os(const VersionInfo& sw_ver, const int os_version)
{
    VersionInfo supportVER;

    if (os_version >= 150) {
        if (sw_ver.main_version < 4) {
            supportVER.set_version(3, 10, 1);
        } else if (sw_ver.main_version == 4) {
            supportVER.set_version(4, 3, 1);
        }
        if (supportVER > sw_ver) {
            return false;
        }
    }
    return true;
}

int Application::w2l_upgrade_for_office(LogicEvent* event)
{
    int net_type = -1;
    if (is_dev_eth_up()) {
        if (is_dev_wired_dot1x_up()) {
            net_type = 2;    //dot1x up
        } else {
            net_type = 0;    //eth up
        }
    } else if (is_dev_wlan_up()) {
        net_type = 1;    //wlan up
    }
    SystemHardwareInfo system_hard_info;
    system_hard_info.wifi_function = false;
    _device_interface->getSystemHardInfo(system_hard_info);
    if (is_wifi_terminal() == true) {
        system_hard_info.wifi_function = true;
    }
    _mina->mina_web_upload_basic_info(_basic_data.get(),
                                        _local_network_data.get_ip_info(),
                                        _local_network_data.get_wireless_mac(),
                                        _local_network_data.get_wired_mac(),
                                        net_type,
                                        _hostname_data.get_hostname_info(),
                                        system_hard_info);
    return 0;
}

int Application::w2l_sync_deb_path(LogicEvent* event)
{
    _mina->mina_web_request_deb_path();
    return 0;
}

int Application::w2l_upgrade_for_class(LogicEvent* event)
{
    char ret_buf[64];
    int ret_size = sizeof(ret_buf);

    UpgradeInfo *info = static_cast<UpgradeInfo*>(event->_data);
    ASSERT(info);
    if(event->_error != SUCCESS)
    {
        delete info;
        return event->_error;
    }

    VersionInfo *ver = &(info->versionInfo);

    //sw need update?
    LOG_INFO("server type:%d", info->serverType);
    LOG_INFO("server sw version:\n %s", ver->print_data().c_str());
    LOG_INFO("local sw version:\n %s", _version_data.get_version_info().print_data().c_str());

    //check client type
    if (__upgrade_check_product_type(_basic_data.get().product_id) == TYPE_OLD_RCO_CLIENT)
    {
        LOG_INFO("old rco client, should not update for class");
        l2u_result_updateclient_wrap(3);
        goto END;
    }

    if (check_old_system_version(ret_buf, ret_size))
    {
        LOG_INFO("system error or old os version, should not update for class");
        l2u_result_updateclient_wrap(5);
        goto END;
    }

    //check class hardware anti-upgrade strategy
    if (!__upgrade_rcc_check_product_id(*ver, _basic_data.get().product_id))
    {
        LOG_INFO("old server, should not update for class");
        l2u_result_updateclient_wrap(2);
        goto END;
    }

    //check os version anti-upgrade strategy
    if (!__upgrade_rcc_check_os(*ver, get_system_version()))
    {
        LOG_INFO("new kernal system, should not update for class");
        l2u_result_updateclient_wrap(8);
        goto END;
    }

    //if upgrade to class and execute client restore, should not check data
    if (!get_file_exist(RCC_CLIENT_LOCK))
    {
        if (_vm->vm_check_local_base_exist() || _vm->vm_check_personal_img_exist() || \
            _vm->vm_check_usedisk_exist() || _vm->vm_check_layerdisk_exist())
        {
            LOG_INFO("show data safe tip, should not update for class");
            l2u_show_tips(UI_TIPS_SERVER_IS_CLASS_OPT);
            goto END;
        }
    }

    if(!_vm->vm_is_vm_running())
        upgrade_class_software_version(*info);

END:
    delete info;
    return 0;
}

int Application::w2l_sync_software_version(LogicEvent* event)
{
    VersionInfo* ver;
    bool upgrade_result = true;
    bool upgrade_lock = false;
    char version_buf[128];
    string extra_first_buf;
    int pos;

    LOG_DEBUG("enter");

    ver = static_cast<VersionInfo*>(event->_data);
    ASSERT(ver);
    if(event->_error != SUCCESS)
    {
        delete ver;
        return event->_error;
    }

    //check client hardware type
    if (__upgrade_check_product_type(_basic_data.get().product_id) == TYPE_OLD_RCC_CLIENT) {
        LOG_INFO("old rcc client, should not update for office");
        l2u_result_updateclient_wrap(3);
        upgrade_result = false;
        goto END;
    }

    //check client already up to class
    if (get_file_exist(RCC_CLIENT_LOCK)) {
        LOG_INFO("already rcc version, should not update for office");
        l2u_result_updateclient_wrap(6);
        upgrade_result = false;
        goto END;
    }

    //check upgrade lock file exist
    if (get_file_exist(RCC_UPGRADE_LOCK)) {
        upgrade_lock = true;
    }

    //get version_buf
    extra_first_buf = ver->extra_first;//this variable is meant to delete '_' in extra_first
    pos = extra_first_buf.find("_");
    if (pos >-1)
    {
        extra_first_buf.erase(pos,1);
    }
    sprintf(version_buf, "%d.%d.%d-%s", ver->main_version, ver->minor_version, ver->fourth_version, extra_first_buf.c_str());

    //sw need update?
    LOG_INFO("server sw version:\n %s", ver->print_data().c_str());
    LOG_INFO("local sw version:\n %s", _version_data.get_version_info().print_data().c_str());

    if (upgrade_lock || *ver != _version_data.get_version_info())
    {
        if ((ver->main_version) != 1 && __upgrade_check_product_id(*ver, _basic_data.get().product_id)) {
            if (upgrade_lock) {
                LOG_INFO("%s is exist, need update", RCC_UPGRADE_LOCK);
            } else {
                LOG_INFO("SW version different, need update");
            }
            if(!_vm->vm_is_vm_running())
            {
                // main_version = 4 add layer function, can not support downgrade
                if (ver->main_version < 4 && _version_data.get_version_info().main_version >= 4) {
                    if (_vm->vm_check_layerdisk_exist()) {
                        LOG_INFO("dev locked not allow downgrade to 4.0!!!");
                        l2u_show_layer_downgrade(UI_DEV_LOCK_LAYER_DOWNGRADE);
                        upgrade_result = false;
                        goto END;
                    } else {
                        upgrade_result = upgrade_software_version(version_buf);
                    }
                } else {
                    upgrade_result = upgrade_software_version(version_buf);
                }
            }
            else
            {
                //vm is running, so upgrade next time
            }
        } else {
            LOG_INFO("SW main version is lower than local main version, should not update");
            l2u_result_updateclient_wrap(2);
            upgrade_result = false;
        }
    }

    //check all of packages are upgraded
    if (upgrade_result) {
        if (!_vm->vm_is_vm_running()) {
            check_software_upgrade(version_buf);
        } else {
            //vm is running, so upgrade next time
        }
    }

    if(upgrade_result)
    {
        sync_next_event(W2L_SYNC_SOFTWARE_VERSION);
    }

END:
    delete ver;
    return SUCCESS;
}

int Application::w2l_sync_hostname(LogicEvent* event)
{
    string* hostname;
    LOG_DEBUG("enter");

    hostname = static_cast<string*>(event->_data);
    ASSERT(hostname);
    
    if(event->_error != SUCCESS)
    {
        delete hostname;
        return event->_error;
    }

    _hostname_data.set_hostname_info(*hostname);
    LOG_INFO("server set hostname:%s",_hostname_data.get_hostname_info().c_str());

    sync_next_event(W2L_SYNC_HOSTNAME);

    delete hostname;

    return SUCCESS;
}


int Application::w2l_sync_mode(LogicEvent* event)
{
    ModeInfo* modeinfo;

    LOG_DEBUG("enter");
    
    modeinfo = static_cast<ModeInfo*>(event->_data);
    ASSERT(modeinfo);
    if(event->_error != SUCCESS)
    {
        delete modeinfo;
        return event->_error;
    }
    LOG_DEBUG("local mode: %d, bind_user: %s", _mode_data.get().mode, _mode_data.get().bind_user.username.c_str());
    LOG_DEBUG("server mode: %d, bind_user: %s", modeinfo->mode, modeinfo->bind_user.username.c_str());

    if (!_dev_status.IsLocked())
    {
        if (*modeinfo != _mode_data.get())
        {
            _mode_data.set(*modeinfo);
            _vm->vm_clear_teacher_disk();
            _vm->vm_clear_inst();
            _vm->vm_clear_layer();
        }
    }
    else
    {
        _dev_status.set_mode_info(*modeinfo);
    }

    sync_next_event(W2L_SYNC_MODE);

    delete modeinfo;

    return SUCCESS;
}

int Application::w2l_sync_server_time(LogicEvent*event)
{
    string* server_time;
    char command[512] = {0,};

    LOG_DEBUG("enter w2l synce server time");
    server_time = static_cast<string*>(event->_data);
    ASSERT(server_time);
    if (event->_error !=SUCCESS) {
        delete server_time;
        return event->_error;
    }
    sprintf(command, RCC_SCRIPT_PATH"rcc_convert_timestamps.sh %s", server_time->c_str());
    LOG_DEBUG("w2l_sync_server_time %s\n", command);
    rc_system(command);
    delete server_time;
    sync_next_event(W2L_SYNC_SERVER_TIME);

    return SUCCESS;
}

int Application::w2l_sync_dev_policy(LogicEvent* event)
{
    DevPolicyInfo* tmp;
    DevPolicyInfo  dev_info;
    bool set_recovery = true;
    bool set_userdisk = true;

    LOG_DEBUG("enter");

    tmp = static_cast<DevPolicyInfo*>(event->_data);
    ASSERT(tmp);
    if(event->_error != SUCCESS)
    {
        delete tmp;
        return event->_error;
    }

    dev_info = *tmp;
    LOG_DEBUG("local allow_recovery: %d, allow_userdisk: %d", _dev_policy.allow_recovery(), _dev_policy.allow_userdisk());
    LOG_DEBUG("server allow_recovery: %d, allow_userdisk: %d", dev_info.allow_recovery, dev_info.allow_userdisk);

/*
    if (!_dev_policy.allow_recovery() && (dev_info.allow_recovery == 1) &&
    		_mode_data.get().mode==SPECIAL_MODE && _vm->vm_check_personal_img_exist()) {
    	dev_info.allow_recovery = _dev_policy.allow_recovery();
    	set_recovery = false;
    }

    if (_dev_policy.allow_userdisk() && (dev_info.allow_userdisk == 0) &&
    		_mode_data.get().mode==SPECIAL_MODE && _vm->vm_check_usedisk_exist()) {
    	dev_info.allow_userdisk = _dev_policy.allow_userdisk();
    	set_userdisk = false;
    }
*/

    if (!_dev_status.IsLocked())  {
        if (dev_info.allow_recovery == true) {
            _vm->vm_clear_inst();
            _vm->vm_clear_layer();
        }

        if (dev_info.allow_userdisk == false) {
        	_vm->vm_clear_teacher_disk();
        }
        _dev_policy.update(dev_info);
        _mina->mina_l2w_notify_set_devpolicy(dev_info.id,set_recovery, dev_info.allow_recovery, set_userdisk, dev_info.allow_userdisk);
    }
    else
    {
        _dev_status.set_dev_policy(dev_info);
    	_mina->mina_l2w_notify_set_devpolicy(dev_info.id,false, _dev_policy.allow_recovery(), false, _dev_policy.allow_userdisk());
    }
    //_dev_policy.update_finish();

    delete tmp;

    return SUCCESS;
}

int Application::w2l_sync_image(LogicEvent* event)
{
    ImageInfo* info;
    bool need_sync = true;
    bool need_resize_c = false;
    int need_clean_usedisk;
    int d_disk_size = 0;
    bool enough;
    int c_disk_size = 0;
    int virt_size;
    int set_size;
    int max_c_disk_size = 0;
    bool need_clear_inst = false;
    char download_size[20];
    int update_package_size = 0;
    int unit_index;
    HttpPortInfo hpi;

    ImageInfo local_image_info;
    info = static_cast<ImageInfo*>(event->_data);
    ASSERT(info);
    if(event->_error != SUCCESS)
    {
        delete info;
        return event->_error;
    }

    LOG_INFO("imageinfo: id=%d, name=%s, version=%s, url=%s, md5=%s ostype = %s real_size=%d, virt_size=%d, set_size=%d",
        info->id, info->name.c_str(), info->version.c_str(), info->torrent_url.c_str(), info->md5.c_str(), info->ostype.c_str(), info->real_size, info->virt_size, info->set_size);
        local_image_info = _vm->vm_get_vm_imageinfo();
    //bool download_status = _vm->vm_get_vm_download_status();
    LOG_INFO("local_image_info: id=%d, name=%s, version=%s, url=%s, md5=%s, ostype=%s, real_size=%d, virt_size=%d",
        local_image_info.id, local_image_info.name.c_str(),
        local_image_info.version.c_str(), local_image_info.torrent_url.c_str(),
        local_image_info.md5.c_str(), local_image_info.ostype.c_str(), local_image_info.real_size, local_image_info.virt_size);

    /*
       if id or version of image is different:
            recovery image: update
            personalized image:special mode has inst can not update, otherwise update
       else 
            if recovery of image is different
                personalized->recovery: special mode has inst can not change, otherwise change
                recovery->personalized: change
        */

    _server_image_info = *info;
    delete info;

    if (_dev_status.IsLocked())
    {
        //will not get here, because show dev locked if data protected.
        l2u_show_tips(UI_TIPS_DEV_LOCKED_OPT);
        return SUCCESS;
    }
    else
    {
        if (_dev_policy.allow_recovery() == true) {
            need_clear_inst = true; ///_vm->vm_clear_inst();
            _vm->vm_clear_layer();
        }
        if (_dev_policy.allow_userdisk() == false) {
            _vm->vm_clear_teacher_disk();
        }
    }

    virt_size = _server_image_info.virt_size;
    set_size = _server_image_info.set_size;
    if (_server_image_info.virt_size > _server_image_info.set_size) {
        _server_image_info.set_size = _server_image_info.virt_size;
    }

    enough = _vm->space_enough(_server_image_info.real_size, _server_image_info.set_size, &need_clean_usedisk);
    if (enough) {
        if (need_clean_usedisk) {
            enough = _vm->space_enough(_server_image_info.real_size, _server_image_info.virt_size, &need_clean_usedisk);
        } else {
            _server_image_info.virt_size = _server_image_info.set_size;
        }
    } else {
        enough = _vm->space_enough(_server_image_info.real_size, _server_image_info.virt_size, &need_clean_usedisk);
    }

    //if _status_machine.get_status() == STATUS_NEW_DEPLOY
    //we just do download img but not change status 
    if(_status_machine.get_status() == STATUS_NEW_DEPLOY)
    {
        if(!enough) {
            web_image_sync_error(-5);
            _mina->mina_l2w_notify_partition(0, 0, 0);
            return SUCCESS;
        }

        //FIXME: now we just do the same thing as we do in on_entering_downloading_image
        LOG_INFO("we just do download img but not change status");

        _vm->vm_set_image_downmode(DOWNLOAD_IMAGE_FORCE_DOWNLOAD);
        _newdeploy_manage->enter_download_image();
        //return SUCCESS;
        c_disk_size = _server_image_info.virt_size;
        _vm->cal_d_disk_size(_server_image_info.real_size, _server_image_info.virt_size, &d_disk_size);
        goto end;
    }

   // check download policy
    if (local_image_info.id == _server_image_info.id && local_image_info.version == _server_image_info.version) {
        need_sync = false;
    } else {
        need_sync = true;

        if(_mode_data.get().mode == SPECIAL_MODE && _mode_data.get().bind_user.username.empty() && _vm->vm_check_local_base_exist())
        {
            LOG_DEBUG("SPECIAL_MODE and not bind and we already have a image, we do not need download image this time");
            need_sync = false;
        }

        if (_vm->vm_check_personal_img_exist()) {
            LOG_DEBUG("it has personal img");
            need_sync = false;
        }
        else if (_dev_policy.allow_recovery() == false)
        {   
            LOG_DEBUG("not allow recovery, but WEB RECORVER IMAGE");
            _vm->vm_set_image_downmode(DOWNLOAD_IMAGE_FORCE_DOWNLOAD); //¸ö£ʽÏ »¹ԭÔ×ÃºóÐǿÖsync image
        }

        if (_vm->vm_check_layerdisk_exist()) {
            // if Individual user layer disk is exist
            if (local_image_info.id == _server_image_info.id && local_image_info.version !=_server_image_info.version) {
                if (local_image_info.virt_size > _server_image_info.virt_size) {
                    _server_image_info.virt_size = local_image_info.virt_size;
                }

                need_sync = true;
                _vm->vm_clear_inst();
                enough = _vm->space_enough(_server_image_info.real_size, _server_image_info.virt_size, &need_clean_usedisk);
                LOG_DEBUG("if has layer img, layer_on is on! need_sync %d enough %d", need_sync, enough);
            }

            if (local_image_info.id != _server_image_info.id) {
                need_sync = false;
            }
        }
        
        _userinfomgr.get_http_port_map(hpi);
        /* only when port mapped, allow file doesn't exist and image id is not null */
        if (hpi.public_port != 80) {
            if (!get_file_exist(CLIENT_ALLOW_IMAGE_SYNC_FLAG_FILE)) {
            	if (local_image_info.id != 0) {
            		LOG_DEBUG("http public port %d has been mapped, image sync is disabled",
						hpi.public_port);
					//need_sync = false;
            	} else {
            		LOG_DEBUG("local image id is 0, image sync is enabled");
            	}
            } else {
            	LOG_DEBUG("http public port %d has been mapped, but allow file exists, image sync is enabled",
				    hpi.public_port);
            }
        }
        
        /*
        //same image
            if (local_image_info.name == _server_image_info.name && local_image_info.id ==_server_image_info.id)
            {
                if(_mode_data.get().mode==SPECIAL_MODE && _vm->vm_check_personal_img_exist())
                {
                    LOG_DEBUG("SPECIAL_MODE has personal img, can not update image!");
                    need_sync = false;
                }
            }
        */
    }

    // check space
    if (need_sync) {
        if (!enough || need_clean_usedisk) {
            need_sync = false;

            if (local_image_info.name.empty() || _vm->vm_check_local_base_exist() <= 0) {
                web_image_sync_error(-5);
                _mina->mina_l2w_notify_partition(0, 0, 0);
                return SUCCESS;
            } else {
                //UIPopEvent sync_event("img too big");
                //l2u_show_img_big_tips((void *)Application::uipop_sync ,(void *)&sync_event);
                //sync_event.wait();
                LOG_INFO("not allow sync image becase space not enough; enough(%d), need_clean_usedisk(%d)", enough, need_clean_usedisk);
            }

        }
    }

    if (need_sync) {
        UIPopEvent sync_event("download confirm");
        //bool confirm = true;
        if (_vm->vm_check_personal_img_exist()) {
            l2u_show_download_confirm_tips((void *)Application::uipop_sync, (void *)&sync_event, (void *)&need_sync);
            sync_event.wait();
            if (need_sync == false) {
                LOG_INFO("user cancel image update");
            }
        }
    }

    
    if(DOWNLOAD_IMAGE_NO_OPS != _vm->vm_get_image_downmode())  
    {
        LOG_INFO("Last time downmode : %d",_vm->vm_get_image_downmode());
        if ( (_server_image_info.name!=_vm->vm_get_new_base_name()) || (_server_image_info.id!=_vm->vm_get_new_base_id()) ||(_server_image_info.version!=_vm->vm_get_new_base_version())) //It's in the process of updating, but there's a new image
        {
            if (DOWNLOAD_IMAGE_FORCE_DOWNLOAD == _vm->vm_get_image_downmode())
            {
                 LOG_INFO("Last normal download update incomplete, Clear last update info and re respond to new updates");
                 if (need_clear_inst) _vm->vm_clear_inst();
                 need_sync = true;
                 _vm->vm_set_image_downmode(DOWNLOAD_IMAGE_FORCE_DOWNLOAD);
            }
            else if ((DOWNLOAD_IMAGE_SILENT_DOWNLOAD == _vm->vm_get_image_downmode()) ||(DOWNLOAD_IMAGE_NEED_MERGE == _vm->vm_get_image_downmode())  )
            {
                 LOG_INFO("Last silent download update incomplete, Clear last update info and re respond to new updates");
                 _vm->vm_set_image_downmode(DOWNLOAD_IMAGE_NO_OPS); 
                 rc_system("rm -rf /opt/lessons/diff/*");
            }
            else if (DOWNLOAD_IMAGE_MERGING == _vm->vm_get_image_downmode())
            {
                 LOG_INFO("Last merging incomplete, Clear last update info and re respond to force download");
                 _vm->vm_set_image_downmode(DOWNLOAD_IMAGE_FORCE_DOWNLOAD);
                 need_sync = true;
                 rc_system("rm -rf /opt/lessons/diff/*");
            }
        }
    }
    
    if(_vm->vm_check_local_base_status() != 1)
    {
        LOG_INFO("local base not exist");
        if (need_clear_inst) _vm->vm_clear_inst();
        need_sync = true;
        _vm->vm_set_image_downmode(DOWNLOAD_IMAGE_FORCE_DOWNLOAD);
    }
    
      if ((need_sync)&&(DOWNLOAD_IMAGE_NO_OPS == _vm->vm_get_image_downmode())) {
        
        update_package_size = _vm->vm_get_image_diff_size(_server_image_info.torrent_url,local_image_info.torrent_url);
        if (0 == update_package_size) {
            LOG_INFO("vm_get_image_diff_size error");
            if (need_clear_inst) _vm->vm_clear_inst();
            _vm->vm_set_image_downmode(DOWNLOAD_IMAGE_FORCE_DOWNLOAD);
        }
        else 
        {   
            if (update_package_size<1024){
                sprintf(download_size,"%dKB",((int)update_package_size));
            }
            else if (update_package_size<(1024*1024)) {
                sprintf(download_size,"%dMB",((int)update_package_size)/1024);
            }            
            else {
                sprintf(download_size,"%.1fGB",((float)update_package_size)/(1024*1024));
            }
            
            LOG_INFO("This update need download size: = %s",download_size);
            if (_vm->space_enough_silent(_server_image_info.real_size, _server_image_info.virt_size,(update_package_size/(1024*1024)+1)))
            {
                LOG_INFO("silent_download");
                UIPopEvent sync_event("silent_download tips");
                l2u_set_update_image_size(download_size);
                l2u_show_download_mode_select_tips((void *)Application::uipop_sync, (void *)&sync_event, (void *)&need_sync);
                sync_event.wait();
                
                if (need_sync == false) {
                    //LOG_INFO("need_sync == false");
                    _vm->vm_set_image_downmode(DOWNLOAD_IMAGE_SILENT_DOWNLOAD);
                    need_sync = true;
                }
                else{
                    //LOG_INFO("need_sync == true");
                    _vm->vm_set_image_downmode(DOWNLOAD_IMAGE_FORCE_DOWNLOAD);
                    need_sync = true;
                    }
            }
            else
            {
                UIPopEvent sync_event("img too big");
                l2u_show_img_big_tips((void *)Application::uipop_sync ,(void *)&sync_event);
                sync_event.wait();
                if (need_clear_inst) _vm->vm_clear_inst();
                _vm->vm_set_image_downmode(DOWNLOAD_IMAGE_FORCE_DOWNLOAD);
                LOG_INFO("not allow silent_download image becase space not enough;");
            }

        }
     }
     else if(DOWNLOAD_IMAGE_SILENT_DOWNLOAD == _vm->vm_get_image_downmode()){
        update_package_size = _vm->vm_get_image_diff_size(_server_image_info.torrent_url,local_image_info.torrent_url);  //count total_size for zhjsGT
     }



    if (need_sync) {
/*      if (need_clean_usedisk) {
            _vm->vm_clear_teacher_disk();
        }
*/
        _vm->cal_d_disk_size(_server_image_info.real_size, _server_image_info.virt_size, &d_disk_size);
        c_disk_size = _server_image_info.virt_size;
        _status_machine.change_status(EVENT_IMAGE_NEED_DOWNLOAD);
    } else {
        c_disk_size = local_image_info.virt_size;

        //c_disk_size = _vm->vm_get_c_disk_size();
        d_disk_size = _vm->vm_get_user_disk_size();
        LOG_INFO("get local c disk %d, D disk %d\n", c_disk_size, d_disk_size);
        need_resize_c = true;

        if (!enough ) {
            need_resize_c = false;
            LOG_INFO("not allow reszie  space not enough");
        }

        if ((_server_image_info.name != local_image_info.name || _server_image_info.id != local_image_info.id) && (virt_size == _server_image_info.virt_size && virt_size != set_size)) {
            need_resize_c = false;
            LOG_INFO("not allow reszie  c becase this image not download,  not allow use virt_size resize");
        }

        if ((_server_image_info.virt_size <= local_image_info.virt_size) && _vm->vm_check_personal_img_exist()) {
            need_resize_c = false;
            LOG_INFO("not allow reszie  c becase has personal img and local size %d >= sever size %d", local_image_info.virt_size, _server_image_info.virt_size);
        }

        if (need_clean_usedisk) {
            need_resize_c = false;
            LOG_INFO("not allow reszie  c becase  not allow del usedisk ");
        }

        if (need_resize_c) {
            LOG_INFO("need resize image %s local size %d  reszie %d", local_image_info.name.c_str(), local_image_info.virt_size, _server_image_info.virt_size);
/*			if (need_clean_usedisk) {
				_vm->vm_clear_teacher_disk();
			}
*/
            local_image_info.virt_size = _server_image_info.virt_size;
            _vm->vm_update_image_info(&local_image_info);
            c_disk_size = _server_image_info.virt_size;

/*			if (_vm->vm_resize_image(_server_image_info.name, _server_image_info.virt_size, "qcow2")) {
				//_vm->vm_download_torrent("", _server_image_info.torrent_url);

				local_image_info.virt_size = _server_image_info.virt_size;
				_vm->vm_update_image_info(&local_image_info);
				c_disk_size = _server_image_info.virt_size;

			}
*/
        }
        //if (_server_image_info.name == local_image_info.name && _server_image_info.id == local_image_info.id) {
            _vm->cal_d_disk_size(local_image_info.real_size, c_disk_size, &d_disk_size);
        //}
        if (enough) {
            if (_server_image_info.id == local_image_info.id && _server_image_info.ostype != local_image_info.ostype) {
                LOG_INFO("update ostype to vm_imgage_info.ini");
                local_image_info.ostype = _server_image_info.ostype;
                _vm->vm_update_image_ostype(&local_image_info.ostype);

                //TODO: if ostype is emluation then kill the hotplug server
            }
        }

        if (_server_image_info.id == local_image_info.id && _server_image_info.layer_info.layer_on_1 != local_image_info.layer_info.layer_on_1) {
            LOG_INFO("update layer info to vm_imgage_info.ini");
            local_image_info.layer_info.layer_on_1 = _server_image_info.layer_info.layer_on_1;
            _vm->vm_update_image_info(&local_image_info);
        }

        _status_machine.change_status(EVENT_IMAGE_LATEST);
    }
end:
    if (_dev_policy.allow_userdisk() == true) {
        max_c_disk_size = _vm->cal_max_c_disk_size(local_image_info.real_size, c_disk_size, d_disk_size);
        _mina->mina_l2w_notify_partition(c_disk_size, d_disk_size, max_c_disk_size);
    } else {
        max_c_disk_size = _vm->cal_max_c_disk_size(local_image_info.real_size, c_disk_size, 0);
        _mina->mina_l2w_notify_partition(c_disk_size, 0, max_c_disk_size);
    }

    return SUCCESS;
}

void Application::send_download_progress_info_to_zhjsgt(string downloaded_size,string total_size,string percent,string rate)
{
   _vm->vm_send_download_progress_info_to_zhjsgt(downloaded_size,total_size,percent,rate);
}
void Application::prepare_merge_image()
{
    //bool flag = true;
    LOG_INFO("merge tips");
    if(_vm->vm_is_vm_running()){
         LOG_INFO("vm_is_vm_running");
        _vm->vm_set_guest_merge_tips();
    }
    else{
        logic_download_success_tips();
      /*  UIPopEvent sync_event("merge tips");
        l2u_show_need_merge_tips((void *)Application::uipop_sync, (void *)&sync_event, (void *)&flag);
        sync_event.wait();
        rc_system("reboot");*/
    }
}

void Application::v2l_reboot_system_merge_image()
{
    LOG_INFO("reboot_to_merge");
    rc_system("vmmanager --normal_shutdown");
    sleep(5);
    rc_system("reboot");
}

void Application::v2l_clean_download_success_tips()
{
   _vm->vm_clean_download_success_tips();
}

int Application::w2l_sync_public_policy(LogicEvent* event)
{
    PolicyInfo* policy;
    UserInfoMgr user_info_mgr;

    LOG_DEBUG("enter");
    policy = static_cast<PolicyInfo*>(event->_data);
    ASSERT(policy);
    if(event->_error != SUCCESS)
    {
        delete policy;
        return event->_error;
    }
    _public_policy_info.usb_policy =  policy->usb_policy;
    _public_policy_info.net_policy =  policy->net_policy;
    _public_policy_info.pub.outctrl_day = policy->pub.outctrl_day;

    user_info_mgr.PublicPolicyStore(_public_policy_info);
    LOG_INFO("update pubic policy, usb_policy=%s, net_policy=%d, out_ctrldate %d",
    		policy->usb_policy.c_str(), policy->net_policy, policy->pub.outctrl_day);
    
    sync_next_event(W2L_SYNC_PUBLIC_POLICY);

    delete policy;

    return SUCCESS;
}

int Application::w2l_sync_ipxe(LogicEvent* event)
{
    IpxeInfo* info;
    bool allow_ipxe = true;
    char run_pxe_cmd[512];

    info = static_cast<IpxeInfo*>(event->_data);
    ASSERT(info);
    
    if(event->_error != SUCCESS)
    {
        delete info;
        return event->_error;
    }

    LOG_INFO("begin w2l_sync_ipxe");
    if (info->iso_version.empty() || info->iso_name.empty()) {
        LOG_DEBUG("iso_name or iso_version is empty");
        goto END;
    } else {
        allow_ipxe = _updata_policy.allow_version(info->iso_version);
    }

    //chenw debug
    if (allow_ipxe) {
        ::save_ipxe_configure();
        sprintf(run_pxe_cmd,"sh /usr/cz_prep.sh %s %d %s %s 2 %s l"\
            , _local_network_data.get().ip.c_str()\
            , get_submask_bitcount(_local_network_data.get().submask.c_str())\
            , _local_network_data.get().gateway.c_str()\
            , _mina->mina_get_server_ip().c_str()\
            , info->iso_name.c_str());
        LOG_DEBUG(run_pxe_cmd);
        rc_system(run_pxe_cmd);

    } else {
        UIPopEvent sync_event("iso upgrade err");
        l2u_show_iso_upgrade_err((void *)Application::uipop_sync ,(void *)&sync_event);
        sync_event.wait();
    }

END:
    sync_next_event(W2L_SYNC_IPXE);

    delete info;

    return SUCCESS;

}

int Application::w2l_sync_recover_image(LogicEvent* event)
{
    int* has_command;
        
    LOG_DEBUG("enter");
    
    has_command = static_cast<int*>(event->_data);
    ASSERT(has_command);
    if(event->_error != SUCCESS)
    {
        delete has_command;
        return event->_error;
    }
    
    if(*has_command)
    {
        LOG_INFO("do sync_recovery");
        _vm->vm_clear_inst();
        _vm->vm_clear_layer();
    }

    sync_next_event(W2L_SYNC_RECOVER_IMAGE);

    delete has_command;

    return SUCCESS;

}

int Application::w2l_sync_reset_terminal(LogicEvent* event)
{
    int* has_command;

    LOG_DEBUG("enter");
    
    has_command = static_cast<int*>(event->_data);
    ASSERT(has_command);
    if(event->_error != SUCCESS)
    {
        delete has_command;
        return event->_error;
    }
    
    if(*has_command)
    {
        LOG_INFO("do sync_initial");
        
        //show ui
        l2u_show_tips(UI_TIPS_CLIENT_INIT_OPT);
        l2u_ctrl_winbtn(false);
        l2u_show_wifi_btnbox(2, -1, -1);

        if (_net_auth->getAuthExist() == 0) {
            l2u_show_net_auth_winbtn(UI_AUTHBTN_DISABLED);
        }


        _ui_locking = true;
        _wait_tips_ok = true;

        //step4 reboot
        delete has_command;
        return SUCCESS;
    }

    sync_next_event(W2L_SYNC_RESET_TERMINAL);

    delete has_command;

    return SUCCESS;

}

int Application::w2l_sync_server_ip(LogicEvent* event)
{
    int* is_valid;
        
    LOG_DEBUG("enter");
    
    is_valid = static_cast<int*>(event->_data);
    ASSERT(is_valid);
    if(event->_error != SUCCESS)
    {
        delete is_valid;
        return event->_error;
    }

    if (!_dev_status.IsLocked())
    {
        _mina->mina_set_last_server_ip(_mina->mina_get_server_ip());
    }

    sync_next_event(W2L_SYNC_SERVER_IP);

    delete is_valid;

    return SUCCESS;

}

int Application::w2l_sync_all_userinfo(LogicEvent* event)
{
    UserInfos* userinfos;
        
    LOG_DEBUG("enter");
    
    userinfos = static_cast<UserInfos*>(event->_data);
    ASSERT(userinfos);
    if(event->_error != SUCCESS)
    {
        delete userinfos;
        return event->_error;
    }

    _userinfomgr.updateAllUsersInfo(*userinfos);

    sync_next_event(W2L_SYNC_ALL_USERINFO);

    delete userinfos;

    return SUCCESS;

}


int Application::w2l_get_vm_dev_interface_info(LogicEvent* event)
{
    DevInterfaceInfo* infos; 
    LOG_DEBUG("enter");
    infos = static_cast<DevInterfaceInfo*>(event->_data);
    ASSERT(infos);
    if(event->_error != SUCCESS)
    {
        delete infos;
        return event->_error;
    }
    _userinfomgr.set_dev_interface_info(*infos);
    sync_next_event(W2L_GET_VM_DEV_INTERFACE_INFO);

    delete infos;
    return SUCCESS;
}

int Application::w2l_sync_ssid_whitelist(LogicEvent * event)
{
    vector<string> *whitelist;
    list<net::WifiConfig> his_list;
    
    whitelist = static_cast<vector<string> *>(event->_data);
    ASSERT(whitelist);

    if(event->_error != SUCCESS)
    {
        delete whitelist;
        return event->_error;
    }

    LOG_DEBUG("enter set_white_ssid_list")
  	if (is_wifi_terminal() == true) {
        _userinfomgr.deletessidWhite();
        _userinfomgr.storessidWhiteList(*whitelist);
        l2u_show_not_whitessid_show(*whitelist);
        _wifi_interactive->wifi_forget_not_whitessid_list(*whitelist);
    }
    
    delete whitelist;
    sync_next_event(W2L_SYNC_SSID_WHITELIST);
    return SUCCESS;
}


int Application::w2l_sync_terminal_password(LogicEvent* event)
{
    string *passwd;

    passwd = static_cast<string *>(event->_data);
    ASSERT(passwd);

    // call terminal interface
    LOG_DEBUG("%s passwd: %s", __func__, (*passwd).c_str());
    _device_interface->set_terminal_passwd(*passwd);
    delete passwd;
    sync_next_event(W2L_SYNC_TERMINAL_PSW);
    return SUCCESS;
}


int Application::w2l_sync_printer_switch(LogicEvent* event)
{
    int *printer_switch;
    DiskInfo_t printer_disk;

    LOG_DEBUG("enter w2l_sync_printer_switch")
    printer_switch = static_cast<int *>(event->_data);
    ASSERT(printer_switch);

    if(event->_error != SUCCESS)
    {
        delete printer_switch;
        return event->_error;
    }

    // save the value to diskfile
    printer_disk.disk_name = PRINTER_DISK_NAME;
    printer_disk.is_enable = *printer_switch;
    printer_disk.size = PRINTER_DISK_SIZE;

    _userinfomgr.set_disk_info(printer_disk);

    delete printer_switch;
    sync_next_event(W2L_SYNC_PRINTER_SWITCH);
    return SUCCESS;

}

int Application::w2l_sync_desktop_redir(LogicEvent* event)
{
    string *redir_switch;

    LOG_DEBUG("enter w2l_sync_desktop_redir")
    redir_switch = static_cast<string *>(event->_data);
    ASSERT(redir_switch);

    if(event->_error != SUCCESS)
    {
        delete redir_switch;
        return event->_error;
    }

    _userinfomgr.set_vm_desktop_redir(*redir_switch);

    delete redir_switch;
    sync_next_event(W2L_SYNC_DESKTOP_REDIR);

    return SUCCESS;
}

int Application::w2l_sync_rcdusbconf_info(LogicEvent * event)
{
    string *usb_conf;

    usb_conf = static_cast<string *>(event->_data);
    ASSERT(usb_conf);
    if(event->_error != SUCCESS) {
        delete usb_conf;
        return event->_error;
    }

    RcdUsbConf rcd_usb("w+");
    //RcdUsbConfInfoDB rcd_usb;
    rcd_usb.wite_file(usb_conf->c_str());

    delete usb_conf;
    sync_next_event(W2L_SYNC_RCDUSBCONF_INFO);
    return SUCCESS;
}

bool Application::__mount_nfs(const string& remote_path, const string& local_path)
{
    char command_buf[256];
    char read_result[256];
    memset(command_buf, 0, sizeof(command_buf));
    memset(read_result, 0, sizeof(read_result));
    int read_size = sizeof(read_result);

    //do edit_server_img
    sprintf(command_buf, "timeout 2 showmount -e %s | grep %s", _mina->mina_get_server_ip().c_str(), remote_path.c_str());
    LOG_INFO(command_buf);
    rc_system_rw(command_buf, (unsigned char *)read_result, &read_size, "r");
    if(strlen(read_result) == 0)
    {
        _mina->mina_web_upload_driver_install_result(ERROR_MOUNT_NFS_FAIL);
        return false;
    }
    else
    {
        if(!get_file_exist(local_path.c_str()))
        {
            sprintf(command_buf, "mkdir -p %s", local_path.c_str());
            rc_system(command_buf);
        }
        sprintf(command_buf, "mount %s:%s %s", _mina->mina_get_server_ip().c_str(), remote_path.c_str(), local_path.c_str());
        rc_system(command_buf);
    }
    return true;

}

int Application::w2l_sync_driver_install(LogicEvent* event)
{
    string* msg;
            
    LOG_DEBUG("enter");
    msg = static_cast<string*>(event->_data);
    ASSERT(msg);
    if(event->_error != SUCCESS)
    {
        delete msg;
        return event->_error;
    }
    int action = rc_json_get_int(*msg, "action");
    string path_name        = rc_json_get_string(*msg, "path_name");
    string driver_path_name = rc_json_get_string(*msg, "driver_path_name");


    if(action == 0)
    {
        LOG_INFO("not need to edit_server_img, action = %d", action);
    }
    else
    {
        if(__mount_nfs(path_name, path_name) && __mount_nfs(driver_path_name, driver_path_name))
        {
            _driver_install_parament = *msg;
        }
    }
    sync_next_event(W2L_SYNC_DRIVER_INSTALL);
    delete msg;
    return SUCCESS;
}
/*
int Application::w2l_sync_reload_image(LogicEvent* event)
{
    int* has_command;
        
    LOG_DEBUG("enter");
    
    has_command = static_cast<int*>(event->_data);
    ASSERT(has_command);
    if(event->_error != SUCCESS)
    {
        delete has_command;
        return event->_error;
    }
    
    if(*has_command)
    {

    }

    sync_next_event(W2L_SYNC_RELOAD_IMAGE);

    delete has_command;

    return SUCCESS;

}
*/
int Application::w2l_sync_delete_teacherdisk(LogicEvent* event)
{
    int* has_command;
    int saved_disk_size = 0;

    LOG_DEBUG("enter");
    has_command = static_cast<int*>(event->_data);
    ASSERT(has_command);
    if(event->_error != SUCCESS)
    {
        delete has_command;
        return event->_error;
    }
    
    if(*has_command)
    {
        LOG_INFO("do delete_teacherdisk");
        saved_disk_size = _vm->vm_get_user_disk_size();
        _vm->vm_clear_teacher_disk();
        if (!_vm->vm_create_user_disk(saved_disk_size)) {
            LOG_ERR("vm_create_user_disk failed, saved size = %d", saved_disk_size);
            LOG_NOTICE("you might get a error size of datadisk");
        } 
    }

    sync_next_event(W2L_SYNC_DELETE_TEACHERDISK);

    delete has_command;

    return SUCCESS;

}


int Application::w2l_sync_port_mapping(LogicEvent* event)
{
    PortMappingInfos* infos;    
    unsigned int i = 0;
    LOG_DEBUG("enter");
    
    infos = static_cast<PortMappingInfos*>(event->_data);

    _port_mapping_infos.clear();
    for(i = 0; i < (*infos).size(); i++)
    {
        _port_mapping_infos.push_back((*infos)[i]);
    }
    sync_next_event(W2L_SYNC_PORT_MAPPING);
    delete infos;
    return SUCCESS;
}

void* Application::download_picture(void* data)
{
    struct MainWindowInfo *arg = (struct MainWindowInfo*)data;
    string cmd("wget --tries=3 --wait=1 --connect-timeout=5 --read-timeout=30 --no-check-certificate --output-document=");
    string tmp_file;
    string dst_file;
    int ret, type;

    LOG_INFO("pictureMD5:%s", arg->pictureMD5.c_str());
    LOG_INFO("pictureUrl:%s", arg->pictureUrl.c_str());
    LOG_INFO("type:%s", arg->type.c_str());

    if (arg->type == "LOGO") {
        tmp_file = UI_USER_TMP_LOGO;
        dst_file = UI_USER_LOGO;
        type     = UI_CALL_MAIN_LOGO_SET;
    } else {
        tmp_file = UI_USER_TMP_BG;
        dst_file = UI_USER_BG;
        type     = UI_CALL_MAIN_BG_SET;
    }

    cmd = cmd + tmp_file + " " + arg->pictureUrl;
    LOG_INFO("DOWNLOAD CMD:%s", cmd.c_str());
    ret = rc_system(cmd.c_str());
    if (ret != 0) {
        LOG_WARNING("execute picture download failed!");
        delete arg;
        return NULL;
    }

    LOG_INFO("RENAME TMP-FILE:%s TO FILE:%s", tmp_file.c_str(), dst_file.c_str());
    cmd.clear();
    cmd = "mv " + tmp_file + " " + dst_file;
    ret = rc_system(cmd.c_str());
    if (ret != 0) {
        LOG_WARNING("save picture failed!");
        delete arg;
        return NULL;
    }

    if (!Application::get_application()->_vm->vm_is_vm_running())
        l2u_show_main_window(type);

    delete arg;
    return NULL;
}

void Application::__update_origin_picture(MainWindowInfo &info)
{
    string file;
    int reset_type;
    char file_md5[64] = {0};

    if (info.type == "LOGO") {
        file = UI_USER_LOGO;
        reset_type = UI_CALL_MAIN_LOGO_RESET;
    } else if (info.type == "TERMINAL_BACK_GROUND") {
        file = UI_USER_BG;
        reset_type = UI_CALL_MAIN_BG_RESET;
    } else {
        LOG_ERR("type is not exist!");
        return;
    }

    LOG_INFO("update picture:%s, reset_type:%d", file.c_str(), reset_type);
    LOG_INFO("pictureMD5:%s", info.pictureMD5.c_str());
    LOG_INFO("pictureUrl:%s", info.pictureUrl.c_str());
    LOG_INFO("type:%s", info.type.c_str());

    //pares picture info
    if (info.pictureMD5.empty()) {
        if (get_file_exist(file.c_str())) {
            //reset background
            string cmd = "rm -f " + file;
            rc_system(cmd.c_str());
            LOG_INFO("RESET CMD:%s", cmd.c_str());
            if (!_vm->vm_is_vm_running())
                l2u_show_main_window(reset_type);
        }
    } else {
        if (get_file_exist(file.c_str())) {
            get_file_md5_key(file.c_str(), file_md5);
            LOG_INFO("compare picture:%s, local md5:%s, web md5:%s", info.type.c_str(), file_md5, info.pictureMD5.c_str());
            if (strcmp(info.pictureMD5.c_str(), file_md5)) {
                LOG_INFO("replace picture");
                MainWindowInfo *data = new (struct MainWindowInfo);
                data->pictureMD5 = info.pictureMD5;
                data->pictureUrl = info.pictureUrl;
                data->type = info.type;
                UnjoinableThread download_pic_thread(download_picture, (void *)data);
            }
        } else {
            LOG_INFO("replace picture");
            MainWindowInfo *data = new (struct MainWindowInfo);
            data->pictureMD5 = info.pictureMD5;
            data->pictureUrl = info.pictureUrl;
            data->type = info.type;
            UnjoinableThread download_pic_thread(download_picture, (void *)data);
        }
    }

    return;
}

void* Application::sync_main_window(void* data)
{
    MainWindowInfos* infos = (MainWindowInfos *)data;
    unsigned int i = 0;

    for (i = 0; i < (*infos).size(); i++) {
        Application::get_application()->__update_origin_picture((*infos)[i]);
    }

    delete infos;
    return NULL;
}

int Application::w2l_sync_main_window(LogicEvent* event)
{
    MainWindowInfos* infos;
    unsigned int i = 0;

    LOG_DEBUG("enter");
    infos = static_cast<MainWindowInfos*>(event->_data);

    MainWindowInfos *datas = new MainWindowInfos();
    for (i = 0; i < (*infos).size(); i++) {
        datas->push_back((*infos)[i]);
    }

    //TODO:add to thread
    UnjoinableThread sync_mainwindow_thread(sync_main_window, (void *)datas);
    sync_next_event(W2L_SYNC_MAIN_WINDOW);

    delete infos;
    return SUCCESS;
}

void Application::set_net_disk_port()
{
#define NET_DISK_PORT  (445)
    char command[256];
    unsigned int i = 0;    
    string net_disk_ip = _logined_user_info.netdisk_info.netdisk_ip;
    LOG_INFO("enter set_net_disk_port, netdisk_ip = %s", net_disk_ip.c_str());

    if (!net_disk_ip.empty()) {
        sprintf(command, "bash /usr/local/bin/rc_portmapped.bash clean");
        LOG_INFO(command);
        rc_system(command);
    
        for(i = 0; i < _port_mapping_infos.size(); i++)
        {
            if (_port_mapping_infos[i].src_port == NET_DISK_PORT)
            {
                sprintf(command, "bash /usr/local/bin/rc_portmapped.bash %s %d %d", net_disk_ip.c_str(), NET_DISK_PORT, _port_mapping_infos[i].dst_port);            
                LOG_INFO(command);
                rc_system(command);
            }
        }
    }
#undef NET_DISK_PORT
}


int Application::w2l_init_sync_error(LogicEvent* event)
{
    LOG_WARNING("here comes init_sync_error");
    int* errcode;
    errcode = static_cast<int*>(event->_data);
    ASSERT(errcode);
    LOG_DEBUG("errcode:%d", *errcode);
    _error_tips = true;
    switch(*errcode)
    {
        case 1:     //server has error
            l2u_show_tips(UI_TIPS_SERVER_ERROR_OPT);
            break;
        case 10:    //server has more than 1000 terminals
            l2u_show_tips(UI_TIPS_DEV_OVERRUN_OPT);
            break;
        case 11:    //server has more than 200 terminals
            l2u_show_tips(UI_TIPS_LICENSE_OVERRUN_OPT);
            break;
        case 22:
            l2u_show_tips(UI_TIPS_SERVER_NOT_VALID_OPT);
            break;
        case 99:
            l2u_show_tips(UI_TIPS_SERVER_ERROR_OPT);
            break;
        case 122:
            l2u_show_tips(UI_TIPS_AUTUHOR_NOT_FOUND_OPT);
            break;
        case 123:
            l2u_show_tips(UI_TIPS_AUTUHOR_OUT_OF_RANGE_OPT);
            break;
        default:
            l2u_show_tips(UI_TIPS_SERVER_ERROR_OPT);
            break;
    }
    
    delete errcode;
    return SUCCESS;
}

int Application::w2l_image_sync_error(LogicEvent* event)
{
    LOG_WARNING("here comes image_sync_error");
    
    int* errcode;
    ImageInfo local_image_info;
    int c_disk_size = 0;
    int d_disk_size = 0;
    int max_c_disk_size = 0;

    errcode = static_cast<int*>(event->_data);
    ASSERT(errcode);
    LOG_DEBUG("errcode = 0x%x", *errcode);

    /* 3 cases should not show image sync error:
            1. local has no base
            2. download bind user's image
            3. new deploy status
        other cases can ignore image sync error
       */


    local_image_info = _vm->vm_get_vm_imageinfo();
    if(_vm->vm_check_local_base_exist() == 1 && !local_image_info.name.empty() && _logined==false && _status_machine.get_status()!= STATUS_NEW_DEPLOY)
    {
        LOG_WARNING("local base exist, ignore image sync error!");

        if(_mode_data.get().mode == SPECIAL_MODE && _mode_data.get().bind_user.username.empty())
        {
            LOG_DEBUG("SPECIAL_MODE and not bind and we already have a image, we do not need to show pop dialog");
        }
        else if(_vm->vm_check_personal_img_exist())
        {
            LOG_DEBUG("if has personal img, do not need to show pop dialog");
        }
        else if(_ui_locking == false)
        {
            //if (*errcode == 11)
            //{
            //    UIPopEvent sync_event("bad driver");
            //    l2u_show_bad_driver_tips((void *)Application::uipop_sync ,(void *)&sync_event);
            //    sync_event.wait();
            //}
            //else
            //{
            //    UIPopEvent sync_event("img abnormal");
            //    l2u_show_image_abnormal_tips((void *)Application::uipop_sync ,(void *)&sync_event);
            //    sync_event.wait();
            //}
            switch (*errcode) {
            case 19:
            {
                UIPopEvent sync_event("nonsupport window 10 32");
                l2u_show_nonsupportwin10_32_tips((void *)Application::uipop_sync ,(void *)&sync_event);
                sync_event.wait();
            }
                break;
            case 20:
            {
                UIPopEvent sync_event("nonsupport window xp");
                l2u_show_nonsupportxp_tips((void *)Application::uipop_sync ,(void *)&sync_event);
                sync_event.wait();
            }
                break;
            default:
                break;
            }
        }

        //upload local image info to web
        c_disk_size = local_image_info.virt_size;
    	_vm->cal_d_disk_size(local_image_info.real_size, local_image_info.virt_size, &d_disk_size);
        LOG_INFO("image sync error, local C disk %d, D disk %d\n", c_disk_size, d_disk_size);
    
        if (_dev_policy.allow_userdisk() == true) {
            max_c_disk_size = _vm->cal_max_c_disk_size(local_image_info.real_size, c_disk_size, d_disk_size);
        	_mina->mina_l2w_notify_partition(c_disk_size, d_disk_size, max_c_disk_size);
        } else {
            max_c_disk_size = _vm->cal_max_c_disk_size(local_image_info.real_size, c_disk_size, 0);
        	_mina->mina_l2w_notify_partition(c_disk_size, 0, max_c_disk_size);
        }

        _status_machine.change_status(EVENT_IMAGE_LATEST);
    }
    else
    {
        _error_tips = true;
        _new_deploy_processing = false;
        if (_ui_locking == false) {
            switch (*errcode) {
            case -5:
                l2u_show_tips(UI_TIPS_IMG_BIG_OPT);
                break;
            case 11:
                l2u_show_image_err_tips(UI_TIPS_IMG_BAD_DRIVER_OPT, *errcode);
                break;
            case 7:
                l2u_show_image_err_tips(UI_TIPS_IMG_USER_ERR_OPT, *errcode);
                break;
            case 4:
                l2u_show_image_err_tips(UI_TIPS_IMG_NOT_FOUND_OPT, *errcode);
                break;
            case 18:
                l2u_show_image_err_tips(UI_TIPS_IMG_VERSION_GT_OUTDATED_OPT, *errcode);
                break;
            case 19:
                l2u_show_image_err_tips(UI_TIPS_DEV_NONSPUPPORT_OSTYPE32_OPT, *errcode);
                break;
            case 20:
                l2u_show_image_err_tips(UI_TIPS_DEV_NONSPUPPORT_OSTYPEXP_OPT, *errcode);
                break;
            default:
                l2u_show_image_err_tips(UI_TIPS_IMG_ABNORMAL_OPT, *errcode);
                break;
            }
        }

        //upload local image info to web
        c_disk_size = local_image_info.virt_size;
        _vm->cal_d_disk_size(local_image_info.real_size, local_image_info.virt_size, &d_disk_size);
        LOG_INFO("image sync error, local C disk %d, D disk %d\n", c_disk_size, d_disk_size);
    
        if (_dev_policy.allow_userdisk() == true) {
            max_c_disk_size = _vm->cal_max_c_disk_size(local_image_info.real_size, c_disk_size, d_disk_size);
        	_mina->mina_l2w_notify_partition(c_disk_size, d_disk_size, max_c_disk_size);
        } else {
            max_c_disk_size = _vm->cal_max_c_disk_size(local_image_info.real_size, c_disk_size, 0);
        	_mina->mina_l2w_notify_partition(c_disk_size, 0, max_c_disk_size);
        }
        
        // prevent disconnect with web
        //_status_machine.change_status(EVENT_IMAGE_SYNC_ERROR);
    }
    
    delete errcode;
    return SUCCESS;
}


int Application::w2l_login_success(LogicEvent* event)
{
     UserInfo* server_info;
     LOG_INFO("w2l_login_success");
     server_info = static_cast<UserInfo*>(event->_data);
     ASSERT(server_info);
     if(event->_error != SUCCESS && event->_error != 98)
     {
         return event->_error;
     }

    if (event->_error == 98)
    {
        LOG_WARNING("netdisk err!");
    }

    if ("123456" == gloox::password_codec_xor(_input_user_info.password, false))
    {
        LOG_ERR("password is default passwd!");
        l2u_result_user(7);
        _status_machine.change_status(EVENT_LOGIN_FAILED);
        delete server_info;
        return SUCCESS;
    }

    _logined_user_info = *server_info;
    _logined_user_info.password = _input_user_info.password;
    _logined_user_info.remember_flag = _input_user_info.remember_flag;
    _logined = true;

    //save userinfo in local file
    _userinfomgr.UserLoginedInfoStore(_mode_data.get().mode, _logined_user_info);
    _userinfomgr.setUserLogined(_logined);
    _userinfomgr.saveLoginedUser(_logined_user_info.username);
    _userinfomgr.saveLastLoginedUser(_input_user_info.username, _input_user_info.password,
        _input_user_info.remember_flag);
    _userinfomgr.set_vm_desktop_redir(_logined_user_info.desktop_redir);
    
    if(_mode_data.get().mode == SPECIAL_MODE && _mode_data.get().bind_user.username.empty())
    {
        LOG_DEBUG("unbind, bind user?");
        l2u_show_userbind();
        //_mina->mina_web_bind(_logined_user_info);
    }
    else
    {
        _status_machine.change_status(EVENT_LOGIN_SUCCESS);
    }
    delete server_info;
    return SUCCESS;
}

int Application::w2l_login_fail(LogicEvent* event)
{
    int* errcode;
    errcode = static_cast<int*>(event->_data);
    ASSERT(errcode);
    LOG_DEBUG("%d", errcode);

    if (event->_error != SUCCESS) {
        return event->_error;
    }

    if (*errcode == 16) {
        LOG_ERR("user do not allow login!");
        l2u_result_user(4);
    } else if (*errcode == 23) {
        LOG_ERR("ad domain not exist!");
        l2u_result_user(20);
    } else if (*errcode == 24) {
        LOG_ERR("ad user disable!");
        l2u_result_user(21);
    } else if (*errcode == 25) {
        LOG_ERR("ad user expire!");
        l2u_result_user(22);
    } else if (*errcode == 26) {
        LOG_ERR("ad user not auth right now!");
        l2u_result_user(23);
    } else if (*errcode == 28) {
        LOG_ERR("ldap user login fail!");
        l2u_result_user(24);
    } else if (*errcode == 17) {
        l2u_result_user(200);
    } else if (*errcode == -2) {
        LOG_ERR("login timeout");
        l2u_result_user(99);        
    } else {
        //login fail
        LOG_DEBUG("login fail! call l2u_result_user(1)");
        l2u_result_user(1);
    }

    delete errcode;
    //LOG_DEBUG("call l2u_result_user(false)");
    //l2u_result_user(!SUCCESS);// !0 indicate login fail
    _status_machine.change_status(EVENT_LOGIN_FAILED);
    return SUCCESS;
}

int Application::w2l_modify_password_success(LogicEvent* event)
{
    struct UserInfo userinfo;
    int ret;

    if (event->_error != SUCCESS) {
        return event->_error;
    }
    ret = _userinfomgr.UserInfoRead(_input_user_info.username, &userinfo);
    if (ret == 0) {
        userinfo.password = _input_user_info.new_password;
        _userinfomgr.UserInfoWrite(userinfo);
    }
    l2u_result_user(102);
    sleep(2);
    ui_show_login();
    return SUCCESS;
}

int Application::w2l_modify_password_fail(LogicEvent* event)
{
    int* errcode;
    errcode = static_cast<int*>(event->_data);
    ASSERT(errcode);
    LOG_DEBUG("%d", errcode);
    
    if(event->_error != SUCCESS)
    {
        return event->_error;
    }

    if(*errcode == 20)
    {
        //modify passwd err for ad domain
        l2u_result_user(108);
    }
    else if(*errcode == 27)
    {
        //modify passwd err for ldap user
        l2u_result_user(109);
    }
    else
    {
        //FIXME: return error code to UI?
        l2u_result_user(101);
    }

    delete errcode;
    return SUCCESS;
}

int Application::w2l_bind_seccess(LogicEvent* event)
{
    UserInfo* info;
    ModeInfo mode;
    LOG_INFO("w2l_bind_seccess");
    info = static_cast<UserInfo*>(event->_data);
    ASSERT(info);
    if(event->_error != SUCCESS)
    {
        delete info;
        return event->_error;
    }

	_mina->mina_web_request_dev_policy(SPECIAL_MODE, info->username);
	_dev_policy.wait4update(2*1000*1000*1000);
/*
    if(info->username != _input_user_info.username)
    {
        LOG_ERR("server's username is different from input usrname");
        //ui_show_error
        delete info;
        return -1;
    }
*/
    if(_status_machine.get_status() == STATUS_NEW_DEPLOY)
    {
    	_mina->mina_web_request_image(_vm->vm_get_vm_imageinfo());
    }
    else
    {
    	_status_machine.change_status(EVENT_LOGIN_SUCCESS);
    }

    // WARNNING: set bind_user after status change, for we change status according to whether bind_user is empty 
    mode.mode = SPECIAL_MODE;
    mode.bind_user.username = info->username;
    _mode_data.set(mode);
    
    delete info;

    return SUCCESS;
}

int Application::w2l_bind_fail(LogicEvent* event)
{
    int* errcode;
    errcode = static_cast<int*>(event->_data);
    ASSERT(errcode);
    LOG_DEBUG("%d", *errcode);
    if(event->_error != SUCCESS)
    {
        return event->_error;
    }

    if(_status_machine.get_status() == STATUS_NEW_DEPLOY)
    {
        _new_deploy_processing = false;
        if(*errcode == 7)
        {
            //user is not exist
            l2u_result_settype(1);
        }
        if(*errcode == 6)
        {
            //user is binded by other terminal
            l2u_result_settype(2);
        }
        if(*errcode == 9)
        {
            //terminal is binded by other user
            l2u_result_settype(3);
        }
    }else{
        LOG_ERR("user bind error! errcode=%d", *errcode);
        if (*errcode == 6)
        {
            //user is binded by other terminal
            ui_show_login();
            l2u_result_user(12);
        }
        else
        {
            //user bind error
            ui_show_login();
            l2u_result_user(3);
        }
        _status_machine.change_status(EVENT_LOGIN_FAILED);
    }
    delete errcode;
    return SUCCESS;
}



int Application::w2l_notify_check_vm_status(LogicEvent* event)
{
    LOG_DEBUG(" ");
    if(event->_error != SUCCESS)
    {
        return event->_error;
    }
    _vm->vm_get_vm_info();
    return SUCCESS;
}


int Application::w2l_notify_shutdown(LogicEvent* event)
{
    LOG_INFO("%s enter", __func__);

    _shutdown_flag = TERMINAL_SHUTDOWN;
    if(_vm->vm_is_vm_running())
    {
        //if vm is running, should shutdown vm first
        _vm->vm_shutdown_VM_normal();
    }
    else
    {
        rc_system("shutdown -h now");
    }
    
    return SUCCESS;
}

int Application::w2l_notify_reboot(LogicEvent* event)
{
    LOG_INFO("%s enter", __func__);

    _shutdown_flag = TERMINAL_REBOOT;
    if(_vm->vm_is_vm_running())
    {
        if(is_installing_driver())
        {
            rc_system("reboot");
        }
        //if vm is running, should shutdown vm first
        _vm->vm_shutdown_VM_normal();
    }
    else
    {
        rc_system("reboot");
    }

    return SUCCESS;
}

int Application::w2l_notify_recorver_image(LogicEvent* event)
{
    LOG_INFO("w2l_notify_recorver_image");
    if(event->_error != SUCCESS)
    {
        return event->_error;
    }
    
    _vm->vm_clear_inst();
    _vm->vm_clear_layer();
    l2u_show_tips(UI_TIPS_RECOVERY_IMAGE_OPT);
    l2u_ctrl_winbtn(false);
    l2u_show_wifi_btnbox(2, -1, -1);

    if (_net_auth->getAuthExist() == 0) {
        l2u_show_net_auth_winbtn(UI_AUTHBTN_DISABLED);
    }

    return SUCCESS;
}

int Application::w2l_notify_modify_password(LogicEvent* event)
{
    struct UserInfo *buf;
    struct UserInfo userinfo;
    string bind_username;
    int ret;

    LOG_INFO("enter w2l_notify_modify_password");
    buf = static_cast<UserInfo*>(event->_data);
    ASSERT(buf);
    if (event->_error != SUCCESS) {
        delete buf;
        return event->_error;
    }
    if (_mode_data.get().mode != SPECIAL_MODE) {
        LOG_ERR("now terminal mode is not SPECIAL_MODE, so ignore w2l_notify_modify_password");
        delete buf;
        return -1;
    }
    bind_username = _mode_data.get().bind_user.username;
    if (bind_username != buf->username) {
        LOG_ERR("now terminal bind username is not suited, so ignore w2l_notify_modify_password");
        delete buf;
        return -1;
    }

    // update local userinfo db.
    ret = _userinfomgr.UserInfoRead(bind_username, &userinfo);
    if (ret == 0) {
        userinfo.password = buf->new_password;
        _userinfomgr.UserInfoWrite(userinfo);
    }
    delete buf;
    return SUCCESS;
}

int Application::w2l_notify_modify_local_network(LogicEvent* event)
{
    NetworkInfo* info;
    LOG_INFO("w2l_notify_modify_local_network");
    info = static_cast<NetworkInfo*>(event->_data);
    ASSERT(info);
    if(event->_error != SUCCESS)
    {
        delete info;
        return event->_error;
    }
    if(*info != _local_network_data.get())
    {
        _local_network_data.set(*info);
    }
    delete info;
    return SUCCESS;
}

int Application::w2l_notify_modify_vm_network(LogicEvent* event)
{
    NetworkInfo* info;
    LOG_INFO("w2l_notify_modify_vm_network");
    info = static_cast<NetworkInfo*>(event->_data);
    ASSERT(info);
    if(event->_error != SUCCESS)
    {
        delete info;
        return event->_error;
    }
    _vm->vm_set_vm_netinfo(info, _status_machine.get_status());
    delete info;
    return SUCCESS;
}

int Application::w2l_notify_collect_log(LogicEvent* event)
{
    //TODO:collect_log
    FtpLogInfo *info;
    if(event->_error != SUCCESS)
    {
        return event->_error;
    }

    info = static_cast<FtpLogInfo*>(event->_data);
    ASSERT(info);
    // TODO: collect VM log
    UnjoinableThread collect_log_thread(collect_log, info);
    //delete info do in collect_log
    return SUCCESS;
}
int Application::w2l_notify_modify_hostname(LogicEvent* event)
{
    string* hostname;
    hostname = static_cast<string*>(event->_data);
    ASSERT(hostname);
    if(event->_error != SUCCESS)
    {
        delete hostname;
        return event->_error;
    }
    _hostname_data.set_hostname_info(*hostname);
    LOG_INFO("server set hostname:%s",_hostname_data.get_hostname_info().c_str());
    delete hostname;

    return SUCCESS;
}

int Application::w2l_notify_reset_to_initial(LogicEvent* event)
{
    if(event->_error != SUCCESS)
    {
        return event->_error;
    }
    LOG_INFO("enter reset to initial, _ui_locking!");
    _wait_tips_ok = true;
    _ui_locking = true;
    //show UI 
    l2u_show_tips(UI_TIPS_CLIENT_INIT_OPT);
    l2u_ctrl_winbtn(false);
    l2u_show_wifi_btnbox(2, -1, -1);

    if (_net_auth->getAuthExist() == 0) {
        l2u_show_net_auth_winbtn(UI_AUTHBTN_DISABLED);
    }


    return SUCCESS;
}
/*
int Application::w2l_notify_reload_image(LogicEvent* event)
{
    LOG_INFO("w2l_notify_recorver_image");
    if(event->_error != SUCCESS)
    {
        return event->_error;
    }
    
    l2u_show_tips();
    l2u_ctrl_winbtn(false);
    
    return SUCCESS;
}
*/
int Application::w2l_notify_delete_teacherdisk(LogicEvent* event)
{
    LOG_INFO("w2l_notify_delete_teacherdisk");
    if(event->_error != SUCCESS)
    {
        return event->_error;
    }
    
    int saved_disk_size = _vm->vm_get_user_disk_size();
    _vm->vm_clear_teacher_disk();
    if(!_vm->vm_create_user_disk(saved_disk_size))
    {
        LOG_ERR("vm_create_user_disk failed, saved size = %d", saved_disk_size);
        LOG_NOTICE("you might get a error size of datadisk");
    }

    l2u_show_tips(UI_TIPS_DELETE_TEACHERDISK_OPT);
    l2u_ctrl_winbtn(false);
    l2u_show_wifi_btnbox(2, -1, -1);

    if (_net_auth->getAuthExist() == 0) {
        l2u_show_net_auth_winbtn(UI_AUTHBTN_DISABLED);
    }
    return SUCCESS;
}

int Application::w2l_notify_http_port(LogicEvent* event)
{
    HttpPortInfo* hpi;
    hpi = static_cast<HttpPortInfo*>(event->_data);
    ASSERT(hpi);
    if(event->_error != SUCCESS)
    {
        delete hpi;
        return event->_error;
    }
    LOG_DEBUG("pub_ip:%s, pub_port:%d, pri_ip:%s, pri_port:%d, bt ports range:%s.",
        hpi->public_ip.c_str(), hpi->public_port,
        hpi->private_ip.c_str(), hpi->private_port, hpi->btPortsRange.c_str());

    _userinfomgr.set_http_port_map(*hpi);
    _local_network_data.set_port_convert();
    _userinfomgr.set_http_last_port_map(*hpi);
    delete hpi;

    return SUCCESS;
}

int Application::w2l_notify_resync(LogicEvent* event)
{
    int* action = static_cast<int*>(event->_data);
    ASSERT(action);
    if(event->_error != SUCCESS)
    {
        delete action;
        return event->_error;
    }
    LOG_INFO("enter web notify resync, action=%d", *action);
    _wait_tips_ok = true;
    _ui_locking = true;
    if(_status_machine.get_status() == STATUS_DOWNLOADING_IMAGE)
    {
        _vm->vm_stop_copy_usb_image();
        _vm->vm_quit_download_image();
        _process_loop.erase_event(V2L_VM_DOWNLOAD_PROGRESS_STATUS);
    }

    switch(*action)
    {
        /*
            0: unbind
            1: bind
            2: change mode
            */
        case 0:
        case 1:
            l2u_show_tips(UI_TIPS_DEV_USER_CHANGE_OPT);
            break;
        case 2:
            l2u_show_tips(UI_TIPS_DEV_MODE_CHANGE_OPT);
            break;
        default:
            LOG_WARNING("unknown action:%d!", *action);
            break;
    }

    //disable other buttons
    l2u_ctrl_winbtn(false);
    l2u_show_wifi_btnbox(2, -1, -1);

    if (_net_auth->getAuthExist() == 0) {
        l2u_show_net_auth_winbtn(UI_AUTHBTN_DISABLED);
    }
    delete action;
    return SUCCESS;
}

int Application::w2l_notify_do_ipxe(LogicEvent* event)
{
    //string* iso_name;
    IpxeInfo* info;
    bool allow_ipxe = true;
    char run_pxe_cmd[512];
    int ret;

    info = static_cast<IpxeInfo*>(event->_data);
    ASSERT(info);
    
    if(event->_error != SUCCESS)
    {
        delete info;
        return event->_error;
    }

    //if (is_dev_wlan_up())
    //{
    //    LOG_ERR("should not do ipxe if wlan up!");
    //    delete info;
    //    return ERROR_IPXE_WLAN_UP;
    //}
     if(_vm->vm_is_vm_running())
    {
        //if vm is running, should shutdown vm first
        rc_system("vmmanager --normal_shutdown");
    }
     
    LOG_INFO("begin do ipxe");
    if (info->iso_version.empty()) {
        allow_ipxe = true;
    } else {
        allow_ipxe = _updata_policy.allow_version(info->iso_version);
    }

    //chenw debug
    if (allow_ipxe) {
        ::save_ipxe_configure();
        sprintf(run_pxe_cmd,"sh /usr/cz_prep.sh %s %d %s %s 2 %s l"\
            , _local_network_data.get().ip.c_str()\
            , get_submask_bitcount(_local_network_data.get().submask.c_str())\
            , _local_network_data.get().gateway.c_str()\
            , _mina->mina_get_server_ip().c_str()\
            , info->iso_name.c_str());
        LOG_DEBUG(run_pxe_cmd);
        rc_system(run_pxe_cmd);
 
        //LOG_DEBUG("!!!!!!  upgrade ok");
        ret = SUCCESS;
    } else {
        UIPopEvent sync_event("iso upgrade err");
        l2u_show_iso_upgrade_err((void *)Application::uipop_sync ,(void *)&sync_event);
        sync_event.wait();
        ret = ERROR_ISO_VERSION_ERR;
    }
 
    delete info;
    return ret;

}

int Application::w2l_notify_ssid_whitelist(LogicEvent *event)
{
    vector<string> *whitelist;

    whitelist = static_cast<vector<string> *>(event->_data);
    ASSERT(whitelist);

    if(event->_error != SUCCESS)
    {
        delete whitelist;
        return event->_error;
    }

    LOG_DEBUG("enter");
    if (is_wifi_terminal() == true) {
        _userinfomgr.deletessidWhite();
        _userinfomgr.storessidWhiteList(*whitelist);
        l2u_show_not_whitessid_show(*whitelist);
        _wifi_interactive->wifi_forget_not_whitessid_list(*whitelist);
    }
    delete whitelist;
    return SUCCESS;
}

int Application::w2l_notify_reset_netdisk(LogicEvent *event)
{
    NetdiskInfo* info;
    info = static_cast<NetdiskInfo*>(event->_data);
    ASSERT(info);
    if(event->_error != SUCCESS)
    {
        delete info;
        return event->_error;
    }
    LOG_DEBUG("enter");
    if(_status_machine.get_status() == STATUS_RUNNING_VM) {
        _vm->vm_set_vm_diskinfo(*info);
    }
    delete info;

    return SUCCESS;
}

int Application::w2l_recv_guesttool_msg(LogicEvent *event)
{
    string *guesttool_msg;
    
    guesttool_msg = static_cast<string *>(event->_data);
    ASSERT(guesttool_msg);
    if (event->_error != SUCCESS) {
        delete guesttool_msg;
        return event->_error;
    }

    LOG_DEBUG("begin send guesttool msg");
    _vm->vm_send_web_info(*guesttool_msg);

    delete guesttool_msg;
    return SUCCESS;
}

void* Application::sync_time_thread(void* data)
{
    //sync server's time
    Application* app = Application::get_application();
    char ntp_command[128];
    sprintf(ntp_command, "ntpdate %s", app->_mina->mina_get_server_ip().c_str());
    rc_system(ntp_command);
    rc_system("hwclock --systohc");
    app->lm_web_update_onLineTime();
    return NULL;
}

int Application::w2l_mina_connection_established(LogicEvent* event)
{
    /***************************************************
    we won't try reconnecting to web if we enter local mode in v0.9
    BUT if we need try reconnecting to web, 
    the action here will be seperated by STATUS_CHECKING_LOCAL 
    ***************************************************/
    /***************************************************
    if(_status_machine.get_status() == STATUS_CHECKING_LOCAL)
    {
        //_status_machine();
        //jump to STATUS_INITING
        return 0;
    }
    ***************************************************/

    if(event->_error != SUCCESS)
    {
        return event->_error;
    }
    UnjoinableThread sync_time(sync_time_thread, NULL);
    _mina->mina_web_check_server_type();
    return SUCCESS;
}


int Application::w2l_mina_connection_destroyed(LogicEvent* event)
{
    /**************************************************
    we won't judge whether _status_machine, for we judge the status in _status_machine
    **************************************************/
    if(event->_error != SUCCESS)
    {
        return event->_error;
    }
    _error_tips = false;
    if (!_userinfomgr.on_local_mode() && !_vm->vm_is_usb_downloading()) {
        // on local mode, we do not change the state to init.
        // on usb downloading, we do not change the state to init.
        if (_status_machine.get_status() == STATUS_NEW_DEPLOY)
        {
            _newdeploy_manage->reset_status();
        }
        _status_machine.reset_status();
        _status_machine.change_status(EVENT_WEB_DISCONNECT);
    }
    return 0;
}


//zhf
//v2l api for vm
int Application::v2l_image_exist_result(LogicEvent* event){
	int result = event->getusrdatasize();
	if (STATUS_CHECKING_LOCAL == _status_machine.get_status()) {
		if (NULL == localmode){
			LOG_ERR("invalid localmode obj");
			return ERROR_PROCESSING;
		}
		LocalImageStatus ret;
		ret = (LocalImageStatus) result;
		LOG_DEBUG("rcv vm localimage ack:%d", ret);
        
		if (0 != localmode->VM_Ack_LocalImageStatus(ret)) {
			LOG_INFO("local image does not exist or status error\n");
            _userinfomgr.leave_local_mode();
			_status_machine.change_status(EVENT_LOCAL_CHECK_FAILED);
		}
		return SUCCESS;
	}else {
	    //todo:
		LOG_DEBUG("v2l image status ack");
		return ERROR_PROCESSING;
	}
}

//only used for uploading vm running info when vm start/shutdown/reboot
void Application::__upload_vm_info(bool is_vm_running)
{
    struct VMInfo vm_info = {
        .running = is_vm_running,
    };

    LOG_DEBUG("enter __upload_vm_info: running %d", vm_info.running);
    _mina->mina_web_upload_vm_info(vm_info, _logined_user_info);
}

int Application::v2l_vm_start_result(LogicEvent* event)
{
    int result = event->getusrdatasize();

    LOG_DEBUG("enter");
    LOG_INFO("V2L: VM started %s!", (result == START_SUCCESS ? "successfully" : "failed"));

    if (result == START_FAIL) {
        if (is_installing_driver()) {
           handle_install_driver_on_vm_fail();
        } else {
             rc_system("rm -f /tmp/user_status.ini");

             if (_vm->vm_is_xserver_exited()) {
                // X server exited, we could only reboot system.
                handle_xserver_exited_on_vm_fail();
                // Never return here, for system already reboot.
             } else {
                // X server still alived, just show error.
                handle_xserver_alive_on_vm_fail();
             }
        }
    } else {
        handle_on_vm_start_success();
    }
    return SUCCESS;
}

int Application::v2l_vm_shutdown_result(LogicEvent* event)
{
    char command_buf[128];
	int result = event->getusrdatasize();
    int ret = 0;

    LOG_INFO("--------- v2l_shutdown_result:%d ---------", result);

    if (result == SHUTDOWN_SUCCESS) {
       handle_on_vm_stop_success();
    }

    if((is_installing_driver()) && (_shutdown_flag == 0))
    {
        sprintf(command_buf, "umount -fl %s", rc_json_get_string(_driver_install_parament, "path_name").c_str());
        rc_system(command_buf);
        sprintf(command_buf, "umount -fl %s", rc_json_get_string(_driver_install_parament, "driver_path_name").c_str());
        rc_system(command_buf);
        if((result == SHUTDOWN_SUCCESS))
        {
            _mina->mina_web_upload_driver_install_result(SUCCESS);
            LOG_INFO("driver_install SUCCESS!!");
        }
        else
        {
            _mina->mina_web_upload_driver_install_result(ERROR_DRIVER_INSTALL_FAIL);
            LOG_ERR("driver_install fail!!! shutdown result = %d", result);
        }
        sleep(3);
        rc_system("reboot");
    }

    // if VM shutdown successfully, system will shutdown.
    // FIXME: if kernel supports graphic devices unbind, system need not shutdown here.
    if (result == SHUTDOWN_SUCCESS) 
    {
        /*
                0: user shutdown VM
                TERMINAL_SHUTDOWN: web shutdown terminal
            */
        if(_shutdown_flag==TERMINAL_SHUTDOWN || _shutdown_flag==0)
        {
            LOG_NOTICE("VM shutdown successfully, system will shutdown soon.\n");
            rc_system("sync");
            sleep(1);
            ret = rc_system("shutdown -h now");
            if (ret != 0) 
            {
                LOG_ALERT("System shutdown failed!\n");
                return ERROR_PROCESSING;
            }
        }
        else if(_shutdown_flag == TERMINAL_REBOOT)
        {
            LOG_NOTICE("VM shutdown successfully, system will reboot soon.\n");
            ret = rc_system("reboot");
            if (ret != 0) 
            {
                LOG_ALERT("System reboot failed!\n");
                return ERROR_PROCESSING;
            }
        }
        else if(_shutdown_flag == TERMINAL_LOGOUT)
        {
            LOG_NOTICE("VM shutdown successfully, system will switchuser.\n");
            //FIXME:replace with back_to_client.sh
            ret = rc_system("/usr/bin/back_to_client.sh");
            if (_vm->get_vmManage()->get_start_vm_is_emulation()) {
                ret = rc_system("/etc/RCC_Client/scripts/back_to_client_callback.sh");
            }
        }
        // Never return here.
    } 
    else 
    {
        _shutdown_flag = 0;
        LOG_CRIT("VM shutdown failed! Please check your image!\n");
    }
    
    return SUCCESS;
}

int Application::v2l_vm_reboot_result(LogicEvent* event)
{
	int result = event->getusrdatasize();
	LOG_INFO("--------- v2l_vm_reboot_result:%d ---------", result);

    if (result == REBOOT_SUCCESS) {
        // set guesttool as disconnected, for re-forwarding VM messages
        _vm->vm_set_guest_disconnected();
    }

    //TODO: VM & device reboot linkages?
	return SUCCESS;
}


int Application::v2l_vm_download_progress_status(LogicEvent* event)
{
    DownloadInfo* dlinfo;
    int status;
    static int downloading_count = 0;//send web downloading status every 20s
        
    LOG_DEBUG("%s enter", __func__);
    dlinfo = static_cast<DownloadInfo*>(event->getusrdata());
    status = _status_machine.get_status();
    if(status != STATUS_DOWNLOADING_IMAGE && status != STATUS_NEW_DEPLOY)//we allow download img in new deply status
    {
        LOG_ERR("error! state:%d, event:%s!", status, EVENT_TO_STR(V2L_VM_DOWNLOAD_PROGRESS_STATUS));
    	if(dlinfo != NULL){
    		delete dlinfo;
    	}
        return ERROR_EVENT;
    }

    if(dlinfo != NULL)
    {
    	//todo ui will add error msg
//    	char buf[0x100];
    	l2u_download_info.status = 0;
    	l2u_download_info.err_code = UI_DIALOG_DOWNLOAD_ERR_UNKNOWN;
    	l2u_download_info.speed = NULL;
    	l2u_download_info.process = 0;
        l2u_download_info.filesize = 0;
        l2u_download_info.downloaded = 0;
        l2u_download_info.remain_sec = 0;
        if(dlinfo->status == DOWNLOAD_IMAGE_ERROR)
        {
        	l2u_download_info.status = UI_DIALOG_DOWNLOAD_ST_ERROR;
            LOG_ERR("download image error!");
            //wait 30s and retry
            _process_loop.activate_interval_timer(_redownload_timer, 30* 1000);
            l2u_show_download(&l2u_download_info);
        }
        else if(dlinfo->status == DOWNLOAD_IMAGE_DOWNLOADING)
        {
        	l2u_download_info.status = UI_DIALOG_DOWNLOAD_ST_NORMAL;
        	l2u_download_info.speed = (char *)dlinfo->speed.c_str();
        	l2u_download_info.process = dlinfo->percent_double;
            LOG_INFO("download image progress:%s, speed:%s", dlinfo->percent.c_str(), dlinfo->speed.c_str());
            l2u_show_download(&l2u_download_info);
        }
        else if(dlinfo->status == DOWNLOAD_IMAGE_CHECKING)
        {
        	l2u_download_info.status = UI_DIALOG_DOWNLOAD_ST_CHECKING;
        	l2u_download_info.speed = NULL;
        	l2u_download_info.process = dlinfo->percent_double;
            LOG_INFO("chenking image progress:%s, speed:%s", dlinfo->percent.c_str(), dlinfo->speed.c_str());
            l2u_show_download(&l2u_download_info);
        }
        else if(dlinfo->status == DOWNLOAD_IMAGE_MERGE)
        {
        	l2u_download_info.status = UI_DIALOG_DOWNLOAD_ST_MERGE;
        	l2u_download_info.speed = NULL;
        	l2u_download_info.process = dlinfo->percent_double;
            LOG_INFO("Merge image progress:%s", dlinfo->percent.c_str());
            l2u_show_download(&l2u_download_info);
        }
        
        else if(dlinfo->status == DOWNLOAD_IMAGE_SUCCESS)
        {
        	l2u_download_info.status = UI_DIALOG_DOWNLOAD_ST_NORMAL;
        	l2u_download_info.speed = (char *)dlinfo->speed.c_str();
        	l2u_download_info.process = dlinfo->percent_double;
            LOG_INFO("download image success!");
			_download_retry = false;

			if(_status_machine.get_status() == STATUS_NEW_DEPLOY)
			{
                _new_deploy_processing = false;
                _input_user_info.username.clear();
                _input_user_info.password.clear();
                _newdeploy_manage->enter_newdeploy_finish();
			}else{
				_status_machine.change_status(EVENT_DOWNLOAD_IMAGE_SUCCEESS);
			}
        }
        else if(dlinfo->status == DOWNLOAD_IMAGE_CHECKING_MD5)
        {
        	l2u_download_info.status = UI_DIALOG_DOWNLOAD_ST_NORMAL;
        	l2u_download_info.speed = NULL;
        	l2u_download_info.process = dlinfo->percent_double;
            LOG_INFO("download image checking, cal md5");
            l2u_show_imageupdating();
        }
        else if(dlinfo->status == DOWNLOAD_IMAGE_NOSPACE)
        {
        	l2u_download_info.status = UI_DIALOG_DOWNLOAD_ST_ERROR;
        	l2u_download_info.err_code = UI_DIALOG_DOWNLOAD_ERR_NOSPACE;
        	l2u_download_info.speed = NULL;
        	l2u_download_info.process = dlinfo->percent_double;
            LOG_INFO("download image no space");
            l2u_show_download(&l2u_download_info);
        }
        else if(dlinfo->status == DOWNLOAD_IMAGE_TIMEOUT)
        {
        	l2u_download_info.status = UI_DIALOG_DOWNLOAD_ST_ERROR;
        	l2u_download_info.err_code = UI_DIALOG_DOWNLOAD_ERR_TIMEOUT;
        	l2u_download_info.speed = NULL;
        	l2u_download_info.process = dlinfo->percent_double;
            LOG_INFO("download image timeout");
            l2u_show_download(&l2u_download_info);
        }
        else if(dlinfo->status == DOWNLOAD_IMAGE_CHECKED_MD5_FAIL)
        {
        	l2u_download_info.status = UI_DIALOG_DOWNLOAD_ST_ERROR;
        	l2u_download_info.err_code = UI_DIALOG_DOWNLOAD_ERR_MD5;
        	l2u_download_info.speed = NULL;
        	l2u_download_info.process = dlinfo->percent_double;
            LOG_INFO("download image md5 error");
            l2u_show_download(&l2u_download_info);
        }
        else if(dlinfo->status == DOWNLOAD_COPING_USB_IMAGE)
        {
        	l2u_download_info.status = UI_DIALOG_CP_BASE_ST_NORMAL;
        	l2u_download_info.speed = (char *)dlinfo->speed.c_str();
        	l2u_download_info.process = dlinfo->percent_double;
            LOG_INFO("copy usb image progress:%s, speed:%s", dlinfo->percent.c_str(), dlinfo->speed.c_str());
            l2u_show_usb_copy(&l2u_download_info);
        }
        else if(dlinfo->status == DOWNLOAD_CP_BASE_MOUNTED_DEV_NOT_EXIST)
        {
        	l2u_download_info.status = UI_DIALOG_CP_BASE_ST_ERROR;
        	l2u_download_info.err_code = UI_DIALOG_CP_BASE_MOUNTED_DEV_NOT_EXIST;
        	l2u_download_info.speed = NULL;
        	l2u_download_info.process = dlinfo->percent_double;
            LOG_INFO("copy usb image dev not exist");
            l2u_show_usb_copy(&l2u_download_info);
        }
        else if(dlinfo->status == DOWNLOAD_CP_BASE_UPDATE_STATUS_ERROR)
        {
        	l2u_download_info.status = UI_DIALOG_CP_BASE_ST_ERROR;
        	l2u_download_info.err_code = UI_DIALOG_CP_BASE_UPDATE_STATUS_ERROR;
        	l2u_download_info.speed = NULL;
        	l2u_download_info.process = dlinfo->percent_double;
            LOG_INFO("copy usb image update status error");
            l2u_show_usb_copy(&l2u_download_info);
        }
        else if(dlinfo->status == DOWNLOAD_CP_BASE_UPDATE_STATUS_BASE_READY)
        {
            l2u_download_info.status = UI_DIALOG_CP_BASE_ST_NORMAL;
        	l2u_download_info.speed = NULL;
        	l2u_download_info.process = 100;
            LOG_INFO("dlinfo->status == DOWNLOAD_CP_BASE_UPDATE_STATUS_BASE_READY");
            l2u_show_usb_copy(&l2u_download_info);
        }

        //if dlinfo->status == DOWNLOAD_IMAGE_DOWNLOADING || dlinfo->status == DOWNLOAD_IMAGE_CHECKING
        //we send this msg every 20s
        //to avoid server's overload
        if(dlinfo->status == DOWNLOAD_IMAGE_DOWNLOADING || dlinfo->status == DOWNLOAD_IMAGE_CHECKING || dlinfo->status == DOWNLOAD_COPING_USB_IMAGE)
        {
            if(downloading_count++ >= 20)
            {
                downloading_count = 0;
                _mina->mina_web_upload_download_info(*dlinfo);
            }
        }
        //otherwise the message is more important, we send it immediately
        else
        {
            _mina->mina_web_upload_download_info(*dlinfo);
        }
        delete dlinfo;
    }
    else
    {
        return ERROR_INPUT;
    }
    
    return SUCCESS;
}

int Application::v2l_vm_download_quit_result(LogicEvent* event)
{
    int resulst;
    int status;

    LOG_DEBUG("%s enter", __func__);
    resulst = event->getusrdatasize();
    status = _status_machine.get_status();
    if(status != STATUS_DOWNLOADING_IMAGE && status != STATUS_NEW_DEPLOY)
    {
        LOG_ERR("error! state:%d, event:%s!", status, EVENT_TO_STR(V2L_VM_DOWNLOAD_PROGRESS_STATUS));
        return ERROR_EVENT;
    }
    //todo
    if(resulst == 0)
    {
    	LOG_INFO("download image quit ok");
        //dlinfo->status: <0: error, 0: downloading, >0: success
    }
    else
    {
    	LOG_ERR("download image quit failed");
    }
    //_status_machine.change_status(EVENT_DOWNLOAD_IMAGE_FAILED);
    return SUCCESS;
}

int Application::v2l_get_vm_info_result(LogicEvent* event){
	struct VMInfo *buf;
	buf = static_cast<VMInfo*>(event->getusrdata());
	if (NULL == buf){
		LOG_ERR("v2l data invaled(null pt)");
		return ERROR_INPUT;
	}
    _mina->mina_web_upload_vm_info(*buf, _logined_user_info);
    //todo when other call function vm_get_vm_info_status
	LOG_INFO("--------- %s: ---------", __FUNCTION__);
	delete buf;
	return SUCCESS;
}

int Application::v2l_set_vm_net_policy_result(LogicEvent* event){
	int result = event->getusrdatasize();
	LOG_INFO("--------- %s:%d ---------", __FUNCTION__, result);
	//todo
	return SUCCESS;
}

int Application::v2l_set_vm_usb_policy_result(LogicEvent* event){
	int result = event->getusrdatasize();
	LOG_INFO("--------- %s:%d ---------", __FUNCTION__, result);
	//todo
	return SUCCESS;
}

int Application::v2l_set_vm_netinfo_result(LogicEvent* event){
	int result = event->getusrdatasize();
	LOG_INFO("--------- %s:%d ---------", __FUNCTION__, result);
	//todo
	return SUCCESS;
}

int Application::v2l_get_vm_netinfo_result(LogicEvent* event){
	struct NetworkInfo *buf;
	buf = static_cast<NetworkInfo*>(event->getusrdata());
	if (NULL == buf){
		LOG_ERR("v2l data invaled(null pt)");
		return ERROR_INPUT;
	}
	LOG_INFO("--------- %s: ---------", __FUNCTION__);
	//todo
	delete buf;
	return SUCCESS;
}


int Application::v2l_set_local_wired_network_info(LogicEvent* event)
{
    struct NetworkInfo *buf;
	buf = static_cast<NetworkInfo*>(event->getusrdata());
	if (NULL == buf){
		LOG_ERR("v2l data invaled(null pt)");
		return ERROR_INPUT;
	}
	LOG_INFO("--------- %s: ---------", __FUNCTION__);
	if(*buf != _local_network_data.get())
    {
        _local_network_data.set(*buf);
    }   
	delete buf;
	return SUCCESS;
}

int Application::v2l_set_vm_diskinfo_result(LogicEvent* event){
	int result = event->getusrdatasize();
	LOG_INFO("--------- %s:%d ---------", __FUNCTION__, result);
	//todo
	return SUCCESS;
}

int Application::handle_easy_deploy(LogicEvent* event)
{
    EasyDeploy *easy_deploy;
    easy_deploy = static_cast<EasyDeploy*>(event->getusrdata());
    ASSERT(easy_deploy);

    // notify that we enter easy deploy processing.
    _on_easy_deploy = true;

    //clear ui lock
    _ui_locking = false;

    //step1: set server_ip
    if(!easy_deploy->server_ip.empty())
    {
        LOG_INFO("easy deploy deploy server_ip = %s", easy_deploy->server_ip.c_str());
        if(easy_deploy->server_ip != _mina->mina_get_server_ip())
        {
            _mina->mina_destroy_connection();
            sleep(1);//FIXME: wait mina thread sync
            _mina->mina_save_server_ip(easy_deploy->server_ip);
            _configure_serverip_sync_flag = true;
            LOG_INFO("easy deploy set server_ip = %s", easy_deploy->server_ip.c_str());
        }
    }

    //step2: set hostname
    if(!easy_deploy->hostname.empty())
    {
        LOG_INFO("easy deploy deploy hostname = %s", easy_deploy->hostname.c_str());
        if(1 || easy_deploy->hostname != _hostname_data.get_hostname_info())//this value should alsays be set, to inform server 
        {
            _hostname_data.set_hostname_info(easy_deploy->hostname);
            _configure_hostname = easy_deploy->hostname;
            LOG_INFO("easy deploy set hostname = %s", easy_deploy->hostname.c_str());
            if(_mina->mina_get_connected())
            {
                _mina->mina_l2w_change_hostname(easy_deploy->hostname);
            }
        }
    }

    //step3: set auto power
    if(easy_deploy->set_autopower)
    {
        int ret = set_power_boot(easy_deploy->autopower);
        LOG_INFO("easy deploy set autopower ret=%d", ret);
    }

    //step4: set mode
    if(easy_deploy->mode != EASYDEPLOY_INVALID)
    {
        LOG_INFO("easy deploy deploy mode = %d", easy_deploy->mode);
        if(1 || easy_deploy->mode != _mode_data.get().mode)//this value should alsays be set, to inform server 
        {
            LOG_INFO("easy deploy set mode = %d", easy_deploy->mode);
            ModeInfo modeinfo;
            modeinfo.mode = (Mode)easy_deploy->mode;
            _mode_data.set(modeinfo);
            _configure_mode = easy_deploy->mode;
            _vm->vm_clear_inst();
            _vm->vm_clear_layer();
            _vm->vm_clear_teacher_disk();
            if(_mina->mina_get_connected())
            {
                _mina->mina_l2w_change_mode((Mode)easy_deploy->mode);
            }
        }
    }

    //step5: set vm network
    if(easy_deploy->set_vm_net)
    {
        LOG_DEBUG("EASY_DEPLOY_DEBUG network dhcp = %d autodns = %d", easy_deploy->vm_net.dhcp, easy_deploy->vm_net.auto_dns);
        LOG_INFO("easy deploy deploy vm_net = %s", easy_deploy->vm_net.ip.c_str());
        if(_vm->vm_get_vm_netinfo() != easy_deploy->vm_net)
        {
            LOG_INFO("easy deploy set vm_net = %s", easy_deploy->vm_net.ip.c_str());
            _vm->vm_set_vm_netinfo(&(easy_deploy->vm_net), _status_machine.get_status());
            if(_mina->mina_get_connected())
            {
                _mina->mina_web_upload_vm_network_info(easy_deploy->vm_net, _vm->get_vm_mac());
            }
        }
    }

    //step6: set local network
    if(easy_deploy->set_local_net)
    {
        //TODO:if wifi, not support to set local network
        if (_local_network_data.is_dev_wlan_up()) {
            LOG_WARNING("wifi was connected, so do not exec easy deploy");
            if(!_ui_locking)
            {
                UIPopEvent sync_event("Easy deploy forbidden");
                l2u_show_easydeploy_err((void *)Application::uipop_sync, (void *)&sync_event);
                sync_event.wait();
            }
            goto END;
        }

        LOG_DEBUG("EASY_DEPLOY_DEBUG local dhcp = %d autodns = %d", easy_deploy->local_net.dhcp, easy_deploy->local_net.auto_dns);
        LOG_INFO("easy deploy deploy local_net = %s", easy_deploy->local_net.ip.c_str());
        if(_local_network_data.get_ip_info() != easy_deploy->local_net)
        {
            LOG_INFO("easy deploy set local_net = %s", easy_deploy->local_net.ip.c_str());
            _local_network_data.set(easy_deploy->local_net);

            //wait_localnet();

            //if it reconnect, it would also send these msg to web again
            if(_mina->mina_get_connected())
            {
                _mina->mina_web_upload_local_network_info(easy_deploy->local_net);
            }
        }
    }

    //step7: show a pop window tip
    if(!_ui_locking)
    {
        UIPopEvent sync_event("Easy deploy OK");
        l2u_show_easydeploy_tips((void *)Application::uipop_sync ,(void *)&sync_event);
        sync_event.wait();
    }

END:
    // notify that easy deploy finished.
    _on_easy_deploy = false;

    //step7: reset status machine
    logic_reset_status_machine();

    delete easy_deploy;
    return SUCCESS;
}

int Application::set_local_network_callback(int callback_ret, void *data)
{
    Application* app = Application::get_application();
    LOG_INFO("local_network complete");
    //TODO:notify UI
    //app->notify_ui();

    //UI set net info
    if(app->_ui_need_sync_flag != 0)
    {
        app->_ui_has_sync_flag |= UI_NET_INFO_SYNC;
        if(callback_ret != SUCCESS)
        {
            app->_ui_setting_error |= UI_NET_INFO_ERR;
            //if set network fail, update _network_info
            app->_local_network_data.update();
        }
        if(app->_ui_has_sync_flag==app->_ui_need_sync_flag)
        {
            //app->l2u_notify_setting_result();
            app->ui_notify_setting_result();
        }
    }

    //notify web set net info
    app->_mina->mina_web_upload_local_network_info(app->_local_network_data.get_ip_info());
    app->notify_localnet();

    //if vm is running, recover vm net
    if(app->_vm->vm_is_vm_running())
    {
        rc_system("brctl addif br0 vnet0");
        DevInterfaceInfo inter_info;
        app->get_UsrUserInfoMgr()->get_dev_interface_info(inter_info);
        if (inter_info.net_passthrough == 1){
            rc_system("brctl addif br0 vnet1");
        }
    }
    return 0;
}

void Application::easy_deploy(int sig)
{
    //simplely complete for this function

    Application* app = Application::get_application();
    EasyDeploy *easy_deploy = new EasyDeploy;
    dictionary *d;
    int tmp;//to get whether set local net

    //init dictionary
    d = iniparser_load(EASY_DEPLOY_INI_FILENAME);
    if(d == NULL)
    {
        LOG_ERR("there is no ini file");
        return;
    }

    //get detail items
    easy_deploy->server_ip              = iniparser_getstring   (d, "easy_deploy:serverip"  , "");
    easy_deploy->hostname               = iniparser_getstring   (d, "easy_deploy:hostname"  , "");
    easy_deploy->mode                   = iniparser_getint      (d, "easy_deploy:mode"      , EASYDEPLOY_INVALID);
    easy_deploy->local_net.dhcp         = iniparser_getint      (d, "easy_deploy:dhcpip"    , 0);
    easy_deploy->local_net.ip           = iniparser_getstring   (d, "easy_deploy:ip"        , "");
    easy_deploy->local_net.submask      = iniparser_getstring   (d, "easy_deploy:netmask"   , "");
    easy_deploy->local_net.gateway      = iniparser_getstring   (d, "easy_deploy:gateway"   , "");
    easy_deploy->local_net.auto_dns     = iniparser_getint      (d, "easy_deploy:dhcpdns"   , 0);
    easy_deploy->local_net.main_dns     = iniparser_getstring   (d, "easy_deploy:dns"       , "");
    easy_deploy->local_net.back_dns     = iniparser_getstring   (d, "easy_deploy:backdns"   , "");
    easy_deploy->vm_net.dhcp            = iniparser_getint      (d, "easy_deploy:vm_dhcpip" , 0);
    easy_deploy->vm_net.ip              = iniparser_getstring   (d, "easy_deploy:vm_ip"     , "");
    easy_deploy->vm_net.submask         = iniparser_getstring   (d, "easy_deploy:vm_netmask", "");
    easy_deploy->vm_net.gateway         = iniparser_getstring   (d, "easy_deploy:vm_gateway", "");
    easy_deploy->vm_net.auto_dns        = iniparser_getint      (d, "easy_deploy:vm_dhcpdns", 0);
    easy_deploy->vm_net.main_dns        = iniparser_getstring   (d, "easy_deploy:vm_dns"    , "");
    easy_deploy->vm_net.back_dns        = iniparser_getstring   (d, "easy_deploy:vm_backdns", "");
    easy_deploy->autopower              = iniparser_getint      (d, "easy_deploy:autopower",  0);

    //if dhcpip has no value, the whole network inso is not neccessary to be set
    tmp                                 = iniparser_getint      (d, "easy_deploy:dhcpip"    , EASYDEPLOY_INVALID);
    easy_deploy->set_local_net = ((tmp == EASYDEPLOY_INVALID) ? false : true);
    tmp                                 = iniparser_getint      (d, "easy_deploy:vm_dhcpip" , EASYDEPLOY_INVALID);
    easy_deploy->set_vm_net = ((tmp == EASYDEPLOY_INVALID) ? false : true);
    tmp                                 = iniparser_getint      (d, "easy_deploy:power_need_deploy", EASYDEPLOY_INVALID);
    easy_deploy->set_autopower = ((tmp <= 0) ? false : true);

    iniparser_freedict(d);

    LOG_INFO("easy_deploy->server_ip:%s",             easy_deploy->server_ip.c_str());
    LOG_INFO("easy_deploy->hostname:%s",              easy_deploy->hostname.c_str());
    LOG_INFO("easy_deploy->mode:%d",                  easy_deploy->mode);
    LOG_INFO("easy_deploy->local_net.dhcp:%d",        easy_deploy->local_net.dhcp);
    LOG_INFO("easy_deploy->local_net.ip:%s",          easy_deploy->local_net.ip.c_str());
    LOG_INFO("easy_deploy->local_net.submask:%s",     easy_deploy->local_net.submask.c_str());
    LOG_INFO("easy_deploy->local_net.gateway:%s",     easy_deploy->local_net.gateway.c_str());
    LOG_INFO("easy_deploy->local_net.auto_dns:%d",    easy_deploy->local_net.auto_dns);
    LOG_INFO("easy_deploy->local_net.main_dns:%s",    easy_deploy->local_net.main_dns.c_str());
    LOG_INFO("easy_deploy->local_net.back_dns:%s",    easy_deploy->local_net.back_dns.c_str());
    LOG_INFO("easy_deploy->autopower:%d",             easy_deploy->autopower);
    LOG_INFO("easy_deploy->set_local_net:%d",         easy_deploy->set_local_net);
    LOG_INFO("easy_deploy->set_vm_net:%d",            easy_deploy->set_vm_net);
    LOG_INFO("easy_deploy->set_autopower:%d",         easy_deploy->set_autopower);

    //create event and push
    int ret = 0;
    LogicEvent *eve = new LogicEvent(app, L2L_EASY_DEPLOY, static_cast<void*>(easy_deploy), 0);
    ret = app->_process_loop.push_event(eve);
    eve->unref();
    if(ret < 0)
    {
        LOG_ERR("now the processloop cannot handle easy deploy, so ignore");
        delete easy_deploy;
        return;
    }
    return;

}

//ui wrap interface
void Application::l2u_show_unconfig_wrap()
{
	if(_userinfomgr.UI_permitted())
    {
        LOG_DEBUG("call l2u_show_unconfig");
        l2u_show_unconfig();
    }
}

void Application::l2u_show_connecting_wrap()
{
	if(_userinfomgr.UI_permitted())
    {
        LOG_DEBUG("call l2u_show_connecting");
        l2u_show_connecting();
    }
}

void Application::l2u_show_disconnect_wrap()
{
    if(_userinfomgr.UI_permitted())
    {
        LOG_DEBUG("call l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT)");
		LOG_INFO("call l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT)1313131313");
        //l2u_show_disconnect();
        l2u_show_tips(UI_TIPS_NET_DISCONNETCT_OPT);
    }
}

void Application::l2u_show_updateclient_wrap()
{
    if(_userinfomgr.UI_permitted())
    {
        LOG_DEBUG("call l2u_show_updateclient");
        l2u_show_updateclient();
    }
}


void Application::l2u_result_updateclient_wrap(int result)
{
	if(_userinfomgr.UI_permitted())
    {
        LOG_DEBUG("call l2u_result_updateclient");
        l2u_result_updateclient(result);
    }
}

void Application::l2u_show_not_whitessid_show(vector<string> &whitelist)
{
    int size = 0, i = 0;
    ui_wifiinfo *connect_info = NULL;
    int is_white = 1;
    
    size = whitelist.size();
    connect_info = _wifi_interactive->wifi_status_query_result();
    if (connect_info != NULL && strcmp(connect_info->ssid, "") != 0) {
        for (i = 0; i < size; i++) {
            is_white = 0;
            if (strcmp(connect_info->ssid, whitelist[i].c_str()) == 0) {
                is_white = 1;
                break;
            }
        }
    }
    
    if (!is_white) {
       l2u_show_dialog_connect_network(UI_TIPS_CONNECT_NOT_WHITESSID);
    }   
}

void Application::uipop_sync(void *event)
{
    UIPopEvent *pop_event = (UIPopEvent *)event;
    pop_event->response();
}

void Application::handle_xserver_exited_on_vm_fail()
{
    // X server exited, we could only reboot system.
    LOG_EMERG("VM started failed, but Graphic is already unusable, system will restart soon.\n");
    rc_system("touch /root/.rcc_exception_reboot");
    rc_system("sync");
    sleep(1);
    rc_system("reboot");
    // Never return here
    exit(-1);
}

void  Application::handle_xserver_alive_on_vm_fail(){
    if (_vm->vm_is_xserver_exited()) {
        return;
    }

    UnjoinableThread *ui_pthread;
    struct arg_t *arg = new (struct arg_t);
    int err_code = _vm->get_vmManage()->get_start_vm_err();
    string err_data;
    UIPopEvent sync_event("VM started failed");
    
    arg->argc = _argc;
    arg->argv = _argv;
    ui_pthread = new UnjoinableThread(Application::ui_top_thread, (void *)arg);

    LOG_INFO("handle_xserver_alive_on_vm_fail err_code %d", err_code);
    if (err_code == START_NOT_SUPPORT_CPU) {
        l2u_show_vmerr_tips((void *)Application::uipop_sync, (void *)&sync_event, (void*)(_basic_data.get().cpu.c_str()), err_code);
    } else if(err_code == START_NOT_SUPPORT_OSTYPE) {
        _vm->vm_get_image_ostype(err_data);
        l2u_show_vmerr_tips((void *)Application::uipop_sync, (void *)&sync_event, (void*)(err_data.c_str()), err_code);
    } else if (err_code == START_OSTYPE_UNKNOWN) {
        err_data = "未知";
        l2u_show_vmerr_tips((void *)Application::uipop_sync, (void *)&sync_event, (void*)(err_data.c_str()), err_code);
    } else if (err_code == START_INTEL_NO_AUDIO_DEVICE) {
        l2u_show_vmerr_tips((void *)Application::uipop_sync, (void *)&sync_event, NULL, err_code);
    } else {
        l2u_show_vmerr_tips((void *)Application::uipop_sync, (void *)&sync_event, NULL, err_code);
    }

    sync_event.wait();
    ui_pthread->cancel();
    delete ui_pthread;
    exit(1); // exit, and daemon will restart it.

}

void Application::handle_install_driver_on_vm_fail()
{
    char command_buf[128] = {0};
    UnjoinableThread *ui_pthread;
    struct arg_t *arg;
    string driver_ostype;
    int err_code = START_VM_CHECK_OK;

    sprintf(command_buf, "umount %s", rc_json_get_string(_driver_install_parament, "path_name").c_str());
    rc_system(command_buf);

    err_code = _vm->get_vmManage()->get_start_vm_err();
    LOG_INFO("handle_install_driver_on_vm_fail err_code %d", err_code);
    if (err_code == START_VM_FAILED) {
        _mina->mina_web_upload_driver_install_result(ERROR_START_VM_FAIL);
        sleep(2);
        rc_system("reboot");
        return;
    }

    arg = new (struct arg_t);
    arg->argc = _argc;
    arg->argv = _argv;
    ui_pthread = new UnjoinableThread(Application::ui_thread, (void *)arg);

    UIPopEvent sync_event_drive("VM started failed with driver error");
    switch(err_code) {
    case START_DRIVER_OSTYPE_NOTADAPT: // vmmode is emulation or passthrough
    {
        _mina->mina_web_upload_driver_install_result(ERROR_DRIVER_OSTYPE_NOTADAPT);
        sleep(2);

        driver_ostype = rc_json_get_string(_driver_install_parament, "ostype");
        l2u_show_vmerr_tips((void *)Application::uipop_sync, (void *)&sync_event_drive, (void*)(driver_ostype.c_str()), err_code);
        sync_event_drive.wait();
        break;
    }
    case START_NOT_SUPPORT_CPU:
    {
        _mina->mina_web_upload_driver_install_result(ERROR_DRIVER_OSTYPE_NOTADAPT);
        sleep(2);

        l2u_show_vmerr_tips((void *)Application::uipop_sync, (void *)&sync_event_drive, (void*)(_basic_data.get().cpu.c_str()), err_code);
        sync_event_drive.wait();
        break;
    }
    case START_NOT_SUPPORT_OSTYPE:
    {
        _mina->mina_web_upload_driver_install_result(ERROR_DRIVER_OSTYPE_NOTADAPT);
        sleep(2);

        driver_ostype = rc_json_get_string(_driver_install_parament, "ostype");
        l2u_show_vmerr_tips((void *)Application::uipop_sync, (void *)&sync_event_drive, (void*)(driver_ostype.c_str()), err_code);
        sync_event_drive.wait();
        break;
    }
    case START_VMMODEINT_LOST:
    {
        _mina->mina_web_upload_driver_install_result(ERROR_DRIVER_OSTYPE_NOTADAPT);
        sleep(2);

        l2u_show_vmerr_tips((void *)Application::uipop_sync, (void *)&sync_event_drive, NULL, err_code);
        sync_event_drive.wait();
        break;
    }
    case START_INTEL_NO_AUDIO_DEVICE: 
    {
        _mina->mina_web_upload_driver_install_result(ERROR_START_VM_FAIL);
        sleep(2);
        l2u_show_vmerr_tips((void *)Application::uipop_sync, (void *)&sync_event_drive, NULL, err_code);
        sync_event_drive.wait();
        break;
    }
    default:
        _mina->mina_web_upload_driver_install_result(ERROR_START_VM_FAIL);
        sleep(2);
        break;
    }

    ui_pthread->cancel();
    delete ui_pthread;
    // send data to web then reboot
    rc_system("reboot");
    return;
}

void Application::handle_on_vm_start_success()
{
    LOG_DEBUG("handle_on_vm_start_success");
    
    UnjoinableThread *ui_pthread;
    struct arg_t *arg;
    int is_emulation = false;
    string driver_ostype;
   // int clear_operation;
    string ostype;

    // if install driver, then is passthrough
    if (!is_installing_driver()) {
        is_emulation = _vm->get_vmManage()->get_start_vm_is_emulation();
        _vm->vm_get_image_ostype(ostype);
        LOG_DEBUG("ostype %s is_local_start %d", ostype.c_str(), is_localmode_startvm());
    } else {
        LOG_DEBUG("_driver_install_parament %s", _driver_install_parament.c_str());
        driver_ostype = rc_json_get_string(_driver_install_parament, "ostype");
        is_emulation = _vm->get_vmManage()->get_start_vm_is_emulation(true, driver_ostype);
    }

    if (is_emulation) {
        // start est sock if vmmode is emulation
        if (_device_interface->startPaService() < 0) {
            LOG_WARNING("startPaService start err");
        }
        _vm->vm_start_est_sock();
        arg = new (struct arg_t);
        arg->argc = _argc;
        arg->argv = _argv;

        // sync destroy win main bg then quit ui thread
        ui_pthread = new UnjoinableThread(Application::ui_top_thread, (void *)arg);
        LOG_DEBUG("l2u_destroy_screen begin");
        l2u_destroy_screen();
        l2u_ui_thread_quit();
        _ui_waitsync_event.wait();
        
        delete ui_pthread;
    }

    // notify web vm start
    __upload_vm_info(true);
}

void Application::handle_on_vm_stop_success()
{
    LOG_DEBUG("handle_on_vm_stop_success");
    int is_emulation;
    string driver_ostype;

    if (!is_installing_driver()) {
        is_emulation = _vm->get_vmManage()->get_start_vm_is_emulation();
    } else {
        LOG_DEBUG("_driver_install_parament %s", _driver_install_parament.c_str());
        driver_ostype = rc_json_get_string(_driver_install_parament, "ostype");
        is_emulation = _vm->get_vmManage()->get_start_vm_is_emulation(true, driver_ostype);
    }

    LOG_DEBUG("is_driver %d driver_ostype %s is_emulation %d", is_installing_driver(), driver_ostype.c_str(), is_emulation);

    if (is_emulation) {
        //stop est sock
        _device_interface->stopPaService();
        _vm->vm_stop_est_sock();
    }

    //notify web vm shutdown
    __upload_vm_info(false);
}

void Application::config_net_auth_start(void* data)
{
    int* is_success = static_cast<int*>(data);
    if (*is_success == 1) {
        //auth success
        _ui_setting_tip = UI_SETTING_OK;
    } else if (*is_success == 0) {
        //auth fail
        _ui_setting_tip = UI_SETTING_AUTH_FAIL;
        _ui_setting_error |= UI_AUTH_ERR;
    } else {
        LOG_ERR("unknown net auth result, ret = %d", *is_success);
    }
    delete is_success;
}

void Application::config_net_auth_cancel(void* data)
{
    int* result = static_cast<int*>(data);
    //TODO cancel net auth
    delete result;
}

int Application::config_sync_event_response(int event_id, void* data)
{
    LOG_INFO("enter config_sync_event_response. event_id: %d", event_id);
    if (_config_event_mgr == NULL) {
        LOG_CRIT("_config_event_mgr is NULL!");
        return -1;
    }
    _config_event_mgr->response(event_id, data);
    return SUCCESS;
}

void UIPopEvent::do_response()
{
    LOG_INFO("Wake up from event '%s'\n", _event_desc.c_str());
}

void UIPopEvent::print_event_desc()
{
    LOG_INFO("Wait on event '%s'\n", _event_desc.c_str());
}

void ConfigSyncEvent::do_response()
{
    LOG_INFO("Wake up from event '%s'\n", ConfigEventDesc[_event_id].c_str());
}



void DevPolicyEvent::do_response()
{
    LOG_INFO("Wake up from event '%s'\n", _event_desc.c_str());
}

void DevPolicyEvent::print_event_desc()
{
    LOG_INFO("Wait on event '%s'\n", _event_desc.c_str());
}

#ifdef UNIT_TEST
void* Application::unittest(void* data)
{
    Application* app = (Application*)data;
    sleep(3);
    app->_status_machine.change_status(EVENT_WEB_INFO_SYNCHRONIZED);
    sleep(1);
//    l2u_download_t arg;
//    l2u_show_download(&arg);
//sleep(3);
    LOG_DEBUG("UNITTEST: W2L_SYNC_IMAGE");
    ImageInfo *imageinfo1 = new ImageInfo;
    LogicEvent* evet;
    imageinfo1->id=11;
	imageinfo1->md5 = "12345656778899";
	imageinfo1->recovery = true;
	imageinfo1->name = "win7_test.base";
	imageinfo1->torrent_url = "http://172.21.111.193/module/image/downloadTorrent/win7_test.base.torrent";
	imageinfo1->version = "1";
	evet = new LogicEvent(app, W2L_SYNC_IMAGE, static_cast<void*>(imageinfo1), 0);
	app->_process_loop.push_event(evet);
	evet->unref();

//	UserInfo info;
//	app->web_login_success(info);
    while(1);
//	delete evet;
//	delete imageinfo1;
    return NULL;
/*	sleep(2);
	printf("======__l2u_show_disconnect__=====\n");
//	l2u_show_unconfig();

	struct ImageInfo imageInfo =
	{
			true,
			"win7",
			"1",
			"http://idvtest/client/SublimeText.exe.torrent",
			"md5",
	};

//	printf("cur state:%d change_status\n", app->_status_machine.get_status());
//	app->_status_machine.change_status(EVENT_IMAGE_NEED_DOWNLOAD);
//	sleep(30);
	app->_vm->vm_start_download_image(&imageInfo);
	sleep(18);
	struct ImageInfo imageInfo2 = app->_vm->vm_get_vm_imageinfo();
	printf("===torrent url:%s md5:%s\n", imageInfo2.torrent_url.c_str(), imageInfo2.md5.c_str());

	NetworkInfo networkInfo =
	{
	 100,
	 false,
	 "111.1.10.11",
	 "225.255.252.0",
	 "10.1.10.1",
	 false,
	 "15.114.114.1",
	 "15.114.1.1"
   };;
	app->_vm->vm_set_vm_netinfo_to_ini(networkInfo);

	printf("cur state:%d change_status\n", app->_status_machine.get_status());
	app->_status_machine.change_status(EVENT_LOCAL_MODE_CHOOSEN);
	printf("===WAIT 80s\n");
	sleep(80);
	printf("===req SHUTDOWN\n");
	app->_vm->vm_shutdown_VM_normal();
	printf("===WAIT SHUTDOWN\n");
	sleep(30);
	printf("===SHUTDOWN OK\n");
	printf("===del teacher disk\n");
	app->_vm->vm_clear_teacher_disk();
	sleep(10);
	printf("===del inst\n");
	app->_vm->vm_clear_inst();
	sleep(10);
	printf("===del base\n");
	app->_vm->vm_clear_base();

//	l2u_show_disconnect();
    printf("%s\n", __func__);
    sleep(1);*/

#if 0
    LogicEvent* eve1 = new LogicEvent(app, U2L_CANCEL_CONNECTION, NULL, 0);
    app->_process_loop.push_event(eve1);

    UserInfo userinfo1;
    userinfo1.username = "chenli";
    userinfo1.password = "123";
    UserInfo* userinfo2 = new UserInfo;
    userinfo2->username = userinfo1.username;
    userinfo2->password = userinfo1.password;
    LogicEvent* eve2 = new LogicEvent(app, U2L_MODIFY_PASSWORD, (void*)userinfo2, 0);
    app->_process_loop.push_event(eve2);
#endif
    BasicInfo basicinfo;
    LogicEvent* eve;
    UserInfo* userinfo;
    ImageInfo* imageinfo;
    DownloadInfo* dlinfo;
    NetworkInfo netinfo;
    char* name = NULL; 
    VersionInfo* ver;
    ModeInfo* modeinfo;
    PolicyInfo* policy;
    string* hostname;

#define UNITTEST_CONNECT_SERVER \
    LOG_DEBUG("UNITTEST: W2L_MINA_CONNECTION_ESTABLISHED");\
        eve = new LogicEvent(app, W2L_MINA_CONNECTION_ESTABLISHED, NULL, 0);\
        app->_process_loop.push_event(eve);\
        eve->unref();

#define UNITTEST_SYNC_SOFTWARE_VERSION \
    LOG_DEBUG("UNITTEST: W2L_SYNC_SOFTWARE_VERSION");\
     ver = new VersionInfo;\
     ver->main_version = 2;\
     ver->minor_version = 1;\
     ver->third_version = 3;\
     ver->fourth_version = 45;\
     ver->extra_first = "_R1";\
     ver->extra_second = "_HQDX";\
     eve = new LogicEvent(app, W2L_SYNC_SOFTWARE_VERSION,  static_cast<void*>(ver), 0);\
     app->_process_loop.push_event(eve);\
     eve->unref();

#define UNITTEST_SYNC_PUBLIC_POLICY \
    LOG_DEBUG("UNITTEST: W2L_SYNC_PUBLIC_POLICY");\
        policy = new PolicyInfo;\
        policy->net_policy = true;\
        policy->usb_policy = "on";\
        eve = new LogicEvent(app, W2L_SYNC_PUBLIC_POLICY, static_cast<void*>(policy), 0);\
        app->_process_loop.push_event(eve);\
        eve->unref();\

#define UNITTEST_SYNC_MODE(MODE) \
    LOG_DEBUG("UNITTEST: W2L_SYNC_MODE");\
        modeinfo = new ModeInfo;\
        modeinfo->mode = MODE;\
        app->_mode_data.set(*modeinfo);\
        eve = new LogicEvent(app, W2L_SYNC_MODE, static_cast<void*>(modeinfo), 0);\
        app->_process_loop.push_event(eve);\
        eve->unref();

#define UNITTEST_GET_VM_DEV_INTERFACE_INFO \
    LOG_DEBUG("UNITTEST: W2L_GET_VM_DEV_INTERFACE_INFO");\
    eve = new LogicEvent(app, W2L_GET_VM_DEV_INTERFACE_INFO, NULL, 0);\
    app->_process_loop.push_event(eve);\
    eve->unref();

#define UNITTEST_SYNC_HOSTNAME \
    LOG_DEBUG("UNITTEST: W2L_SYNC_HOSTNAME");\
        hostname = new string("student");\
        eve = new LogicEvent(app, W2L_SYNC_HOSTNAME, static_cast<void*>(hostname), 0);\
        app->_process_loop.push_event(eve);\
        eve->unref();

#define UNITTEST_CHECK_ADMIN_PASSWD \
       int admin_ret = 0; \
       admin_ret = ::ui_check_admin_passwd((char*)app->_admin_passwd.c_str()); \
       LOG_DEBUG("admin_ret %d\n", admin_ret);

#define UNITTEST_SET_SERVER_IP \
     string* server_ip = new string("127.0.0.1"); \
     eve = new LogicEvent(app, U2L_SET_SERVER_IP, static_cast<void*>(server_ip), 0); \
     app->_process_loop.push_event(eve); \
     eve->unref(); 

#define UNITTEST_SET_HOST_NAME \
     name = new char[64]; \
     strcpy(name, "teacher"); \
     eve = new LogicEvent(app, U2L_SET_HOST_NAME, static_cast<void*>(name), 0); \
     app->_process_loop.push_event(eve); \
     eve->unref(); 

#define UNITTEST_ACCOUNT_LOGIN1 \
    userinfo = new UserInfo; \
    userinfo->username = "testuser"; \
    userinfo->password = "test passwd"; \
    eve = new LogicEvent(app, U2L_ACCOUNT_LOGIN, static_cast<void*>(userinfo), 0); \
    app->_process_loop.push_event(eve); \
    eve->unref(); 

#define UNITTEST_LOGIN_FAIL \
    eve = new LogicEvent(app, W2L_LOGIN_FAIL, NULL, 0);\
    app->_process_loop.push_event(eve);\
    eve->unref();

#define UNITTEST_ACCOUNT_LOGIN2 \
    userinfo = new UserInfo; \
    userinfo->username = "testuser"; \
    userinfo->password = "testtest"; \
    eve = new LogicEvent(app, U2L_ACCOUNT_LOGIN, static_cast<void*>(userinfo), 0); \
    app->_process_loop.push_event(eve); \
    eve->unref();

#define UNITTEST_LOGIN_SUCCESS \
    eve = new LogicEvent(app, W2L_LOGIN_SUCCESS, NULL, 0); \
    app->_process_loop.push_event(eve);\
    eve->unref();

#define UNITTEST_BIND_USER \
    int* bind = new int; \
     *bind = 1;\
     eve = new LogicEvent(app, U2L_BIND_USER, bind, 0);\
     app->_process_loop.push_event(eve);\
     eve->unref();

#define UNITTEST_BIND_SECCESS \
    LOG_DEBUG("UNITTEST: W2L_BIND_SECCESS");\
        userinfo = new UserInfo;\
        userinfo->username = "testuser";\
        userinfo->password = "testtest";\
        eve = new LogicEvent(app, W2L_BIND_SECCESS, static_cast<void*>(userinfo), 0);\
        app->_process_loop.push_event(eve);\
        eve->unref();

#define UNITTEST_SYNC_IMAGE \
    LOG_DEBUG("UNITTEST: W2L_SYNC_IMAGE");\
    imageinfo = new ImageInfo;\
    imageinfo->md5 = "12345656778899";\
    imageinfo->recovery = false;\
    imageinfo->name = "win7";\
    imageinfo->torrent_url = "172.21.112.250/etc/";\
    imageinfo->version = "1.01";\
    eve = new LogicEvent(app, W2L_SYNC_IMAGE, static_cast<void*>(imageinfo), 0);\
    app->_process_loop.push_event(eve);\
    eve->unref();

#define UNITTEST_DOWNLOAD_FAIL \
    LOG_DEBUG("UNITTEST: V2L_VM_DOWNLOAD_PROGRESS_STATUS fail");\
        dlinfo = new DownloadInfo;\
        dlinfo->status = -1;\
        eve = new LogicEvent(app, V2L_VM_DOWNLOAD_PROGRESS_STATUS, static_cast<void*>(dlinfo), 0);\
        app->_process_loop.push_event(eve);\
        eve->unref();

#define UNITTEST_DOWNLOAD_PROGRESS \
    LOG_DEBUG("UNITTEST: V2L_VM_DOWNLOAD_PROGRESS_STATUS 50%"); \
    dlinfo = new DownloadInfo;\
    dlinfo->status = 0;\
    dlinfo->image_name = "win7";\
    dlinfo->image_version = "1.01";\
    dlinfo->percent = "50%";\
    dlinfo->speed = "10Mbps";\
    eve = new LogicEvent(app, V2L_VM_DOWNLOAD_PROGRESS_STATUS, static_cast<void*>(dlinfo), 0);\
    app->_process_loop.push_event(eve);\
    eve->unref();

#define UNITTEST_DOWNLOAD_SUCCESS  \
    LOG_DEBUG("UNITTEST: V2L_VM_DOWNLOAD_PROGRESS_STATUS success");\
    dlinfo = new DownloadInfo;\
    dlinfo->status = 1;\
    dlinfo->image_name = "win7";\
    dlinfo->image_version = "1.01";\
    dlinfo->percent = "100%";\
    dlinfo->speed = "10Mbps";\
    eve = new LogicEvent(app, V2L_VM_DOWNLOAD_PROGRESS_STATUS, static_cast<void*>(dlinfo), 0);\
    app->_process_loop.push_event(eve);\
    eve->unref();

#define UNITTEST_VM_START_SUCCESS  \
    LOG_DEBUG("UNITTEST: V2L_VM_START_RESULT"); \
       eve = new LogicEvent(app, V2L_VM_START_RESULT, NULL, 0);\
       app->_process_loop.push_event(eve);\
       eve->unref();

#define UNITTEST_GUEST_LOGIN \
    LOG_DEBUG("UNITTEST: U2L_GUEST_LOGIN"); \
    eve = new LogicEvent(app, U2L_GUEST_LOGIN, NULL, 0);\
    app->_process_loop.push_event(eve);\
    eve->unref();  

    // 0 query info
    
    app->ui_get_basic_info(&basicinfo);    
    LOG_DEBUG("unittest basicinfo SN:%s, id=%s, mac=%s, sw ver:%s, hw ver:%s, os ver:%s, cpu:%s, mem:%s, storage:%s", 
                    basicinfo.serial_number.c_str(), basicinfo.product_id.c_str(), basicinfo.mac.c_str(), basicinfo.software_version.c_str(), basicinfo.hardware_version.c_str(), 
                    basicinfo.os_version.c_str(), basicinfo.cpu.c_str(), basicinfo.memory.c_str(), basicinfo.storage.c_str());

    
   
    app->ui_get_net_info(&netinfo);    
    LOG_DEBUG("unittest DHCP:%d, auto dns:%d, ip:%s, summask:%s, gateway:%s, main_dns:%s, back_dns:%s, speed:%d", 
            netinfo.dhcp, netinfo.auto_dns, netinfo.ip.c_str(), netinfo.submask.c_str(), netinfo.gateway.c_str(), netinfo.main_dns.c_str(), netinfo.back_dns.c_str(), netinfo.netcard_speed);

    name = new char[64];
    app->ui_get_host_name(name);
    LOG_DEBUG("unittest hostname=%s", name);
    delete name;
    name = NULL;

// special mode
#if 1
    // 1 sync server
    UNITTEST_CONNECT_SERVER
    UNITTEST_SYNC_SOFTWARE_VERSION
    UNITTEST_SYNC_PUBLIC_POLICY
    UNITTEST_SYNC_MODE(SPECIAL_MODE)
    UNITTEST_SYNC_HOSTNAME
    UNITTEST_GET_VM_DEV_INTERFACE_INFO

    // 2 wait login
    UNITTEST_CHECK_ADMIN_PASSWD
    UNITTEST_SET_SERVER_IP
    UNITTEST_ACCOUNT_LOGIN1
    UNITTEST_LOGIN_FAIL
    UNITTEST_ACCOUNT_LOGIN2
    UNITTEST_LOGIN_SUCCESS
    UNITTEST_BIND_USER
    UNITTEST_BIND_SECCESS

    // 3 download image
    UNITTEST_SYNC_IMAGE
    UNITTEST_DOWNLOAD_FAIL
    UNITTEST_SYNC_IMAGE
    UNITTEST_DOWNLOAD_PROGRESS
    UNITTEST_DOWNLOAD_SUCCESS
    
    // 4 start vm success
    UNITTEST_VM_START_SUCCESS
#endif

//multiuser mode
#if 0
    // 1 sync server
    UNITTEST_CONNECT_SERVER
    UNITTEST_SYNC_SOFTWARE_VERSION
    UNITTEST_SYNC_PUBLIC_POLICY
    UNITTEST_SYNC_HOSTNAME
    UNITTEST_SYNC_MODE(MULTIUSER_MODE)

    // 2 download image
    UNITTEST_SYNC_IMAGE
    UNITTEST_DOWNLOAD_FAIL
    UNITTEST_SYNC_IMAGE
    UNITTEST_DOWNLOAD_PROGRESS
    UNITTEST_DOWNLOAD_SUCCESS

    // 3 login
    UNITTEST_CHECK_ADMIN_PASSWD
    UNITTEST_SET_SERVER_IP
    UNITTEST_ACCOUNT_LOGIN1 
    UNITTEST_LOGIN_FAIL
    UNITTEST_ACCOUNT_LOGIN2
    UNITTEST_LOGIN_SUCCESS

    // 4 download image again
    UNITTEST_SYNC_IMAGE
    UNITTEST_DOWNLOAD_PROGRESS
    UNITTEST_DOWNLOAD_SUCCESS

    // 5 start vm success
    UNITTEST_VM_START_SUCCESS
#endif

//public mode 
#if 0
     // 1 sync server
     UNITTEST_CONNECT_SERVER
     UNITTEST_SYNC_SOFTWARE_VERSION
     UNITTEST_SYNC_PUBLIC_POLICY
     UNITTEST_SYNC_HOSTNAME
     UNITTEST_SYNC_MODE(PUBLIC_MODE)
 
     // 2 download image
     UNITTEST_SYNC_IMAGE
     UNITTEST_DOWNLOAD_FAIL
     UNITTEST_SYNC_IMAGE
     UNITTEST_DOWNLOAD_PROGRESS
     UNITTEST_DOWNLOAD_SUCCESS
 
     // 3 start vm success
     UNITTEST_VM_START_SUCCESS
 #endif

 //special mode guest login
#if 0
     // 1 sync server
    UNITTEST_CONNECT_SERVER
    UNITTEST_SYNC_SOFTWARE_VERSION
    UNITTEST_SYNC_PUBLIC_POLICY
    UNITTEST_SYNC_MODE(SPECIAL_MODE)
    UNITTEST_SYNC_HOSTNAME

    // 2 wait login
    UNITTEST_CHECK_ADMIN_PASSWD
    UNITTEST_SET_HOST_NAME
     ModeInfo modeinfo2 = app->_mode_data.get();
    modeinfo2.bind_user.username.clear();
    app->_mode_data.set(modeinfo2);
    UNITTEST_GUEST_LOGIN

    sleep(1); //must sleep or next _mode_data.set will excute before first UNITTEST_GUEST_LOGIN

    modeinfo2.bind_user.username = "student1";
    app->_mode_data.set(modeinfo2);
    UNITTEST_GUEST_LOGIN

    // 3 download image
    UNITTEST_SYNC_IMAGE
    UNITTEST_DOWNLOAD_FAIL
    UNITTEST_SYNC_IMAGE
    UNITTEST_DOWNLOAD_PROGRESS
    UNITTEST_DOWNLOAD_SUCCESS
    
    // 4 start vm success
    UNITTEST_VM_START_SUCCESS
 #endif
 
    /*
    UserInfo *userinfo = new UserInfo;
    userinfo->group_id = 1;
    userinfo->netdisk_info.netdisk_enable = true;
    userinfo->netdisk_info.netdisk_ip = "127.0.0.1";
    userinfo->netdisk_info.netdisk_password = "12345";
    userinfo->netdisk_info.netdisk_path = "/mnt/";
    userinfo->netdisk_info.netdisk_username = "user";
    userinfo->new_password = "67890";
    userinfo->password = "12345";
    userinfo->username = "user";
    userinfo->policy_info.net_policy = true;
    userinfo->policy_info.usb_policy = "1234";
    LogicEvent* eve9 = new LogicEvent(app, U2L_BIND_USER, static_cast<void*>(userinfo), 0);
    app->_process_loop.push_event(eve9);
    eve9->unref();

    LogicEvent* eve10 = new LogicEvent(app, U2L_REBOOT_TERMINAL, NULL, 0);
    app->_process_loop.push_event(eve10);
    eve10->unref();

    LogicEvent* eve11 = new LogicEvent(app, U2L_SHUTDOWN_TERMINAL, NULL, 0);
    app->_process_loop.push_event(eve11);
    eve11->unref();

    LogicEvent* eve12 = new LogicEvent(app, W2L_NOTIFY_SHUTDOWN, NULL, 0);
    app->_process_loop.push_event(eve12);
    eve12->unref();
*/

//	printf("cur state:%d change_status\n", app->_status_machine.get_status());
//    app->_status_machine.change_status(EVENT_IMAGE_NEED_DOWNLOAD);
//    sleep(30);
//    app->_vm->vm_quit_download_image();
//	printf("cur state:%d change_status\n", app->_status_machine.get_status());
//    app->_status_machine.change_status(EVENT_LOCAL_MODE_CHOOSEN);
//    sleep(80);


/*
    VersionInfo ver;
    ver.main_version = 1;
    ver.minor_version = 2;
    ver.third_version = 3;
    ver.fourth_version = 4;
    ver.extra_first = "extra_a";
    ver.extra_second = "extra_b";
    ver.build_date = "2017.2.24";

    PolicyInfo poli;
    poli.usb_policy = "{diasble:usb}";
    poli.net_policy = true;

    ModeInfo special_mode;
    special_mode.mode = SPECIAL_MODE;
    special_mode.bind_user.username = "bangding";

    ModeInfo unbind_mode;
    unbind_mode.mode = SPECIAL_MODE;
    unbind_mode.bind_user.username = "";

    ImageInfo image;
    image.md5 = "test_md5_wqsadvdiyuyewrqd";
    image.name = "889757";
    image.version = "V1.0.2.2";
    image.recovery = true;
    image.torrent_url = "172.21.136.12:/etc";

    string hostname = "rcd";

    UserInfo userinfo;
    userinfo.group_id = 1;
    userinfo.netdisk_info.netdisk_enable = true;
    userinfo.netdisk_info.netdisk_ip = "127.0.0.1";
    userinfo.netdisk_info.netdisk_password = "12345";
    userinfo.netdisk_info.netdisk_path = "/mnt/";
    userinfo.netdisk_info.netdisk_username = "user";
    userinfo.new_password = "xinmima";
    userinfo.password = "mima";
    userinfo.username = "pandeng";
    userinfo.policy_info.net_policy = true;
    userinfo.policy_info.usb_policy = "1234";

    DownloadInfo download_checking;
    download_checking.image_name = "89757";
    download_checking.image_version = "33.33%";
    download_checking.speed = "10mb/s";
    download_checking.status = DOWNLOAD_IMAGE_CHECKING_MD5;

    DownloadInfo download_ing;
    download_ing.image_name = "89757";
    download_ing.image_version = "33.33%";
    download_ing.speed = "10mb/s";
    download_ing.status = DOWNLOAD_IMAGE_DOWNLOADING;

    DownloadInfo download_success;
    download_success.image_name = "89757";
    download_success.image_version = "33.33%";
    download_success.speed = "10mb/s";
    download_success.status = DOWNLOAD_IMAGE_SUCCESS;

    DownloadInfo download_failed;
    download_failed.image_name = "89757";
    download_failed.image_version = "33.33%";
    download_failed.speed = "10mb/s";
    download_failed.status = DOWNLOAD_IMAGE_ERROR;


    //1 *************************special unbind begin**************************

    //2 STATUS_INITING
    app->web_sync_software_version(ver);
    app->web_sync_public_policy(poli);
    app->web_sync_mode(unbind_mode);
    app->web_sync_hostname(hostname);

    //2 STATUS_WAITING_LOGIN 
    //3 try wrong password
    ::ui_account_login("pandeng", "cuomima");
    app->web_login_fail();

    //3 try right password, but not bind
    ::ui_account_login("pandeng", "mima");
    app->web_login_success(userinfo);
    ::ui_bind_user(0);

    //3 try right password, and bind
    ::ui_account_login("pandeng", "mima");
    app->web_login_success(userinfo);
    ::ui_bind_user(1);
    app->web_bind_success(userinfo);

    //2 STATUS_PREPARING_IMAGE
    //3 TOADD image is different

    //3 image is same
    app->web_sync_image(image);
    app->vm_download_progress(&download_ing);
    app->vm_download_progress(&download_ing);
    app->vm_download_progress(&download_ing);
    app->vm_download_progress(&download_failed);

    app->web_sync_image(image);
    app->vm_download_progress(&download_ing);
    app->vm_download_progress(&download_ing);
    app->vm_download_progress(&download_ing);
    app->vm_download_progress(&download_checking);
    app->vm_download_progress(&download_success);

    //1 *************************special unbind end**************************
*/

    return NULL;
}
#endif /* UNIT_TEST */
