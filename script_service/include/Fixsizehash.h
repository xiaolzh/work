#ifndef __FIXSIZE_HASH_H_
#define __FIXSIZE_HASH_H_

#include "IDAlloc.h"
#include "log.h"

template<typename K,typename V> class Fixsizehash
{
public:
	struct Blk
	{
		K key;
		V val;
		unsigned int next;
	};
	Fixsizehash(int maxnum,int bucknum);
	~Fixsizehash();
	int Init(Log_file *plog);
	int Add(K key,V *val);
	int nolockAdd(K key,V *val);
	int Mod(K key,int off,int size,void *val);
	int IncVal(K key,V inc,V &val);
	int Del(K key);
	int Search(K key,V &val);
	int Count();
	void Clear();
	int Save(char *filenm);
	int Load(char *filenm);
	void GotoHead();
	int GetNext(K *pkey,V *pval);

private:
	/* lock */
	pthread_rwlock_t m_lockhash;
	/* blkmanager */
	IDAlloc *m_pID;
	Blk *m_pMemblk;
	/* buckets */
	unsigned int *m_buckets;
	int m_maxnum;
	int m_bucknum;
	int m_curnum;
	/* iter */
	int m_iteridx;
	Blk *m_iter;
	/* log */
	Log_file *pLog;
};

/* implementation */
template<typename K,typename V> Fixsizehash<K,V>::Fixsizehash(int maxnum,int bucknum)
{
	if(maxnum<=0 || bucknum<=0)
	{
		m_bucknum=4091;
		m_maxnum=8192;
	}
	else
	{
		m_maxnum=maxnum;
		m_bucknum=bucknum;
	}
	m_curnum=0;
	memset(&m_lockhash,0,sizeof(pthread_rwlock_t));
	m_pID=NULL;
	m_pMemblk=NULL;
	m_buckets=NULL;
	m_iter=NULL;
	m_iteridx=0;	
}

template<typename K,typename V> Fixsizehash<K,V>::~Fixsizehash()
{
	pthread_rwlock_destroy(&m_lockhash);
	memset(&m_lockhash,0,sizeof(pthread_rwlock_t));
	if(m_pID!=NULL)
	{
		delete m_pID;
		m_pID=NULL;
	}
	if(m_pMemblk!=NULL)
	{
		free(m_pMemblk);
		m_pMemblk=NULL;
	}
	if(m_buckets!=NULL)
	{
		free(m_buckets);
		m_buckets=NULL;
	}
}

template<typename K,typename V> int Fixsizehash<K,V>::Init(Log_file *plog)
{
	/* log */
	if(NULL==plog)
	{
		printf("Bad parameter in Fixsizehash::Init!\n");
		return -1;
	}
	pLog=plog;
	/* ini locks */
	if(pthread_rwlock_init(&m_lockhash,NULL))
    {
        printf("Init lock1 failed in Fixsizehash::Init!\n");
        pLog->writelog("Init lock1 failed in Fixsizehash::Init\n");
        return -2;
    }
	/* init memory blks */
	m_pID=new IDAlloc();
	if(NULL==m_pID)
	{
        printf("new idalloc failed in Fixsizehash::Init!\n");
        pLog->writelog("new idalloc failed in Fixsizehash::Init\n");
        return -4;
	}
	if(m_pID->init(m_maxnum)<=0)
	{
        printf("idalloc init failed in Fixsizehash::Init!\n");
        pLog->writelog("idalloc init failed in Fixsizehash::Init\n");
        return -5;
	}
	/* alloc memory */
	m_pMemblk=(Blk *)calloc(m_maxnum + 2,sizeof(Blk));
	//m_pMemblk=(Blk *)malloc(m_maxnum * sizeof(Blk));
	if(NULL==m_pMemblk)
	{
        printf("alloc blkmem failed in Fixsizehash::Init!\n");
        pLog->writelog("alloc blkmem failed in Fixsizehash::Init\n");
        return -7;
	}
	//memset(m_pMemblk,0xff,m_maxnum * sizeof(Blk));
	/* init buckets */
	m_buckets=(unsigned int *)calloc(m_bucknum,sizeof(unsigned int));
	if(NULL==m_buckets)
	{
        printf("alloc bucketmem failed in Fixsizehash::Init!\n");
        pLog->writelog("alloc bucketmem failed in Fixsizehash::Init\n");
        return -8;
	}
	return 1;
}

