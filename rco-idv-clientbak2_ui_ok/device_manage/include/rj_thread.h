#ifndef __NET_THREAD_H__
#define __NET_THREAD_H__

#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

#ifndef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP PTHREAD_RECURSIVE_MUTEX_INITIALIZER
#endif


#define THREAD_NAME_LEN (64)

typedef void *(*RjThreadFunc)(void *);

typedef enum {
    NET_THREAD_UNINITED,
    NET_THREAD_RUNNING,
    NET_THREAD_WAITING,
    NET_THREAD_STOPPING,
} RjThreadStatus;


#include "dev_log.h"

class Mutex;
class Condition;

/*
 * for shorter type name and function name
 */
class Mutex
{
public:
    Mutex();
    ~Mutex();

    void lock();
    void unlock();
    int  trylock();

    class Autolock
    {
    public:
        explicit inline Autolock(Mutex& mutex) : mLock(mutex)
        {
            mLock.lock();
        }
        explicit inline Autolock(Mutex* mutex) : mLock(*mutex)
        {
            mLock.lock();
        }
        inline ~Autolock()
        {
            mLock.unlock();
        }
    private:
        Mutex& mLock;
    };

private:
    friend class Condition;

    pthread_mutex_t mMutex;

    Mutex(const Mutex &);
    Mutex &operator = (const Mutex&);
};

inline Mutex::Mutex()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mMutex, &attr);
    pthread_mutexattr_destroy(&attr);
}
inline Mutex::~Mutex()
{
    pthread_mutex_destroy(&mMutex);
}
inline void Mutex::lock()
{
    pthread_mutex_lock(&mMutex);
}
inline void Mutex::unlock()
{
    pthread_mutex_unlock(&mMutex);
}
inline int Mutex::trylock()
{
    return pthread_mutex_trylock(&mMutex);
}

typedef Mutex::Autolock AutoMutex;


/*
 * for shorter type name and function name
 */
class Condition
{
public:
    Condition();
    explicit Condition(int type);
    ~Condition();
    void wait(Mutex& mutex);
    int timedwait(Mutex& mutex, int timeout);
    void signal();

private:
    pthread_cond_t mCond;
};

inline Condition::Condition()
{
    pthread_condattr_t condaddr;
    pthread_condattr_init(&condaddr);
    pthread_condattr_setclock(&condaddr, CLOCK_MONOTONIC);
    pthread_cond_init(&mCond, &condaddr);
    pthread_condattr_destroy(&condaddr);
}
inline Condition::~Condition()
{
    pthread_cond_destroy(&mCond);
}
inline void Condition::wait(Mutex& mutex)
{
    pthread_cond_wait(&mCond, &mutex.mMutex);
}
inline int Condition::timedwait(Mutex& mutex, int timeout)
{
    struct timespec time;
    time.tv_sec  = (time_t)(timeout);
    time.tv_nsec = 0;

    return pthread_cond_timedwait(&mCond, &mutex.mMutex, &time);
}
inline void Condition::signal()
{
    pthread_cond_signal(&mCond);
}

class RjMutexCond
{
public:
    RjMutexCond() {};
    ~RjMutexCond() {};

    void lock()
    {
        mLock.lock();
    }
    void unlock()
    {
        mLock.unlock();
    }
    void signal()
    {
        mCondition.signal();
    }
    void wait()
    {
        mCondition.wait(mLock);
    }
    int  timedwait(int t)
    {
        return mCondition.timedwait(mLock, t);
    }
private:
    Mutex           mLock;
    Condition       mCondition;
};

class RjThread
{
public:
    RjThread(RjThreadFunc func, void *ctx, const char *name = NULL);
    ~RjThread() {};

    RjThreadStatus get_status() const;
    void set_status(RjThreadStatus status);

    void start();
    void stop();

    void lock()
    {
        mMutexCond.lock();
    }

    void unlock()
    {
        mMutexCond.unlock();
    }

    void wait()
    {
        mMutexCond.wait();
    }

    int timedwait(int timeout)
    {
        return mMutexCond.timedwait(timeout);
    }

    void signal()
    {
        mMutexCond.signal();
    }

private:
    pthread_t       mThread;
    RjMutexCond    mMutexCond;

    RjThreadStatus mStatus;
    RjThreadFunc   mFunction;
    char            mName[THREAD_NAME_LEN];
    void            *mContext;

    RjThread();
    RjThread(const RjThread &);
    RjThread &operator=(const RjThread &);
};


#endif /*__NET_THREAD_H__*/
