#ifndef _EVENT_SOURCE_H
#define _EVENT_SOURCE_H

#include "common.h"
#include "thread.h"
#include <vector>

class EventSource;
class Trigger;
class EventSources;


class EventSource 
{
public:
    EventSource(){}
    virtual ~EventSource(){}
    virtual void action()  = 0;
    virtual int get_fd(){return _fd;}
protected:
    int _fd;
};

class Trigger: public EventSource
{
public:
    Trigger();
    virtual ~Trigger();
    virtual void trigger();
    virtual void action();
    virtual void on_action() = 0;

protected:
    virtual bool reset_event();
    int _eventsource_write_fd;
    bool _pending_int;
    Mutex _mutex;
};

class EventSources
{
//this class is not thread safe
public:
    EventSources(){}
    virtual ~EventSources(){}
    void add_eventsource(int fd, EventSource* source);
    void remove_eventsource(EventSource* source);
    /* return true if the events loop should quit */
    bool wait_eventsources(int timeout_ms = INFINITE);

private:
    std::vector<EventSource*> _sources;
    std::vector<int> _fds;
};



#endif //_EVENT_SOURCE_H

