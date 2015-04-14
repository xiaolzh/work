#include "ListRanking.h"
#include <dlfcn.h>
#include <unistd.h>
#include <sys/stat.h>
#include "Class.h"
#include "configer.h"
#include "logger.h"
#include "utilities.h"
#include "UtilTemplateFunc.h"
#include "scripter/common_func.h"
#include "timer.h"

/// @see here we rewrite the LOG macro as COMMON_LOG if it is released, 
///      but keep it when DEBUG mode, for easy to UNIT TEST
#ifndef DEBUG
#ifdef LOG
#undef LOG
#endif
#define LOG(level, format, ...) \
    do{ \
        switch(level) { \
            case LOG_DEBUG: \
                COMMON_LOG(SL_DEBUG, format, ##__VA_ARGS__ ); \
                break; \
            case LOG_INFO: \
                COMMON_LOG(SL_INFO, format, ##__VA_ARGS__ ); \
                break; \
            case LOG_WARN: \
                COMMON_LOG(SL_WARN, format, ##__VA_ARGS__ ); \
                break; \
            case LOG_ERROR: \
                COMMON_LOG(SL_ERROR, format, ##__VA_ARGS__ ); \
                break; \
            case LOG_FATAL: \
                COMMON_LOG(SL_FATAL, format, ##__VA_ARGS__ ); \
                break; \
        } \
    }while(0)
#endif  // DEBUG

static const char* SO_DIR = "./list_ranking_so/";

CListRanking::~CListRanking() {
    typedef map<u64, Weighter*>::iterator MItor;
    for(MItor it = m_custom_weighters.begin(); it != m_custom_weighters.end();
        ++ it) {
        delete it->second;
        it->second = NULL;
    }
    m_custom_weighters.clear();

    for(size_t i=0; i < m_dl_handles.size(); ++i) {
        dlclose(m_dl_handles[i]);
        m_dl_handles[i] = NULL;
    }
    m_dl_handles.clear();
    LOG(LOG_INFO, "list ranking leave\n");
}

/*
 *初始化函数需要具体模块重载
 *Parameter psdi 搜索数据传递
 *Parameter strConf 模块各自的配置文件地址
 *return true初始化成功，false初始化失败 
 */
bool CListRanking::Init(SSearchDataInfo* psdi, const string& strConf) {
    /// cost more than 10 seconds should give a warning
    TimerCounter tc("init list_ranking", LOG_WARN, 10000);

    CModule::Init(psdi, strConf);

    //init priority docids
    _LoadPriorityDocid();

    cls_buf cbInvalid;
    cbInvalid.chCLs[0] = 0;
    //if(!m_staticCatPathName.load_serialized_hash_file(
    //  (m_strModulePath + CLASS_PATH_TO_NAME).c_str(), cbInvalid))
    //return false;
    /// check if the SO FILE is exist
    if( -1 == access(SO_DIR, F_OK)) {
        mkdir(SO_DIR, 0777);
    }
    /// init the field profiles
    if( !_LoadProfile("stock_status", m_stock_profile)) 
        m_stock_profile = NULL;
    if( !_LoadProfile("sale_week", m_sale_week_profile)) 
        m_sale_week_profile = NULL;
    if( !_LoadProfile("is_share_product", m_is_share_product_profile))
        m_is_share_product_profile = NULL;
    if( !_LoadProfile("total_review_count", m_review_count_profile))
        m_review_count_profile = NULL;
    if( !_LoadProfile("score", m_score_profile)) 
        m_score_profile = NULL;
    if( !_LoadProfile("first_input_date", m_first_input_date_profile))
        m_first_input_date_profile = NULL;
    if( !_LoadProfile("publish_date", m_publish_date_profile)) 
        m_publish_date_profile = NULL;
    


    /// set default search weight coffs
    m_default_weight.stock = 10;
    m_default_weight.commerce = 40;
    m_default_weight.review = 10;
    m_default_weight.sale = 10;

    /// set default gobal weighter
    Weighter* wt_ptr = new DefaultWeighter;
    m_custom_weighters[0x00] = wt_ptr;

    if( !_LoadConfig(strConf)) {
        LOG(LOG_ERROR, "Fail to load config [%s]", strConf.c_str());
        return false;
    }

    LOG(LOG_INFO, "list ranking init ok");
    return true;
}

void CListRanking::QueryRewrite(hash_map<string,string>& hmParams) {
    /// - use query_pack's QueryRewrite
    MOD_MAP::iterator i = m_mapMods->find("query_pack");
    if ( i != m_mapMods->end())
        i->second.pMod->QueryRewrite(hmParams);
}

IAnalysisData* CListRanking::QueryAnalyse(SQueryClause& qc) {
    ListAnalysisData* pa = new ListAnalysisData;
    pa->m_hmUrlPara = qc.hmUrlPara;//保存全部URL参数
    if( _IsMall(qc.cat)) pa->m_list_type = ListAnalysisData::MALL_LIST;
    else pa->m_list_type = ListAnalysisData::PUB_LIST;
    LOG(LOG_DEBUG, "list type [%s]", 
        pa->m_list_type==ListAnalysisData::MALL_LIST?"mall":"pub");
    pa->m_cat_id = qc.cat;
    /// save the current time here for later computing
    pa->cur_date = time(NULL);
    pa->page_size = atoi(pa->m_hmUrlPara[PS].c_str());
    if (pa->page_size <= 0) 
        LOG(LOG_ERROR, "Wrong page size [%d]", pa->page_size);
    return pa;
}

void CListRanking::ComputeWeight(IAnalysisData* pad, vector<SResult>& vRes){
    /// compute weight using feed back data, etc.
    ListAnalysisData* pa = (ListAnalysisData*)pad;
    u64 cat_id = _GetWeightCat(pa->m_cat_id);    
    hash_map<long long, hash_map<long long, int> >::iterator huh_iter;
    hash_map<long long, int>::iterator hui_iter;
    hash_map<int, int>::iterator hii_iter;
    hash_map<int, int> h_doc_score;
    vector<long long> v_pid;
    vector<int> v_score;
    vector<int> v_docid;
    int pid_filed_id = GetFieldId("product_id");
    huh_iter = m_priority_cat_pid.find(pa->m_cat_id);
    if (huh_iter != m_priority_cat_pid.end()) {
        for(hui_iter = huh_iter->second.begin(); hui_iter != huh_iter->second.end(); hui_iter++){
            v_pid.push_back(hui_iter->first);
            v_score.push_back(hui_iter->second);
        }
        m_funcGetDocsByPkPtr(m_pSearcher, pid_filed_id, v_pid, v_docid);
        for(size_t i = 0; i < v_docid.size(); i++){
            h_doc_score.insert(make_pair(v_docid[i], v_score[i]));
        }
    }

    //long long product_id;

    for(size_t i=0; i< vRes.size(); ++i) {
        vRes[i].nScore = m_custom_weighters[cat_id]->Work(vRes[i].nDocId, pa);
        if(huh_iter != m_priority_cat_pid.end()){
            hii_iter = h_doc_score.find(vRes[i].nDocId);
            if(hii_iter != h_doc_score.end()){
                vRes[i].nScore = hii_iter->second;
            }
        }
    }
} 

void CListRanking::ReRanking(vector<SResult>& vRes, IAnalysisData* pad) {
    /// @todo nothing yet
}

void CListRanking::FillGroupByData(vector<pair<int, 
    vector<SGroupByInfo> > >& vGpInfo) {
    /**
     *   @see here is useless, searcher will call query_pack
     typedef pair<int, vector<SGroupByInfo> > PIV;
     vector<PIV>::iterator i;
     vector<SGroupByInfo>::iterator j;
     cls_buf cb;
     for (i = vGpInfo.begin(); i != vGpInfo.end(); ++i)
     {
     if (m_vFieldInfo[i->first].nSpecialType == CAT_FIELD)
     {
     for (j = i->second.begin(); j != i->second.end(); ++j)
     {
     cb = m_staticCatPathName[j->nGid];
     if(!cb.chCLs[0])
     continue;
     strncpy(j->bufName, cb.chCLs, sizeof(j->bufName));
     j->bufName[sizeof(j->bufName)-1] = 0;
     }
     }
     }
     */
}

void CListRanking::SortForDefault(vector<SResult>& vRes, int from, int to,
    IAnalysisData* pad){
    /// check if need to scatter sort or not
    ListAnalysisData* pa = (ListAnalysisData*)pad;
    u64 cat_id = pa->m_cat_id;
    u64 base_cat = 0x0;
    int cat_lvl = GetClsLevel(cat_id);
    for ( int i = cat_lvl; i > 0; --i) {
        base_cat = GetClassByLevel(i, cat_id);
        if ( m_scatters.end() != m_scatters.find(base_cat)) {
            int unit_size = pa->page_size > 5 ? pa->page_size : 5;
            int unit = (unit_size << 16) + m_scatters[base_cat].unit_num;
            m_funcScatterResult(m_pSearcher, vRes, 
                m_scatters[base_cat].priority_list, 
                m_scatters[base_cat].field_id, unit, to);
            return;
        }
    }
    /// default sort, using default score
    SortRange(vRes ,from, to); 
}

void CListRanking::ShowResult(IAnalysisData* pad, CControl& ctrl,
    GROUP_RESULT &vGpRes, vector<SResult>& vRes, vector<string>& vecRed,
    string& strRes) {
    MOD_MAP::iterator i = m_mapMods->find("query_pack");
    if ( i != m_mapMods->end())
        i->second.pMod->ShowResult(pad, ctrl, vGpRes, vRes, vecRed, strRes);
}

int CListRanking::ComputeDefaultWeight(int doc_id, IAnalysisData* pa) {
    int score = 0;
    /// default weight compute
    if(m_default_weight.stock > 0)
        score += _GetStockWeight(doc_id, m_default_weight.stock);
    if(m_default_weight.sale > 0)
        score += _GetSaleWeight(doc_id, m_default_weight.sale);
    if(m_default_weight.review > 0)
        score += _GetReviewWeight(doc_id, m_default_weight.review);
    if(m_default_weight.commerce > 0)
        score += _GetCommerceWeight(doc_id, m_default_weight.commerce, pa);
    return score;
}

bool CListRanking::_LoadConfig(const string& conf_file) {
    /// load config
    Configer cfger;
    if( !cfger.Load(conf_file)) return false;
    /// config scatters
    vector<string> scatters = cfger.GetList("scatter", "basic");
    for ( size_t i=0; i != scatters.size(); ++i) 
        if ( ! _ConfigScatter(scatters[i]) ) 
            return false;
    /// config default weight
    vector<string> factors = cfger.GetList("default", "default_weight");
    if( ! _ConfigDefaultWeight(factors)) return false;
    /// get custom weight config in K-V
    map<string, string> cat_configs;
    vector<string> weight_vec = cfger.GetKeys("custom_weight");
    if( ! weight_vec.empty()) {
        for(size_t i=0; i < weight_vec.size(); ++i) {
            cat_configs[weight_vec[i]] = cfger.Get(weight_vec[i], 
                "custom_weight");
        }
    }
    /// get bakup custom weight config in K-V
    /// for checking if the config is changed.
    Configer bak_cfger;
    /// the bak conf is with so files in a save place
    string bak_conf_file = string(SO_DIR) + "list_ranking.conf.second";
    if( FileExist(bak_conf_file.c_str()) && !bak_cfger.Load(bak_conf_file) ) 
        return false;
    map<string, string> bak_configs;
    vector<string> bak_weight_vec = bak_cfger.GetKeys("custom_weight");
    if( ! bak_weight_vec.empty()) {
        for(size_t i=0; i < bak_weight_vec.size(); ++i) {
            bak_configs[bak_weight_vec[i]] = bak_cfger.Get(bak_weight_vec[i],
                "custom_weight");
        }
    }
    if( ! _ConfigCustomWeight(cat_configs, bak_configs)) return false;
    /// save the new config to backup file
    if( ! cfger.Save(bak_conf_file)) return false;
    return true;
}

bool CListRanking::_ConfigDefaultWeight(const vector<string>& factors) {
    for(size_t i=0; i < factors.size(); ++i){
        vector<string> vec = Split(factors[i], ":");
        if(vec.size() != 2) {
            LOG(LOG_ERROR, "Illegal config [%s]", factors[i].c_str());
            return false;
        }
        if(vec[0] == "stock") 
            m_default_weight.stock = atoi(vec[1].c_str());
        if(vec[0] == "commerce")
            m_default_weight.commerce = atoi(vec[1].c_str());
        if(vec[0] == "sale") 
            m_default_weight.sale = atoi(vec[1].c_str());
        if(vec[0] == "review") 
            m_default_weight.review = atoi(vec[1].c_str());
    }
    return true;
}

bool CListRanking::_ConfigCustomWeight(
    const map<string, string>& cat_configs, 
    const map<string, string>& bak_configs ) {
    /// cost more than 5 seconds should give a warning
    TimerCounter tc("load custom weight", LOG_WARN, 5000);
    typedef map<string, string>::const_iterator MItor;
    for(MItor it = cat_configs.begin(); it != cat_configs.end(); ++it) { 
        u64 cat_id = TranseClsPath2ID(it->first.c_str(), it->first.size());
        /// cat_id may be 00, which means the root category of dangdang 
        if( 0 == cat_id && "00" != it->first) {
            LOG(LOG_ERROR, "Wrong key [%s]", it->first.c_str());
            return false;
        }
        if( m_custom_weighters.end() != m_custom_weighters.find(cat_id)) {
            LOG(LOG_DEBUG, "Duplicate category path [%s]", it->first.c_str());
            delete m_custom_weighters[cat_id];
            m_custom_weighters[cat_id] = NULL;
        }
        /// if cat config is changed, need to rebuild the weight so
        MItor bakit = bak_configs.find(it->first);
        if( bak_configs.end() == bakit || bakit->second != it->second) {
            string scripter_path = m_strModulePath + string("/");
            /// rebuild the weight so
            string build_cmd = scripter_path + string("formula_analyzer ") 
                + string(" \"") + it->first + string("\" \"") 
                + it->second + string("\" ") + scripter_path;
            int ret = system(build_cmd.c_str());
            if( 0 != ret) {
                LOG(LOG_ERROR, "Fail to build weight so [%s]",
                        build_cmd.c_str());
                return false;
            }   
            /// copy the weight so to calling place
            string copy_cmd = string("mv -f ") + scripter_path + it->first 
                + string(".* ") + string(SO_DIR);
            ret = system(copy_cmd.c_str());
            if( 0 != ret) {
                LOG(LOG_ERROR, "Fail to copy weight so [%s]", 
                        copy_cmd.c_str());
                return false;
            }
        }
        /// load so func
        string so_path = string(SO_DIR) + it->first + string(".so");
        void* dl_handle = dlopen(so_path.c_str(), RTLD_LAZY);
        if( NULL == dl_handle) {
            LOG(LOG_ERROR, "Fail to load so [%s], err [%s]", 
                so_path.c_str(), dlerror());
            return false;
        }
        m_dl_handles.push_back(dl_handle);
        WeightFunc func = (WeightFunc)dlsym(dl_handle, "compute_weight");
        char *error = dlerror();
        if( NULL != error) {
            LOG(LOG_ERROR, "Fail to load func, [%s]", error);
            return false;
        }
        /// field weighters
        string formula = it->second;
        vector<string> fields;
        if( ! ParseFormula(formula, fields)) {
            LOG(LOG_ERROR, "Fail to parse formula [%s]", 
                formula.c_str());
            return false;
        }
        vector<Weighter*> cat_weighters;
        for(size_t i=0; i < fields.size(); ++i) {
            if( "default" == fields[i]) {
                Weighter* wt_ptr = new DefaultWeighter(this);
                cat_weighters.push_back(wt_ptr);
            }else{
                /// it's field
                void* profile = NULL;
                if( ! _LoadProfile(fields[i].c_str(), profile))
                    return false;
                Weighter* wt_ptr = new FieldWeighter(m_funcFrstInt, profile);
                cat_weighters.push_back(wt_ptr);
            }
        }
        m_custom_weighters[cat_id] = new CatWeighter(cat_weighters, func);
    }
    return true;
}

bool CListRanking::_ConfigScatter(const string& scatter) {
    vector<string> cat_scatter = Split(scatter, ":");
    if ( cat_scatter.size() < 3) {
        LOG(LOG_ERROR, "scatter is wrong [%s]", scatter.c_str());
        return false;
    }
    u64 cat_id = TranseClsPath2ID(cat_scatter[0].c_str(), 
        cat_scatter[0].size());
    ScatterInfo info;
    info.field_id = GetFieldId(cat_scatter[1].c_str());
    if ( info.field_id < 0) {
        LOG(LOG_ERROR, "Wrong field is set [%s]", cat_scatter[1].c_str());
        return false;
    }
    info.unit_num = atoi(cat_scatter[2].c_str());
    if ( info.unit_num <= 0 ) {
        LOG(LOG_ERROR, "Wrong num is set [%s]", cat_scatter[2].c_str());
        return false;
    }
    if ( info.unit_num > 20 ) {
        info.unit_num = 20;
        LOG(LOG_INFO, "Too big to set, here set default [%d] instead",
            info.unit_num);
    }
    if ( cat_scatter.size() > 3) {
        vector<string> priors = Split(cat_scatter[3], "|");
        for(size_t i=0; i < priors.size(); ++i) {
            vector<string> item_pair = Split(priors[i], "-");
            if ( item_pair.size() < 2) {
                LOG(LOG_ERROR, "Abnormal format [%s]", priors[i].c_str());
                return false;
            }
            int id = atoi(item_pair[0].c_str());
            info.priority_list.push_back(id);
            int unit_base = atoi(item_pair[1].c_str());
            if ( unit_base < 0 ) {
                LOG(LOG_ERROR, "Abnormal unit [%s]", priors[i].c_str());
            }
            info.priority_list.push_back(unit_base);
        }
    }
    if ( m_scatters.end() != m_scatters.find(cat_id)) {
        LOG(LOG_ERROR, "Depulicate cat is set [%ld]", cat_id);
        return false;
    }
    m_scatters[cat_id] = info;
    LOG(LOG_INFO, "cat [%s], field [%s], unit num [%s]", 
        cat_scatter[0].c_str(), cat_scatter[1].c_str(), 
        cat_scatter[2].c_str());
    return true;
}

bool CListRanking::_LoadProfile(const char* field, void*& profile) {
    profile = FindProfileByName(field);
    if( NULL == profile) {
        LOG(LOG_ERROR, "[%s] profile does not exist", field);
        return false;
    }
    return true;
}

/*
 *function init priority doc_id
 * */
void CListRanking::_LoadPriorityDocid(){
    int init_score = 0x7fffffff - 100;
    hash_map<long long, int> h_pid_score;
    char buf[201]= {'\0'};
    string buf_s;           
    vector<string> vs_row;  
    vector<string> vs_item; 
    u64 cat_id;
    vector<u64> vpids;
    vector<int> vweight;

    FILE* fd = fopen((m_strModulePath+"sort_priority").c_str(),"r");
    if(fd){
        while(fgets(buf, 200, fd)){
            buf_s = buf;
            vs_row = Split(TrimBoth(buf_s), "\t");
            //todo cat_id faild 
            cat_id = TranseClsPath2ID(vs_row[0].c_str(), vs_row[0].size());
            for(size_t i = 1; i < vs_row.size(); i++){
                vs_item = Split(TrimBoth(vs_row[i]), ":"); 
                vpids.push_back(atoi(vs_item[0].c_str()));

                if(atoi(vs_item[1].c_str()) < 100){//防止大于100的weight异常导致score超出int范围
                    vweight.push_back(atoi(vs_item[1].c_str())); 
                }else{
                    vweight.push_back(100);
                }
            }

            for(size_t i = 0; i < vpids.size(); i++){
                h_pid_score.insert(make_pair(vpids[i], init_score + vweight[i]));
            }

            m_priority_cat_pid.insert(make_pair(cat_id, h_pid_score));
            vpids.clear();
            vweight.clear();
            h_pid_score.clear();
        }
        fclose(fd);
    }
}


u64 CListRanking::_GetWeightCat(u64 cat_id) {
    int cat_lvl = GetClsLevel(cat_id);
    u64 weight_cat = 0x0;
    for(int i=cat_lvl;i > 0;--i) {
        weight_cat = GetClassByLevel(i, cat_id);
        if( m_custom_weighters.end() != m_custom_weighters.find(weight_cat))
            return weight_cat;
    }
    return 0;
}

int CListRanking::_GetCommerceWeight(int doc_id, int coff, IAnalysisData* pa){
    ListAnalysisData* pla = (ListAnalysisData*)pa;
    int cur_date = pla->cur_date;
    int commerce_scr = 0;
    int sale_week = 0;
    if(NULL != m_sale_week_profile)
        sale_week = m_funcFrstInt(m_sale_week_profile, doc_id);
    int stock = 0;
    if(NULL != m_stock_profile)
        stock = m_funcFrstInt(m_stock_profile, doc_id);
    int first_input_date = 0;
    if(NULL != m_first_input_date_profile)
        first_input_date = m_funcFrstInt(m_first_input_date_profile, doc_id);
    int publish_date = 0;
    if(NULL != m_publish_date_profile)
        publish_date = m_funcFrstInt(m_publish_date_profile, doc_id);
    bool new_product = false;
    if( (cur_date-first_input_date)/86400 < 30 && stock > 0) {
        /// @see Here check if new is only about pub product, no b2c!
        if( 0 == publish_date || (cur_date>=publish_date 
                    && (cur_date-publish_date)/86400 < 240))
            new_product = true;
    }
    if( sale_week > 0) {
        if(sale_week < 300) {
            if(sale_week < 10) commerce_scr = 4;
            else if(sale_week < 80) commerce_scr = sale_week < 35 ? 5 : 6;
            else commerce_scr = sale_week < 160 ? 7 : 8;
        }
        else commerce_scr = 9;
        if(new_product) commerce_scr += 3;
    }else{
        int review_count = 0;
        if(NULL != m_review_count_profile)
            review_count = m_funcFrstInt(m_review_count_profile, doc_id);
        if( new_product) commerce_scr = 5;
        else if( review_count >= 100) commerce_scr = 3;
        else commerce_scr = review_count > 35 ? 2 : 1;
    }
    /// normalize to be [0, 100]
    return commerce_scr*coff;
}


void CListRanking::SetGroupByAndFilterSequence(IAnalysisData* pad, 
    vector<SFGNode>& vec) {
    MOD_MAP::iterator i = m_mapMods->find("query_pack");
    if ( i != m_mapMods->end())
        i->second.pMod->SetGroupByAndFilterSequence(pad,vec);
}

void CListRanking::CustomFilt(IAnalysisData* pad, vector<int>& vDocIds, 
    SFGNode& fgNode) {
    MOD_MAP::iterator i = m_mapMods->find("query_pack");
    if ( i != m_mapMods->end())
        i->second.pMod->CustomFilt(pad,vDocIds, fgNode);
}

void CListRanking::CustomGroupBy(IAnalysisData* pad, vector<int>& vDocIds, 
    SFGNode& gfNode, pair<int, vector<SGroupByInfo> >& prGpInfo) {
    MOD_MAP::iterator i = m_mapMods->find("query_pack");
    if ( i != m_mapMods->end())
        i->second.pMod->CustomGroupBy(pad,vDocIds, gfNode,prGpInfo);
}

void  CListRanking::SortForCustom(vector<SResult>& vRes, int from, int to, 
    IAnalysisData* pad) {
    MOD_MAP::iterator i = m_mapMods->find("query_pack");
    if ( i != m_mapMods->end())
        i->second.pMod->SortForCustom(vRes,from, to,pad);
}

void CListRanking::BeforeGroupBy(IAnalysisData* pad, vector<SResult>& vRes, 
    vector<PSS>& vGpName, GROUP_RESULT& vGpRes) {
	MOD_MAP::iterator i = m_mapMods->find("query_pack");
	if ( i != m_mapMods->end())
		i->second.pMod->BeforeGroupBy(pad,vRes,vGpName,vGpRes);
}   

#ifndef _WIN32
// linux dll
CListRanking list_ranking;
#endif

