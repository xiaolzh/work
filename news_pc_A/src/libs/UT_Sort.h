// UT_Sort.h

#ifndef _UT_SORT_H_
#define _UT_SORT_H_

#include "UH_Define.h"
#include "UT_Queue.h"

/************************************************************************/
// compare key 比较函数
/************************************************************************/
template <class T_Key_1, class T_Key_2>
var_4 compare_key(T_Key_1& key_1_1, T_Key_1& key_1_2, T_Key_2& key_2_1, T_Key_2& key_2_2)
{
	if(key_1_1 > key_1_2)
		return 1;
	if(key_1_1 < key_1_2)
		return -1;
	
	if(key_2_1 > key_2_2)
		return 1;
	if(key_2_1 < key_2_2)
		return -1;

	return 0;
}

template <class T_Key_1, class T_Key_2, class T_Key_3>
var_4 compare_key(T_Key_1& key_1_1, T_Key_1& key_1_2, T_Key_2& key_2_1, T_Key_2& key_2_2, T_Key_3& key_3_1, T_Key_3& key_3_2)
{
	if(key_1_1 > key_1_2)
		return 1;
	if(key_1_1 < key_1_2)
		return -1;

	if(key_2_1 > key_2_2)
		return 1;
	if(key_2_1 < key_2_2)
		return -1;

	if(key_3_1 > key_3_2)
		return 1;
	if(key_3_1 < key_3_2)
		return -1;

	return 0;
}

/************************************************************************/
// quick sort 快速排序                                                        
/************************************************************************/
template <class T_Key_1, class T_Key_2, class T_Key_3, class T_Key_4, class T_Key_5>
var_4 qs_recursion(var_4 lBegin, var_4 lEnd, T_Key_1* tKey_1, T_Key_2* tKey_2, T_Key_3* tKey_3, T_Key_4* tKey_4, T_Key_5* tKey_5)
{
	if(lBegin >= lEnd)
		return 0;

	T_Key_1 tTmp_1;
	T_Key_2 tTmp_2;
	T_Key_3 tTmp_3;
	T_Key_4 tTmp_4;
	T_Key_5 tTmp_5;

	if(lBegin + 1 == lEnd)
	{
		if(compare_key<T_Key_1, T_Key_2, T_Key_3>(tKey_1[lBegin], tKey_1[lEnd], tKey_2[lBegin], tKey_2[lEnd], tKey_3[lBegin], tKey_3[lEnd]) > 0)
		{
			tTmp_1 = tKey_1[lBegin];
			tKey_1[lBegin] = tKey_1[lEnd];
			tKey_1[lEnd] = tTmp_1;

			tTmp_2 = tKey_2[lBegin];
			tKey_2[lBegin] = tKey_2[lEnd];
			tKey_2[lEnd] = tTmp_2;

			tTmp_3 = tKey_3[lBegin];
			tKey_3[lBegin] = tKey_3[lEnd];
			tKey_3[lEnd] = tTmp_3;

			tTmp_4 = tKey_4[lBegin];
			tKey_4[lBegin] = tKey_4[lEnd];
			tKey_4[lEnd] = tTmp_4;

			tTmp_5 = tKey_5[lBegin];
			tKey_5[lBegin] = tKey_5[lEnd];
			tKey_5[lEnd] = tTmp_5;
		}
		return 0;
	}

	var_4 lMid = (lBegin + lEnd)>>1;
	var_4 m = lBegin, n = lEnd;

	T_Key_1 tMid_1 = tKey_1[lMid];
	T_Key_2 tMid_2 = tKey_2[lMid];
	T_Key_3 tMid_3 = tKey_3[lMid];

	while(lBegin < lEnd)
	{
		while(lBegin < lEnd && compare_key<T_Key_1, T_Key_2, T_Key_3>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2, tKey_3[lBegin], tMid_3) < 0) lBegin++;
		while(lBegin < lEnd && compare_key<T_Key_1, T_Key_2, T_Key_3>(tKey_1[lEnd], tMid_1, tKey_2[lEnd], tMid_2, tKey_3[lEnd], tMid_3) > 0) lEnd--;
		if(lBegin < lEnd)
		{
			tTmp_1 = tKey_1[lBegin];
			tKey_1[lBegin] = tKey_1[lEnd];
			tKey_1[lEnd] = tTmp_1;

			tTmp_2 = tKey_2[lBegin];
			tKey_2[lBegin] = tKey_2[lEnd];
			tKey_2[lEnd] = tTmp_2;

			tTmp_3 = tKey_3[lBegin];
			tKey_3[lBegin] = tKey_3[lEnd];
			tKey_3[lEnd] = tTmp_3;

			tTmp_4 = tKey_4[lBegin];
			tKey_4[lBegin] = tKey_4[lEnd];
			tKey_4[lEnd] = tTmp_4;

			tTmp_5 = tKey_5[lBegin];
			tKey_5[lBegin] = tKey_5[lEnd];
			tKey_5[lEnd] = tTmp_5;

			if(++lBegin < lEnd)
				lEnd--;
		}
	}

	if(compare_key<T_Key_1, T_Key_2, T_Key_3>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2, tKey_3[lBegin], tMid_3) < 0)
		lBegin++;

	if(lBegin - 1 > m)
		qs_recursion(m, lBegin - 1, tKey_1, tKey_2, tKey_3, tKey_4, tKey_5);
	if(lEnd < n)
		qs_recursion(lEnd, n, tKey_1, tKey_2, tKey_3, tKey_4, tKey_5);

	return 0;
};

