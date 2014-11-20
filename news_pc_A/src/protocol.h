#ifndef _PROTOCAL_H_
#define _PROTOCAL_H_
#include "libs/log.h"
#include "libs/UH_Define.h"

#define OLD_DATA

#define MAX_RECV_PACK_LEN	(1<<25)
#define MAX_XML_LEN			(1<<20)
#define ALLOC_SIZE			(1<<20)
#define DEFAULT_TAG_LEN		128
#define DEFAULT_DOC_WEIGHT	5
#define CDS					"<![CDATA["
#define CDE					"]]>"
#define DATA_HEADER			"$$$$"
#define CHECK_HEADER		"NEWSNEWS"
#define IDXHEADLEN			16
#define RECV_PROTOCAL		5001
#define SEND_PROTOCAL		5002
#define SMALLER(a,b)		((a)>(b)?(b):(a))

const var_4      add_command  = 3301;
const var_4      del_command  = 3302;
const var_4   update_command  = 3303;
const var_4  refresh_command  = 3304;

const var_4     query_command = 3305;
const var_4      similar_docs = 3306;
const var_4   capacity_status = 3307;
const var_4     binary_import = 3308;
const var_4     binary_export = 3309;
const var_4       global_push = 3310;

//doc infomation, parse from the whole doc data
typedef struct _doc_buf_
{
	var_u8 docid;				// �ĵ�ID
	var_4  doclen;
	var_u1 datatype;			// �ı����ͣ�����Ϊ��0x77
	var_1  sourcetype;
	var_4  force;
	var_u8 finger;				// ָ��
	var_u8 eventid;
	var_u8 titleid;				// ����ָ�ƣ�������
	var_u4 time;				// ����ʱ��
	// д����5��tag start
	var_2  cllen;
	var_4  cloff;
	var_2  solen;
	var_4  sooff;
	var_2  arlen;
	var_4  aroff;
	var_2  hylen;
	var_4  hyoff;
	var_2  sllen;
	var_4  sloff;
	// tag end
	
	var_2  urllen;
	var_4  urloff;
	var_2  picidlen;
	var_4  picidoff;
	var_2  picszlen;
	var_4  picszoff;
	var_2  encodelen;
	var_4  encodeoff;
	var_2  titlelen;
	var_4  titleoff;
	var_4  txtlen;
	var_4  txtoff;
	var_2  relativelen;
	var_4  relativeoff;
	var_4  relativenum;
	var_2  otherlen;
	var_4  otheroff;			// ��Ѷ6.0�����¼ӵ�tag��Ϣ������
	var_4  buflen;
	var_1  *buff;
	
	var_2  newdata_flag;		// �Ƿ�Ϊ�����ݸ�ʽ�������ɻ���������ʱ�����룬�����¼�����ʱʹ��0��1��
	var_2  eventword_len;		// �¼��ʳ�
	var_4  eventword_off;		// �¼������ݵ�ƫ��
	var_2  tag_len;				// tag��Ԫ���ܳ�������ɺ���Ľṹ��Ϣ
	var_4  tag_off;				// tag����ƫ��
	var_2  nextotherlen;
	var_4  nextotheroff;
	var_2  picset_len;			// ��ͼ
	var_4  picset_off;			// 
	var_u1 newtag_wt;			// Ȩ�أ�Ҫ��6.0������other�ֶ��з�������
	var_u1 newtag_iscenter;		// �Ƿ�����ڴ�վ��ҳ��ԭʼ�����н����õ�
	var_u1 newtag_firstpub;		// �Ƿ�վ�׷�
}DOC_BUF;


static var_4 GetWrdCt(var_1 *, var_4);
static var_4 GetRelativeNum(DOC_BUF *pDoc);
static var_4 GetTagBuf(DOC_BUF *pDoc, var_1 *outXml, var_4 leftLen);
static var_4 GetPicsets(DOC_BUF *pDoc, var_1 *outXml, var_4 leftLen); 
static var_4 MergeSingleTag(var_1* oldtag, var_4 oldlen, var_1 *newtag, var_4 newlen, var_1* dsttag);
static var_vd ParseOtherField(DOC_BUF* pDoc);

