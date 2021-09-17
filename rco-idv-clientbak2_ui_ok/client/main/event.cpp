#include "event.h"

EventsQueue::EventsQueue()
    : _events_gen (0)
{
}

EventsQueue::~EventsQueue()
{
    clear_queue();
}

void EventsQueue::clear_queue()
{
    Lock lock(_events_lock);
    while (!_events.empty()) {
        Event* event = _events.front();
        _events.pop_front();
        event->unref();
    }
}

int EventsQueue::push_event(Event* event)
{
    int ret = 0;
    Lock lock(_events_lock);
    ret = event->pre_push();
    if(ret >= 0)
    {
        _events.push_back(event);
        event->set_generation(_events_gen);
        event->ref();
        return _events.size();
    }
    else
    {
        return ret;
    }
}

void EventsQueue::erase_event(int event_id)
{
    std::list<Event*>::iterator iter;
    Lock lock(_events_lock);
    for (iter = _events.begin(); iter != _events.end();)
    {
        Event* event = *iter;
        if (event->erase_check(event_id))
        {
            _events.erase(iter++);
            event->unref();
        }
        else
        {
            iter++;
        }
    }
}

void EventsQueue::process_events()
{
    _events_gen++;

    for (;;) {
        Event* event;
        Lock lock(_events_lock);
        if (_events.empty()) {
            return;
        }
        event = _events.front();
        if (event->get_generation() == _events_gen) {
            return;
        }
        _events.pop_front();

        lock.unlock();
        if(event->pre_response() >= 0)
        {
            event->response();
        }
        event->unref();
    }
}

bool EventsQueue::is_empty()
{
    Lock lock(_events_lock);
    return _events.empty();
}