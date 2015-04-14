#ifndef _PUBLIC_FUNC_H_
#define _PUBLIC_FUNC_H_
#include<iostream>
#include<vector>
#include<set>
using namespace std;

namespace public_func
{
	//�ļ�������
	int write_lock(int fd, off_t offset, int whence, off_t len); 

	//���̼��ļ���
	int FileLock(int *fd_ptr,const char * lockfilename);

	//ɾ���ļ���
	int FileUnLock(int fd,const char * filename);

	//�ַ����ָ��
	void split(const string& str, const string& sp, vector<string>& out);

	//ʱ�����ú���
	int timeSetting(/*char lower[7],*/char upper[7]);

	//����ת�ַ�������
	void Itoa(char a[], int i,size_t sz);

	//��ȡϵͳʱ��
	string GetLocalTime();

	//ȥ�ո���(����string)
	string& Trim(const string& str);

	//ȥ�ո���(���� char*)
	char* Trim(char* Str);

	//��ֵ�Ĺ�����
	class CExceptionLog :public std::exception
	{
	private:
		std::string  error_str;	//������Ϣ
		int exception_type;     //�쳣������
		int exception_no;       //�쳣�ı�ʶ
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

