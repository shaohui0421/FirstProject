/**
 * Copyright(C) 2017 Ruijie Network Inc. All rights reserved.
 *
 * user_db.cpp
 * Original Author: yejx@ruijie.com.cn, 2017-2-19
 *
 * User Info Database operations.
 */

#include <iniparser/dictionary.h>
#include <iniparser/iniparser.h>
#include <rc/rc_log.h>
#include <stdio.h>
#include <errno.h>
#include "user_mgr.h"
#include "user_db.h"
#include "application.h"
#include "common.h"

#include <iostream>
#include <vector>
using namespace std;

#define DEBUG_DETAIL    0
#define ENTRY_LEN       128

UserDB::UserDB(const char *inifile)
    : _inifile(inifile)
    , _ini(NULL)
    , _occured_error(0)
    , _changed(false)
{
#if DEBUG_DETAIL
    LOG_DEBUG("Init ini file %s, to create User Database.", inifile);
#endif
}

int UserDB::saveUserDB(void)
{
    int ret;

    if (_ini == NULL)
        return _occured_error;
    if (_changed) {
        /* we need write the ini file only if database was chaned */
        LOG_INFO("Writing ini file %s ...", _inifile);
        ret = iniparser_dump_ini(_ini, _inifile);
        if (ret == -1) {
            LOG_PERROR("Failed to write ini file %s\n", _inifile);
            _occured_error = -USRERR_DBWFAIL;
            return _occured_error;
        }
#if DEBUG_DETAIL
        LOG_INFO("Database %s saved.", _inifile);
#endif
    }
    return 0;
}

void UserDB::load(void)
{
    _ini = iniparser_load(_inifile);
    if (_ini == NULL) {
        if (errno != ENOENT) {
            LOG_PERROR("Failed to read ini file %s\n", _inifile);
            _occured_error = -USRERR_DBRFAIL;
            return;
        }

        /* ini file does not exist, create it. */
        LOG_NOTICE("Creating new ini file %s ...", _inifile);
        _ini = dictionary_new(0);
        if (_ini == NULL) {
            LOG_ERR("No enough memory to create new ini file %s\n", _inifile);
            _occured_error = -USRERR_NOMEM;
        }
    }
#if DEBUG_DETAIL
    LOG_DEBUG("Database %s loaded.", _inifile);
#endif
}

void UserDB::unload(void)
{
    if (_ini == NULL)
        return;
    iniparser_freedict(_ini);
#if DEBUG_DETAIL
    LOG_DEBUG("Database %s unloaded.\n", _inifile);
#endif
}

const char *UserDB::_get_entry(const char *section, const char *key, const char *def)
{
    char entry[ENTRY_LEN];

    snprintf(entry, ENTRY_LEN, "%s:%s", section, key);
    return iniparser_getstring(_ini, entry, def);
}

int UserDB::_get_entry(const char *section, const char *key, int def)
{
    char entry[ENTRY_LEN];

    snprintf(entry, ENTRY_LEN, "%s:%s", section, key);
    return iniparser_getint(_ini, entry, def);
}

bool UserDB::_check_if_entry_changed(const char *section, const char *key, const char *value)
{
    const char *old_value;

    old_value = _get_entry(section, key, "");
    if (strcmp(old_value, value))
        return true;
    else
        return false;
}

bool UserDB::_check_if_entry_changed(const char *section, const char *key, int value)
{
    int old_value;

    old_value = _get_entry(section, key, -1);
    if (old_value != value)
        return true;
    else
        return false;
}

const char *UserDB::getEntry(const char *section, const char *key, const char *def)
{
    char entry[ENTRY_LEN];
    const char *value;

    if (_ini == NULL)
        return NULL;
    snprintf(entry, ENTRY_LEN, "%s:%s", section, key);
    if (!iniparser_find_entry(_ini, entry)) {
        LOG_ERR("Entry %s does not exist.\n", entry);
        _occured_error = -USRERR_NOENTRY;
        return def;
    }
#if DEBUG_DETAIL
    LOG_DEBUG("Try to read entry %s [default:%s]...", entry, def);
#endif
    value = iniparser_getstring(_ini, entry, def);
#if DEBUG_DETAIL
    LOG_DEBUG("  Got value: %s\n", value);
#endif
    return value;
}

int UserDB::getEntry(const char *section, const char *key, int def)
{
    char entry[ENTRY_LEN];
    int value;

    if (_ini == NULL)
        return def;
    snprintf(entry, ENTRY_LEN, "%s:%s", section, key);
    if (!iniparser_find_entry(_ini, entry)) {
        LOG_ERR("Entry %s does not exist.\n", entry);
        _occured_error = -USRERR_NOENTRY;
        return def;
    }
#if DEBUG_DETAIL
    LOG_DEBUG("Read entry %s [default:%d] ...", entry, def);
#endif
    value = iniparser_getint(_ini, entry, def);
#if DEBUG_DETAIL
    LOG_DEBUG("  Got value: %d\n", value);
#endif
    return value;
}

int UserDB::_add_new_section(const char *section)
{
    int ret;

    LOG_DEBUG("Adding a new section %s ...\n", section);
    ret = iniparser_set(_ini, section, NULL);
    if (ret != 0) {
        LOG_ERR("Failed to add a new section %s: Cannot allocate memory\n", section);
        _occured_error = -USRERR_NOMEM;
    }
    return ret;
}

void UserDB::setEntry(const char *section, const char *key, const char *value)
{
    char entry[ENTRY_LEN];
    int ret;

    if (_ini == NULL)
        return;
    snprintf(entry, ENTRY_LEN, "%s:%s", section, key);
    if (!iniparser_find_entry(_ini, section)) {
        // add a new section
        ret = _add_new_section(section);
        if (ret != 0)
            return;
    } else {
        // if entry not found, just add it.
        if (iniparser_find_entry(_ini, entry)
            && !_check_if_entry_changed(section, key, value)) {
#if DEBUG_DETAIL
            LOG_DEBUG("Entry %s:%s is not changed (value: %s).\n", section, key, value);
#endif
            return;
        }
    }

    ret = iniparser_set(_ini, entry, value);
    if (ret == 0) {
#if DEBUG_DETAIL
        LOG_DEBUG("Stored: %s = %s\n", entry, value);
#endif
        _changed = true;
    } else {
        LOG_ERR("Failed to store (string)value %s of key %s: Cannot allocate memory\n", value, key);
        _occured_error = -USRERR_NOMEM;
    }
}

void UserDB::setEntry(const char *section, const char *key, int value)
{
    char entry[ENTRY_LEN];
    char valstr[ENTRY_LEN];
    int ret;

    if (_ini == NULL)
        return;
    snprintf(entry, ENTRY_LEN, "%s:%s", section, key);
    if (!iniparser_find_entry(_ini, section)) {
        // add a new section
        ret = _add_new_section(section);
        if (ret != 0)
            return;
    } else {
        // if entry not found, just add it.
        if (iniparser_find_entry(_ini, entry)
            && !_check_if_entry_changed(section, key, value)) {
#if DEBUG_DETAIL
            LOG_DEBUG("Entry %s:%s is not changed (value: %d).\n", section, key, value);
#endif
            return;
        }
    }

    snprintf(valstr, ENTRY_LEN, "%d", value);
    ret = iniparser_set(_ini, entry, valstr);
    if (ret == 0) {
#if DEBUG_DETAIL
        LOG_DEBUG("Stored: %s = %d\n", entry, value);
#endif
        _changed = true;
    } else {
        LOG_ERR("Failed to store (int)value %d of key %s: Cannot allocate memory\n", value, key);
        _occured_error = -USRERR_NOMEM;
    }
}

void UserDB::deleteEntry(const char *section, const char *key)
{
    char entry[ENTRY_LEN];

    if (_ini == NULL)
        return;
    snprintf(entry, ENTRY_LEN, "%s:%s", section, key);
    if (iniparser_find_entry(_ini, entry)) {
        iniparser_unset(_ini, entry);
        _changed = true;
    }
}

bool UserDB::findEntry(const char *section, const char *key)
{
    char entry[ENTRY_LEN];

    if (_ini == NULL) {
        return false;
    }
    snprintf(entry, ENTRY_LEN, "%s:%s", section, key);
    if (iniparser_find_entry(_ini, entry)) {
        return true;
    }
    return false;
}

void UserDB::deleteSection(const char *section)
{
    int i, nkeys;
    const char ** keys;

    if (_ini == NULL)
        return;
    if (iniparser_find_entry(_ini, section)) {
        nkeys = iniparser_getsecnkeys(_ini, section);
        if (nkeys > 0) {
            keys = (const char **)malloc(nkeys * sizeof(char *));
            if (keys == NULL)
            {
                LOG_ERR("No enough memory to delete Sections.\n");
                _occured_error = -USRERR_NOMEM;
                return;
            }
            keys = iniparser_getseckeys(_ini, section, keys);
            for (i = 0; i < nkeys; i++) {
                iniparser_unset(_ini, keys[i]);
            }
            free(keys);
        }
        iniparser_unset(_ini, section);
        _changed = true;
    }
}

bool UserDB::findSection(const char *section)
{
    if (_ini == NULL) {
        return false;
    }
    if (iniparser_find_entry(_ini, section)) {
        return true;
    }
    return false;
}

/**
 * return all the sections with entry "key=value"
 * @sec_count: return entry number.
 */
const char **UserDB::getSections_withEntry(const char *key, const char *value, int *sec_count)
{
    const char *secname;
    int i, nsec;
    const char *tmp_val;
    int count = 0;
    const char **sections;

    if (_ini == NULL)
        return NULL;
    nsec = iniparser_getnsec(_ini);
    if (nsec < 1) {
        // No sections, ini file format error */
        LOG_WARNING("No sections, maybe the ini file has error format.\n");
        _occured_error = -USRERR_NOSECTIONS;
        return NULL;
    }

    sections = (const char **)malloc(nsec * sizeof(char *));
    if (sections == NULL) {
        LOG_ERR("No enough memory to get Sections with '%s = %s'.\n", key, value);
        _occured_error = -USRERR_NOMEM;
        return NULL;
    }
    memset(sections, 0, nsec * sizeof(char *));
    for (i = 0; i < nsec; i++) {
        secname = iniparser_getsecname(_ini, i);
        if (secname == NULL)
            continue;
        tmp_val = _get_entry(secname, key, "");
        if (!strcmp(tmp_val, value))
            sections[count++] = secname;
    }
    *sec_count = count;
    return sections;
}

