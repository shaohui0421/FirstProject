#include "application_c_interfaces.h"
#include "application.h"
#include "common.h"
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif
#include "rc/rc_public.h"
#include "rc/rc_systeminfo.h"
#include "rc/rc_netif.h"
#ifdef __cplusplus
}
#endif

#define APPLICATION_ACTION_WITH_PARAMENT(action, parament)\
    int ret = 0;\
    Application* app = Application::get_application();\
    LOG_DEBUG("enter");\
    ret = app->action(parament);\
    return ret;

#define APPLICATION_ACTION_WITHOUT_PARAMENT(action)\
    int ret = 0;\
    Application* app = Application::get_application();\
    LOG_DEBUG("enter");\
    ret = app->action();\
    return ret;

int ui_get_about_info(struct about_info* about_info)
{
    int ret;
    Application* app;
    BasicInfo basicinfo;
    char hostname[64] = {0};

    if(about_info != NULL)
    {
        app = Application::get_application();
        app->ui_get_basic_info(&basicinfo);
        app->ui_get_host_name(hostname);

        strcpy(about_info->model, basicinfo.product_name.c_str());
        strcpy(about_info->hw_ver, basicinfo.hardware_version.c_str());
        strcpy(about_info->sw_ver, basicinfo.software_version.c_str());
        strcpy(about_info->os_ver, basicinfo.os_version.c_str());
        strcpy(about_info->sn, basicinfo.serial_number.c_str());
        strcpy(about_info->mac, basicinfo.mac.c_str());
        strcpy(about_info->wlan_mac, app->get_local_network_data().get_wireless_mac().c_str());
        strcpy(about_info->hostname, hostname);
        strcpy(about_info->ip, app->get_local_network_data().get_ip_info().ip.c_str());
        LOG_DEBUG("model=%s, hw_ver=%s, sw_ver=%s, os_ver=%s, sn=%s, mac=%s, wlan_mac=%s, hostname=%s, ip=%s", 
            about_info->model, about_info->hw_ver, about_info->sw_ver, about_info->os_ver, about_info->sn,
            about_info->mac, about_info->wlan_mac, about_info->hostname, about_info->ip);
        ret = SUCCESS;
    }
    else
    {
        ret = ERROR_INPUT;
    }

    return ret;
}

#define INET_ATON(cp, inp)\
    if (inet_aton(cp.c_str(), &inp) == 0) {\
       LOG_EMERG("inet_aton error");\
       LOG_PERROR(" ");\
    }

static void __networkinfo_transfer(rc_netifdev_t* info, NetworkInfo& buf)
{
    if(buf.dhcp)
    {
        info->iptype    =  RC_IPTYPE_DHCP;
    }
    else
    {
        info->iptype    =  RC_IPTYPE_STATIC;
    }

    if(buf.auto_dns)
    {
        info->dnstype   =  RC_DNSTYPE_DHCP;
    }
    else
    {
        info->dnstype   =  RC_DNSTYPE_STATIC;
    }
    
    INET_ATON(buf.ip, info->ip);
    INET_ATON(buf.submask,  info->netmask);
    INET_ATON(buf.gateway,  info->gateway);
    
    /*
     *if main_dns or back_dns is empty
     *we regard it as cleaning the configure in system
     *because these configures can be cleaned actually
     */
    if(buf.main_dns.empty())
    {
        info->dns1.s_addr = INADDR_NONE;
    }
    else
    {
        INET_ATON(buf.main_dns,   info->dns1);
    }
    if(buf.back_dns.empty())
    {
        info->dns2.s_addr = INADDR_NONE;
    }
    else
    {
        INET_ATON(buf.back_dns,   info->dns2);
    }

    
    if(buf.netcard_speed == 0)
    {
        info->status = 0;
    }
    else
    {
        info->status = 1;
    }
    info->link_speed = buf.netcard_speed;
    return;
}

#undef INET_ATON

static void __netifdev_transfer(rc_netifdev_t* info, NetworkInfo& buf)
{
    buf.dhcp =      (info->iptype==RC_IPTYPE_DHCP);
    buf.auto_dns =  (info->dnstype==RC_DNSTYPE_DHCP);
    buf.ip =        inet_ntoa(info->ip);
    buf.submask =   inet_ntoa(info->netmask);
    buf.gateway =   inet_ntoa(info->gateway);

    /*
     *if (INADDR_NONE == _dev->dns.s_addr) || (INADDR_ANY == _dev->dns.s_addr)
     *we would get "0.0.0.0" or "255.255.255.255" which is invalid
     *now we clean dns section in _network_info to unify ui
     */
    if((INADDR_NONE == info->dns1.s_addr) || (INADDR_ANY == info->dns1.s_addr))
    {
        buf.main_dns.clear();
    }
    else
    {
        buf.main_dns = inet_ntoa(info->dns1);
    }

    if((INADDR_NONE == info->dns2.s_addr) || (INADDR_ANY == info->dns2.s_addr))
    {
        buf.back_dns.clear();
    }
    else
    {
        buf.back_dns = inet_ntoa(info->dns2);
    }
}

int ui_get_net_info(rc_netifdev_t* info)
{
    ASSERT(info);
    int ret = 0;
    Application* app = Application::get_application();
    NetworkInfo buf;
    ret = app->ui_get_net_info(&buf);
    if(ret != SUCCESS)
    {
        return ret;
    }

    LOG_DEBUG("net info DHCP:%d, auto dns:%d, ip:%s, summask:%s, gateway:%s, main_dns:%s, back_dns:%s, speed:%d", 
                buf.dhcp, buf.auto_dns, buf.ip.c_str(), buf.submask.c_str(), buf.gateway.c_str(), buf.main_dns.c_str(), buf.back_dns.c_str(), buf.netcard_speed);
    __networkinfo_transfer(info, buf);
    return SUCCESS;
}