static  var_4 ProcessBufToDoc(var_1* inBuf, var_4 inBufLen, DOC_BUF *pDoc)
{
	if (inBuf == NULL || pDoc == NULL)
	{
		return -1;
	}
	// ��ʼ��pDoc
	memset(pDoc, 0, sizeof(DOC_BUF));
	var_2 cllen, solen, arlen, hylen, sllen;
	var_2 urllen, encodelen, titlelen, relativelen, otherlen;
	var_2 picidlen, picszlen;
	var_4 doclen, txtlen, buflen;
	var_4 processLen = 0;
	// 12bytes��ͷ
	pDoc->docid = *(var_u8*)inBuf;
	doclen = *(var_4*)(inBuf + 8);
	if (doclen < 0 || doclen > ALLOC_SIZE || doclen + 12 > inBufLen)
	{
		return -2;
	}
	// ����Э�飺
	// 1byte���ı�����
	// 1byte��TYPE
	// 1byte���Ƿ�ǿ�Ƹ���
	// 1byte���Ƿ�Ϊ��վ�׷�
	// 1byte���Ƿ�����ڴ�վ��ҳ
	pDoc->datatype = *(var_u1*)(inBuf + 12);
	pDoc->sourcetype = *(var_1*)(inBuf + 13);
	pDoc->force = *(var_1*)(inBuf + 14);
	pDoc->newtag_firstpub = *(var_1*)(inBuf + 15);
	pDoc->newtag_iscenter = *(var_u1*)(inBuf + 16);
	pDoc->eventid = *(var_u8*)(inBuf + 17);
	pDoc->finger  = *(var_u8*)(inBuf + 25);
	// title id�Ѳ���
	pDoc->titleid = *(var_u8*)(inBuf + 33);
	// ����8���ֽ�
	pDoc->time    = *(var_u4*)(inBuf + 49);
	processLen = processLen + 53;

	cllen = *(var_2*)(inBuf + processLen);
	if (cllen < 0 || cllen > DEFAULT_TAG_LEN || processLen + 2 + cllen  > inBufLen)
	{
		return -3;
	}
	pDoc->cllen = cllen;
	pDoc->cloff = processLen + 2;
	processLen = processLen + 2 + cllen;

	solen = *(var_2*)(inBuf + processLen);
	if (solen < 0 || solen > DEFAULT_TAG_LEN || processLen + 2 + solen > inBufLen)
	{
		return -4;
	}
	pDoc->solen = solen;
	pDoc->sooff = processLen + 2;
	processLen = processLen + 2 + solen;

	arlen = *(var_2*)(inBuf + processLen);
	if (arlen < 0 || arlen > DEFAULT_TAG_LEN || processLen + 2 + arlen > inBufLen)
	{
		return -5;
	}
	pDoc->arlen = arlen;
	pDoc->aroff = processLen + 2;
	processLen = processLen + 2 + arlen;

	hylen = *(var_2*)(inBuf + processLen);
	if (hylen < 0 || hylen > DEFAULT_TAG_LEN || processLen + 2 + hylen > inBufLen)
	{
		return -6;
	}
	pDoc->hylen = hylen;
	pDoc->hyoff = processLen + 2;
	processLen = processLen + 2 + hylen;

	sllen = *(var_2*)(inBuf + processLen);
	if (sllen < 0 || sllen > DEFAULT_TAG_LEN || processLen + 2 + sllen > inBufLen)
	{
		return -7;
	}
	pDoc->sllen = sllen;
	pDoc->sloff = processLen + 2;
	processLen = processLen + 2 + sllen;

	urllen = *(var_2*)(inBuf + processLen);
	if (urllen < 0 || processLen + 2 + urllen > inBufLen)
	{
		return -8;
	}
	pDoc->urllen = urllen;
	pDoc->urloff = processLen + 2;
	processLen = processLen + 2 + urllen;

	picidlen = *(var_u1*)(inBuf + processLen);
	if (picidlen < 0 || processLen + 1 + picidlen > inBufLen)
	{
		return -9;
	}
	pDoc->picidlen = picidlen;
	pDoc->picidoff = processLen + 1;
	processLen = processLen + 1 + picidlen;
	
	picszlen = *(var_u1*)(inBuf + processLen);
	if (picszlen < 0 || processLen + 1 + picszlen > inBufLen)
	{
		return -10;
	}
	pDoc->picszlen = picszlen;
	pDoc->picszoff = processLen + 1;
	processLen = processLen + picszlen + 1;

	encodelen = *(var_u1*)(inBuf + processLen);
	if (encodelen < 0 || processLen + 1 + encodelen > inBufLen)
	{
		return -11;
	}
	pDoc->encodelen = encodelen;
	pDoc->encodeoff = processLen + 1;
	processLen = processLen + 1 + encodelen;

	titlelen = *(var_u1*)(inBuf + processLen);
	if (titlelen < 0 || processLen + 1 + titlelen > inBufLen)
	{
		return -12;
	}
	pDoc->titlelen = titlelen;
	pDoc->titleoff = processLen + 1;
	processLen = processLen + 1 + titlelen;

	txtlen = *(var_4*)(inBuf + processLen);
	if (txtlen < 0 || processLen + 4 + txtlen > inBufLen)
	{
		return -13;
	}
	pDoc->txtlen = txtlen;
	pDoc->txtoff = processLen + 4;
	processLen = processLen + 4 + txtlen;
	
	relativelen = *(var_2*)(inBuf + processLen);
	if (relativelen < 0 || processLen + 2 + relativelen > inBufLen)
	{
		return -14;
	}
	pDoc->relativelen = relativelen;
	pDoc->relativeoff = processLen + 2;
	processLen = processLen + 2 + relativelen;
	
	otherlen = *(var_2*)(inBuf + processLen);
	if (otherlen < 0 || processLen + 2 + otherlen > inBufLen)
	{
		return -15;
	}
	pDoc->otherlen = otherlen;
	pDoc->otheroff = processLen + 2;
	processLen = processLen + 2 + otherlen;

	pDoc->buflen = processLen;
	pDoc->buff = inBuf;
	pDoc->relativenum = GetRelativeNum(pDoc);
	
	// ����other�ֶ��ж����ݸ�ʽ�����ṹ�帳ֵ 
	ParseOtherField(pDoc);

	return 0;
}

static var_4 GetRelatives(var_1 *outXml, var_4 leftLen, DOC_BUF *pDoc, var_4& totalNum)
{
	var_1 *pCursor= NULL;
	var_1 *pEnd   = NULL;
	var_1 *url    = NULL;
	var_1 *tmpXml = NULL;
	var_1 *title  = NULL;
	var_4 iRet, urlLen, titleLen;
	var_u4 time;

	if (pDoc == NULL || outXml == NULL)
	{
		return -1;
	}
	pCursor = pDoc->buff + pDoc->relativeoff;
	pEnd = pDoc->buff + pDoc->otheroff - 2;
	tmpXml = outXml;
	while (pCursor < pEnd)
	{
		time = *(var_u4*)pCursor;
		urlLen = *(var_2*)(pCursor + 4);
		if (urlLen < 0 || (pCursor + 2 + 4 + urlLen) > pEnd)
		{
			break;
		}
		url = pCursor + 4 + 2;
		pCursor += 4 + 2 + urlLen;
		titleLen = *pCursor;
		if (titleLen <0 || (pCursor + 1 + titleLen) > pEnd)
		{
			break;
		}
		title = pCursor + 1;
		pCursor += 1 + titleLen;
		// url
		iRet = SPRINT(tmpXml, leftLen, "%.*s", urlLen, url);
		if (iRet > 0)
		{
			tmpXml += iRet;
			leftLen -= iRet;
		}
		*tmpXml++ = 0x08;
		leftLen--;
		// category
		iRet = SPRINT(tmpXml, leftLen, "CL:%.*s;SO:%.*s;AR:%.*s;HY:%.*s;SL:%.*s;", pDoc->cllen, pDoc->buff + pDoc->cloff,
					pDoc->solen, pDoc->buff + pDoc->solen, pDoc->arlen, pDoc->buff + pDoc->aroff, pDoc->hylen, pDoc->buff + pDoc->hyoff,
					pDoc->sllen, pDoc->buff + pDoc->sloff);
		if (iRet > 0)
		{
			tmpXml += iRet;
			leftLen -= iRet;
		}
		*tmpXml++ = 0x08;
		leftLen--;
		// title
		iRet = SPRINT(tmpXml, leftLen, "%.*s", titleLen, title);
		if (iRet > 0)
		{
			tmpXml += iRet;
			leftLen -= iRet;
		}
		*tmpXml++ = 0x08;
		leftLen--;
		// encoding
		iRet = SPRINT(tmpXml, leftLen, "%.*s", pDoc->encodelen, pDoc->buff + pDoc->encodeoff);
		if (iRet > 0)
		{
			tmpXml += iRet;
			leftLen -= iRet;
		}
		*tmpXml++ = 0x08;
		leftLen--;
		// next
		*tmpXml++ = 0x07;
		leftLen--;
		totalNum++;
	}
	return (tmpXml - outXml);
}