template<typename K,typename V>	void Fixsizehash<K,V>::GotoHead()
{
	m_iteridx = 0;
    m_iter = NULL;
}

template<typename K,typename V> int Fixsizehash<K,V>::GetNext(K *pkey,V *pval)
{
	unsigned int nxt=0;
	int ct=0;
	if(NULL!=m_iter)
	{
		if(m_iter->next != 0)
		{
			nxt = m_iter->next;
			m_iter = &m_pMemblk[nxt];
			memcpy(pkey,&m_iter->key,sizeof(K));
			memcpy(pval,&m_iter->val,sizeof(V));
		}
		else
		{
			ct=0;
			do
			{
				m_iteridx = (m_iteridx + 1) % m_bucknum;
				nxt = m_buckets[m_iteridx];
				ct++;
			}while(nxt==0 && ct<m_bucknum - 1);
			if(nxt==0)
			{
				return 0;
			}
			else
			{
				m_iter = &m_pMemblk[nxt];
				memcpy(pkey,&m_iter->key,sizeof(K));
				memcpy(pval,&m_iter->val,sizeof(V));
			}
		}
	}
	else
	{
		nxt=m_buckets[0];	
		ct=1;
		while(nxt==0 && ct<m_bucknum)
		{
			m_iteridx = (m_iteridx + 1) % m_bucknum;
			nxt = m_buckets[m_iteridx];
			ct++;
		}
		if(nxt==0)
		{
			return 0;
		}
		else
		{
			m_iter = &m_pMemblk[nxt];
			memcpy(pkey,&m_iter->key,sizeof(K));
			memcpy(pval,&m_iter->val,sizeof(V));
		}
	}
	return 1;
}

template<typename K,typename V> void Fixsizehash<K,V>::Clear()
{
	m_pID->reset();
	memset(m_pMemblk,0,(m_maxnum + 2)*sizeof(Blk));
	memset(m_buckets,0,m_bucknum*sizeof(unsigned int));
	m_curnum=0;
}

template<typename K,typename V> int Fixsizehash<K,V>::Add(K key,V *val)
{
	unsigned int id=0;
	unsigned int nxt=0;
	unsigned int lidx=(unsigned int)(key % m_bucknum);
	int ret=0;
	Blk *pblk;

	if(pthread_rwlock_wrlock(&m_lockhash))
	{
		pLog->writelog("require write lock failed in Fixsizehash::Add %d!\n",errno);
		return -1;
	}
	nxt=m_buckets[lidx];
	if(nxt==0)
	{
		/* alloc a block */
		ret=m_pID->getID(id);
		if(ret<=0)
		{
			pthread_rwlock_unlock(&m_lockhash);
			pLog->writelog("Warn:getBlkID failed in Fixsizehash::Add %d!\n",ret);
			return -2;
		}
		/* set parameters */
		m_pMemblk[id].key=key;
		memcpy(&m_pMemblk[id].val,val,sizeof(V));
		m_pMemblk[id].next=0;
		m_buckets[lidx]=id;
		m_curnum++;
	}
	else
	{
		/* try to locate */
		while(nxt)
		{
			pblk=&m_pMemblk[nxt];
			if(pblk->key==key)
			{
				memcpy(&(pblk->val),val,sizeof(V));	
				pthread_rwlock_unlock(&m_lockhash);
				return 1;
			}
			nxt = pblk->next;
		}
		/* not found,we should alloc one */
		ret=m_pID->getID(id);
		if(ret<=0)
		{
			pthread_rwlock_unlock(&m_lockhash);
			pLog->writelog("getBlkID failed2 in Fixsizehash::Add %d!\n",ret);
			return -3;
		}
		/* set parameters */
		m_pMemblk[id].key=key;
		memcpy(&m_pMemblk[id].val,val,sizeof(V));
		m_pMemblk[id].next=m_buckets[lidx];
		m_buckets[lidx]=id;
		m_curnum++;
	}
	pthread_rwlock_unlock(&m_lockhash);
	return 1;
}

