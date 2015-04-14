#include "ta_dic_w.h"
#include <iostream>
#include <queue>
#include <sstream>
#include <algorithm>

#ifdef __GNUC__
static const std::string space = " ";
static const size_t spsz = space.size();

inline bool compare_pair_val(
	const std::pair<short, short>& lft, 
	const std::pair<short, short>& rht)
{
	return lft.second > rht.second;
}

#else
static const std::string space = " ";
static const size_t spsz = space.size();

inline bool compare_pair_val(
	const std::pair<short, short>& lft, 
	const std::pair<short, short>& rht)
{
	return lft.second > rht.second;
}
#endif

segment::CDic_w::CDic_w(void)
{
}

segment::CDic_w::~CDic_w(void)
{
	ClearData();
}

bool segment::CDic_w::AmbKeyFilter(
	const VEC_PAIR_STR_INT& In, 
	const std::string& infile, 
	const std::string& outfile)
{	std::ifstream fin(infile.c_str());
	if (!fin.is_open())
	{
		std::cerr << infile << " open failed! "
			<< "Func(AmbKeyFilter) error." << std::endl;
		return false;
	}
	
	//load&sort
	MAP_STR_INT mKey2Seg;
	{	
		std::string line;
		while (getline(fin, line))
		{
			size_t pos = line.find(space);
			if (0 == pos || std::string::npos == pos)
			{
				std::cerr << infile << " format error,should be 'term" 
					<< space << "weight'. Func(AmbKeyFilter) error" << std::endl;
				return false;
			}
			mKey2Seg[line.substr(0,pos)] = 
				atoi(line.substr(pos+spsz,line.size()-pos-spsz).c_str());
		}
	}
	
	//filter&output
	VEC_STR Out;
	if (false == CreateLexIdx(In, Out))
	{
		std::cerr << "Func(AmbKeyFilter) error" << std::endl;
		return false;
	}
	
	std::ofstream fout(outfile.c_str(), std::ios::binary);
	if (!fout.is_open())
	{
		std::cerr << outfile << " open failed! "
			<< "Func(AmbKeyFilter) error" << std::endl;
		return false;
	}
	
	std::string childTerm;
	const int LEN = ((WORDCNTMAX_CH + KEYCNTMAX)<<1) * sizeof(short);
	char buff[LEN + 1];
	MAP_STR_INT::const_iterator cur = mKey2Seg.begin();
	for (; cur != mKey2Seg.end(); ++cur)
	{
		std::string term = cur->first;
		int info = cur->second >> AMBOFFSET;
		bool hasChild = false;
		{
			if (false == childTerm.empty() && 
				term.substr(0, childTerm.size()) == childTerm)
			{
				hasChild = true;
			}
			else 
			{
				childTerm = term;
			}
		}
		std::string amb_seg, nor_seg;
		{	
			size_t beg = 0;
			while (info) 
			{
				size_t len = 2;
				while (!(info & 1)) 
				{
					info >>= 1; 
					len += 2;
				}
				amb_seg += term.substr(beg, len) +  "/";
				beg += len;
				info >>= 1; 
			}
			
			std::vector<struct Term> vSeg;	
			const char* str = term.c_str();		
			
			if (false == OmniSegment(str, str+strlen(str), vSeg, buff, LEN+1))
			{
				std::cerr << "segment space not enough! "
					<< "Func(AmbKeyFilter) error" << std::endl;
				return false;
			}
			for (size_t i = 0; i < vSeg.size(); ++i) 
			{
				nor_seg += term.substr(vSeg[i].pos, vSeg[i].len) + "/";
			}	
		}		
		if (amb_seg != nor_seg || hasChild) 
		{
			fout << term << space << cur->second << std::endl;
		}
	}
	return true;
}

bool segment::CDic_w::BuildDATrie()
{	
    if (Trie.empty()) 
	{
		std::cerr << "Trie is empty! Func(BuildDATrie) error." << std::endl;
		return false;
	}
	m_viBase.clear(); 
	m_viCheck.clear(); 
	m_viWeight.clear();

	std::vector<std::pair<short,short> > vpWord2SubCnt; 
	{
		for (size_t i = 0; i < Trie.size(); ++i) 
		{
			if (Trie[i]) 
			{
				vpWord2SubCnt.push_back( 
					std::pair<short, short>((short)i, Trie[i]->front().wSubCnt));
			}
		}
		sort(vpWord2SubCnt.begin(),vpWord2SubCnt.end(),compare_pair_val); 
	}
	size_t fwSize = vpWord2SubCnt.size();  


	m_viBase.resize(SHRT_MAX);	
	m_viCheck.resize(SHRT_MAX);
	m_viWeight.resize(SHRT_MAX);

	std::priority_queue<PosCnt, std::vector<PosCnt> > pq; 
	PosCnt pc;					
	PosCnt pcSub;				
	TRIE_NODE* pNodeFrst;	
	short wBeg, wEnd;			

	int arraySz = (int)m_viBase.size();  
	int nSquence;			
	int t;					
	int nBaseVal;			
	bool bCheck;			
	VEC_INT viEndTag;	

	for (size_t i = 0; i < fwSize; ++i)	
	{
		nSquence = vpWord2SubCnt[i].first;
		pNodeFrst = &(Trie[nSquence]->front());	

		if(!pNodeFrst->wSubCnt)						
		{	
			m_viBase[nSquence] = -nSquence;
			m_viWeight[nSquence] = pNodeFrst->weight;
		}
		else 
		{
			m_viCheck[nSquence] = 0;
			pc.nCnt = pNodeFrst->wSubCnt;
			pc.nPosInTrie = 0;	//
			pc.nPosInBase = nSquence;
			pq.push(pc);	

			while (!pq.empty()) 
			{	
				pc = pq.top();
				pq.pop();
				
				int base = pc.nPosInBase;
				short pos =  pc.nPosInTrie;
				short subCnt = pc.nCnt;
				if (subCnt)	
				{
					wBeg = pNodeFrst[pos].wSubBeg + pos;	
					wEnd = wBeg + pNodeFrst[pos].wSubCnt;	

					if (pNodeFrst[pos].bEnd) 
					{	
						viEndTag.push_back(base);			
						m_viWeight[base] = pNodeFrst[pos].weight;
						if (pNodeFrst[pos].weight >> AMBOFFSET)
						{	
							m_viCheck[base] *= -1;			
						}
					}

					nBaseVal = base;

					
					do 
					{
						bCheck = true;
						for (short j = wBeg; j < wEnd; ++j)		
						{
							short c = pNodeFrst[j].wData;
							t = nBaseVal + c;	

							if(t <= FRST_OFFSET)				
							{	
								nBaseVal = FRST_OFFSET + 1 - c;								
								bCheck = false;
								break;	
							}
							if (t >= arraySz)					
							{
								m_viBase.resize(t + SHRT_MAX);
								m_viCheck.resize(t + SHRT_MAX);
								m_viWeight.resize(t + SHRT_MAX);
								arraySz = t + SHRT_MAX;
							}
							if (m_viCheck[t])					
							{	
								bCheck = false;
								++nBaseVal;															
								break;								
							}
						}

						if ( bCheck && (nBaseVal == base) &&	
							 pNodeFrst[pos].bEnd )		
						{	
							++nBaseVal;	
							bCheck = false;
						}
					}while (!bCheck);

					m_viBase[base] = nBaseVal;			
					for (short j = wBeg; j < wEnd; ++j)	
					{
						
						t = nBaseVal + pNodeFrst[j].wData;
						assert(t > FRST_OFFSET);
						m_viCheck[t] = base;

						pcSub.nCnt = pNodeFrst[j].wSubCnt;
						pcSub.nPosInTrie = j;
						pcSub.nPosInBase = t;
						pq.push(pcSub);
					}	
				}
				else 
				{	 
					m_viBase[base] = -base;
					m_viWeight[base] = pNodeFrst[pos].weight;
					if (pNodeFrst[pos].weight >> AMBOFFSET)
					{	
						m_viCheck[base] *= -1;			
					}
				}
			}
		}
	}


	size_t sz = viEndTag.size();
	for (size_t i = 0; i < sz; ++i) 
	{
		m_viBase[viEndTag[i]] *= -1;
	}
	return true;
}

