#include "timer.h"

void Timer::arm(unsigned int msec)
{
    _interval = msec;
    _expiration = get_now();
    calc_next_expiration_time();
    _is_armed = true;
}

void Timer::disarm()
{
    _is_armed = false;
}

void Timer::calc_next_expiration_time(unsigned long long now)
{
    calc_next_expiration_time();
    if (_expiration <= now) {
        _expiration = now;
        calc_next_expiration_time();
    }
}

unsigned long long Timer::get_now()
{
    return (get_monolithic_time() / 1000 / 1000);
}



TimersQueue::TimersQueue()
{
}

TimersQueue::~TimersQueue()
{
    clear_queue();
}

void TimersQueue::clear_queue()
{
    Lock lock(_timers_lock);
    itTimersSet iter;
    for (iter = _armed_timers.begin(); iter != _armed_timers.end(); iter++) 
    {
        (*iter)->disarm();
    }
    _armed_timers.clear();
}

void TimersQueue::activate_interval_timer(Timer* timer, unsigned int millisec)
{
    Lock lock(_timers_lock);
    timer->ref();
    deactivate_interval_timer(timer);
    timer->arm(millisec);
    _armed_timers.insert(timer);
}

void TimersQueue::deactivate_interval_timer(Timer* timer)
{
    Lock lock(_timers_lock);
    if (timer->is_armed()) 
    {
        _armed_timers.erase(timer);
        timer->disarm();
        timer->unref();
    }
}

unsigned int TimersQueue::get_soonest_timeout()
{
    Lock lock(_timers_lock);
	itTimersSet iter;
	unsigned long long now;
    unsigned long long next_time;

    iter = _armed_timers.begin();
    if (iter == _armed_timers.end()) 
    {
        return INFINITE;
    }
	now = Timer::get_now();
	next_time = (*iter)->get_expiration();

    if (next_time <= now) 
    {
        return 0;
    }
    return (unsigned int)(next_time - now);
}


void TimersQueue::timers_action()
{
    Lock lock(_timers_lock);
    unsigned long long now = Timer::get_now();
    itTimersSet iter;
    Timer* timer;

    while (((iter = _armed_timers.begin()) != _armed_timers.end()) &&
           ((*iter)->get_expiration() <= now)) 
    {
        timer = *iter;
        _armed_timers.erase(iter);
        timer->calc_next_expiration_time(now);
        _armed_timers.insert(timer);
        timer->response();
    }
}

