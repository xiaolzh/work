// UH_Define.h

#ifndef _UH_DEFINE_H_
#define _UH_DEFINE_H_

// 系统类型定义
#ifdef WIN32

#define _WIN32_ENV_
#define _SYSTEM_X86_

#else

#define _LINUX_ENV_
#define _SYSTEM_X64_

#endif

#define _MACOS_ENV_

// 头文件包含定义
#ifdef _WIN32_ENV_
#pragma warning(disable:4996)
#pragma warning(disable:4244)

#include <winsock2.h>
#pragma comment(lib, "ws2_32")

#include <UrlMon.h>
#pragma comment(lib, "UrlMon")

#include <mswsock.h>
#include <windows.h>

#include <io.h>
#include <time.h>
#include <share.h>
#include <direct.h>
#include <sys/stat.h>

#else

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _MACOS_ENV_
#else
#include <sys/epoll.h>
#endif

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <stdarg.h>
#include <iconv.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <signal.h>

#include <netdb.h>

#include <net/if.h>
#include <errno.h>
#include <unistd.h>

#endif

#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <errno.h>
#include <algorithm>

// 变量类型定义
typedef               char var_1;
typedef      unsigned char var_u1;
typedef              short var_2;
typedef     unsigned short var_u2;
typedef                int var_4;
typedef       unsigned int var_u4;
typedef              float var_f4;
typedef             double var_d8;
typedef				  void var_vd;
typedef				  bool var_bl;

#ifdef _SYSTEM_X86_

#define BITS_PER_LONG 32
#define BYTES_PER_LONG 4
#define SHIFT_BITS_PER_LONG 5

#ifdef _WIN32_ENV_
typedef            __int64 var_8;
typedef   unsigned __int64 var_u8;
#else
typedef          long long var_8;
typedef unsigned long long var_u8;
#endif // _WIN32_ENV_

#else

#define BITS_PER_LONG 64
#define BYTES_PER_LONG 8
#define SHIFT_BITS_PER_LONG 6

#ifdef _WIN32_ENV_
typedef            __int64 var_8;
typedef   unsigned __int64 var_u8;
#else
typedef               long var_8;
typedef      unsigned long var_u8;
#endif // _WIN32_ENV_

#endif // _SYSTEM_X86_

#define BIT(nr)                 (1UL << (nr))
#define BIT_MASK(nr)            (~0UL << ((nr) % BITS_PER_LONG))
#define BIT_CLEAR(nr)           (~0UL >> (BITS_PER_LONG - ((nr) % BITS_PER_LONG)))
#define BIT_WORD(nr)            ((nr) >> SHIFT_BITS_PER_LONG)

__inline var_u4 cp_add_and_fetch(var_u4* val)
{
#ifdef _WIN32_ENV_
    return InterlockedIncrement(val);
#else
    return __sync_add_and_fetch(val, 1);
#endif
}

__inline var_u4 cp_sub_and_fetch(var_u4* val)
{
#ifdef _WIN32_ENV_
    return InterlockedDecrement(val);
#else
    return __sync_sub_and_fetch(val, 1);
#endif
}

__inline var_u8 cp_add_and_fetch(var_u8* val)
{
#ifdef _WIN32_ENV_
    return InterlockedIncrement(val);
#else
    return __sync_add_and_fetch(val, 1);
#endif
}

__inline var_u8 cp_sub_and_fetch(var_u8* val)
{
#ifdef _WIN32_ENV_
    return InterlockedDecrement(val);
#else
    return __sync_sub_and_fetch(val, 1);
#endif
}

__inline void cp_lock_inc(var_u4* val)
{
#ifdef _WIN32_ENV_
    InterlockedIncrement(val);
#else
    __sync_add_and_fetch(val, 1);
#endif
}

__inline void cp_lock_dec(var_u4* val)
{
#ifdef _WIN32_ENV_    
    InterlockedDecrement(val);
#else
    __sync_sub_and_fetch(val, 1);
#endif
}

__inline void cp_lock_inc(var_u8* val)
{
#ifdef _WIN32_ENV_
    InterlockedIncrement(val);
#else
    __sync_add_and_fetch(val, 1);
#endif
}

__inline void cp_lock_dec(var_u8* val)
{
#ifdef _WIN32_ENV_
    InterlockedDecrement(val);
#else
    __sync_sub_and_fetch(val, 1);
#endif
}

// 休眠
#ifdef _WIN32_ENV_
#define cp_sleep(x)			Sleep(x)
#else
#define cp_sleep(x)			usleep(x*1000)
#endif

#ifdef _LINUX_ENV_
#define _snprintf	snprintf
#define THREADFUNTYPE  void*
#define WSAGetLastError()	errno
#else
#define THREADFUNTYPE  DWORD WINAPI
#endif

static var_vd cp_wait_dispose()
{
	for(;;) cp_sleep(5000);
}

// 互斥锁 读写锁
typedef struct CP_MutexLock_RW_LCK
{
	var_u4 rd_c; // read count
	var_u4 wr_f; // write flag

	CP_MutexLock_RW_LCK()
	{
		rd_c = 0;
		wr_f = 0;
	}
} CP_MUTEXLOCK_RW_LCK;

#ifdef _WIN32_ENV_
#include <windows.h>
typedef struct CP_MutexLock
{
	CRITICAL_SECTION mutex;

	CP_MutexLock()
	{
		InitializeCriticalSection(&mutex);
	}

	~CP_MutexLock()
	{
		DeleteCriticalSection(&mutex);
	}

	inline void lock()
	{
		EnterCriticalSection(&mutex);
	}

	inline void unlock()
	{
		LeaveCriticalSection(&mutex);
	}
} CP_MUTEXLOCK;

typedef struct CP_MutexLock_RW
{
	CRITICAL_SECTION mutex;
	
	var_u4 rd_c; // read count
	var_u4 wr_f; // write flag

	CP_MutexLock_RW()
	{
		InitializeCriticalSection(&mutex);
		
		rd_c = 0;
		wr_f = 0;
	}

	~CP_MutexLock_RW()
	{
		DeleteCriticalSection(&mutex);
	}

	inline void lock_r()
	{
		for(;;)
		{
			EnterCriticalSection(&mutex);
			if(wr_f != 0)
			{
				LeaveCriticalSection(&mutex);
				cp_sleep(1);

				continue;
			}	
			rd_c++;
			LeaveCriticalSection(&mutex);

			break;
		}
	}

	inline void lock_w()
	{
		for(;;)
		{
			EnterCriticalSection(&mutex);
			if(wr_f != 0 || rd_c != 0)
			{
				LeaveCriticalSection(&mutex);
				cp_sleep(1);

				continue;
			}
			wr_f = 1;
			LeaveCriticalSection(&mutex);

			break;
		}
	}

	inline void unlock()
	{
		EnterCriticalSection(&mutex);
		if(wr_f != 0)
			wr_f = 0;
		else
		{
			if(rd_c <= 0)
				printf("RW_MUTEXLOCK error\n");
			else
				rd_c--;
		}
		LeaveCriticalSection(&mutex);
	}
} CP_MUTEXLOCK_RW;