const char **UserDB::getSections_withEntry(const char *key, int value, int *sec_count)
{
    const char *secname;
    int i, nsec;
    int tmp_val;
    int count = 0;
    const char **sections;

    if (_ini == NULL)
        return NULL;
    nsec = iniparser_getnsec(_ini);
    if (nsec < 1) {
        // No sections, ini file format error */
        LOG_WARNING("No sections, maybe the ini file has error format.\n");
        _occured_error = -USRERR_NOSECTIONS;
        return NULL;
    }

    sections = (const char **)malloc(nsec * sizeof(char *));
    if (sections == NULL) {
        LOG_ERR("No enough memory to Sections with '%s = %d'.\n", key, value);
        _occured_error = -USRERR_NOMEM;
        return NULL;
    }
    memset(sections, 0, nsec * sizeof(char *));
    for (i = 0; i < nsec; i++) {
        secname = iniparser_getsecname(_ini, i);
        if (secname == NULL)
            continue;
        tmp_val = _get_entry(secname, key, -1);
        if (tmp_val == value)
            sections[count++] = secname;
    }
    *sec_count = count;
    return sections;
}

const char **UserDB::getAllSections(int *sec_count)
{
    const char *secname;
    int i, nsec;
    int count = 0;
    const char **sections;

    if (_ini == NULL)
        return NULL;
    nsec = iniparser_getnsec(_ini);
    if (nsec < 1) {
        // No sections, ini file format error */
        LOG_ERR("No sections, maybe the ini file has error format.\n");
        _occured_error = -USRERR_NOSECTIONS;
        return NULL;
    }

    sections = (const char **)malloc(nsec * sizeof(char *));
    if (sections == NULL) {
        LOG_ERR("No enough memory to get All the Sections.\n");
        _occured_error = -USRERR_NOMEM;
        return NULL;
    }
    memset(sections, 0, nsec * sizeof(char *));
    for (i = 0; i < nsec; i++) {
        secname = iniparser_getsecname(_ini, i);
        if (secname == NULL)
            continue;
        sections[count++] = secname;
    }
    *sec_count = count;
    return sections;
}

int UserInfoDB::userInfoInit()
{
    int nsec = 0, i = 0, ret = 0;
    const char **sections;
    string password, password_xor;

    sections = getAllSections(&nsec);
    if (sections == NULL) {
        LOG_ERR("userinfo init is null");
        return UserDBcheckError();
    }

    //IDV:4.1R1T1 Network security, then "password" => "password_xor"
    for (i = 0; i < nsec; i++) {
        LOG_DEBUG("sections[%d] %s", i, sections[i]);
        if (findEntry(sections[i], "password")) {
            if (findEntry(sections[i], "password_xor") == false) {
                password = getEntry(sections[i], "password", "");
                if ((ret = UserDBcheckError()) != 0) {
                    LOG_ERR("userinfo init error");
                    free(sections);
                    return ret;
                }
                // decode: openssl + base64
                password_xor = gloox::password_codec(password, false);
                //LOG_DEBUG("password_xor %s", password_xor.c_str());
                // encode: xor + base64
                setEntry(sections[i], "password_xor", gloox::password_codec_xor(password_xor, true).c_str());
            }

            deleteEntry(sections[i], "password");
            saveUserDB();
        }
    }

    free(sections);
    return UserDBcheckError();
}

int UserInfoDB::readUserInfo(struct UserInfo *user_info)
{
    char section_name[ENTRY_LEN];
    string username, password;
    int groupid;
    string user_auth_type;
    string ad_domain;
    string usb_policy;
    int error;

    username = user_info->username;
    snprintf(section_name, ENTRY_LEN, "%s_info", username.c_str());
    password = getEntry(section_name, "password_xor", "");
    if (password == "") {
        LOG_ERR("Failed to read User %s's password!\n", username.c_str());
        goto _error;
    }
    groupid = getEntry(section_name, "group", -1);
    if (groupid == -1) {
        LOG_ERR("Failed to read User %s's group ID!\n", username.c_str());
        goto _error;
    }
    //IDV3.0 history reason, "ad_user" => "auth_type"
    if (findEntry(section_name, "ad_user")) {
        if (findEntry(section_name, "auth_type") == false) {
            string is_ad_user = getEntry(section_name, "ad_user", "false");
            setEntry(section_name, "auth_type", (is_ad_user == "false" ? "LOCAL" : "AD"));
        }
        deleteEntry(section_name, "ad_user");
        saveUserDB();
    }
    user_auth_type = getEntry(section_name, "auth_type", "LOCAL");
    ad_domain = getEntry(section_name, "ad_domain", "");
    
    user_info->password = password;
    user_info->group_id = groupid;
    user_info->user_auth_info.user_auth_type = user_auth_type;
    user_info->user_auth_info.ad_domain = ad_domain;

    return 0;

_error:
    error = UserDBcheckError();
    if (error == -USRERR_NOENTRY)
        error = -USRERR_NOUSER;
    return error;
}

int UserInfoDB::readUserInforUsbPolicy(string &username, struct PolicyInfo *policyinfo)
{
    string usb_policy;
    char section_name[ENTRY_LEN];

    snprintf(section_name, ENTRY_LEN, "%s_info", username.c_str());
    usb_policy = getEntry(section_name, "usb", "{}");
    if(usb_policy != "{}") {
        LOG_DEBUG("readUserInforUsbPolicy is %s\n",policyinfo->usb_policy.c_str());
        policyinfo->usb_policy = usb_policy;
    }
    return 0;
}

int UserInfoDB::storeUserInfo(struct UserInfo &user_info)
{
    char section_name[ENTRY_LEN];
    string username, password;
    int groupid;
    string user_auth_type;
    string ad_domain;
    string usb_policy;

    username = user_info.username;
    password = user_info.password;
    groupid = user_info.group_id;
    user_auth_type = user_info.user_auth_info.user_auth_type;
    ad_domain = user_info.user_auth_info.ad_domain;
    usb_policy = user_info.policy_info.usb_policy;
    snprintf(section_name, ENTRY_LEN, "%s_info", username.c_str());
    setEntry(section_name, "username", username.c_str());
    setEntry(section_name, "password_xor", password.c_str());
    setEntry(section_name, "group", groupid);
    setEntry(section_name, "auth_type", user_auth_type.c_str());
    setEntry(section_name, "ad_domain", ad_domain.c_str());
    //setEntry(section_name, "usb", usb_policy.c_str());
    return UserDBcheckError();
}

int UserInfoDB::deleteUserInfo(string &username)
{
    char section_name[ENTRY_LEN];

    snprintf(section_name, ENTRY_LEN, "%s_info", username.c_str());
    deleteSection(section_name);
    return UserDBcheckError();
}

string *UserInfoDB::getUsers_withGroup(int &group_id, int *user_count, int *error)
{
    const char **sections;
    int i, sec_count;
    const char *username;
    string *users;
    int count = 0;
    int ret;

    sections = getSections_withEntry("group", group_id, &sec_count);
    if (sections == NULL) {
        *error = UserDBcheckError();
        return NULL;
    }

    users = new string[sec_count];
    for (i = 0; i < sec_count; i++) {
        username = getEntry(sections[i], "username", "");
        if ((ret = UserDBcheckError()) != 0) {
            free(sections);
            delete [] users;
            *error = ret;
            return NULL;
        }
        users[count++] = username;
    }
    *user_count = count;
    *error = 0;
    free(sections);
    return users;
}

int UserInfoDB::getAllUsers(vector<string> &users)
{
    const char **sections;
    int i, sec_count;
    string username;
    int ret;

    sections = getAllSections(&sec_count);
    if (sections == NULL) {
        return UserDBcheckError();
    }

    for (i = 0; i < sec_count; i++) {
        username = getEntry(sections[i], "username", "");
        if ((ret = UserDBcheckError()) != 0) {
            free(sections);
            users.clear();
            return ret;
        }
        users.push_back(username);
    }
    free(sections);
    return 0;
}

/**
 * Description: convert full ad username to a short name
 * Input:	full ad username contains username & domain
 * Return: username removed domain.
 */
int UserInfoDB::convertToShortName(string &full_name)
{
    const char **sections;
    int i, sec_count;
    string username;
    string user_auth_type;
    string ad_domain;

    sections = getAllSections(&sec_count);
    if (sections == NULL) {
        return UserDBcheckError();
    }

    for (i = 0; i < sec_count; i++) {
        username = getEntry(sections[i], "username", "");
        //IDV3.0 history reason, "ad_user" => "auth_type"
        if (findEntry(sections[i], "ad_user")) {
            if (findEntry(sections[i], "auth_type") == false) {
                string is_ad_user = getEntry(sections[i], "ad_user", "false");
                setEntry(sections[i], "auth_type", (is_ad_user == "false" ? "LOCAL" : "AD"));
            }
            deleteEntry(sections[i], "ad_user");
            saveUserDB();
        }
        user_auth_type = getEntry(sections[i], "auth_type", "LOCAL");
        ad_domain = getEntry(sections[i], "ad_domain", "");

        if (user_auth_type == "AD") {
            if(_adname_match_rule(full_name, username, ad_domain)) {
                full_name = username;
                free(sections);
                return 0;
            }
        } else {
            //only match username
            if(full_name == username) {
                free(sections);
                return 0;
            }
        }
    }
    free(sections);    
    return -USRERR_NOUSER;
}

bool UserInfoDB::_adname_match_rule(const string &full_name, const string &user_name, const string &domain)
{
    string tmp_name;
    if(full_name.empty())
    {
        LOG_DEBUG("ad match: full_name is empty");
        return false;
    }
    if(user_name.empty())
    {
        LOG_DEBUG("ad match: user_name is empty");
        return false;
    }

    //match rule
    //(full_name) == (user_name)
    if(full_name == user_name) {
        return true;
    }

    //(full_name) == (user_name@domain)
    tmp_name = user_name + '@' + domain;
    if(full_name == tmp_name) {
        return true;
    }

    //(full_name) == (domain_name\user_name)
    int nPos = domain.find_first_of(".");
    string domain_name = domain.substr( 0 , nPos );
    tmp_name = domain_name + '\\' + user_name;
    if(full_name == tmp_name) {
        return true;
    }

    return false;
}


