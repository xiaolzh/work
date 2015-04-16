// UT_HashTable_Pro.h

#ifndef __UT_HASH_TABLE_PRO_H__
#define __UT_HASH_TABLE_PRO_H__

#include "UH_Define.h"
#include "UC_Allocator_Recycle.h"

#define MAX_TABLE_LOCKER_NUM	1000
#define MIN_TABLE_LOCKER_NUM	1

#define MAX_DELAY_LOCKER_NUM	1000
#define MIN_DELAY_LOCKER_NUM	10

typedef struct _UT_Hash_Table_Pro_Travel_Info_
{
	var_4  is_dynamic;
	var_4  is_delete;
	var_4  is_lock;
	var_4  travel_idx;
	var_1* travel_pos;
	var_1* travel_pre;
} UT_HASH_TABLE_PRO_TRAVEL_INFO;

template <class T_Key>
class UT_HashTable_Pro
{
public:
	UT_HashTable_Pro()
	{
	}
	
	~UT_HashTable_Pro()
	{
	}
	
	var_4 init(var_8 table_size, var_u8 max_key_num = -1, var_4 store_size = 0, var_1* name_sto = NULL, var_1* name_inc = NULL, var_1* name_flg = NULL, var_4 is_delay_recycle = 0)
	{
		if(table_size <= 0 || store_size < 0)
			return -1;
		
		if(is_delay_recycle == 1 && store_size == 0)
			return -1;
		
		m_table_size = table_size;
		m_store_size = store_size;
		
		if(m_table_size == 1) // for key is negative max value
			m_table_size++;
		
		m_max_key_num = max_key_num;
		m_now_key_num = 0;
		
		m_is_persistent = 0;
		
		if(name_sto && name_inc && name_flg)
		{
			strcpy(m_name_sto, name_sto);
			strcpy(m_name_inc, name_inc);
			strcpy(m_name_flg, name_flg);
			
			m_is_persistent = 1;
		}
		
		m_is_delay_recycle = is_delay_recycle;
		
		// key + ptr + lck(1) + ref(4) + del(1) + buf
		m_len_ptr = sizeof(T_Key);
		
		if(m_is_delay_recycle == 1)
		{
			m_len_lck = m_len_ptr + sizeof(var_vd*);
			m_len_ref = m_len_lck + sizeof(var_1);
		}
		else
		{
			m_len_lck = 0xEFFFFFFF;
			m_len_ref = m_len_ptr + sizeof(var_vd*);
		}
		
		m_len_del = m_len_ref + sizeof(var_u4);
		m_len_sto = m_len_del + sizeof(var_1);
		m_len_mem = m_len_sto + m_store_size;
		
		m_table_ptr = new var_1*[m_table_size];
		if(m_table_ptr == NULL)
			return -1;
		memset(m_table_ptr, 0, sizeof(var_1*) * m_table_size);
		
		m_table_flg = new var_4[m_table_size];
		if(m_table_flg == NULL)
			return -1;
		memset(m_table_flg, 0, sizeof(var_4) * m_table_size);
		
		m_table_lck_num = (var_4)(m_table_size / 1000);
		if(m_table_lck_num > MAX_TABLE_LOCKER_NUM)
			m_table_lck_num = MAX_TABLE_LOCKER_NUM;
		if(m_table_lck_num < MIN_TABLE_LOCKER_NUM)
			m_table_lck_num = MIN_TABLE_LOCKER_NUM;
		
		m_table_lck = new CP_MUTEXLOCK[m_table_lck_num];
		if(m_table_lck == NULL)
			return -1;
		
		if(m_is_delay_recycle == 1)
		{
			m_store_lck_num = (var_4)(m_max_key_num / 1000);
			if(m_store_lck_num < 0 || m_store_lck_num > MAX_DELAY_LOCKER_NUM)
				m_store_lck_num = MAX_DELAY_LOCKER_NUM;
			if(m_store_lck_num < MIN_DELAY_LOCKER_NUM)
				m_store_lck_num = MIN_DELAY_LOCKER_NUM;
			
			m_store_lck = new CP_MUTEXLOCK[m_store_lck_num];
			if(m_store_lck == NULL)
				return -1;
		}
		
		if(m_ar.initMem(m_len_mem))
			return -1;
		
		if(m_is_persistent)
		{
			if(restore())
				return -1;
		}
		
		return 0;
	}
	