template <class T_Key_1, class T_Key_2, class T_Key_3, class T_Key_4>
var_4 qs_unrecursion(var_4 lBegin, var_4 lEnd, T_Key_1* tKey_1, T_Key_2* tKey_2, T_Key_3* tKey_3, T_Key_4* tKey_4)
{
	var_4 stack_buf[1024];
	var_4 stack_pos = 0;

	stack_buf[stack_pos++] = lBegin;
	stack_buf[stack_pos++] = lEnd;

	for(;;)
	{
		if(stack_pos == 0)
			break;

		lEnd = stack_buf[--stack_pos];
		lBegin = stack_buf[--stack_pos];

		if(lBegin >= lEnd)
			continue;

		T_Key_1 tTmp_1;
		T_Key_2 tTmp_2;
		T_Key_3 tTmp_3;
		T_Key_4 tTmp_4;

		if(lBegin + 1 == lEnd)
		{
			if(compare_key<T_Key_1, T_Key_2, T_Key_3>(tKey_1[lBegin], tKey_1[lEnd], tKey_2[lBegin], tKey_2[lEnd], tKey_3[lBegin], tKey_3[lEnd]) > 0)
			{
				tTmp_1 = tKey_1[lBegin];
				tKey_1[lBegin] = tKey_1[lEnd];
				tKey_1[lEnd] = tTmp_1;

				tTmp_2 = tKey_2[lBegin];
				tKey_2[lBegin] = tKey_2[lEnd];
				tKey_2[lEnd] = tTmp_2;

				tTmp_3 = tKey_3[lBegin];
				tKey_3[lBegin] = tKey_3[lEnd];
				tKey_3[lEnd] = tTmp_3;

				tTmp_4 = tKey_4[lBegin];
				tKey_4[lBegin] = tKey_4[lEnd];
				tKey_4[lEnd] = tTmp_4;
			}
			continue;
		}

		var_4 lMid = (lBegin + lEnd)>>1;
		var_4 m = lBegin, n = lEnd;

		T_Key_1 tMid_1 = tKey_1[lMid];
		T_Key_2 tMid_2 = tKey_2[lMid];
		T_Key_3 tMid_3 = tKey_3[lMid];

		while(lBegin < lEnd)
		{
			while(lBegin < lEnd && compare_key<T_Key_1, T_Key_2, T_Key_3>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2, tKey_3[lBegin], tMid_3) < 0) lBegin++;
			while(lBegin < lEnd && compare_key<T_Key_1, T_Key_2, T_Key_3>(tKey_1[lEnd], tMid_1, tKey_2[lEnd], tMid_2, tKey_3[lEnd], tMid_3) > 0) lEnd--;
			if(lBegin < lEnd)
			{
				tTmp_1 = tKey_1[lBegin];
				tKey_1[lBegin] = tKey_1[lEnd];
				tKey_1[lEnd] = tTmp_1;

				tTmp_2 = tKey_2[lBegin];
				tKey_2[lBegin] = tKey_2[lEnd];
				tKey_2[lEnd] = tTmp_2;

				tTmp_3 = tKey_3[lBegin];
				tKey_3[lBegin] = tKey_3[lEnd];
				tKey_3[lEnd] = tTmp_3;

				tTmp_4 = tKey_4[lBegin];
				tKey_4[lBegin] = tKey_4[lEnd];
				tKey_4[lEnd] = tTmp_4;

				if(++lBegin < lEnd)
					lEnd--;
			}
		}

		if(compare_key<T_Key_1, T_Key_2, T_Key_3>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2, tKey_3[lBegin], tMid_3) < 0)
			lBegin++;

		if(lEnd < n)
		{
			stack_buf[stack_pos++] = lEnd;
			stack_buf[stack_pos++] = n;
		}
		if(lBegin - 1 > m)
		{
			stack_buf[stack_pos++] = m;
			stack_buf[stack_pos++] = lBegin - 1;
		}
	}	

	return 0;
};

template <class T_Key_1, class T_Key_2, class T_Key_3, class T_Key_4, class T_Key_5>
var_4 qs_unrecursion(var_4 lBegin, var_4 lEnd, T_Key_1* tKey_1, T_Key_2* tKey_2, T_Key_3* tKey_3, T_Key_4* tKey_4, T_Key_5* tKey_5)
{
	var_4 stack_buf[1024];
	var_4 stack_pos = 0;

	stack_buf[stack_pos++] = lBegin;
	stack_buf[stack_pos++] = lEnd;

	for(;;)
	{
		if(stack_pos == 0)
			break;

		lEnd = stack_buf[--stack_pos];
		lBegin = stack_buf[--stack_pos];

		if(lBegin >= lEnd)
			continue;

		T_Key_1 tTmp_1;
		T_Key_2 tTmp_2;
		T_Key_3 tTmp_3;
		T_Key_4 tTmp_4;
		T_Key_5 tTmp_5;

		if(lBegin + 1 == lEnd)
		{
			if(compare_key<T_Key_1, T_Key_2, T_Key_3>(tKey_1[lBegin], tKey_1[lEnd], tKey_2[lBegin], tKey_2[lEnd], tKey_3[lBegin], tKey_3[lEnd]) > 0)
			{
				tTmp_1 = tKey_1[lBegin];
				tKey_1[lBegin] = tKey_1[lEnd];
				tKey_1[lEnd] = tTmp_1;

				tTmp_2 = tKey_2[lBegin];
				tKey_2[lBegin] = tKey_2[lEnd];
				tKey_2[lEnd] = tTmp_2;

				tTmp_3 = tKey_3[lBegin];
				tKey_3[lBegin] = tKey_3[lEnd];
				tKey_3[lEnd] = tTmp_3;

				tTmp_4 = tKey_4[lBegin];
				tKey_4[lBegin] = tKey_4[lEnd];
				tKey_4[lEnd] = tTmp_4;

				tTmp_5 = tKey_5[lBegin];
				tKey_5[lBegin] = tKey_5[lEnd];
				tKey_5[lEnd] = tTmp_5;
			}
			continue;
		}

		var_4 lMid = (lBegin + lEnd)>>1;
		var_4 m = lBegin, n = lEnd;

		T_Key_1 tMid_1 = tKey_1[lMid];
		T_Key_2 tMid_2 = tKey_2[lMid];
		T_Key_3 tMid_3 = tKey_3[lMid];

		while(lBegin < lEnd)
		{
			while(lBegin < lEnd && compare_key<T_Key_1, T_Key_2, T_Key_3>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2, tKey_3[lBegin], tMid_3) < 0) lBegin++;
			while(lBegin < lEnd && compare_key<T_Key_1, T_Key_2, T_Key_3>(tKey_1[lEnd], tMid_1, tKey_2[lEnd], tMid_2, tKey_3[lEnd], tMid_3) > 0) lEnd--;
			if(lBegin < lEnd)
			{
				tTmp_1 = tKey_1[lBegin];
				tKey_1[lBegin] = tKey_1[lEnd];
				tKey_1[lEnd] = tTmp_1;

				tTmp_2 = tKey_2[lBegin];
				tKey_2[lBegin] = tKey_2[lEnd];
				tKey_2[lEnd] = tTmp_2;

				tTmp_3 = tKey_3[lBegin];
				tKey_3[lBegin] = tKey_3[lEnd];
				tKey_3[lEnd] = tTmp_3;

				tTmp_4 = tKey_4[lBegin];
				tKey_4[lBegin] = tKey_4[lEnd];
				tKey_4[lEnd] = tTmp_4;

				tTmp_5 = tKey_5[lBegin];
				tKey_5[lBegin] = tKey_5[lEnd];
				tKey_5[lEnd] = tTmp_5;

				if(++lBegin < lEnd)
					lEnd--;
			}
		}

		if(compare_key<T_Key_1, T_Key_2, T_Key_3>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2, tKey_3[lBegin], tMid_3) < 0)
			lBegin++;

		if(lEnd < n)
		{
			stack_buf[stack_pos++] = lEnd;
			stack_buf[stack_pos++] = n;
		}
		if(lBegin - 1 > m)
		{
			stack_buf[stack_pos++] = m;
			stack_buf[stack_pos++] = lBegin - 1;
		}
	}	

	return 0;
};