int UserNetdiskDB::readUserInfo(struct UserInfo *user_info)
{
    char section_name[ENTRY_LEN];
    string username;
    string netusername, netpassword;
    string netdisk_enable, netdisk_ip, netdisk_path;
    int ret;

    username = user_info->username;
    snprintf(section_name, ENTRY_LEN, "%s_netdisk", username.c_str());

    /* if read NetDiskInfo failed, just disable the NetDisk */
    netdisk_enable = getEntry(section_name, "enable", "false");
    netusername = getEntry(section_name, "username", "");
    netpassword = getEntry(section_name, "password", "");
    netdisk_ip = getEntry(section_name, "ip", "");
    netdisk_path = getEntry(section_name, "path", "");

    user_info->netdisk_info.netdisk_enable = (netdisk_enable == "true" ? true : false);
    ret = UserDBcheckError();
    if (ret == 0) {
        user_info->netdisk_info.netdisk_username = netusername;
        user_info->netdisk_info.netdisk_password = netpassword;
        user_info->netdisk_info.netdisk_ip = netdisk_ip;
        user_info->netdisk_info.netdisk_path = netdisk_path;
    } else {
        if (ret == -USRERR_NOENTRY)
            ret = -USRERR_NONETDISK;
    }
    return ret;
}

int UserNetdiskDB::storeUserInfo(struct UserInfo &user_info)
{
    char section_name[ENTRY_LEN];
    struct NetdiskInfo netdisk_info;
    string username;
    string netdisk_enable;

    username = user_info.username;
    netdisk_info = user_info.netdisk_info;
    if (netdisk_info.netdisk_enable)
        netdisk_enable = "true";
    else
        netdisk_enable = "false";

    snprintf(section_name, ENTRY_LEN, "%s_netdisk", username.c_str());
    setEntry(section_name, "enable", netdisk_enable.c_str());
    setEntry(section_name, "username", netdisk_info.netdisk_username.c_str());
    setEntry(section_name, "password", netdisk_info.netdisk_password.c_str());
    setEntry(section_name, "ip", netdisk_info.netdisk_ip.c_str());
    setEntry(section_name, "path", netdisk_info.netdisk_path.c_str());
    return UserDBcheckError();
}

int UserNetdiskDB::deleteUserInfo(string &username)
{
    char section_name[ENTRY_LEN];

    snprintf(section_name, ENTRY_LEN, "%s_netdisk", username.c_str());
    deleteSection(section_name);
    return UserDBcheckError();
}


int DevInterfaceDB::readDevInterfaceInfo(DevInterfaceInfo &inter_info)
{
    int ret;
    inter_info.interface_info = getEntry("default", "inter_info", "");
    string net_passthrough = getEntry("default", "net_passthrough", "0");
    inter_info.net_passthrough = atoi(net_passthrough.c_str());
    ret = UserDBcheckError();
    if (ret == -USRERR_NOENTRY)
        ret = -USRERR_NONETDISK;
    return ret;
}

int DevInterfaceDB::storeDevInterfaceInfo(const DevInterfaceInfo &inter_info)
{
    setEntry("default", "inter_info", inter_info.interface_info.c_str());
    setEntry("default", "net_passthrough", inter_info.net_passthrough);
    return UserDBcheckError();
}



void GroupInfoDB::_groupPolicyLoad(const char *section, struct PolicyInfo *group_policy)
{
    string netuse, outctr_day;

    //usb_policy = getEntry(section, "usb", "{}");
    netuse = getEntry(section, "netuse", "false");
    //group_policy->usb_policy = usb_policy;
    group_policy->net_policy = (netuse == "true" ? true : false);

    if (strcmp(section, "public_policy") == 0) {
    	outctr_day = getEntry(section, "outctrl_day", "0");
    	group_policy->pub.outctrl_day = atoi(outctr_day.c_str());
    }
}

int GroupInfoDB::readUserInfo(struct UserInfo *user_info)
{
    char section_name[ENTRY_LEN];
    int ret;

    snprintf(section_name, ENTRY_LEN, "g%d_policy", user_info->group_id);
    _groupPolicyLoad(section_name, &user_info->policy_info);
    ret = UserDBcheckError();
    if (ret == -USRERR_NOENTRY)
        ret = -USRERR_NOGROUP;
    return ret;
}

int GroupInfoDB::readPublicPolicy(struct PolicyInfo *public_policy)
{
    int ret;

    _groupPolicyLoad("public_policy", public_policy);
    ret = UserDBcheckError();
    if (ret == -USRERR_NOENTRY)
        ret = -USRERR_NOPUBLIC;
    return ret;
}

void GroupInfoDB::_groupPolicyStore(const char *section, struct PolicyInfo &group_policy)
{
    string netuse;

    if (group_policy.net_policy)
        netuse = "true";
    else
        netuse = "false";
    //setEntry(section, "usb", group_policy.usb_policy.c_str());
    setEntry(section, "netuse", netuse.c_str());

    if (strcmp(section, "public_policy") == 0) {
    	setEntry(section, "outctrl_day", group_policy.pub.outctrl_day);
    }
}

int GroupInfoDB::storeUserInfo(struct UserInfo &user_info)
{
    char section_name[ENTRY_LEN];
    struct PolicyInfo group_policy;
    int groupid;

    groupid = user_info.group_id;
    group_policy = user_info.policy_info;
    snprintf(section_name, ENTRY_LEN, "g%d_policy", groupid);
    setEntry(section_name, "group", groupid);
    _groupPolicyStore(section_name, group_policy);
    return UserDBcheckError();
}

int GroupInfoDB::storePublicPolicy(struct PolicyInfo &public_policy)
{
    _groupPolicyStore("public_policy", public_policy);
    return UserDBcheckError();
}

int GroupInfoDB::deleteUserGroup(int &group_id)
{
    char section_name[ENTRY_LEN];

    snprintf(section_name, ENTRY_LEN, "g%d_policy", group_id);
    deleteSection(section_name);
    return UserDBcheckError();
}

int GroupInfoDB::deletePublicGroup()
{
    deleteSection("public_policy");
    return UserDBcheckError();
}

int GroupInfoDB::getAllGroups(vector<int> &groups)
{
    const char **sections;
    int i, sec_count;
    int group_id;

    sections = getAllSections(&sec_count);
    if (sections == NULL) {
        return UserDBcheckError();
    }

    for (i = 0; i < sec_count; i++) {
        group_id = getEntry(sections[i], "group", -1);
        if (group_id == -1)
            continue;
        groups.push_back(group_id);
    }
    free(sections);
    return 0;
}


string UserStatusDB::getLoginedUser()
{
    string logined_user;

    logined_user = getEntry("current", "username", "");
    return logined_user;
}

void UserStatusDB::setLoginedUser(string &username)
{
    setEntry("current", "username", username.c_str());
}

bool UserStatusDB::getUserLoginedStatus()
{
    string logined_status;

    logined_status = getEntry("current", "logined", "false");
    if (logined_status == "true")
        return true;
    else
        return false;
}

void UserStatusDB::setUserLoginedStatus(bool logined)
{
    string logined_status;

    if (logined)
        logined_status = "true";
    else
        logined_status = "false";
    setEntry("current", "logined", logined_status.c_str());
}

int UserStatusDB::getSavedState()
{
    int saved_state;

    saved_state = getEntry("current", "state", STATUS_NONE);
    return saved_state;
}

void UserStatusDB::setSavedState(int state)
{
    setEntry("current", "state", state);
}

bool UserStatusDB::getUIPermitted()
{
    string ui_permit;

    // by default, UI is permitted.
    ui_permit = getEntry("current", "UIpermit", "true");
    if (ui_permit == "true")
        return true;
    else
        return false;
}

void UserStatusDB::setUIPermitted(bool permitted)
{
    string ui_permit;

    if (permitted)
        ui_permit = "true";
    else
        ui_permit = "false";
    setEntry("current", "UIpermit", ui_permit.c_str());
}

bool UserStatusDB::get_localmode_flag()
{
    string on_localmode = getEntry("current", "localmode", "false");
    return (on_localmode == "true" ? true : false);
}

void UserStatusDB::set_localmode_flag(bool flag)
{
    string on_localmode = (flag ? "true" : "false");
    setEntry("current", "localmode", on_localmode.c_str());
}

int UserStatusDB::delete_user_status_ini()
{
    if (remove(USER_STATUS_INI) < 0) {
        LOG_PERROR("the file is not exits, delete error\n");
        return -1;
    };

    rc_system("sync");
    return 0;
}

void UserLastLoginedDB::getLoginedUser(string *username, string *password, int *remember_flag)
{
    string remember;

    *username = getEntry("last", "username", "");
    *password = getEntry("last", "password", "");
    remember = getEntry("last", "remember", "false");
    if (remember == "true")
        *remember_flag = 1;
    else
        *remember_flag = 0;
}

void UserLastLoginedDB::setLoginedUser(string &username, string &password, int &remember_flag)
{
    if (remember_flag) {
        setEntry("last", "remember", "true");
        setEntry("last", "username", username.c_str());
        setEntry("last", "password", password.c_str());
    } else {
        setEntry("last", "remember", "false");
        deleteEntry("last", "username");
        deleteEntry("last", "password");
    }
}

int SsidWhiteListDB::readssidWhiteUser(vector<string> &whitelist)
{
    const char **sections;
    int i = 0, sec_count = 0;
    string ssid;
    int ret = 0;
    vector<string>::iterator it;

    sections = getAllSections(&sec_count);

    if (sections == NULL) {
        return UserDBcheckError();
    }

    for (i = 0; i < sec_count; i++) {
        ssid = getEntry(sections[i], "ssid", ssid.c_str());
        if ((ret = UserDBcheckError()) != 0) {
            free(sections);
            whitelist.clear();
            return ret;
        }
        whitelist.push_back(ssid);
    }

    for (it = whitelist.begin(); it != whitelist.end(); it++) {
        *it = gloox::Base64::decode64(*it);
    }
    
    free(sections);
    return ret;
}


