#ifndef TA_DIC_W_NEW_H
#define TA_DIC_W_NEW_H

#include "dic_common.h"
#include "ini_config.h"

namespace segment
{


	//***********分词自定义类********************//
	class CDic_w
	{

	public:
		CDic_w(void);
		~CDic_w(void);
		void ClearData();

	public:
		//Entrances
		bool AmbKeyFilter(const VEC_PAIR_STR_INT&, const std::string&, const std::string&);//歧义串精简
		bool CreateLexIdx(const VEC_PAIR_STR_INT&, const VEC_STR&);		//新建词典索引
		bool Load(const std::string&, const std::string&);			//加载词典索引
		bool OmniSegment(const char*, const char*, std::vector<Term>&, char *buff, int);//全切分
		bool MaxSegment(const char*, const char*, std::vector<Term>&, char *buff, int);	//最大切分
		bool MinSegment(const char*, const char*, std::vector<Term>&, char *buff, int);	//最小切分		
		void WordSegment(const char*, const char*, std::vector<Term>&);	//按字切分
		void MaxSegment(const char *, const char*, std::vector<Term>&);

	private:
		void BuildTrie(const std::vector<TERM_CODE>&);//构造Trie树
		bool BuildDATrie();			//构造双数组索引树
		bool GetKeyCode(std::vector<short>&, const PAIR_STR_INT&); //获取词典词的内码（建表）
		bool LoadChineseTable(); 	//繁简体映射表生成
		bool LoadLexicon(const PAIR_STR_INT& lexicon, MAP_STR_INT& mKey2W); //加载词条文件
		bool NormalizeCHKey(std::string&);	//汉语词条标准化
		bool NormalizeEnKey(std::string&);	//非汉语词条标准化
		char* NormalizeString(char*); //字符串小写化，汉字简体化，半角化处理	
		void SegHalfWidth(int, short[], int, int, std::vector<Term>&, int);	//单字节串切分		
		void SegMaxCH(int, short [], int, int, std::vector<Term>&);				//汉语串最大切分
		void SegMinCH(int, short [], int, int, std::vector<Term>&);				//汉语串最小切分
		void SegOmniCH(int, short[], int, int, std::vector<Term>&);				//汉语串全切分
		
		inline short GetCHCode(unsigned char, unsigned char);	//获取汉字的内码（查询）
		inline short GetENCode(char);												//获取非汉字的内码（查询）
		inline bool IsGB2312(unsigned char, unsigned char); //判断字节是否为gb2312汉字
		inline bool IsGBKCH(unsigned char, unsigned char); //判断字节是否为gbk汉字
		inline bool SegBoundCheck(short, short); 						//半角词边界检查
		inline void SegByPunc(int, short[], short, short, std::vector<Term>&, bool); //半角字符切分
		inline bool SegInside(int, const short [], short, short, std::vector<Term>&);	//汉语内切分检查
		
	private:
		//***********双数组字位设计************
		//设置首字区域为8000个汉字位，95个ascii码字位[32-126]，1-6768位为GB2312汉字，后面1232字位为扩展汉字字位备用
		//前6768位按GB2312内码映射后再加1作为序号，后1232位用hash存储内码，以索引+6769为序号，最后将hash存成文件
		//歧义串切分信息用int大小的二进制数记录，两低字节为0，两高字节为切分信息
		
		static const short AMBOFFSET = 16;					//歧义串切分存储偏移
		static const short FRST_OFFSET = 8095;			//首字区域长度
		static const short FRST_OFFSET_CH = 8000;		//汉字部分首字区域长度
		static const short GB2312_OFFSET = 6768;		//GB2312区域长度
		static const short KEYCNTMAX = 8;						//汉字最多连续成词数（与等级数匹配）
		static const int KEYTYPEBIT = 0xffff;				//歧义串切分低字节信息位
		static const short WORDCNTMAX_CH = 15;			//汉语词最大字长
		static const short WORDCNTMAX_EN = 30;			//非汉语词最大字长
		static const short WORDCNTMAX = 
			WORDCNTMAX_CH > WORDCNTMAX_EN ? WORDCNTMAX_CH : WORDCNTMAX_EN;	//用于Trie树

	private:		
		enum lex_type{ambiguity, common, special, english}; //词典类型，汉歧/汉普/汉专/英专
		enum seg_type{omniSeg, maxSeg, minSeg};	//切分类型
	
		//Double-Array Trie
		VEC_INT m_viBase;			//直接子结点转移量、是否成词
		VEC_INT m_viCheck;		//前驱字序号、是否为歧义串
		VEC_INT m_viWeight;		//前两字节存放歧义串切分、后两字节存放词的权重标识
		std::vector<std::vector<TRIE_NODE>* > Trie;		//Trie树
		hash_map<std::string, std::string> hmChinese;	//繁简体汉字映射
		hash_map<unsigned short, short> hmCode2Idx;		//GBK扩展字内码映射
	};
}
#endif