// �����Ƹ�ʽ���������� time:4�ֽ�;urllen:2�ֽ�;url:urllen�ֽ�;titlelen:1�ֽ�;title:titlelen�ֽ�;
static var_4 GetRelativeNum(DOC_BUF *pDoc)
{
	var_4 num=0;
	var_2 urllen=0, titlelen=0;
	var_1 *pstart = NULL;
	var_1 *pend = NULL;
	if (pDoc == NULL)
	{
		return -1;
	}
	pstart = pDoc->buff + pDoc->relativeoff;	// ���_doc_buf�ṹ��
	pend = pDoc->buff + pDoc->otheroff - 2;		// ��ȥ2�ֽڵ�otherlen
	// �������е�relatives
	while (pstart < pend)
	{
		pstart += 4;							// 4�ֽ�time
		urllen = *(var_2*)pstart;
		if (urllen < 0 || (pstart + 2 + urllen) > pend)
		{
			break;
		}
		pstart += 2 + urllen;					// 2�ֽ�urllen
		titlelen = *pstart;						// titlelenռ1�ֽ�
		if (titlelen < 0)
		{
			break;
		}
		pstart += 1 + titlelen;					// 1�ֽ�titlelen
		num++;
	}
	return num;
}


static var_4 GenerateAddXml(DOC_BUF *pDoc, var_4 sameNum, var_1* outXml, var_4 outXmlLen)
{
	var_4 iRet, processLen = 0;
	if (pDoc == NULL || outXml == NULL || outXmlLen <= 0)
   	{
		return -1;
	}
	iRet = SPRINT(outXml + processLen, outXmlLen - processLen, "<? xml version=\"1.0\" encoding=\"gb2312\" standalone=\"yes\" ?>\n<textIndex>\n");
	if (iRet > 0)
	{
		processLen += iRet;
		iRet = SPRINT(outXml + processLen, outXmlLen - processLen, "<title>%s%.*s%s</title>\n", CDS, pDoc->titlelen, pDoc->buff + pDoc->titleoff, CDE);
	}
	if (iRet > 0)
	{
	    processLen += iRet;
	    iRet = SPRINT(outXml + processLen, outXmlLen - processLen, "<content>%s%.*s%s</content>\n", CDS, pDoc->txtlen, pDoc->buff + pDoc->txtoff, CDE);
	}
	// ��ǩ��Ԫ���轫';'����' '
	if (iRet > 0)
	{
	    processLen += iRet;
		SymbolToSpace(pDoc->buff + pDoc->cloff, pDoc->cllen);
	    iRet = SPRINT(outXml + processLen, outXmlLen - processLen, "<tag_cl>%s%.*s%s</tag_cl>\n", CDS, pDoc->cllen, pDoc->buff + pDoc->cloff, CDE);
	}
	if (iRet > 0)
	{
	    processLen += iRet;
		SymbolToSpace(pDoc->buff + pDoc->sooff, pDoc->solen);
	    iRet = SPRINT(outXml + processLen, outXmlLen - processLen, "<tag_so>%s%.*s%s</tag_so>\n", CDS, pDoc->solen, pDoc->buff + pDoc->sooff, CDE);
	}

	if (iRet > 0)
	{
	    processLen += iRet;
		SymbolToSpace(pDoc->buff + pDoc->aroff, pDoc->arlen);
	    iRet = SPRINT(outXml + processLen, outXmlLen - processLen, "<tag_ar>%s%.*s%s</tag_ar>\n", CDS, pDoc->arlen, pDoc->buff + pDoc->aroff, CDE);
	}
	if (iRet > 0)
	{
	    processLen += iRet;
		SymbolToSpace(pDoc->buff + pDoc->hyoff, pDoc->hylen);
	    iRet = SPRINT(outXml + processLen, outXmlLen - processLen, "<tag_hy>%s%.*s%s</tag_hy>\n", CDS, pDoc->hylen, pDoc->buff + pDoc->hyoff, CDE);
	}
	if (iRet > 0)
	{
	    processLen += iRet;
		SymbolToSpace(pDoc->buff + pDoc->sloff, pDoc->sllen);
	    iRet = SPRINT(outXml + processLen, outXmlLen - processLen, "<tag_sl>%s%.*s%s</tag_sl>\n", CDS, pDoc->sllen, pDoc->buff + pDoc->sloff, CDE);
	}
	if (iRet > 0)
	{
	    processLen += iRet;
		// GetTagBuf�������ص��ǳɹ�д���outXml����
		// return value  less than outXmlLen - processLen
		iRet = GetTagBuf(pDoc, outXml + processLen, outXmlLen - processLen);

		processLen += iRet;
		iRet = SPRINT(outXml + processLen, outXmlLen - processLen, "</textIndex>\n");
	}
	
	if (iRet > 0)
	{
		processLen += iRet;
		iRet = SPRINT(outXml + processLen, outXmlLen - processLen, "<numericIndex>\n<time>%s%u%s</time>\n<title_len>%s%d%s</title_len>\n"
						"<content_len>%s%d%s</content_len>\n<is_picture>%s%d%s</is_picture>\n<newtag_wt>%s%d%s</newtag_wt>\n"
						"<is_newscenter>%s%d%s</is_newscenter>\n</numericIndex>\n", 
						CDS, pDoc->time, CDE,
						CDS, GetWrdCt(pDoc->buff + pDoc->titleoff, pDoc->titlelen), CDE,
						CDS, GetWrdCt(pDoc->buff + pDoc->txtoff, pDoc->txtlen), CDE,
						CDS, (pDoc->picidlen > 0), CDE, 
						CDS, pDoc->newtag_wt, CDE,
						CDS, pDoc->newtag_iscenter, CDE
						);
	}

	if (iRet > 0)
	{
		processLen += iRet;
	    iRet = SPRINT(outXml + processLen, outXmlLen - processLen, "<attachmentInfos>\n<doc_id>%s%lu%s</doc_id>\n"
						"<page_id>%s%u%s</page_id>\n<picture_id>%s%.*s%s</picture_id>\n<picture_size>%s%.*s%s</picture_size>\n"
						"<link>%s%.*s%s</link>\n<coding>%s%.*s%s</coding>\n<repeat_num>%s%d%s</repeat_num>\n<relativity_num>%s%d%s</relativity_num>\n"
						"<event_num>%s%u%s</event_num>\n<event_id>%s%lu%s</event_id>\n",
						CDS, pDoc->docid, CDE,
						CDS, pDoc->time, CDE,
						CDS, pDoc->picidlen, pDoc->buff + pDoc->picidoff, CDE,
						CDS, pDoc->picszlen, pDoc->buff + pDoc->picszoff, CDE,
						CDS, pDoc->urllen, pDoc->buff + pDoc->urloff, CDE,
						CDS, pDoc->encodelen, pDoc->buff + pDoc->encodeoff, CDE,
						CDS, sameNum, CDE,
						CDS, pDoc->relativenum, CDE,
						CDS, sameNum, CDE,
						CDS, pDoc->finger, CDE
						);
	}
	if (iRet > 0)
	{
		processLen += iRet;
		// ��ͼ��Ϣ
		iRet = GetPicsets(pDoc, outXml + processLen, outXmlLen - processLen);
		
		processLen += iRet;
		
		iRet = SPRINT(outXml + processLen, outXmlLen - processLen, "</attachmentInfos>\n");
 		processLen += iRet;

		outXml[processLen] = 0;
	}
	else
	{
		processLen = 0;
	}
	return processLen;
}

