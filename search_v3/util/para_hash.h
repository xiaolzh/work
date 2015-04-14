#include <vector>
using std::vector;

#define PARA_GET_BUCK_NUM(num, buck_num) ((num) & ((buck_num) - 1)) 

struct value_node_s
{
	unsigned int value_id;
	unsigned int value_cnt;
};


struct para_out_s
{
	unsigned int name_id;
	unsigned int value_id;
	unsigned int cnt;
};
inline bool CMP_PARA_BY_CNT(const para_out_s& l, const para_out_s& r)
{
	return l.cnt > r.cnt;
};

struct chain_node_s
{
	unsigned int name_id;
	unsigned short chain_off;
	unsigned short value_array_id;

};


//there is a limit for para_name_cnt 10000 ,limit for para_value 
class para_hash_c
{
	enum {VALUE_SLOT = 32, PARA_NAME_LIMIT = 8192};
	
public:
	para_hash_c()
	{
		m_p_buck = NULL;
		m_p_chain = NULL;
		m_p_vnode = NULL;
		m_name_cnt = 0;
	}

	bool init(int buck_pow, int para_name_limit)
	{
		if (buck_pow < 10)  
			buck_pow = 10;
		if (buck_pow > 16) 
			buck_pow = 16;

		m_buck_num = (1 << buck_pow);
		m_p_buck = new int [m_buck_num];
		if (!m_p_buck) 
			return false;
		memset(m_p_buck, 0, sizeof(int) * m_buck_num);

		if (para_name_limit > PARA_NAME_LIMIT)	
			para_name_limit = PARA_NAME_LIMIT;
		if (para_name_limit < 1) 
			para_name_limit = 1;
		m_pname_limit = para_name_limit;
		m_p_chain = new chain_node_s [m_pname_limit + 1];
		if (!m_p_chain)
			return false;
		//memset(m_p_chain, 0, m_pname_limit * sizeof(chain_node_s));

		m_p_vnode = new value_node_s [m_pname_limit * VALUE_SLOT];
		if (!m_p_vnode)
			return false;
		memset(m_p_vnode, 0, m_pname_limit * sizeof(value_node_s) * VALUE_SLOT);
		return true;
	}

	bool add_para(int key, int val)
	{
		int bpos = PARA_GET_BUCK_NUM(key, m_buck_num);

		int coff = m_p_buck[bpos];
		value_node_s* pv  = NULL;
		chain_node_s* pn;
		int add_chain_node = 0;
		if (coff)
		{
			pn = m_p_chain + coff;
			while (pn != m_p_chain && pn->name_id != key)
			{
				pn = m_p_chain + pn->chain_off;
			}

			if (pn != m_p_chain) //find
			{
				pv = m_p_vnode + (int)pn->value_array_id * VALUE_SLOT ;
			}
			else if (m_name_cnt < m_pname_limit)
			{
				add_chain_node = 1;
			}
		}
		else if (m_name_cnt < m_pname_limit)
		{
			add_chain_node = 1;
		}

		if (add_chain_node)
		{
			pn = m_p_chain + m_name_cnt + 1;
			pn->chain_off = coff;
			pn->name_id = key;
			pn->value_array_id = m_name_cnt;
			++m_name_cnt;//add one pname;
			m_p_buck[bpos] = m_name_cnt;
			pv = m_p_vnode + (int)pn->value_array_id * VALUE_SLOT ;
		}

		if (!pv)// no space left
			return false;
		
		unsigned int vpos = val % VALUE_SLOT;
		if (pv[vpos].value_id  == val)
		{
			++pv[vpos].value_cnt;
		}
		else if (pv[vpos].value_id == 0)
		{
			pv[vpos].value_cnt = 1;
			pv[vpos].value_id = val;

		}
		else //collision
		{
			unsigned int i = (vpos + 1)%VALUE_SLOT;
			while(i!= vpos && pv[i].value_id && pv[i].value_id != val)
			{
				++i;
				i%=VALUE_SLOT;
			}
			if (i != vpos) //find 
			{
				pv[i].value_id = val;
				++pv[i].value_cnt;
			}
			else //para_val exceed 32;
				return false;
		}
		
		return true;
	}

	void optimize()
	{
		value_node_s* pv;
		chain_node_s* pn;
		int i,j,k;
		for (i = 0; i < m_buck_num; ++i)
		{
			pn = m_p_chain + m_p_buck[i];
			while(pn != m_p_chain)
			{
				pv = m_p_vnode + (int)pn->value_array_id * VALUE_SLOT ;
				k = 0;
				for (j = 0; j < VALUE_SLOT; ++j)
				{
					if (pv[j].value_id)
					{
						pv[k++]=pv[j];
					}
				}
				if (k < VALUE_SLOT)
				{
					pv[k].value_id = 0;
				}
				pn = m_p_chain + pn->chain_off;
			}
		}
	}

	void transfer_to_vector(vector<para_out_s>& vec)
	{
		value_node_s* pv;
		chain_node_s* pn;
		para_out_s pos;
		int i,j,k;
		vec.reserve(m_pname_limit);

		for (i = 0; i < m_buck_num; ++i)
		{
			pn = m_p_chain + m_p_buck[i];
			while(pn != m_p_chain)
			{
				pv = m_p_vnode + (int)pn->value_array_id * VALUE_SLOT ;
				for (j = 0; j < VALUE_SLOT; ++j)
				{
					if (pv[j].value_id)
					{
						pos.name_id = pn->name_id;
						pos.value_id = pv[j].value_id;
						pos.cnt = pv[j].value_cnt;
						vec.push_back(pos);
					}
				}
				pn = m_p_chain + pn->chain_off;
			}
		}
	}

	

	~para_hash_c()
	{
		delete[] m_p_buck;
		delete[] m_p_chain;
		delete[] m_p_vnode; 
	}


	int* m_p_buck;
	chain_node_s* m_p_chain;
	value_node_s* m_p_vnode;

	int m_buck_num;
	int m_pname_limit;
	int m_name_cnt;

};