typedef struct CP_MutexLock_RW_FUN
{
	CRITICAL_SECTION mutex;

	CP_MutexLock_RW_FUN()
	{
		InitializeCriticalSection(&mutex);
	}

	~CP_MutexLock_RW_FUN()
	{
		DeleteCriticalSection(&mutex);
	}

	inline void lock_r(CP_MUTEXLOCK_RW_LCK* lck)
	{
		for(;;)
		{
			EnterCriticalSection(&mutex);
			if(lck->wr_f != 0)
			{
				LeaveCriticalSection(&mutex);
				cp_sleep(1);

				continue;
			}	
			lck->rd_c++;
			LeaveCriticalSection(&mutex);

			break;
		}
	}

	inline void lock_w(CP_MUTEXLOCK_RW_LCK* lck)
	{
		for(;;)
		{
			EnterCriticalSection(&mutex);
			if(lck->wr_f != 0 || lck->rd_c != 0)
			{
				LeaveCriticalSection(&mutex);
				cp_sleep(1);

				continue;
			}
			lck->wr_f = 1;
			LeaveCriticalSection(&mutex);

			break;
		}
	}

	inline void unlock(CP_MUTEXLOCK_RW_LCK* lck)
	{
		EnterCriticalSection(&mutex);
		if(lck->wr_f != 0)
			lck->wr_f = 0;
		else
		{
			if(lck->rd_c <= 0)
				printf("RW_MUTEXLOCK error\n");
			else
				lck->rd_c--;
		}
		LeaveCriticalSection(&mutex);
	}
} CP_MUTEXLOCK_RW_FUN;
#else
typedef struct CP_MutexLock
{
	pthread_mutex_t mutex;

	CP_MutexLock()
	{
		pthread_mutex_init(&mutex, NULL);
	}

	~CP_MutexLock()
	{
		pthread_mutex_destroy(&mutex);
	}

	inline void lock()
	{
		pthread_mutex_lock(&mutex);
	}

	inline void unlock()
	{
		pthread_mutex_unlock(&mutex);
	}
} CP_MUTEXLOCK;

typedef struct CP_MutexLock_RW
{
	var_u4 sign_mask;
	var_u4 sign_lock;
	
	pthread_mutex_t locker;

	CP_MutexLock_RW(var_bl is_writer_wins = true)
	{
		// w_using(1bit) | r_stop(1bit) | r_count(30bit)
		sign_mask = is_writer_wins == true ? 0xc0000000 : 0x80000000;
		sign_lock = 0;
		
		pthread_mutex_init(&locker, NULL);
	}
	
	~CP_MutexLock_RW()
	{
		pthread_mutex_destroy(&locker);
	}
	
	inline void lock_r()
	{
		for(;;)
		{
			pthread_mutex_lock(&locker);
			
			if (!(sign_lock & sign_mask)) {	// no w_using, no r_stop(if there is)
				sign_lock++;
				pthread_mutex_unlock(&locker);
				break;
			}
			pthread_mutex_unlock(&locker);
			usleep(1000);
		}
	}
	
	inline void lock_w()
	{
		for(;;)
		{
			pthread_mutex_lock(&locker);
			
			if(!(sign_lock & 0xbfffffff)) // no w_using, no r_count
			{
				sign_lock |= sign_mask;
				pthread_mutex_unlock(&locker);
				break;
			}
			sign_lock |= sign_mask & 0x40000000; // set r_stop to sign_lock(if there is)
			
			pthread_mutex_unlock(&locker);
			usleep(1000);
		}
	}
	
	inline void unlock()
	{
		pthread_mutex_lock(&locker);
		
		if ((sign_lock & 0xc0000000) == sign_mask)	// unlock w_lock
			sign_lock = 0;
		else
			sign_lock--;
		
		pthread_mutex_unlock(&locker);
	}
} CP_MUTEXLOCK_RW;

typedef struct CP_MutexLock_RW_FUN
{
	pthread_mutex_t mutex;

	CP_MutexLock_RW_FUN()
	{
		pthread_mutex_init(&mutex, NULL);
	}

	~CP_MutexLock_RW_FUN()
	{
		pthread_mutex_destroy(&mutex);
	}

	inline void lock_r(CP_MUTEXLOCK_RW_LCK* lck)
	{
		for(;;)
		{
			pthread_mutex_lock(&mutex);
			if(lck->wr_f != 0)
			{
				pthread_mutex_unlock(&mutex);
				cp_sleep(1);

				continue;
			}	
			lck->rd_c++;
			pthread_mutex_unlock(&mutex);

			break;
		}
	}

	inline void lock_w(CP_MUTEXLOCK_RW_LCK* lck)
	{
		for(;;)
		{
			pthread_mutex_lock(&mutex);
			if(lck->wr_f != 0 || lck->rd_c != 0)
			{
				pthread_mutex_unlock(&mutex);
				cp_sleep(1);

				continue;
			}
			lck->wr_f = 1;
			pthread_mutex_unlock(&mutex);

			break;
		}
	}

	inline void unlock(CP_MUTEXLOCK_RW_LCK* lck)
	{
		pthread_mutex_lock(&mutex);
		if(lck->wr_f != 0)
			lck->wr_f = 0;
		else
		{
			if(lck->rd_c <= 0)
				printf("RW_MUTEXLOCK error\n");
			else
				lck->rd_c--;
		}
		pthread_mutex_unlock(&mutex);
	}
} CP_MUTEXLOCK_RW_FUN;

#endif

#ifdef _WIN32_ENV_
#define CP_P64		"%I64d"
#define CP_PU64		"%I64u"
#else
#define CP_P64		"%ld"
#define CP_PU64		"%lu"
#endif

// 64bit string to value
#ifdef _WIN32_ENV_

#if _MSC_VER == 1200
#define cp_strtoval_u64(x)		((var_u8)_atoi64(x))
#define cp_strtoval_64(x)		_atoi64(x)
#else
#define cp_strtoval_u64(x)		_strtoui64(x, NULL, 10)
#define cp_strtoval_64(x)		_strtoi64(x, NULL, 10)
#endif

#else
#define cp_strtoval_u64(x)		strtoul(x, NULL, 10)
#define cp_strtoval_64(x)		strtol(x, NULL, 10)
#endif

#ifdef _WIN32_ENV_
static var_4 cp_strncasecmp(const var_1* s1, const var_1* s2, size_t n)
{
    return strnicmp(s1, s2, n);
}
static var_4 cp_strnicmp(const var_1* s1, const var_1* s2, size_t n)
{
    return strnicmp(s1, s2, n);
}
#else
static var_4 cp_strncasecmp(const var_1* s1, const var_1* s2, size_t n)
{
    return strncasecmp(s1, s2, n);
}
static var_4 cp_strnicmp(const var_1* s1, const var_1* s2, size_t n)
{
    return strncasecmp(s1, s2, n);
}
#endif

