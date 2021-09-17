/**
 * Copyright(C) 2017 Ruijie Network Inc. All rights reserved.
 *
 * user_mgr.cpp
 * Original Author: yejx@ruijie.com.cn, 2017-2-18
 *
 * IDV client local user info manager.
 * class interfaces definition & processes.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <rc/rc_log.h>
#include <iniparser/iniparser.h>
#include <iniparser/dictionary.h>
#include "user_db.h"
#include "user_mgr.h"
#include "application.h"
#include <vector>
#include <algorithm>
#include "base64.h"
#include "ui_api.h"

using namespace std;

const static string usrerr_log[USRERR_MAXNO] = {
    "Success.",
    "User password incorrect!",
    "No enough memory!",
    "Argument invalid!",
    "No entry found in the user database!",
    "No sections found in the user database!",
    "Error occurred while reading the user database!",
    "Error occurred while writing the user database!",
    "No user found in the user database!",
    "No user netdisk info found in the user database!",
    "No group policy found in the user database!",
    "No public policy found in the user database!",
    "No ad domain info found in the user database!",
};

const static string &unknown_error = "Unknown error code!";

const string &UserInfoMgr::usrmgr_strerror(int err)
{
    if (err > 0)
        return unknown_error;
    else
        return usrerr_log[-err];
}

int UserInfoMgr::UserLoginAuthentication(Mode mode, string &username, string &password,
        struct UserInfo *user_info)
{
    if (mode < SPECIAL_MODE || mode > PUBLIC_MODE)
        return -USRERR_INVAL;
    if (user_info == NULL)
        return -USRERR_INVAL;

    LOG_DEBUG("User %s request logining, Mode: %d\n", username.c_str(), mode);
#if 0
    //XXX: guest login forbidden on local mode?
    if (mode == PUBLIC_MODE || username == "guest") {
#else
    if (mode == PUBLIC_MODE) {
#endif
        // guest uses public policy
        return PublicPolicyRead(&user_info->policy_info);
    } else {
        int ret;

        //convert full_ad_name to short_name
        ret = convert_to_short_name(username);
        if (ret != 0) {
            LOG_WARNING("Failed to convert User %s to a short name!", username.c_str());
        }

        if (username == "" || password == "")
            return -USRERR_PSWERROR;
        ret = UserInfoRead(username, user_info);
        if (ret != 0)
            return ret;

        /* Authentication: check if user password is correct */
        if (gloox::password_codec_xor(password, false) == gloox::password_codec_xor(user_info->password, false)) {
            LOG_NOTICE("Authentication Pass, User %s allows to login.\n", username.c_str());
            return 0;
        } else {
            LOG_WARNING("Authentication Failed, User %s logined with an incorrect password!\n",
                username.c_str());
            return -USRERR_PSWERROR;
        }
    }
}

int UserInfoMgr::UserLoginedInfoStore(Mode mode, struct UserInfo &user_info)
{
    if (mode < SPECIAL_MODE || mode > PUBLIC_MODE)
        return -USRERR_INVAL;

    LOG_DEBUG("Logined user: %s, Mode: %d\n", user_info.username.c_str(), mode);
    if (mode == PUBLIC_MODE) {
        /* Public mode does not need to store user info */
        return PublicPolicyStore(user_info.policy_info);
    } else {
        return UserInfoWrite(user_info);
    }
}

int UserInfoMgr::PublicPolicyStore(struct PolicyInfo &public_policy)
{
    GroupInfoDB db_groupinfo;
    char file_name[FLIE_LEN] = {0};
    int ret;

    ret = db_groupinfo.storePublicPolicy(public_policy);
    if (ret != 0) {
        LOG_ERR("Failed to store public policy!\n");
    } else {
        LOG_NOTICE("Public policy stored successfully!\n");
        ret = db_groupinfo.saveUserDB();
    }

    //save public usb_policy
    strncpy(file_name, USB_PUBLIC_POLICY, FLIE_LEN);
    UsbPolicy usb_policy(file_name);
    usb_policy.wite_file(public_policy.usb_policy.c_str());
    return ret;
}