void segment::CDic_w::BuildTrie(
	const std::vector<struct TERM_CODE>& vKeyCode)
{	
	if (!Trie.empty()) 
	{
		std::vector<std::vector<TRIE_NODE>* >::iterator it;
		for (it = Trie.begin(); it != Trie.end(); ++it)
		{
			delete *it;
		}		
		Trie.clear();
	}
	Trie.resize(FRST_OFFSET + 1);	
	
	std::vector<TERM_CODE>::const_iterator itF;	
	std::vector<TERM_CODE>::const_iterator it;
	std::vector<std::vector<TERM_CODE>::const_iterator> vecIter; 
	std::vector<std::vector<TERM_CODE>::const_iterator>::iterator itVecIter;

	short wordIndex;			
	short wordFrst;				
	short wordCur;			
	short preLevelWordFrst;	
	short preLevelWordCur;	
	short nLevel;				
	int nPreLevelPosFrst;	
	int nlevelCount;		

	TRIE_NODE treeNode;
	
	
	hash_map<int, short> hmTree; 
	int nKey;					 

	for (it = vKeyCode.begin(); it != vKeyCode.end();) 
	{	
		itF = it;
		wordIndex = it->code[0]; 

		memset(&treeNode, 0, sizeof treeNode);
		treeNode.wData = wordIndex; 
		Trie[wordIndex] = new std::vector<TRIE_NODE>;
		Trie[wordIndex]->push_back(treeNode);

		
		while (it != vKeyCode.end() && it->code[0] == wordIndex)
		{	
			if (it->code.size() > 1) 
			{	
				vecIter.push_back(it);
			}	
			else									
			{	
				assert(0 == (int)(it - itF));
				nKey = (int)(it - itF) * WORDCNTMAX; 
				hmTree.insert(std::pair<int, short>(nKey, 0)); 

				Trie[wordIndex]->front().bEnd = true;	
				Trie[wordIndex]->front().weight = it->weight;
			}		
			++it;
		}

		
		nLevel = 1;					
		
		while (!vecIter.empty()) 
		{   	
			wordFrst = 0;
			nlevelCount = 0;
			preLevelWordFrst = 0;
			nPreLevelPosFrst = -1;

			for (itVecIter = vecIter.begin(); itVecIter != vecIter.end();) 
			{	
				preLevelWordCur = (*itVecIter)->code[nLevel - 1]; 
				wordCur = (*itVecIter)->code[nLevel];						
				
				nKey = (int)((*itVecIter) - itF) * WORDCNTMAX + nLevel - 1; 
				
				if ( preLevelWordCur != preLevelWordFrst || 
					(preLevelWordCur == preLevelWordFrst && 
					hmTree[nKey] != hmTree[nPreLevelPosFrst]) )	
				{	
							
					(*Trie[wordIndex])[hmTree[nKey]].wSubCnt = 1; 
					(*Trie[wordIndex])[hmTree[nKey]].wSubBeg = 
						(short)Trie[wordIndex]->size() - hmTree[nKey];

					preLevelWordFrst = preLevelWordCur;	
					wordFrst = 0;	
				}
				else							
				{	
					if (wordCur != wordFrst)	
					{
						++(*Trie[wordIndex])[hmTree[nKey]].wSubCnt;			
					}
				}

				
				nKey = (int)((*itVecIter) - itF) * WORDCNTMAX + nLevel; 

				if (wordCur != wordFrst)
				{	
					++nlevelCount;
					wordFrst = wordCur;

					memset(&treeNode, 0, sizeof treeNode);
					treeNode.wData = wordCur; 
					
					hmTree.insert(
						std::pair<int, short>(nKey, (short)Trie[wordIndex]->size())); 
					nPreLevelPosFrst = 
						(int)((*itVecIter)-itF) * WORDCNTMAX + nLevel - 1; 

					if ((int)(*itVecIter)->code.size() == nLevel + 1)				
					{	
						treeNode.bEnd = true;
						treeNode.weight = (*itVecIter)->weight;						
						itVecIter = vecIter.erase(itVecIter);
					}
					else
					{
						++itVecIter;
					}
					Trie[wordIndex]->push_back(treeNode);						
				}
				else	
				{	
					
					assert((int)(*itVecIter)->code.size() > nLevel + 1);
					++itVecIter;
					
					hmTree.insert(
						std::pair<int, short>(nKey, (short)Trie[wordIndex]->size() - 1)); 
				}							
			}
			++nLevel;
		}
		hmTree.clear();				
	}
}

inline bool segment::CDic_w::SegBoundCheck(short c1, short c2)
{	
	assert(!c1 || (c1>=FRST_OFFSET_CH+1 && c1<=FRST_OFFSET_CH+95));
	assert(!c2 || (c2>=FRST_OFFSET_CH+1 && c2<=FRST_OFFSET_CH+95));
	
	if (c1)
	{
		c1 -= FRST_OFFSET_CH - 31;
	}
	if (c2)
	{
		c2 -= FRST_OFFSET_CH - 31;
	}
	
	if ((isalpha(c1) && isalpha(c2)) ||
		(isdigit(c1) && isdigit(c2)) )
	{
		return false;
	}
	else 
	{
		return true;
	}
}