static var_4 cp_dio_open(const var_1* name, const var_1* mode)
{
#ifdef _MACOS_ENV_
	return -1;
#else
	if(*mode == 'w')
		return open(name, O_CREAT | O_WRONLY | O_DIRECT);
	if(*mode == 'r')
		return open(name, O_RDONLY | O_DIRECT);
#endif
}

static inline var_4 cp_dio_close(var_4 fildes)
{
	return close(fildes);
}

static inline size_t cp_dio_write(var_4 fildes, const var_vd* buf, size_t nbyte)
{
	return write(fildes, buf, nbyte);
}

static inline size_t cp_dio_read(var_4 fildes, var_vd* buf, size_t nbyte)
{
	return read(fildes, buf, nbyte);
}

static inline off_t cp_dio_seek(var_4 fildes, off_t offset, var_4 whence)
{
	return lseek(fildes, offset, whence);
}

static inline var_vd* cp_dio_alloc(size_t size)
{
	var_vd* memptr = NULL;
	
	//if(posix_memalign(&memptr, 512, size))
		return NULL;
	
	return memptr;
}

static inline var_vd cp_dio_free(var_vd* memptr)
{
	free(memptr);
}

#ifdef _WIN32_ENV_
static var_4 cp_change_file_size(FILE* fp, var_8 size)
{
	if(chsize(fileno(fp), size))
		return -1;
	return 0;
}
#else
static var_4 cp_change_file_size(FILE* fp, off_t size)
{
	if(ftruncate(fileno(fp), size))
		return -1;
	return 0;
}
#endif

#ifdef _WIN32_ENV_
static var_4 cp_dir_open(var_vd*& handle, const var_1* path_name)
{
    typedef struct dir_travel_handle
    {
        var_4 travel_flag;
        var_4 dir_handle;
        struct _finddata_t file_handle;
    } DIR_TRAVEL_HANDLE;
    
    DIR_TRAVEL_HANDLE* dir_handle = new DIR_TRAVEL_HANDLE;

	var_1 real_path_name[256];
	sprintf(real_path_name, "%s/*.*", path_name);

    dir_handle->travel_flag = 1;
    dir_handle->dir_handle = _findfirst(real_path_name, &dir_handle->file_handle);
    if(dir_handle->dir_handle == -1)
    {
        delete dir_handle;
        return -1;
    }
    
    handle = (var_vd*)dir_handle;
    
    return 0;
}

static var_4 cp_dir_travel(var_vd* handle, var_1* file_name)
{
    typedef struct dir_travel_handle
    {
        var_4 travel_flag;
        var_4 dir_handle;
        struct _finddata_t file_handle;
    } DIR_TRAVEL_HANDLE;
    
    DIR_TRAVEL_HANDLE* dir_handle = (DIR_TRAVEL_HANDLE*)handle;
    
    for(;;)
    {
        if(dir_handle->travel_flag != 1)
        {
            if(_findnext(dir_handle->dir_handle, &dir_handle->file_handle))
                return -1;
        }
        else
            dir_handle->travel_flag = 0;
    
        var_4 len = strlen(dir_handle->file_handle.name);
        
        if(len == 1 && dir_handle->file_handle.name[0] == '.')
            continue;
        if(len == 2 && *(var_u2*)dir_handle->file_handle.name == *(var_u2*)"..")
            continue;
        if(dir_handle->file_handle.attrib & _A_SUBDIR)
            continue;
        
        memcpy(file_name, dir_handle->file_handle.name, len);
        file_name[len] = 0;
        
        break;
    }
    
    return 0;
}

static var_4 cp_dir_close(var_vd*& handle)
{
    typedef struct dir_travel_handle
    {
        var_4 travel_flag;
        var_4 dir_handle;
        struct _finddata_t file_handle;
    } DIR_TRAVEL_HANDLE;
    
    _findclose(((DIR_TRAVEL_HANDLE*)handle)->dir_handle);
    
    delete (DIR_TRAVEL_HANDLE*)handle;
    handle = NULL;
    
    return 0;
}
#else
static var_4 cp_dir_open(var_vd*& handle, const var_1* path_name)
{
    handle = (var_vd*)opendir(path_name);
    if(handle == NULL)
        return -1;
    
    return 0;
}
static var_4 cp_dir_travel(var_vd* handle, var_1* file_name)
{
    struct dirent* dirent;
    
    for(;;)
    {
        dirent = readdir((DIR*)handle);
        if(dirent == NULL)
            return -1;
        
        var_4 len = (var_4)strlen(dirent->d_name);
        
        if(len == 1 && dirent->d_name[0] == '.')
            continue;
        if(len == 2 && *(var_u2*)dirent->d_name == *(var_u2*)"..")
            continue;
/*
        if(dirent->d_namlen == 1 && dirent->d_name[0] == '.')
            continue;
        if(dirent->d_namlen == 2 && *(var_u2*)dirent->d_name == *(var_u2*)"..")
            continue;
*/
        if(dirent->d_type == DT_DIR)
            continue;
        
        memcpy(file_name, dirent->d_name, len);
        file_name[len] = 0;
        
        break;
    }
    
    return 0;
}

static var_4 cp_dir_close(var_vd*& handle)
{
    closedir((DIR*)handle);
    handle = NULL;
    
    return 0;
}
#endif

static var_8 cp_get_file_size(var_1* in_file)
{
	struct stat file_info;
	if(stat(in_file, &file_info))
		return -1;
	return file_info.st_size;
}

static var_4 cp_fix_file(var_1* file, const var_1* end_flag, var_4 end_flag_len, var_8* fix_len = NULL)
{
    if(end_flag_len > 128)
        return -1;
    
    if(access(file, 0))
        return -1;
    
    var_8 real_len = 0;
    var_8 file_len = cp_get_file_size(file);
    if(file_len < 0)
        return -1;
    if(file_len == 0)
	{
		if(fix_len)
			*fix_len = file_len;

        return 0;
	}
    
    var_4  read_len = 0;
    var_1* read_buf = new var_1[1<<20];
    if(read_buf == NULL)
        return -1;
    
    FILE* fp = fopen(file, "rb+");
    if(fp == NULL)
    {
        delete read_buf;
        
        return -1;
    }
    
    for(;;)
    {
        read_len = 1<<20;
        if(file_len < read_len)
            read_len = (var_4)file_len;
        file_len -= read_len;
        
        fseek(fp, file_len, SEEK_SET);
        
        if(fread(read_buf, read_len, 1, fp) != 1)
        {
            fclose(fp);
            delete read_buf;
            
            return -1;
        }
        
        var_4 loop_num = read_len - end_flag_len + 1;
        if(loop_num <= 0) // not enough length
            break;
        
        var_1* beg_pos = read_buf + (read_len - end_flag_len);
        var_1* cur_pos = beg_pos;
        for(var_4 i = 0; i < loop_num; i++)
        {
            if(memcmp(cur_pos, end_flag, end_flag_len) == 0)
                break;
            cur_pos--;
        }
        
        if(cur_pos + loop_num != beg_pos)
        {
            real_len = file_len + read_len - (beg_pos - cur_pos);
            break;
        }
        
        if(file_len <= 0)
            break;
    }
    
    if(cp_change_file_size(fp, real_len))
    {
        fclose(fp);
        delete read_buf;
        return -1;
    }
    
    fclose(fp);
    
    delete read_buf;
    
    if(fix_len)
        *fix_len = real_len;
    
    return 0;
}

