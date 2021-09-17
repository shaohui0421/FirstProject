#ifndef _NEWDEPLOY_MANAGE_H
#define _NEWDEPLOY_MANAGE_H

#include "common.h"
#include "application.h"

enum NewDeployLogicStateType
{
    STATUS_NEWDEPLOY_NONE = 0,
    STATUS_NEWDEPLOY_CONNECT_FAIL,
	STATUS_NEWDEPLOY_SET_IP,
    STATUS_NEWDEPLOY_SET_TYPE,
    STATUS_NEWDEPLOY_DOWNLOADING_IMAGE,
    STATUS_NEWDEPLOY_FINISH,
};

enum NewDeployLogicMachineEventType
{
    EVENT_NEWDEPLOY_CONNECT_FAIL = 1,
    EVENT_NEWDEPLOY_SET_IP,	
    EVENT_NEWDEPLOY_SET_TYPE,
    EVENT_NEWDEPLOY_DOWNLOAD_IMAGE,
    EVENT_NEWDEPLOY_SUCCESS,
};

class NewDeployManage;

class NewDeployStatusMachine: public StatusMachine<NewDeployManage>
{
public:
    NewDeployStatusMachine(NewDeployManage* manage);
    virtual ~NewDeployStatusMachine() {}
    virtual bool check_valid_event_type(int type);
	void reset_status();
protected:
    virtual int get_next_status(int event_type);
private:
    int get_initing_next_status(int event_type);
    int get_connect_fail_next_status(int event_type);
    int get_set_ip_next_status(int event_type);
    int get_set_type_next_status(int event_type);
    int get_downloading_image_next_status(int event_type);
};

class NewDeployManage
{
public:
    NewDeployManage(Application *app);
    virtual ~NewDeployManage() {}
	bool is_new_terminal();
	void set_new_terminal(bool new_terminal);
	bool get_download_status();
	void set_download_status(bool download_status);
	void start_new_deploy();
    void enter_set_type();
    void enter_download_image();
    void enter_newdeploy_finish();
    void quit_new_deploy();
    void quit_new_deploy_abnormal();
    void reset_status();
	int get_newdeploy_status();
    bool is_ui_locking();
    void show_newdeploy_disconnect();

private:
    void on_entering_connect_fail();
    void on_entering_set_ip();
    void on_entering_set_type();
    void remove_one_unnecessary_golden_image(string exclude);
    void on_entering_downloading_image();
    void on_entering_newdeploy_finish();
	
	Application *_app;
    NewDeployCtrlInfoDB _newdeploy_data;
    NewDeployStatusMachine _newdeploy_status_machine;

    
    friend class NewDeployStatusMachine;
};

#endif