// �ϲ���ǩ��pDoc1Ϊ��doc��pDoc2Ϊ��doc�������ߺϲ���ΪpDoc3����
static var_4 MergeDoc(DOC_BUF *pDoc1, DOC_BUF* pDoc2, DOC_BUF* pDoc3, var_1* pMemory, var_4 ith)
{
	var_4 iRet, processLen = 0;
	var_1 *buff = NULL;
	var_1 oldbuf[DEFAULT_TAG_LEN] = {0};
	var_1 newbuf[DEFAULT_TAG_LEN] = {0};
	if (pDoc1 == NULL || pDoc2 == NULL || pDoc3 == NULL)
	{
		return -1;
	}
	buff = (var_1*)(pMemory + ith * ALLOC_SIZE);
	// �Ծ�doc��pDoc1Ϊ׼
	pDoc3->docid = pDoc1->docid;
	*(var_u8*)buff = pDoc1->docid;
	////////////////////////////////////////////
	// ����λ��doclen
	// /////////////////////////////////////////
	pDoc3->datatype = pDoc1->datatype;
	*(var_u1*)(buff + 12) = pDoc1->datatype;
	pDoc3->sourcetype = pDoc1->sourcetype;
	*(var_1*)(buff + 13) = pDoc1->sourcetype;
	pDoc3->force = pDoc1->force;
	*(var_1*)(buff + 14) = pDoc1->force;
	pDoc3->newtag_firstpub = pDoc1->newtag_firstpub;
	*(var_1*)(buff + 15) = pDoc1->newtag_firstpub;
	pDoc3->newtag_iscenter = pDoc1->newtag_iscenter;
	*(var_u1*)(buff + 16) = pDoc1->newtag_iscenter;
	pDoc3->eventid = pDoc1->eventid;
	*(var_u8*)(buff + 17) = pDoc1->eventid;
	pDoc3->finger = pDoc1->finger;
	*(var_u8*)(buff + 25) = pDoc1->finger;
	// title id �Ѳ���
	pDoc3->titleid = pDoc1->titleid;
	*(var_u8*)(buff + 33) = pDoc1->titleid;
	// Ԥ��8���ֽ�
	//
	pDoc3->time = pDoc1->time;
	*(var_u4*)(buff + 49) = pDoc1->time;
	processLen = processLen + 53;

	/////////////////////////////////////////////
	// �ϲ�tag��Ԫ��
	// //////////////////////////////////////////
	// cl
	memcpy(oldbuf, pDoc1->buff + pDoc1->cloff, pDoc1->cllen);
	memcpy(newbuf, pDoc2->buff + pDoc2->cloff, pDoc2->cllen);
	iRet = MergeSingleTag(oldbuf, pDoc1->cllen, newbuf, pDoc2->cllen, buff + processLen + 2);
	if (iRet < 0 || iRet > 128)
	{
		return -2;
	}
	*(var_2*)(buff + processLen) = iRet;
	pDoc3->cllen = iRet;
	pDoc3->cloff = processLen + 2; // 2bytes: sizeof(pDoc->cllen)
	processLen = processLen + iRet + 2;

	// so
	memcpy(oldbuf, pDoc1->buff + pDoc1->sooff, pDoc1->solen);
	memcpy(newbuf, pDoc2->buff + pDoc2->sooff, pDoc2->solen);
	iRet = MergeSingleTag(oldbuf, pDoc1->solen, newbuf, pDoc2->solen, buff + processLen + 2);
	if (iRet < 0 || iRet > 128)
	{
		return -3;
	}
	*(var_2*)(buff + processLen) = iRet;
	pDoc3->solen = iRet;
	pDoc3->sooff = processLen + 2; // 2bytes: sizeof(pDoc->solen)
	processLen = processLen + iRet + 2;
	
	// ar
	memcpy(oldbuf, pDoc1->buff + pDoc1->aroff, pDoc1->arlen);
	memcpy(newbuf, pDoc2->buff + pDoc2->aroff, pDoc2->arlen);
	iRet = MergeSingleTag(oldbuf, pDoc1->arlen, newbuf, pDoc2->arlen, buff + processLen + 2);
	if (iRet < 0 || iRet > 128)
	{
		return -4;
	}
	*(var_2*)(buff + processLen) = iRet;
	pDoc3->arlen = iRet;
	pDoc3->aroff = processLen + 2; // 2bytes: sizeof(pDoc->arlen)
	processLen = processLen + iRet + 2;
	
	// hy
	memcpy(oldbuf, pDoc1->buff + pDoc1->hyoff, pDoc1->hylen);
	memcpy(newbuf, pDoc2->buff + pDoc2->hyoff, pDoc2->hylen);
	iRet = MergeSingleTag(oldbuf, pDoc1->hylen, newbuf, pDoc2->hylen, buff + processLen + 2);
	if (iRet < 0 || iRet > 128)
	{
		return -5;
	}
	*(var_2*)(buff + processLen) = iRet;
	pDoc3->hylen = iRet;
	pDoc3->hyoff = processLen + 2; // 2bytes: sizeof(pDoc->hylen)
	processLen = processLen + iRet + 2;

	// sl
	memcpy(oldbuf, pDoc1->buff + pDoc1->sloff, pDoc1->sllen);
	memcpy(newbuf, pDoc2->buff + pDoc2->sloff, pDoc2->sllen);
	iRet = MergeSingleTag(oldbuf, pDoc1->sllen, newbuf, pDoc2->sllen, buff + processLen + 2);
	if (iRet < 0 || iRet > 128)
	{
		return -6;
	}
	*(var_2*)(buff + processLen) = iRet;
	pDoc3->sllen = iRet;
	pDoc3->sloff = processLen + 2; // 2bytes: sizeof(pDoc->sllen)
	processLen = processLen + iRet + 2;

	pDoc3->urllen = pDoc1->urllen;
	pDoc3->urloff = processLen + 2;
	*(var_2*)(buff + processLen) = pDoc1->urllen;
	memcpy(buff + processLen + 2, pDoc1->buff + pDoc1->urloff, pDoc1->urllen);
	processLen = processLen + 2 + pDoc1->urllen;
	
	pDoc3->picidlen = pDoc1->picidlen;
	pDoc3->picidoff = processLen + 1;
	*(var_1*)(buff + processLen) = pDoc1->picidlen;
	memcpy(buff + processLen + 1, pDoc1->buff + pDoc1->picidoff, pDoc1->picidlen);
	processLen = processLen + 1 + pDoc1->picidlen;
	
	pDoc3->picszlen = pDoc1->picszlen;
	pDoc3->picszoff = processLen + 1;
	*(var_1*)(buff + processLen) = pDoc1->picszlen;
	memcpy(buff + processLen + 1, pDoc1->buff + pDoc1->picszoff, pDoc1->picszlen);
	processLen = processLen + 1 + pDoc1->picszlen;
	
	pDoc3->encodelen = pDoc1->encodelen;
	pDoc3->encodeoff = processLen + 1;
	*(var_u1*)(buff + processLen) = pDoc1->encodelen;
	memcpy(buff + processLen + 1, pDoc1->buff + pDoc1->encodeoff, pDoc1->encodelen);
	processLen = processLen + 1 + pDoc1->encodelen;

	pDoc3->titlelen = pDoc1->titlelen;
	pDoc3->titleoff = processLen + 1;
	*(var_u1*)(buff + processLen) = pDoc1->titlelen;
	memcpy(buff + processLen + 1, pDoc1->buff + pDoc1->titleoff, pDoc1->titlelen);
	processLen = processLen + 1 + pDoc1->titlelen;

	pDoc3->txtlen = pDoc1->txtlen;
	pDoc3->txtoff = processLen + 4;
	*(var_4*)(buff + processLen) = pDoc1->txtlen;
	memcpy(buff + processLen + 4, pDoc1->buff + pDoc1->txtoff, pDoc1->txtlen);
	processLen = processLen + 4 + pDoc1->txtlen;

	pDoc3->relativelen = pDoc1->relativelen;
	pDoc3->relativeoff = processLen + 2;
	*(var_2*)(buff + processLen) = pDoc1->relativelen;
	memcpy(buff + processLen + 2, pDoc1->buff + pDoc1->relativeoff, pDoc1->relativelen);
	processLen = processLen + 2 + pDoc1->relativelen;

	pDoc3->otherlen = pDoc1->otherlen;
	pDoc3->otheroff = processLen + 2;
	*(var_2*)(buff + processLen) = pDoc1->otherlen;
	memcpy(buff + processLen + 2, pDoc1->buff + pDoc1->otheroff, pDoc1->otherlen);
	processLen = processLen + 2 + pDoc1->otherlen;
	
	pDoc3->buflen = processLen;
	pDoc3->buff = buff;
	pDoc3->relativenum = pDoc1->relativenum;
	
	pDoc3->doclen = processLen - 12;
	*(var_4*)(buff + 8)= pDoc3->doclen;

	return 0;
}


