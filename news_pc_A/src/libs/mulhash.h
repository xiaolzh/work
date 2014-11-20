#ifndef __MULTI_HASH_H_
#define __MULTI_HASH_H_

#include "IDAlloc.h"
#include "log.h"

/* one key--->multi values */

template<typename K,typename V> class Mulhash
{
public:
	/* id to values area */
	struct Idv
	{
		u32 next;
		u32 count;
	};
	/* key to id area*/
	struct Keyblk
	{
		K key;
		u32 id;
		u32 next;
	};
	/* values area */
	struct Valblk
	{
		V val;
		u32 next;
	};
	
	/* construction stuff */
	Mulhash(int maxkeynum,int maxvalnum);
	~Mulhash();
	int Init(Log_file *plog,int recovery,char *filenm);
	int Add(K key,V val);
	int Delval(K key,V val);
	int Delkey(K key);
	int GetVals(K key,unsigned int startnum,unsigned int &num,V *vals);
	int CountKey(K key);
	int CountAll();
	int Save(char *filenm);
	int Load(char *filenm);

private:
	/* lock */
	pthread_rwlock_t m_lockhash;

	/* blkmanager,data areas */
	IDAlloc *m_pIDkey;
	Keyblk  *m_pMemkey;
	IDAlloc *m_pIDval;
	Valblk  *m_pMemval;

	/* buckets */
	u32* m_buckkeys;
	Idv *m_buckvals;

	/* numbers */
	unsigned int m_maxkeynum;
	unsigned int m_maxvalnum;
	int m_curkeynum;
	int m_curvalnum;

	int m_buckkeynum;
	int m_buckvalnum;

	/* log */
	Log_file *pLog;
};

/* implementation */
template<typename K,typename V> Mulhash<K,V>::Mulhash(int maxkeynum,int maxvalnum)
{
	if(maxkeynum<=0 || maxvalnum<=0)
	{
		m_maxkeynum = 10000;
		m_maxvalnum = 100000;
	}
	else
	{
		m_maxkeynum = maxkeynum;
		m_maxvalnum = maxvalnum;
	}
	m_curkeynum = 0;
	m_curvalnum = 0;
	m_buckkeynum = 0;
	m_buckvalnum = 0;
	memset(&m_lockhash,0,sizeof(pthread_rwlock_t));
	m_pIDkey = NULL;
	m_pMemkey = NULL;
	m_pIDval = NULL;
	m_pMemval = NULL;

	pLog = NULL;
}

template<typename K,typename V> Mulhash<K,V>::~Mulhash()
{
	pthread_rwlock_destroy(&m_lockhash);
	memset(&m_lockhash,0,sizeof(pthread_rwlock_t));
	if(m_pIDkey != NULL)
	{
		delete m_pIDkey;
		m_pIDkey = NULL;
	}
	if(m_pIDval != NULL)
	{
		delete m_pIDval;
		m_pIDval = NULL;
	}
	if(m_pMemkey != NULL)
	{
		free(m_pMemkey);
		m_pMemkey = NULL;
	}
	if(m_pMemval != NULL)
	{
		free(m_pMemval);
		m_pMemval = NULL;
	}
	if(m_buckkeys != NULL)
	{
		free(m_buckkeys);
		m_buckkeys = NULL;
	}
	if(m_buckvals != NULL)
	{
		free(m_buckvals);
		m_buckvals = NULL;
	}
}

