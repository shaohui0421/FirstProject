#include "newdeploy_manage.h"
#include "ui_api.h"

NewDeployManage::NewDeployManage(Application *app)
    :_app(app)
    ,_newdeploy_status_machine(this)
{
    
}

bool NewDeployManage::is_new_terminal()
{
    return _newdeploy_data.get_new_terminal();
}

void NewDeployManage::set_new_terminal(bool new_terminal)
{
    if(new_terminal == false)
    {
        _app->_mina->mina_web_upload_new_terminal_complete();
    }
    _newdeploy_data.set_new_terminal(new_terminal);
}

bool NewDeployManage::get_download_status()
{
    return _newdeploy_data.get_download_status();
}

void NewDeployManage::set_download_status(bool download_status)
{
    _newdeploy_data.set_download_status(download_status);
}

void NewDeployManage::start_new_deploy()
{
    LOG_DEBUG("======================start new deploy=====================");
    //ASSERT(get_newdeploy_status() == STATUS_NEWDEPLOY_NONE);
    _newdeploy_status_machine.reset_status();
    if(_app->is_dev_network_up() == false )
    {
        _newdeploy_status_machine.change_status(EVENT_NEWDEPLOY_CONNECT_FAIL);
    }
    else
    {
        _newdeploy_status_machine.change_status(EVENT_NEWDEPLOY_SET_IP);
    }
}

void NewDeployManage::enter_set_type()
{
    _newdeploy_status_machine.change_status(EVENT_NEWDEPLOY_SET_TYPE);
}

void NewDeployManage::enter_download_image()
{
    _newdeploy_status_machine.change_status(EVENT_NEWDEPLOY_DOWNLOAD_IMAGE);
}

void NewDeployManage::enter_newdeploy_finish()
{
    _newdeploy_status_machine.change_status(EVENT_NEWDEPLOY_SUCCESS);
}

void NewDeployManage::quit_new_deploy()
{
    LOG_DEBUG("======================quit new deploy=====================");
    set_new_terminal(false);
    _app->logic_reset_status_machine();
}

void NewDeployManage::quit_new_deploy_abnormal()
{
    LOG_DEBUG("==================quit new deploy abnormal================="); 
    _app->logic_reset_status_machine();
}

void NewDeployManage::reset_status()
{
    LOG_DEBUG("==================reset new deploy status=================");
    _newdeploy_status_machine.reset_status();
}

int NewDeployManage::get_newdeploy_status()
{
    return _newdeploy_status_machine.get_status();
}

bool NewDeployManage::is_ui_locking()
{
    return _app->_ui_locking;
}
/*
// TODO: zjj
static int t1 = 10;
static char t2[20] = "test 2";

static void test1(void* args) {
    LOG_INFO("test1 %d", *((int*)args));
}

static void test2(void* args) {
    LOG_INFO("test2 %s", (char*)args);
}
*/
void NewDeployManage::on_entering_connect_fail()
{
    LOG_INFO("######NEWDEPLOY_STATUS_MACHINE###### connect fail");
    ASSERT(_app->_status_machine.get_status() == STATUS_NEW_DEPLOY);
    
    if(!is_ui_locking())
    {
        LOG_DEBUG("call l2u_show_newdeploy_connect: fail");
        l2u_show_newdeploy_connect(0);
    }
    //l2u_show_newdeploy_finish();
    //l2u_show_dialog_connect_network(UI_TIPS_CONNECTING);
    //l2u_show_dialog_config_ip();
    //l2u_show_dialog_copy_base((void*)test1, (void*)&t1, (void*)test2, (void*)&t2);
    //l2u_show_dialog_copy_base_fail((void*)test1, (void*)&t1);
    //l2u_show_wifi_auth(1);
}

void NewDeployManage::show_newdeploy_disconnect()
{
    LOG_DEBUG("call l2u_show_newdeploy_connect");
    l2u_show_newdeploy_connect(1);
}

void NewDeployManage::on_entering_set_ip()
{
    LOG_INFO("######NEWDEPLOY_STATUS_MACHINE###### set ip");
    ASSERT(_app->_status_machine.get_status() == STATUS_NEW_DEPLOY);

    string server_ip = _app->_mina->mina_get_server_ip();    
    if(!server_ip.empty())
    {
        if (!is_ui_locking())
        {
            LOG_DEBUG("call l2u_show_connecting_wrap()");
            _app->l2u_show_connecting_wrap();
        }
        _app->_mina->mina_establish_connection();
    }
    else
    {
        if(!is_ui_locking())
        {
            LOG_DEBUG("call show_ui_disconnect()");
            show_newdeploy_disconnect();
        }
    }
    
}

void NewDeployManage::on_entering_set_type()
{
    LOG_INFO("######NEWDEPLOY_STATUS_MACHINE###### set type");
    ASSERT(_app->_status_machine.get_status() == STATUS_NEW_DEPLOY);
    //TODO: add a dialog tip?
    _app->_vm->vm_clear_teacher_disk();
    _app->_vm->vm_clear_inst();
    _app->_vm->vm_clear_layer();
    if(is_ui_locking() == false)
    {
        string bind_username = _app->_mode_data.get().bind_user.username;
        LOG_DEBUG("call l2u_show_settype name=%s, size=%d", bind_username.c_str(), bind_username.size());
	    l2u_show_settype(bind_username.c_str(), bind_username.size());
    }
}

void NewDeployManage::remove_one_unnecessary_golden_image(string exclude) {

    if (_app->_vm != NULL) {
        LOG_INFO("Trying to remove unnecessary golden image exclude %s.\n", exclude.c_str());
        _app->_vm->vm_remove_one_unnecessary_golden_image(exclude);
    }
}

