#ifndef _TIMER_H
#define _TIMER_H

#include "common.h"
#include "thread.h"
#include <set>

class Timer;
class TimersQueue;

class Timer
{
public:
    Timer() : _refs (1), _is_armed (false) {}
    virtual void response() = 0;
    bool is_armed() {return _is_armed;}
    void ref() { ++_refs;}
    void unref() {if (--_refs == 0) delete this;}

protected:
    virtual ~Timer() {}

private:
    
    void arm(unsigned int msec);
    void disarm();
    unsigned long long get_expiration() const { return _expiration;}
    void calc_next_expiration_time() { _expiration += _interval;}
    void calc_next_expiration_time(unsigned long long now);
    static unsigned long long get_now();
        
    int _refs;
    bool _is_armed;
    unsigned int _interval;
    unsigned long long _expiration;

    class Compare {
    public:
        bool operator () (const Timer* timer1, const Timer* timer2) const
        {
            if (timer1->get_expiration() < timer2->get_expiration()) {
                return true;
            } else if (timer1->get_expiration() > timer2->get_expiration()) {
                return false;
            } else { // elements must be unique (for insertion into set)
                return timer1 < timer2;
            }
        }
    };

    friend class TimersQueue;
};

class TimersQueue {
public:
    TimersQueue();
    virtual ~TimersQueue();

    void activate_interval_timer(Timer* timer, unsigned int millisec);
    void deactivate_interval_timer(Timer* timer);

    unsigned int get_soonest_timeout();
    void timers_action();

private:
    void clear_queue();

private:
    typedef std::set<Timer*, Timer::Compare> TimersSet;
    typedef TimersSet::iterator itTimersSet;
    TimersSet _armed_timers;
    RecurciveMutex _timers_lock;
};


#endif //_TIMER_H