template<typename K,typename V> int Mulhash<K,V>::Init(Log_file *plog,int recovery,char *filenm)
{
	/* log */
	if(NULL==plog || (recovery==1 && NULL==filenm))
	{
		printf("Bad parameter in Mulhash::Init!\n");
		return -1;
	}
	pLog=plog;
	/* init locks */
	if(pthread_rwlock_init(&m_lockhash,NULL))
    {
        pLog->writelog("Init lock1 failed in Mulhash::Init\n");
        return -2;
    }
	/* init ID allocs */
	m_pIDkey=new IDAlloc();
	if(NULL==m_pIDkey)
	{
        pLog->writelog("new idalloc failed in Mulhash::Init\n");
        return -3;
	}
	m_pIDval=new IDAlloc();
	if(NULL==m_pIDval)
	{
        pLog->writelog("idval alloc failed in Mulhash::Init\n");
        return -5;
	}

	if(recovery==0)
	{
		if(m_pIDkey->init(m_maxkeynum)<=0)
		{
	        pLog->writelog("idkey init failed in Mulhash::Init\n");
	        return -4;
		}
		if(m_pIDval->init(m_maxvalnum)<=0)
		{
	        pLog->writelog("idval init failed in Mulhash::Init\n");
	        return -6;
		}
		/* alloc memorys */
		m_pMemkey=(Keyblk *)calloc(m_maxkeynum + 2,sizeof(Keyblk));
		if(NULL==m_pMemkey)
		{
	        pLog->writelog("alloc key blkmem failed in Mulhash::Init\n");
	        return -7;
		}
		m_pMemval=(Valblk *)calloc(m_maxvalnum + 2,sizeof(Valblk));
		if(NULL==m_pMemval)
		{
	        pLog->writelog("alloc val blkmem failed in Mulhash::Init\n");
	        return -8;
		}
		/* init buckets */
		/* calculate bucket number first */
		m_buckkeynum = m_maxkeynum / 3;
		m_buckkeys=(u32 *)calloc(m_buckkeynum,sizeof(unsigned int));
		if(NULL==m_buckkeys)
		{
	        pLog->writelog("alloc bucket key mem failed in Mulhash::Init\n");
	        return -9;
		}
		m_buckvalnum = m_maxkeynum + 2;
		m_buckvals=(Idv *)calloc(m_buckvalnum,sizeof(Idv));
		if(NULL==m_buckvals)
		{
	        pLog->writelog("alloc bucket val mem failed in Mulhash::Init\n");
	        return -10;
		}
	}
	else
	{
		return Load(filenm);
	}
	return 1;
}

template<typename K,typename V> int Mulhash<K,V>::Add(K key,V val)
{
	unsigned int idk=0;
	unsigned int idv=0;
	unsigned int nxt=0;
	unsigned int nxtv=0;
	unsigned int id4key=0;
	unsigned int lidx=(unsigned int)(key % m_buckkeynum);
	int ret=0;
	unsigned int i=0;
	Keyblk *pkblk;
	Valblk *pvblk;

	if(pthread_rwlock_wrlock(&m_lockhash))
	{
		pLog->writelog("require write lock failed in Mulhash::Add %d!\n",errno);
		return -1;
	}
	/* first check if key,val exist */
	nxt=m_buckkeys[lidx];
	if(nxt==0)
	{
		/* if not exist,alloc id & blk */
		/* alloc a id for key */
		ret=m_pIDkey->getID(idk);
		if(ret<=0)
		{
			pthread_rwlock_unlock(&m_lockhash);
			pLog->writelog("Warn:getBlkID failed2 in Mulhash::Add %d!\n",ret);
			/* maybe we should resize ??? */
			return -2;
		}
		/* id is blk idx*/
		m_pMemkey[idk].key = key;
		m_pMemkey[idk].id  = idk;
		m_pMemkey[idk].next = 0;
		m_buckkeys[lidx] = idk;
		m_curkeynum++;

		/* attach to val list */
		ret=m_pIDval->getID(idv);
		if(ret<=0)
		{
			pthread_rwlock_unlock(&m_lockhash);
			pLog->writelog("Warn:getBlkID failed3 in Mulhash::Add %d!\n",ret);
			/* maybe we should resize ??? */
			return -3;
		}
		m_pMemval[idv].val = val;
		m_pMemval[idv].next = 0;
		m_buckvals[idk].next = idv;
		m_buckvals[idk].count = 1;
	}
	else
	{
		/* try to locate key */
		while(nxt)
		{
			pkblk=&m_pMemkey[nxt];
			if(pkblk->key==key)
			{
				id4key = pkblk->id;
				break;
			}
			nxt = pkblk->next;
		}
		/* not found,we should alloc one */
		if(id4key == 0)
		{
			/* alloc a id for key */
			ret=m_pIDkey->getID(idk);
			if(ret<=0)
			{
				pthread_rwlock_unlock(&m_lockhash);
				pLog->writelog("Warn:getBlkID failed5 in Mulhash::Add %d!\n",ret);
				/* maybe we should resize ??? */
				return -5;
			}
			/* id is blk idx,attach to buckeys */
			m_pMemkey[idk].key = key;
			m_pMemkey[idk].id  = idk;
			m_pMemkey[idk].next = m_buckkeys[lidx];
			m_buckkeys[lidx] = idk;
			m_curkeynum++;

			/* attach to val list */
			ret=m_pIDval->getID(idv);
			if(ret<=0)
			{
				pthread_rwlock_unlock(&m_lockhash);
				pLog->writelog("Warn:getBlkID failed6 in Mulhash::Add %d!\n",ret);
				/* maybe we should resize ??? */
				return -6;
			}
			m_pMemval[idv].val = val;
			m_pMemval[idv].next = 0;
			m_buckvals[idk].next = idv;
			m_buckvals[idk].count = 1;
		}
		else
		{
			i = 0;
			/* try to found val */
			nxtv = m_buckvals[id4key].next;
			while(nxtv && i<m_buckvals[id4key].count)
			{
				pvblk = &m_pMemval[nxtv];
				if(pvblk->val == val)
				{
					pthread_rwlock_unlock(&m_lockhash);
					return 1;
				}
				nxtv = pvblk->next;
				i++;
			}
			/* not found,try to alloc a blk & attach to list */
			ret=m_pIDval->getID(idv);
			if(ret<=0)
			{
				pthread_rwlock_unlock(&m_lockhash);
				pLog->writelog("Warn:getBlkID failed4 in Mulhash::Add %d!\n",ret);
				/* maybe we should resize ??? */
				return -4;
			}
			m_pMemval[idv].val = val;
			m_pMemval[idv].next = m_buckvals[id4key].next;
			m_buckvals[id4key].next = idv;
			m_buckvals[id4key].count ++;
		}
	}
	pthread_rwlock_unlock(&m_lockhash);
	return 1;
}