int UserInfoMgr::PublicPolicyRead(struct PolicyInfo *public_policy)
{
    GroupInfoDB db_groupinfo;
    char file_name[FLIE_LEN];
    char *buff = NULL;
    int file_length = 0;
    int ret;

    /* Public mode, just return the public policy */
    ret = db_groupinfo.readPublicPolicy(public_policy);
    if (ret != 0) {
        LOG_ERR("Failed to read public policy!\n");
        return ret;
    }

    strncpy(file_name, USB_PUBLIC_POLICY, FLIE_LEN);
    UsbPolicy usb_policy(file_name);
    file_length = usb_policy.get_file_length();
    if (file_length <= 0) {
        LOG_ERR("%s No such file or directory", file_name);
        return -1;
    }
    buff = (char *)malloc(file_length + 1);
    memset(buff, 0, file_length + 1);
    usb_policy.read_file(buff, file_length);
    public_policy->usb_policy = buff;
    free(buff);

    return 0;
}

int UserInfoMgr::UserInfoInitconfig()
{
    UserInfoDB db_userinfo;
    int ret = 0;

    db_userinfo.userInfoInit();
    if (ret != 0) {
        LOG_ERR("Failed to UserInfoInitconfig: %s", usrmgr_strerror(ret).c_str());
    }
    return ret;
}

int UserInfoMgr::UserInfoWrite(struct UserInfo &user_info)
{
    UserInfoDB db_userinfo;
    UserNetdiskDB db_usernetdisk;
    GroupInfoDB db_groupinfo;
    char file_name[FLIE_LEN] = {0};
    int ret;

    if (user_info.username == "" || user_info.password == "")
        return -USRERR_INVAL;

    ret = db_userinfo.storeUserInfo(user_info);
    if (ret != 0) {
        LOG_ERR("Failed to store user %s's info!\n", user_info.username.c_str());
        return ret;
    }
    ret = db_userinfo.saveUserDB();
    if (ret != 0)
        return ret;

    ret = db_usernetdisk.storeUserInfo(user_info);
    if (ret != 0) {
        LOG_ERR("Failed to store user %s's netdisk info!\n", user_info.username.c_str());
        return ret;
    }
    ret = db_usernetdisk.saveUserDB();
    if (ret != 0)
        return ret;

    ret = db_groupinfo.storeUserInfo(user_info);
    if (ret != 0) {
        LOG_ERR("Failed to store group %d's policy (for user %s)!\n",
            user_info.group_id, user_info.username.c_str());
        return ret;
    }
    ret = db_groupinfo.saveUserDB();
    if (ret != 0)
        return ret;
    LOG_NOTICE("User %s's information stored successfully!\n", user_info.username.c_str());

    //save group usb_policy
    snprintf(file_name, FLIE_LEN, "%s%s_policy", USB_POLICY_PATH, user_info.username.c_str());
    UsbPolicy usb_policy(file_name);
    usb_policy.wite_file(user_info.policy_info.usb_policy.c_str());
    return 0;
}

int UserInfoMgr::UserInfoRead(string &username, struct UserInfo *user_info)
{
    UserInfoDB db_userinfo;
    UserNetdiskDB db_usernetdisk;
    GroupInfoDB db_groupinfo;
    int ret;

    if (username == "" || user_info == NULL)
        return -USRERR_INVAL;

    user_info->username = username;
    ret = db_userinfo.readUserInfo(user_info);
    if (ret != 0) {
        LOG_ERR("Failed to read user %s's info!\n", username.c_str());
        return ret;
    }
    ret = db_usernetdisk.readUserInfo(user_info);
    if (ret != 0) {
        LOG_ERR("Failed to read user %s's netdisk info!\n", username.c_str());
        return ret;
    }

    ret = db_groupinfo.readUserInfo(user_info);
    if (ret != 0) {
        LOG_ERR("Failed to read user %s's group policy (group ID: %d)!\n",
            username.c_str(), user_info->group_id);
        return ret;
    }

    // use usb_policy in user_info.ini
    //db_userinfo.readUserInforUsbPolicy(username, &user_info->policy_info);
    char file_name[FLIE_LEN];
    char *buff = NULL;
    int file_length = 0;

    snprintf(file_name, FLIE_LEN, "%s%s_policy", USB_POLICY_PATH, user_info->username.c_str());
    UsbPolicy usb_policy(file_name);
    file_length = usb_policy.get_file_length();
    if (file_length <= 0) {
        LOG_ERR("%s No such file or directory", file_name);
        return 0;
    }
    buff = (char *)malloc(file_length + 1);
    memset(buff, 0, file_length + 1);
    usb_policy.read_file(buff, file_length);
    user_info->policy_info.usb_policy = buff;
    free(buff);

    return 0;
}