void segment::CDic_w::ClearData()	
{	
	if (!Trie.empty()) 
	{
		std::vector<std::vector<TRIE_NODE>* >::iterator it;
		for (it = Trie.begin(); it != Trie.end(); ++it)
		{
			delete *it;
		}		
		Trie.clear();
	}
}

bool segment::CDic_w::CreateLexIdx(
	const VEC_PAIR_STR_INT& InFile, const VEC_STR& OutFile)
{	
	MAP_STR_INT mKey2Weight;	
	{	
		for (size_t i = 0; i < InFile.size(); ++i) 
		{
			if (false == LoadLexicon(InFile[i], mKey2Weight))
			{
				std::cerr << "Func(CreateLexIdx) error." << std::endl;
				return false;
			}
		}
		if (mKey2Weight.empty())
		{
			std::cerr << "Lexicon empty! Func(CreateLexIdx) error." << std::endl;
			return false;
		}
	}
	
	
	size_t keyCnt = mKey2Weight.size();
	std::vector<struct TERM_CODE>  vKeyCode(keyCnt);
	{
		std::vector<struct TERM_CODE>::iterator vit = vKeyCode.begin();
		MAP_STR_INT::const_iterator mit = mKey2Weight.begin();
		
		while (vit != vKeyCode.end() && mit != mKey2Weight.end())
		{
			if (false == GetKeyCode(vit->code, *mit))
			{
				std::cerr << "Func(CreateLexIdx) error." << std::endl;
				return false;
			}
			(vit++)->weight = (mit++)->second;
		}
	}		
	
	
	{
		BuildTrie(vKeyCode);
		if (!BuildDATrie()) 
		{
			std::cerr << "double array Trie build failed." 
				<< "Func(CreateLexIdx) error." << std::endl;
			return false; 
		}
	}
	

	if (2 == OutFile.size()) 
	{
		std::ofstream fdat(OutFile[0].c_str(), std::ios::binary);
		std::ofstream fmap(OutFile[1].c_str(), std::ios::binary);
		
		DaTrie daTrie;
		std::vector<int>::reverse_iterator it = m_viBase.rbegin();
		while (it < m_viBase.rend() && !*it)
		{
			++it;
		}
		int arrSz = (int)(m_viBase.rend() - it);	
		for (int i = 0; i < arrSz; ++i)
		{
			daTrie.base = m_viBase[i];
			daTrie.check = m_viCheck[i];
			daTrie.weight = m_viWeight[i];
			fdat.write((char*)(&daTrie), sizeof(DaTrie));
		}

		KeyMap keyMap;
		hash_map<unsigned short, short>::const_iterator hit = hmCode2Idx.begin();
		for (; hit != hmCode2Idx.end(); ++hit)
		{
			keyMap.code = hit->first;
			keyMap.index = hit->second;
			fmap.write((char*)(&keyMap), sizeof(KeyMap));
		}
	}
	else
	{
		std::cout << "Warning: Lexicon no output file." << std::endl;
	}


	return true;
}

inline short segment::CDic_w::GetCHCode(	
	unsigned char ch1, unsigned char ch2)
{	//get chinese encode
	if (IsGB2312(ch1, ch2))
	{
		return (ch1 - 176) * 94 + (ch2 - 161) + 1;
	}
	else
	{
		unsigned short c = ch1 << 8 | ch2;
		return hmCode2Idx.find(c) != hmCode2Idx.end() ?
			(hmCode2Idx[c] + GB2312_OFFSET + 1) : 0; 
	}
}

inline short segment::CDic_w::GetENCode(char ch)
{	//get half-wild encode
	if (ch >= 32 && ch <= 126)
	{
		return (short)ch - 32 + FRST_OFFSET_CH + 1;
	}
	else
	{
		return 0;
	}
}

bool segment::CDic_w::GetKeyCode(
	std::vector<short>& vCode, const PAIR_STR_INT& term_pair)
{
	std::string term = term_pair.first;
	int weight = term_pair.second;
	vCode.clear();

	if (weight == english)
	{	
		for (size_t i = 0; i < term.size(); ++i)
		{
			if (term[i] >= 32 && term[i] <= 126)
			{	
				vCode.push_back((short)term[i] - 32 + FRST_OFFSET_CH + 1);	
			}
			else
			{
				std::cerr << "key code out_of_range:" << term
					<< " should be in 32-126" << std::endl;
				return false;
			}
		}
	}
	else
	{	
		int length = (int)term.size();
		int k = 0;
		static int hashIdx = 0;

		while (k < length - 1) 
		{
			unsigned char ch1 = term[k];
			unsigned char ch2 = term[k + 1];

			short code = 0;
			if (true == IsGB2312(ch1, ch2))	
			{
				code = (ch1 - 176) * 94 + (ch2 - 161) + 1;
			}
			else if (ch1 > 127)
			{	
				unsigned short c = (ch1 << 8) | ch2;
				if (hmCode2Idx.find(c) == hmCode2Idx.end()) 
				{
					hmCode2Idx[c] = hashIdx++;
				}
				code = hmCode2Idx[c] + GB2312_OFFSET + 1;
			}
			else
			{
				std::cerr << "compose non-chinese:" << term << std::endl;
				return false;
			}
			if (code > FRST_OFFSET_CH || code <= 0) 
			{
				std::cerr << "beyond first-word code range:" << code
					<< " should be in 1-"<< FRST_OFFSET_CH << std::endl;
				return false;
			}
			vCode.push_back(code);
			k += 2;
		}
	}
	return true;
}

inline bool segment::CDic_w::IsGB2312(
	unsigned char ch1, unsigned char ch2)
{	//already gbk chinese,judge if gb2312 chinese
	return (ch1 >= 176) && (ch1 <= 247) && (ch2 >= 161);
}

inline bool segment::CDic_w::IsGBKCH(
	unsigned char ch1, unsigned char ch2)
{	//judge if gbk chinese
	return (  (ch2 >= 64) && (ch2 <= 254) && (ch2 != 127) &&
		( (ch1 >= 129 && ch1 <= 160) || (ch1 >= 170 && ch1 < 254) ||
		(ch1 == 254 && ch2 <= 160) )  );
}

