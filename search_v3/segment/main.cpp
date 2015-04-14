#include <ctime>
#include "ta_dic_w.h"

using namespace std;

#ifndef __GNUC__
#include <windows.h>
#include <direct.h>
#endif

int main()
{

#ifndef __GNUC__ 
	char chFullPath[1024];
	// 得到本程序的可执行文件全名，存储在chFullPath中
	GetModuleFileNameA(::GetModuleHandleA(NULL), chFullPath, 1024);
	int n = strlen(chFullPath) - 1;
	while (chFullPath[n]!='/' && chFullPath[n]!='\\')
	{
		n--;
	}
	// 得到本程序的可执行文件所在目录，存储在chFullPath中
	chFullPath[n+1] = '\0';
	_chdir(chFullPath);
#endif

	segment::PidConfig *m_pconfig = new segment::PidConfig();
	if(m_pconfig->InitPara("./static/config.ini"))
	{
		segment::CDic_w segObj;	
		if (m_pconfig->mode)
		{	//词典索引建立模式
			std::cout << "词典索引模式"<< std::endl;
					
			segment::VEC_PAIR_STR_INT In;
			In.push_back(
				segment::PAIR_STR_INT(m_pconfig->Dict_Com, segment::common));//汉普
			In.push_back(
				segment::PAIR_STR_INT(m_pconfig->Dict_Pro, segment::special));//汉专
			//精简汉歧
			if (false == segObj.AmbKeyFilter(In, m_pconfig->Dict_Amb, m_pconfig->Dict_AmbF))
			{
				return -1;
			}
			In.push_back(
				segment::PAIR_STR_INT(m_pconfig->Dict_AmbF, segment::ambiguity));//汉歧
			
			In.push_back(
				segment::PAIR_STR_INT(m_pconfig->Dict_Eng, segment::english));//英专
			
			//输出
			segment::VEC_STR Out;
			Out.push_back(m_pconfig->Idx_Dat); //双数组索引树
			Out.push_back(m_pconfig->Idx_Map); //扩展汉字映射表
			if (false == segObj.CreateLexIdx(In, Out))
			{
				return -1;
			}
		}
		else 
		{	//分词模式
			std::cout << "分词模式"<< std::endl;

			//加载词典索引
			if (false == segObj.Load(m_pconfig->Idx_Dat, m_pconfig->Idx_Map))
			{
				return -1;			
			}

#define SingleTest 1
			if (SingleTest)
			{   //特例串测试
				std::vector<segment::Term> vec;
				char buff[2048];
				
				/*
				std::string h = "并联机器人机构学理论及控制";
				segObj.MaxSegment(&h[0],&h[0]+h.size(),v,b, 200);
				for (size_t i=0; i< v.size(); ++i)
					std::cout << h.substr(v[i].pos, v[i].len) << "/";
				*/
				
				//const char* s = "人民币";
				//const char* s= "研究所在猫和老鼠并联机器人机构学理论及控制c++.net,.NET32c2c#primer office2008officer d&g n73";
				const char* s = "the face shop";
				std::string ss = s;
				if (true == segObj.OmniSegment(s, s+strlen(s), vec, buff, 2048))
				{
					for (size_t i = 0; i < vec.size(); ++i)
					{
						std::cout << vec[i].level << ":";	
						std::cout << ss.substr(vec[i].pos, vec[i].len) << "/";
					}
					std::cout << std::endl;
				}
				else
				{
					std::cerr << "OmniSegment false!" << std::endl;
				}
				vec.clear();
				/*segObj.MaxSegment(s, s+strlen(s), vec, buff, 1024);
				for (size_t i=0; i< vec.size(); ++i)
					std::cout << ss.substr(vec[i].pos, vec[i].len) << "/";
				std::cout << std::endl;
				*/
			}

#define SetTest 0
			if (SetTest)
			{	//集合测试
				std::ifstream fin("E:\\work\\Segment\\data\\test");////E:\\work\\GM-Seg&Rank\\program\\DATrie\\release\\title.txt");//
				std::ofstream fout("E:\\work\\GM-Seg&Rank\\program\\DATrie\\release\\test_max0220");////
				if (!fin.is_open())
				{
					std::cerr << "file open fail"<< std::endl;
				}
				std::vector<std::string> titles;
				std::string line;
				while (getline(fin, line)) 
					titles.push_back(line);

				std::cout << "start!" << std::endl;
				std::clock_t start_time,end_time;
				start_time =  std::clock();

				size_t size = titles.size();
				std::vector<struct segment::Term> vec, vec_o, vec_a, vec_i;	//分词结果


				const char* str;
				char buff[4096];
				for (size_t j = 0; j < size; ++j) 
				{
					str = titles[j].c_str();
					
			/*		{
						segObj.OmniSegment(str, str+strlen(str), vec, buff, 2048);
						fout << titles[j] << std::endl;
						size_t sz = vec.size();
						for (size_t i= 0; i < sz; ++i) 
						{
							//std::cout << vec[i].pos << " " << vec[i].len << " " <<  vec[i].level << std::endl;
							fout << titles[j].substr(vec[i].pos, vec[i].len) << " ";
						}
						fout << std::endl; 
						//std::cout << std:: endl;
						vec.clear();
					}
			*/	
			/*		{
					//检验算法正确性：全切分结果是否包含最大最小切分结果
					segObj.OmniSegment(str, str+strlen(str), vec_o, buff, 2048);
					segObj.MaxSegment(str, str+strlen(str), vec_a, buff, 2048);
					segObj.MinSegment(str, str+strlen(str), vec_i, buff, 2048);

					//max
					for (size_t i = 0; i < vec_a.size(); ++i) 
					{
						if (std::find(vec_o.begin(), vec_o.end(), vec_a[i]) == vec_o.end()) 
						{
							fout << "Omn:";
							for (size_t m= 0; m < vec_o.size(); ++m) 
							{
								fout << titles[j].substr(vec_o[m].pos, vec_o[m].len) << "/";
							}
							fout << std::endl << "Max:";
							for (size_t k= 0; k < vec_a.size(); ++k) {
								fout << titles[j].substr(vec_a[k].pos, vec_a[k].len) << "/";
							}
							fout << std::endl<< std::endl;
						}
					}
					//min
					for (size_t i = 0; i < vec_i.size(); ++i) 
					{
						if (std::find(vec_o.begin(), vec_o.end(), vec_i[i]) == vec_o.end()) 
						{
							fout << "Omn:";
							for (size_t m= 0; m < vec_o.size(); ++m) 
							{
								fout << titles[j].substr(vec_o[m].pos, vec_o[m].len) << "/";
							}
							fout << std::endl << "Min:";
							for (size_t k= 0; k < vec_i.size(); ++k) {
								fout << titles[j].substr(vec_i[k].pos, vec_i[k].len) << "/";
							}
							fout << std::endl<< std::endl;
						}
					}
					
					vec_o.clear();
					vec_a.clear();
					vec_i.clear();
					}
			*/	
					{	//人民日报测试文本

						if (false == segObj.MaxSegment(str, str+strlen(str), vec, buff, 4096))
						{
							std::cerr << "segment false." << titles[j] << std::endl;
						}
						for (size_t i = 0; i < vec.size(); ++i) 
						{
							fout << titles[j].substr(vec[i].pos, vec[i].len) << "  ";
						}
						fout << std::endl; 
						vec.clear();
					}
				}

				end_time = std::clock();
				double t= (double)(end_time-start_time)/1000;
				std::cout <<t<<"s"<< std::endl;
				fout.close();
			}
		
#define SegmentTxt 0
			if (SegmentTxt)
			{
				//集合切分
				std::ifstream fin("E:\\work\\GM-Seg&Rank\\data\\Lexicon\\4-gram.txt");
				std::ofstream fout("E:\\work\\GM-Seg&Rank\\data\\Lexicon\\4-gram_flt_seg");
				if (!fin.is_open())
				{
					std::cerr << "file open fail"<< std::endl;
				}
				
				const std::string space = "\t";
				const size_t sz = space.size();
				std::vector<struct segment::Term> vec;	//分词结果
				const char* str;
				char buff[256];

				size_t pos;
				std::string line, word;
				while (getline(fin, line)) 
				{
					fout << line << space;
					pos = line.find(space);
					word = line.substr(0, pos);
					str = word.c_str();
					if (false == segObj.MinSegment(str, str+strlen(str), vec, buff, 256))
					{
						std::cerr << "segment false." << word << std::endl;
					}
					for (size_t i = 0; i < vec.size(); ++i) 
					{
						fout << word.substr(vec[i].pos, vec[i].len) << "/";
					}
					fout << std::endl; 
					vec.clear();
				}	
			}
		}
		segObj.ClearData();
	}
	delete m_pconfig;
	m_pconfig = 0;
	
	return 0;
}