int UserInfoMgr::UserInfoDelete(string &username)
{
    UserInfoDB db_userinfo;
    UserNetdiskDB db_usernetdisk;
    int ret;

    if (username == "")
        return -USRERR_INVAL;

    // delete User info
    ret = db_userinfo.deleteUserInfo(username);
    if (ret != 0) {
        LOG_ERR("Failed to delete user %s's info!\n", username.c_str());
        return ret;
    }
    ret = db_userinfo.saveUserDB();
    if (ret != 0)
        return ret;

    // delete User netdisk info
    ret = db_usernetdisk.deleteUserInfo(username);
    if (ret != 0) {
        LOG_ERR("Failed to delete user %s's netdisk info!\n", username.c_str());
    } else {
        LOG_NOTICE("User %s's information deleted successfully!\n", username.c_str());
        ret = db_usernetdisk.saveUserDB();
    }
    return ret;
}

int UserInfoMgr::UserGroupPolicyDelete(int &group_id)
{
    UserInfoDB db_userinfo;
    UserNetdiskDB db_usernetdisk;
    GroupInfoDB db_groupinfo;
    int ret;
    string *group_users;
    int i, user_count;

    if (group_id < 0)
        return -USRERR_INVAL;
    // delete User group info
    ret = db_groupinfo.deleteUserGroup(group_id);
    if (ret != 0) {
        LOG_ERR("Failed to delete group %d's policy info!\n", group_id);
        return ret;
    }
    ret = db_groupinfo.saveUserDB();
    if (ret != 0)
        return ret;

    // delete users of the group
    group_users = db_userinfo.getUsers_withGroup(group_id, &user_count, &ret);
    if (group_users == NULL) {
        LOG_ERR("Failed to get Users of the group %d!\n", group_id);
        return ret;
    }
    for (i = 0; i < user_count; i++) {
        ret = db_userinfo.deleteUserInfo(group_users[i]);
        if (ret != 0) {
            LOG_WARNING("Failed to delete user %s's info!\n", group_users[i].c_str());
            break;
        }
        ret = db_usernetdisk.deleteUserInfo(group_users[i]);
        if (ret != 0) {
            LOG_WARNING("Failed to delete user %s's netdisk info!\n", group_users[i].c_str());
            break;
        }
    }
    if (ret == 0) {
        ret = db_userinfo.saveUserDB();
        ret |= db_usernetdisk.saveUserDB();
    }
    delete [] group_users;
    return ret;
}

int UserInfoMgr::getAllUsers(vector<string> &users)
{
    UserInfoDB db_userinfo;

    return db_userinfo.getAllUsers(users);
}