bool segment::CDic_w::Load(
	const std::string& fdat, const std::string& fmap)
{	
	std::ifstream fDat(fdat.c_str(), std::ios::binary);
	std::ifstream fMap(fmap.c_str(), std::ios::binary);
	if (!fDat.is_open() || !fMap.is_open()) 
	{
		std::cerr << "Lexicon index files are opened failed." 
			<< "Func(Load) error." << std::endl;
		return false;
	}

	DaTrie dat;
	while(!fDat.eof()) 
	{
		fDat.read((char*)(&dat),sizeof(dat));
		m_viBase.push_back(dat.base);
		m_viCheck.push_back(dat.check);
		m_viWeight.push_back(dat.weight);
	}
	fDat.close();

	KeyMap kmp;
	while(!fMap.eof()) 
	{
		fMap.read((char*)(&kmp),sizeof(kmp));
		hmCode2Idx.insert(std::make_pair(kmp.code,kmp.index));
	}
	fMap.close();

	if (m_viBase.empty() || hmCode2Idx.empty()) 
	{
		std::cerr << "Double-array or hashmap is empty!" 
			<< "Func(Load) error." << std::endl;
		return false;
	}
	
	
	if (hmChinese.empty())
	{
		LoadChineseTable();
	}
	return true;
}

bool segment::CDic_w::LoadChineseTable()
{		
/*	std::vector<std::string> words;
	size_t words_size = sizeof(complex_2_simplified_chinese_table) / sizeof(char*);	
	for (size_t i = 0; i < words_size; ++i)
	{
		words.push_back(complex_2_simplified_chinese_table[i]);
	}

	std::string chTraditional, chSimple;
	for(size_t i = 0; i < words.size(); ++i)
	{
		std::istringstream stream(words[i]);		
		stream >> chTraditional;
		stream >> chSimple;
	 
		if (chTraditional.length() != 2 || chSimple.length() != 2)
		{
			std::cerr << "Func(LoadChineseTable) error!" << std::endl;
			return false;
		}
		hmChinese[chTraditional]=chSimple;
	}	 */
	return true;
}

bool segment::CDic_w::LoadLexicon(
	const PAIR_STR_INT& lexicon, MAP_STR_INT& mKey2W)
{	
	if (hmChinese.empty())
	{
		LoadChineseTable();
	}

	std::ifstream fin(lexicon.first.c_str());
	if (!fin.is_open())
	{
		std::cerr << lexicon.first << " open failed! "
			<< "Func(LoadChLexicon) error." << std::endl;
		return false;
	}

	switch(lexicon.second)
	{
		case ambiguity:
		{	
			std::string line;
			while (getline(fin, line))
			{
				size_t pos = line.find(space);
				if (!pos || std::string::npos == pos)
				{
					std::cerr << "amb-file format error, must 'term" << space 
						<< "weight'. Func(LoadChLexicon) error." << std::endl;
					return false;
				}
				
				
				std::string term = line.substr(0, pos);	
				if ( true == NormalizeCHKey(term) &&
					( mKey2W.find(term) == mKey2W.end() ||
					 !(mKey2W[term] >> AMBOFFSET) ) )
				{	
					mKey2W[term] += atoi(
						line.substr(pos + spsz, line.size() - pos - spsz).c_str());
				}
			}
		}
		break;
		case common: case special:
		{	
			std::string line;
			while (getline(fin, line))
			{
				
				if ( true == NormalizeCHKey(line) &&
					( mKey2W.find(line) == mKey2W.end() ||
					 (mKey2W[line] >> AMBOFFSET) ) )
				{	
					mKey2W[line] += lexicon.second;
				}
			}
		}	
		break;
		case english:
		{	
			std::string line;
			while (getline(fin, line))
			{
				if (true == NormalizeEnKey(line))
				{
					mKey2W[line] = english;
				}
			}
		}
		break;
	}
	return true;
}

bool segment::CDic_w::NormalizeCHKey(std::string& term)
{	
	size_t size = term.size();
	if (size && !(size & 1) && (size <= (WORDCNTMAX_CH<<1)))
	{
		size_t i = 0;
		while (i < size && 
			IsGBKCH((unsigned char)term[i], (unsigned char)term[i+1]))
		{
			std::string word = term.substr(i, 2);
			if (hmChinese.find(word) != hmChinese.end())
			{	
				term[i] = hmChinese[word][0];
				term[i+1] = hmChinese[word][1];
			}
			i += 2;
		}
		if (i == size)
		{	
			return true;
		}
	}
	return false;
}

bool segment::CDic_w::NormalizeEnKey(std::string& term)
{	
	static const int BUFF = WORDCNTMAX_EN + 1; 
	if (!term.empty() && term.size() < BUFF) 
	{
		char tmp[BUFF];
		strncpy/*_s*/(tmp, /*BUFF,*/ term.c_str(), term.size());
		tmp[term.size()] = '\0';
		char *src, *dest;
		src = dest = tmp;
		while (*src != '\0')
		{
			if (*src < 0)
			{
				if (*(src+1) != '\0')
				{	
					if ( 0xa3 == (unsigned char)*src 
						&& (unsigned char)*(src+1) >= 0xa1
						&& (unsigned char)*(src+1) <= 0xfe)
					{	
						*dest++ = tolower((unsigned char)*(src+1) - 0x80);
						src += 2;
					}
					else if ((unsigned char)*src == 0xa1 
						&& (unsigned char)*(src+1) == 0xa1)
					{	
						*dest++ = 0x20;
						src += 2;
					}
					else
					{	
						return false;
					}			
				}
				else
				{	
					return false;
				}
			}
			else 
			{	
				if (*src >= 0x20 && *src <= 0x7e)
				{
					*src = tolower(*src);
					*dest++ = *src++;
				}
				else
				{
					return false;
				}
			}
		}
		*dest = '\0';
		term = tmp;
		size_t beg = term.find_first_not_of(" ");
		size_t end = term.find_last_not_of(" ");
		if (beg != std::string::npos && end != std::string::npos && beg <= end)
		{
			term = term.substr(beg, end+1-beg);
			return true;
		}
	}
	return false;
}

char* segment::CDic_w::NormalizeString(char *src)
{	
	if (NULL == src || *src == '\0') 
	{
		return src;
	}
	char *dest = src;
	while (*src != '\0')
	{
		if (*src < 0)
		{
			if (*(src+1) != '\0')
			{ 
				char word[3] = {*src, *(src+1), '\0'};
				if(hmChinese.find(word) != hmChinese.end())
				{ 
					*dest++ = hmChinese[word][0];
					*dest++ = hmChinese[word][1];
					src += 2;
				}
				else 
				{ 
					if ( 0xa3 == (unsigned char)*src 
						&& (unsigned char)*(src+1) >= 0xa1
						&& (unsigned char)*(src+1) <= 0xfe)
					{ 
						*dest++ = tolower((unsigned char)*(src+1) - 0x80);
						src += 2;
					}
					else if ((unsigned char)*src == 0xa1 
						&& (unsigned char)*(src+1) == 0xa1)
					{ 
						*dest++ = 0x20;
						src += 2;
					}
					else
					{ 
						*dest++ = *src++;
						*dest++ = *src++;
					}
				}				
			}
			else
			{ 
				break;
			}
		}
		else
		{ 
			*src = tolower(*src);
			*dest++ = *src++;
		}
	}
	*dest = '\0';
	return dest;
}