/************************************************************************/
// 新整理
/************************************************************************/
template <class T_Key>
var_4 qs_recursion_1k(var_4 lBegin, var_4 lEnd, T_Key* tKey_1)
{
	if(lBegin >= lEnd)
		return 0;

	T_Key tTmp_1;

	if(lBegin + 1 == lEnd)
	{
		if(tKey_1[lBegin] > tKey_1[lEnd])
		{
			tTmp_1 = tKey_1[lBegin];
			tKey_1[lBegin] = tKey_1[lEnd];
			tKey_1[lEnd] = tTmp_1;
		}
		return 0;
	}

	var_4 lMid = (lBegin + lEnd)>>1;
	var_4 m = lBegin, n = lEnd;

	T_Key tMid_1 = tKey_1[lMid];

	while(lBegin < lEnd)
	{
		while(lBegin < lEnd && tKey_1[lBegin] < tMid_1) lBegin++;
		while(lBegin < lEnd && tKey_1[lEnd] > tMid_1) lEnd--;
		if(lBegin < lEnd)
		{
			tTmp_1 = tKey_1[lBegin];
			tKey_1[lBegin] = tKey_1[lEnd];
			tKey_1[lEnd] = tTmp_1;

			if(++lBegin < lEnd)
				lEnd--;
		}
	}

	if(tKey_1[lBegin] < tMid_1)
		lBegin++;

	if(lBegin - 1 > m)
		qs_recursion_1k(m, lBegin - 1, tKey_1);
	if(lEnd < n)
		qs_recursion_1k(lEnd, n, tKey_1);

	return 0;
};

template <class T_Key>
var_4 qs_unrecursion_1k(var_4 lBegin, var_4 lEnd, T_Key* tKey)
{
	var_4 stack_buf[1024];
	var_4 stack_pos = 0;

	stack_buf[stack_pos++] = lBegin;
	stack_buf[stack_pos++] = lEnd;

	for(;;)
	{
		if(stack_pos == 0)
			break;

		lEnd = stack_buf[--stack_pos];
		lBegin = stack_buf[--stack_pos];

		if(lBegin >= lEnd)
			continue;

		T_Key tTmp;

		if(lBegin + 1 == lEnd)
		{
			if(tKey[lBegin] > tKey[lEnd])
			{
				tTmp = tKey[lBegin];
				tKey[lBegin] = tKey[lEnd];
				tKey[lEnd] = tTmp;
			}
			continue;
		}

		var_4 lMid = (lBegin + lEnd)>>1;
		var_4 m = lBegin, n = lEnd;

		T_Key tMid = tKey[lMid];

		while(lBegin < lEnd)
		{
			while(lBegin < lEnd && tKey[lBegin] < tMid) lBegin++;
			while(lBegin < lEnd && tKey[lEnd] > tMid) lEnd--;
			if(lBegin < lEnd)
			{
				tTmp = tKey[lBegin];
				tKey[lBegin] = tKey[lEnd];
				tKey[lEnd] = tTmp;

				if(++lBegin < lEnd)
					lEnd--;
			}
		}

		if(tKey[lBegin] < tMid)
			lBegin++;

		if(lEnd < n)
		{
			stack_buf[stack_pos++] = lEnd;
			stack_buf[stack_pos++] = n;
		}
		if(lBegin - 1 > m)
		{
			stack_buf[stack_pos++] = m;
			stack_buf[stack_pos++] = lBegin - 1;
		}
	}

	return 0;
};

template <class T_Key>
var_4 qs_unrecursion_1k(var_4 lBegin, var_4 lEnd, T_Key* tKey, var_4 (*_compare_fun)(T_Key, T_Key))
{
	var_4 stack_buf[1024];
	var_4 stack_pos = 0;

	stack_buf[stack_pos++] = lBegin;
	stack_buf[stack_pos++] = lEnd;

	for(;;)
	{
		if(stack_pos == 0)
			break;

		lEnd = stack_buf[--stack_pos];
		lBegin = stack_buf[--stack_pos];

		if(lBegin >= lEnd)
			continue;

		T_Key tTmp;

		if(lBegin + 1 == lEnd)
		{
			if(_compare_fun(tKey[lBegin], tKey[lEnd]) > 0)
			{
				tTmp = tKey[lBegin];
				tKey[lBegin] = tKey[lEnd];
				tKey[lEnd] = tTmp;
			}
			continue;
		}

		var_4 lMid = (lBegin + lEnd)>>1;
		var_4 m = lBegin, n = lEnd;

		T_Key tMid = tKey[lMid];

		while(lBegin < lEnd)
		{
			while(lBegin < lEnd && _compare_fun(tKey[lBegin], tMid) < 0) lBegin++;
			while(lBegin < lEnd && _compare_fun(tKey[lEnd], tMid) > 0) lEnd--;
			if(lBegin < lEnd)
			{
				tTmp = tKey[lBegin];
				tKey[lBegin] = tKey[lEnd];
				tKey[lEnd] = tTmp;

				if(++lBegin < lEnd)
					lEnd--;
			}
		}

		if(_compare_fun(tKey[lBegin], tMid) < 0)
			lBegin++;

		if(lEnd < n)
		{
			stack_buf[stack_pos++] = lEnd;
			stack_buf[stack_pos++] = n;
		}
		if(lBegin - 1 > m)
		{
			stack_buf[stack_pos++] = m;
			stack_buf[stack_pos++] = lBegin - 1;
		}
	}

	return 0;
};

