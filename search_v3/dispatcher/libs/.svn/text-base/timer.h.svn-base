#ifndef TIMER_H
#define TIMER_H
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
class timer_impl 
{
public:
	timer_impl() 
	{
		QueryPerformanceFrequency(&miPerSecond);
	}
	~timer_impl(){}
	/// begin timing
	void start() 
	{
		miFSpendTime = 0.0;
		QueryPerformanceCounter(&miStart);
	}
	/// stop timing
	void end() 
	{
		QueryPerformanceCounter(&miEnd);
		miFSpendTime = (double)(miEnd.QuadPart - miStart.QuadPart) 
            / (double)(miPerSecond.QuadPart);
		miCost = (int)(miFSpendTime*1000);  //ms
	}
	/// time cost in millisecond
	int cost() const 
	{
		return miCost;
	}

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
class timer_impl 
{
private:
	/// time cost
	struct timeval tvafter,tvpre;
	struct timezone tz;

public:
	/// begin timing
	void start() 
	{
		gettimeofday (&tvpre , &tz);
	}
	/// stop timing
	void end() 
	{	
	gettimeofday (&tvafter , &tz);
	}
	/// time cost in millisecond
	int cost() const 
	{
		return (tvafter.tv_sec-tvpre.tv_sec)*1000 
            + (tvafter.tv_usec-tvpre.tv_usec)/1000;
	}
};

/// A precise timer timing in microsecond
class precise_timer
{
private:
	/// time cost
	struct timeval tvafter,tvpre;
	struct timezone tz;

public:
	/// begin timing
	void start()
	{
		gettimeofday (&tvpre , &tz);
	}
	/// stop timing
	void end()
	{	
		gettimeofday (&tvafter , &tz);
	}
	/// time cost in microsecond
	int cost() const
	{
		return (tvafter.tv_sec-tvpre.tv_sec)*1000000 
            + (tvafter.tv_usec-tvpre.tv_usec);
	}
};
#endif // WIN32

/// A timer for timing
class timer
{
public:
	/// begin timing
	void start()
	{
		m_impl.start();
	}

	/// stop timing
	void end()
	{
		m_impl.end();
	}

	/// time cost in milliseconds
	int cost() const
	{
		return m_impl.cost();
	}

private:
	timer_impl m_impl;
};


/// A counter to log the time cost of a process in debug mode
class debug_timer_counter
{
#if _DEBUG
public:
	debug_timer_counter(const std::string& info, Level lvl = LOG_INFO, 
        int limit = 0)
        : m_info(info), m_lvl(lvl), m_limit(limit)
	{
		m_timer.start();
	}
	~debug_timer_counter()
	{
		m_timer.end();
        int cost = m_timer.cost();
        if( cost > m_limit)
            LOG(m_lvl, "%s, cost: %d ms", m_info.c_str(), cost);
	}

private:
	timer m_timer;
	std::string m_info;
    Level m_lvl;
    int m_limit;
#else

public:
	debug_timer_counter(const std::string& info, Level lvl = LOG_INFO, 
        int limit = 0) 
    {}
#endif
};


/// A counter to log the time cost of a process
class timer_counter
{
public:
	timer_counter(const std::string& info, Level lvl = LOG_INFO, 
        int limit = 0)
        : m_info(info), m_lvl(lvl), m_limit(limit)
	{
		m_timer.start();
	}
	~timer_counter()
	{
		m_timer.end();
        int cost = m_timer.cost();
        if( cost > m_limit)
            LOG(m_lvl, "%s, cost: %d ms", m_info.c_str(), cost);
	}

private:
	timer m_timer;
	std::string m_info;
    Level m_lvl;
    int m_limit;
};

#endif // ~>.!.<~