inline void segment::CDic_w::SegByPunc(
	int offset, short buff[], short beg, short end, 
	std::vector<Term>& out, bool isKey)
{	
	short length = end - beg;
	short level = isKey ? 1 : 0;
	short vbeg = -1;	
	
	while (beg < end)
	{
		assert(0 == buff[beg] || (buff[beg] >= FRST_OFFSET_CH + 1 
			&& buff[beg] <= FRST_OFFSET_CH + 95));

		if (0 == buff[beg])
		{	
			if (vbeg >= 0 && vbeg < beg)
			{	
				out.push_back(Term(offset+vbeg, beg-vbeg, level));
				vbeg = -1;
			}
			out.push_back(Term(offset+beg, 1, level)); //
		}
		else
		{
			if (isalnum(buff[beg] - FRST_OFFSET_CH + 31))
			{	
				if (-1 == vbeg)
				{
					vbeg = beg;
				}
			}
			else
			{	
				if (vbeg >= 0 && vbeg < beg)
				{	
					out.push_back(Term(offset+vbeg, beg-vbeg, level));
					vbeg = -1;
				}
				out.push_back(Term(offset+beg, 1, level)); 
			}
		}
		++beg;
	}

	bool isWhole = false;	
	
	if (vbeg >= 0 && vbeg < beg)
	{	
		if (beg-vbeg == length)
		{
			isWhole = true;
			level = 0;
		}
		out.push_back(Term(offset+vbeg, beg-vbeg, level));
	}

	if (false == isWhole && true == isKey)
	{	
		out.push_back(Term(offset+(end-length), length, 0));
	}
}


void segment::CDic_w::SegHalfWidth(
	int offset, short buff[], int buffSize, int wordCount, 
	std::vector<Term>& out, int segType)
{	
	if (wordCount <= 1 || wordCount >= SHRT_MAX)
	{
		while (wordCount)
		{
			short len = wordCount > SHRT_MAX ? SHRT_MAX : wordCount;
			out.push_back(Term(offset, len, 0));
			offset += len;
			wordCount -= len;
		}	
		return;
	}

	if (buffSize < wordCount + KEYCNTMAX)
	{
		//std::cerr << "Segment NO enough buff!" << std::endl; //to ErrorLog
		return;
	}

	short wordCnt = (short)wordCount;				
	bool level = segType == minSeg ? 1 : 0;	//
	short ccur, kbeg, kend, nbeg, nend; //
	int tcur;						
	int s, t, base, check;						  
	
	
	ccur = kbeg = nbeg = 0;	 
	while (ccur < wordCnt) 
	{
		
		while (ccur < wordCnt)
		{
			s = buff[ccur];
			base = m_viBase[s];
			if (base && base != -s && (0 == ccur || 
				true == SegBoundCheck(buff[ccur], buff[ccur-1])))
			{	
				break;
			}
			++ccur;
		}

		kbeg = ccur;
		tcur = wordCnt;		
		
		
		while (++ccur < (short)wordCnt)
		{
			t = abs(base) + buff[ccur];
			check = m_viCheck[t];
			if (abs(check) != s)
			{	
				break;
			}

			base = m_viBase[t];
			if (base < 0)
			{	
				buff[tcur++] = ccur + 1;
				if (base == -t || tcur - wordCnt >= KEYCNTMAX)
				{	
					break;
				}
			}
			s = t;
		}

		
		if (tcur > wordCnt)
		{				
			while (--tcur >= wordCnt)
			{	
				assert(buff[tcur] <= wordCnt && buff[tcur] > 0);
				if (buff[tcur] == wordCnt || 
					true == SegBoundCheck(buff[buff[tcur]-1], buff[buff[tcur]]))
				{		
					break;
				}
			}

			if (tcur >= wordCnt)
			{	
				kend = buff[tcur];
				nend = kbeg;

				if (nend > nbeg)
				{	
					1 == nend - nbeg ?
						out.push_back(Term(offset+nbeg, 1, level)) :
						SegByPunc(offset, buff, nbeg, nend, out, false);
				}

				
				switch (segType)
				{
					case omniSeg:
					{	
						1 == kend - kbeg ?
							out.push_back(Term(offset+kbeg, 1, level)) :
							SegByPunc(offset, buff, kbeg, kend, out, true);
					}
					break;
					case maxSeg:
					{	
						out.push_back(Term(offset+kbeg, kend-kbeg, level));
					}
					break;
					case minSeg:
					{	
						1 == kend - kbeg ?
							out.push_back(Term(offset+kbeg, 1, level)) :
							SegByPunc(offset, buff, kbeg, kend, out, false);
					}
					break;
				}
								
				ccur = nbeg = kend;	
			}
			else
			{	
				ccur = ++kbeg;
			}
		}
		else
		{	
			ccur = ++kbeg;
		}
	}
	if (nbeg < wordCnt)
	{	
		nend = wordCnt;
		1 == nend - nbeg ?
			out.push_back(Term(offset+nbeg, 1, level)) :
			SegByPunc(offset, buff, nbeg, nend, out, false);
	}
}
inline bool segment::CDic_w::SegInside(
	int offset, const short buff[], short beg, 
	short end, std::vector<Term>& out)
{	
	bool hasInside = false;
	short kbeg, nbeg;
	int s, base, t, check;
	bool level = 1;
	
	nbeg = beg;	
	++beg;
	while (beg < end)
	{
		kbeg = beg;	
		
		s = buff[beg];
		base = m_viBase[s];
		if (!base || base == -s)
		{
			++beg;
			continue; 
		}
		while (++beg < end)
		{
			t = abs(base) + buff[beg];				
			check = m_viCheck[t];
			if (abs(check) != s)
			{
				break;
			}

			base = m_viBase[t];
			if (base < 0 && check > 0)
			{	
				while (nbeg < kbeg)
				{	
					out.push_back(Term(offset+(nbeg<<1), 2, level));
					++nbeg;
				}
				
				++beg; 
				out.push_back(Term(offset+(kbeg<<1),(beg-kbeg)<<1,level));
				hasInside = true;
				nbeg = beg;
				break;
			}
			s = t;		
		}
	}
	if (hasInside)
	{
		while (nbeg < end)
		{	
			out.push_back(Term(offset+(nbeg<<1), 2, level));
			nbeg += 2;
		}
	}
	return hasInside;
}

