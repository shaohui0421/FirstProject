#ifndef _AUTH_MANAGER_H
#define _AUTH_MANAGER_H

#include "common.h"
#include "local_network_data.h"

//auth timeout: 90s
#define AUTHENTICATE_TIMEOUT  (90)
#define WIRED_WPA_CONF_PATH   "/etc/wpa_supplicant/wired_wpa_supplicant.conf"

enum AuthType
{
    AUTH_NONE    = 0x00,    //no auth
    AUTH_DOT1X   = 0x01,    //802.1x auth
    AUTH_WEB     = 0x02,    //web auth
};

enum AuthAutomatic {
    DISAUTO_CONNECT,
    AUTO_CONNECT,
};
/*
enum AuthStatus
{
    STAT_AUTH_UNKNOWN   = -1,
    STAT_AUTH_NONE      = 0,
    STAT_AUTH_PASS      = 1,
    STAT_AUTH_FAIL      = 2,
    STAT_AUTHING        = 3,
    STAT_AUTH_TIMEOUT   = 4,
};
*/
enum AuthErr
{
    AUTH_OK                = 0,
    AUTH_ERR_USER_EMPTY    = -1,
    AUTH_ERR_SAVE_FAIL     = -2,
};

class AuthManager
{
public:
    AuthManager();
    virtual ~AuthManager() {}
    //void getAuthIni(int *auth_type, string &username, string &password);
    void getAuthIni(struct AuthInfo &info);
    void saveAuthIni(struct AuthInfo &auth_info);
    void saveAutoConnect(int auto_connect);
    int startAuthentication();
    int stopAuthentication();
    int getAuthStatus();
    int getAuthExist();
    void authResultHandle(int result_status);
    void authEnvHandle(int result_status);

private:
    net::WiredAuthConfig getConfig();
    int _auth_type;
    int _auto_connect;
    string _auth_user;
    string _auth_passwd;

};

/*
class Dot1xAuthManager : public AuthManager
{
public:
    Dot1xAuthManager() : AuthManager(AUTH_DOT1X, STAT_AUTH_UNKNOWN) {}
    virtual ~Dot1xAuthManager() {}    
    int init_auth() {}
    int start_auth() {}
    int stop_auth() {}
};
*/
#endif //_AUTH_MANAGER_H