int UserInfoMgr::updateAllUsersInfo(const vector<struct UserInfo> &usersinfo)
{
    int ret;
    vector<string> db_users;
    vector<string> new_users;
    vector<int> old_groups;
    vector<int> new_groups;
    UserInfoDB db_userinfo;
    GroupInfoDB db_groupinfo;
    struct UserInfo user_info;
    unsigned int i;
    vector<int>::iterator group_it;
    vector<struct UserInfo>::const_iterator user_it;
    char cmd[128] = {0};

    if (usersinfo.empty()) {
        // web usersinfo empty, that means all the local users will be deleted.
        ret = db_groupinfo.getAllGroups(old_groups);
        if (ret == 0) {
            for (i = 0; i < old_groups.size(); i++) {
                LOG_DEBUG("Deleting group %d that was not found on web ...", old_groups[i]);
                UserGroupPolicyDelete(old_groups[i]);
            }
        }
    } else {
        // update users' info into local db.
        for (user_it = usersinfo.begin(); user_it != usersinfo.end(); user_it++) {
            LOG_DEBUG("Update user %s's info (in group %d) ...",
                user_it->username.c_str(), user_it->group_id);
            user_info = *user_it;
            ret = UserInfoWrite(user_info);
            if (ret != 0)
                return ret;
            new_users.push_back(user_it->username);
            new_groups.push_back(user_it->group_id);
        }

        // delete users not found.
        db_userinfo.getAllUsers(db_users);
        for (i = 0; i < db_users.size(); i++) {
            LOG_DEBUG("Search if user %s exists ...", db_users[i].c_str());
            if (find(new_users.begin(), new_users.end(), db_users[i]) == new_users.end()) {
                // user not found, delete it.
                LOG_INFO("Delete user %s while it was not found after updated.\n",
                    db_users[i].c_str());
                UserInfoDelete(db_users[i]);
                //delete group usb_policy
                sprintf(cmd, "rm -f %s%s_policy", USB_POLICY_PATH, db_users[i].c_str());
                rc_system(cmd);
            }
        }

        // delete user groups not found.
        ret = db_groupinfo.getAllGroups(old_groups);
        if (ret == 0) {
            for (i = 0; i < old_groups.size(); i++) {
                LOG_DEBUG("Search if group %d exists ...", old_groups[i]);
                group_it = find(new_groups.begin(), new_groups.end(), old_groups[i]);
                if (group_it == new_groups.end()) {
                    // group not fond, delete it.
                    LOG_INFO("Delete group %d while it was not found after updated.\n",
                        old_groups[i]);
                    UserGroupPolicyDelete(old_groups[i]);
                }
            }
        }
    }
    return 0;
}


/*
 * for logined user saved & state machine reset.
 */

void UserInfoMgr::loadSavedStatus()
{
    UserStatusDB db_runningsaved;

    _logined_user = db_runningsaved.getLoginedUser();
    _saved_logined = db_runningsaved.getUserLoginedStatus();
    _saved_state = db_runningsaved.getSavedState();
    _ui_permit = db_runningsaved.getUIPermitted();
    _on_localmode = db_runningsaved.get_localmode_flag();
}

void UserInfoMgr::permitUI(bool permitted)
{
    UserStatusDB db_runningsaved;

    _ui_permit = permitted;
    db_runningsaved.setUIPermitted(permitted);
    db_runningsaved.saveUserDB();
}

void UserInfoMgr::saveCurrentState(int state)
{
    UserStatusDB db_runningsaved;

    _saved_state = state;
    db_runningsaved.setSavedState(state);
    db_runningsaved.saveUserDB();
}


int UserInfoMgr::delete_user_status_ini()
{
    UserStatusDB db_runningsaved;
    return db_runningsaved.delete_user_status_ini();
}

void UserInfoMgr::setUserLogined(bool logined)
{
    UserStatusDB db_runningsaved;

    _saved_logined = logined;
    db_runningsaved.setUserLoginedStatus(logined);
    db_runningsaved.saveUserDB();
}

void UserInfoMgr::saveLoginedUser(string &username)
{
    UserStatusDB db_runningsaved;

    _logined_user = username;
    db_runningsaved.setLoginedUser(username);
    if (_saved_logined) {
        db_runningsaved.saveUserDB();
    }
}

void UserInfoMgr::saveLastLoginedUser(string &username, string &password, int &remember_flag)
{
    UserLastLoginedDB db_lastsaved;

    if (username != "public" && username != "guest") {
        db_lastsaved.setLoginedUser(username, password, remember_flag);
        db_lastsaved.saveUserDB();
    }
}