template <class T_Key_1, class T_Key_2>
var_4 qs_recursion_2k(var_4 lBegin, var_4 lEnd, T_Key_1* tKey_1, T_Key_2* tKey_2)
{
	if(lBegin >= lEnd)
		return 0;

	T_Key_1 tTmp_1;
	T_Key_2 tTmp_2;

	if(lBegin + 1 == lEnd)
	{
		if(compare_key<T_Key_1, T_Key_2>(tKey_1[lBegin], tKey_1[lEnd], tKey_2[lBegin], tKey_2[lEnd]) > 0)
		{
			tTmp_1 = tKey_1[lBegin];
			tKey_1[lBegin] = tKey_1[lEnd];
			tKey_1[lEnd] = tTmp_1;

			tTmp_2 = tKey_2[lBegin];
			tKey_2[lBegin] = tKey_2[lEnd];
			tKey_2[lEnd] = tTmp_2;
		}
		return 0;
	}

	var_4 lMid = (lBegin + lEnd)>>1;
	var_4 m = lBegin, n = lEnd;

	T_Key_1 tMid_1 = tKey_1[lMid];
	T_Key_2 tMid_2 = tKey_2[lMid];	

	while(lBegin < lEnd)
	{
		while(lBegin < lEnd && compare_key<T_Key_1, T_Key_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2) < 0) lBegin++;
		while(lBegin < lEnd && compare_key<T_Key_1, T_Key_2>(tKey_1[lEnd], tMid_1, tKey_2[lEnd], tMid_2) > 0) lEnd--;
		if(lBegin < lEnd)
		{
			tTmp_1 = tKey_1[lBegin];
			tKey_1[lBegin] = tKey_1[lEnd];
			tKey_1[lEnd] = tTmp_1;

			tTmp_2 = tKey_2[lBegin];
			tKey_2[lBegin] = tKey_2[lEnd];
			tKey_2[lEnd] = tTmp_2;

			if(++lBegin < lEnd)
				lEnd--;
		}
	}

	if(compare_key<T_Key_1, T_Key_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2) < 0)
		lBegin++;

	if(lBegin - 1 > m)
		qs_recursion_2k(m, lBegin - 1, tKey_1, tKey_2);
	if(lEnd < n)
		qs_recursion_2k(lEnd, n, tKey_1, tKey_2);

	return 0;
};

template <class T_Key_1, class T_Key_2>
var_4 qs_unrecursion_2k(var_4 lBegin, var_4 lEnd, T_Key_1* tKey_1, T_Key_2* tKey_2)
{
	var_4 stack_buf[1024];
	var_4 stack_pos = 0;

	stack_buf[stack_pos++] = lBegin;
	stack_buf[stack_pos++] = lEnd;

	for(;;)
	{
		if(stack_pos == 0)
			break;

		lEnd = stack_buf[--stack_pos];
		lBegin = stack_buf[--stack_pos];

		if(lBegin >= lEnd)
			continue;

		T_Key_1 tTmp_1;
		T_Key_2 tTmp_2;

		if(lBegin + 1 == lEnd)
		{
			if(compare_key<T_Key_1, T_Key_2>(tKey_1[lBegin], tKey_1[lEnd], tKey_2[lBegin], tKey_2[lEnd]) > 0)
			{
				tTmp_1 = tKey_1[lBegin];
				tKey_1[lBegin] = tKey_1[lEnd];
				tKey_1[lEnd] = tTmp_1;

				tTmp_2 = tKey_2[lBegin];
				tKey_2[lBegin] = tKey_2[lEnd];
				tKey_2[lEnd] = tTmp_2;
			}
			continue;
		}

		var_4 lMid = (lBegin + lEnd)>>1;
		var_4 m = lBegin, n = lEnd;

		T_Key_1 tMid_1 = tKey_1[lMid];
		T_Key_2 tMid_2 = tKey_2[lMid];		

		while(lBegin < lEnd)
		{
			while(lBegin < lEnd && compare_key<T_Key_1, T_Key_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2) < 0) lBegin++;
			while(lBegin < lEnd && compare_key<T_Key_1, T_Key_2>(tKey_1[lEnd], tMid_1, tKey_2[lEnd], tMid_2) > 0) lEnd--;
			if(lBegin < lEnd)
			{
				tTmp_1 = tKey_1[lBegin];
				tKey_1[lBegin] = tKey_1[lEnd];
				tKey_1[lEnd] = tTmp_1;

				tTmp_2 = tKey_2[lBegin];
				tKey_2[lBegin] = tKey_2[lEnd];
				tKey_2[lEnd] = tTmp_2;

				if(++lBegin < lEnd)
					lEnd--;
			}
		}

		if(compare_key<T_Key_1, T_Key_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2) < 0)
			lBegin++;

		if(lEnd < n)
		{
			stack_buf[stack_pos++] = lEnd;
			stack_buf[stack_pos++] = n;
		}
		if(lBegin - 1 > m)
		{
			stack_buf[stack_pos++] = m;
			stack_buf[stack_pos++] = lBegin - 1;
		}
	}	

	return 0;
};

