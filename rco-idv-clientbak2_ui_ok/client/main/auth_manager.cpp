#include "user_mgr.h"
#include "auth_manager.h"
#include "local_network_data.h"
#include "application.h"
#include "ui_api.h"

AuthManager::AuthManager()
    :_auth_type(AUTH_NONE)
    ,_auto_connect(0)
    ,_auth_user("")
    ,_auth_passwd("")
{
}

void AuthManager::getAuthIni(struct AuthInfo &info)
{
    UserInfoMgr usermgr;

    usermgr.getAuthInfo(_auth_type, _auto_connect, _auth_user, _auth_passwd);
    info.auth_type = _auth_type;
    info.auto_connect = _auto_connect;
    info.auth_user = _auth_user;
    info.auth_passwd = _auth_passwd;
}


void AuthManager::saveAuthIni( struct AuthInfo &auth_info)
{
    UserInfoMgr usermgr;
    
    _auth_type = auth_info.auth_type;
    _auth_user = auth_info.auth_user;
    _auth_passwd = auth_info.auth_passwd;
    usermgr.setAuthInfo(auth_info);
}

void AuthManager::saveAutoConnect(int auto_connect)
{
    UserInfoMgr usermgr;
    _auto_connect = auto_connect;

    usermgr.setAuthConnect(auto_connect);
}

int AuthManager::startAuthentication()
{
    Application* app;
    int ret = 0;

    LOG_DEBUG("enter %s", __func__);
    
    app = Application::get_application();
    //show ui
    if (_auth_user.empty() || _auth_passwd.empty()) {
        l2u_show_net_auth_winbtn(UI_TIPS_NET_AUTH_FAIL_USER_ERR);
        l2u_show_net_auth(UI_TIPS_NET_AUTH_FAIL_USER_ERR);
        return -1;
    }

    if (_auth_type == AUTH_NONE) {
        l2u_show_net_auth_winbtn(UI_TIPS_NET_AUTH_FAIL);
        l2u_show_net_auth(UI_TIPS_NET_AUTH_FAIL);
        return -1;
    }

    //check wired plugged
    if (app->is_wired_plugged() == false) {
        l2u_show_net_auth_winbtn(UI_TIPS_NET_AUTH_FAIL);
        l2u_show_net_auth(UI_TIPS_NET_AUTH_FAIL);
        return -1;
    }

    //start auth
    l2u_show_net_auth_winbtn(UI_TIPS_NET_AUTHING);
    l2u_show_net_auth(UI_TIPS_NET_AUTHING);

    ret = app->get_local_network_data().connect_wired_auth(getConfig());
    return ret;
}

int AuthManager::stopAuthentication()
{
    Application* app;
    int ret = -1;

    LOG_DEBUG("enter %s", __func__);

    if (_auth_type != AUTH_NONE) {
        l2u_show_net_auth_winbtn(UI_TIPS_NET_AUTH_FAIL);
        l2u_show_net_auth(UI_TIPS_NET_AUTH_FAIL);
        return -1;
    }

    //cancel auth
    app = Application::get_application();
    ret = app->get_local_network_data().cancel_wired_auth();
    if (ret == 0) {
        l2u_show_net_auth_winbtn(UI_TIPS_NET_AUTH_DISCONNECT_SUCCESS);
        l2u_show_net_auth(UI_TIPS_NET_AUTH_DISCONNECT_SUCCESS);
    } else {
        LOG_DEBUG("stopAuthentication failed")
    }

    return ret;
}

int AuthManager::getAuthStatus()
{
    Application* app;
    int ret;

    app = Application::get_application();
    ret = app->get_local_network_data().get_wired_auth_status();
    return ret;
}

int AuthManager::getAuthExist()
{
    Application* app;
    int ret;

    app = Application::get_application();
    ret = app->get_local_network_data().get_wired_auth_exist();
    return ret;
}

void AuthManager::authResultHandle(int result_status)
{
#if 0 
    // if auth env not exist we should not show ui about 1x / bugid:567950
    Application* app;

    app = Application::get_application();
    if (app->get_local_network_data().get_wired_auth_exist() < 0) {
        return;
    }
#endif

    switch (result_status) {
    case net::ETH_EAP_AUTH_SUCCESS:
        l2u_show_net_auth_winbtn(UI_TIPS_NET_AUTH_SUCCESS);
        l2u_show_net_auth(UI_TIPS_NET_AUTH_SUCCESS);
        break;
    case net::ETH_EAP_AUTH_FAILED:
        l2u_show_net_auth_winbtn(UI_TIPS_NET_AUTH_FAIL);
        l2u_show_net_auth(UI_TIPS_NET_AUTH_FAIL);
        break;
    default:
        break;
    }

}

void AuthManager::authEnvHandle(int result_status)
{
    switch (result_status) {
    case net::ETH_EAP_AUTH_EXIST:
        l2u_show_net_auth_winbtn(UI_TIPS_NET_AUTH_EXIST);
        break;
    case net::ETH_EAP_AUTH_NOT_EXIST:
        l2u_show_net_auth_winbtn(UI_TIPS_NET_AUTH_DISEXIST);
        break;
    default:
        break;
    }
}

net::WiredAuthConfig AuthManager::getConfig()
{    
    net::WiredAuthConfig config;
    if (_auth_type == AUTH_NONE) {
        config.type = net::WiredAuthNone;
    } else if (_auth_type == AUTH_DOT1X) {
        config.type = net::WiredAuthDot1x;
    } else if (_auth_type == AUTH_WEB) {
        config.type = net::WiredAuthWeb;
    }
    if (_auto_connect == AUTO_CONNECT) {
        config.saved = TRUE;
    } else if (_auto_connect == DISAUTO_CONNECT) {
        config.saved = FALSE;
    }
    config.identity = _auth_user;
    config.password = _auth_passwd;
    config.saved = _auto_connect;
    config.eap = 0;
    config.flags = 0;

    LOG_INFO("----------------------AUTH CONFIG----------------------");
    LOG_INFO("type:     %d", config.type);
    LOG_INFO("identity: %s", gloox::password_codec_xor(config.identity.c_str(), true).c_str());
    LOG_INFO("password: %s", gloox::password_codec_xor(config.password.c_str(), true).c_str());
    LOG_INFO("saved:    %d", config.saved);
    LOG_INFO("eap:      %d", config.eap);
    LOG_INFO("flags:    %d", config.flags);
    LOG_INFO("-------------------------------------------------------");
    return config;
}