void UserInfoMgr::getLastLoginedUser(string *username, string *password, int *remember_flag)
{
    UserLastLoginedDB db_lastsaved;

    db_lastsaved.getLoginedUser(username, password, remember_flag);
    if(!password->empty())
    {
        *password = gloox::password_codec_xor(*password, false);

        //origin password is not empty
        //but it became emptyafter decode, so we must come across some wrong
        if(password->empty())
        {
            //clear database
            string tmp_username = "";
            string tmp_password = "";
            int tmp_remember_flag = 0;
            db_lastsaved.setLoginedUser(tmp_username, tmp_password, tmp_remember_flag);
            db_lastsaved.saveUserDB();
            LOG_NOTICE("getLastLoginedUser decoding come across some wrong");

            //re-get
            db_lastsaved.getLoginedUser(username, password, remember_flag);
        }
    }
}

int UserInfoMgr::readssidWhiteList(vector<string> &whitelist)
{
    SsidWhiteListDB db_whitelist;

    return db_whitelist.readssidWhiteUser(whitelist);
}

void UserInfoMgr::storessidWhiteList(vector<string> &whitelist)
{
    SsidWhiteListDB db_whitelist;

    db_whitelist.storessidWhiteUser(whitelist);
    db_whitelist.saveUserDB();
}

int UserInfoMgr::deletessidWhite()
{
    int ret = 0;
    SsidWhiteListDB db_whitelist;

    ret = db_whitelist.deletessidWhite();
    if (ret == 0) {
        ret = db_whitelist.saveUserDB();
    } else {
        LOG_WARNING("Fail to delete ssid_white_list.ini\n");
    }
    return ret;
}

bool UserInfoMgr::is_display_info_ini_exist()
{
    return get_file_exist(USER_DISPLAY_INFO_INI);
}

int UserInfoMgr:: delete_display_info_ini()
{
    int ret = 0;
    DisplayinfoDB db_displayinfo;
    ret = db_displayinfo.delete_display_info_ini();
    return ret;
}

int UserInfoMgr:: is_display_info_section_exist(const char *section)
{
    int ret = 0;
    DisplayinfoDB db_displayinfo;
    ret = db_displayinfo.is_display_info_section_exist(section);

    return ret;
}

int UserInfoMgr:: delete_resolution_info_section()
{
    int ret = 0;
    DisplayinfoDB db_displayinfo;
    ret = db_displayinfo.delete_resolution_info_section();
    LOG_WARNING("delete_resolution_info_section ret = %d", ret);
    db_displayinfo.saveUserDB();
    return ret;
}

void UserInfoMgr::set_ext_display_info(const struct DisplayInfo &dpi_info,int num)
{
    DisplayinfoDB db_displayinfo;
    db_displayinfo.set_ext_display_info(dpi_info, num);
    db_displayinfo.saveUserDB();
}

void UserInfoMgr::get_ext_display_info(ExtDisplayInfo *res_info)
{
    DisplayinfoDB db_displayinfo;

    db_displayinfo.get_ext_display_info(res_info);
}

void UserInfoMgr::get_ext_display_info(int port, string &width, string &height)
{
    DisplayinfoDB db_displayinfo;
    int custom;

    db_displayinfo.get_ext_display_info(port, width, height, custom);
}

void UserInfoMgr::get_ext_display_info(int port, string &width, string &height, map<string, int> &res_list)
{
    DisplayinfoDB db_displayinfo;
    int custom;
    char resolution[32] = {0};
    map<string, int>::iterator tempiter;

    db_displayinfo.get_ext_display_info(port, width, height, custom);
    if (custom == FALSE) {
        memset(resolution, 0, sizeof(resolution));
        sprintf(resolution, "%sx%s", width.c_str(), height.c_str());
        tempiter = res_list.find(resolution);
        if (tempiter != res_list.end()) {
            if (tempiter->second & IS_RES_BEST) {
                width = "0";
                height = "0";
            }
        } else {
            width = "0";
            height = "0";
        }
    }
}

void UserInfoMgr::set_display_info(const struct DisplayInfo &dpi_info)
{
    DisplayinfoDB db_displayinfo;
    db_displayinfo.set_display_info(dpi_info);
    db_displayinfo.saveUserDB();
}