	var_4 add(T_Key key, var_vd* value = NULL, var_vd** const reference = NULL, var_4 is_overwrite = 0, var_4 is_lock = 1)
	{
		if(is_lock)
			m_global_lck.lock_r();
		
		var_8 idx = (var_8)(key % m_table_size);
		if(idx < 0)
			idx *= -1;
		
		lock_table_w(idx);
		
		if(m_is_persistent && is_lock && persistent(key, value, 0))
		{
			unlock_table(idx);
			
			m_global_lck.unlock();
			
			return -1;
		}
		
		var_1* ptr = m_table_ptr[idx];
		var_1* pre = ptr;
		
		while(ptr)
		{
			if(*(T_Key*)ptr == key)
			{
				if(is_overwrite) // overwrite data
				{
					var_u4* counter = (var_u4*)(ptr + m_len_ref);
					
					if(m_is_delay_recycle == 0) // no delay recycle
					{
						while(*counter) // wait counter is zero
							cp_sleep(1);
						
						memcpy(ptr + m_len_sto, value, m_store_size); // copy data
					}
					else if(*counter == 0) // delay recycle and counter is zero
					{
						memcpy(ptr + m_len_sto, value, m_store_size);
					}
					else // delay recycle and counter is nonzero
					{
						lock_store(ptr);
						
						if(*counter == 0) // watch counter is zero, again
							memcpy(ptr + m_len_sto, value, m_store_size);
						else
						{
							// create new node
							var_1* buf = m_ar.AllocMem();
							if(buf == NULL)
							{
								unlock_store(ptr);
								unlock_table(idx);
								
								if(is_lock)
									m_global_lck.unlock();
								
								return -1;
							}
							
							*(T_Key*)buf = key;
							*(var_1*)(buf + m_len_lck) = 0;
							*(var_u4*)(buf + m_len_ref) = 0;
							*(buf + m_len_del) = 0;
							if(m_store_size && value)
								memcpy(buf + m_len_sto, value, m_store_size);
							
							// insert new node to list and drop old node
							*(var_1**)(buf + m_len_ptr) = *(var_1**)(ptr + m_len_ptr);
							
							if(pre == ptr)
								m_table_ptr[idx] = buf;
							else
								*(var_1**)(pre + m_len_ptr) = buf;
							
							// delete old node
							*(ptr + m_len_del) = 1;
							
							unlock_store(ptr);
							
							ptr = buf;
						}
					}
				}
				
				if(reference)
				{
					if(m_store_size == 0)
						*reference = NULL;
					else
					{
						cp_lock_inc((var_u4*)(ptr + m_len_ref));
						*reference = (var_vd*)(ptr + m_len_sto);
					}
				}
				
				unlock_table(idx);
				
				if(is_lock)
					m_global_lck.unlock();
				
				return 1;
			}
			
			pre = ptr;
			ptr = *(var_1**)(ptr + m_len_ptr);
		}
		
		ptr = m_ar.AllocMem();
		if(ptr == NULL)
		{
			unlock_table(idx);
			
			if(is_lock)
				m_global_lck.unlock();
			
			return -1;
		}
		
		m_lck_key_num.lock();
		if(m_now_key_num >= m_max_key_num)
		{
			m_lck_key_num.unlock();
			
			unlock_table(idx);
			
			if(is_lock)
				m_global_lck.unlock();
			
			return -1;
		}
		
		m_now_key_num++;
		m_lck_key_num.unlock();
		
		*(T_Key*)ptr = key;
		if (m_is_delay_recycle)
			*(var_1*)(ptr + m_len_lck) = 0;
		*(var_u4*)(ptr + m_len_ref) = 0;
		*(ptr + m_len_del) = 0;
		if(m_store_size && value)
			memcpy(ptr + m_len_sto, value, m_store_size);
		
		*(var_1**)(ptr + m_len_ptr) = m_table_ptr[idx];
		m_table_ptr[idx] = ptr;
		
		if(reference)
		{
			if(m_store_size == 0)
				*reference = NULL;
			else
			{
				cp_lock_inc((var_u4*)(ptr + m_len_ref));
				*reference = (var_vd*)(ptr + m_len_sto);
			}
		}
		
		unlock_table(idx);
		
		if(is_lock)
			m_global_lck.unlock();
		
		return 0;
	}
	
