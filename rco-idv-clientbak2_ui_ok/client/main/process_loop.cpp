#include "process_loop.h"

ProcessLoop::ProcessLoop(void* owner)
    : _owner (owner)
    , _quitting (false)
    , _exit_code (0)
    , _started (false)
{
    _event_sources.add_eventsource(_wakeup_trigger.get_fd(), &_wakeup_trigger);
}

ProcessLoop::~ProcessLoop()
{
    _event_sources.remove_eventsource(&_wakeup_trigger);
}

int ProcessLoop::run()
{
    _thread = pthread_self();
    _started = true;
    while(1)
    {
        if (_event_sources.wait_eventsources(_timers_queue.get_soonest_timeout())) 
        {
            _quitting = true;
            break;
        }
        _timers_queue.timers_action();
        process_events_queue();
        if (_quitting)
        {
            break;
        }
    }

    return _exit_code;
}

void ProcessLoop::process_events_queue()
{
    ASSERT(!_started || pthread_equal(pthread_self(), _thread));
    _events_queue.process_events();
    if (!_events_queue.is_empty()) 
    {
        wakeup();
    }
}

void ProcessLoop::wakeup()
{
    _wakeup_trigger.trigger();
}

void ProcessLoop::add_eventsource(EventSource& eventsource)
{
    ASSERT(!_started || pthread_equal(pthread_self(), _thread));
    _event_sources.add_eventsource(eventsource.get_fd(), &eventsource);
}

void ProcessLoop::remove_eventsource(EventSource& eventsource)
{
    ASSERT(!_started || pthread_equal(pthread_self(), _thread));
    _event_sources.remove_eventsource(&eventsource);
}


int ProcessLoop::push_event(Event* event)
{
    int ret = _events_queue.push_event(event);
    if (ret == 1) 
    { // queue was empty before the push
        wakeup();
    }
    return ret;
}

void ProcessLoop::erase_event(int event_id)
{
    _events_queue.erase_event(event_id);
}

void ProcessLoop::clear_event_queue()
{
    _events_queue.clear_queue();
}

void ProcessLoop::activate_interval_timer(Timer* timer, unsigned int millisec)
{
    _timers_queue.activate_interval_timer(timer, millisec);

    if (_started && !pthread_equal(pthread_self(), _thread))
    {
        wakeup();
    }
}

void ProcessLoop::deactivate_interval_timer(Timer* timer)
{
    _timers_queue.deactivate_interval_timer(timer);
}
