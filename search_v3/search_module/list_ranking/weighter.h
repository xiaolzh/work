#ifndef WEIGHTER_H
#define WEIGHTER_H
#include <string>
#include <vector>
#include "Module.h"

typedef int (*WeightFunc)(int*, int);

class Weighter {
public:
    virtual ~Weighter() {};
    virtual int Work(int doc_id, IAnalysisData* pa) = 0;
};

class CListRanking;

class DefaultWeighter : public Weighter {
public:
    DefaultWeighter(CListRanking* this_ptr = NULL) : m_this_ptr(this_ptr) {}
    ~DefaultWeighter() { m_this_ptr = NULL; }

    virtual int Work(int doc_id, IAnalysisData* pa);
private:
    CListRanking* m_this_ptr;
};

class CatWeighter : public Weighter {
public:
    CatWeighter(const std::vector<Weighter*> weighters, WeightFunc func)
        : m_weighters(weighters), m_func(func) { }
    ~CatWeighter() { 
        for(size_t i=0; i < m_weighters.size(); ++i) {
            delete m_weighters[i];
            m_weighters[i] = NULL;
        }
        m_weighters.clear();
    }

    virtual int Work(int doc_id, IAnalysisData* pa);

private:
    std::vector<Weighter*> m_weighters;
    WeightFunc m_func;
};

class FieldWeighter : public Weighter {
public:
    FieldWeighter(FPTR_GET_FRST_INT_VAL field_func, void* field_profile)
        : m_field_func(field_func), m_field_profile(field_profile) {
    }
    ~FieldWeighter() { 
        m_field_profile = NULL;
    }

    virtual int Work(int doc_id, IAnalysisData* pa) {
        return m_field_func(m_field_profile, doc_id);
    }
private:
    FPTR_GET_FRST_INT_VAL m_field_func;
    void* m_field_profile;
};

#endif  // ~>.!.<~
