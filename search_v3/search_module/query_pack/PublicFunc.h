#ifndef _PUBLIC_FUNC_H_
#define _PUBLIC_FUNC_H_
#include<iostream>
#include<vector>
#include<set>
using namespace std;

namespace public_func
{
	//文件锁函数
	int write_lock(int fd, off_t offset, int whence, off_t len); 

	//进程级文件锁
	int FileLock(int *fd_ptr,const char * lockfilename);

	//删除文件锁
	int FileUnLock(int fd,const char * filename);

	//字符串分割函数
	void split(const string& str, const string& sp, vector<string>& out);

	//时间设置函数
	int timeSetting(/*char lower[7],*/char upper[7]);

	//整型转字符串函数
	void Itoa(char a[], int i,size_t sz);

	//获取系统时间
	string GetLocalTime();

	//去空格函数(传入string)
	string& Trim(const string& str);

	//去空格函数(传入 char*)
	char* Trim(char* Str);

	//日值的管理类
	class CExceptionLog :public std::exception
	{
	private:
		std::string  error_str;	//错误信息
		int exception_type;     //异常的类型
		int exception_no;       //异常的标识
	public:
		CExceptionLog();
		explicit CExceptionLog(const std::string&);
		CExceptionLog(int exception_type,int exception_no,string exception_info);
		CExceptionLog(const CExceptionLog&);
		~CExceptionLog() throw(){}
		const char* what() const throw();
		int GetExceptionType();
		int GetExceptionNo();
	};
	
};
#endif