void segment::CDic_w::SegMaxCH(
	int offset, short buff[], int buffSize, 
	int wordCount, std::vector<Term>& out) 
{	
	if (wordCount <= 1 || wordCount >= SHRT_MAX)
	{
		while (wordCount)
		{
			short len = wordCount > SHRT_MAX ? SHRT_MAX : wordCount;
			out.push_back(Term(offset, len, 0));
			offset += len;
			wordCount -= len;
		}	
		return;
	}
	if (buffSize < wordCount)
	{
		//std::cerr << "Segment NO enough buff!" << std::endl;//to ErrorLog
		return;
	}	

	const short wordCnt = (short)wordCount; 
	short ccur, kbeg; 
	int s, t, base, check; 
	int pos;	  	
	int ambInfo;  
	short maxLen; 
	bool level = 0;	

	
	ccur = 0;
	while (ccur < wordCnt) 
	{				
		kbeg = ccur; 
		pos = offset + (kbeg << 1);
		ambInfo = maxLen = 0;
		
		s = buff[ccur];
		base = m_viBase[s];
		if (!base || base == -s) 
		{	
			out.push_back(Term(pos, 2, level));
			++ccur;	
			continue;
		}
		
		
		maxLen = 2;
		while (++ccur < wordCnt)
		{				
			t = abs(base) + buff[ccur];				
			check = m_viCheck[t];
			if (abs(check) != s)
			{
				break; 
			}
			
			base = m_viBase[t];
			if (base < 0) 
			{						
				maxLen = (ccur - kbeg + 1) << 1;	
				ambInfo = check < 0 ? m_viWeight[t] : 0;	
				if (base == -t)
				{
					break; 
				}
			}				
			s = t;										
		} 
		
		ccur = kbeg + (maxLen>>1);	
		
		if (ambInfo && ambiguity == (ambInfo & KEYTYPEBIT)) 
		{	
			ambInfo >>= AMBOFFSET;
			assert(ambInfo);
			short len = 0;
			
			while (ambInfo) 
			{
				len = 2;
				while (0 == (ambInfo & 1)) 
				{
					ambInfo >>= 1; 
					len += 2;
				}
				out.push_back(Term(pos, len, level));
				pos += len;
				ambInfo >>= 1; 
			}
			out.pop_back();
			ccur -= len >> 1; 
			assert(ccur > 0);			
		}				
		else
		{	
			out.push_back(Term(pos, maxLen, level));
		}
	}
}

void segment::CDic_w::SegMinCH(
	int offset, short buff[], int size, 
	int wordCount, std::vector<Term>& out)
{	
	if (wordCount <= 1 || wordCount >= SHRT_MAX)
	{
		while (wordCount)
		{
			short len = wordCount > SHRT_MAX ? SHRT_MAX : wordCount;
			out.push_back(Term(offset, len, 1));
			offset += len;
			wordCount -= len;
		}	
		return;
	}
	if (size < wordCount + (KEYCNTMAX<<1))
	{
		//std::cerr << "Segment NO enough buff!" << std::endl;//to ErrorLog
		return;
	}	
	
	
	const short wordCnt = (short)wordCount;
	const int typeBeg = wordCnt + KEYCNTMAX;
	short ccur, kbeg; 
	int lcur; 
	int tcur; //
	int s, t, base, check;	 
	int pos;	
	int keyCnt; 
	int ambInfo; 
	bool level = 1;	

	ccur = 0; 
	while (ccur < wordCnt)
	{
		kbeg = ccur; 
		pos = offset + (kbeg << 1);
		ambInfo = 0;

		s = buff[ccur];
		base = m_viBase[s];
		if (!base || base == -s) 
		{
			out.push_back(Term(pos, 2, level));
			++ccur;
			continue;
		}	

		
		lcur = wordCnt;
		tcur = typeBeg;
		while (++ccur < wordCnt) 
		{		
			t = abs(base) + buff[ccur];				
			check = m_viCheck[t];
			if (abs(check) != s)
			{
				break;
			}

			base = m_viBase[t];
			if (base < 0) 
			{	
				ambInfo = check < 0 ? m_viWeight[t] : 0;	
				buff[lcur++] = ccur - kbeg + 1;				//
				buff[tcur++] = m_viWeight[t] & KEYTYPEBIT; 
				if (base == -t || lcur - wordCnt >= KEYCNTMAX)
				{	
					break;
				}
			}	
			s = t;								
		}

		
		keyCnt = lcur - wordCnt; 
		if (0 == keyCnt) 
		{	
			out.push_back(Term(pos, 2, level));
			ccur = ++kbeg; 
			continue;
		}

		
		assert(tcur > typeBeg);
		switch(buff[--tcur])
		{
			case ambiguity: 
			{	
				short _wordCnt = buff[tcur - KEYCNTMAX];
				bool alsoKey = (ambInfo & KEYTYPEBIT) == ambiguity ? false : true;
				ccur = kbeg + _wordCnt;
				ambInfo >>= AMBOFFSET;
				assert(ambInfo);

				short _kbeg = kbeg;	
				short _cnt = 0;		
				while (ambInfo) 
				{
					_cnt = 1;
					while (0 == (ambInfo & 1)) 
					{
						ambInfo >>= 1; 
						++_cnt;
					}
					bool hasInsideKey = false;
					if (_cnt > 2 && ((ambInfo>>1) || alsoKey))
					{	
						hasInsideKey = SegInside(offset, buff, _kbeg, _kbeg+_cnt, out);
					}
					if (false == hasInsideKey)
					{	
						out.push_back(Term(offset+(_kbeg<<1), _cnt<<1, level));
					}
					_kbeg += _cnt;
					ambInfo >>= 1; 
				}
				if (false == alsoKey) 
				{	
					out.pop_back();	
					ccur -= _cnt;
				} 
			}
			break;
			case common:
			{	
				lcur = wordCnt;
				
				while (lcur < wordCnt + keyCnt && 
					buff[lcur + KEYCNTMAX] == ambiguity)
				{
					++lcur;
				}
				assert(lcur < wordCnt + keyCnt);
				
				bool hasInsideKey = false;
				if (buff[lcur + KEYCNTMAX] == common && buff[lcur] > 2)
				{
					hasInsideKey = 
						SegInside(offset, buff, kbeg, kbeg+buff[lcur], out);
				}
				if (false == hasInsideKey)
				{
					out.push_back(Term(pos, buff[lcur]<<1, level));
				}
				ccur = kbeg + buff[lcur];
			}
			break;
			case special:
			{	
				lcur = tcur - KEYCNTMAX;
				ccur = kbeg + buff[lcur];
				out.push_back(Term(pos, buff[lcur]<<1, level));
			}
			break;
		}
	}
}