	var_4 query_key(T_Key key)
	{
		var_8 idx = (var_8)(key % m_table_size);
		if(idx < 0)
			idx *= -1;
		
		lock_table_r(idx);
		
		var_1* ptr = m_table_ptr[idx];
		
		while(ptr)
		{
			if(*(T_Key*)ptr == key)
			{
				unlock_table(idx);
				
				return 0;
			}
			
			ptr = *(var_1**)(ptr + m_len_ptr);
		}
		
		unlock_table(idx);
		
		return -1;
	}
	
	var_4 pop_value(T_Key key, var_vd*& reference)
	{
		var_8 idx = (var_8)(key % m_table_size);
		if(idx < 0)
			idx *= -1;
		
		lock_table_r(idx);
		
		var_1* ptr = m_table_ptr[idx];
		
		while(ptr)
		{
			if(*(T_Key*)ptr == key)
			{
				cp_lock_inc((var_u4*)(ptr + m_len_ref));
				reference = (var_vd*)(ptr + m_len_sto);
				
				unlock_table(idx);
				
				return 0;
			}
			
			ptr = *(var_1**)(ptr + m_len_ptr);
		}
		
		unlock_table(idx);
		
		return -1;
	}
	
	var_vd push_value(var_vd*& reference)
	{
		var_1* ptr = (var_1*)reference - m_len_sto;
		
		if (m_is_delay_recycle == 0)
			cp_lock_dec((var_u4*)(ptr + m_len_ref));
		else
		{
			lock_store(ptr);
			
			cp_lock_dec((var_u4*)(ptr + m_len_ref));
			
			if(*(var_u4*)(ptr + m_len_ref) == 0 && *(ptr + m_len_del) == 1)
			{
				unlock_store(ptr);
				
				m_ar.FreeMem(ptr);
			}
			else
				unlock_store(ptr);
		}
		
		reference = NULL;
	}
	
	// return: failure is -1, success is 0, no exist is 1
	var_4 del(T_Key key, var_4 is_lock = 1)
	{
		if(is_lock)
			m_global_lck.lock_r();
		
		var_8 idx = (var_8)(key % m_table_size);
		if(idx < 0)
			idx *= -1;
		
		lock_table_w(idx);
				
		var_1* ptr = m_table_ptr[idx];
		var_1* pre = ptr;
		
		while(ptr)
		{
			if(*(T_Key*)ptr == key)
				break;
			
			pre = ptr;
			ptr = *(var_1**)(ptr + m_len_ptr);
		}
		
		if(ptr == NULL) // no find
		{
			unlock_table(idx);
			
			if(is_lock)
				m_global_lck.unlock();
			
			return 1;
		}
		
		if(m_is_persistent && is_lock && persistent(key, NULL, 1))
		{
			unlock_table(idx);
			
			m_global_lck.unlock();
			
			return -1;
		}

		// drop node from list
		if (pre == ptr)
			m_table_ptr[idx] = *(var_1**)(ptr + m_len_ptr);
		else
			*(var_1**)(pre + m_len_ptr) = *(var_1**)(ptr + m_len_ptr);
		
		// delete node
		var_u4* counter = (var_u4*)(ptr + m_len_ref);
		
		if(m_is_delay_recycle == 0) // no delay recycle
		{
			while(*counter) // wait counter is zero
				cp_sleep(1);
			
			m_ar.FreeMem(ptr);
		}
		else if(*counter == 0) // delay recycle and counter is zero
		{
			m_ar.FreeMem(ptr);
		}
		else // delay recycle and counter is nonzero
		{
			lock_store(ptr);
			
			if(*counter == 0) // watch counter is zero, again
			{
				unlock_store(ptr);
				
				m_ar.FreeMem(ptr);
			}
			else
			{
				*(ptr + m_len_del) = 1; // set delete flag
				
				unlock_store(ptr);
			}
		}
		
		unlock_table(idx);
		
		m_lck_key_num.lock();
		m_now_key_num--;
		m_lck_key_num.unlock();
		
		if(is_lock)
			m_global_lck.unlock();
		
		return 0;
	}
	
