#include "event_source.h"

Trigger::Trigger()
	:_pending_int(false)
{
    int fd[2];
    if (pipe(fd) == -1) {
        LOG_WARNING("pipe create error errno = ", errno);
    }
    _fd = fd[0];
    _eventsource_write_fd = fd[1];
    set_non_blocking(_fd);
}

Trigger::~Trigger()
{
    close(_fd);
    close(_eventsource_write_fd);
}

void Trigger::trigger()
{
    const int val = 1;
    Lock lock(_mutex);
    if (_pending_int) {
        return;
    }
    _pending_int = true;
    if (write(_eventsource_write_fd, &val, sizeof(val)) != sizeof(val)) {
        LOG_WARNING("write eventsource failed");
    }
}

bool Trigger::reset_event()
{
    int val;
    Lock lock(_mutex);
    if (!_pending_int) {
        return false;
    }
    if (::read(_fd, &val, sizeof(val)) != sizeof(val)) {
        LOG_WARNING("eventsource read error");
    }
    _pending_int = false;
    return true;
}

void Trigger::action()
{
    if (reset_event())
    {
        on_action();
    }
}

void EventSources::add_eventsource(int fd, EventSource* source)
{
    int size = _sources.size();
    _sources.resize(size + 1);
    _fds.resize(size + 1);
    _sources[size] = source;
    _fds[size] = fd;
}

void EventSources::remove_eventsource(EventSource* source)
{
    int size = _sources.size();
	int i = 0;
    for (i = 0; i < size; i++) 
    {
        if (_sources[i] == source) 
        {
            for (i++; i < size; i++) 
            {
                _sources[i - 1] = _sources[i];
                _fds[i - 1] = _fds[i];
            }
            _sources.resize(size - 1);
            _fds.resize(size - 1);
            return;
        }
    }
    LOG_WARNING("event not found");
}

bool EventSources::wait_eventsources(int timeout_msec)
{
    int maxfd = 0;
    fd_set rfds;
    struct timeval tv;
    struct timeval *tvp;
    int ready;

    FD_ZERO(&rfds);

    int size = _sources.size();
    for (int i = 0; i < size; i++)
    {
        maxfd = MAX(maxfd, _fds[i]);
        FD_SET(_fds[i], &rfds);
    }

    if (timeout_msec == INFINITE) 
    {
        tvp = NULL;
    } 
    else if(timeout_msec < 0)
    {
        //WARNNING:we get unsigned int in get_soonest_timeout, so we might get a negative value
        LOG_ALERT("timeout_msec get a negative value in get_soonest_timeout!!!!!!!!, timeout_msec = %d", timeout_msec);
        tvp = NULL;
    }
    else 
    {
        tv.tv_sec = timeout_msec / 1000;
        tv.tv_usec = (timeout_msec % 1000) * 1000;
        tvp = &tv;
    }

    /* Right now we only use read polling*/
    ready = select(maxfd+1, &rfds, NULL, NULL, tvp);

    if (ready == -1) 
    {
        if (errno == EINTR) 
        {
            return false;
        }
        LOG_WARNING("wait error select failed");
    } 
    else if (ready == 0)
    {
        return false;
    }
    else
    {
        for (unsigned int i = 0; i < _sources.size(); i++) 
        {
            if (FD_ISSET(_fds[i], &rfds)) 
            {
                _sources[i]->action();
                /* The action may have removed / added event sources changing
                   our array, so leave the loop and handle other events the next
                   time we are called */
                return false;
            }
        }
    }
    return false;
}