static var_4 MergeSingleTag(var_1* oldtag, var_4 oldlen, var_1 *newtag, var_4 newlen, var_1* dsttag)
{
	var_1 tmpc;
	var_1 *tagsplit = NULL;
	var_4 i, curPos = -1;
	if (oldlen == 0 && newlen == 0 )	
	{
		return 0;
	}
	if (oldlen > DEFAULT_TAG_LEN || newlen > DEFAULT_TAG_LEN)
	{
		return 0;
	}
	if (oldlen == 0)
	{
		memcpy(dsttag, newtag, newlen);
		return newlen;
	}
	if (newlen == 0)
	{
		memcpy(dsttag, oldtag, oldlen);
		return oldlen;
	}
	//��':'����';'   ���磺����:����; --> ����;����;
	for (i = 0; i < newlen; i++)
	{
		if (newtag[i] == ':' || newtag[i] == '#')
		{
			newtag[i] = ';';
		}
	}
	// ��;�ŷָ��ǩ��;�ű�������
	if (oldtag[oldlen - 1] != ';')
	{
		oldtag[oldlen] = ';';
		oldlen++;
	}
	oldtag[oldlen] = 0;
	if (newtag[newlen - 1] != ';')
	{
		newtag[newlen] = ';';
		newlen++;
	}
	newtag[newlen] = 0;
	// �ȿ����±�ǩ
	memcpy(dsttag, newtag, newlen);
	dsttag[newlen] = 0;
	tagsplit = strchr(oldtag, ';');
	while (tagsplit)
	{
		tmpc = *(tagsplit + 1);   // �ֽ��ҵ���;�ź�һλ��0���ݴ���tmpc�У������ٻ�ԭ����
		*(tagsplit + 1) = 0;
		// ���ǣ�"��ʳ�Ļ���""��ʳ��""�Ļ���" ����������ʵı�ǩ
		if (strstr(dsttag, oldtag) == NULL ||
				// ��ֹ���� dsttag����ʳ�Ļ���oldtag���Ļ���ʱ����ǩ������������
				 (curPos > 0 && dsttag[curPos] != ';'))
		{
			// curPosָ��;��ǰһ��λ��
			curPos = curPos + tagsplit - oldtag + 1;
			// ��ֹ��ǩ�������
			if (newlen + tagsplit - oldtag + 1 >= DEFAULT_TAG_LEN)
			{
				break;
			}
			memcpy(dsttag + newlen, oldtag, tagsplit - oldtag + 1);
			newlen = newlen + tagsplit - oldtag + 1;
			dsttag[newlen] = 0;
		}
		*(tagsplit + 1) = tmpc;
		oldtag = tagsplit + 1;
		tagsplit = strchr(oldtag, ';');
	}
	
	return newlen;
}

