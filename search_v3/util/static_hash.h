/*
*by liugang 2011.06.28
*/

#ifndef _STATIC_HASH_
#define _STATIC_HASH_

#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifndef _WIN32
  #include <unistd.h>
#endif

using namespace std;

#define num_to_bucket_id(num, buck_num) (num & (buck_num - 1)) 

// JSHash Hash Function   may be not suitable for integer
static unsigned int js_hash(const char *str, unsigned int len)
{
	unsigned int hash = 1315423911;
	while (len)
	{
		hash ^= ((hash << 5) + (*str++) + (hash >> 2));
		--len;
	}

	return (hash & 0x7FFFFFFF);
}

// BKDR Hash Function   may be not suitable for integer
/*
unsigned int bkdr_hash(const char *str, unsigned int len)
{
	unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
	unsigned int hash = 0;

	while (len != 0)
	{
		hash = hash * seed + (*str++);
		--len;
	}
	return (hash & 0x7FFFFFFF);
}
*/

template <class Kty>
inline unsigned int hash_wrap(const Kty& k)
{
	return js_hash((const char*)&k, sizeof(k));
}

template <>
inline unsigned int hash_wrap<string>(const string & k)
{
	return js_hash(k.c_str(), k.length());
}

template <class Kty>
inline unsigned int get_size(const Kty& k)
{
	return sizeof(Kty);
}

template <>
inline unsigned int get_size(const string& k)
{
	return k.length() + 1;
}

template <class Kty>
inline void write_data(const Kty& k, char* dest)
{
	memcpy(dest,(void*)(&k), sizeof(k));
}

template <>
inline void write_data(const string& k, char* dest)
{
	memcpy(dest, k.c_str(), k.length()+1);
}

template <class Kty>
inline bool cmp_key(const Kty& k, const char* dest)
{
	return k == *(Kty* )dest;;
}

template <>
inline bool cmp_key(const string& k, const char* dest)
{
	return strcmp(k.c_str(), dest) == 0;
}

template <class Kty>
inline void get_value(Kty& v,const char* dest)
{
	v = *(Kty* )dest;
}

template <>
inline void get_value(string& v, const char* dest)
{
	v.assign(dest);
}


struct hasher_s 
{
	unsigned int hasher;
	unsigned int data_off;
	bool operator<(const hasher_s& r)const
	{
		return this->hasher < r.hasher||
			this->hasher == r.hasher && this->data_off < r.data_off;
	}
};


struct tmp_hasher_s 
{
	unsigned int hasher;
	unsigned int next;
	unsigned int data_off;
};

struct table_header_s
{
	unsigned int bucket_num;
	unsigned int element_cnt;
	unsigned int data_size;
// 	short key_type;
// 	short val_type;
};




// the implement of serialized static hash
template <class Kty,class Vty>
class static_hash_map
{
public:


	bool load_serialized_hash_file(const char* file, const Vty& invalid_val);

	Vty operator[](const Kty& k);

	template<class C>
	bool container_to_hash_file(C& dynamic_container, unsigned int bucket_pow, const char* file)
	{
		if (bucket_pow < 10)
		{
			bucket_pow = 10;
		}
		else if (bucket_pow > 25) //32000000+
		{
			bucket_pow = 25;
		}
		unsigned int bucket_num = (1<<bucket_pow);

		//checking length of all data
		unsigned int len = 0;
		unsigned int data_len = 0;
		vector<pair<Kty, Vty> > vec(dynamic_container.begin(), dynamic_container.end());
		for (size_t i = 0; i != vec.size(); ++i)
		{
			data_len += get_size(vec[i].first);
			data_len += get_size(vec[i].second);
		}

		unsigned int total_len = sizeof(table_header_s) + bucket_num*sizeof(unsigned int) + (vec.size()+2)*sizeof(hasher_s) + data_len;
		char* ptr_table = new char[total_len];
		tmp_hasher_s* p_tmp_hasher = new tmp_hasher_s[vec.size() + 1];
		if (!ptr_table || !p_tmp_hasher)
		{
			return false;
		}

		unsigned int id; //bucket id
		unsigned int* p_bucket = (unsigned int*)(ptr_table + sizeof(table_header_s));
		hasher_s* p_hasher = (hasher_s*)(p_bucket + bucket_num);
		char* p_data = (char*)(p_hasher + vec.size() + 2);

		table_header_s sh;
		sh.bucket_num = bucket_num;
		sh.data_size = data_len;
		sh.element_cnt = 0;


		unsigned int last_off;
		unsigned int kv_size;
		memset(p_bucket, 0, sizeof(unsigned int)*bucket_num);
		for (size_t i = 0; i != vec.size(); ++i)
		{
			++sh.element_cnt;  //0 position reserve
			p_tmp_hasher[sh.element_cnt].hasher = hash_wrap(vec[i].first);
			p_tmp_hasher[sh.element_cnt].next = 0;
			p_tmp_hasher[sh.element_cnt].data_off = len;
			id = num_to_bucket_id(p_tmp_hasher[sh.element_cnt].hasher, bucket_num);

			if (p_bucket[id] == 0) //not assign
			{
				p_bucket[id] = sh.element_cnt;
			}
			else	
			{
				last_off = p_bucket[id];
				while (p_tmp_hasher[last_off].next > 0)
				{
					last_off = p_tmp_hasher[last_off].next;
				}
				p_tmp_hasher[last_off].next = sh.element_cnt;
			}

			kv_size = get_size(vec[i].first);
			write_data(vec[i].first, p_data + len);
			write_data(vec[i].second, p_data + len + kv_size);
			kv_size += get_size(vec[i].second);
			len += kv_size;
		}

		memcpy(ptr_table, &sh, sizeof(sh));
		
		//optimize storage
		unsigned int cur_id = 1;
		unsigned int old_id = 1;
		tmp_hasher_s *p_tmp_node;
		for (size_t i = 0; i != bucket_num; ++i)
		{
			if( p_bucket[i] != 0)
			{
				p_tmp_node = p_tmp_hasher + p_bucket[i];
				do
				{
					p_hasher[cur_id].hasher = p_tmp_node->hasher;
					p_hasher[cur_id].data_off = p_tmp_node->data_off;
					++cur_id;
					p_tmp_node = p_tmp_hasher + p_tmp_node->next;
				} while (p_tmp_node != p_tmp_hasher);
				

				if(cur_id - old_id > 1)
					sort(p_hasher + old_id, p_hasher + cur_id);

				p_bucket[i] = old_id;
				old_id = cur_id;
			}
		}
		
		p_hasher[cur_id].hasher = 0x80000000; // the additional one end mark
		p_hasher[cur_id].data_off = 0;

		assert(cur_id == vec.size()+1);
		

		FILE* fp = fopen(file, "wb");
		if (!fp || fwrite(ptr_table, 1, total_len, fp) != total_len)
		{
			delete ptr_table;
			delete p_tmp_hasher;
			return false;
		}
		fclose(fp);
		delete ptr_table;
		delete p_tmp_hasher;

		return true;
	}

		
	inline unsigned int size()
	{
		return m_size;
	}

