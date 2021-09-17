#ifndef _EVENT_H
#define _EVENT_H

#include "common.h"
#include "thread.h"
#include <list>
class Event;
class SyncEvent;
class EventsQueue;

class Event
{
friend class EventsQueue;
public:
    Event():_refs(1){}
    
    //judge whether the event queue accept pushing this event
    virtual int pre_push() {return 0;}

    //judge whether the event queue accept handling this event
    virtual int pre_response() {return 0;}

    //judge whether the event queue accept erasing this event
    virtual bool erase_check(int event_id) {return false;}

    virtual void response() = 0;
    void ref(){++_refs;}
    void unref() {if (--_refs == 0) delete this;}
    void set_generation(unsigned int gen) { _generation = gen;}
    unsigned int get_generation() { return _generation;}
protected:
    virtual ~Event(){}
    unsigned int _generation;
    int _refs;
};

class SyncEvent:public Event
{
public:
    SyncEvent(): _ready (false){}
    void wait()
    {
        Lock lock(_mutex);
        while (!_ready) 
        {
            _condition.wait(lock);
        }
    }

    bool wait_timeout(unsigned long long nano)
    {
        Lock lock(_mutex);
        if (!_ready)
        {
            _condition.timed_wait(lock, nano);
        }

        if (_ready) {
        	return false;
        }
        return true;
    }

    virtual void response()
    {
        do_response();
        Lock lock(_mutex);
        _ready = true;
        _condition.notify_one();
    }
    virtual void do_response() = 0;

protected:
    virtual ~SyncEvent(){}

private:
    Mutex _mutex;
    Condition _condition;
    bool _ready;
};

class EventsQueue
{
public:
    EventsQueue();
    virtual ~EventsQueue();
    /* return the size of the queue (post-push) */
    int push_event(Event* event);
    void erase_event(int event_id);
    void process_events();
    bool is_empty();
    void clear_queue();

private:
    std::list<Event*> _events;
    Mutex _events_lock;
    unsigned int _events_gen;
};

#endif //_EVENT_H