static var_4 GetWrdCt(var_1 *buff, var_4 bufLen)
{
	var_4 last = 0, ct = 0;
	if (bufLen == 0)
	{
		return 0;
	}
	for(var_4 i = 0; i < bufLen; i++)     
	{  
		if(buff[i] == ' ')      
		{
			if(last == 0)
	        {
	         	ct++;
		        last = 1;
			}
		}
		else
		{
			last = 0;
		}
	}
	return (ct+1);
}

static var_4 GetPicsets(DOC_BUF* pDoc, var_1 *outXml, var_4 leftLen)
{
	var_1* start_buf = NULL;
	var_1* end_buf = NULL;
	var_1* picid_buf = NULL;
	var_1* picsize_buf = NULL;
	var_2 picidLen, picsizeLen;
	var_4 iRet, processLen = 0;

	if (pDoc->newdata_flag && pDoc->picset_len > 0)
	{
		iRet = SPRINT(outXml + processLen, leftLen - processLen, "<pic_set>%s", CDS);
		processLen += iRet;

		start_buf = pDoc->buff + pDoc->picset_off;
		end_buf = pDoc->buff + pDoc->picset_off + pDoc->picset_len;
		while (start_buf < end_buf)
		{
			picidLen = *(var_u1*)start_buf;
			picid_buf = start_buf + 1;
			picsizeLen = *(var_u1*)(start_buf + 1 + picidLen);
			picsize_buf = start_buf + 1 + picidLen + 1;
			iRet = SPRINT(outXml + processLen, leftLen - processLen, "%.*s%s%.*s\n", picidLen, picid_buf, "*", picsizeLen, picsize_buf);
			processLen += iRet;
			start_buf += 2 + picidLen + picsizeLen; 
		}
		iRet = SPRINT(outXml + processLen, leftLen - processLen, "%s</pic_set>\n", CDE);
		processLen += iRet;
	}
	else
	{
		iRet = SPRINT(outXml + processLen, leftLen - processLen, "<pic_set>%s%s</pic_set>\n", CDS, CDE);
		processLen += iRet;
	}
	return processLen;
}

static var_4 GetTagBuf(DOC_BUF *pDoc, var_1 *outXml, var_4 leftLen)
{
	var_1 tagType, tagNameLen;
	var_2 tagValueLen;
	var_4 iRet, tagsLen, processLen = 0;

	var_1 *bufCursor = NULL;
	var_1 *bufEnd = NULL;
	var_1 *tagName = NULL;
	var_1 *tagValue = NULL;

	if (pDoc->newdata_flag)
	{
		// �¼��ʳ�
		if (pDoc->eventword_len + 28 < leftLen)
		{
			iRet = SPRINT(outXml + processLen, leftLen, "<event_words>%s%.*s%s</event_words>\n", 
					CDS, pDoc->eventword_len, pDoc->buff + pDoc->eventword_off, CDE);
		}
		// �����ǩ
		/*
		 * TYPE		1			TAG���ͣ�0�ı���1������2����
		 * NAMELEN	1			TAG���ֳ���
		 * NAME		NAMELEN		TAG����
		 * VALUELEN	2			TAGֵ����
		 * VALUE	VALUELEN	TAGֵ
		 * ......
		 * �ظ�����
		 **/
		bufCursor = pDoc->buff + pDoc->tag_off;
		bufEnd = pDoc->buff + pDoc->tag_off + pDoc->tag_len;
		while (bufCursor < bufEnd)
		{
			tagType = *bufCursor;
			tagNameLen = *(bufCursor + 1);
			tagName = bufCursor + 2;
			bufCursor += 2 + tagNameLen;
			tagValueLen = *(var_2*)bufCursor;
			tagValue = bufCursor + 2;
			bufCursor += 2 + tagValueLen;
			// ��ȡһ��tag����Ϣ��
			if (tagType == 0)	// ���ͣ�0�ı���1������2����
			{
				UpperToLower(tagName, tagNameLen);
				SymbolToSpace(tagValue, tagValueLen);
				// bytes
				// 1:TAGTYPE;1:TAGNAMELEN;2:TAGVALUELEN;20:<newtag_></newtag_>\n		
				if (processLen + 24 + tagNameLen + tagValueLen < leftLen)
				{	
					iRet = SPRINT(outXml + processLen, leftLen, "<newtag_%.*s>%s%.*s%s</newtag_%.*s>\n", 
							tagNameLen, tagName, CDS, tagValueLen, tagValue, CDE, tagNameLen, tagName);
					processLen += iRet;
					leftLen -= iRet;
				}
				else
				{
					break;
				}
			}
			else if (tagType == 1) // ����newtag_wt
			{
				var_1 tmpc1 =  tagName[tagNameLen]; // ��'\0'�ض�tagName,������strstr�������ضϵ��ַ�����ʱtmpc1��������������ɺ�ָ�tagName
				tagName[tagNameLen] = 0;
				if (strstr(tagName, "wt") || strstr(tagName, "WT"))
				{
					var_1 tmpc2 = tagValue[tagValueLen];// ����ͬ��
					tagValue[tagValueLen] = 0;
					pDoc->newtag_wt = atoi(tagValue);

					/*if (pDoc->newtag_wt <= 0 || pDoc->newtag_wt > 10)
					{
						pDoc->newtag_wt = 1;
					}
					*/
					tagValue[tagValueLen] = tmpc2; // ��ԭtagValue
				}
				tagName[tagNameLen] = tmpc1; // ��ԭtagName
			}
		}
	}
	else
	{
		// �����ݣ�д����ǩ
		iRet = SPRINT(outXml, leftLen, "<newtag_kw>%s%s</newtag_kw>\n<newtag_zz>%s%s</newtag_zz>\n"
							"<newtag_tp>%s%s</newtag_tp>\n<newtag_lv>%s%s</newtag_lv>\n"
							"<newtag_pr>%s%s</newtag_pr>\n<newtag_ex1>%s%s</newtag_ex1>\n"
							"<newtag_ex2>%s%s</newtag_ex2>\n<newtag_ex3>%s%s</newtag_ex3>\n"
							"<newtag_ex4>%s%s</newtag_ex4>\n<newtag_ex5>%s%s</newtag_ex5>\n"
							"<newtag_ex6>%s%s</newtag_ex6>\n<newtag_ex7>%s%s</newtag_ex7>\n"
							"<newtag_ex8>%s%s</newtag_ex8>\n<newtag_ex9>%s%s</newtag_ex9>\n"
							"<newtag_ex10>%s%s</newtag_ex10>\n<event_words>%s%s</event_words>\n",
							CDS, CDE, CDS, CDE, CDS, CDE, CDS, CDE, CDS, CDE, CDS, CDE, 
							CDS, CDE, CDS, CDE, CDS, CDE, CDS, CDE, CDS, CDE, CDS, CDE,
							CDS, CDE, CDS, CDE, CDS, CDE, CDS, CDE);
		processLen += iRet;
	}
	// �����ݻ������ݵ�û��Ȩ���ֶΣ���ֵweight=1
	if (pDoc->newdata_flag == 0 || pDoc->newtag_wt <= 0 || pDoc->newtag_wt > 10)
	{
		pDoc->newtag_wt = 1;
	}
	return processLen;
}