	var_4 num()
	{
		return m_now_key_num;
	}
	
	var_4 clear()
	{
		m_global_lck.lock_w();
		
		if(m_is_persistent)
		{
			FILE* fp = fopen(m_name_flg, "wb");
			if(fp == NULL)
			{
				m_global_lck.unlock();
				return -1;
			}
            fclose(fp);
		}
		
		for(var_4 i = 0; i < m_table_size; i++)
			lock_table_w(i);
		
		for(var_4 i = 0; i < m_table_size; i++)
		{
			var_1* ptr = m_table_ptr[i];
			
			while(ptr)
			{
				while(*(var_u4*)(ptr + m_len_ref))
					cp_sleep(1);
				
				ptr = *(var_1**)(ptr + m_len_ptr);
			}
		}
		
		while(m_is_delay_recycle == 1 && m_ar.GetUseMemNum() != m_now_key_num)
			cp_sleep(1);
		
		m_now_key_num = 0;
		
		memset(m_table_ptr, 0, sizeof(var_1*) * m_table_size);
		
		m_ar.ResetAllocator();
		
		if(m_is_persistent)
		{
			fclose(m_file_inc);
			
			cp_remove_file(m_name_sto);
			cp_remove_file(m_name_inc);
			cp_remove_file(m_name_flg);
			
			m_file_inc = fopen(m_name_inc, "wb");
			while(m_file_inc == NULL)
			{
				printf("UT_HashTable_Pro.clear - create %s failure\n", m_name_inc);
				cp_sleep(5000);
				
				m_file_inc = fopen(m_name_inc, "wb");
			}
		}
		
		for(var_4 i = 0; i < m_table_size; i++)
			unlock_table(i);
		
		m_global_lck.unlock();
		
		return 0;
	}
	
	var_4 travel_prepare(var_vd*& handle, var_4 is_dynamic = 0, var_4 is_lock = 1)
	{
		UT_HASH_TABLE_PRO_TRAVEL_INFO* ti = new UT_HASH_TABLE_PRO_TRAVEL_INFO;
		if(ti == NULL)
			return -1;
		
		if(is_dynamic == 0 && is_lock)
		{
			m_static_travel_lck.lock();
			
			if(m_static_travel == 0)
				m_global_lck.lock_w();
			
			m_static_travel++;
			
			m_static_travel_lck.unlock();
		}
		
		ti->is_dynamic = is_dynamic;
		ti->is_delete = 0;
		ti->is_lock = is_lock;
		ti->travel_idx = -1;
		ti->travel_pos = NULL;
		ti->travel_pre = NULL;
		
		handle = (var_vd*)ti;
		
		return 0;
	}
	
	var_4 travel_key(var_vd* handle, T_Key& key, var_vd** const reference = NULL)
	{
		UT_HASH_TABLE_PRO_TRAVEL_INFO* ti = (UT_HASH_TABLE_PRO_TRAVEL_INFO*)handle;
		
		var_4  travel_idx = ti->travel_idx;
		var_1* travel_pos = ti->travel_pos;
		var_1* travel_pre = ti->travel_pre;
		
		for(;;)
		{
			if(travel_pos == NULL)
			{
				if(travel_idx >= 0)
					unlock_table(travel_idx);
				
				travel_idx++;
				
				if(travel_idx >= m_table_size)
				{
					ti->travel_idx = m_table_size;
					ti->travel_pos = NULL;
					ti->travel_pre = NULL;
					ti->is_delete = 0;
					
					return -1;
				}
				
				lock_table_w(travel_idx);
				
				travel_pos = m_table_ptr[travel_idx];
				travel_pre = m_table_ptr[travel_idx];
				
				if(travel_pos)
				    break;
				
				continue;
			}
			
			if(ti->is_delete == 0)
			{
				travel_pre = travel_pos;
				travel_pos = *(var_1**)(travel_pos + m_len_ptr);
			}
			
			if(travel_pos)
				break;			
		}
		
		key = *(T_Key*)travel_pos;
		
		if(reference)
		{
			if(m_store_size == 0)
				*reference = NULL;
			else
				*reference = (var_vd*)(travel_pos + m_len_sto);
		}
		
		ti->travel_idx = travel_idx;
		ti->travel_pos = travel_pos;
		ti->travel_pre = travel_pre;
		ti->is_delete = 0;
		
		return 0;
	}
	