template<typename K,typename V> int Fixsizehash<K,V>::nolockAdd(K key,V *val)
{
	unsigned int id=0;
	unsigned int nxt=0;
	unsigned int lidx=(unsigned int)(key % m_bucknum);
	int ret=0;
	Blk *pblk;

	nxt=m_buckets[lidx];
	if(nxt==0)
	{
		/* alloc a block */
		ret=m_pID->getID(id);
		if(ret<=0)
		{
			pLog->writelog("Warn:getBlkID failed in Fixsizehash::nolockAdd %d!\n",ret);
			return -1;
		}
		/* set parameters */
		m_pMemblk[id].key=key;
		memcpy(&m_pMemblk[id].val,val,sizeof(V));
		m_pMemblk[id].next=0;
		m_buckets[lidx]=id;
		m_curnum++;
	}
	else
	{
		/* try to locate */
		while(nxt)
		{
			pblk=&m_pMemblk[nxt];
			if(pblk->key==key)
			{
				memcpy(&(pblk->val),val,sizeof(V));	
				return 1;
			}
			nxt = pblk->next;
		}
		/* not found,we should alloc one */
		ret=m_pID->getID(id);
		if(ret<=0)
		{
			pLog->writelog("getBlkID failed2 in Fixsizehash::nolockAdd %d!\n",ret);
			return -2;
		}
		/* set parameters */
		m_pMemblk[id].key=key;
		memcpy(&m_pMemblk[id].val,val,sizeof(V));
		m_pMemblk[id].next=m_buckets[lidx];
		m_buckets[lidx]=id;
		m_curnum++;
	}
	return 1;
}

template<typename K,typename V> int Fixsizehash<K,V>::Mod(K key,int off,int size,void *val)
{
	unsigned int nxt=0;
	unsigned int lidx=(unsigned int)(key % m_bucknum);
	char *buff=NULL;
	Blk *pblk;

	if(off<0 || size<=0 || size>(int)sizeof(V))
	{
		pLog->writelog("Bad parameter in Fixsizehash::Mod(%d,%d)!\n",off,size);
		return -1;
	}
	if(pthread_rwlock_wrlock(&m_lockhash))
	{
		pLog->writelog("require write lock failed in Fixsizehash::Mod %d!\n",errno);
		return -2;
	}
	nxt=m_buckets[lidx];
	if(nxt==0)
	{
		/* no such a block */
		pthread_rwlock_unlock(&m_lockhash);
		pLog->writelog("Block not found1 in Fixsizehash::Mod %lu!\n",key);
		return 0;
	}
	else
	{
		/* try to locate */
		while(nxt)
		{
			pblk=&m_pMemblk[nxt];
			if(pblk->key==key)
			{
				buff = (char *)(&(pblk->val));
				buff += off;
				memcpy(buff,val,size);	
				pthread_rwlock_unlock(&m_lockhash);
				return 1;
			}
			nxt = pblk->next;
		}
		/* not found such a block */
		pthread_rwlock_unlock(&m_lockhash);
		pLog->writelog("Block not found2 in Fixsizehash::Mod %lu!\n",key);
		return 0;
	}
}

template<typename K,typename V> int Fixsizehash<K,V>::Del(K key)
{
	unsigned int nxt=0;
	unsigned int nxt1=0;
	unsigned int lidx=(unsigned int)(key % m_bucknum);

	if(pthread_rwlock_wrlock(&m_lockhash))
	{
		pLog->writelog("require write lock failed in Fixsizehash::Del %d!\n",errno);
		return -1;
	}
	nxt=m_buckets[lidx];
	if(nxt!=0)
	{
		/* try to locate,first */
		if(m_pMemblk[nxt].key==key)
		{
			m_buckets[lidx]=m_pMemblk[nxt].next;
			m_pID->freeID(nxt);
			m_curnum--;
			pthread_rwlock_unlock(&m_lockhash);
			return 1;
		}
		nxt1=nxt;
		nxt=m_pMemblk[nxt].next;
		while(nxt)
		{
			if(m_pMemblk[nxt].key==key)
			{
				m_pMemblk[nxt1].next=m_pMemblk[nxt].next;
				m_pID->freeID(nxt);
				m_curnum--;
				pthread_rwlock_unlock(&m_lockhash);
				return 1;
			}
			nxt1=nxt;
			nxt=m_pMemblk[nxt].next;
		}
	}
	/* not found */
	pthread_rwlock_unlock(&m_lockhash);
	return 0;
}

