/*
*by liugang 2011.12.20
*vector only push_back and get 
*one push multi get lockfree
*/


#ifndef  ARRAY_RP_NOLOCK_H
#define  ARRAY_RP_NOLOCK_H

#define ELE_PTR(data, n, ssz, pow) (m_data[n>>pow] + (n & (ssz-1)))

template<class T>
class array_rp_nolock
{
public:
	array_rp_nolock():POW(18),SLICE_SZ(1<<18),MAX_SLICE(16384)
	{
		m_size = 0; 
		m_data = new T*[MAX_SLICE];
		memset(m_data, 0, sizeof(T*)*MAX_SLICE);
	}
	~array_rp_nolock()
	{
		for (int i = 0; i < MAX_SLICE; ++i)
		{
			delete[] m_data[i];
		}
		delete[] m_data;
	}

	T* operator[](int n)
	{
		return ELE_PTR(m_data, n, SLICE_SZ, POW);
	}

	T* at(int n)//with check,
	{
		if (n < m_size)
			return ELE_PTR(m_data, n, SLICE_SZ, POW);
		else
			return NULL;
	}
	inline int size(){return m_size;}

	bool push_back(const T& t)
	{
		if (m_size & (SLICE_SZ - 1))
		{
		}
		else// expend
		{
			if ((m_size>>POW) >=MAX_SLICE)
			{
				return false;
			}
			m_data[m_size>>POW] = new T[SLICE_SZ];
		}
		*ELE_PTR(m_data, m_size, SLICE_SZ, POW)= t;
		++m_size;
		return true;
	}

protected:
private:
	int m_size;
	T** m_data;
	const int SLICE_SZ;
	const int MAX_SLICE;
	const int POW;
};
#endif
