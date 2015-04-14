/**
 * Copyright 2011 DangDang Inc. 技术研发部
 * All rights reserved.
 *
 * 文件名称: TimeUtil.h
 * 摘    要: 用于一段程序代码执行时间统计
 *
 * 当前版本: 1.0
 * 作    者: 刘鸿超
 * 完成日期: 2011-06-20
 */

#ifndef _PRODUCT_INFO_UPDATER_TIME_UTIL_H_
#define _PRODUCT_INFO_UPDATER_TIME_UTIL_H_

#include <sys/time.h>

class TimeUtil
{
public:
    /**
     * 初始化当前类对象产生的时间
     */
    TimeUtil()
    {
        gettimeofday(&start_time, NULL);
    }
    
    /**
     * 函数介绍: 获取从当前类对象产生到此方法调用经历的毫秒数
     * 输入参数: void
     * 返回值  : 经历的毫秒数
     *
     * 用例:
     * {
     *   TimeUtil tu;
     *   // some code
     *   long passTime = tu.getPassedTime();
     * }
     */
    int getPassedTime()
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        int r = (now.tv_sec - start_time.tv_sec) * 1000;
        r += (now.tv_usec - start_time.tv_usec) / 1000;
        return r;
    }
    
private:
    struct timeval start_time;
};

#endif // _PRODUCT_INFO_UPDATER_TIME_UTIL_H_