template<typename K,typename V> int Fixsizehash<K,V>::Search(K key,V &val)
{
	unsigned int nxt=0;
	unsigned int lidx=(unsigned int)(key % m_bucknum);

	if(pthread_rwlock_rdlock(&m_lockhash))
	{
		pLog->writelog("require read lock failed in Fixsizehash::Search %d!\n",errno);
		return -1;
	}
	nxt=m_buckets[lidx];
	while(nxt)
	{
		if(m_pMemblk[nxt].key==key)
		{
			memcpy(&val,&m_pMemblk[nxt].val,sizeof(V));
			pthread_rwlock_unlock(&m_lockhash);
			return 1;
		}
		nxt=m_pMemblk[nxt].next;
	}
	pthread_rwlock_unlock(&m_lockhash);
	return 0;
}

template<typename K,typename V> int Fixsizehash<K,V>::IncVal(K key,V inc,V &val)
{
	unsigned int nxt=0;
	unsigned int lidx=(unsigned int)(key % m_bucknum);

	if(pthread_rwlock_wrlock(&m_lockhash))
	{
		pLog->writelog("require read lock failed in Fixsizehash::IncVal %d!\n",errno);
		return -1;
	}
	nxt=m_buckets[lidx];
	while(nxt)
	{
		if(m_pMemblk[nxt].key==key)
		{
			m_pMemblk[nxt].val += inc;
			val = m_pMemblk[nxt].val;
			pthread_rwlock_unlock(&m_lockhash);
			return 1;
		}
		nxt=m_pMemblk[nxt].next;
	}
	pthread_rwlock_unlock(&m_lockhash);
	return 0;
}

template<typename K,typename V> int Fixsizehash<K,V>::Count()
{
	return m_curnum;
}

template<typename K,typename V> int Fixsizehash<K,V>::Save(char *filenm)
{
	FILE *fp=NULL;
	char fname[MAX_FILE_PATH] = {0};
	int j=0;
	int ret=0;
	int crc=0;
	int now=0;
	int iLen=0;
	/* first open files */
	if(NULL==filenm)
	{
		pLog->writelog("NULL parameter in Fixsizehash::Save!\n");
		return -1;
	}
	iLen = strlen(filenm);
	if (strncmp(filenm+iLen- 4, ".bak", 4) == 0)
	{
		filenm[iLen-4]='\0';
		j=_snprintf(fname,MAX_FILE_PATH,"%s.idm.bak",filenm);	
	}
	else
	{
		j=_snprintf(fname,MAX_FILE_PATH,"%s.idm",filenm);
	}
	if(j<=0)
	{
		pLog->writelog("Get filenm failed in Fixsizehash::Save!\n");
		return -2;
	}
	fp=fopen(filenm,"wb");
	if(NULL==fp)
	{
		pLog->writelog("OPen file %s for write failed in Fixsizehash::Save!\n",filenm);
		return -3;
	}
	/* lock */
	if(pthread_rwlock_rdlock(&m_lockhash))
	{
		fclose(fp);
		pLog->writelog("require read lock failed in Fixsizehash::Save %d!\n",errno);
		return -4;
	}
	/* write */
	if((ret=m_pID->save(fname))<=0)
	{
		pthread_rwlock_unlock(&m_lockhash);
		fclose(fp);
		pLog->writelog("save ID failed in Fixsizehash::Save %d!\n",ret);
		return -5;
	}
	/* write header */
	j=fwrite("$$$$",4,1,fp);
	now=(int)time(NULL);
	if(j==1)
	{
		j=fwrite(&now,sizeof(int),1,fp);
	}
	if(j==1)
	{
		j=fwrite(&m_maxnum,sizeof(int),1,fp);
	}
	if(j==1)
	{
		j=fwrite(&m_bucknum,sizeof(int),1,fp);
	}
	if(j==1)
	{
		j=fwrite(&m_curnum,sizeof(int),1,fp);
	}
	crc = m_maxnum ^ m_bucknum ^ m_curnum ^ now;
	if(j==1)
	{
		j=fwrite(&crc,sizeof(int),1,fp);
	}
	if(j==1)
	{
		j=fwrite("$$$$$$$$",8,1,fp);
	}
	/* write bucket idx */
	if(j==1)
	{
		j=fwrite(m_buckets,sizeof(unsigned int) * m_bucknum,1,fp);
	}
	/* write body */	
	if(j==1)
	{
		j=fwrite(m_pMemblk,sizeof(Blk) * (m_maxnum + 2),1,fp);
	}
	/* unlock */
	pthread_rwlock_unlock(&m_lockhash);
	/* close files */
	fclose(fp);
	return j;
}