template <class T_Key_1, class T_Key_2>
var_4 qs_recursion_1k_1p(var_4 lBegin, var_4 lEnd, T_Key_1* tKey_1, T_Key_2* tKey_2)
{
	if(lBegin >= lEnd)
		return 0;

	T_Key_1 tTmp_1;
	T_Key_2 tTmp_2;

	if(lBegin + 1 == lEnd)
	{
		if(tKey_1[lBegin] > tKey_1[lEnd])
		{
			tTmp_1 = tKey_1[lBegin];
			tKey_1[lBegin] = tKey_1[lEnd];
			tKey_1[lEnd] = tTmp_1;

			tTmp_2 = tKey_2[lBegin];
			tKey_2[lBegin] = tKey_2[lEnd];
			tKey_2[lEnd] = tTmp_2;
		}
		return 0;
	}

	var_4 lMid = (lBegin + lEnd)>>1;
	var_4 m = lBegin, n = lEnd;

	T_Key_1 tMid_1 = tKey_1[lMid];	

	while(lBegin < lEnd)
	{
		while(lBegin < lEnd && tKey_1[lBegin] < tMid_1) lBegin++;
		while(lBegin < lEnd && tKey_1[lEnd] > tMid_1) lEnd--;
		if(lBegin < lEnd)
		{
			tTmp_1 = tKey_1[lBegin];
			tKey_1[lBegin] = tKey_1[lEnd];
			tKey_1[lEnd] = tTmp_1;

			tTmp_2 = tKey_2[lBegin];
			tKey_2[lBegin] = tKey_2[lEnd];
			tKey_2[lEnd] = tTmp_2;

			if(++lBegin < lEnd)
				lEnd--;
		}
	}

	if(tKey_1[lBegin] < tMid_1)
		lBegin++;

	if(lBegin - 1 > m)
		qs_recursion_1k_1p(m, lBegin - 1, tKey_1, tKey_2);
	if(lEnd < n)
		qs_recursion_1k_1p(lEnd, n, tKey_1, tKey_2);

	return 0;
};

template <class T_Key_1, class T_Pld_1, class T_Pld_2, class T_Pld_3, class T_Pld_4>
var_4 qs_recursion_1k_4p(var_4 lBegin, var_4 lEnd, T_Key_1* tKey_1, T_Pld_1* tPld_1, T_Pld_2* tPld_2, T_Pld_3* tPld_3, T_Pld_4* tPld_4)
{
	if(lBegin >= lEnd)
		return 0;
    
	T_Key_1 kTmp_1;
    
    T_Pld_1 tTmp_1;
	T_Pld_2 tTmp_2;
    T_Pld_3 tTmp_3;
    T_Pld_4 tTmp_4;
    
	if(lBegin + 1 == lEnd)
	{
		if(tKey_1[lBegin] > tKey_1[lEnd])
		{
			kTmp_1 = tKey_1[lBegin];
			tKey_1[lBegin] = tKey_1[lEnd];
			tKey_1[lEnd] = kTmp_1;
            
            tTmp_1 = tPld_1[lBegin];
			tPld_1[lBegin] = tPld_1[lEnd];
			tPld_1[lEnd] = tTmp_1;

			tTmp_2 = tPld_2[lBegin];
			tPld_2[lBegin] = tPld_2[lEnd];
			tPld_2[lEnd] = tTmp_2;
            
            tTmp_3 = tPld_3[lBegin];
			tPld_3[lBegin] = tPld_3[lEnd];
			tPld_3[lEnd] = tTmp_3;
            
            tTmp_4 = tPld_4[lBegin];
			tPld_4[lBegin] = tPld_4[lEnd];
			tPld_4[lEnd] = tTmp_4;
		}
        
		return 0;
	}
    
	var_4 lMid = (lBegin + lEnd)>>1;
	var_4 m = lBegin, n = lEnd;
    
	T_Key_1 tMid_1 = tKey_1[lMid];
    
	while(lBegin < lEnd)
	{
		while(lBegin < lEnd && tKey_1[lBegin] < tMid_1) lBegin++;
		while(lBegin < lEnd && tKey_1[lEnd] > tMid_1) lEnd--;
		if(lBegin < lEnd)
		{
			kTmp_1 = tKey_1[lBegin];
			tKey_1[lBegin] = tKey_1[lEnd];
			tKey_1[lEnd] = kTmp_1;
            
            tTmp_1 = tPld_1[lBegin];
			tPld_1[lBegin] = tPld_1[lEnd];
			tPld_1[lEnd] = tTmp_1;
            
			tTmp_2 = tPld_2[lBegin];
			tPld_2[lBegin] = tPld_2[lEnd];
			tPld_2[lEnd] = tTmp_2;
            
            tTmp_3 = tPld_3[lBegin];
			tPld_3[lBegin] = tPld_3[lEnd];
			tPld_3[lEnd] = tTmp_3;
            
            tTmp_4 = tPld_4[lBegin];
			tPld_4[lBegin] = tPld_4[lEnd];
			tPld_4[lEnd] = tTmp_4;
            
			if(++lBegin < lEnd)
				lEnd--;
		}
	}
    
	if(tKey_1[lBegin] < tMid_1)
		lBegin++;
    
	if(lBegin - 1 > m)
		qs_recursion_1k_4p(m, lBegin - 1, tKey_1, tPld_1, tPld_2, tPld_3, tPld_4);
	if(lEnd < n)
		qs_recursion_1k_4p(lEnd, n, tKey_1, tPld_1, tPld_2, tPld_3, tPld_4);
    
	return 0;
};