void NewDeployManage::on_entering_downloading_image()
{
    LOG_INFO("######NEWDEPLOY_STATUS_MACHINE###### download image");
    ASSERT(_app->_status_machine.get_status() == STATUS_NEW_DEPLOY);
    set_download_status(true);
    _app->_ui_locking = false;
    _app->l2u_download_info.status = UI_DIALOG_DOWNLOAD_ST_INITING;
    _app->l2u_download_info.speed = NULL;
    _app->l2u_download_info.process = 0.0;
    l2u_show_image_initing(&_app->l2u_download_info);
    //l2u_show_download(&_app->l2u_download_info);
    remove_one_unnecessary_golden_image(_app->_server_image_info.name);
    _app->_vm->vm_start_download_image(&_app->_server_image_info);
}

void NewDeployManage::on_entering_newdeploy_finish()
{
    LOG_INFO("######NEWDEPLOY_STATUS_MACHINE###### newdeploy finish");
    ASSERT(_app->_status_machine.get_status() == STATUS_NEW_DEPLOY);
    set_download_status(false);
    if(!is_ui_locking())
    {
        //l2u_ctrl_winbtn(false);
        LOG_DEBUG("call l2u_show_newdeploy_finish");
        l2u_show_newdeploy_finish();
        //hold 3s, then quit new deploy
        sleep(3);
    }
    quit_new_deploy();
}

NewDeployStatusMachine::NewDeployStatusMachine(NewDeployManage* manage)
    :StatusMachine<NewDeployManage> (manage)
{
    _status = STATUS_NEWDEPLOY_NONE;
    set_handler(STATUS_NEWDEPLOY_CONNECT_FAIL,         &NewDeployManage::on_entering_connect_fail);
    set_handler(STATUS_NEWDEPLOY_SET_IP,               &NewDeployManage::on_entering_set_ip);
    set_handler(STATUS_NEWDEPLOY_SET_TYPE,             &NewDeployManage::on_entering_set_type);
    set_handler(STATUS_NEWDEPLOY_DOWNLOADING_IMAGE,    &NewDeployManage::on_entering_downloading_image);
    set_handler(STATUS_NEWDEPLOY_FINISH,               &NewDeployManage::on_entering_newdeploy_finish);
}

bool NewDeployStatusMachine::check_valid_event_type(int type)
{
    return true;
}

int NewDeployStatusMachine::get_next_status(int event_type)
{
    int next_status = 0;

    switch (_status) {
    case STATUS_NEWDEPLOY_NONE:
        next_status = get_initing_next_status(event_type);
        break;
    case STATUS_NEWDEPLOY_CONNECT_FAIL:
        next_status = get_connect_fail_next_status(event_type);
        break;
    case STATUS_NEWDEPLOY_SET_IP:
        next_status = get_set_ip_next_status(event_type);
        break;
    case STATUS_NEWDEPLOY_SET_TYPE:
        next_status = get_set_type_next_status(event_type);
        break;
    case STATUS_NEWDEPLOY_DOWNLOADING_IMAGE:
        next_status = get_downloading_image_next_status(event_type);
        break;
    case STATUS_NEWDEPLOY_FINISH:
        next_status = _status;  //STATUS_NEWDEPLOY_FINISH is a final state.
        break;
    default:
        ASSERT(0);
        break;
    }
    
    LOG_INFO("event_type: %d, cur_newdeploy_status: %d, next_newdeploy_status: %d", event_type, _status, next_status);
    return next_status;
}

int NewDeployStatusMachine::get_initing_next_status(int event_type)
{
    switch(event_type)
    {
    case EVENT_NEWDEPLOY_CONNECT_FAIL:
        return STATUS_NEWDEPLOY_CONNECT_FAIL;
    case EVENT_NEWDEPLOY_SET_IP:
        return STATUS_NEWDEPLOY_SET_IP;
    default:
        return _status;
    }
}

int NewDeployStatusMachine::get_connect_fail_next_status(int event_type)
{
    switch(event_type)
    {
    case EVENT_NEWDEPLOY_SET_TYPE:
        return STATUS_NEWDEPLOY_SET_TYPE;
    default:
        return _status;
    }
}

int NewDeployStatusMachine::get_set_ip_next_status(int event_type)
{
    switch(event_type)
    {
    case EVENT_NEWDEPLOY_SET_TYPE:
        return STATUS_NEWDEPLOY_SET_TYPE;
    case EVENT_NEWDEPLOY_DOWNLOAD_IMAGE:
        return STATUS_NEWDEPLOY_DOWNLOADING_IMAGE;
    default:
        return _status;
    }
}

int NewDeployStatusMachine::get_set_type_next_status(int event_type)
{
    switch(event_type)
    {
    case EVENT_NEWDEPLOY_DOWNLOAD_IMAGE:
        return STATUS_NEWDEPLOY_DOWNLOADING_IMAGE;
    case EVENT_NEWDEPLOY_SUCCESS:
        return STATUS_NEWDEPLOY_FINISH;
    default:
        return _status;
    }
}

int NewDeployStatusMachine::get_downloading_image_next_status(int event_type)
{
    switch(event_type)
    {
    case EVENT_NEWDEPLOY_SUCCESS:
        return STATUS_NEWDEPLOY_FINISH;
    default:
        return _status;
    }
}

void NewDeployStatusMachine::reset_status()
{
    _status = STATUS_NEWDEPLOY_NONE;
}

