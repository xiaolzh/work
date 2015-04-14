#ifndef KEYRANKING_H
#define KEYRANKING_H
#include "Module.h"
#include "GlobalDef.h"

//query��������������
class CKeySrchAnalysis:public IAnalysisData
{
public:
	CKeySrchAnalysis(){}
	virtual ~CKeySrchAnalysis(){;}
	//do it .
};



class CKeyRanking:public CModule
{
public:
	virtual ~CKeyRanking(){fprintf(stderr, "key ranking leave\n");}

	/*
	*��ʼ��������Ҫ����ģ������
	*Parameter psdi �������ݴ���
	*Parameter strConf ģ����Ե������ļ���ַ
	*return true��ʼ���ɹ���false��ʼ��ʧ�� 
	*/
	virtual bool Init(SSearchDataInfo* psdi, const string& strConf);

	/*
	* Method:    query����ת�����ض�����ģ�飬ÿ��ҵ�����ҽ���һ�����ģ�飬�����ж������ģ��
	* Returns:   void
	* Parameter: SQueryClause & qc �ṹ���Ĳ�ѯ���
	* return: CModule*  ���ص�ת������ģ��ָ��
	*/
	virtual CModule* QueryClassify(SQueryClause& qc);

	
	/*
	* Method:    query����
	* Returns:   void
	* Parameter: SQueryClause & qc �ṹ���Ĳ�ѯ���
	* return: IAnalysisData*  ���ص�query�������� �û���̬���ɣ���ܸ������٣�����ʧ���践��NULL
	*/
	virtual IAnalysisData* QueryAnalyse(SQueryClause& qc);


	/*
	*Method:  ���ͳ����Ϣ�����ڲ���IDͳ�Ƶ���ת��Ϊ���� ����CLASS PATH--> CLASS NAME
	*Parameter:vGpInfo �������� �ֶ�ID��Ӧ-->ͳ����Ϣ
	*/
	virtual void FillGroupByData(GROUP_RESULT&  vGpInfo);

	virtual void SortForCustom(vector<SResult>& vRes, int from, int to, IAnalysisData* pad);
	
	/*
	* Method:    ����Ȩ�ط���
	* Returns:   void
	* Parameter: IAnalysisData * pad ���ص�query��������
	* Parameter: SMatchElement & meÿ���ĵ���ƥ����Ϣ
	* Parameter: SResult & rt ��ֽ��
	*/
	virtual void ComputeWeight(IAnalysisData* pad, SMatchElement& me, SResult& rt);

	/*
	* Method:    ShowResult �Զ���չ����Ϣ<result><cost></cost>*****�û��Զ����������******</result>"
	* Returns:   void
	* Parameter: IAnalysisData* Pad,    ���ص�query��������
	* Parameter: CControl & ctrl        ��ѯ���Ʋ���������ҳ���������
	* Parameter: GROUP_RESULT & vGpRes  �������ͳ����Ϣ��
	* Parameter: vector<SResult> & vRes ������ֽ��
	* Parameter: vector<string> & vecRed ����ַ�������
	* Parameter: string & strRes         �����ַ���
	*/
	virtual void ShowResult(IAnalysisData* pad, CControl& ctrl, GROUP_RESULT &vGpRes,
							vector<SResult>& vRes, vector<string>& vecRed, string& strRes);
	 /*
	*Method:  ������GROUP_BY֮ǰ;
	*Parameter: IAnalysisData* Pad,    ���ص�query��������
	*Parameter: vector<SResult> & vRes ������ֽ��
	*Parameter: vector<PSS> & vGpName  ������ܵ��ֶ�������Ϣֵ
	*Parameter: GROUP_RESULT    vGpRes �������� �ֶ�ID��Ӧ-->ͳ����Ϣ
	*/
	virtual void BeforeGroupBy(IAnalysisData* pad, vector<SResult>& vRes, vector<PSS>& vGpName, GROUP_RESULT& vGpRes);

	virtual void SetGroupByAndFilterSequence(IAnalysisData* pad, vector<SFGNode>& vec);
	virtual void CustomGroupBy(IAnalysisData* pad, vector<int>& vDocIds, SFGNode& gfNode,
                                    pair<int, vector<SGroupByInfo> >& prGpInfo);

protected:
	void* m_pflSalePrice ;
	void* m_pflPromoPrice ;

};
#endif	