int ui_get_vm_net_info(rc_netifdev_t* info)
{
    ASSERT(info);
    int ret = 0;
    Application* app = Application::get_application();
    NetworkInfo buf;
    ret = app->ui_get_vm_net_info(&buf);
    if(ret != SUCCESS)
    {
        return ret;
    }
    __networkinfo_transfer(info, buf);
    return SUCCESS;

}

int ui_get_server_ip(char* ip)
{
    ASSERT(ip);
    APPLICATION_ACTION_WITH_PARAMENT(ui_get_server_ip, ip);
}

int ui_get_host_name(char* name)
{
    ASSERT(name);
    APPLICATION_ACTION_WITH_PARAMENT(ui_get_host_name, name);
}

int ui_get_terminal_mode(int* mode, int* readonly)
{
    //ASSERT(mode);
    //APPLICATION_ACTION_WITH_PARAMENT(ui_get_terminal_mode, mode);
    int ret = 0;
    if(mode==NULL || readonly==NULL)
    {
        ret = ERROR_INPUT;
    }
    else
    {
        Application* app = Application::get_application();
        ret = app->ui_get_terminal_mode(mode, readonly);
    }
    return ret;
}
int ui_get_connection_info(int* connected)
{
    ASSERT(connected);
    APPLICATION_ACTION_WITH_PARAMENT(ui_get_connection_info, connected);
}

void ui_get_last_logined_user(char *username, size_t user_len,
    char *password, size_t psw_len, int *remember_flag)
{
    int ret;
    struct UserInfo userinfo;
    Application *app = Application::get_application();
    ASSERT(username && password && remember_flag);

    LOG_DEBUG("UI requests Last Logined user info ...");
    memset(username, '\0', user_len);
    memset(password, '\0', psw_len);
    ret = app->ui_get_last_logined_user(&userinfo);
    if (ret == 0) {
        if (userinfo.username.length() >= user_len
            || userinfo.username.length() >= psw_len) {
            // Username or Password beyond max length.
            return;
        }
        *remember_flag = userinfo.remember_flag;
        if (*remember_flag) { // if remember flag is false, we do not return username & password.
            strncpy(username, userinfo.username.c_str(), userinfo.username.length());
            strncpy(password, userinfo.password.c_str(), userinfo.password.length());
            username[userinfo.username.length()] = '\0';
            password[userinfo.password.length()] = '\0';
        }
    }
}

int ui_localmode_quit(int result)
{
    APPLICATION_ACTION_WITH_PARAMENT(ui_localmode_quit, result);
}

int ui_localmode_checkerror(int result)
{
    APPLICATION_ACTION_WITH_PARAMENT(ui_localmode_checkerror, result);
}

int ui_account_login(char* username, char* passwd, int remember_flag)
{
    ASSERT(username && passwd);
    UserInfo  userinfo;
    LOG_DEBUG("ui_account_login %s, %s", gloox::password_codec_xor(username, true).c_str() , gloox::password_codec_xor(passwd, true).c_str());
    userinfo.username = username;
    userinfo.password = passwd;
    userinfo.remember_flag = remember_flag;
    APPLICATION_ACTION_WITH_PARAMENT(ui_account_login, userinfo);
    return ret;
}

int ui_bind_user(int bind)
{
    APPLICATION_ACTION_WITH_PARAMENT(ui_bind_user, bind);
}



int ui_check_admin_passwd(char* passwd)
{
    APPLICATION_ACTION_WITH_PARAMENT(ui_check_admin_passwd, passwd);
}

int ui_shutdown_terminal()
{
    APPLICATION_ACTION_WITHOUT_PARAMENT(ui_shutdown_terminal);
}

int ui_reboot_terminal()
{
     APPLICATION_ACTION_WITHOUT_PARAMENT(ui_reboot_terminal);
}

int ui_guest_login()
{
     APPLICATION_ACTION_WITHOUT_PARAMENT(ui_guest_login);
}

int ui_public_login()
{
     APPLICATION_ACTION_WITHOUT_PARAMENT(ui_public_login);
}

int ui_establish_connection()
{
     APPLICATION_ACTION_WITHOUT_PARAMENT(ui_establish_connection);
}
int ui_destroy_connection()
{
     APPLICATION_ACTION_WITHOUT_PARAMENT(ui_destroy_connection);
}
int ui_modify_passwd(const char* username, const char* password, const char* new_passwd)
{
    UserInfo buf;
    buf.username        = username;
    buf.password        = password;
    buf.new_password    = new_passwd;

    APPLICATION_ACTION_WITH_PARAMENT(ui_modify_passwd, buf);
}

int ui_enter_local_mode()
{
    APPLICATION_ACTION_WITHOUT_PARAMENT(ui_enter_local_mode);
}

int ui_enter_settings()
{
    APPLICATION_ACTION_WITHOUT_PARAMENT(ui_enter_settings);
}
int ui_notify_setting_result()
{
    APPLICATION_ACTION_WITHOUT_PARAMENT(ui_notify_setting_result);
}