// ����other�ֶΣ��ж��Ƿ�Ϊ�¸�ʽ���ݣ�
// ���Ϊ�¸�ʽ���ݣ�����չ��ǩ��ֵ��
static var_vd ParseOtherField(DOC_BUF* pDoc)
{
	var_1 tagType, tagNameLen;
	var_2 tagValueLen;
	var_2 nextother_len;
	var_4 iRet, tagsLen, processLen = 0;
	var_4 isNewData = 1;

	var_1 *bufCursor = NULL;
	var_1 *bufEnd = NULL;
	var_1 *tagName = NULL;
	var_1 *tagValue = NULL;
	var_1 *nextother_buf = NULL;


	if (pDoc->otherlen <= 0)
	{
		isNewData = 0;
	}
	if (isNewData)
	{
		bufCursor = pDoc->buff + pDoc->otheroff;
		bufEnd = pDoc->buff + pDoc->otheroff + pDoc->otherlen;

		// �¼��ʳ�
		pDoc->eventword_len = *(var_2*)bufCursor;
		if (pDoc->eventword_len < 0 || pDoc->eventword_len > 1024)
		{
			isNewData = 0;
		}
		else
		{
			bufCursor += 2 + pDoc->eventword_len; // 2bytes : sizeof(pDoc->eventword_len)
			if (bufCursor > bufEnd)
			{
				isNewData = 0;
			}
		}
	}
	if (isNewData)
	{
		pDoc->eventword_off = pDoc->otheroff + 2; // 2bytes : sizeof(pDoc->otherlen)
		// tag��
		pDoc->tag_len = *(var_2*)bufCursor;
		if (pDoc->tag_len < 0 || pDoc->tag_len > 1650)
		{
			isNewData = 0;
		}
		else
		{
			bufCursor += 2 + pDoc->tag_len;		  // 2bytes : sizeof(pDoc->tag_len)
			if (bufCursor > bufEnd)
			{
				isNewData = 0;
			}
		}
	}
	if (isNewData)
	{
		pDoc->tag_off = pDoc->eventword_off + pDoc->eventword_len + 2; // 2bytes: sizeof(pDoc->tag_len)
		// ��ʱbufCursorָ��nextother_len, ���ֶ�δʹ����λ0
		pDoc->newdata_flag = 1;
	}
	else
	{
		pDoc->newdata_flag = 0;
		// ������ֱ�ӷ��أ�Ĭ��weightΪDEFAULT_DOC_WEIGHT
		// ע�⣺��AddDocʱ��û��Ȩ�ص����Ÿ�ֵweigth=5(Ȩ�ؽϴ�)����Ҫ��Ϊ���ڻ�leaderʱ�����ױ��滻
		//       �����ɷ��͸�������xml��ʱ����������ݣ�û��Ȩ�أ��������¸�ֵweight=1
		pDoc->newtag_wt = DEFAULT_DOC_WEIGHT;
		return;
	}
	bufCursor = pDoc->buff + pDoc->tag_off;
	bufEnd = pDoc->buff + pDoc->tag_off + pDoc->tag_len;
	while (bufCursor < bufEnd)
	{
		tagType = *bufCursor;
		tagNameLen = *(bufCursor + 1);
		tagName = bufCursor + 2;
		bufCursor += 2 + tagNameLen;
		tagValueLen = *(var_2*)bufCursor;
		tagValue = bufCursor + 2;
		bufCursor += 2 + tagValueLen;
		// ��ȡһ��tag����Ϣ��
		if (tagType == 0)	// ���ͣ�0�ı���1������2����
		{
			//do nothing
		}
		else if (tagType == 1) // ����newtag_wt
		{
			var_1 tmpc1 =  tagName[tagNameLen]; // ��'\0'�ض�tagName,������strstr�������ضϵ��ַ�����ʱtmpc1��������������ɺ�ָ�tagName
			tagName[tagNameLen] = 0;
			if (strstr(tagName, "wt") || strstr(tagName, "WT"))
			{
				var_1 tmpc2 = tagValue[tagValueLen];// ����ͬ��
				tagValue[tagValueLen] = 0;
				pDoc->newtag_wt = atoi(tagValue);
				tagValue[tagValueLen] = tmpc2; // ��ԭtagValue
			}
			tagName[tagNameLen] = tmpc1; // ��ԭtagName
		}
	}
	if (pDoc->newtag_wt <= 0 || pDoc->newtag_wt > 10)
	{
		pDoc->newtag_wt = DEFAULT_DOC_WEIGHT;
	}
	//������ͼЭ��
	nextother_buf = bufEnd;
	nextother_len = *(var_2*)(nextother_buf);
	pDoc->nextotherlen = nextother_len;
	pDoc->nextotheroff = pDoc->tag_off + pDoc->tag_len + 2;
	if (nextother_len > 4)
	{
		pDoc->picset_len = *(var_2*)(nextother_buf + 2);
		pDoc->picset_off = pDoc->nextotheroff + 2;
	}
	else
	{
		pDoc->picset_len = 0;
	}
	return;
}