void SsidWhiteListDB::storessidWhiteUser(vector<string> &whitelist)
{
    char section_name[ENTRY_LEN];
    vector<string> _whitelist;
    vector<string>::iterator it;
    int i = 1;

    _whitelist.assign(whitelist.begin(), whitelist.end());
    
    for (it = _whitelist.begin(); it != _whitelist.end(); it++) {
        *it = gloox::Base64::encode64(*it);
        memset(section_name, 0, sizeof(section_name));
        snprintf(section_name, ENTRY_LEN, "SSID%d", i);
        setEntry(section_name, "ssid", (*it).c_str());
        i++;
    }
}

int SsidWhiteListDB::deletessidWhite()
{
    int i = 0, ret = 0;
    int sec_count = 0;
    const char **sections;

    sections = getAllSections(&sec_count);
    if (sections == NULL) {
        return UserDBcheckError();
    }

    for (i = 0; i < sec_count; i++) {
        deleteSection(sections[i]);
        if ((ret = UserDBcheckError()) != 0) {
            break;
        }
    }

    free(sections);
    return ret;
}

bool VmmodeInfoDB::checkvmmode_section(const string &section)
{
    return findSection(section.c_str());
}

bool VmmodeInfoDB::checkvmmode_entry(const string &section, const string &entry)
{
    return findEntry(section.c_str(), entry.c_str());
}

// vmmode
int VmmodeInfoDB::getvmmode_value(const string &section, const string &entry, string &value)
{
    int ret = 0;

    LOG_INFO("section=%s entry=%s", section.c_str(), entry.c_str());
  
    value = getEntry(section.c_str(), entry.c_str(), value.c_str());
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("getvmmode_vmmode error = %d", ret);
        return ret;
    }
    return ret;
} 


void DisplayinfoDB::set_display_info(const DisplayInfo &dpi_info)
{
#define DISPLAY_OPT_FLAG       (1<<0)
//#define DISPLAY_CUR_FLAG       (1<<1)
    setEntry("resolution_info", "width", dpi_info.width);
    setEntry("resolution_info", "height", dpi_info.height);

    if(dpi_info.flag & DISPLAY_OPT_FLAG) {
        setEntry("resolution_info", "opt", "True");
    } else {
        setEntry("resolution_info", "opt", "False");
    }
    if(dpi_info.custom == TRUE) {
        setEntry("resolution_info", "custom", "True");
    } else {
        setEntry("resolution_info", "custom", "False");
    }
#undef DISPLAY_OPT_FLAG
//#undef DISPLAY_CUR_FLAG
}

int DisplayinfoDB::delete_display_info_ini()
{
    if(remove(USER_DISPLAY_INFO_INI) < 0) {
        LOG_PERROR("the file is not exits, delete error\n");
        return -1;
     };

    rc_system("sync");
    return 0;
}

void DisplayinfoDB::set_ext_display_info(const struct DisplayInfo &dpi_info, int num)
{
    char section[64] = {0};

    sprintf(section, "extend_screen%d_info", num + 1);
#define DISPLAY_OPT_FLAG       (1<<0)
//#define DISPLAY_CUR_FLAG       (1<<1)
    setEntry(section, "width", dpi_info.width);
    setEntry(section, "height", dpi_info.height);

    if(dpi_info.flag & DISPLAY_OPT_FLAG) {
        setEntry(section, "opt", "True");
    } else {
        setEntry(section, "opt", "False");
    }
    if(dpi_info.custom == TRUE) {
        setEntry(section, "custom", "True");
    } else {
        setEntry(section, "custom", "False");
    }
#undef DISPLAY_OPT_FLAG
}

void DisplayinfoDB::get_ext_display_info(ExtDisplayInfo *res_info)
{
    string custom_buf;
    int i;
    char section[64] = {0};

    for (i = 0; i < MAX_EXT_SCREEN_NUM; i++) {
        memset(section, 0, sizeof(section));
        sprintf(section, "extend_screen%d_info", i + 1);
        res_info->width[i] = getEntry(section, "width", 0);
        res_info->height[i] = getEntry(section, "height", 0);
        custom_buf = getEntry(section, "custom", "False");
        if (custom_buf == "True") {
            res_info->custom[i] = TRUE;
        } else if (custom_buf == "False") {
            res_info->custom[i] = FALSE;
        }
    }

}

void DisplayinfoDB::get_ext_display_info(int port, string &width, string &height, int &custom)
{
    char section[64] = {0};
    string custom_buf;

    sprintf(section, "extend_screen%d_info", port + 1);

    width = getEntry(section, "width", "0");
    height = getEntry(section, "height", "0");
    custom_buf = getEntry(section, "custom", "False");
    if (custom_buf == "True") {
        custom = TRUE;
    } else if (custom_buf == "False") {
        custom = FALSE;
    }
}

int DisplayinfoDB::is_display_info_section_exist(const char* section)
{
    if (access(USER_DISPLAY_INFO_INI, F_OK) != 0) {
        LOG_PERROR("the file is not exits, delete section error\n");
        return 0;
    }

    return findSection(section);
}

int DisplayinfoDB::delete_resolution_info_section()
{
    if (access(USER_DISPLAY_INFO_INI, F_OK) != 0) {
        LOG_PERROR("the file is not exits, delete section error\n");
        return -1;
    }

    deleteSection("resolution_info");
    return UserDBcheckError();
}

void DisplayinfoDB::get_display_info(int *width, int *height, int *custom)
{
    string custom_buf;

    *width = getEntry("resolution_info", "width", 0);
    *height = getEntry("resolution_info", "height", 0);
    custom_buf = getEntry("resolution_info", "custom", "False");
    if (custom_buf == "True") {
        *custom = TRUE;
    } else if (custom_buf == "False") {
        *custom = FALSE;
    }
}

void HdmiAudioinfoDB::set_hdmi_audio_info(int hdmiaudio)
{
    if (hdmiaudio) {
        setEntry("hdmiaudio_info", "hdmiaudio", "support");
    } else {
        setEntry("hdmiaudio_info", "hdmiaudio", "nonsupport");
    }
    saveUserDB();
}

/**
 * Description: Gets information on whether hdmi audio output is supported
 * Return: 0 can't support  1 support
 */
int HdmiAudioinfoDB::get_hdmi_audio_info()
{
    string hdmiaudio;

    hdmiaudio = getEntry("hdmiaudio_info", "hdmiaudio", " ");
    if (strcmp(hdmiaudio.c_str(), "support")) {
        return 0;
    }
    return 1;
}

void HdmiAudioinfoDB::delete_hdmi_audio_info()
{
    if (remove(USER_HDMI_AUDIO_INI) < 0) {
        LOG_PERROR("the file is not exits, delete error\n");
        return;
    }
    rc_system("sync");
    return;
}

void DiskInfoDB::set_disk_info(const DiskInfo_t & diskinfo)
{
     char section_name[ENTRY_LEN];

     strcpy(section_name, diskinfo.disk_name.c_str());
     setEntry(section_name, "enable", diskinfo.is_enable);
     setEntry(section_name, "size", diskinfo.size);
}

int DiskInfoDB::get_disk_info(const string &disk_name, DiskInfo_t &diskinfo)
{
    int ret = 0;

    diskinfo.is_enable  = getEntry(disk_name.c_str(), "enable", 0);
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("db get_disk_info enable error %d", ret);
        return ret;
    }

    diskinfo.size = getEntry(disk_name.c_str(), "size", 0);
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("db get_disk_info size error %d", ret);
        return ret;
    }

    diskinfo.disk_name = disk_name;
    return ret;
}

int DiskInfoDB::delete_diskinfo_ini()
{
    if(remove(DISKINFO_INI) < 0) {
        LOG_PERROR("the file is not exits, delete error\n");
        return -1;
     };

    rc_system("sync");
    return 0;
}

int AuthDB::authIniInit()
{
    string passwd;
    int ret = 0;

    passwd = getEntry("default", "password", "");
    if ((ret = UserDBcheckError()) != 0) {
        return ret;
    }

    if (!passwd.empty()) {
        setEntry("default", "password", "");
        setEntry("default", "password_xor", gloox::password_codec_xor(passwd, true).c_str());
        saveUserDB();
    }


    return UserDBcheckError();
}

int AuthDB::getAuthInfo(int &auth_type, int &auto_connect, string &username, string &password)
{
    string passwd;

    LOG_DEBUG("%s", __func__);
    auth_type = getEntry("default", "authtype", 0);
    auto_connect = getEntry("default", "auto_connect", 1);
    username = getEntry("default", "username", "");

    passwd = getEntry("default", "password_xor", "");
    LOG_DEBUG("password_xor %s", passwd.c_str());
    password = gloox::password_codec_xor(passwd, false);
    return UserDBcheckError();
}

int AuthDB::setAuthInfo(struct AuthInfo &auth_info)
{
    setEntry("default", "authtype",  auth_info.auth_type);
    if (auth_info.auth_type == AUTH_WEB) {
        // we do not need to store web's user info.
         deleteEntry("default", "username");
         deleteEntry("default", "password");
         deleteEntry("default", "password_xor");
    } else {
        setEntry("default", "username", auth_info.auth_user.c_str());
        setEntry("default", "password", "");
        setEntry("default", "password_xor", gloox::password_codec_xor(auth_info.auth_passwd, true).c_str());
    }

    return UserDBcheckError();
}

int AuthDB::setAuthConnect(int auto_connect)
{
    setEntry("default", "auto_connect", auto_connect);
    return UserDBcheckError();
}

int AuthDB::delete_auth_info()
{
    deleteEntry("default", "username");
    deleteEntry("default", "password");
    deleteEntry("default", "password_xor");
    return 0;
}


//severip
void ServeripInfoDB::set_serverip_info(const string& serverip)
{
    int ret = 0;

    setEntry("default", "server_ip", serverip.c_str());
    saveUserDB();
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("save set_serverip_info ini err %d", ret);
    } 

    return;
}