void ui_save_setting_auth(const auth_info_t *auth_info)
{
    Application *app;
    AuthInfo new_authinfo;

    app = Application::get_application();
    
    if(auth_info != NULL)
    {
        new_authinfo.auth_type = auth_info->auth_type;
        new_authinfo.auth_user = auth_info->username;
        new_authinfo.auth_passwd = auth_info->password;
        app->ui_set_auth_info(new_authinfo);
    }

}

int ui_save_settings(const char* hostname, const char* serverip, struct rc_netifdev_t* net_info, struct rc_netifdev_t* vm_net_info, struct wifi_switch_t* sw, int mode, int* err)
{
    int ret = ERROR_INPUT;   //ERROR_INPUT: input param error, 0: need not set, 1: set without error 2 : set with error
    int flag = 0;
    Application* app; 
    string str_name;
    string str_ip;
    char* name = NULL;
    char* ip = NULL;
    int size = 64;
    NetworkInfo old_netinfo, new_netinfo, old_vmnetinfo, new_vmnetinfo;
    int old_mode;
    AuthInfo new_authinfo;
    int connected;
    int readonly;

    app = Application::get_application();
    
    //todo:check input ip valid
    //check mode & connected
    app->ui_clean_all_set_flag();
    if(err == NULL)
    {
        LOG_ERR("errno ptr is NULL!");
        ret = ERROR_INPUT;
        return ret;
    }
    else
    {
        *err = 0;
    }
     
    //if hostname different, set hostname
    if(hostname != NULL)
    {
        name = (char*)malloc(size);
        memset(name, 0, size);
        app->ui_get_host_name(name);
        if(strcmp(hostname, name) != 0)
        {
            LOG_DEBUG("hostname different! old:%s, new:%s", name, hostname);
            str_name = hostname;
            flag |= UI_HOSTNAME_SYNC;
        }  
    }
    
    //if server ip different, set server ip
    if(serverip != NULL)
    {
        ip = (char*)malloc(size);
        memset(ip, 0, size);
        app->ui_get_server_ip(ip);
        if(strcmp(serverip, ip) != 0)
        {
            LOG_DEBUG("server ip different! old:%s, new:%s", ip, serverip);
            str_ip = serverip;
            flag |= UI_SERVER_IP_SYNC;   
        }
    }
    
    //If the current is a wireless connecting, it does not set the net_info & vm_net_info
    int result = ui_net_status_query_result();

    //cannot change ip if wlan up
    if(result != 2) {
        if(net_info != NULL)
        {
            //if net info different, set net info
            app->ui_get_net_info(&old_netinfo);
            __netifdev_transfer(net_info, new_netinfo);
            LOG_DEBUG("1 new net info DHCP:%d, auto dns:%d, ip:%s, summask:%s, gateway:%s, main_dns:%s, back_dns:%s, speed:%d", 
                           new_netinfo.dhcp, new_netinfo.auto_dns, new_netinfo.ip.c_str(), new_netinfo.submask.c_str(), new_netinfo.gateway.c_str(), new_netinfo.main_dns.c_str(), new_netinfo.back_dns.c_str(), new_netinfo.netcard_speed);
            if((old_netinfo.auto_dns != new_netinfo.auto_dns) ||
                (old_netinfo.dhcp != new_netinfo.dhcp) ||
                (old_netinfo.ip.compare(new_netinfo.ip) != 0) ||
                (old_netinfo.submask.compare(new_netinfo.submask) != 0) ||
                (old_netinfo.gateway.compare(new_netinfo.gateway) != 0) ||
                (old_netinfo.main_dns.compare(new_netinfo.main_dns) != 0) ||
                (old_netinfo.back_dns.compare(new_netinfo.back_dns) != 0)
                )
            {
                LOG_DEBUG("netinfo different!");
                LOG_DEBUG("old net info DHCP:%d, auto dns:%d, ip:%s, summask:%s, gateway:%s, main_dns:%s, back_dns:%s, speed:%d", 
                    old_netinfo.dhcp, old_netinfo.auto_dns, old_netinfo.ip.c_str(), old_netinfo.submask.c_str(), old_netinfo.gateway.c_str(), old_netinfo.main_dns.c_str(), old_netinfo.back_dns.c_str(), old_netinfo.netcard_speed);
                LOG_DEBUG("new net info DHCP:%d, auto dns:%d, ip:%s, summask:%s, gateway:%s, main_dns:%s, back_dns:%s, speed:%d", 
                           new_netinfo.dhcp, new_netinfo.auto_dns, new_netinfo.ip.c_str(), new_netinfo.submask.c_str(), new_netinfo.gateway.c_str(), new_netinfo.main_dns.c_str(), new_netinfo.back_dns.c_str(), new_netinfo.netcard_speed);

                flag |= UI_NET_INFO_SYNC; 
            }
        }

        if(vm_net_info != NULL)
        {
            //if vm net info different, set vm net info
            app->ui_get_vm_net_info(&old_vmnetinfo);
            __netifdev_transfer(vm_net_info, new_vmnetinfo);
            if((old_vmnetinfo.auto_dns != new_vmnetinfo.auto_dns) ||
                (old_vmnetinfo.dhcp != new_vmnetinfo.dhcp) ||
                (old_vmnetinfo.ip.compare(new_vmnetinfo.ip) != 0) ||
                (old_vmnetinfo.submask.compare(new_vmnetinfo.submask) != 0) ||
                (old_vmnetinfo.gateway.compare(new_vmnetinfo.gateway) != 0) ||
                (old_vmnetinfo.main_dns.compare(new_vmnetinfo.main_dns) != 0) ||
                (old_vmnetinfo.back_dns.compare(new_vmnetinfo.back_dns) != 0)
                )
            {
                LOG_DEBUG("VM netinfo different!");
                LOG_DEBUG("old net info DHCP:%d, auto dns:%d, ip:%s, summask:%s, gateway:%s, main_dns:%s, back_dns:%s, speed:%d", 
                    old_vmnetinfo.dhcp, old_vmnetinfo.auto_dns, old_vmnetinfo.ip.c_str(), old_vmnetinfo.submask.c_str(), old_vmnetinfo.gateway.c_str(), old_vmnetinfo.main_dns.c_str(), old_vmnetinfo.back_dns.c_str(), old_vmnetinfo.netcard_speed);
                LOG_DEBUG("new net info DHCP:%d, auto dns:%d, ip:%s, summask:%s, gateway:%s, main_dns:%s, back_dns:%s, speed:%d", 
                           new_vmnetinfo.dhcp, new_vmnetinfo.auto_dns, new_vmnetinfo.ip.c_str(), new_vmnetinfo.submask.c_str(), new_vmnetinfo.gateway.c_str(), new_vmnetinfo.main_dns.c_str(), new_vmnetinfo.back_dns.c_str(), new_vmnetinfo.netcard_speed);
                flag |= UI_VM_NET_INFO_SYNC;
            }
        }
    }

    if(sw != NULL) {
         if (sw->new_wifi_status != sw->old_wifi_status || sw->new_wssid_status != sw->old_wssid_status) {
            flag |= UI_WIFI_STATUS_SYNC; 
         }
    }

    app->ui_get_connection_info(&connected);
    if(mode >= 0 && connected)
    {
        //if mode different, set mode
        app->ui_get_terminal_mode(&old_mode, &readonly);
        if(old_mode != mode)
        {
            LOG_DEBUG("mode different! old:%d, new:%d", old_mode, mode);
            flag |= UI_MODE_SYNC;   
        }
    }
    else
    {
        //*err |= UI_MODE_ERR;
        //LOG_ERR("can not set mode now! mode=%d, connected=%d", mode, connected);
    }
    


    app->ui_set_sync_flag(flag);

    if((flag&UI_HOSTNAME_SYNC)!=0)
    {
        ret = app->ui_set_host_name(str_name);
        if(ret < 0)
        {
            LOG_ERR("can not set host name now!, ret=%d", ret);
            *err |= UI_HOSTNAME_ERR;
        }
    }

    if((flag&UI_SERVER_IP_SYNC)!=0)
    {
        ret = app->ui_set_server_ip(str_ip);
        if(ret < 0)
        {
            LOG_ERR("can not set server ip now!, ret=%d", ret);
            *err |= UI_SERVER_IP_ERR;
        }
    }

    if((flag&UI_NET_INFO_SYNC)!=0)
    {
        ret = app->ui_set_net_info(new_netinfo);
        if(ret < 0)
        {
            LOG_ERR("can not set net info now!, ret=%d", ret);
            *err |= UI_NET_INFO_ERR;
        }
    }

    if((flag&UI_VM_NET_INFO_SYNC)!=0)
    {
        ret = app->ui_set_vm_net_info(new_vmnetinfo);
        if(ret < 0)
        {
            LOG_ERR("can not set vm net info now!, ret=%d", ret);
            *err |= UI_VM_NET_INFO_ERR;
        }
    }

    if((flag&UI_MODE_SYNC)!=0)
    {
        ret = app->ui_set_terminal_mode(mode);
        if(ret < 0)
        {
            LOG_ERR("can not set terminal mode now!, ret=%d", ret);
            *err |= UI_MODE_ERR;
        }
    }

    if ( (flag&UI_WIFI_STATUS_SYNC) != 0) {
        if (is_wifi_terminal()) {
            ret = app->ui_set_wifi_mode(*sw);
            if(ret < 0)
            {
                LOG_ERR("can not ui_set_wifi_mode now!, ret=%d", ret);
                *err |= UI_MODE_ERR;
            } 
        }
    }
    
    if(flag == 0)
    {
        //not change any setting
        ret = 0;
        app->ui_show_next_picture();
    }
    else if(*err == 0)
    {
        ret = 1;
    }
    else 
    {
    	LOG_ERR("can not set config now!, err=%x", *err);
    	//_ui_setting_error = *err;
    	app->ui_set_error_flag(*err);
    	app->ui_show_next_picture();

        ret = 2;
    }

    if(name)
    {
        free(name);
    }
    if(ip)
    {
        free(ip);
    }
    
    return ret;
}