	static_hash_map()
	{
		m_ptr_hashtable = NULL;
		m_ptr_bucket = NULL;
		m_invalid_val = Vty();
		m_size = 0;
	}

	~static_hash_map()
	{
		delete m_ptr_hashtable;
	}

private:

	const unsigned int*   m_ptr_bucket;
	const hasher_s*       m_ptr_hasher;
	const char*           m_ptr_data;
	const table_header_s* m_ptr_table_header;

	char* m_ptr_hashtable;
	Vty   m_invalid_val;
	unsigned int   m_size;
};


template <class Kty, class Vty>
bool static_hash_map<Kty, Vty>::load_serialized_hash_file(const char* file, const Vty& invalid_val)
{
	m_invalid_val = invalid_val;
	FILE* fp = fopen(file, "rb");
	if (!fp)
	{
		return false;
	}

	size_t sz;
	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	m_ptr_hashtable = new char[sz];
	if (!m_ptr_hashtable)
	{
		return false;
	}

	if (sz != fread(m_ptr_hashtable, 1, sz, fp))
	{
		delete m_ptr_hashtable;
		return false;
	}
	fclose(fp);

	m_ptr_table_header = (table_header_s*)m_ptr_hashtable;
	m_size = m_ptr_table_header->element_cnt;

	m_ptr_bucket = (unsigned int*)(m_ptr_hashtable + sizeof(table_header_s));
	m_ptr_hasher = (hasher_s*)(m_ptr_bucket + m_ptr_table_header->bucket_num);
	m_ptr_data = (char*)(m_ptr_hasher + m_ptr_table_header->element_cnt + 2);
}



template <class Kty,class Vty>
Vty static_hash_map<Kty, Vty>::operator[](const Kty& k)
{
	if (!m_ptr_bucket)
	{
		return m_invalid_val;
	}

	unsigned int hasher = hash_wrap(k);
	unsigned int id = num_to_bucket_id(hasher, m_ptr_table_header->bucket_num);
	if (m_ptr_bucket[id] != 0)
	{
		const register hasher_s* p_hasher = m_ptr_hasher + m_ptr_bucket[id];
		if (p_hasher->hasher == hasher && cmp_key(k, m_ptr_data + p_hasher->data_off))
		{
			Vty v;
			get_value(v, m_ptr_data + p_hasher->data_off + get_size(k));
			return v;
		}
		else
		{
			++p_hasher;
		}

		while (num_to_bucket_id(p_hasher->hasher, m_ptr_table_header->bucket_num) == id &&
				p_hasher->hasher < hasher)
		{
			++p_hasher;
		}

		while (p_hasher->hasher == hasher && !cmp_key(k, m_ptr_data + p_hasher->data_off))
		{
			++p_hasher;
		}

		if (p_hasher->hasher == hasher )
		{
			Vty v;
			get_value(v, m_ptr_data + p_hasher->data_off + get_size(k));
			return v;
		}
	}
	return m_invalid_val;
}

#endif