static var_4 cp_remove_file(var_1* src_file)
{
	while(access(src_file, 0) == 0 && remove(src_file))
    {
        printf("cp_remove_file - remove %s error\n", src_file);
        cp_sleep(5000);
    }

	return 0;
}


static var_4 sys_error_code()
{
#ifdef _WIN32_ENV_
    return GetLastError();
#else
    return errno;
#endif
}

static var_4 cp_rename_file(var_1* src_file, var_1* des_file)
{
	if(access(src_file, 0))
		return -1;
	
    while(access(des_file, 0) == 0 && remove(des_file))
    {
        printf("cp_rename_file - remove %s error\n", des_file);
        cp_sleep(5000);
    }
    while(rename(src_file, des_file))
    {
        printf("cp_rename_file - rename %s to %s error\n", src_file, des_file);
        cp_sleep(5000);
    }
    
    return 0;
}

static var_4 cp_swap_file(var_1* new_file, var_1* org_file)
{
    var_1 bak_file[256];
    sprintf(bak_file, "%s.recovery", org_file);
    
    while(access(bak_file, 0) == 0 && remove(bak_file))
    {
        printf("cp_change_file - remove %s error\n", bak_file);
        cp_sleep(5000);
    }
    while(access(org_file, 0) == 0 && rename(org_file, bak_file))
    {
        printf("cp_change_file - rename %s to %s error\n", org_file, bak_file);
        cp_sleep(5000);
    }
    while(access(new_file, 0) == 0 && rename(new_file, org_file))
    {
        printf("cp_change_file - rename %s to %s error\n", new_file, org_file);
        cp_sleep(5000);
    }
    
    return 0;
}

static var_4 cp_swap_file_nb(var_1* new_file, var_1* org_file)
{
    var_1 bak_file[256];
    sprintf(bak_file, "%s.recovery", org_file);
    
    if(access(bak_file, 0) == 0 && remove(bak_file))
    {
        printf("cp_change_file - remove %s error\n", bak_file);
        return -1;
    }
    if(access(org_file, 0) == 0 && rename(org_file, bak_file))
    {
        printf("cp_change_file - rename %s to %s error\n", org_file, bak_file);
        return -1;
    }
    if(access(new_file, 0) == 0 && rename(new_file, org_file))
    {
        printf("cp_change_file - rename %s to %s error\n", new_file, org_file);
        return -1;
    }
    
    return 0;
}

static var_4 cp_recovery_file(var_1* org_file)
{
    if(access(org_file, 0) == 0)
        return 0;
    
    var_1 bak_file[256];
    sprintf(bak_file, "%s.recovery", org_file);
    
    if(access(bak_file, 0))
        return 0;
    
    if(rename(bak_file, org_file))
    {
        printf("cp_recovery_file - rename %s to %s error\n", bak_file, org_file);
        return -1;
    }
    
    return 0;
}

static var_4 cp_clear_file(var_1* org_file)
{
    var_1 bak_file[256];
    sprintf(bak_file, "%s.recovery", org_file);

    if(access(bak_file, 0) == 0 && remove(bak_file))
    {
        printf("cp_clear_file - remove %s error\n", bak_file);
        return -1;
    }

    if(access(org_file, 0) == 0 && remove(org_file))
    {
        printf("cp_clear_file - remove %s error\n", org_file);
        return -1;
    }
    
    return 0;
}

static var_1* get_file_name(var_1* full_path)
{
    var_1* end = full_path + strlen(full_path) - 1;
    while (end != full_path)
    {
        if (('\\' == *end) || ('/' == *end))
        {
            ++end;  
            break;
        }
        --end;
    }

    return (end == full_path) ? NULL : end;
}

static var_4 cp_copy_file(var_1* org_file, var_1* des_dir, var_1* buf = NULL, var_u4 len = 0)
{
    var_1 des_file[256];
    sprintf(des_file, "%s/%s", des_dir, get_file_name(org_file));
    var_1 tmp_file[256];
    sprintf(tmp_file, "%s.tmp", des_file);

    var_1* buf_tmp = buf;
    var_u4 buf_len = len;
    if ((NULL == buf_tmp) || (0 == buf_len))
    {
        buf_len = 1<<20;
        buf_tmp = new var_1[buf_len];
        if (NULL == buf_tmp)
        {
            return -1;
        }
    }

    FILE* fh_rd = NULL, *fh_wt = NULL;
    try
    {
        var_u4 size_rd = cp_get_file_size(org_file);
        if (0 > size_rd)
        {
            throw -1;
        }

        fh_rd = fopen(org_file, "rb");
        if (NULL == fh_rd)
        {
            throw -2;
        }

        fh_wt = fopen(tmp_file, "wb");
        if (NULL == fh_wt)
        {
            throw -3;
        }

        while (size_rd > 0)
        {
            var_u4 one_rd = buf_len;
            if (one_rd > size_rd)
            {
                one_rd = size_rd;
            }

            var_4 ret = fread(buf_tmp, one_rd, 1, fh_rd);
            if (1 != ret)
            {
                throw -4;
            }

            ret = fwrite(buf_tmp, one_rd, 1, fh_wt);
            if (1 != ret)
            {
                throw -5;
            }

            size_rd -= one_rd;
        }

        fclose(fh_wt);
        fh_wt = NULL;

        cp_rename_file(tmp_file, des_file);

        throw 0;
    }
    catch (const var_4 _err_code)
    {
    	if ((NULL == buf) && (NULL != buf_tmp))
    	{
            delete[] buf_tmp;
    	}
        if (NULL != fh_rd)
        {
            fclose(fh_rd);
        }
        if (NULL != fh_wt)
        {
            fclose(fh_wt);
        }

        return _err_code;
    }
}

/************************************************************************/
// 通讯相关
/************************************************************************/
#ifdef _WIN32_ENV_
#define CP_SOCKET_T		SOCKET
#else
#define CP_SOCKET_T		int
#endif

#ifdef _WIN32_ENV_
#define cp_close_socket(x)		shutdown(x, 0x02); ::closesocket(x)
#else
#define cp_close_socket(x)		shutdown(x, 0x02); ::close(x)
#endif

static var_4 cp_set_overtime(CP_SOCKET_T in_socket, var_4 in_time)
{
#ifdef _WIN32_ENV_
	setsockopt(in_socket, SOL_SOCKET, SO_RCVTIMEO,(var_1*)&in_time, sizeof(in_time));
	setsockopt(in_socket, SOL_SOCKET, SO_SNDTIMEO,(var_1*)&in_time, sizeof(in_time));
#else
	struct timeval over_time;
	over_time.tv_sec = in_time / 1000;
	over_time.tv_usec = (in_time % 1000) * 1000;

	setsockopt(in_socket, SOL_SOCKET, SO_RCVTIMEO,(var_1*)&over_time, sizeof(over_time));
	setsockopt(in_socket, SOL_SOCKET, SO_SNDTIMEO,(var_1*)&over_time, sizeof(over_time));
#endif
	return 0;
}