template <class T_Key_1, class T_Key_2>
var_4 qs_unrecursion_1k_1p(var_4 lBegin, var_4 lEnd, T_Key_1* tKey_1, T_Key_2* tKey_2)
{
	var_4 stack_buf[1024];
	var_4 stack_pos = 0;

	stack_buf[stack_pos++] = lBegin;
	stack_buf[stack_pos++] = lEnd;

	for(;;)
	{
		if(stack_pos == 0)
			break;

		lEnd = stack_buf[--stack_pos];
		lBegin = stack_buf[--stack_pos];

		if(lBegin >= lEnd)
			continue;

		T_Key_1 tTmp_1;
		T_Key_2 tTmp_2;

		if(lBegin + 1 == lEnd)
		{
			if(tKey_1[lBegin] > tKey_1[lEnd])
			{
				tTmp_1 = tKey_1[lBegin];
				tKey_1[lBegin] = tKey_1[lEnd];
				tKey_1[lEnd] = tTmp_1;

				tTmp_2 = tKey_2[lBegin];
				tKey_2[lBegin] = tKey_2[lEnd];
				tKey_2[lEnd] = tTmp_2;
			}
			continue;
		}

		var_4 lMid = (lBegin + lEnd)>>1;
		var_4 m = lBegin, n = lEnd;

		T_Key_1 tMid_1 = tKey_1[lMid];

		while(lBegin < lEnd)
		{
			while(lBegin < lEnd && tKey_1[lBegin] < tMid_1) lBegin++;
			while(lBegin < lEnd && tKey_1[lEnd] > tMid_1) lEnd--;
			if(lBegin < lEnd)
			{
				tTmp_1 = tKey_1[lBegin];
				tKey_1[lBegin] = tKey_1[lEnd];
				tKey_1[lEnd] = tTmp_1;

				tTmp_2 = tKey_2[lBegin];
				tKey_2[lBegin] = tKey_2[lEnd];
				tKey_2[lEnd] = tTmp_2;

				if(++lBegin < lEnd)
					lEnd--;
			}
		}

		if(tKey_1[lBegin] < tMid_1)
			lBegin++;

		if(lEnd < n)
		{
			stack_buf[stack_pos++] = lEnd;
			stack_buf[stack_pos++] = n;
		}
		if(lBegin - 1 > m)
		{
			stack_buf[stack_pos++] = m;
			stack_buf[stack_pos++] = lBegin - 1;
		}
	}	

	return 0;
};

template <class T_Key_1, class T_Key_2, class T_Key_3>
var_4 qs_recursion_1k_2p(var_4 lBegin, var_4 lEnd, T_Key_1* tKey_1, T_Key_2* tKey_2, T_Key_3* tKey_3)
{
	if(lBegin >= lEnd)
		return 0;
    
	T_Key_1 tTmp_1;
	T_Key_2 tTmp_2;
    T_Key_3 tTmp_3;
    
	if(lBegin + 1 == lEnd)
	{
		if(tKey_1[lBegin] > tKey_1[lEnd])
		{
			tTmp_1 = tKey_1[lBegin];
			tKey_1[lBegin] = tKey_1[lEnd];
			tKey_1[lEnd] = tTmp_1;
            
			tTmp_2 = tKey_2[lBegin];
			tKey_2[lBegin] = tKey_2[lEnd];
			tKey_2[lEnd] = tTmp_2;
            
            tTmp_3 = tKey_3[lBegin];
			tKey_3[lBegin] = tKey_3[lEnd];
			tKey_3[lEnd] = tTmp_3;
		}
		return 0;
	}
    
	var_4 lMid = (lBegin + lEnd)>>1;
	var_4 m = lBegin, n = lEnd;
    
	T_Key_1 tMid_1 = tKey_1[lMid];
    
	while(lBegin < lEnd)
	{
		while(lBegin < lEnd && tKey_1[lBegin] < tMid_1) lBegin++;
		while(lBegin < lEnd && tKey_1[lEnd] > tMid_1) lEnd--;
		if(lBegin < lEnd)
		{
			tTmp_1 = tKey_1[lBegin];
			tKey_1[lBegin] = tKey_1[lEnd];
			tKey_1[lEnd] = tTmp_1;
            
			tTmp_2 = tKey_2[lBegin];
			tKey_2[lBegin] = tKey_2[lEnd];
			tKey_2[lEnd] = tTmp_2;
            
            tTmp_3 = tKey_3[lBegin];
			tKey_3[lBegin] = tKey_3[lEnd];
			tKey_3[lEnd] = tTmp_3;

			if(++lBegin < lEnd)
				lEnd--;
		}
	}
    
	if(tKey_1[lBegin] < tMid_1)
		lBegin++;
    
	if(lBegin - 1 > m)
		qs_recursion_1k_2p(m, lBegin - 1, tKey_1, tKey_2, tKey_3);
	if(lEnd < n)
		qs_recursion_1k_2p(lEnd, n, tKey_1, tKey_2, tKey_3);
    
	return 0;
};

template <class T_Key_1, class T_Key_2, class T_Key_3>
var_4 qs_unrecursion_1k_2p(var_4 lBegin, var_4 lEnd, T_Key_1* tKey_1, T_Key_2* tKey_2, T_Key_3* tKey_3)
{
	var_4 stack_buf[1024];
	var_4 stack_pos = 0;
    
	stack_buf[stack_pos++] = lBegin;
	stack_buf[stack_pos++] = lEnd;
    
	for(;;)
	{
		if(stack_pos == 0)
			break;
        
		lEnd = stack_buf[--stack_pos];
		lBegin = stack_buf[--stack_pos];
        
		if(lBegin >= lEnd)
			continue;
        
		T_Key_1 tTmp_1;
		T_Key_2 tTmp_2;
        T_Key_3 tTmp_3;
        
		if(lBegin + 1 == lEnd)
		{
			if(tKey_1[lBegin] > tKey_1[lEnd])
			{
				tTmp_1 = tKey_1[lBegin];
				tKey_1[lBegin] = tKey_1[lEnd];
				tKey_1[lEnd] = tTmp_1;
                
				tTmp_2 = tKey_2[lBegin];
				tKey_2[lBegin] = tKey_2[lEnd];
				tKey_2[lEnd] = tTmp_2;
                
                tTmp_3 = tKey_3[lBegin];
                tKey_3[lBegin] = tKey_3[lEnd];
                tKey_3[lEnd] = tTmp_3;
			}
			continue;
		}
        
		var_4 lMid = (lBegin + lEnd)>>1;
		var_4 m = lBegin, n = lEnd;
        
		T_Key_1 tMid_1 = tKey_1[lMid];
        
		while(lBegin < lEnd)
		{
			while(lBegin < lEnd && tKey_1[lBegin] < tMid_1) lBegin++;
			while(lBegin < lEnd && tKey_1[lEnd] > tMid_1) lEnd--;
			if(lBegin < lEnd)
			{
				tTmp_1 = tKey_1[lBegin];
				tKey_1[lBegin] = tKey_1[lEnd];
				tKey_1[lEnd] = tTmp_1;
                
				tTmp_2 = tKey_2[lBegin];
				tKey_2[lBegin] = tKey_2[lEnd];
				tKey_2[lEnd] = tTmp_2;
                
                tTmp_3 = tKey_3[lBegin];
                tKey_3[lBegin] = tKey_3[lEnd];
                tKey_3[lEnd] = tTmp_3;
                
				if(++lBegin < lEnd)
					lEnd--;
			}
		}
        
		if(tKey_1[lBegin] < tMid_1)
			lBegin++;
        
		if(lEnd < n)
		{
			stack_buf[stack_pos++] = lEnd;
			stack_buf[stack_pos++] = n;
		}
		if(lBegin - 1 > m)
		{
			stack_buf[stack_pos++] = m;
			stack_buf[stack_pos++] = lBegin - 1;
		}
	}	
    
	return 0;
};