void ServeripInfoDB::set_last_serverip_info(const string& last_serverip)
{
    int ret = 0;

    setEntry("default", "last_server_ip", last_serverip.c_str());
    saveUserDB();
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("save last_serverip_info ini err %d", ret);
    } 

    return;
}

const string& ServeripInfoDB::get_serverip_info()
{
    int ret = 0;

    _serverip.clear();
    _serverip = getEntry("default", "server_ip", "");
    LOG_DEBUG("get_serverip_info is %s\n", _serverip.c_str());
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("get serverip_info ini err %d", ret);
    } 

    return _serverip;
}

const string& ServeripInfoDB::get_last_serverip_info()
{
    int ret = 0;
    _last_serverip.clear();
    _last_serverip = getEntry("default", "last_server_ip", "");
    LOG_DEBUG("get_serverip_info is %s\n", _last_serverip.c_str());
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("get last_serverip_info ini err %d", ret);
    } 

    return _last_serverip;
}

#if 0
//version_client_idv
void VersioninfoDB::set_info_form_ini()
{
    int ret = 0;
    
    _version_info.main_version      = getEntry("LINUX-RCC-Client", "ruijie.rcc.client.mainVersion", 0);
    _version_info.minor_version     = getEntry("LINUX-RCC-Client", "ruijie.rcc.client.minorVersion", 9);
    _version_info.third_version     = getEntry("LINUX-RCC-Client", "ruijie.rcc.client.threeVersion", 0);
    _version_info.fourth_version    = getEntry("LINUX-RCC-Client", "ruijie.rcc.client.fourVersion", 1);
    _version_info.extra_first       = getEntry("LINUX-RCC-Client", "ruijie.rcc.client.extra1", "_R0");
    _version_info.extra_second      = getEntry("LINUX-RCC-Client", "ruijie.rcc.client.extra2", "");
    _version_info.build_date        = getEntry("LINUX-RCC-Client", "ruijie.rcc.client.buildDate", "");

    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("get last_serverip_info ini err %d", ret);
    } 
}
#endif

const VersionInfo& VersioninfoDB::get_version_info()
{
    int ret = 0;
    
    _version_info.main_version      = getEntry("LINUX-RCC-Client", "ruijie.rcc.client.mainVersion", 0);
    _version_info.minor_version     = getEntry("LINUX-RCC-Client", "ruijie.rcc.client.minorVersion", 9);
    _version_info.third_version     = getEntry("LINUX-RCC-Client", "ruijie.rcc.client.threeVersion", 0);
    _version_info.fourth_version    = getEntry("LINUX-RCC-Client", "ruijie.rcc.client.fourVersion", 1);
    _version_info.extra_first       = getEntry("LINUX-RCC-Client", "ruijie.rcc.client.extra1", "_R0");
    _version_info.extra_second      = getEntry("LINUX-RCC-Client", "ruijie.rcc.client.extra2", "");
    _version_info.build_date        = getEntry("LINUX-RCC-Client", "ruijie.rcc.client.buildDate", "");

    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("get last_serverip_info ini err %d", ret);
    }
    
    return _version_info;
}
//hostname
void HostnameInfoDB::set_hostname_info(const string& hostname)
{
    int ret = 0;
    
    LOG_DEBUG("set_hostname_info is %s\n", hostname.c_str());
    setEntry("default", "hostname", hostname.c_str());
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("set hostname_info ini err %d", ret);
    } 
    saveUserDB();
}

const string& HostnameInfoDB::get_hostname_info()
{
    int ret = 0;

    _hostname.clear();
    _hostname = getEntry("default", "hostname", "rcd");
    LOG_DEBUG("get_hostname_info is %s\n", _hostname.c_str());
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("get hostnameinfo ini err %d", ret);
    } 

    return _hostname;
}

//modeinfo
#if 0
void ModeInfoDB:: set_mode_info_init()
{
    _mode_info.mode  = (Mode)getEntry("default", "mode", SPECIAL_MODE);
    _mode_info.bind_user.username= getEntry("default", "bind_username", "");

}
#endif
void ModeInfoDB:: set(const ModeInfo& modeinfo)
{
    int ret = 0;
    
    setEntry("default", "mode", modeinfo.mode);
    setEntry("default", "bind_username", modeinfo.bind_user.username.c_str());
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("set modeinfo ini err %d", ret);
    } 
	saveUserDB();
}
 ModeInfo& ModeInfoDB:: get()
{
    int ret = 0;
    
    _mode_info.mode = (Mode)getEntry("default", "mode", SPECIAL_MODE);
    _mode_info.bind_user.username = getEntry("default", "bind_username", "");
    LOG_DEBUG("get mode info bind_user is %s\n", _mode_info.bind_user.username.c_str());
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("get modeinfo ini err %d", ret);
    } 
    return _mode_info;
}

//osupdatepolicy inio
 void OsupdataPolicyInfoDB::set_Osupdatepllicy_inifo_init()
{
    string product_name;
    string str;
    VersionInfo  version_info;

    _type = FORCE_UPDATA;
    get_product_id(_section, sizeof(_section));
    product_name = getEntry(_section, "product_name", "");
    if (product_name.empty()) {
        return;
    }
    str = getEntry(_section, "min_version", "");

    if (str2version(str, version_info)) {
        _min_version.push_back(version_info);
    } else {
        LOG_ERR("err version str %s\n", str.c_str());
    }

    get_version_list(_white_list, "white_list");

    get_version_list(_black_list, "black_list");

    if (_min_version.size() > 0 || _white_list.size() > 0 || _black_list.size()) {
        _type = POLICY_UPDATA;
    }

    if (_min_version.size() > 0) {
        LOG_DEBUG("Product [%s] show min version\n", product_name.c_str());
        show_version_info(_min_version);
    }

    if (_white_list.size() > 0) {
        LOG_DEBUG("show _white_list\n");
        show_version_info(_white_list);
    }
    if (_black_list.size() > 0) {
        LOG_DEBUG("show _black_list\n");
        show_version_info(_black_list);
    }

}
 bool OsupdataPolicyInfoDB::allow_version(const string& version_str)
 {
    VersionInfo version_info;
    unsigned int i;
    if ( _type == FORCE_UPDATA) {
        LOG_DEBUG("not find rule upgrade (%d:%d:%d)\n", version_info.main_version, version_info.minor_version, version_info.fourth_version);
        return true;
    }

    if (!str2version(version_str, version_info)) {
        LOG_ERR("err version str allow upgrade %s\n", version_str.c_str());
        return true;
    }

    if (_white_list.size() > 0) {
        for (i = 0; i < _white_list.size(); i++) {
            if (_white_list[i] == version_info) {
                LOG_DEBUG("_white_list allow upgrade (%d:%d:%d)\n", version_info.main_version, version_info.minor_version, version_info.fourth_version);
                return true;
            }
        }

        if (_min_version.size() == 0) {
            LOG_DEBUG("not in white forbid upgrade (%d:%d:%d)\n", version_info.main_version, version_info.minor_version, version_info.fourth_version);
            return false;
        }
    }

    if (_black_list.size() > 0) {
        for (i = 0; i < _black_list.size(); i++) {
            if (_black_list[i] == version_info) {
                LOG_DEBUG("_black_list forbid upgrade (%d:%d:%d)\n", version_info.main_version, version_info.minor_version, version_info.fourth_version);
                return false;
            }
        }
    }

    if (_min_version.size() > 0) {
        if (_min_version[0] > version_info) {
            LOG_DEBUG("_min_version forbid upgrade (%d:%d:%d)\n", version_info.main_version, version_info.minor_version, version_info.fourth_version);
            return false;
        } else {
            LOG_DEBUG("_min_version allow upgrade (%d:%d:%d)\n", version_info.main_version, version_info.minor_version, version_info.fourth_version);
            return true;
        }
    }
    LOG_DEBUG("not valid rule or black list allow upgrade (%d:%d:%d)\n", version_info.main_version, version_info.minor_version, version_info.fourth_version);
    return true;
 }

bool OsupdataPolicyInfoDB::isNum(string str)
{
    stringstream sin(str);
    double d;
    char c;

    if(!(sin >> d))
        return false;
    if (sin >> c)
        return false;
    return true;
}
bool OsupdataPolicyInfoDB::str2version(string str, VersionInfo &info)
{
    size_t begin;
    size_t main_end_pos;
    size_t minor_end_pos;
    size_t fourth_pos, fourth_end_pos;
    string patch_str;

    LOG_DEBUG("Parse version string: %s", str.c_str());
    begin = str.find_last_of("V");
    if (begin >= 0) {
        begin++;
    } else {
        begin = str.find_last_of("v");
        if (begin >= 0) {
            begin++;
        } else {
            begin = 0;
        }
    }

    main_end_pos = str.find(".", begin);
    if (str.npos == main_end_pos) {
    return false;
    }

    LOG_DEBUG("  Main version: %s", str.substr(begin, main_end_pos-begin).c_str());
    if (!isNum(str.substr(begin, main_end_pos-begin))) {
        return false;
    } else {
        info.main_version = atoi(str.substr(begin, main_end_pos-begin).c_str());
    }

    main_end_pos++;
    minor_end_pos = str.find("_", main_end_pos);
    if (str.npos == minor_end_pos) {
        return false;
    }

    LOG_DEBUG("  Minor version: %s", str.substr(main_end_pos, minor_end_pos-main_end_pos).c_str());
    if (!isNum(str.substr(main_end_pos, minor_end_pos-main_end_pos))) {
        return false;
    } else {
        info.minor_version = atoi(str.substr(main_end_pos, minor_end_pos-main_end_pos).c_str());
    }

    fourth_pos = str.find_last_of(".");
    if (fourth_pos <= minor_end_pos || fourth_pos >= str.length()) {
        return false;
    }

    fourth_pos++;
    fourth_end_pos = str.find("_", fourth_pos);
    if (fourth_end_pos == str.npos) {
        patch_str = str.substr(fourth_pos);
    } else {
        patch_str = str.substr(fourth_pos, fourth_end_pos - fourth_pos);
    }
    LOG_DEBUG("  Patch version: %s\n", patch_str.c_str());
    if (!isNum(patch_str)) {
        return false;
    } else {
        info.fourth_version = atoi(patch_str.c_str());
    }

    return true;
}
void OsupdataPolicyInfoDB::add_list(string &buf, vector<VersionInfo> &list)
{
    size_t pos1, pos2;
    VersionInfo version_info;
    pos2 = buf.find(",");
    pos1 = 0;

    while(buf.npos != pos2)
    {
        if (str2version(buf.substr(pos1, pos2-pos1), version_info)) {
            list.push_back(version_info);
        } else {
            LOG_ERR("err version str %s\n", buf.substr(pos1, pos2-pos1).c_str());
    }

    pos1 = pos2 + 1;
        pos2 = buf.find(",", pos1);
    }
    if(pos1 != buf.length()) {
        if (str2version(buf.substr(pos1), version_info)) {
            list.push_back(version_info);
        } else {
            LOG_ERR("err version str %s\n", buf.substr(pos1).c_str());
        }
    }

    return;
}
void OsupdataPolicyInfoDB::get_version_list(vector<VersionInfo> &list, string kw)
{
    string line;
    int i=1;
    char key[32];

    while (1) {
        snprintf(key, sizeof(key)-1, "%s%d", kw.c_str(), i);
        line = getEntry(_section, key, "");
        if (line.empty()) {
            break;
        }
        add_list(line, list);
        i++;
    }

    return;
}
void OsupdataPolicyInfoDB::show_version_info(vector<VersionInfo> &list)
{
    vector<VersionInfo>::const_iterator user_it;

    for (user_it = list.begin(); user_it != list.end(); user_it++) {
        LOG_DEBUG("main:%d; minor:%d; fourth:%d\n", user_it->main_version, user_it->minor_version, user_it->fourth_version);
    }
}