void ui_cancel_settings()
{
    Application* app;

    LOG_DEBUG("enter ui_cancel_settings");
    app = Application::get_application();
    app->ui_show_next_picture();
}

void ui_cancel_auto_shutdown()
{
    Application* app;
    LOG_DEBUG("ui cancel auto shutdown");
    app = Application::get_application();
    app->ui_cancel_auto_shutdown();
}

void ui_show_retry_downloading(){
    Application* app;

    LOG_DEBUG("enter ui_show_retry_downloading");
    app = Application::get_application();
    app->ui_show_retry_downloading();
}
// for web & 802.1x auth
void ui_get_auth_info(auth_info_t *info)
{
    Application *app;
    struct AuthInfo auth_info;

    if (!info) {
        return;
    }

    app = Application::get_application();
    app->ui_read_auth_config(auth_info);

#if 0
    LOG_DEBUG("Auth Info:");
    LOG_DEBUG("  auth_type: %d", auth_info.auth_type);
    LOG_DEBUG("  auto_connect: %d", auth_info.auto_connect);
    LOG_DEBUG("  username: %s", auth_info.auth_user);
    LOG_DEBUG("  password: %s", auth_info.auth_passwd);
#endif
    
	if (auth_info.auth_user.length() > 128 || auth_info.auth_passwd.length() > 128) {
        return;
    }

    // limit username max long is 32 bit
    info->auth_type =auth_info.auth_type;
    info->auto_connect = auth_info.auto_connect;
    strcpy(info->username, auth_info.auth_user.c_str());
    strcpy(info->password, auth_info.auth_passwd.c_str());
}