template<typename K,typename V> int Mulhash<K,V>::Delkey(K key)
{
	unsigned int nxt=0;
	unsigned int nxt1=0;
	unsigned int nxt2=0;
	unsigned int nxt3=0;
	unsigned int id4key=0;
	unsigned int lidx=(unsigned int)(key % m_buckkeynum);

	if(pthread_rwlock_wrlock(&m_lockhash))
	{
		pLog->writelog("require write lock failed in Mulhash::Del %d!\n",errno);
		return -1;
	}
	nxt=m_buckkeys[lidx];
	if(nxt!=0)
	{
		/* try to locate,first */
		if(m_pMemkey[nxt].key==key)
		{
			id4key = m_pMemkey[nxt].id;
			m_buckkeys[lidx]=m_pMemkey[nxt].next;
			m_pIDkey->freeID(nxt);
			m_curkeynum--;

			/* free associate vals */
			if(id4key > 0 && id4key <= m_maxkeynum)
			{
				nxt1 = m_buckvals[id4key].next;
				while(nxt1)
				{
					nxt2 = nxt1;
					nxt1 = m_pMemval[nxt2].next;
					m_pIDval->freeID(nxt2);
					memset(&m_pMemval[nxt2],0,sizeof(Valblk));
				}
				m_buckvals[id4key].next = 0;
				m_buckvals[id4key].count = 0;
			}
			pthread_rwlock_unlock(&m_lockhash);
			return 1;
		}
		nxt1=nxt;
		nxt=m_pMemkey[nxt].next;
		while(nxt)
		{
			if(m_pMemkey[nxt].key==key)
			{
				id4key = m_pMemkey[nxt].id;
				m_pMemkey[nxt1].next=m_pMemkey[nxt].next;
				m_pIDkey->freeID(nxt);
				m_curkeynum--;

				/* free associate vals */
				if(id4key > 0 && id4key <= m_maxkeynum)
				{
					nxt3 = m_buckvals[id4key].next;
					while(nxt3)
					{
						nxt2 = nxt3;
						nxt3 = m_pMemval[nxt2].next;
						m_pIDval->freeID(nxt2);
						memset(&m_pMemval[nxt2],0,sizeof(Valblk));
					}
					m_buckvals[id4key].next = 0;
					m_buckvals[id4key].count = 0;
				}
				pthread_rwlock_unlock(&m_lockhash);
				return 1;
			}
			nxt1=nxt;
			nxt=m_pMemkey[nxt].next;
		}
		/* not found */
	}
	pthread_rwlock_unlock(&m_lockhash);
	return 0;
}