void segment::CDic_w::SegOmniCH(
	int offset, short buff[], int buffSize, 
	int wordCount, std::vector<Term>& out)
{	
	if (wordCount <= 1 || wordCount >= SHRT_MAX)
	{
		while (wordCount)
		{
			short len = wordCount > SHRT_MAX ? SHRT_MAX : wordCount;
			out.push_back(Term(offset, len, 0));
			offset += len;
			wordCount -= len;
		}	
		return;
	}
	if (buffSize < ((wordCount + KEYCNTMAX) <<1))
	{
		//std::cerr << "Segment NO enough buff!" << std::endl; //to ErrorLog
		return;
	}	

	//wordCount: [2-SHRT_MAX)
	const short wordCnt = (short)wordCount; //
	const int lenBeg = wordCnt << 1;		//
	const int typeBeg = lenBeg + KEYCNTMAX; //
	short ccur, kbeg, kend; 
	int fcur; //
	int lcur; //
	int tcur; //
	int s, t, base, check;	 
	bool level;	
	int pos;	
	int keyCnt; 
	int ambInfo; 

	
	fcur = wordCnt;
	while (fcur < lenBeg)
	{
		buff[fcur++] = 0;	
	}
	
	
	ccur = kend = 0; 
	while (ccur < wordCnt)
	{
		kbeg = ccur; 
		pos = offset + (kbeg << 1);
		//ambInfo = level = 0;
		ambInfo = /*level = */0;
		level =  kbeg < kend ? 1 : 0;

		s = buff[ccur];
		base = m_viBase[s];
		if (!base || base == -s) 
		{	
			ccur > kend ? (kend = ccur + 1) : (level = 1); 
			out.push_back(Term(pos, 2, level));
			++ccur;	 
			continue;
		}	

		
		lcur = lenBeg;
		tcur = typeBeg;
		while (++ccur < wordCnt) 
		{		
			t = abs(base) + buff[ccur];				
			check = m_viCheck[t];
			if (abs(check) != s)
			{
				break;
			}

			base = m_viBase[t];
			if (base < 0) 
			{	
				ambInfo = check < 0 ? m_viWeight[t] : 0;	//歧义
				buff[lcur++] = ccur - kbeg + 1;						//字数
				buff[tcur++] = m_viWeight[t] & KEYTYPEBIT;  //词类型
				if (base == -t || lcur - lenBeg >= KEYCNTMAX)
				{	
					break;
				}
			}	
			s = t;								
		}

		
		keyCnt = lcur - lenBeg; 
		if (0 == keyCnt) 
		{	
			kbeg >= kend ? (kend = kbeg + 1) : (level = 1); 
			out.push_back(Term(pos, 2, level));
			ccur = ++kbeg;  
			continue;
		}

		
		assert(tcur > typeBeg);
		switch(buff[--tcur])
		{
			case ambiguity: 
			{	
				short _wordCnt = buff[tcur - KEYCNTMAX];
				short _kend = kbeg + _wordCnt;
				bool alsoKey = (ambInfo & KEYTYPEBIT) == ambiguity ? false : true;
				ambInfo >>= AMBOFFSET;
				assert(ambInfo);

				if (_kend <= kend)
				{	
					level = 1;
				}
				if (true == alsoKey) 
				{	
					out.push_back(Term(pos, _wordCnt<<1, level));
					level = 1;
				}

				short _kbeg = kbeg;	
				short _cnt = 0;		
				while (ambInfo) 
				{
					_cnt = 1;
					while (0 == (ambInfo & 1)) 
					{
						ambInfo >>= 1; 
						++_cnt;
					}
					if (_cnt > 2 && ((ambInfo>>1) || alsoKey))
					{	
						SegInside(offset, buff, _kbeg, _kbeg+_cnt, out);
					}
					out.push_back(Term(offset+(_kbeg<<1), _cnt<<1, level));
					_kbeg += _cnt;
					ambInfo >>= 1; 
				}

				if (false == alsoKey) 
				{	
					out.pop_back();	
					_kend -= _cnt;
				} 
				if (_kend < wordCnt)
				{	
					buff[_kend + wordCnt] = 1; 
				}
				if (_kend > kend)
				{	
					kend = _kend;
				}
			}
			break;
			case common:
			{	
				lcur = lenBeg;
				
				while (lcur < lenBeg + keyCnt && 
					buff[lcur + KEYCNTMAX] == ambiguity)
				{
					++lcur;
				}
				assert(lcur < lenBeg + keyCnt);
				
				if (buff[lcur + KEYCNTMAX] == common && buff[lcur] > 2)
				{
					SegInside(offset, buff, kbeg, kbeg+buff[lcur], out);
				}
				short _kend;
				while (lcur < lenBeg + keyCnt - 1) 
				{	
					if (buff[lcur + KEYCNTMAX] != ambiguity)
					{
						out.push_back(Term(pos, (buff[lcur]<<1), 1));
						_kend = kbeg + buff[lcur];
						if (_kend < wordCnt)
						{
							buff[_kend + wordCnt] = 1; 
						}
					}				
					++lcur;
				}
				
				_kend = kbeg + buff[lcur];
				_kend < kend ? (level = 1) : (kend = _kend);
				out.push_back(Term(pos, (buff[lcur]<<1), level));
				if (_kend < wordCnt)
				{
					buff[_kend + wordCnt] = 1; 
				}
			}
			break;
			case special:
			{	
				short _kend = kbeg + buff[tcur-KEYCNTMAX];
				_kend < kend ? (level = 1) : (kend = _kend);
				out.push_back(Term(pos, (_kend-kbeg)<<1, level));
				if (_kend < wordCnt)
				{
					buff[_kend + wordCnt] = 1; 
				}
			}
			break;
		}

		
		fcur = int(kbeg + wordCnt + 1);
		while (fcur < lenBeg && 0 == buff[fcur])
		{
			++fcur;
		}
		ccur = fcur - wordCnt;
	}
}