int ui_get_auth_status()
{
    Application *app = Application::get_application();
    return app->get_auth_manager()->getAuthStatus();
}

void ui_save_auth_info(const auth_info_t *info)
{
    Application *app;
    struct AuthInfo auth_info;
    auth_info.auth_type = info->auth_type;
    auth_info.auth_user = info->username;
    auth_info.auth_passwd = info->password;

    LOG_DEBUG("enter Authentication process");
    app = Application::get_application();
    app->ui_save_auth_config(auth_info);
}

void ui_save_auto_connect_info(int auto_connect)
{
    Application *app;

    LOG_DEBUG("enter ui_save_auto_connect_info %d", auto_connect);
    app = Application::get_application();
    app->get_auth_manager()->saveAutoConnect(auto_connect);
}

void ui_tips_ok(int type)
{
    Application* app;
    app = Application::get_application();
    app->ui_tips_ok(type);
}

int ui_new_deploy(const int mode, const char* bind_user)
{
    LOG_DEBUG("mode=%d, bind user=%s", mode, bind_user);
    ModeInfo buf;

    buf.mode = (Mode)mode;
    buf.bind_user.username = bind_user;
    APPLICATION_ACTION_WITH_PARAMENT(ui_pass_new_deploy_modeinfo,buf);
    return SUCCESS;
}

int ui_get_bt_service(int *bt_service)
{
    int ret = 0;
    if(bt_service==NULL)
    {
        ret = ERROR_INPUT;
    }
    else
    {
        Application* app = Application::get_application();
        ret = app->ui_get_bt_service(bt_service);
    }
    return ret;
}

int ui_save_bt_service(int bt_service)
{
    Application* app;
	int ret = 0;
    LOG_DEBUG("ui save bt service");
    app = Application::get_application();
    ret = app->ui_save_bt_service(bt_service);
	return ret;
}

int ui_get_power_boot(int *power_boot)
{
    int ret = 0;
    if(power_boot==NULL)
    {
        ret = ERROR_INPUT;
    }
    else
    {
        Application* app = Application::get_application();
        ret = app->ui_get_power_boot(power_boot);
    }
    return ret;
}

int ui_save_power_boot(int power_boot)
{
    APPLICATION_ACTION_WITH_PARAMENT(ui_save_power_boot, power_boot);
}


int ui_is_new_deploy(int *is_new_deploy)
{
    int ret = 0;
    if(is_new_deploy==NULL)
    {
        ret = ERROR_INPUT;
    }
    else
    {
        Application* app = Application::get_application();
        ret = app->ui_is_new_deploy(is_new_deploy);
    }
    return ret;
}

int ui_is_wifi_terminal(int *wifi_terminal)
{
    int ret = 0;
    if(wifi_terminal==NULL)
    {
        ret = ERROR_INPUT;
    }
    else
    {
        Application* app = Application::get_application();
        ret = app->ui_is_wifi_terminal(wifi_terminal);
    }
    return ret;
}

/**
*function : judget if vmmode is emulation
*
*int : 0: start vmmode is not emulation mode 1: start vmmode is emulation
*/
int ui_vm_start_is_emulaton_vmmode()
{
    Application* app = Application::get_application();
    return  app->get_vm()->get_vmManage()->get_start_vm_is_emulation();
}

/**
*function : get ui_userinfomgr_permitted status
*
*@return: ui permitted status
*/
int ui_userinfomgr_permitted()
{
     Application* app = Application::get_application();
     return  app->get_UsrUserInfoMgr()->UI_permitted();
}


int ui_enter_wifi_config()
{
    APPLICATION_ACTION_WITHOUT_PARAMENT(ui_enter_wifi_config);
}
int ui_enter_auth_config()
{
    APPLICATION_ACTION_WITHOUT_PARAMENT(ui_enter_auth_config);
}

int ui_notify_wifi_config_result()
{
    APPLICATION_ACTION_WITHOUT_PARAMENT(ui_notify_wifi_config_result);
}
int ui_notify_auth_config_result()
{
    APPLICATION_ACTION_WITHOUT_PARAMENT(ui_notify_auth_config_result);
 }

void ui_wireless_netcard_enable(const int mode)
{
    Application* app;
    LOG_DEBUG("ui wireless netcard enable");
    app = Application::get_application();
    app->get_wifi_interactive()->wifi_enable_button_handle(mode);
}

void ui_wireless_netcard_disconnect(void)
{
    Application* app;
    LOG_DEBUG("ui wireless netcard disconnect");
    app = Application::get_application();
    app->get_wifi_interactive()->wifi_disconnect_button_handle();
}

void ui_wifi_authenticate_enter(const int type, const upload_config_s config)
{
    Application* app;
    LOG_DEBUG("ui wifi authenticate enter");
    app = Application::get_application();
    app->get_wifi_interactive()->wifi_enter_button_handle(type, config);
}