static var_4 cp_init_socket()
{
#ifdef _WIN32_ENV_
	WSAData cWSAData;
	if(WSAStartup(MAKEWORD(2, 2), &cWSAData))
	{
		printf("cp_init_socket: init socket failure\n");
		return -1;
	}
#else
	sigset_t signal_mask;
	sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGPIPE);
	if(pthread_sigmask(SIG_BLOCK, &signal_mask, NULL))
	{
		printf("cp_init_socket: block sigpipe failure\n");
		return -1;
	}
#endif

	return 0;
}

static var_4 cp_listen_socket(CP_SOCKET_T& in_listen, var_u2 in_port)
{
	struct sockaddr_in my_server_addr;
	my_server_addr.sin_family = AF_INET;
	my_server_addr.sin_port = htons(in_port);
	my_server_addr.sin_addr.s_addr = htons(INADDR_ANY);

	in_listen = socket(AF_INET, SOCK_STREAM, 0);
	if(in_listen < 0)
	{
		printf("cp_listen_socket: create socket failure\n");
		return -1;
	}
	if(bind(in_listen, (struct sockaddr*)&my_server_addr, sizeof(my_server_addr)) < 0)
	{
		cp_close_socket(in_listen);

		printf("cp_listen_socket: bind to port %d failure!\n", in_port);
		return -1;
	}
	if(listen(in_listen, SOMAXCONN) < 0)
	{
		cp_close_socket(in_listen);

		printf("cp_listen_socket: listen to port %d failure\n", in_port);
		return -1;
	}

	return 0;
}

static var_4 cp_connect_socket(CP_SOCKET_T& in_socket, var_1* in_ip, var_u2 in_port)
{
	struct sockaddr_in my_server_addr;
	my_server_addr.sin_family = AF_INET;
	my_server_addr.sin_port = htons(in_port);
	my_server_addr.sin_addr.s_addr = inet_addr(in_ip);

	in_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(in_socket < 0)
	{
		printf("cp_connect_socket: create socket failure\n");
		return -1;
	}
	if(connect(in_socket, (struct sockaddr*)&my_server_addr, sizeof(my_server_addr)) < 0)
	{
		cp_close_socket(in_socket);

		printf("cp_connect_socket: connect %s:%d failure\n", in_ip, in_port);
		return -1;
	}

	return 0;
}

static var_4 cp_accept_socket(CP_SOCKET_T in_listen, CP_SOCKET_T& out_socket)
{
	struct sockaddr_in my_client_addr;
#ifdef _WIN32_ENV_
	int length = sizeof(my_client_addr);
#else
	socklen_t length = sizeof(my_client_addr);
#endif

	out_socket = accept(in_listen, (struct sockaddr*)&my_client_addr, &length);
	if(out_socket == -1)
		return -1;

	return 0;
}

static var_4 cp_sendbuf(CP_SOCKET_T in_socket, var_1* in_buffer, var_4 in_buflen)
{
	var_4 retval = 0, finlen = 0;
	do{
		retval = (var_4)send(in_socket, in_buffer + finlen, in_buflen - finlen, 0);
		if(retval > 0)
			finlen += retval;
#ifdef _WIN32_ENV_
#else
        if(retval < 0 && errno == EINTR)
            continue;
#endif
	}while(retval > 0 && finlen < in_buflen);
	if(retval < 0 || finlen < in_buflen)
		return -1;

	return 0;
}

static var_4 cp_recvbuf(CP_SOCKET_T in_socket, var_1* in_buffer, var_4 in_buflen)
{
	var_4 retval = 0, finlen = 0;
	do{
		retval = (var_4)recv(in_socket, in_buffer + finlen, in_buflen - finlen, 0);
		if(retval > 0)
			finlen += retval;
	}while(retval > 0 && finlen < in_buflen);
	if(retval < 0 || finlen < in_buflen)
		return -1;

	return 0;
}

static var_4 cp_recvbuf_onebyte(CP_SOCKET_T in_socket, var_1 end_char, var_1* in_buffer, var_4& in_buflen)
{
	in_buflen = 0;

	var_4 once_len = 0;

	for(;;)
	{		
		once_len = (var_4)recv(in_socket, in_buffer + in_buflen, 1, 0);
		if(once_len <= 0)
			break;
		in_buflen += once_len;
		if(in_buffer[in_buflen - 1] == end_char)
			break;
	}
	if(once_len <= 0)
		return -1;

	in_buflen--;

	return 0;
}

static var_4 cp_recvbuf_twobyte(CP_SOCKET_T in_socket, var_1 end_one, var_1 end_two, var_1* in_buffer, var_4& in_buflen)
{
	in_buflen = 0;

	var_4 once_len = 0;

	for(;;)
	{		
		once_len = (var_4)recv(in_socket, in_buffer + in_buflen, 1, 0);
		if(once_len <= 0)
			break;
		in_buflen += once_len;
		if(in_buflen >= 2 && in_buffer[in_buflen - 2] == end_one && in_buffer[in_buflen - 1] == end_two)
			break;
	}
	if(once_len <= 0)
		return -1;

	in_buflen -= 2;

	return 0;
}

static var_4 cp_sendfile(CP_SOCKET_T in_socket, var_1* in_file)
{
	var_8 file_len = cp_get_file_size(in_file);
	if(file_len < 0)
	{
		printf("cp_sendfile: get file size failure\n");
		return -1;
	}

	FILE* fp = fopen(in_file, "rb");
	if(fp == NULL)
	{
		printf("cp_sendfile: open file %s failure\n", in_file);
		return -1;
	}

	if(cp_sendbuf(in_socket, (var_1*)&file_len, 4))
	{
		printf("cp_sendfile: send file size failure\n");
		fclose(fp);
		return -1;
	}	

	var_1 send_buf[4096];
	var_8 send_len = 0;

	while(file_len > 0)
	{
		send_len = 4096;
		if(file_len < 4096)
			send_len = file_len;
		
		if(fread(send_buf, send_len, 1, fp) != 1)
		{
			printf("cp_sendfile: read file %s failure\n", in_file);
			fclose(fp);
			return -1;
		}
		if(cp_sendbuf(in_socket, send_buf, (var_4)send_len))
		{
			printf("cp_sendfile: send file content failure\n");
			fclose(fp);
			return -1;
		}

		file_len -= send_len;
	}

	fclose(fp);

	return 0;
}

static var_4 cp_unsegword(var_1* in_buf, var_4 in_len, var_1* out_buf)
{
    var_1* ptr_beg = in_buf;
    var_1* ptr_end = in_buf + in_len;
    var_1* ptr_out = out_buf;
    
    while(ptr_beg < ptr_end)
    {
        if(*ptr_beg < 0)
        {
            *ptr_out++ = *ptr_beg++;
            if(ptr_beg < ptr_end)
                *ptr_out++ = *ptr_beg++;
        }
        else if(*ptr_beg == ' ')
        {
            int flag = 0;
            while(ptr_beg < ptr_end && *ptr_beg == ' ')
            {
                if(flag == 0)
                {
                    ptr_beg++;
                    flag = 1;
                    continue;
                }
                *ptr_out++ = *ptr_beg++;
                flag = 0;
            }
        }
        else
            *ptr_out++ = *ptr_beg++;
    }
    
    return (var_4)(ptr_out - out_buf);
}