	var_4 travel_finish(var_vd*& handle)
	{
		UT_HASH_TABLE_PRO_TRAVEL_INFO* ti = (UT_HASH_TABLE_PRO_TRAVEL_INFO*)handle;
		
		if(ti->travel_idx < m_table_size)
			unlock_table(ti->travel_idx);
		
		if(ti->is_dynamic == 0 && ti->is_lock)
		{
			m_static_travel_lck.lock();
			
			m_static_travel--;
			
			if(m_static_travel == 0)
				m_global_lck.unlock();
			
			m_static_travel_lck.unlock();
		}
		
		delete ti;
		handle = NULL;
		
		return 0;
	}
	
	var_4 travel_del(var_vd*& handle)
	{
		UT_HASH_TABLE_PRO_TRAVEL_INFO* ti = (UT_HASH_TABLE_PRO_TRAVEL_INFO*)handle;
		
		var_4  travel_idx = ti->travel_idx;
		var_1* travel_pos = ti->travel_pos;
		var_1* travel_pre = ti->travel_pre;
		
		if(m_is_persistent && persistent(*(T_Key*)travel_pos, NULL, 1))
			return -1;
		
		// drop node from list
		if (travel_pre == travel_pos)
			m_table_ptr[travel_idx] = *(var_1**)(travel_pos + m_len_ptr);
		else
			*(var_1**)(travel_pre + m_len_ptr) = *(var_1**)(travel_pos + m_len_ptr);
		
		// delete node
		var_u4* counter = (var_u4*)(travel_pos + m_len_ref);
		
		if(m_is_delay_recycle == 0) // no delay recycle
		{
			while(*counter) // wait counter is zero
				cp_sleep(1);
			
			m_ar.FreeMem(travel_pos);
		}
		else if(*counter == 0) // delay recycle and counter is zero
		{
			m_ar.FreeMem(travel_pos);
		}
		else // delay recycle and counter is nonzero
		{
			lock_store(travel_pos);
			
			if(*counter == 0) // watch counter is zero, again
			{
				unlock_store(travel_pos);
				
				m_ar.FreeMem(travel_pos);
			}
			else
			{
				*(travel_pos + m_len_del) = 1; // set delete flag
				
				unlock_store(travel_pos);
			}
		}
		
		m_lck_key_num.lock();
		m_now_key_num--;
		m_lck_key_num.unlock();
		
		ti->travel_pos = *(var_1**)(travel_pre + m_len_ptr);
		ti->is_delete = 1;
		
		return 0;
	}
	
	var_4 travel_modify(var_vd*& handle, var_vd* value)
	{
		UT_HASH_TABLE_PRO_TRAVEL_INFO* ti = (UT_HASH_TABLE_PRO_TRAVEL_INFO*)handle;
		
		if(ti->is_delete == 1)
			return -1;
		
		var_4  travel_idx = ti->travel_idx;
		var_1* travel_pos = ti->travel_pos;
		var_1* travel_pre = ti->travel_pre;
		
		if(m_is_persistent && persistent(*(T_Key*)travel_pos, value, 0))
			return -1;
		
		var_u4* counter = (var_u4*)(travel_pos + m_len_ref);
		
		if(m_is_delay_recycle == 0) // no delay recycle
		{
			while(*counter) // wait counter is zero
				cp_sleep(1);
			
			memcpy(travel_pos + m_len_sto, value, m_store_size); // copy data
		}
		else if(*counter == 0) // delay recycle and counter is zero
		{
			memcpy(travel_pos + m_len_sto, value, m_store_size); // copy data
		}
		else // delay recycle and counter is nonzero
		{
			lock_store(travel_pos);
			
			if(*counter == 0) // watch counter is zero, again
				memcpy(travel_pos + m_len_sto, value, m_store_size); // copy data
			else
			{
				// create new node
				var_1* buf = m_ar.AllocMem();
				if(buf == NULL)
				{
					unlock_store(travel_pos);
					return -1;
				}
				
				*(T_Key*)buf = *(T_Key*)travel_pos;
				*(var_u4*)(buf + m_len_ref) = 0;
				*(buf + m_len_del) = 0;
				if(m_store_size && value)
					memcpy(buf + m_len_sto, value, m_store_size);
				
				// insert new node to list and drop old node
				*(var_1**)(buf + m_len_ptr) = *(var_1**)(travel_pos + m_len_ptr);
				
				if(travel_pre == travel_pos)
					m_table_ptr[travel_idx] = buf;
				else
					*(var_1**)(travel_pre + m_len_ptr) = buf;
				
				// delete old node
				*(travel_pos + m_len_del) = 1;
				
				unlock_store(travel_pos);
				
				travel_pos = buf;
			}
		}
		
		ti->travel_pos = travel_pos;
		
		return 0;
	}
	
