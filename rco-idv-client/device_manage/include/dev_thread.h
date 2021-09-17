#ifndef _THREAD_H
#define _THREAD_H

#include "dev_common.h"

namespace rcdev
{

class Thread;
class JoinableThread;
class UnjoinableThread;
class Mutex;
class RecurciveMutex;
class Lock;
class Condition;

static inline void rel_time(struct timespec& time, unsigned long long delta_nano)
{
    clock_gettime(CLOCK_MONOTONIC, &time);
    delta_nano += (unsigned long long)time.tv_sec * 1000 * 1000 * 1000;
    delta_nano += time.tv_nsec;
    time.tv_sec = long(delta_nano / (1000 * 1000 * 1000));
    time.tv_nsec = long(delta_nano % (1000 * 1000 * 1000));
}

/*********************************************
Thread's safety is depended on thread_main and opaque
*********************************************/
typedef void* (*thread_main_t)(void*);
class Thread
{
public:
    Thread(thread_main_t thread_main, void* opaque)
    {
        pthread_create(&_thread, NULL, thread_main, opaque);
    }
    virtual ~Thread() {};
    pthread_t get_thread_id(){return _thread;}
protected:
    pthread_t _thread;
};
class JoinableThread:public Thread
{
public:
    JoinableThread(thread_main_t thread_main, void* opaque = NULL):Thread(thread_main, opaque){}
    virtual ~JoinableThread();
    void join(){pthread_join(_thread, NULL);}
};
class UnjoinableThread:public Thread
{
public:
    UnjoinableThread(thread_main_t thread_main, void* opaque = NULL)
        :Thread(thread_main, opaque), _running(true), _thread_main(thread_main), _opaque(opaque)
    {
        detach();
    }
    virtual ~UnjoinableThread(){}
    
    void cancel()
    {
        if(_running)
        {
            pthread_cancel(_thread);
            _thread = 0;
            _running = false;
        }
    }
    void create()
    {
        if(!_running)
        {
            pthread_create(&_thread, NULL, _thread_main, _opaque);
            _running = true;
        }
    }
    void reload()
    {
        if(_running)
        {
            cancel();
            create();
        }
    };
private:
    void detach()
    {
        pthread_detach(_thread);
    }
    
    bool _running;
    thread_main_t _thread_main;
    void* _opaque;
};

class Mutex
{
public:
    enum Type {
        NORMAL,
        RECURSIVE,
    };
    Mutex(Type type = NORMAL)
    {
        int r;
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        if (type == NORMAL) 
        {
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
        }
        else if (type == RECURSIVE) 
        {
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        }
        else
        {
            DEV_LOG_ERR("invalid type %d", type);
        }

        if ((r = pthread_mutex_init(&_mutex, &attr))) {
            DEV_LOG_ERR("init failed %d", r);
        }
        pthread_mutexattr_destroy(&attr);
    }
    
    ~Mutex(){pthread_mutex_destroy(&_mutex);}
private:
    friend class Lock;
    pthread_mutex_t* get() {return &_mutex;}
    pthread_mutex_t _mutex;
};

class RecurciveMutex: public Mutex 
{
public:
    RecurciveMutex() : Mutex(Mutex::RECURSIVE) {}
};

class Lock {
public:
    Lock(Mutex& mutex)
        : _locked (true)
        , _mutex (mutex)
    {
        pthread_mutex_lock(_mutex.get());
    }
    ~Lock()
    {
        unlock();
    }
    void lock()
    {
        if (!_locked) {
            pthread_mutex_lock(_mutex.get());
            _locked = true;
        }
    }
    void unlock()
    {
        if (_locked) {
            pthread_mutex_unlock(_mutex.get());
            _locked = false;
        }
    }
    bool is_locked() { return _locked;}
private:
    friend class Condition;
    pthread_mutex_t* get() {return _mutex.get();}
    bool _locked;
    Mutex& _mutex;
};

class Condition {
public:
    Condition()
    {
        pthread_condattr_t attr;
        pthread_condattr_init(&attr);
        pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
        pthread_cond_init(&_condition, &attr);
        pthread_condattr_destroy(&attr);
    }
    ~Condition()
    {
        pthread_cond_destroy(&_condition);
    }
    void notify_one()
    {
        pthread_cond_signal(&_condition);
    }
    void notify_all()
    {
        pthread_cond_broadcast(&_condition);
    }
    void wait(Lock& lock)
    {
        pthread_cond_wait(&_condition, lock.get());
    }

    bool timed_wait(Lock& lock, unsigned long long nano)
    {
        struct timespec time;
        rel_time(time, nano);
        int r = pthread_cond_timedwait(&_condition, lock.get(), &time);
        if (r)
        {
            if (r != ETIMEDOUT)
            {
                DEV_LOG_WARNING("pthread_cond_timedwait %d", r);
            }
            return false;
        }
        return true;
    }

private:
    pthread_cond_t _condition;
};

} //namespace

#endif //_THREAD_H