//devpolicy info
void DevpolicyInfoDB::delete_update_sync()
{
    delete _update_sync;
}

void DevpolicyInfoDB::set_devpolicy_info()
{
    setEntry("default", "allow_guestlogin", _policy_info.allow_guestlogin);
    setEntry("default", "allow_netdisk", _policy_info.allow_netdisk);
    setEntry("default", "allow_otherslogin", _policy_info.allow_otherslogin);
    setEntry("default", "allow_recovery", _policy_info.allow_recovery);
    setEntry("default", "allow_userdisk", _policy_info.allow_userdisk);
    saveUserDB();
}

void DevpolicyInfoDB::set_devpolicy_info_init()
{
    //string policy_str;
    _update = false;
    _update_sync = new DevPolicyEvent();
    get();
    set_devpolicy_info();
}

void DevpolicyInfoDB::update(const DevPolicyInfo& policy_info)
{
    _update = true;
    if (_policy_info != policy_info) {
        _policy_info = policy_info;
        set_devpolicy_info();
    }

}

const struct DevPolicyInfo& DevpolicyInfoDB:: get()
{

    _policy_info.allow_guestlogin = getEntry("default", "allow_guestlogin", -1);
    _policy_info.allow_netdisk = getEntry("default", "allow_netdisk", -1);
    _policy_info.allow_otherslogin = getEntry("default", "allow_otherslogin", -1);
    _policy_info.allow_recovery = getEntry("default", "allow_recovery", -1);
    _policy_info.allow_userdisk = getEntry("default", "allow_userdisk", -1);

    return _policy_info;
}

bool DevpolicyInfoDB:: wait4update(unsigned long long nano)
{
    return _update_sync->wait_timeout(nano);
}

void DevpolicyInfoDB::update_finish()
{
    _update_sync->response();
}

bool DevpolicyInfoDB::update_valid()
{
    return _update;
}

bool DevpolicyInfoDB::check_recovery()
{
    _policy_info.allow_recovery = getEntry("default", "allow_recovery", -1);
    if (_policy_info.allow_recovery == true || _policy_info.allow_recovery == false) {
        return true;
    }
        return false;
}

bool DevpolicyInfoDB::allow_recovery()
{
    if (check_recovery()) {
        return _policy_info.allow_recovery;
    }
        return true;
}

bool DevpolicyInfoDB::check_userdisk()
{
    
    _policy_info.allow_userdisk = getEntry("default", "allow_userdisk", -1);
    if (_policy_info.allow_userdisk == true || _policy_info.allow_userdisk == false) {
        return true;
    }
        return false;
}

int DevpolicyInfoDB::allow_userdisk()
{
    if (check_userdisk()) {
        return _policy_info.allow_userdisk;
    }
        return -1;
}

bool DevpolicyInfoDB::check_gusetlogin()
{
    _policy_info.allow_guestlogin = getEntry("default", "allow_guestlogin", -1);
    if (_policy_info.allow_guestlogin == true || _policy_info.allow_guestlogin == false) {
        return true;
    }
        return false;
}

bool DevpolicyInfoDB::allow_gusetlogin()
{
    if (check_gusetlogin()) {
        return _policy_info.allow_guestlogin;
    }
        return true;
}

bool DevpolicyInfoDB::check_otherslogin()
{
    _policy_info.allow_otherslogin = getEntry("default", "allow_otherslogin", -1);
    if (_policy_info.allow_otherslogin == true || _policy_info.allow_otherslogin == false) {
        return true;
    }
        return false;
}

bool DevpolicyInfoDB::allow_otherslogin()
{
    if (check_otherslogin()) {
        return _policy_info.allow_otherslogin;
    }
        return true;
}


//dev_status.ini
void DevstatusInfoDB::devstatus_info_init()
{

    _locked = getEntry("default", "locked", 0);
    setEntry("default", "locked", to_string(_locked).c_str());
    saveUserDB();
}

void DevstatusInfoDB::DevLock(bool lock)
{
    int ret = 0;

    _locked = lock;
    if (_locked == false) {
        _lock_type = 0;
    }
    setEntry("default", "locked", to_string(_locked).c_str());
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("save dev_status ini err %d", ret);
    } 
    saveUserDB();
}

const bool DevstatusInfoDB::IsLocked()
{
    int ret = 0;
    
    _locked = getEntry("default", "locked", 0);
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("get modeinfo ini err %d", ret);
    } 
    LOG_INFO("IsLocked _locked is %s\n",to_string(_locked).c_str());
    return _locked;
}

void DevstatusInfoDB::set_mode_info(const ModeInfo& mode_info)
{
    _mode_info.mode = mode_info.mode;
    _mode_info.bind_user.username = mode_info.bind_user.username;
}

ModeInfo DevstatusInfoDB::get_mode_info()
{
    return _mode_info;
}

void DevstatusInfoDB::set_dev_policy(const DevPolicyInfo& dev_info)
{
    _dev_info = dev_info;
}

DevPolicyInfo DevstatusInfoDB::get_dev_policy()
{
    return _dev_info;
}

void DevstatusInfoDB::set_lock_type(int lock_type)
{   
    _lock_type = lock_type;
}

int DevstatusInfoDB::get_lock_type()
{
    return _lock_type;
}

//reserverd_memory.ini
void ReservedMemoryInfoDB::reserved_memory_info_init()
{
    _reserved_memory= getEntry("default", "reserved_memory", RCC_RESERVED_MEMORY_PASS_DEFAULT);
    setEntry("default", "reserved_memory",_reserved_memory);
    saveUserDB();
}

const int ReservedMemoryInfoDB::get_resvered_momory_info()
{
    int ret = 0;
    
    _reserved_memory = getEntry("default", "reserved_memory", RCC_RESERVED_MEMORY_PASS_DEFAULT);
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("get resvered momory  ini err %d", ret);
    } 
    return _reserved_memory;
}

void ReservedMemoryInfoDB::set_resvered_memory_info(int reserved_memory)
{
    int ret = 0;
    
    setEntry("default", "reserved_memory",reserved_memory);
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("set resvered memory ini err %d", ret);
    } 
    saveUserDB();
}

//newdeploy_ctrl.ini
void NewDeployCtrlInfoDB::newdeploy_ctrl_info_init()
{
    _new_terminal = getEntry("default", "new_terminal", 1);
    _download_status = getEntry("default", "download_status", 0);
    
    LOG_INFO("newdeploy_ctrl_info_init is %s, %s\n",to_string(_new_terminal).c_str(), to_string(_download_status).c_str())
    setEntry("default", "new_terminal", to_string(_new_terminal).c_str());
    setEntry("default", "download_status", to_string(_download_status).c_str());
    saveUserDB();
}

bool NewDeployCtrlInfoDB::get_new_terminal()
{
    int ret = 0;
    
    _new_terminal = getEntry("default", "new_terminal", 0);
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("get new terminal ini err %d", ret);
    } 
    LOG_INFO("_new_terminal is %s\n",to_string(_new_terminal).c_str());
    return _new_terminal;
}

void NewDeployCtrlInfoDB::set_new_terminal(bool new_terminal)
{
    int ret = 0;

    setEntry("default", "new_terminal", to_string(new_terminal).c_str());
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("set new terminal ini err %d", ret);
    } 

    saveUserDB();
}

bool NewDeployCtrlInfoDB::get_download_status()
{
    int ret = 0;
    
    _download_status = getEntry("default", "download_status", 0);
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("get download status ini err %d", ret);
    } 
    LOG_INFO("get _download_status is %s\n",to_string(_download_status).c_str());

    return _download_status;
}

void NewDeployCtrlInfoDB::set_download_status(bool download_status)
{
    int ret = 0;
    
    LOG_INFO("set _download_status is %s\n",to_string(download_status).c_str())
    setEntry("default", "download_status", to_string(download_status).c_str());
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("set download status ini err %d", ret);
    } 
    saveUserDB();
}

#if 0
//rcdusbfilter
int RccUsbFliterInfoDB::set_rccusbfilter_info(const void *date)
{

    int ret = 0;

    LOG_INFO("RccUsbFliterInfoDB set_rccusbfilter_info is %s\n",(char *)date);
    if(date) {
        _usbinfo = (UsbConfigInfo *)date;
        setEntry("","",(char*)_usbinfo);
        saveUserDB();
    }
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("set rccusbfiler status ini err %d", ret);
    } 

    return ret;
}

int RccUsbFliterInfoDB::get_rccusbfilter_info(const void *buf)
{
    int ret = 0;

    buf = getEntry("", "", "");
    LOG_DEBUG("get rccusbfilter info is %s\n", buf);
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("get rccusbfilter info %d", ret);
    }

    return ret;
}

