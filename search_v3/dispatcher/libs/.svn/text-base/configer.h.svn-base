#ifndef DISPATCHER_CONFIGER_HXX
#define DISPATCHER_CONFIGER_HXX
#include "swstd.h"
#include <string>
#include <map>
#include <vector>
#include <iostream>

/**
 *  @todo modified to be more flexible using general data and data packer
 */
class Configer {
public:
    typedef std::vector<std::string> VecData;
    typedef std::map<std::string, VecData> MapData;
    typedef std::map<std::string, MapData> KVData;
    typedef MapData::iterator MapItor;
    typedef MapData::const_iterator CMapItor;
    typedef KVData::iterator KVItor;
    typedef KVData::const_iterator CKVItor;
public:
    Configer();
    ~Configer();

    bool Load(const std::string& config_file);
    bool Save(const std::string& config_file = "") const;

    std::string Get(const std::string& key, const std::string& section = "")
        const;    
    bool Set(const std::string& key, const std::string& val,
        const std::string& section = "");

    std::vector<std::string> GetList(const std::string& key, 
        const std::string& section = "") const;
    bool SetList(const std::string& key, 
        const std::vector<std::string>& val_list, 
        const std::string& section = "");
    
    std::vector<std::string> GetSections() const;
    std::vector<std::string> GetKeys(const std::string& section = "") const;

    bool FromStr(const std::string& config_str);
    std::string ToStr() const;

    static void Example(std::ostream& out);

private:
    bool _ReadAsIni(std::istream& in);
    bool _WriteAsIni(std::ostream& out) const;

private:
    std::string m_file_name;
    KVData m_data;
};

#endif // ~>.!.<~