void UserInfoMgr::get_display_info(int *width, int *height, int *custom)
{
    DisplayinfoDB db_displayinfo;

    db_displayinfo.get_display_info(width, height, custom);
}

void UserInfoMgr::enter_local_mode()
{
    UserStatusDB db_runningsaved;

    _on_localmode = true;
    db_runningsaved.set_localmode_flag(_on_localmode);
    db_runningsaved.saveUserDB();
}

void UserInfoMgr::leave_local_mode()
{
    UserStatusDB db_runningsaved;

    _on_localmode = false;
    db_runningsaved.set_localmode_flag(_on_localmode);
    db_runningsaved.saveUserDB();
}

int UserInfoMgr::convert_to_short_name(string &full_name)
{
    UserInfoDB db_userinfo;

    return db_userinfo.convertToShortName(full_name);
}

bool UserInfoMgr::checkvmmode_section(const string &section)
{
    VmmodeInfoDB db_vmmode;
    return  db_vmmode.checkvmmode_section(section);
}

bool UserInfoMgr::checkvmmode_entry(const string &section, const string &entry)
{
    VmmodeInfoDB db_vmmode;
    return db_vmmode.checkvmmode_entry(section, entry);
}

int  UserInfoMgr::getvmmode_value(const string &section, const string &entry, string &value)
{
    VmmodeInfoDB db_vmmode;
    return db_vmmode.getvmmode_value(section, entry, value);
}

void UserInfoMgr::init_hdmi_audio_info()
{
    HdmiAudioinfoDB db_hdmiaudio;
    int ret = db_hdmiaudio.get_hdmi_audio_info();
    Application::get_application()->get_device_interface()->setHdmiVoiceEnable(ret);
    db_hdmiaudio.delete_hdmi_audio_info();
    return;
}

const void UserInfoMgr::set_hdmi_audio_info(int hdmiaudio)
{
    HdmiAudioinfoDB db_hdmiaudio;
    db_hdmiaudio.set_hdmi_audio_info(hdmiaudio);
}

int UserInfoMgr::get_hdmi_audio_info()
{
    int ret = 0;
    HdmiAudioinfoDB db_hdmiaudio;
    ret = db_hdmiaudio.get_hdmi_audio_info();
    return ret;
}

void UserInfoMgr::set_disk_info(const DiskInfo_t &diskinfo)
{
    DiskInfoDB db_diskinfo;
    db_diskinfo.set_disk_info(diskinfo);
    db_diskinfo.saveUserDB();
}

/**
* function: get expand disk info
*
* return: 0: ok  -ne 0: error
*/
int UserInfoMgr::get_disk_info(const string &disk_name, DiskInfo_t &diskinfo)
{
    DiskInfoDB db_diskinfo;
    return db_diskinfo.get_disk_info(disk_name, diskinfo);
}

int UserInfoMgr::delete_diskinfo_ini()
{
    DiskInfoDB db_diskinfo;
    return db_diskinfo.delete_diskinfo_ini();
}

int UserInfoMgr::AuthIniInitconfig()
{
    AuthDB db_auth_info;
    int ret = 0;

    ret = db_auth_info.authIniInit();
    if (ret != 0) {
        LOG_ERR("Failed to AuthIniInitconfig: %s", usrmgr_strerror(ret).c_str());
    }
    return ret;
}

void UserInfoMgr::getAuthInfo(int &auth_type, int &auto_connect, string &username, string &password)
{
    AuthDB db_auth_info;
    int ret;

    ret = db_auth_info.getAuthInfo(auth_type, auto_connect, username, password);
    if (ret != 0)
        LOG_ERR("Failed to read Network Authentication info: %s", usrmgr_strerror(ret).c_str());
}

