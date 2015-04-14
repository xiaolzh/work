#ifndef _PRODUCT_INFO_UPDATER_RESOURCE_UTIL_H_
#define _PRODUCT_INFO_UPDATER_RESOURCE_UTIL_H_

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <errno.h>
#include "PublicFunc.h"
#include "hash_wrap.h"
using namespace std;
using namespace public_func;
class ResourceUtil {
      public:
	ResourceUtil() {
	} ~ResourceUtil() {
	}

    /**
     * 函数介绍: 加载配置文件, 整个系统中只需要做一次
     * 输入参数: 
     *     const string & filename: 配置文件名
     * 返回值  : 
     *     bool: 加载是否成功, 成功返回true, 失败返回false
     */
	static bool configure(const string & filename);

    /**
     * 函数介绍: 根据传入的key值, 获取value, key-value在配置文件中为 key=value的格式
     * 输入参数: 
     *     const string & key: 配置文件中的key
     * 返回值  : 
     *     string: 与key对应的value, 若没有返回空串 ""
     */
	static string getProperty(const string & key);

    /**
     * 函数介绍: 根据传入的key值, 获取value, key-value在配置文件中为 key=value的格式
     * 输入参数: 
     *     const string & key: 配置文件中的key
     * 返回值  : 
     *     string: 与key对应的value, 若没有返回空串 ""
     */
	//static string trim(string & str);

    /**
     * 函数介绍: 释放已加载的属性, 可用于重新加载
     * 输入参数: void
     * 返回值  : void
     */
	static void release();

    /**
     * 函数介绍: 以 key=value 形式打印所有配置信息
     * 输入参数: void
     * 返回值  : void
     */
	static void printAll();

    /**
     * 函数介绍: 根据传入的sep字符分隔字符串, 并存入result中
     * 输入参数: 
     *     const string & str: 需要分隔的字符串
     *     const char sep: 分隔字符
     *     vector<string> & result: 存储分隔后的结果字符串
     * 返回值  : void
     */
	static void separate(const string & str, const char sep,
			     vector < string > &result);

      private:

    /**
     * 函数介绍: 读取配置文件中的信息到map缓存, 生成key-value
     * 输入参数: void
     * 返回值  : 
     *     bool: 成功返回true, 失败返回false
     */
	static bool readToMap();

      private:
	static string m_resFilename;	// 配置文件名
	static hash_map < string, string > m_props;	// 缓存key-value数据
	static const char CH_KV_SEPARATOR;	// 配置文件中key-value的分隔符
	static const char CH_ANNOTATION;	// 配置文件中表示注释行的字符
};

#endif				// _PRODUCT_INFO_UPDATER_RESOURCE_UTIL_H_
