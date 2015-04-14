#include "weighter.h"
#include "ListRanking.h"

int DefaultWeighter::Work(int doc_id, IAnalysisData* pa) {
    return m_this_ptr->ComputeDefaultWeight(doc_id, pa);
}

int CatWeighter::Work(int doc_id, IAnalysisData* pa) {
    std::vector<int> field_vals;
    field_vals.resize(m_weighters.size());
    for(size_t i=0; i < m_weighters.size(); ++i) {
        field_vals[i] = m_weighters[i]->Work(doc_id, pa);
    }    
    /// @see Currently we DO NOT need pass in the IAnalysis to func 
    return m_func(&field_vals[0], field_vals.size());
}

// I wanna sing a song for you, dead loop no escape