	var_4 save(var_1* library_file, var_4 is_lock = 1)
	{
		if(is_lock)
			m_global_lck.lock_w();
		
		FILE* fp = fopen(library_file, "wb");
		if(fp == NULL)
		{
			if(is_lock)
				m_global_lck.unlock();
			return -1;
		}
		
		var_4 ret = 0;
		
		try 
		{
			if (fwrite(&m_store_size, 4, 1, fp) != 1)
			{
				throw -2;
			}
			var_vd* handle = NULL;
			
			if(travel_prepare(handle, 0, 0) < 0)
			{
				throw -3;
			}
			T_Key key;
			var_vd* value;
			
			while(travel_key(handle, key, &value) == 0)
			{
				if (fwrite(&key, sizeof(T_Key), 1, fp) != 1)
				{
					throw -4;
				}
				ret = fwrite(value, m_store_size, 1, fp);
				if(m_store_size > 0 && ret != 1)
				{
					throw -5;
				}
			}
			
			travel_finish(handle);
			// if success return 0
			throw 0;
		}
		catch(var_4 err)
		{
			ret = err;
		}
		
		fclose(fp);
		
		if(is_lock)
			m_global_lck.unlock();
		
		return ret;
	}
	
	var_4 load(var_1* library_file, var_4 is_lock = 1)
	{
		if(is_lock)
			m_global_lck.lock_w();
		
		FILE* fp = fopen(library_file, "rb");
		if(fp == NULL)
		{
			if(is_lock)
				m_global_lck.unlock();
			return -1;
		}
		
		var_4  ret = 0;
		T_Key  key;
		var_1* buf = NULL;
		
		try
		{
			var_4 store_size = 0;
			if(fread(&store_size, 4, 1, fp) != 1)
				throw -2;
			
			if(store_size != m_store_size)
				throw -3;
			
			var_8 file_size = cp_get_file_size(library_file);
			if(file_size < 0)
				throw -4;
			
			file_size -= 4;
			if(file_size % (sizeof(T_Key) + store_size))
				throw -5;
			
			file_size /= sizeof(T_Key) + store_size;
			
			if(store_size)
			{
				buf = new var_1[store_size];
				if(buf == NULL)
					throw -6;
			}
			
			for(var_8 i = 0; i < file_size; i++)
			{
				if(fread(&key, sizeof(T_Key), 1, fp) != 1)
					throw -7;
				if(store_size && fread(buf, store_size, 1, fp) != 1)
					throw -8;
				
				if(add(key, buf, NULL, 0, 0))
					throw -9;
			}
		}
		catch(var_4 err)
		{
			if(buf)
				delete buf;
			
			ret = err;
		}
				
		fclose(fp);
		
		if(is_lock)
			m_global_lck.unlock();
		
		return ret;
	}
	