template<typename K,typename V> int Mulhash<K,V>::Delval(K key,V val)
{
	unsigned int nxt=0;
	unsigned int nxt1=0;
	unsigned int nxt2=0;
	unsigned int nxt3=0;
	unsigned int id4key=0;
	unsigned int lidx=(unsigned int)(key % m_buckkeynum);
	int flag = 0;

	if(pthread_rwlock_wrlock(&m_lockhash))
	{
		pLog->writelog("require write lock failed in Mulhash::Del %d!\n",errno);
		return -1;
	}
	nxt=m_buckkeys[lidx];
	if(nxt!=0)
	{
		/* first locate keyid for key */
		nxt3 = 0;
		while(nxt)
		{
			if(m_pMemkey[nxt].key == key)
			{
				id4key = m_pMemkey[nxt].id;
				break;
			}
			nxt3 = nxt;
			nxt = m_pMemkey[nxt].next;
		}

		/* then,try to locate vals */
		if(id4key>0 && id4key <= (unsigned int)m_maxkeynum)
		{
			nxt1 = m_buckvals[id4key].next;
			if(m_pMemval[nxt1].val==val)
			{
				m_buckvals[id4key].next = m_pMemval[nxt1].next;
				m_pIDval->freeID(nxt1);
				memset(&m_pMemval[nxt1],0,sizeof(Valblk));
				m_buckvals[id4key].count --;
				flag = 1;
			}
			else
			{
				nxt2 = nxt1;
				nxt1 = m_pMemval[nxt2].next;
				while(nxt1)
				{
					if(m_pMemval[nxt1].val==val)
					{
						m_pMemval[nxt2].next = m_pMemval[nxt1].next;
						m_pIDval->freeID(nxt1);
						memset(&m_pMemval[nxt1],0,sizeof(Valblk));
						m_buckvals[id4key].count --;
						flag = 1;
						break;
					}
					nxt2 = nxt1;
					nxt1 = m_pMemval[nxt2].next;
				}
			}
			/* not found or found */
			if(m_buckvals[id4key].count == 0)
			{
				/* we should del key from key2ids */
				if(nxt3==0)
				{
					/* is head pointer to the key */
					m_buckkeys[lidx] = m_pMemkey[nxt].next;
					m_pIDkey->freeID(nxt);
					memset(&m_pMemkey[nxt],0,sizeof(Keyblk));
					m_curkeynum--;
				}
				else
				{
					m_pMemkey[nxt3].next = m_pMemkey[nxt].next;
					m_pIDkey->freeID(nxt);
					memset(&m_pMemkey[nxt],0,sizeof(Keyblk));
					m_curkeynum--;
				}
			}
		}
		pthread_rwlock_unlock(&m_lockhash);
		return flag;
	}
	pthread_rwlock_unlock(&m_lockhash);
	return 0;
}

template<typename K,typename V> int Mulhash<K,V>::GetVals(K key,unsigned int startnum,unsigned int &num,V *vals)
{
	unsigned int nxt=0;
	unsigned int nxt1=0;
	unsigned int id4key=0;
	unsigned int lidx=(unsigned int)(key % m_buckkeynum);
	unsigned int i=0;

	if(pthread_rwlock_rdlock(&m_lockhash))
	{
		pLog->writelog("require read lock failed in Mulhash::GetVals %d!\n",errno);
		return -1;
	}
	/* first locate id4key */
	nxt = m_buckkeys[lidx];
	if(nxt > 0)
	{
		while(nxt)
		{
			if(m_pMemkey[nxt].key == key)
			{
				id4key =  m_pMemkey[nxt].id;
				break;
			}
			nxt = m_pMemkey[nxt].next;
		}
		/* then get all vals */
		if(id4key > 0 && id4key <= m_maxkeynum)
		{
			if(m_buckvals[id4key].count>0)
			{
				nxt1 = m_buckvals[id4key].next;
				/* first skip startnum ids */
				for(i=0;nxt1>0 && i<startnum && i<m_buckvals[id4key].count;i++)
				{
					nxt1 = m_pMemval[nxt1].next;
				}
				/* then get num ids */
				i=0;
				for(;nxt1>0 && i<num && (i + startnum)<m_buckvals[id4key].count;i++)
				{
					vals[i] = m_pMemval[nxt1].val;
					nxt1 = m_pMemval[nxt1].next;
				}
				num = i;
				pthread_rwlock_unlock(&m_lockhash);
				return 1;
			}
		}
	}
	num = 0;
	pthread_rwlock_unlock(&m_lockhash);
	return 0;
}

