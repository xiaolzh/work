#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include "PublicFunc.h"

//default file access permissions for new files
#define FILE_MODE  (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
int  public_func::write_lock(int fd, off_t offset, int whence, off_t len) 
	{
			struct flock    lock;

			lock.l_type = F_WRLCK;             /* F_RDLCK, F_WRLCK, F_UNLCK */
			lock.l_start = offset;  /* byte offset, relative to l_whence */
			lock.l_whence = whence; /* SEEK_SET, SEEK_CUR, SEEK_END */
			lock.l_len = len;               /* #bytes (0 means to EOF) */

			return( fcntl(fd, F_SETLK, &lock) );
	}

int public_func::FileLock(int *fd_ptr,const char * lockfilename)
{
	int	fd;
		
	if ( (fd = open(lockfilename, O_WRONLY | O_CREAT, FILE_MODE)) < 0)
	{
		return -1;
	}

	//try and set a write lock on the entire file
	if (write_lock(fd, 0, SEEK_SET, 0) < 0)	
	{
		if (errno == EACCES || errno == EAGAIN)
		{
			close(fd);
			return -1;	/* running */
		}
		else
		{
			close(fd);
			//WriteLog(0,"write_lock error\n");
			//DealInfo(CRIT,"write_lock error\n");
			return -1;
		}
	}
	*fd_ptr=fd;
	return 0;
}

int public_func::FileUnLock(int fd,const char * filename)
{
	close(fd);
	remove(filename);
	return 0;
}

//È¥µô×Ö·û´®Á½¶ËµÄ¿Õ¸ñ(°üÀ©'\n';'\t')
char* public_func::Trim(char* Str)
{
	int len;
	char *p;
	p=Str;
	while(isspace((int)(*p)))
		p++;
	strcpy(Str,p);

	len=strlen(Str);
	if(len<=0)	return Str;
	while(isspace((int)Str[len-1])) len--;
	Str[len] = '\0';
	return Str;
}
string temp;
//È¥µô×Ö·û´®Á½¶ËµÄ¿Õ¸ñ(°üÀ©'\n';'\t')
string& public_func::Trim(const string& str)
{
	int pos=0;
	temp = str;
	while(isspace((int)(temp[pos]))) pos++;
	if (pos) temp.erase(0,pos);
	pos=(int)temp.length()-1;
	while(pos>0&&isspace((int)(temp[pos]))) pos--;
	temp.erase(pos+1,(int)str.length()-pos-1);
	return temp;
}

string public_func::GetLocalTime()
{
	time_t timer;
	struct tm *now;
	char str_t[15];

	timer = time(NULL);
	now   = localtime(&timer);	 
	sprintf(str_t,"%04d%02d%02d%02d%02d%02d",now->tm_year+1900,now->tm_mon+1,now->tm_mday,now->tm_hour,now->tm_min,now->tm_sec);
	return str_t;
}

public_func::CExceptionLog::CExceptionLog()
{
	error_str="";
}

public_func::CExceptionLog::CExceptionLog(int exception_type,int exception_no,string exception_info)
{
    this->exception_type = exception_type;
    this->exception_no = exception_no;
    error_str = exception_info;       
}

public_func::CExceptionLog::CExceptionLog(const string& msg):error_str(msg)
{
}

public_func::CExceptionLog::CExceptionLog(const CExceptionLog& old):error_str(old.error_str)
{	
}

const char* public_func::CExceptionLog::what() const throw()
{
	return error_str.c_str();
}

int public_func::CExceptionLog::GetExceptionType()
{
    return exception_type ;    
}

int public_func::CExceptionLog::GetExceptionNo()
{
    return exception_no;    
}


void public_func::Itoa(char a[], int i,size_t sz)
{
	for(int j=(int)sz-1;j>=0;--j) {
		a[j] = i%10 + '0';
		i/=10;
	}
	a[sz] = '\0';
}

int public_func::timeSetting(/*char lower[7],*/char upper[7])
{
	int months[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	time_t t = time( 0 );
    char tmp[9];
    strftime(tmp, sizeof(tmp), "%Y%m%d",localtime(&t) ); 
	
	if (strlen(tmp) != 8) {
		cerr<<"Error time format:" <<tmp <<endl;
		return 1;
	}

	int time = atoi(tmp);
	int day = time % 100;
	int month = time % 10000 /100;
	int year = time / 10000; 
	
	if (day > 1) --day;
	else {
		if (month > 1) --month;
		else {
			month = 12;
			--year;
		}
		day = months[month];
	}
	int timeupper = year * 10000 + month * 100 + day;
/*	
	if (day > 6) day -= 6;
	else {
		if (month > 1) --month;
		else {
			month = 12;
			--year;
		}		
		day = months[month]- (6 - day);
	}
	int timelower = year*10000 + month*100 + day;
*/	
	Itoa(upper,timeupper,6);
//	Itoa(lower,timelower,6);
	cout<<upper<<endl/*<<lower<<endl*/;
	
	return 0;
}
//×Ö·û´®·Ö¸îº¯Êý
void public_func::split(const string& str, const string& sp, vector<string>& out)
{
	out.clear();
	string s = str;
	size_t beg, end;
	while (!s.empty())
	{
		beg = s.find_first_not_of(sp);
		if (beg == string::npos)
		{
			break;
		}
		end = s.find(sp, beg);
		out.push_back(s.substr(beg,end-beg));
		if (end == string::npos)
		{
			break;
		}
		s = s.substr(end, s.size()-end);
	}
}