	var_4 trim()
	{
		var_1 file_tmp[256];
		sprintf(file_tmp, "%s.tmp", m_name_sto);
		
		var_1 file_new[256];
		sprintf(file_new, "%s.new", m_name_sto);
		
		m_global_lck.lock_w();
		
		if(save(file_tmp, 0))
		{
			m_global_lck.unlock();
			return -1;
		}
		
		fclose(m_file_inc);
		
		while(cp_rename_file(file_tmp, file_new))
		{
			printf("UT_HashTable_Pro.trim - cp_rename_file(%s, %s) failure\n", file_tmp, file_new);
			cp_sleep(5000);
		}
		
		cp_remove_file(m_name_sto);
		cp_remove_file(m_name_inc);
		
		cp_rename_file(file_new, m_name_sto);
		
		m_file_inc = fopen(m_name_inc, "wb");
		while(m_file_inc == NULL)
		{
			printf("UT_HashTable_Pro.trim - create %s failure\n", m_name_inc);
			cp_sleep(5000);
			
			m_file_inc = fopen(m_name_inc, "wb");
		}
		
		m_global_lck.unlock();
		
		return 0;
	}
	
	const var_1* version()
	{
		// v1.000 - 2013.05.21 - 初始版本
		return "v1.000";
	}
	
private:
	inline var_vd lock_table_r(var_8 idx)
	{
		var_4 lock_no = idx % m_table_lck_num;
		
		for(;;)
		{
			m_table_lck[lock_no].lock();
			
			if(m_table_flg[idx] >= 0)
				break;
			
			m_table_lck[lock_no].unlock();
			
			cp_sleep(1);
		}
		
		m_table_flg[idx]++;
		
		m_table_lck[lock_no].unlock();
	}
	
	inline var_vd lock_table_w(var_8 idx)
	{
		var_4 lock_no = idx % m_table_lck_num;
		
		for(;;)
		{
			m_table_lck[lock_no].lock();
			
			if(m_table_flg[idx] == 0)
				break;
			
			m_table_lck[lock_no].unlock();
			
			cp_sleep(1);
		}
		
		m_table_flg[idx] = -1;
		
		m_table_lck[lock_no].unlock();
	}
	
	inline var_vd unlock_table(var_8 idx)
	{
		var_4 lock_no = idx % m_table_lck_num;
		
		m_table_lck[lock_no].lock();
		
		if(m_table_flg[idx] < 0)
			m_table_flg[idx] = 0;
		else
			m_table_flg[idx]--;
		
		m_table_lck[lock_no].unlock();
	}
	
	inline var_vd lock_store(var_1* head)
	{
		var_4  lock_no = *(T_Key*)head % m_store_lck_num;
		var_1* lck_ptr = (var_1*)(head + m_len_lck);
		
		for(;;)
		{
			m_store_lck[lock_no].lock();
			if(*lck_ptr == 0)
				break;
			
			m_store_lck[lock_no].unlock();
			cp_sleep(1);
		}
		
		*lck_ptr = 1;
		
		m_store_lck[lock_no].unlock();
	}
	
	inline var_vd unlock_store(var_1* head)
	{
		*(var_1*)(head + m_len_lck) = 0;
	}
	
	var_4 restore()
	{
		if(access(m_name_flg, 0) == 0) // clear
		{
			cp_remove_file(m_name_sto);
			cp_remove_file(m_name_inc);
			cp_remove_file(m_name_flg);
		}
		
		var_1 file_new[256];
		sprintf(file_new, "%s.new", m_name_sto);
		
		if(access(file_new, 0) == 0)
		{
			cp_remove_file(m_name_sto);
			cp_remove_file(m_name_inc);
			
			cp_rename_file(file_new, m_name_sto);
		}
		
		// load lib file
		if(access(m_name_sto, 0) == 0 && load(m_name_sto))
			return -1;
		
		// load inc file
		if(access(m_name_inc, 0) == 0)
		{
			var_4  len = 8 + sizeof(T_Key) + m_store_size;
			var_1* buf = new var_1[len];
			if(buf == NULL)
				return -1;
			
			var_8 cur_size = 0;
			var_8 all_size = cp_get_file_size(m_name_inc);
			if(all_size < 0)
				return -1;
			
			m_file_inc = fopen(m_name_inc, "rb");
			if(m_file_inc == NULL)
				return -1;
						
			while(cur_size < all_size)
			{
				if(fread(buf, 8 + sizeof(T_Key), 1, m_file_inc) != 1)
					break;
				
				if(*(var_u8*)buf == (var_u8)0xABABABABABABABAB)
				{
					var_4 ret = fread(buf + 8 + sizeof(T_Key), m_store_size, 1, m_file_inc);
					if (m_store_size > 0 && ret != 1)
					{
						break;
					}
					if(add(*(T_Key*)(buf + 8), buf + 8 + sizeof(T_Key), NULL, 1, 0) < 0)
					{
						return -1;
					}
					cur_size += 8 + sizeof(T_Key) + m_store_size;
				}
				else if(*(var_u8*)buf == (var_u8)0xCDCDCDCDCDCDCDCD)
				{
					if(del(*(T_Key*)(buf + 8), 0))
						return -1;
					
					cur_size += 8 + sizeof(T_Key);
				}
				else
				{
					return -1;
				}
			}
						
			fclose(m_file_inc);
			
			if(cur_size < all_size)
				printf("UT_HashTable_Pro - %s size error (all_size = " CP_P64 ", cur_size = " CP_P64 ", min_size = " CP_P64"), fix ok\n", m_name_inc, all_size, cur_size, 8 + sizeof(T_Key));
			
			m_file_inc = fopen(m_name_inc, "rb+");
			if(m_file_inc == NULL)
				return -1;
			
			if(fseek(m_file_inc, cur_size, SEEK_CUR))
				return -1;
		}
		else
		{
			m_file_inc = fopen(m_name_inc, "wb");
			if(m_file_inc == NULL)
				return -1;
		}
		
		return 0;
	}
	