template <class T_Key_1, class T_Key_2, class T_Key_3>
var_4 qs_unrecursion_2k_1p(var_4 lBegin, var_4 lEnd, T_Key_1* tKey_1, T_Key_2* tKey_2, T_Key_3* tKey_3)
{
	var_4 stack_buf[1024];
	var_4 stack_pos = 0;

	stack_buf[stack_pos++] = lBegin;
	stack_buf[stack_pos++] = lEnd;

	for(;;)
	{
		if(stack_pos == 0)
			break;

		lEnd = stack_buf[--stack_pos];
		lBegin = stack_buf[--stack_pos];

		if(lBegin >= lEnd)
			continue;

		T_Key_1 tTmp_1;
		T_Key_2 tTmp_2;
		T_Key_3 tTmp_3;

		if(lBegin + 1 == lEnd)
		{
			if(compare_key<T_Key_1, T_Key_2>(tKey_1[lBegin], tKey_1[lEnd], tKey_2[lBegin], tKey_2[lEnd]) > 0)
			{
				tTmp_1 = tKey_1[lBegin];
				tKey_1[lBegin] = tKey_1[lEnd];
				tKey_1[lEnd] = tTmp_1;

				tTmp_2 = tKey_2[lBegin];
				tKey_2[lBegin] = tKey_2[lEnd];
				tKey_2[lEnd] = tTmp_2;

				tTmp_3 = tKey_3[lBegin];
				tKey_3[lBegin] = tKey_3[lEnd];
				tKey_3[lEnd] = tTmp_3;
			}
			continue;
		}

		var_4 lMid = (lBegin + lEnd)>>1;
		var_4 m = lBegin, n = lEnd;

		T_Key_1 tMid_1 = tKey_1[lMid];
		T_Key_2 tMid_2 = tKey_2[lMid];

		while(lBegin < lEnd)
		{
			while(lBegin < lEnd && compare_key<T_Key_1, T_Key_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2) < 0) lBegin++;
			while(lBegin < lEnd && compare_key<T_Key_1, T_Key_2>(tKey_1[lEnd], tMid_1, tKey_2[lEnd], tMid_2) > 0) lEnd--;
			if(lBegin < lEnd)
			{
				tTmp_1 = tKey_1[lBegin];
				tKey_1[lBegin] = tKey_1[lEnd];
				tKey_1[lEnd] = tTmp_1;

				tTmp_2 = tKey_2[lBegin];
				tKey_2[lBegin] = tKey_2[lEnd];
				tKey_2[lEnd] = tTmp_2;

				tTmp_3 = tKey_3[lBegin];
				tKey_3[lBegin] = tKey_3[lEnd];
				tKey_3[lEnd] = tTmp_3;

				if(++lBegin < lEnd)
					lEnd--;
			}
		}

		if(compare_key<T_Key_1, T_Key_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2) < 0)
			lBegin++;

		if(lEnd < n)
		{
			stack_buf[stack_pos++] = lEnd;
			stack_buf[stack_pos++] = n;
		}
		if(lBegin - 1 > m)
		{
			stack_buf[stack_pos++] = m;
			stack_buf[stack_pos++] = lBegin - 1;
		}
	}	

	return 0;
};

template <class T_Key_1, class T_Key_2, class T_Key_3, class T_Key_4>
var_4 qs_unrecursion_2k_2p(var_4 lBegin, var_4 lEnd, T_Key_1* tKey_1, T_Key_2* tKey_2, T_Key_3* tKey_3, T_Key_4* tKey_4)
{
	var_4 stack_buf[1024];
	var_4 stack_pos = 0;

	stack_buf[stack_pos++] = lBegin;
	stack_buf[stack_pos++] = lEnd;

	for(;;)
	{
		if(stack_pos == 0)
			break;

		lEnd = stack_buf[--stack_pos];
		lBegin = stack_buf[--stack_pos];

		if(lBegin >= lEnd)
			continue;

		T_Key_1 tTmp_1;
		T_Key_2 tTmp_2;
		T_Key_3 tTmp_3;
		T_Key_4 tTmp_4;

		if(lBegin + 1 == lEnd)
		{
			if(compare_key<T_Key_1, T_Key_2>(tKey_1[lBegin], tKey_1[lEnd], tKey_2[lBegin], tKey_2[lEnd]) > 0)
			{
				tTmp_1 = tKey_1[lBegin];
				tKey_1[lBegin] = tKey_1[lEnd];
				tKey_1[lEnd] = tTmp_1;

				tTmp_2 = tKey_2[lBegin];
				tKey_2[lBegin] = tKey_2[lEnd];
				tKey_2[lEnd] = tTmp_2;

				tTmp_3 = tKey_3[lBegin];
				tKey_3[lBegin] = tKey_3[lEnd];
				tKey_3[lEnd] = tTmp_3;

				tTmp_4 = tKey_4[lBegin];
				tKey_4[lBegin] = tKey_4[lEnd];
				tKey_4[lEnd] = tTmp_4;
			}
			continue;
		}

		var_4 lMid = (lBegin + lEnd)>>1;
		var_4 m = lBegin, n = lEnd;

		T_Key_1 tMid_1 = tKey_1[lMid];
		T_Key_2 tMid_2 = tKey_2[lMid];

		while(lBegin < lEnd)
		{
			while(lBegin < lEnd && compare_key<T_Key_1, T_Key_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2) < 0) lBegin++;
			while(lBegin < lEnd && compare_key<T_Key_1, T_Key_2>(tKey_1[lEnd], tMid_1, tKey_2[lEnd], tMid_2) > 0) lEnd--;
			if(lBegin < lEnd)
			{
				tTmp_1 = tKey_1[lBegin];
				tKey_1[lBegin] = tKey_1[lEnd];
				tKey_1[lEnd] = tTmp_1;

				tTmp_2 = tKey_2[lBegin];
				tKey_2[lBegin] = tKey_2[lEnd];
				tKey_2[lEnd] = tTmp_2;

				tTmp_3 = tKey_3[lBegin];
				tKey_3[lBegin] = tKey_3[lEnd];
				tKey_3[lEnd] = tTmp_3;

				tTmp_4 = tKey_4[lBegin];
				tKey_4[lBegin] = tKey_4[lEnd];
				tKey_4[lEnd] = tTmp_4;

				if(++lBegin < lEnd)
					lEnd--;
			}
		}

		if(compare_key<T_Key_1, T_Key_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2) < 0)
			lBegin++;

		if(lEnd < n)
		{
			stack_buf[stack_pos++] = lEnd;
			stack_buf[stack_pos++] = n;
		}
		if(lBegin - 1 > m)
		{
			stack_buf[stack_pos++] = m;
			stack_buf[stack_pos++] = lBegin - 1;
		}
	}	

	return 0;
};