template<typename K,typename V> int Mulhash<K,V>::CountAll()
{
	return m_curkeynum;
}

template<typename K,typename V> int Mulhash<K,V>::CountKey(K key)
{
	unsigned int nxt=0;
	unsigned int id4key=0;
	unsigned int lidx=(unsigned int)(key % m_buckkeynum);

	if(pthread_rwlock_rdlock(&m_lockhash))
	{
		pLog->writelog("require read lock failed in Mulhash::CountKey %d!\n",errno);
		return -1;
	}
	/* first locate id4key */
	nxt = m_buckkeys[lidx];
	if(nxt > 0)
	{
		while(nxt)
		{
			if(m_pMemkey[nxt].key == key)
			{
				id4key =  m_pMemkey[nxt].id;
				break;
			}
			nxt = m_pMemkey[nxt].next;
		}
		/* then get all vals */
		if(id4key > 0U && id4key <= (unsigned int)m_maxkeynum)
		{
			pthread_rwlock_unlock(&m_lockhash);
			return m_buckvals[id4key].count;
		}
	}
	pthread_rwlock_unlock(&m_lockhash);
	return 0;
}

template<typename K,typename V> int Mulhash<K,V>::Save(char *filenm)
{
	FILE *fp1=NULL;
	FILE *fp2=NULL;
	char fnm1[_MAX_PATH];
	char fnm2[_MAX_PATH];
	char fnm3[_MAX_PATH];
	char fnm4[_MAX_PATH];
	int j=0;
	int j1=0;
	int j2=0;
	int j3=0;
	int j4=0;
	int ret=0;
	int crc=0;
	int now=0;

	if(NULL==filenm)
	{
		pLog->writelog("NULL parameter in Mulhash::Save!\n");
		return -1;
	}
	j1=SPRINT(fnm1,_MAX_PATH,"%s.idkey",filenm);
	j2=SPRINT(fnm2,_MAX_PATH,"%s.idval",filenm);
	j3=SPRINT(fnm3,_MAX_PATH,"%s.datakey",filenm);
	j4=SPRINT(fnm4,_MAX_PATH,"%s.dataval",filenm);
	if(j1<=0 || j2<=0 || j3<=0 || j4<=0)
	{
		pLog->writelog("Get filenm failed in Mulhash::Save!\n");
		return -2;
	}
	fp1=fopen(fnm3,"wb");
	fp2=fopen(fnm4,"wb");
	if(NULL==fp1 || NULL==fp2)
	{
		if(fp1) { fclose(fp1); fp1=NULL; }
		if(fp2) { fclose(fp2); fp2=NULL; }
		pLog->writelog("OPen files %s for write failed in Mulhash::Save!\n",filenm);
		return -3;
	}
	if(pthread_rwlock_rdlock(&m_lockhash))
	{
		fclose(fp1);
		fclose(fp2);
		pLog->writelog("require read lock failed in Mulhash::Save %d!\n",errno);
		return -4;
	}
	if((ret=m_pIDkey->save(fnm1))<=0)
	{
		pthread_rwlock_unlock(&m_lockhash);
		fclose(fp1);
		fclose(fp2);
		pLog->writelog("save ID for key failed in Mulhash::Save %d!\n",ret);
		return -5;
	}
	if((ret=m_pIDval->save(fnm2))<=0)
	{
		pthread_rwlock_unlock(&m_lockhash);
		fclose(fp1);
		fclose(fp2);
		pLog->writelog("save ID for val failed in Mulhash::Save %d!\n",ret);
		return -6;
	}
	/* first write file for key data */
	j=fwrite("$$$$",4,1,fp1);
	now=(int)time(NULL);
	if(j==1)
	{
		j=fwrite(&now,sizeof(int),1,fp1);
	}
	if(j==1)
	{
		j=fwrite(&m_maxkeynum,sizeof(int),1,fp1);
	}
	if(j==1)
	{
		j=fwrite(&m_buckkeynum,sizeof(int),1,fp1);
	}
	if(j==1)
	{
		j=fwrite(&m_curkeynum,sizeof(int),1,fp1);
	}
	crc = m_maxkeynum ^ m_buckkeynum ^ m_curkeynum ^ now;
	if(j==1)
	{
		j=fwrite(&crc,sizeof(int),1,fp1);
	}
	if(j==1)
	{
		j=fwrite("$$$$$$$$",8,1,fp1);
	}
	if(j==1)
	{
		j=fwrite(m_buckkeys,sizeof(u32) * m_buckkeynum,1,fp1);
	}
	if(j==1)
	{
		j=fwrite(m_pMemkey,sizeof(Keyblk) * (m_maxkeynum + 2),1,fp1);
	}
	/* write file for val data */
	j=fwrite("$$$$",4,1,fp2);
	if(j==1)
	{
		j=fwrite(&now,sizeof(int),1,fp2);
	}
	if(j==1)
	{
		j=fwrite(&m_maxvalnum,sizeof(int),1,fp2);
	}
	if(j==1)
	{
		j=fwrite(&m_buckvalnum,sizeof(int),1,fp2);
	}
	if(j==1)
	{
		j=fwrite(&m_curvalnum,sizeof(int),1,fp2);
	}
	crc = m_maxvalnum ^ m_buckvalnum ^ m_curvalnum ^ now;
	if(j==1)
	{
		j=fwrite(&crc,sizeof(int),1,fp2);
	}
	if(j==1)
	{
		j=fwrite("$$$$$$$$",8,1,fp2);
	}
	if(j==1)
	{
		j=fwrite(m_buckvals,sizeof(Idv) * m_buckvalnum,1,fp2);
	}
	if(j==1)
	{
		j=fwrite(m_pMemval,sizeof(Valblk) * (m_maxvalnum + 2),1,fp2);
	}
	pthread_rwlock_unlock(&m_lockhash);
	fclose(fp1);
	fclose(fp2);
	return j;
}