#ifdef _WIN32_ENV_
static var_4 cp_lock_file(var_4 fd)
{
	return 0;
}
static var_4 cp_unlock_file(var_4 fd)
{
	return 0;
}
#else
static void print_lock(struct flock lock)
{
	printf(" -----------------------------\n");

	if (lock.l_type == F_RDLCK) {
		printf("\tl_type: F_RDLCK\n");
	}
	else if (lock.l_type == F_WRLCK) {
		printf("\tl_type: F_WRLCK\n");
	}
	else if (lock.l_type == F_UNLCK) {
		printf("\tl_type: F_UNLCK\n");
	}

	printf("\tl_start: %d\n", (int)lock.l_start);

	if (lock.l_whence == SEEK_SET) {
		printf("\tl_whence: SEEK_SET\n");
	}
	else if (lock.l_whence == SEEK_CUR) {
		printf("\tl_whence: SEEK_CUR\n");
	}
	else if (lock.l_whence == SEEK_END) {
		printf("\tl_whence: SEEK_END\n");
	}

	printf("\tl_len: %d\n", (int)lock.l_len);

	printf(" -----------------------------\n");
}

static var_4 cp_lock_file(var_4 fd, var_2 type = F_RDLCK)
{
printf("F_RDLCK = %d, F_WRLCK = %d, F_UNLCK = %d\n", F_RDLCK, F_WRLCK, F_UNLCK);
	struct flock lock;

	// 加锁整个文件
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;

	for(;;)
	{
		lock.l_type = type;

		if(fcntl(fd, F_SETLK, &lock) == 0) 
		{
			// 共享锁
			if(lock.l_type == F_RDLCK)
				printf("read only set by %d\n", getpid());
			// 互斥锁
			else if(lock.l_type == F_WRLCK)
				printf("write lock set by %d\n", getpid());
			// 释放锁
			else if(lock.l_type == F_UNLCK)
				printf("release lock by %d\n", getpid());

			print_lock(lock);
			return 0;
		}
		else 
		{
			/*
			* 获得lock的描述，也就是将文件fd的加锁信息存入到lock结构中
			* 如果成功则返回0
			* 如果失败则返回-1
			*/
			if(fcntl(fd, F_GETLK, &lock) == 0) 
			{
				if(lock.l_type != F_UNLCK) 
				{
					if(lock.l_type == F_RDLCK)
						printf("read lock already set by %d\n", lock.l_pid);
					else if(lock.l_type == F_WRLCK)
						printf("write lock already set by %d\n", lock.l_pid);		
				}
			}
			else
				printf("cannot get the description of struck flock.\n");
		}
	}

	return 0;
}

static var_4 cp_unlock_file(var_4 fd)
{
	struct flock fd_lock;
	fd_lock.l_type = F_UNLCK;
	fd_lock.l_whence = SEEK_SET;
	fd_lock.l_start = 0;
	fd_lock.l_len = 0;

	if(fcntl(fd, F_SETLK, &fd_lock) == -1)
		return -1;

	return 0;
}
#endif

/************************************************************************/
// 线程相关
/************************************************************************/
#ifdef _WIN32_ENV_
#define CP_THREAD_T		DWORD WINAPI
#else
#define CP_THREAD_T		void*
#endif

#ifdef _WIN32_ENV_
static var_4 cp_create_thread(LPTHREAD_START_ROUTINE in_function, void* in_argv)
{
	DWORD dwThreadID;
	HANDLE hThreadHandle = CreateThread(NULL, 0, in_function, in_argv, 0, &dwThreadID);
	if(hThreadHandle == NULL)
		return -1;
	CloseHandle(hThreadHandle);
	return 0;
}
#else
static var_4 cp_create_thread(void* (in_function)(void*), void* in_argv)
{
	pthread_t pid;
	if(pthread_create(&pid, 0, in_function, in_argv))
		return -1;
	return 0;
}
#endif

static struct tm* cp_localtime(time_t local_sec, struct tm* local_tm, var_4 time_zone = 8)
{
    const char days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};  
    
    time_t now_sec = local_sec;
    
#ifdef _WIN32_ENV_
    now_sec += time_zone * 3600;
#else
    local_tm->tm_gmtoff = time_zone * 3600;
    local_tm->tm_zone = NULL;
    
    now_sec += local_tm->tm_gmtoff;
#endif
	
	local_tm->tm_wday = ((now_sec / 86400 + 4) % 7);
    
    local_tm->tm_isdst = 0;
        
    local_tm->tm_sec = now_sec % 60;
    now_sec /= 60;
    local_tm->tm_min = now_sec % 60;
    now_sec /= 60;
    
    var_4 four_year_hour_num = (var_4)(now_sec / 35064); // (365*4 + 1) * 24
    
    local_tm->tm_year = (four_year_hour_num << 2) + 70;
    now_sec %= 35064;
	
    for(;;)
    {
        var_4 one_year_hour_num = 8760; // 365 * 24
        if((local_tm->tm_year & 3) == 0)
            one_year_hour_num += 24;
        if(now_sec < one_year_hour_num)
            break;
        local_tm->tm_year++;
        now_sec -= one_year_hour_num;
    }
    
    local_tm->tm_hour = now_sec % 24;
    now_sec /= 24;
    
    local_tm->tm_yday = (int)now_sec;
    
    now_sec++;
    if((local_tm->tm_year & 3) == 0)
    {
        if(now_sec > 60)
            now_sec--;
        else 
        {
            if(now_sec == 60)
            {
                local_tm->tm_mon = 1;
                local_tm->tm_mday = 29;
                return local_tm;
            }
        }
    }
    
    for(local_tm->tm_mon = 0; days[local_tm->tm_mon] < now_sec; local_tm->tm_mon++)
        now_sec -= days[local_tm->tm_mon];
    
    local_tm->tm_mday = (int)now_sec;
    
    return local_tm;
}

static var_u1 BM_RULE_BIT[] = {0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1};
static var_u1 BM_RULE_ZERO_BIT[] = {0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE};

#define BM_SET_BIT(bm, val)			(((var_u1*)(bm))[(val)>>3] |= BM_RULE_BIT[(val)&0x7])
#define BM_SET_BIT_ZERO(bm, val)	(((var_u1*)(bm))[(val)>>3] &= BM_RULE_ZERO_BIT[(val)&0x7])
#define BM_GET_BIT(bm, val)			(((var_u1*)(bm))[(val)>>3] & BM_RULE_BIT[(val)&0x7])

#define BM_LEN_1BYTE(num)	(((num) + 7) / 8)
#define BM_LEN_2BYTE(num)	(((num) + 15) / 16)
#define BM_LEN_4BYTE(num)	(((num) + 31) / 32)
#define BM_LEN_8BYTE(num)	(((num) + 63) / 64)