int UserInfoMgr::setAuthInfo(struct AuthInfo &auth_info)
{
    AuthDB db_auth_info;
    int ret;

    ret = db_auth_info.setAuthInfo(auth_info);
    if (ret != 0) {
        LOG_ERR("Failed to set Network Authentication info: %s", usrmgr_strerror(ret).c_str());
        return ret;
    }

    ret = db_auth_info.saveUserDB();
    if (ret != 0) {
        LOG_ERR("Failed to set Network Authentication info: %s", usrmgr_strerror(ret).c_str());
        return ret;
    }
    return 0;
}

int UserInfoMgr::setAuthConnect(int auto_connect)
{
    AuthDB db_auth_info;
    int ret;

    ret = db_auth_info.setAuthConnect(auto_connect);
    if (ret != 0) {
        LOG_ERR("Failed to set Network Authentication info: %s", usrmgr_strerror(ret).c_str());
        return ret;
    }
    ret = db_auth_info.saveUserDB();
    if (ret != 0) {
        LOG_ERR("Failed to set Network Authentication info: %s", usrmgr_strerror(ret).c_str());
        return ret;
    }
    return 0;
}

int UserInfoMgr::delete_auth_info()
{
    AuthDB db_auth_info;

    db_auth_info.delete_auth_info();
    db_auth_info.saveUserDB();

    return 0;
}

void UserInfoMgr::set_vm_desktop_redir(const string &redir_switch)
{
    VMConfigDB db_vm_config;

    db_vm_config.set_vm_desktop_redir(redir_switch);
    db_vm_config.saveUserDB();
}

void UserInfoMgr::get_vm_desktop_redir(string &redir_switch)
{
    VMConfigDB db_vm_config;
    db_vm_config.get_vm_desktop_redir(redir_switch);
}

void UserInfoMgr::set_e1000_netcard(const bool &netcard_switch)
{
    OtherConfigDB db_other_config;
    db_other_config.set_e1000_netcard(netcard_switch);
    db_other_config.saveUserDB();
}

bool UserInfoMgr::is_using_e1000_netcard(void)
{
    OtherConfigDB db_other_config;
    return db_other_config.is_using_e1000_netcard();
}

void UserInfoMgr::set_usb_emulation(const bool &is_emulation)
{
    OtherConfigDB db_other_config;
    db_other_config.set_usb_emulation(is_emulation);
    db_other_config.saveUserDB();
}

bool UserInfoMgr::is_usb_emulation(void)
{
    OtherConfigDB db_other_config;
    return db_other_config.is_usb_emulation();
}

void UserInfoMgr::set_app_layer_switch(const int &status)
{
    OtherConfigDB db_other_config;
    db_other_config.set_app_layer_switch(status);
    db_other_config.saveUserDB();
}

int UserInfoMgr::get_app_layer_switch()
{
    OtherConfigDB db_other_config;
    return db_other_config.get_app_layer_switch();
}

/* idv over wan */
void UserInfoMgr::set_http_port_map(const HttpPortInfo &info)
{
    HttpPortMapDB db_httpportmap;

    db_httpportmap.set_http_port_map(info);
    db_httpportmap.saveUserDB();
}

void UserInfoMgr::get_http_port_map(HttpPortInfo &info)
{
    HttpPortMapDB db_httpportmap;
    db_httpportmap.get_http_port_map(info);
}

void UserInfoMgr::set_http_last_port_map(const HttpPortInfo &info)
{
    HttpPortMapDB db_httpportmap;

    db_httpportmap.set_http_last_port_map(info);
    db_httpportmap.saveUserDB();
}

void UserInfoMgr::get_http_last_port_map(HttpPortInfo &info)
{
    HttpPortMapDB db_httpportmap;
    db_httpportmap.get_http_last_port_map(info);
}

void UserInfoMgr::set_dev_interface_info(const DevInterfaceInfo &inter_info)
{
    DevInterfaceDB db_inter_info;
    db_inter_info.storeDevInterfaceInfo(inter_info);
    db_inter_info.saveUserDB();
}

int UserInfoMgr::get_dev_interface_info(DevInterfaceInfo &inter_info)
{
    DevInterfaceDB db_inter_info;
    return db_inter_info.readDevInterfaceInfo(inter_info);
}