template<typename K,typename V> int Mulhash<K,V>::Load(char *filenm)
{
	FILE *fp1=NULL;
	FILE *fp2=NULL;
	char fnm1[_MAX_PATH];
	char fnm2[_MAX_PATH];
	char fnm3[_MAX_PATH];
	char fnm4[_MAX_PATH];
	int j1=0;
	int j2=0;
	int j3=0;
	int j4=0;

	char header[16]={0};
	int j=0;
	int ret=0;
	int crc=0;
	int now=0;
	int now1=0;
	int maxnum=0;
	int bucknum=0;
	int curnum=0;

	if(NULL==filenm || m_maxkeynum<=0 || m_maxvalnum<=0 )
	{
		pLog->writelog("NULL parameter in Mulhash::Load!\n");
		return -1;
	}
	j1=SPRINT(fnm1,_MAX_PATH,"%s.idkey",filenm);
	j2=SPRINT(fnm2,_MAX_PATH,"%s.idval",filenm);
	j3=SPRINT(fnm3,_MAX_PATH,"%s.datakey",filenm);
	j4=SPRINT(fnm4,_MAX_PATH,"%s.dataval",filenm);
	if(j1<=0 || j2<=0 || j3<=0 || j4<=0)
	{
		pLog->writelog("Get filenm failed in Mulhash::Load!\n");
		return -2;
	}
	fp1=fopen(fnm3,"rb");
	fp2=fopen(fnm4,"rb");
	if(NULL==fp1 || NULL==fp2)
	{
		if(fp1) { fclose(fp1); fp1=NULL; }
		if(fp2) { fclose(fp2); fp2=NULL; }
		pLog->writelog("OPen files %s for write failed in Mulhash::Save!\n",filenm);
		return -3;
	}
	if((ret=m_pIDkey->restore(fnm1))<=0)
	{
		if(fp1) { fclose(fp1); fp1=NULL; }
		if(fp2) { fclose(fp2); fp2=NULL; }
		pLog->writelog("restore ID for key failed in Mulhash::Load %d!\n",ret);
		return -4;
	}
	if((ret=m_pIDval->restore(fnm2))<=0)
	{
		if(fp1) { fclose(fp1); fp1=NULL; }
		if(fp2) { fclose(fp2); fp2=NULL; }
		pLog->writelog("restore ID for val failed in Mulhash::Load %d!\n",ret);
		return -5;
	}
	/* now process files 1*/
	j=fread(header,4,1,fp1);
	if(j==1)
	{
		if(strncmp(header,"$$$$",4)!=0)
		{
			if(fp1) { fclose(fp1); fp1=NULL; }
			if(fp2) { fclose(fp2); fp2=NULL; }
			pLog->writelog("bad file format in Mulhash::Load(%s)!\n",fnm3);
			return -6;
		}
		j=fread(&now,sizeof(int),1,fp1);
	}
	if(j==1)
	{
		j=fread(&maxnum,sizeof(int),1,fp1);
	}
	if(j==1)
	{
		j=fread(&bucknum,sizeof(int),1,fp1);
	}
	if(j==1)
	{
		j=fread(&curnum,sizeof(int),1,fp1);
	}
	if(j==1)
	{
		j=fread(&crc,sizeof(int),1,fp1);
	}
	if(m_maxkeynum!=(unsigned int)maxnum)
	{
		pLog->writelog("Parameters changes in Mulhash::Load(%s) [new:maxkeynum:%d,buckkeynum:%d]!\n",fnm3,maxnum,bucknum);
	}
	if(crc != (maxnum ^ bucknum ^ curnum ^ now))
	{
		if(fp1) { fclose(fp1); fp1=NULL; }
		if(fp2) { fclose(fp2); fp2=NULL; }
		pLog->writelog("bad crc in Mulhash::Load(%s,crc:%d,should:%d)!\n",fnm3,crc,(maxnum ^ bucknum ^ curnum ^ now));
		return -7;
	}
	if(curnum<0 || maxnum<=10 || bucknum<=1)
	{
		if(fp1) { fclose(fp1); fp1=NULL; }
		if(fp2) { fclose(fp2); fp2=NULL; }
		pLog->writelog("unexpected parameters in Mulhash::Load(%s)!\n",fnm3);
		return -8;
	}
	m_curkeynum=curnum;
	m_maxkeynum = maxnum;
	m_buckkeynum = bucknum;
	/* now alloc memory parameters */
	m_pMemkey=(Keyblk *)calloc(m_maxkeynum + 2,sizeof(Keyblk));
    if(NULL==m_pMemkey)
    {
       	pLog->writelog("alloc key blkmem failed in Mulhash::Load\n");
       	return -9;
    }
    m_buckkeys=(u32 *)calloc(m_buckkeynum,sizeof(unsigned int));
    if(NULL==m_buckkeys)
    {
        pLog->writelog("alloc bucket key mem failed in Mulhash::Load\n");
        return -10;
    }
	if(j==1)
	{
		j=fread(header,8,1,fp1);
		if(strncmp(header,"$$$$$$$$",8)!=0)
		{
			if(fp1) { fclose(fp1); fp1=NULL; }
			if(fp2) { fclose(fp2); fp2=NULL; }
			pLog->writelog("bad file format2 in Mulhash::Load(%s)!\n",fnm3);
			return -11;
		}
	}
	if(j==1)
	{
		j=fread(m_buckkeys,sizeof(unsigned int) * m_buckkeynum,1,fp1);
	}
	if(j==1)
	{
		j=fread(m_pMemkey,sizeof(Keyblk) * (m_maxkeynum + 2),1,fp1);
	}
	if(j!=1)
	{
		if(fp1) { fclose(fp1); fp1=NULL; }
		if(fp2) { fclose(fp2); fp2=NULL; }
		pLog->writelog("read file failed in Mulhash::Load(%s)!\n",fnm3);
		return -12;
	}
	fclose(fp1);
	fp1=NULL;
	/* now file 2 */
	j=fread(header,4,1,fp2);
	if(j==1)
	{
		if(strncmp(header,"$$$$",4)!=0)
		{
			if(fp1) { fclose(fp1); fp1=NULL; }
			if(fp2) { fclose(fp2); fp2=NULL; }
			pLog->writelog("bad file format in Mulhash::Load(%s)!\n",fnm4);
			return -13;
		}
		j=fread(&now1,sizeof(int),1,fp2);
	}
	if(j==1)
	{
		if(now != now1)
		{
			if(fp1) { fclose(fp1); fp1=NULL; }
			if(fp2) { fclose(fp2); fp2=NULL; }
			pLog->writelog("time not much in Mulhash::Load(file:%s:time:%d;file:%s,time:%d)!\n",fnm3,now,fnm4,now1);
			return -13;
		}
		j=fread(&maxnum,sizeof(int),1,fp2);
	}
	if(j==1)
	{
		j=fread(&bucknum,sizeof(int),1,fp2);
	}
	if(j==1)
	{
		j=fread(&curnum,sizeof(int),1,fp2);
	}
	if(j==1)
	{
		j=fread(&crc,sizeof(int),1,fp2);
	}
	if(m_maxvalnum!=(unsigned int)maxnum)
	{
		pLog->writelog("Parameters changes2 in Mulhash::Load(%s) [new:maxvalnum:%d,buckvalnum:%d]!\n",fnm4,maxnum,bucknum);
	}
	if(crc != (maxnum ^ bucknum ^ curnum ^ now))
	{
		if(fp1) { fclose(fp1); fp1=NULL; }
		if(fp2) { fclose(fp2); fp2=NULL; }
		pLog->writelog("bad crc2 in Mulhash::Load(%s,crc:%d,should:%d)!\n",fnm4,crc,(maxnum ^ bucknum ^ curnum ^ now));
		return -14;
	}
	if(curnum<0 || maxnum<=10 || bucknum<=1)
	{
		if(fp1) { fclose(fp1); fp1=NULL; }
		if(fp2) { fclose(fp2); fp2=NULL; }
		pLog->writelog("unexpected parameters2 in Mulhash::Load(%s)!\n",fnm4);
		return -15;
	}
	m_curvalnum=curnum;
	m_maxvalnum = maxnum;
	m_buckvalnum = bucknum;
	/* now alloc memory parameters */
	m_pMemval=(Valblk *)calloc(m_maxvalnum + 2,sizeof(Valblk));
    if(NULL==m_pMemval)
    {
        pLog->writelog("alloc val blkmem failed in Mulhash::Load\n");
        return -16;
    }
	m_buckvals=(Idv *)calloc(m_buckvalnum,sizeof(Idv));
    if(NULL==m_buckvals)
    {
        pLog->writelog("alloc bucket val mem failed in Mulhash::Load\n");
        return -17;
    }
	if(j==1)
	{
		j=fread(header,8,1,fp2);
		if(strncmp(header,"$$$$$$$$",8)!=0)
		{
			if(fp1) { fclose(fp1); fp1=NULL; }
			if(fp2) { fclose(fp2); fp2=NULL; }
			pLog->writelog("bad file format2 in Mulhash::Load(%s)!\n",fnm4);
			return -18;
		}
	}
	if(j==1)
	{
		j=fread(m_buckvals,sizeof(Idv) * m_buckvalnum,1,fp2);
	}
	if(j==1)
	{
		j=fread(m_pMemval,sizeof(Valblk) * (m_maxvalnum + 2),1,fp2);
	}
	if(j!=1)
	{
		if(fp1) { fclose(fp1); fp1=NULL; }
		if(fp2) { fclose(fp2); fp2=NULL; }
		pLog->writelog("read file failed in Mulhash::Load(%s)!\n",fnm4);
		return -19;
	}
	fclose(fp2);
	fp2=NULL;
	return j;
}
#endif