//rcdusbconf
void RcdUsbConfInfoDB::set_rcdusbconf_info(const char * date)
{
    LOG_INFO("RcdUsbConfInfoDB set_rcdusbconf_info is %s\n",date);
    if (date) {
        setEntry("", "", date);
        saveUserDB();
    }
}
int RcdUsbConfInfoDB::get_rcdusbconf_info(const void * buf)
{
   int ret = 0;
    
    buf = getEntry("", "", "");
    LOG_DEBUG("get rccusbconf is %s\n", buf);
    if ((ret = UserDBcheckError()) != 0) {
        LOG_ERR("get rccusbconf is %d", ret);
    }
    
    return ret;
}
#endif
//networkinfo

void VmNetworkInfoDB::set_network_info_init()
{
    get_vm_network();
}
void VmNetworkInfoDB::set_vm_network(const NetworkInfo& network_info)
{

    _network_info = network_info;
    if(_network_info.dhcp == false) {
        setEntry("network-info", "dhcp", "false");
    } else {
        setEntry("network-info", "dhcp", "true");
    }
    switch(_network_info.netcard_speed) {
    case 0:
        setEntry("network-info", "netcard_speed", "0");
        break;
    case 10:
        setEntry("network-info", "netcard_speed", "10");
        break;
    case 100:
        setEntry("network-info", "netcard_speed", "100");
        break;
    case 1000:
        setEntry("network-info", "netcard_speed", "1000");
        break;
    }
    setEntry("network-info", "ip", _network_info.ip.c_str());
    setEntry("network-info", "submask", _network_info.submask.c_str());
    setEntry("network-info","gateway", _network_info.gateway.c_str());
    if(_network_info.auto_dns == false) {
        setEntry("network-info", "auto_dns", "false");
    } else {
        setEntry("network-info", "auto_dns", "true");
    }

    setEntry("network-info", "main_dns", _network_info.main_dns.c_str());
    setEntry("network-info", "back_dns", _network_info.back_dns.c_str()); 
    saveUserDB();
}


NetworkInfo VmNetworkInfoDB:: get_vm_network()
{
    string networkinfo_dhcp;
    string networkinfo_autodns;
    string net_speed;
    
    networkinfo_dhcp = getEntry("network-info", "dhcp", "true");
    if (networkinfo_dhcp == "true") {
        _network_info.dhcp = true;
    } else {
        _network_info.dhcp = false;
    }
    
    networkinfo_autodns  = getEntry("network-info", "auto_dns", "true");
    if (networkinfo_autodns == "true") {
        _network_info.auto_dns = true;
    } else {
        _network_info.auto_dns = false;
    }
    
    net_speed= getEntry("network-info", "netcard_speed", "");
    if(net_speed == "0") {
      _network_info.netcard_speed = 0;
    } else if(net_speed == "10") {
        _network_info.netcard_speed = 10;
    } else if(net_speed == "100") {
        _network_info.netcard_speed = 100;
    } else if(net_speed == "1000") {
        _network_info.netcard_speed = 1000;
    }

    _network_info.ip        = getEntry("network-info", "ip", "");
    _network_info.submask   = getEntry("network-info", "submask", "");
    _network_info.gateway   = getEntry("network-info", "gateway", "");
    
    _network_info.main_dns  = getEntry("network-info", "main_dns", "");
    _network_info.back_dns  = getEntry("network-info", "back_dns", "");
     LOG_DEBUG(" NetworkInfoDB .get is %s, %s\n", to_string(_network_info.dhcp).c_str(),to_string(_network_info.auto_dns).c_str());

    return _network_info;
}

//add for imaginfo.ini
void ImageInfoDB::set_imaginfo_init()
{
    int default_real_size = 0;
    int default_virt_size = 0;
    
    get();
    if ( _image_info.name.empty() == 0 ) {
        default_real_size = _image_manage->get_image_real_size(_image_info.name);
        default_virt_size = _image_manage->get_image_default_virt_size(_image_info.name);
    }
    _image_info.real_size = getEntry("image-info", "real_size", default_real_size);
    _image_info.virt_size = getEntry("image-info", "virt_size", default_virt_size);
    get_download_status();
    set(_image_info, _download_status);
}
void ImageInfoDB::set(const ImageInfo& image_info, bool downloading)
{
    
    setEntry("image-info", "id", image_info.id);
    setEntry("image-info", "name", image_info.name.c_str());
    setEntry("image-info", "version", image_info.version.c_str());
    setEntry("image-info", "torrent_url", image_info.torrent_url.c_str());
    setEntry("image-info", "md5", image_info.md5.c_str());
    setEntry("image-info", "ostype", image_info.ostype.c_str());
    setEntry("image-info", "real_size", image_info.real_size);
    setEntry("image-info", "virt_size", image_info.virt_size);
    //setEntry("image-info", "set_size", image_info.set_size);
    if (downloading == false) {
        setEntry("image-info", "_download_status", "false");
    } else {
        setEntry("image-info", "_download_status", "true");
    }

    setEntry("image-info", "layer_disk_number", image_info.layer_info.layer_disk_number);
    setEntry("image-info", "layer_disk_serial_1", image_info.layer_info.layer_disk_serial_1.c_str());
    setEntry("image-info", "layer_on_1", image_info.layer_info.layer_on_1.c_str());
    setEntry("image-info", "layer_x64_1", image_info.layer_info.layer_x64_1.c_str());

    saveUserDB();
}
void ImageInfoDB::set_ostype(const string &ostype)
{
    setEntry("image-info", "ostype", ostype.c_str());
    saveUserDB();
}

void ImageInfoDB::get_layerdisk_info(LayerDiskInfo &layer_info)
{
    layer_info.layer_disk_number = getEntry("image-info", "layer_disk_number", 1);
    layer_info.layer_disk_serial_1 = getEntry("image-info", "layer_disk_serial_1", "layer001");
    layer_info.layer_on_1 = getEntry("image-info", "layer_on_1", "N");
    layer_info.layer_x64_1 = getEntry("image-info", "layer_x64_1", "N");
    
}
int ImageInfoDB::get_ostype(string &ostype)
{
    int ret = findEntry("image-info", "ostype");

    ostype = getEntry("image-info", "ostype", "");
    LOG_DEBUG("get-image ostype:%s ret:%d", ostype.c_str(), ret);

    return ret;
}
void ImageInfoDB::set_download_status(bool downloading)
{
    LOG_DEBUG("set_download_status is %s", to_string(downloading).c_str());
    if (downloading == false) {
        setEntry("image-info", "_download_status", "false");
    } else {
        setEntry("image-info", "_download_status", "true");
    }
    saveUserDB();
}
void ImageInfoDB::set_download_basename(string name)
{
    setEntry("image-info", "name", name.c_str());
    saveUserDB();
}

void ImageInfoDB::set_download_mode(int mode)
{
    switch(mode){
    case DOWNLOAD_IMAGE_NO_OPS:
        setEntry("image-info", "download_mode", "no_ops");
        break;
    case DOWNLOAD_IMAGE_FORCE_DOWNLOAD:
        setEntry("image-info", "download_mode", "force_download");
        break;
    case DOWNLOAD_IMAGE_SILENT_DOWNLOAD:
        setEntry("image-info", "download_mode", "silent_download");
        break;
    case DOWNLOAD_IMAGE_SILENT_DOWNLOADING:
        setEntry("image-info", "download_mode", "silent_downloading");
        break;
    case DOWNLOAD_IMAGE_NEED_MERGE:
        setEntry("image-info", "download_mode", "merge");
        break;
    case DOWNLOAD_IMAGE_MERGING:
        setEntry("image-info", "download_mode", "merging");
        break;
    default:
        break;
    }
    saveUserDB();
}


    
void ImageInfoDB::set_new_base_name(string name)
{
    setEntry("image-info", "new_base_name", name.c_str());
    saveUserDB();
}
string ImageInfoDB::get_new_base_name()
{
    return getEntry("image-info", "new_base_name","");
}

void ImageInfoDB::set_new_base_version(string version)
{
    setEntry("image-info", "new_base_version", version.c_str());
    saveUserDB();
}
string ImageInfoDB::get_new_base_version()
{
    return getEntry("image-info", "new_base_version","");
}

void ImageInfoDB::set_new_base_id(int id)
{
    setEntry("image-info", "new_base_id", id);
    saveUserDB();
}
int ImageInfoDB::get_new_base_id()
{
    return getEntry("image-info", "new_base_id",0);
}

int ImageInfoDB::get_download_mode()
{
    int ret = 0;
    string download_mode;
    download_mode = getEntry("image-info", "download_mode", "no_ops");
    if (download_mode == "no_ops") {
        ret = DOWNLOAD_IMAGE_NO_OPS;
    } else if (download_mode == "force_download") {
        ret = DOWNLOAD_IMAGE_FORCE_DOWNLOAD;
    }else if (download_mode == "silent_download") {
        ret = DOWNLOAD_IMAGE_SILENT_DOWNLOAD;
    }else if (download_mode == "silent_downloading") {
        ret = DOWNLOAD_IMAGE_SILENT_DOWNLOADING;
    }else if (download_mode == "merge") {
        ret = DOWNLOAD_IMAGE_NEED_MERGE;
    }else if (download_mode == "merging") {
        ret = DOWNLOAD_IMAGE_MERGING;
    }
    return ret;
}
bool ImageInfoDB::is_silent_download(){
    string download_mode;
    download_mode = getEntry("image-info", "download_mode", "no_ops");
    if ((download_mode == "silent_download")|| (download_mode == "silent_downloading")){
       return true;
    }
    return false;
}