	var_4 persistent(T_Key key, var_vd* value, var_4 is_delete)
	{
		m_persistent_lck.lock();
		
		var_8 size = ftell(m_file_inc);
		var_4 ret  = 0;	
		try
		{
			var_u8 flag = 0;
			
			if(is_delete == 0)
				flag = 0xABABABABABABABAB;
			else
				flag = 0xCDCDCDCDCDCDCDCD;
			
			if (fwrite(&flag, 8, 1, m_file_inc) != 1)
			{
				throw -1;
			}
			if (fwrite(&key, sizeof(T_Key), 1, m_file_inc) != 1)
			{
				throw -2;
			}
			if(is_delete == 0)
			{
				ret = fwrite(value, m_store_size, 1, m_file_inc); 
				if (m_store_size > 0 && ret != 1)
				{
					throw -3;
				}
			}
			if(fflush(m_file_inc))
			{
				throw -4;
			}
		}
		catch (var_4 err)
		{
			printf("UT_HashTable_Pro.persistent - write failure, code = %d\n", err);
			
			while(cp_change_file_size(m_file_inc, size))
			{
				printf("UT_HashTable_Pro.persistent - cp_change_file_size failure\n");
				cp_sleep(5000);
			}
			
			while(fflush(m_file_inc))
			{
				printf("UT_HashTable_Pro.persistent - fflush failure\n");
				cp_sleep(5000);
			}
			
			m_persistent_lck.unlock();
			
			return -1;
		}
		
		m_persistent_lck.unlock();
		
		return 0;
	}
public:
	//var_4 get_size()
	//{
	//	return m_now_key_num;
	//}

private:
	var_8 m_table_size;
	var_4 m_store_size;
	
	var_1 m_name_sto[256];
	var_1 m_name_inc[256];
	var_1 m_name_flg[256];
	
	var_u8       m_now_key_num;
	var_u8       m_max_key_num;
	CP_MUTEXLOCK m_lck_key_num;
	
	var_4 m_is_persistent;
	var_4 m_is_delay_recycle;
	
	var_4 m_len_ptr;
	var_4 m_len_lck;
	var_4 m_len_ref;
	var_4 m_len_del;
	var_4 m_len_sto;
	var_4 m_len_mem;
	
	var_1** m_table_ptr;
	var_4*  m_table_flg;
	
	CP_MUTEXLOCK* m_table_lck;
	var_4         m_table_lck_num;
	
	CP_MUTEXLOCK* m_store_lck;
	var_4         m_store_lck_num;
	
	CP_MUTEXLOCK_RW m_global_lck; // for clear, save, trim
	CP_MUTEXLOCK    m_persistent_lck;
	
	UC_Allocator_Recycle m_ar;
	
	FILE* m_file_inc;
	
	var_4        m_static_travel;
	CP_MUTEXLOCK m_static_travel_lck;
};

#endif // __UT_HASH_TABLE_PRO_H__