template<typename K,typename V> int Fixsizehash<K,V>::Load(char *filenm)
{
	FILE *fp=NULL;
	char fname[MAX_FILE_PATH];
	char header[16]={0};
	int j=0;
	int ret=0;
	int crc=0;
	int now=0;
	int maxnum=0;
	int bucknum=0;
	int curnum=0;
	/* first open files */
	if(NULL==filenm || m_maxnum<=0 || m_buckets==NULL || m_pMemblk==NULL)
	{
		pLog->writelog("NULL parameter in Fixsizehash::Load!\n");
		return -1;
	}
	j=_snprintf(fname,MAX_FILE_PATH,"%s.idm",filenm);	
	if(j<=0)
	{
		pLog->writelog("Get filenm failed in Fixsizehash::Load!\n");
		return -2;
	}
	fp=fopen(filenm,"rb");
	if(NULL==fp)
	{
		pLog->writelog("OPen file %s for read failed in Fixsizehash::Load!\n",filenm);
		return -3;
	}
	/* restore */
	if((ret=m_pID->restore(fname))<=0)
	{
		fclose(fp);
		pLog->writelog("%s:%d restore ID failed in Fixsizehash::Load %d!\n",FL, LN, ret);
		return -5;
	}
	/* read header */
	j=fread(header,4,1,fp);
	if(j==1)
	{
		if(strncmp(header,"$$$$",4)!=0)
		{
			fclose(fp);
			pLog->writelog("bad file format in Fixsizehash::Load(%s)!\n",fname);
			return -6;
		}
		j=fread(&now,sizeof(int),1,fp);
	}
	if(j==1)
	{
		j=fread(&maxnum,sizeof(int),1,fp);
	}
	if(j==1)
	{
		j=fread(&bucknum,sizeof(int),1,fp);
	}
	if(j==1)
	{
		j=fread(&curnum,sizeof(int),1,fp);
	}
	if(j==1)
	{
		j=fread(&crc,sizeof(int),1,fp);
	}
	if(m_maxnum!=maxnum || m_bucknum!=bucknum)
	{
		fclose(fp);
		pLog->writelog("not the expected file in Fixsizehash::Load(%s),expect maxnum:%u[%u],buck:%u[%u]!\n",fname,m_maxnum,maxnum,m_bucknum,bucknum);
		return -7;
	}
	if(crc != (maxnum ^ bucknum ^ curnum ^ now))
	{
		fclose(fp);
		pLog->writelog("bad crc in Fixsizehash::Load(%s,crc:%d,should:%d)!\n",fname,crc,(maxnum ^ bucknum ^ curnum ^ now));
		return -8;
	}
	if(m_curnum<0 || m_maxnum<=10 || m_bucknum<=1)
	{
		fclose(fp);
		pLog->writelog("unexpected parameters in Fixsizehash::Load(%s)!\n",fname);
		return -9;
	}
	m_curnum=curnum;
	if(j==1)
	{
		j=fread(header,8,1,fp);
		if(strncmp(header,"$$$$$$$$",8)!=0)
		{
			fclose(fp);
			pLog->writelog("bad file format2 in Fixsizehash::Load(%s)!\n",fname);
			return -10;
		}
	}
	/* read bucket idx */
	if(j==1)
	{
		j=fread(m_buckets,sizeof(unsigned int) * m_bucknum,1,fp);
	}
	/* read body */	
	if(j==1)
	{
		j=fread(m_pMemblk,sizeof(Blk) * (m_maxnum + 2),1,fp);
	}
	fclose(fp);
	return j;
}
#endif