bool ImageInfoDB::is_need_merge(){
    string download_mode;
    download_mode = getEntry("image-info", "download_mode", "no_ops");
    if ((download_mode == "merge")|| (download_mode == "merging")){
       return true;
    }
    return false;
}
ImageInfo ImageInfoDB::get()
{
    _image_info.id = getEntry("image-info", "id", 0);
    _image_info.name = getEntry("image-info", "name", "");
    _image_info.version = getEntry("image-info", "version", "");
    _image_info.torrent_url = getEntry("image-info", "torrent_url", "");
    _image_info.md5= getEntry("image-info", "md5", "");
    _image_info.ostype = getEntry("image-info", "ostype", "");
    _image_info.real_size = getEntry("image-info", "real_size", 0);
    _image_info.virt_size = getEntry("image-info", "virt_size", 0);
    //_image_info.set_size= getEntry("image-info", "set_size", 0);
    _image_info.layer_info.layer_disk_number= getEntry("image-info", "layer_disk_number", 1);
    _image_info.layer_info.layer_disk_serial_1= getEntry("image-info", "layer_disk_serial_1", "layer001");
    _image_info.layer_info.layer_on_1= getEntry("image-info", "layer_on_1", "N");
    _image_info.layer_info.layer_x64_1= getEntry("image-info", "layer_x64_1", "N");

    LOG_INFO("_image_info:  id=%d, name=%s, version=%s, url=%s, md5=%s, ostype=%s, download_status:%d  real_size %dG, virt_size %dG, layer_disk_number %d,layer_disk_serail1 %s, layer_on_1 %s, layer_x64_1 %s",
                 _image_info.id, _image_info.name.c_str(), 
                 _image_info.version.c_str(), _image_info.torrent_url.c_str(), _image_info.md5.c_str(), _image_info.ostype.c_str(),
                 _download_status, _image_info.real_size, _image_info.virt_size, _image_info.layer_info.layer_disk_number, _image_info.layer_info.layer_disk_serial_1.c_str(),
                 _image_info.layer_info.layer_on_1.c_str(), _image_info.layer_info.layer_x64_1.c_str());
    return _image_info;
}
bool ImageInfoDB ::get_download_status()
{
    string download_status;
    download_status = getEntry("image-info", "_download_status", "false");
    if (download_status == "false") {
        _download_status = false;
    } else {
        _download_status = true;
    }

    return _download_status;
}

int ImageInfoDB::delete_image_info() {

    deleteEntry("image-info", "id");
    deleteEntry("image-info", "name");
    deleteEntry("image-info", "version");
    deleteEntry("image-info", "torrent_url");
    deleteEntry("image-info", "md5");
    deleteEntry("image-info", "ostype");
    deleteEntry("image-info", "real_size");
    deleteEntry("image-info", "virt_size");
    deleteEntry("image-info", "_download_status");
    deleteEntry("image-info", "layer_disk_number");
    deleteEntry("image-info", "layer_disk_serial_l");
    deleteEntry("image-info", "layer_on_1");
    deleteEntry("image-info", "layer_x64_1");


    if(remove(USER_DISPLAY_INFO_INI) < 0) {
        LOG_PERROR("the file is not exits, delete error\n");
        return -1;
    };
    rc_system("sync");

    return 0;
}

void VMConfigDB::set_vm_desktop_redir(const string &redir_switch)
{
    if (redir_switch == "Y" || redir_switch == "N") {
        setEntry("desktop_info", "desktop_redir", redir_switch.c_str());
    } else {
        LOG_ERR("set_vm_desktop_redir err! info: %s", redir_switch.c_str());
    }
}

void VMConfigDB::get_vm_desktop_redir(string &redir_switch)
{
    redir_switch = getEntry("desktop_info", "desktop_redir", "Y");
}


int OtherConfigDB::get_boot_speedup()
{
	//int _boot_speedup;
	//Application *app = Application::get_application();
	//app->get_vm()->vm_get_vm_otherSetting(_boot_speedup);

	string switch_str = getEntry("bootSpeed", "switch", "1");
	if(switch_str == "1")
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void OtherConfigDB::set_boot_speedup(const bool &_boot_speedup)
{
	//int _boot_speedup;
	//Application *app = Application::get_application();
	//app->get_vm()->vm_set_vm_otherSetting(_boot_speedup);
	if(_boot_speedup)
	{
		setEntry("bootSpeedup", "switch", "1");
	}
	else
	{
		setEntry("bootSpeedup", "switch", "0");
	}
}


void OtherConfigDB::set_e1000_netcard(const bool &netcard_switch)
{
    if (netcard_switch) {
        setEntry("e1000_nic_info", "switch", "ON");
    } else {
        setEntry("e1000_nic_info", "switch", "OFF");
    }
}

bool OtherConfigDB::is_using_e1000_netcard(void)
{
    string switch_str = getEntry("e1000_nic_info", "switch", "OFF");
    return (switch_str == "ON");
}

void OtherConfigDB::set_usb_emulation(const bool &is_emulation)
{
    if (is_emulation) {
        setEntry("usb_emulation", "switch", "ON");
    } else {
        setEntry("usb_emulation", "switch", "OFF");
    }
}

bool OtherConfigDB::is_usb_emulation(void)
{
    string switch_str = getEntry("usb_emulation", "switch", "OFF");
    return (switch_str == "ON");
}

void OtherConfigDB::set_app_layer_switch(const int &status)
{
    if (status == 1) {
        setEntry("app_layer", "switch", "ON");
    } else if (status == 0) {
        setEntry("app_layer", "switch", "OFF");
    }
}

int OtherConfigDB::get_app_layer_switch()
{
    LayerDiskInfo layer_info;
    Application *app = Application::get_application();

    app->get_vm()->vm_get_vm_layerdisk_info(layer_info);
    if (layer_info.layer_on_1 == "N") {
        return -1;
    }

    string switch_str = getEntry("app_layer", "switch", "ON");
    if (switch_str == "ON") {
        return 1;
    } else {
        return 0;
    }
}

/* key for current setting: wan ip */
#define K_IDVOWAN_WAN_IP "WAN_IP"

#define K_IDVOWAN_PRIVATE_IP "PRIVATE_IP"

#define K_IDVOWAN_WAN_PORT "WAN_PORT"

#define K_IDVOWAN_PRIVATE_PORT "PRIVATE_PORT"

#define K_IDVOWAN_BT_PORTS_RANGE "BT_PORTS_RNAGE"


/* values */
#define V_IDVOWAN_BT_PORTS_RANGE_DEFAULT "6881:7180"

#define V_IDVOWAN_WAN_IP ""

#define V_IDVOWAN_PRIVATE_IP ""

#define V_IDVOWAN_WAN_PORT 80

#define V_IDVOWAN_PRIVATE_PORT 80


/* key for last time setting: wan ip */
#define K_IDVOWAN_WAN_IP_L "WAN_IP_L"

/* key for last time setting: wan ip */
#define K_IDVOWAN_PRIVATE_IP_L "PRIVATE_IP_L"

/* key for last time setting: wan port */
#define K_IDVOWAN_WAN_PORT_L "WAN_PORT_L"

/* key for last time setting: private port, or LAN port */
#define K_IDVOWAN_PRIVATE_PORT_L "PRIVATE_PORT_L"

/* key for last time setting: bt ports range */
#define K_IDVOWAN_BT_PORTS_RANGE_L "BT_PORTS_RNAGE_L"

/* section */
#define S_IDVOWAN "default"


void HttpPortMapDB::set_http_port_map(const HttpPortInfo &info)
{
    setEntry(S_IDVOWAN, K_IDVOWAN_WAN_IP, info.public_ip.c_str());
    setEntry(S_IDVOWAN, K_IDVOWAN_WAN_PORT, info.public_port);
    setEntry(S_IDVOWAN, K_IDVOWAN_PRIVATE_IP, info.private_ip.c_str());
    setEntry(S_IDVOWAN, K_IDVOWAN_PRIVATE_PORT, info.private_port);
    setEntry(S_IDVOWAN, K_IDVOWAN_BT_PORTS_RANGE, info.btPortsRange.c_str());
}

void HttpPortMapDB::get_http_port_map(HttpPortInfo &info)
{
    info.public_ip = getEntry(S_IDVOWAN, K_IDVOWAN_WAN_IP, V_IDVOWAN_WAN_IP);
    info.public_port = getEntry(S_IDVOWAN, K_IDVOWAN_WAN_PORT, V_IDVOWAN_WAN_PORT);
    info.private_ip = getEntry(S_IDVOWAN, K_IDVOWAN_PRIVATE_IP, V_IDVOWAN_PRIVATE_IP);
    info.private_port = getEntry(S_IDVOWAN, K_IDVOWAN_PRIVATE_PORT, V_IDVOWAN_PRIVATE_PORT);
    info.btPortsRange = getEntry(S_IDVOWAN, K_IDVOWAN_BT_PORTS_RANGE,
       V_IDVOWAN_BT_PORTS_RANGE_DEFAULT);
}
 
 
void HttpPortMapDB::set_http_last_port_map(const HttpPortInfo &info)
{
    setEntry(S_IDVOWAN, K_IDVOWAN_WAN_IP_L, info.public_ip.c_str());
    setEntry(S_IDVOWAN, K_IDVOWAN_WAN_PORT_L, info.public_port);
    setEntry(S_IDVOWAN, K_IDVOWAN_PRIVATE_IP_L, info.private_ip.c_str());
    setEntry(S_IDVOWAN, K_IDVOWAN_PRIVATE_PORT_L, info.private_port);
    setEntry(S_IDVOWAN, K_IDVOWAN_BT_PORTS_RANGE_L, info.btPortsRange.c_str());
}

void HttpPortMapDB::get_http_last_port_map(HttpPortInfo &info)
{
    info.public_ip = getEntry(S_IDVOWAN, K_IDVOWAN_WAN_IP_L, V_IDVOWAN_WAN_IP);
    info.public_port = getEntry(S_IDVOWAN, K_IDVOWAN_WAN_PORT_L, V_IDVOWAN_WAN_PORT);
    info.private_ip = getEntry(S_IDVOWAN, K_IDVOWAN_PRIVATE_IP_L, V_IDVOWAN_PRIVATE_IP);
    info.private_port = getEntry(S_IDVOWAN, K_IDVOWAN_PRIVATE_PORT_L, V_IDVOWAN_PRIVATE_PORT);
    info.btPortsRange = getEntry(S_IDVOWAN, K_IDVOWAN_BT_PORTS_RANGE_L,
        V_IDVOWAN_BT_PORTS_RANGE_DEFAULT);
}
 

