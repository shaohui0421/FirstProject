#ifndef _STATUS_MACHINE_H
#define _STATUS_MACHINE_H
#include "common.h"
#include <map>
#include <set>

typedef std::set<int> ValidEvents;
typedef std::map<int, ValidEvents> StatusValidEvents;
typedef ValidEvents::iterator itValidEvents;

template <class HandlerClass>

class StatusMachine
{
    typedef void (HandlerClass::*Handler)();
    typedef std::map<int, Handler> Handlers;
public:
    StatusMachine(HandlerClass* obj) :_obj (obj) {}

    void change_status(int event_type)
    {
        int next_status = get_next_status(event_type);
        if(_status != next_status)
        {
            ASSERT(_action_handlers.count(next_status) > 0);
        	LOG_INFO("cur state=%d, next state=%d", _status, next_status);
            _status = next_status;
            (_obj->*_action_handlers[_status])();
        }
    }

    int get_status(){return _status;}    

    virtual bool check_valid_event_type(int type) = 0;
protected:
    
    void set_handler(int id, Handler action_handle)
    {
        _action_handlers[id] = action_handle;
    }

    /*this function shoul be called after set_handler*/
    void append_valid_event_type(int id, int type)
    {
        //ASSERT(_action_handlers.count(id) > 0)
        _status_valid_events[id].insert(type);
    }

    virtual int get_next_status(int event_type) = 0;
    
    int _status;
    HandlerClass* _obj;
    Handlers _action_handlers;
    StatusValidEvents _status_valid_events;
};



#endif//_STATUS_MACHINE_H