#ifdef _WIN32_ENV_
typedef struct _cp_memory_map_file_
{
	var_vd* pointer;

	_cp_memory_map_file_()
	{
		in_file = NULL;
		pointer = NULL;
	}
	~_cp_memory_map_file_()
	{
		close_MMF();
	}

	var_4 open_MMF(var_1* file,var_1 iMode=0)
	{
		if(file==NULL)
			return -1;

// 		DWORD dwAccess=GENERIC_READ;
// 		DWORD dwShare=FILE_SHARE_READ;
// 		DWORD dwCreat=OPEN_EXISTING;
// 		DWORD dwAttr=FILE_ATTRIBUTE_READONLY|FILE_FLAG_RANDOM_ACCESS;
// 
// 		DWORD  dwProtect=PAGE_READONLY;
// 
// 		DWORD dwMapAccess=FILE_MAP_READ;
// 
// 		if(iMode==1)
// 		{
// 			dwAccess=GENERIC_WRITE;
// 			dwShare=FILE_SHARE_WRITE;
// 			dwCreat=TRUNCATE_EXISTING|CREATE_NEW;
// 			dwAttr= FILE_FLAG_SEQUENTIAL_SCAN;
// 
// 			dwProtect=PAGE_WRITECOPY;
// 
// 			dwMapAccess=FILE_MAP_ALL_ACCESS ;
// 		}
// 		else if(iMode==2)
// 		{
// 			dwAccess=GENERIC_WRITE | GENERIC_READ;
// 
// 			dwProtect=PAGE_READWRITE;
// 
// 			dwMapAccess=FILE_MAP_ALL_ACCESS ;
// 		}
// 
// 		var_8 ulFileSize=cp_get_file_size(file);
// 		if(ulFileSize<=0)
// 		{
// 			printf("file: %s empty!\n",file);
// //			return -2;
// 			return 0;
// 		}
// 		
// 		HANDLE hFile = CreateFile((LPCWSTR)file,dwAccess,dwShare,NULL,dwCreat, dwAttr,NULL);
// 		if(hFile==NULL)
// 		{
// 			printf("createfile:%s failed!,ret=%d\n",file,WSAGetLastError());
// 			return -3;
// 		}
// 
// 		in_file=CreateFileMapping(hFile,NULL,dwProtect,(DWORD)(ulFileSize>>32),(DWORD)(ulFileSize&0xFFFFFFFF),NULL);
// 		if(in_file==NULL)
// 		{	
// 			printf("CreateFileMapping failed! ret=%d\n",WSAGetLastError());
// 			CloseHandle(hFile);	
// 			return -4;
// 		}
// 		CloseHandle(hFile);
// 
// 	//	SYSTEM_INFO sinf;
// 	//	GetSystemInfo(&sinf);
// 	//	DWORD dwAllocSize=(ulFileSize/sinf.dwAllocationGranularity);
// 	//	dwAllocSize+=1;
// 
// 		pointer =MapViewOfFile(in_file,dwMapAccess,0,0,ulFileSize);
// 
// 		if(pointer==NULL)
// 		{
// 			printf("MapViewOfFile failed! ret=%d\n",WSAGetLastError());
// 
// 			CloseHandle(hFile);
// 			CloseHandle(in_file);	
// 			return -5;
// 		}

		return 0;
	}

	var_4 close_MMF()
	{
		if(pointer)// 从进程的地址空间撤消文件数据映像
		{
			UnmapViewOfFile(pointer);
			pointer=NULL;
		}
		if(in_file)
		{
			CloseHandle(in_file);
			in_file=NULL;
		}

		return 0;
	}
private:
	HANDLE  in_file;

} CP_MMF;
#else
typedef struct _cp_memory_map_file_
{
	var_vd* pointer;

	_cp_memory_map_file_()
	{
		pointer = NULL;
		siFileSize=0;
		m_cOpenFlag=-1;
	}

	~_cp_memory_map_file_()
	{
		close_MMF();
	}

	var_4 open_MMF(var_1* file,var_1 iMode=0)
	{
		if(file==NULL)
			return -1;

		m_cOpenFlag=iMode;

		struct stat stFile;
		if(stat(file,&stFile)<0)
		{
			printf("error to stat %s\n",file);
			return -2;
		}
		siFileSize=stFile.st_size;
		if(siFileSize<=0)
		{
//			printf("file %s len:%d\n",file,stFile.st_size);
			return 0;
		}
		int iFlags=O_RDONLY;

		int iPro=PROT_READ;
		if(iMode==1)
		{
			iFlags=O_WRONLY|O_CREAT;

			iPro=PROT_WRITE;
		}
		else if(iMode==2)
		{
			iFlags=O_RDWR;
		}

		int fd=open(file, iFlags);
		if(fd<0)
		{
			printf("error to open %s\n",file);
			return -4;
		}

		pointer = mmap(NULL,siFileSize, iPro,MAP_SHARED,fd,0);
		if(MAP_FAILED == pointer)
		{
			printf("error to mmap,ret:%d\n",errno);
			close(fd);
			return -5;
		}
		close(fd);

		//void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offsize);
		return 0;
	}

	var_4 close_MMF()
	{
		if(pointer)
		{
			if(m_cOpenFlag==1)
				msync(pointer,siFileSize,MS_SYNC);

			munmap(pointer, siFileSize);
			pointer=NULL;
		}
        return 0;
	}

private:
	size_t  siFileSize;
	var_1   m_cOpenFlag;
} CP_MMF;
#endif

static var_4 cp_skip_useless_char(var_1* str, var_4 len, var_1*& beg, var_1*& end)
{
    var_1* beg_pos = str;
    var_1* end_pos = str + len - 1;
    
    while(beg_pos <= end_pos && (*beg_pos == '\t' || *beg_pos == '\n' || *beg_pos == '\r' || *beg_pos == ' '))
        beg_pos++;
    
    if(beg_pos > end_pos)
        return -1;
        
    while(beg_pos <= end_pos && (*end_pos == '\t' || *end_pos == '\n' || *end_pos == '\r' || *end_pos == ' '))
        end_pos--;
    
    if(beg_pos > end_pos)
        return -1;
    
    beg = beg_pos;
    end = end_pos;

    return 0;
}

static var_1* cp_skip_useless_char_front(var_1* str, var_4 len)
{
    var_1* beg = str;
    var_1* end = str + len;
    
    while(beg < end && (*beg == '\t' || *beg == '\n' || *beg == '\r' || *beg == ' '))
        beg++;
        
    return beg;
}

static var_1* cp_skip_useless_char_back(var_1* str, var_4 len)
{
    var_1* beg = str;
    var_1* end = str + len - 1;
    
    while(beg < end && (*end == '\t' || *end == '\n' || *end == '\r' || *end == ' '))
        end--;
    
    return end;
}

static var_4 cp_drop_useless_char(var_1* str_ptr)
{
	var_4 len = (var_4)strlen(str_ptr) - 1;
	while(len >= 0 && (str_ptr[len] == '\t' || str_ptr[len] == '\n' || str_ptr[len] == '\r' || str_ptr[len] == ' '))
		len--;	
	str_ptr[++len] = 0;

	return len;
}