void ui_wireless_netcard_connect(const int hide_net, const ui_scanresult *data)
{
    Application* app;
    LOG_DEBUG("ui wireless netcard connect");
    app = Application::get_application();
    app->get_wifi_interactive()->wifi_connect_button_handle(hide_net, data);
}

ui_scanresult* ui_wifi_list_scan_result(int *length)
{
    Application* app;
    ui_scanresult* ret;
    LOG_DEBUG("ui wifi list scan result");
    app = Application::get_application();
    ret = app->get_wifi_interactive()->wifi_list_scan_result(length);
    return ret;
}

ui_wifiinfo* ui_wifi_status_query_result(void)
{
    Application* app;
    ui_wifiinfo* ret;
    LOG_DEBUG("ui wifi status query result");
    app = Application::get_application();
    ret = app->get_wifi_interactive()->wifi_status_query_result();
    return ret;
}

int ui_wireless_netcard_query_result(void)
{
    Application* app;
    int ret;
    LOG_DEBUG("wireless netcard query result");
    app = Application::get_application();
    ret = app->get_wifi_interactive()->wireless_netcard_query_result();
    return ret;
}

int ui_wifi_forget_net_handle(const int net_id)
{
    Application* app;
    int ret;
    LOG_DEBUG("wifi_forget_net_handle");
    app = Application::get_application();
    ret = app->get_wifi_interactive()->wifi_forget_net_handle(net_id);
    return ret;
}

int ui_wifi_saved_net_query_result(const char *ssid, const char *ssid_mac, const int proto)
{
    Application* app;
    int ret;
    LOG_DEBUG("wifi_saved_net_query_result");
    app = Application::get_application();
    ret = app->get_wifi_interactive()->wifi_saved_net_query_result(ssid, ssid_mac, proto);
    LOG_DEBUG("wifi_saved_net_query_result %d", ret);
    return ret;
}

int ui_net_status_query_result(void)
{
    Application* app;
    int ret;
    LOG_DEBUG("net status query result");
    app = Application::get_application();
    ret = app->get_local_network_data().get_net_status();
    LOG_DEBUG("net status query result %d", ret);
    return ret;
}

void ui_cancel_wifi_config()
{
    Application* app;

    LOG_DEBUG("enter ui_cancel_wifi_config");
    app = Application::get_application();
    app->ui_show_wifi_next_picture();
}

void ui_wifi_cancel_button_handle(void)
{
    Application* app;
    LOG_DEBUG("wifi_cancel_button_handle");
    app = Application::get_application();
    app->get_wifi_interactive()->wifi_cancel_button_handle();
}

void ui_wifi_cancel_show_wifipwd()
{
    Application* app;
    LOG_DEBUG("ui_wifi_cancel_show_wifipwd");
    app = Application::get_application();
    app->get_wifi_interactive()->wifi_cancel_show_wifipwd();
}

void ui_cancel_auth_config()
{
    Application* app;

    LOG_DEBUG("enter ui_cancel_wifi_config");
    app = Application::get_application();
    app->ui_show_auth_next_picture();
}

int ui_usb_copy_base(void)
{
    APPLICATION_ACTION_WITHOUT_PARAMENT(ui_usb_copy_base);
}

int ui_get_hide_guest_login()
{
    Application* app;
    app = Application::get_application();
    return app->ui_get_hide_guest_login();
}


/**
* get ssid_white_list.ini data
* @*ssid[128]：保存ssidwhite信息
* @max_list_num：ssidwhite 最大支持数
*
* return :ini文件中白名单个数
*/
int ui_getssid_whitelist(char (*ssid)[128], int max_list_num)
{
    Application *app;
    int ret = 0, i = 0, lenth = 0, size = 0;
    vector<string> ssidlist;

    if (ssid == NULL) {
        return ret;
    }

    app = Application::get_application();

    ret = (app->get_UsrUserInfoMgr()->readssidWhiteList(ssidlist));
    if (ret != 0) {
       LOG_ERR("ui_getssid_whitelist read error %d", ret);
       return 0;
    }

    size = ssidlist.size();
    if (size > max_list_num) {
        size = max_list_num;
    } 

    for (i = 0; i < size; i++) {
        lenth = strlen(ssidlist[i].c_str()) + 1;
        if (lenth > 128){
            lenth = 128 - 1; 
            LOG_INFO("ui_getssid_whitelist ssid lenth is too long");
        }
        strncpy(ssid[i], ssidlist[i].c_str(), lenth);
    }

    ret = size;
    return ret;
}

/**
* get ssid_white_list.ini data num
* return :ini文件中白名单个数
*/
int ui_get_whitelist_num(void)
{
    Application *app;
    app = Application::get_application();
    vector<string> ssidlist;
    int ret = 0;
    
    ret = app->get_UsrUserInfoMgr()->readssidWhiteList(ssidlist);
    if (ret != 0) {
        LOG_ERR("ui_getssid_whitelist read error %d", ret);
        return 0;
    }

    return ssidlist.size();
}

int ui_set_resolution(const int width, const int height)
{
    int ret = 0;
    Application *app = Application::get_application();
    
    ret = app->get_device_interface()->setResolution(width, height);
    if(ret != 0) {
        LOG_ERR("store Resolution List error\n");
    }
    return ret;
}