/************************************************************************/
// 并行快排
/************************************************************************/
template <class T_Key>
class UT_Parallel_QS
{
public:	
	struct ORG_INFO
	{
		CP_MUTEXLOCK lock;
		var_4 flg;
		T_Key* key;
	};
	
	struct SORT_INFO
	{
		ORG_INFO* org;
		var_4 beg;
		var_4 end;
	};
	
	static CP_THREAD_T thread_sort(void* argv)
	{
		UT_Parallel_QS<T_Key>* cc = (UT_Parallel_QS<T_Key>*)argv;

		for(;;)
		{
			SORT_INFO* node = cc->m_busy_lst.PopData();

			//
			T_Key* tKey = node->org->key;
			var_4 lBegin = node->beg;
			var_4 lEnd = node->end;

			if(lEnd - lBegin < cc->m_threshold_normal)
			{
				qs_unrecursion_1k<T_Key>(lBegin, lEnd, tKey);
				node->org->lock.lock();
				node->org->flg -= 2;
				node->org->lock.unlock();
				cc->m_idle_lst.PushData(node);
				continue;
			}

			var_4 lMid = (lBegin + lEnd)>>1;
			var_4 m = lBegin, n = lEnd;

			T_Key tMid = tKey[lMid];

			while(lBegin < lEnd)
			{
				while(lBegin < lEnd && tKey[lBegin] < tMid) lBegin++;
				while(lBegin < lEnd && tKey[lEnd] > tMid) lEnd--;
				if(lBegin < lEnd)
				{
					T_Key tTmp = tKey[lBegin];
					tKey[lBegin] = tKey[lEnd];
					tKey[lEnd] = tTmp;

					if(++lBegin < lEnd)
						lEnd--;
				}
			}

			if(tKey[lBegin] < tMid)
				lBegin++;

			SORT_INFO* new_node = NULL;

			new_node = cc->m_idle_lst.PopData();
			new_node->beg = lEnd;
			new_node->end = n;
			new_node->org = node->org;
			new_node->org->lock.lock();
			new_node->org->flg++;
			new_node->org->lock.unlock();
			cc->m_busy_lst.PushData(new_node);

			new_node = cc->m_idle_lst.PopData();
			new_node->beg = m;
			new_node->end = lBegin - 1;
			new_node->org = node->org;
			new_node->org->lock.lock();
			new_node->org->flg++;
			new_node->org->lock.unlock();
			cc->m_busy_lst.PushData(new_node);

			cc->m_idle_lst.PushData(node);
		}

		return 0;
	}

	var_4 init_qs(var_4 threshold_normal = 1000, var_4 list_size = 100000, var_4 thread_num = 10)
	{
		m_threshold_normal = threshold_normal;
		m_list_size = list_size;
		m_thread_num = thread_num;

		if(m_idle_lst.InitQueue(m_list_size))
			return -1;
		if(m_busy_lst.InitQueue(m_list_size))
			return -1;
		SORT_INFO* node_lst = new SORT_INFO[m_list_size];
		if(node_lst == NULL)
			return -1;
		for(var_4 i = 0; i < m_list_size; i++)
			m_idle_lst.PushData(node_lst + i);			
		for(var_4 i = 0; i < m_thread_num; i++)
			cp_create_thread(thread_sort, this);
		return 0;
	}

	var_4 data_qs(var_4 begin, var_4 end, T_Key* key)
	{
		ORG_INFO org_info;
		org_info.key = key;
		org_info.flg = 1;	

		SORT_INFO* node = m_idle_lst.PopData();		
		node->beg = begin;
		node->end = end;
		node->org = &org_info;

		node->org->flg++;
		m_busy_lst.PushData(node);

		while(org_info.flg) cp_sleep(1);

		return 0;
	}

	UT_Queue<SORT_INFO*> m_idle_lst;
	UT_Queue<SORT_INFO*> m_busy_lst;

	var_4 m_threshold_normal;
	var_4 m_list_size;
	var_4 m_thread_num;
};

/************************************************************************/
// radix sort 基数排序                                                                     
/************************************************************************/
template <class T_Key>
void radix_sort_one_byte(var_4 num, T_Key* src, T_Key* des)
{
	var_4 pass_num = sizeof(T_Key);

	for(var_4 i = 0; i < pass_num;i++)
	{
		printf(" -- %d pass\n", i);

		var_4 bucket[256];
		memset(bucket, 0, 256<<2);

		for(var_4 j = 0; j < num; j++)		
			bucket[*((var_u1*)(src + j) + i)]++;

		var_4 now = 0, all = 0;
		for(var_4 j = 0; j < 256; j++)
		{
			now = bucket[j];
			bucket[j] = all;
			all += now;
		}
				
		for(var_4 j = 0; j < num; j++)		
			des[bucket[*((var_u1*)(src + j) + i)]++] = src[j];

		T_Key* tmp = des;
		des = src;
		src = tmp;
	}
}

#endif // _UT_SORT_H_