bool segment::CDic_w::OmniSegment(
	const char *beg, const char *end, 
	std::vector<Term>& out, char *buff, int buffSize)
{	
	
	if (NULL == beg || NULL == end || end < beg || 
		buffSize < int((end - beg + (KEYCNTMAX<<1)) * sizeof(short)))	
	{
		return false;
	}

	short *code = (short*)buff;		 
	const char *en, *nch, *ch, *cur; 
	en = nch = ch = cur = beg;

	while (cur < end) 
	{
		
		int charCnt = 0;
		while (cur < end && *cur >= 0)
		{
			code[charCnt++] = GetENCode(*cur);
			++cur;
		}
		if (charCnt) 
		{
			1 == charCnt ? 
				out.push_back(Term(int(en - beg), 1, 0)) :
				SegHalfWidth(
					int(en - beg), code, buffSize, charCnt, out, omniSeg);
			nch = ch = cur;
		}

		
		if (cur == end - 1) 
		{		
			out.push_back(Term(int(cur - beg), 1, 0));		
			break;				
		}

		
		while (cur < end -1 && *cur < 0 && 
			!IsGBKCH((unsigned char)*cur, (unsigned char)*(cur+1))) 
		{
			out.push_back(Term(int(cur - beg), 2, 0));
			cur += 2;
		}
		if (nch < cur) 	
		{
			en = ch = cur;
		}
		
		
		int wordCnt = 0;
		while (cur < end -1 && *cur < 0 && 
			IsGBKCH((unsigned char)*cur, (unsigned char)*(cur+1))) 
		{
			code[wordCnt++] = 
				GetCHCode((unsigned char)*cur, (unsigned char)*(cur+1));
			cur += 2;
		}
		if (wordCnt) 
		{
			1 == wordCnt ?	
				out.push_back(Term(int(ch - beg), 2, 0)) :
				SegOmniCH(int(ch - beg), code, buffSize, wordCnt, out);
			en = nch = cur;
		}
	}
	return true;
}

bool segment::CDic_w::MaxSegment(
	const char *beg, const char *end, 
	std::vector<Term>& out, char *buff, int buffSize)
{	
	if (NULL == beg || NULL == end || end < beg || 
		buffSize < int((end - beg + KEYCNTMAX) * sizeof(short)))	
	{
		return false;
	}

	short *code = (short*)buff;		 //for store chinese code
	const char *en, *nch, *ch, *cur; //cursors
	en = nch = ch = cur = beg;

	while (cur < end) 
	{
		//single byte
		int charCnt = 0;
		while (cur < end && *cur >= 0)
		{
			code[charCnt++] = GetENCode(*cur);
			++cur;
		}
		if (charCnt) 
		{
			1 == charCnt ? 
				out.push_back(Term(int(en - beg), 1, 0)) :
				SegHalfWidth(int(en - beg), code, buffSize, charCnt, out, maxSeg);
			nch = ch = cur;
		}

		//half word
		if (cur == end - 1) 
		{		
			out.push_back(Term(int(cur - beg), 1, 0));		
			break;				
		}

		//double byte, non-gbk-chinese
		while (cur < end -1 && *cur < 0 && 
			!IsGBKCH((unsigned char)*cur, (unsigned char)*(cur+1))) 
		{
			out.push_back(Term(int(cur - beg), 2, 0));
			cur += 2;
		}
		if (nch < cur) 	
		{
			en = ch = cur;
		}
		
		//chinese
		int wordCnt = 0;
		while (cur < end -1 && *cur < 0 && 
			IsGBKCH((unsigned char)*cur, (unsigned char)*(cur+1))) 
		{
			code[wordCnt++] = 
				GetCHCode((unsigned char)*cur, (unsigned char)*(cur+1));
			cur += 2;
		}
		if (wordCnt) 
		{
			1 == wordCnt ?	
				out.push_back(Term(int(ch - beg), 2, 0)) :
				SegMaxCH(int(ch - beg), code, buffSize, wordCnt, out);
			en = nch = cur;
		}
	}
	return true;
}

bool segment::CDic_w::MinSegment(
	const char *beg, const char *end, 
	std::vector<Term>& out, char *buff, int buffSize)
{	
	if (NULL == beg || NULL == end || end < beg || 
		buffSize < int((end - beg + (KEYCNTMAX<<1)) * sizeof(short)))	
	{
		return false;
	}

	short *code = (short*)buff;		 
	const char *en, *nch, *ch, *cur;
	en = nch = ch = cur = beg;

	while (cur < end) 
	{
		//single byte
		int charCnt = 0;
		while (cur < end && *cur >= 0)
		{
			code[charCnt++] = GetENCode(*cur);
			++cur;
		}
		if (charCnt) 
		{
			1 == charCnt ? 
				out.push_back(Term(int(en - beg), 1, 0)) :
				SegHalfWidth(int(en - beg), code, buffSize, charCnt, out, minSeg);
			nch = ch = cur;
		}

		//half word
		if (cur == end - 1) 
		{		
			out.push_back(Term(int(cur - beg), 1, 0));		
			break;				
		}

		//double byte, non-gbk-chinese
		while (cur < end -1 && *cur < 0 && 
			!IsGBKCH((unsigned char)*cur, (unsigned char)*(cur+1))) 
		{
			out.push_back(Term(int(cur - beg), 2, 0));
			cur += 2;
		}
		if (nch < cur) 	
		{
			en = ch = cur;
		}
		
		//chinese
		int wordCnt = 0;
		while (cur < end -1 && *cur < 0 && 
			IsGBKCH((unsigned char)*cur, (unsigned char)*(cur+1))) 
		{
			code[wordCnt++] = 
				GetCHCode((unsigned char)*cur, (unsigned char)*(cur+1));
			cur += 2;
		}
		if (wordCnt) 
		{
			1 == wordCnt ?	
				out.push_back(Term(int(ch - beg), 2, 0)) :
				SegMinCH(int(ch - beg), code, buffSize, wordCnt, out);
			en = nch = cur;
		}
	}
	return true;
}


void segment::CDic_w::MaxSegment(
	const char *beg, const char *end, std::vector<Term>& out)

{
	int size = int((end - beg + KEYCNTMAX) * sizeof(short));
	char * buff = new char[size];

	if(NULL == buff)
	{
		return;
	}

	MaxSegment(beg, end, out, buff, size);    

	delete [] buff;
}


void segment::CDic_w::WordSegment(
	const char *_beg, const char *_end, std::vector<Term>& out)
{
	if (!_beg || !_end || _beg >= _end)
	{
		return;
	}
	out.clear();

	const unsigned char *beg = (const unsigned char *)_beg;
	const unsigned char *start = beg;
	const unsigned char *end = (const unsigned char *)_end;

	const short DEFAULT_WORD_LEVEL = 1;

	while (beg < end)
	{
		if (*beg < 0x80) 
		{
			
			Term item;
			item.pos = int(beg - start);
			item.len = 1;
			item.level = DEFAULT_WORD_LEVEL;
			out.push_back(item);
			beg++;
		}
		else
		{
			if (beg+1 < end)
			{
				if (beg[0]>=0x81 && beg[1]<=0xFE)
				{
					
					Term item;
					item.pos = int(beg - start);
					item.len = 2;
					item.level = DEFAULT_WORD_LEVEL;
					out.push_back(item);
					beg += 2;
				}
				else 
				{
					
					
					beg++;
				}
			}
			else
			{
				
				beg++;
			}
		}
	}
}