//TODO zjj 是否要改成ui_res_info**
int ui_get_resolution_list(ui_res_info *res_info, int max_list_size)
{
    int size = 0, ret = 0;
    Application *app = Application::get_application();
    vector<DevResInfo> resList;

    int width = 0, height = 0;
    int isize = 0;

    ret = app->get_device_interface()->getResolutionList(resList);
    if(ret != 0) {
        LOG_ERR("ui_get_resolution_list error, list is default\n");
    }
    if (res_info == NULL) {
        LOG_ERR("res_info is NULL");
        return 0;
    }

    size = resList.size();
    if (size > max_list_size) {
        size = max_list_size;
    }

    for (int i = 0; i < size; i++) {
        sscanf(resList[i].res.c_str(), "%dx%d", &width, &height);
        if (width < LIMIT_DISPLAY_WIDTH || height < LIMIT_DISPLAY_HEIGHT|| (width * height < LIMIT_DISPLAY_WIDTH * LIMIT_DISPLAY_HEIGHT_SPEC)) {
            continue;
        }
        res_info[isize].width = width;
        res_info[isize].height = height;
        res_info[isize].flag = resList[i].flag;
        LOG_DEBUG("res_info[i].flag  =%d,res_info[i].dpi = %dx%d\n",res_info[isize].flag, res_info[isize].width, res_info[isize].height);
        isize++;
    }

    return isize;
}

int ui_set_display_info(const ui_res_info *res_info)
{
    Application *app = Application::get_application();
    struct DisplayInfo dpi_info;

    if(res_info == NULL) {
        return -1;
    }

    dpi_info.width = res_info->width;
    dpi_info.height = res_info->height;
    dpi_info.flag = res_info->flag;
    dpi_info.custom = res_info->custom;

    app->get_UsrUserInfoMgr()->set_display_info(dpi_info);
    return 0;
}

void ui_get_display_info(int* width, int* height, int* custom)
{
    if (!width || !height || !custom) {
        return;
    }
    Application *app = Application::get_application();
    app->get_UsrUserInfoMgr()->get_display_info(width, height, custom);
}

