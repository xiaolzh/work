#ifndef BANG_TIMER_H
#define BANG_TIMER_H
#include "swstd.h"
#include <string>
#include <stdio.h>
#include "logger.h"

#ifdef WIN32
#include "win_libs.h"
#else
#include <sys/time.h>
#endif

#ifdef WIN32
class TimerImpl {
    public:
        TimerImpl() {
            QueryPerformanceFrequency(&miPerSecond);
        }
        ~TimerImpl(){}
        /// begin timing
        void Start() {
            miFSpendTime = 0.0;
            QueryPerformanceCounter(&miStart);
        }
        /// stop timing
        void End() {
            QueryPerformanceCounter(&miEnd);
            miFSpendTime = (double)(miEnd.QuadPart - miStart.QuadPart) 
                            / (double)(miPerSecond.QuadPart);
            miCost = (int)(miFSpendTime*1000);  //ms
        }
        /// time cost in millisecond
        int Cost() const {return miCost;}
    private:
        double miFSpendTime;
        LARGE_INTEGER miStart;
        LARGE_INTEGER miEnd;
        /// CPU Hz
        LARGE_INTEGER miPerSecond;
        /// time cost
        int miCost;
};
#else
/// The implement of timer in linux
class TimerImpl {
    public:
        /// begin timing
        void Start() {
            gettimeofday (&tvpre , &tz);
        }
        /// stop timing
        void End() {   
            gettimeofday (&tvafter , &tz);
        }
        /// time cost in millisecond
        int Cost() const {
            return (tvafter.tv_sec-tvpre.tv_sec)*1000
                    + (tvafter.tv_usec-tvpre.tv_usec)/1000;
        }
    private:
        /// time cost
        struct timeval tvafter,tvpre;
        struct timezone tz;
};

/// A precise timer timing in microsecond
class PreciseTimer {
    public:
        /// begin timing
        void Start() {
            gettimeofday (&tvpre , &tz);
        }
        /// stop timing
        void End() {   
            gettimeofday (&tvafter , &tz);
        }
        /// time cost in microsecond
        int Cost() const {
            return (tvafter.tv_sec-tvpre.tv_sec)*1000000
                    + (tvafter.tv_usec-tvpre.tv_usec);
        }
    private:
        /// time cost
        struct timeval tvafter,tvpre;
        struct timezone tz;
};
#endif // WIN32

/// A timer for timing
class Timer {
public:
    /// begin timing
    void Start() { m_impl.Start();}
    /// stop timing
    void End() { m_impl.End();}
    /// time cost in milliseconds
    int Cost() const { return m_impl.Cost();}
private:
    TimerImpl m_impl;
};

/// A counter to log the time cost of a process in debug mode
class DebugTimerCounter {
#if _DEBUG
public:
    DebugTimerCounter(const std::string& info, Level lvl = LOG_INFO, 
        int limit = 0)
        : m_info(info), m_lvl(lvl), m_limit(limit) {
        m_timer.Start();
    }
    ~DebugTimerCounter() {
        m_timer.End();
        int cost = m_timer.Cost();
        if( cost > m_limit)
            LOG(m_lvl, "%s, cost: %d ms", m_info.c_str(), cost);
    }
private:
    Timer m_timer;
    std::string m_info;
    Level m_lvl;
    int m_limit;
#else
public:
    DebugTimerCounter(const std::string& info, Level lvl = LOG_INFO, 
        int limit = 0)
    {}
#endif
};

/// A counter to log the time cost of a process
class TimerCounter {
public:
    TimerCounter(const std::string& info, Level lvl = LOG_INFO, 
        int limit = 0) 
        : m_info(info), m_lvl(lvl), m_limit(limit) {
        m_timer.Start();
    }
    ~TimerCounter() {
        m_timer.End();
        int cost = m_timer.Cost();
        if( cost > m_limit)
            LOG(m_lvl, "%s, cost: %d ms", m_info.c_str(), cost);
    }
private:
    Timer m_timer;
    std::string m_info;
    Level m_lvl;
    int m_limit;
};

#endif // ~>.!.<~