static var_4 cp_drop_useless_char_url(var_1* str_ptr)
{
	var_4 len = (var_4)strlen(str_ptr) - 1;
	while(len >= 0 && (str_ptr[len] == '\t' || str_ptr[len] == '\n' || str_ptr[len] == '\r' || str_ptr[len] == ' ' || str_ptr[len] == '/'))
		len--;	
	str_ptr[++len] = 0;

	return len;
}

// 时间统计
#ifdef _WIN32_ENV_
typedef struct _cp_stat_time_
{
	DWORD dwTime_1, dwTime_2;

	inline void set_time_begin()
	{
		dwTime_1 = GetTickCount();
	}

	inline void set_time_end()
	{
		dwTime_2 = GetTickCount();
	}

	inline var_4 get_time_s()
	{
		return (dwTime_2 - dwTime_1) / 1000;
	}

	inline var_4 get_time_ms()
	{
		return dwTime_2 - dwTime_1;
	}

	inline var_4 get_time_us()
	{
		return dwTime_2 - dwTime_1;
	}

	static var_4 get_time()
	{
		return GetTickCount();
	}

	inline void clear()
	{
		dwTime_1 = 0;
		dwTime_2 = 0;
	}

} CP_STAT_TIME;
#else
typedef struct _cp_stat_time_
{
	struct timeval tv_1, tv_2;
	
	inline void set_time_begin()
	{
		gettimeofday(&tv_1, 0);
	}

	inline void set_time_end()
	{
		gettimeofday(&tv_2, 0);
	}

	inline var_4 get_time_s()
	{
		return (var_4)((tv_2.tv_sec - tv_1.tv_sec) + (tv_2.tv_usec - tv_1.tv_usec)/1000000);
	}

	inline var_4 get_time_ms()
	{
		return (var_4)((tv_2.tv_sec - tv_1.tv_sec)*1000 + (tv_2.tv_usec - tv_1.tv_usec)/1000);
	}

	inline var_4 get_time_us()
	{
		return (var_4)((tv_2.tv_sec - tv_1.tv_sec)*1000000 + (tv_2.tv_usec - tv_1.tv_usec));
	}

	static var_8 get_time()
	{
		struct timeval tv_T;
		gettimeofday(&tv_T, 0);

		return tv_T.tv_sec*1000000 + tv_T.tv_usec;
	}
	
	inline void clear()
	{
		memset(&tv_1, 0, sizeof(struct timeval));
		memset(&tv_2, 0, sizeof(struct timeval));
	}

} CP_STAT_TIME;
#endif

static var_4 cp_create_dir(var_1* dir)
{
    if(dir == NULL)
        return -1;
    
    var_4 len = (var_4)strlen(dir);
    if(len <= 0 || len >= 256)
        return -1;
    
    char  dir_name[256];
    char* dir_ptr = dir_name;
    char* ptr = dir;

    if(*ptr == '\\' || *ptr == '/')
        *dir_ptr++ = *ptr++;
    
    if(*ptr == 0)
        return -1;
    
    for(;;)
    {
        while(*ptr)
        {
            if(*ptr == '\\' || *ptr == '/')
                break;
            *dir_ptr++ = *ptr++;
        }
        
        *dir_ptr = 0;
        
#ifdef _WIN32_ENV_
        if(_mkdir(dir_name))
#else
         if(mkdir(dir_name, S_IRWXO|S_IRWXU))
#endif
        {
            if(errno != EEXIST)
                return -1;
        }
        
        if(*ptr)
            *dir_ptr++ = *ptr++;
        
        if(ptr - dir >= len)
            break;
    }

    return 0;
}

#ifdef _WIN32_ENV_
static var_1* cp_get_local_ip()
{
	var_1 hostname[128];
	gethostname(hostname, 128);

	struct hostent* ptr_hostent = gethostbyname(hostname); 
	   
	return inet_ntoa(*(struct in_addr*)ptr_hostent->h_addr_list[0]); 
}
#else
static var_1* cp_get_local_ip()
{
	CP_SOCKET_T sock = socket(AF_INET, SOCK_DGRAM, 0); 
	if(socket < 0)
	{
		printf("cp_get_local_ip: create socket failure\n");
		return NULL;
	}

	struct ifreq ifr;
	strcpy(ifr.ifr_name, "eth0");

	if(ioctl(sock, SIOCGIFADDR, &ifr) < 0) 
	{ 
		cp_close_socket(sock);

		printf("cp_get_local_ip: call ioctl failure\n");
		return NULL;
	} 

	struct sockaddr_in sin;
	memcpy(&sin, &ifr.ifr_addr, sizeof(struct sockaddr_in)); 

	cp_close_socket(sock);

	return inet_ntoa(sin.sin_addr); 
}
#endif

#ifdef _WIN32_ENV_
#else
static void catch_kill(var_vd (*func)(var_4))
{
	signal(SIGINT, func);  // catch ctrl + c
	signal(SIGTERM, func); // catch kill
}

static var_4 get_process_id(var_1* process_name)
{
	if(process_name == NULL)
		return -1;

	var_1 command[512];
	sprintf(command, "ps ax | grep %s > _tmp_process.list", process_name);		

	var_4 status = system(command);
	if(WIFEXITED(status) && status ==0)
	{
		printf("get_process_id success, status = %d, WIFEXITED(status) = %d\n", status, WIFEXITED(status));
	}
	else
	{
		printf("get_process_id failure, status = %d, WIFEXITED(status) = %d\n", status, WIFEXITED(status));
		return -1;
	}

	FILE* fp = fopen("_tmp_process.list", "r");
	if(fp == NULL)
		return -1;
	while(fgets(command, 512, fp))
	{
		if(strstr(command, process_name))
			break;
	}
	fclose(fp);

	remove("_tmp_process.list");

	if(strstr(command, process_name) == NULL)
		return 0;

	var_1* ptr = command;
	while(ptr && *ptr == ' ')
		ptr++;
	if(ptr == NULL)
		return -1;

	return (var_4)atol(ptr);
}
#endif

template <class T_Key>
const var_vd cp_generate_random(T_Key* result, var_4 num)
{
	std::random_shuffle<T_Key>(result, result + num);
}

/*
子进程的结束状态返回后存于status，底下有几个宏可判别结束情况
WIFEXITED(status)如果子进程正常结束则为非0值。
WEXITSTATUS(status)取得子进程exit()返回的结束代码，一般会先用WIFEXITED 来判断是否正常结束才能使用此宏。
WIFSIGNALED(status)如果子进程是因为信号而结束则此宏值为真
WTERMSIG(status)取得子进程因信号而中止的信号代码，一般会先用WIFSIGNALED 来判断后才使用此宏。
WIFSTOPPED(status)如果子进程处于暂停执行情况则此宏值为真。一般只有使用WUNTRACED 时才会有此情况。
WSTOPSIG(status)取得引发子进程暂停的信号代码，
*/

#endif // _UH_DEFINE_H_