int ui_is_display_info_ini_exist()
{
    Application *app = Application::get_application();
    if (app->get_UsrUserInfoMgr()->is_display_info_ini_exist()) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int ui_delete_display_info_ini()
{
    Application *app = Application::get_application();
    if (app->get_UsrUserInfoMgr()->delete_display_info_ini() == 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int ui_set_ext_display_info(const ui_res_info *res_info, int num)
{
    Application *app = Application::get_application();
    struct DisplayInfo dpi_info;

    if (res_info == NULL) {
        return -1;
    }

    dpi_info.width = res_info->width;
    dpi_info.height = res_info->height;
    dpi_info.flag = res_info->flag;
    dpi_info.custom = res_info->custom;

    app->get_UsrUserInfoMgr()->set_ext_display_info(dpi_info, num);

    return 0;
}

void ui_get_ext_display_info(ui_ext_res_info *res_info)
{
    if (!res_info) {
        return;
    }
    Application *app = Application::get_application();
    app->get_UsrUserInfoMgr()->get_ext_display_info((ExtDisplayInfo *)res_info);
}

int ui_is_display_info_section_exist(const char *section)
{
    Application *app = Application::get_application();
    if (app->get_UsrUserInfoMgr()->is_display_info_section_exist(section)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int ui_delete_resolution_info_section()
{
    Application *app = Application::get_application();
    if (app->get_UsrUserInfoMgr()->delete_resolution_info_section() == 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void ui_show_dialog_tips_dpi(void *callback1, void* args1, void* callback2, void* args2)
{
    l2u_show_dialog_save_dpi(callback1, args1, callback2, args2);
}

int ui_regex_match(const char *input_str, const char *regex_str)
{
    if (input_str == NULL || regex_str == NULL) {
        return -1;
    }
    return regex_match(input_str, regex_str);
}

int ui_upgrade_for_class(void)
{
    APPLICATION_ACTION_WITHOUT_PARAMENT(ui_upgrade_for_class);
}

void ui_set_hdmiaudio_info(int hdmiaudio)
{
    Application *app = Application::get_application();
    app->get_device_interface()->setHdmiVoiceEnable(hdmiaudio);
}

int ui_get_hdmiaudio_info()
{
    Application *app = Application::get_application();
    return app->get_device_interface()->getHdmiVoiceEnable();
}

int ui_is_hdmi_connected()
{
    Application *app = Application::get_application();
    if (app->get_device_interface()->isHdmiConnected()) {
        return 1;
    }
    return 0;
}

void ui_get_current_dpi(char *width, char *height)
{
    string width_str;
    string height_str;
    Application *app = Application::get_application();
    
    app->get_device_interface()->getCurResolution(width_str, height_str, IS_RES_CURRENT);
    strcpy(width, width_str.c_str());
    strcpy(height, height_str.c_str());
}

int ui_get_wired1xauth_exist()
{
    Application *app = Application::get_application();

    return app->get_auth_manager()->getAuthExist();
}

int ui_delete_auth_info()
{
    Application *app = Application::get_application();

    return app->get_UsrUserInfoMgr()->delete_auth_info();
}

void ui_save_boot_speedup(int boot_speedup)
{
    Application *app = Application::get_application();
    app->get_UsrUserInfoMgr()->set_boot_speedup(boot_speedup);
}


void ui_save_e1000_netcard(int e1000_netcard)
{
    Application *app = Application::get_application();
    app->get_UsrUserInfoMgr()->set_e1000_netcard(e1000_netcard);
}

int ui_using_e1000_netcard()
{
    string ostype;
    int is_emulation = 0;
    Application *app = Application::get_application();
    app->get_vm()->vm_get_image_ostype(ostype);
    is_emulation = app->get_vm()->get_vmManage()->get_start_vm_is_emulation();

    if (is_emulation && ostype == OSTYPE_WINDOWS_7 && (!app->is_dev_wlan_up())) {
        if (app->get_UsrUserInfoMgr()->is_using_e1000_netcard()) {
            return 1;
        } else {
            return 0;
        }
    }
    return -1;
}

void ui_save_usb_emulation(int usb_emulation)
{
    Application *app = Application::get_application();
    app->get_UsrUserInfoMgr()->set_usb_emulation(usb_emulation);
}

int ui_is_usb_emulation()
{
    string ostype;
    int is_emulation = 0;
    Application *app = Application::get_application();
    app->get_vm()->vm_get_image_ostype(ostype);
    is_emulation = app->get_vm()->get_vmManage()->get_start_vm_is_emulation();

    if (is_emulation && ostype == OSTYPE_WINDOWS_7) {
        if (app->get_UsrUserInfoMgr()->is_usb_emulation()) {
            return 1;
        } else {
            return 0;
        }
    }
    return -1;
}

int ui_is_using_app_layer()
{
    int is_app_layer;

    Application *app = Application::get_application();
    is_app_layer = app->get_UsrUserInfoMgr()->get_app_layer_switch();

    return is_app_layer;
}

void ui_set_app_layer_status(int status)
{
    Application *app = Application::get_application();
    app->get_UsrUserInfoMgr()->set_app_layer_switch(status);
}

void ui_show_dialog_tips_layer(void *callback1, void* args1, void* callback2, void* args2)
{
    l2u_show_dialog_layer_manage(callback1, args1, callback2, args2);
}

void ui_reset_logic_machine()
{
    Application *app = Application::get_application();
    
    rc_system("touch " CLIENT_ALLOW_IMAGE_SYNC_FLAG_FILE"");
    app->logic_reset_status_machine();
}

int ui_using_powerboot()
{
    int powerboot =0;
    Application *app = Application::get_application();
    powerboot = app->get_device_interface()->getPowerBootState();
    return powerboot;
}



#if 0
int ui_set_net_info(rc_netifdev_t* info)
{
    ASSERT(info);
    NetworkInfo buf;
    __netifdev_transfer(info, buf);
    APPLICATION_ACTION_WITH_PARAMENT(ui_set_net_info, buf);
}

int ui_set_vm_net_info(rc_netifdev_t* info)
{
    ASSERT(info);
    NetworkInfo buf;
    __netifdev_transfer(info, buf);
    APPLICATION_ACTION_WITH_PARAMENT(ui_set_vm_net_info, buf);
}

int ui_set_server_ip(char* ip)
{
    ASSERT(ip);
    APPLICATION_ACTION_WITH_PARAMENT(ui_set_server_ip, ip);
}

int ui_set_host_name(char* name)
{
    ASSERT(name);
    APPLICATION_ACTION_WITH_PARAMENT(ui_set_host_name, name);
}

int ui_set_terminal_mode(int mode)
{
    APPLICATION_ACTION_WITH_PARAMENT(ui_set_terminal_mode, mode);
}
#endif

int ui_get_display_port_number()
{
    int ret = 0;
    Application *app = Application::get_application();

    ret = app->get_device_interface()->getDisplayPortNumber();

    return ret;
}

int ui_get_display_port_connected_number()
{
    int ret = 0;
    Application *app = Application::get_application();

    ret = app->get_device_interface()->getDisplayConnectedNumber();

    return ret;
}

int ui_is_display_connected(int port)
{
    int ret = 0;
    Application *app = Application::get_application();

    ret = app->get_device_interface()->isDisplayConnected(port);

    return ret;
}

int ui_is_display_enable(int port)
{
    int ret = 0;
    Application *app = Application::get_application();

    ret = app->get_device_interface()->isDisplayEnable(port);

    return ret;
}

int ui_get_display_resolution_list(int port, ui_res_info *res_info, int max_list_size)
{
    int size = 0, ret = 0;
    Application *app = Application::get_application();
    vector<DevResInfo> resList;
    int width = 0, height = 0, isize = 0;

    ret = app->get_device_interface()->getDisplayResolutionList(port, resList);
    if (ret == -2) {
        LOG_ERR("ui_get_resolution_list is no connected\n");
        return 0;
    }

    if (res_info == NULL) {
        LOG_ERR("res_info is NULL");
        return 0;
    }

    size = resList.size();
    if (size > max_list_size) {
        size = max_list_size;
    }

    for (int i = 0; i < size; i++) {
        sscanf(resList[i].res.c_str(), "%dx%d", &width, &height);
        if (width < EXT_LIMIT_DISPLAY_WIDTH || height < EXT_LIMIT_DISPLAY_HEIGHT || (width * height < EXT_LIMIT_DISPLAY_WIDTH * EXT_LIMIT_DISPLAY_HEIGHT)) {
            continue;
        }

        res_info[isize].width = width;
        res_info[isize].height = height;
        res_info[isize].flag = resList[isize].flag;
        LOG_DEBUG("res_info[%d].flag  =%d,res_info[%d].dpi = %dx%d\n", isize, res_info[isize].flag, isize, res_info[isize].width, res_info[isize].height);
        isize++;
    }

    return isize;
}