static var_u8 GetDocWeight(DOC_BUF *pDoc)
{
	var_u8 weight;
	// get doc weight
	if (pDoc->newdata_flag==1)
	{
		weight = pDoc->newtag_wt;
	}
	else
	{
		weight = DEFAULT_DOC_WEIGHT;
	}
	return weight;
}

static var_4 GetDigest(var_1* inBuf, var_4 inBufLen, var_1* outBuf)
{
	var_4 i =0;
	var_1 *tmpInBuf = inBuf;
	var_1 *tmpOutBuf = outBuf;
	
	if (inBuf == NULL || outBuf == NULL)
	{
		return -1;
	}
	while (i < 256 && i < inBufLen)
	{
		if (*tmpInBuf == ' ' || *tmpInBuf == 0)
		{
			tmpInBuf++;
			inBufLen--;
		}
		if (*tmpInBuf > 0)
		{
			*tmpOutBuf = *tmpInBuf;
			tmpOutBuf++;
			tmpInBuf++;
			i++;
		}
		else
		{
			*tmpOutBuf = *tmpInBuf;
			*(tmpOutBuf + 1) = *(tmpInBuf + 1);
			tmpOutBuf += 2;
			tmpInBuf += 2;
			i += 2;
		}
	}
	return (tmpOutBuf - outBuf);
}

static var_4 DigestXml(DOC_BUF* pDoc, var_1* outXml, var_4& leftLen, var_4 optType, var_4 isSelf)
{
	var_4 iRet, i, j;
	var_1 soTagBuf[128] = {0};
	var_1 *pTmp = NULL;
	var_1 *tmpXml = outXml;

	if (pDoc == NULL || outXml == NULL || leftLen <= 1152)
	{
		return -1;
	}
	if (isSelf == 2)
	{
		// 1301 ��ͬ����+���
		// 1401 �������+���
		// 1501 �¼�����+���
		if (optType == 1301 || optType == 1401 || optType == 1501)
		{
			iRet = GetDigest(pDoc->buff + pDoc->txtoff, pDoc->txtlen, tmpXml);
			if (iRet > 0)
			{
				tmpXml += iRet;
				leftLen -= iRet;		
			}
			*tmpXml++ = 0x07;
			leftLen--;
		}
		else
		{
			// ȫ��
			for (i=0, j=0; j < leftLen && i < pDoc->txtlen; ++i)
			{
				if (*(pDoc->buff + pDoc->txtoff + i) != ' ')
				{
					*tmpXml++ = *(pDoc->buff + pDoc->txtoff + i);
					j++;
				}
			}
			leftLen -= j;
			*tmpXml++ = 0x07;
		}
	}
	// isSelf <> 2
	else 
	{
		if (optType == 1303 || optType == 1403)
		{
			if (isSelf == 1)
			{
				*tmpXml++ = '1';
			}
			else
			{
				*tmpXml++ = '0';
			}
			*tmpXml++ = 0x08;
			leftLen -= 2;
		}
		// url, 0x08��β 
		iRet = SPRINT(tmpXml, leftLen, "%.*s", pDoc->urllen, pDoc->buff + pDoc->urloff);
		if (iRet > 0)
		{
			tmpXml  += iRet;
			leftLen -= iRet;
		}
		*tmpXml++ = 0x08;
		leftLen--;

		// tags, 0x08��β�����so��ǩ�ж��ֵ��ֻȡһ��
		pTmp = pDoc->buff + pDoc->sooff;
		for (i = 0; i < pDoc->solen; ++i)
		{
			if (*pTmp == ' '||*pTmp == ':'||*pTmp == ';'||*pTmp=='#')
			{
				pTmp++;
			}
			else
			{
				break;
			}
		}
		for (j = 0; i< pDoc->solen; ++i)
		{
			if (*pTmp == ' '||*pTmp == ':'||*pTmp == ';'||*pTmp=='#')
			{
				break;
			}
			else
			{
				soTagBuf[j++] = *pTmp;
				pTmp++;
			}
		}
		//
		iRet = SPRINT(tmpXml, leftLen, "CL %.*s;SO %.*s;AR %.*s;HY %.*s;SL %.*s;",pDoc->cllen,  pDoc->buff + pDoc->cloff,
						j, soTagBuf, pDoc->arlen, pDoc->buff + pDoc->aroff, pDoc->hylen, pDoc->buff + pDoc->hyoff,
						pDoc->sllen, pDoc->buff + pDoc->sloff); 
		if (iRet > 0)
		{
			tmpXml += iRet;
			leftLen -= iRet;
		}
		*tmpXml++ = 0x08;
		leftLen--;

		// title, �128�ֽ�
		for (i=0, j=0; j < 128 && i < pDoc->titlelen; ++i)
		{
			if (*(pDoc->buff + pDoc->titleoff + i) != ' ')
			{
				*tmpXml++ = *(pDoc->buff + pDoc->titleoff + i);
				j++;
			}
		}
		*tmpXml++ = 0x08;
		leftLen -= j + 1;
		// time
		iRet = SPRINT(tmpXml, leftLen, "%u", pDoc->time);
		if (iRet > 0)
		{
			tmpXml += iRet;
			leftLen -= iRet;
		}
		*tmpXml++ = 0x08;
		leftLen--;

		// encode
		iRet = SPRINT(tmpXml, leftLen, "%.*s", pDoc->encodelen, pDoc->buff + pDoc->encodeoff);
		if (iRet > 0)
		{
			tmpXml += iRet;
			leftLen -= iRet;
		}
		*tmpXml++ = 0x08;
		leftLen--;
		*tmpXml++ = 0x07;
		leftLen--;
	}
	return (tmpXml - outXml);
}
#endif

