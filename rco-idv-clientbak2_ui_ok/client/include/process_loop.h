#ifndef _PROCESS_LOOP_H
#define _PROCESS_LOOP_H

#include "event.h"
#include "event_source.h"
#include "timer.h"

class ProcessLoop
{
public:
    ProcessLoop(void* owner);
    virtual ~ProcessLoop();
    int run();

    /* Event sources to track. Note: the following methods are not thread safe, thus,
       they mustn't be called from other thread than the process loop thread. */
    void add_eventsource(EventSource & trigger);
    void remove_eventsource(EventSource & trigger);

    /* events queue */
    int push_event(Event* event);
    void erase_event(int event_id);
    void clear_event_queue();

    /* timer */
    void activate_interval_timer(Timer* timer, unsigned int millisec);
    void deactivate_interval_timer(Timer* timer);

    void* get_owner() { return _owner;}

    bool is_same_thread(pthread_t thread) { return _started && pthread_equal(_thread, thread);}

private:
    class WakeupTrigger: public Trigger {
    public:
        virtual void on_action() {}
    };
    void wakeup();
    void process_events_queue();
    
    EventSources _event_sources;
    EventsQueue _events_queue;
    TimersQueue _timers_queue;

    WakeupTrigger _wakeup_trigger;

    void* _owner;

    bool _quitting;
    int _exit_code;
    bool _started;
    pthread_t _thread;
};

#endif //_PROCESS_LOOP_H