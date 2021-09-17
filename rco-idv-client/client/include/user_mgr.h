/**
 * Copyright(C) 2017 Ruijie Network Inc. All rights reserved.
 *
 * user_mgr.h
 * Original Author: yejx@ruijie.com.cn, 2017-2-18
 *
 * IDV client local user info manager.
 * Header file, defines the class.
 */
#ifndef _USER_MGR_H_
#define _USER_MGR_H_

#include "data_struct.h"
#include <vector>
#include <map>
#include "user_db.h"

using std::vector;
using std::map;

/* Error code definition */
enum {
    USRERR_PSWERROR = 1,    // password incorrect
    USRERR_NOMEM = 2,       // No enough memory
    USRERR_INVAL = 3,       // argument invalid
    USRERR_NOENTRY = 4,     // some database entries not found
    USRERR_NOSECTIONS = 5,  // No any sections found
    USRERR_DBRFAIL = 6,     // database read failed
    USRERR_DBWFAIL = 7,     // database write failed
    USRERR_NOUSER = 8,      // User not found
    USRERR_NONETDISK = 9,   // User netdisk info not found
    USRERR_NOGROUP = 10,    // User group policy not found
    USRERR_NOPUBLIC = 11,   // public policy not found
    USRERR_NOADDOMAIN = 12, // ad domain not found
    /* add new error code here. */
    USRERR_MAXNO,
};

class UserInfoMgr {
public:
    UserInfoMgr() {}
    ~UserInfoMgr() {}
    const string &usrmgr_strerror(int err);

    int UserLoginAuthentication(Mode mode, string &username, string &password,
        struct UserInfo *user_info);
    int UserLoginedInfoStore(Mode mode, struct UserInfo &user_info);
    int PublicPolicyStore(struct PolicyInfo &public_policy);
    int PublicPolicyRead(struct PolicyInfo *public_policy);

    // add for local userinfo database update.
    int UserInfoInitconfig();
    int UserInfoWrite(struct UserInfo &user_info);
    int UserInfoRead(string &username, struct UserInfo *user_info);
    int UserInfoDelete(string &username);
    int UserGroupPolicyDelete(int &group_id);
    int getAllUsers(vector<string> &users);
    int updateAllUsersInfo(const vector<UserInfo> &usersinfo);

    // add for logined user saved & state machine reset.
    void loadSavedStatus();
    void permitUI(bool permitted);
    void saveCurrentState(int state);
    int  delete_user_status_ini();
    void setUserLogined(bool logined);
    void saveLoginedUser(string &username);
    void saveLastLoginedUser(string &username, string &password, int &remember_flag);
    void getLastLoginedUser(string *username, string *password, int *remember_flag);
    int  readssidWhiteList(vector<string> &whitelist);
    void storessidWhiteList(vector<string> &whitelist);
    int  deletessidWhite();
    bool UI_permitted() { return _ui_permit; }
    int get_saved_state() { return _saved_state; }
    bool user_logined() { return _saved_logined; }
    string getCurrentLoginedUser() { return _logined_user; }
    bool on_local_mode() { return _on_localmode; }
    void enter_local_mode();
    void leave_local_mode();
    int convert_to_short_name(string &full_name);

    // add for web & 802.1x authentication
    int AuthIniInitconfig();
    void getAuthInfo(int &auth_type, int &auto_connect, string &username, string &password);
    int setAuthInfo(struct AuthInfo &auth_info);
    int setAuthConnect(int auto_connect);
    int delete_auth_info();

    //add for vmmode
    bool checkvmmode_section(const string &section);
    bool checkvmmode_entry(const string &section, const string &entry);
    int  getvmmode_value(const string &section, const string &entry, string &value);
	
    bool is_display_info_ini_exist();
    int  delete_display_info_ini();
    int is_display_info_section_exist(const char *section);
    int delete_resolution_info_section();
    void set_ext_display_info(const struct DisplayInfo &dpi_info, int num);
    void get_ext_display_info(ExtDisplayInfo *res_info);
    void get_ext_display_info(int port, string &width, string &height, map<string, int> &res_list);
    void get_ext_display_info(int port, string &width, string &height);
    void set_display_info(const struct DisplayInfo &dpi_info);
	void get_display_info(int *width, int *height, int *custom);
    const void set_hdmi_audio_info(int hdmiaudio);
	int get_hdmi_audio_info();
    void init_hdmi_audio_info();

    void set_disk_info(const DiskInfo_t &diskinfo);
    int get_disk_info(const string &disk_name, DiskInfo_t &diskinfo);
    int delete_diskinfo_ini();

    void set_vm_desktop_redir(const string &redir_switch);
    void get_vm_desktop_redir(string &redir_switch);

    void set_e1000_netcard(const bool &netcard_switch);
    bool is_using_e1000_netcard(void);
    void set_usb_emulation(const bool &is_emulation);
    bool is_usb_emulation(void);
    //app_layer_manage
    void set_app_layer_switch(const int &status);
    int get_app_layer_switch();

    void set_http_port_map(const HttpPortInfo &info);
    void get_http_port_map(HttpPortInfo &info);
    void set_http_last_port_map(const HttpPortInfo &info);
    void get_http_last_port_map(HttpPortInfo &info);
    //DevInterfaceInfo
    void set_dev_interface_info(const DevInterfaceInfo &inter_info);
    int get_dev_interface_info(DevInterfaceInfo &inter_info);   

private:
    string _logined_user;
    bool _saved_logined;
    int _saved_state;
    bool _ui_permit;
    bool _on_localmode;

};

#endif /* _USER_MGR_H_ */

