#include "configer.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include "logger.h"
using namespace std;

Configer::Configer() {}

Configer::~Configer() {}

bool Configer::Load(const string& config_file) {
    m_file_name = config_file;
    ifstream fin(m_file_name.c_str());
    if( !fin) {
        LOG(LOG_ERROR, "Can't open file [%s] correctly", config_file.c_str());
        return false;
    }
    return _ReadAsIni(fin);
}

bool Configer::Save(const string& config_file) const{
    string save_file = config_file;
    if("" == config_file) {
        save_file = m_file_name;
    }
    ofstream fout(save_file.c_str());
    if( !fout) {
        LOG(LOG_ERROR, "Can't open file [%s] correctly", save_file.c_str());
        return false;
    }
    return _WriteAsIni(fout);
}

string Configer::Get(const string& key, const string& section/* = ""*/) const{
    CKVItor kvit = m_data.find(section);
    if(m_data.end() != kvit) {
        CMapItor sec_it = kvit->second.find(key);
        if(kvit->second.end() != sec_it) {
            if( !sec_it->second.empty()) {
                return sec_it->second[0];
            }
        }
    }
    return "";
}

bool Configer::Set(const string& key, const string& val, 
    const string& section/* = ""*/) {
    vector<string> val_list;
    val_list.push_back(val);
    m_data[section][key] = val_list;
    return true;
}

vector<string> Configer::GetList(const string& key, 
    const string& section/* = ""*/)  const{
    CKVItor kvit = m_data.find(section);
    if(m_data.end() != kvit) {
        CMapItor sec_it = kvit->second.find(key);
        if(kvit->second.end() != sec_it) {
            return sec_it->second;
        }
    }
    return vector<string>();
}

bool Configer::SetList(const string& key, const vector<string>& val_list,
    const string& section/* = ""*/) {
    m_data[section][key] = val_list;
    return true;
}

vector<string> Configer::GetSections() const {
    vector<string> sec_vec;
    for(CKVItor itor = m_data.begin(); itor != m_data.end(); ++itor) {
        sec_vec.push_back(itor->first);
    }
    return sec_vec;
}

vector<string> Configer::GetKeys(const string& section/* = ""*/) const{
    vector<string> key_vec;
    CKVItor sec_it = m_data.find(section);
    if(m_data.end() == sec_it) return key_vec;
    for(CMapItor itor = sec_it->second.begin();itor != sec_it->second.end(); 
        ++itor) {
        key_vec.push_back(itor->first);
    }
    return key_vec;
}

bool Configer::FromStr(const string& config_str) {
    istringstream str_in(config_str);
    return _ReadAsIni(str_in);
}

string Configer::ToStr() const{
    ostringstream str_out;
    if( ! _WriteAsIni(str_out) ) {
        LOG(LOG_ERROR, "Fail to to string");
        return "";
    }
    return str_out.str();
}

void Configer::Example(ostream& out) {
    static const char* EXAMPLE = 
    "#----- this is an example of ini config ------\n"
    "# # is annotation character\n"
    "# [section] is section field\n"
    "# key=value is kv pair to config, which should be unique in a section\n"
    "# key=value1, value2, ..., valueN, is key - value_vector pair\n\n"
    "global_key = value # this value is default under section \"\"\n\n"
    "[ section_name ] \n"
    "key = value\n"
    "key = value2 # this kv pair will cover the upper kv pair\n\n"
    "[ section2 ]\n"
    "key = 1,2,3,4 # this kv pair is OK cause it's unique in this section\n"
    "key2 = value of section2 \n"
    "# following will be ignored \n"
    "key value\n"
    "to be continue\n";
    out<<EXAMPLE<<endl;
}

bool Configer::_ReadAsIni(istream& in) {
    static const char ANNOTATION = '#';
    string str_in;
    string section = "";
    while( getline(in, str_in, '\n')) {
        /// trim space and annotation
        boost::trim(str_in);
        size_t pos = str_in.find_first_of(ANNOTATION);
        if(string::npos != pos) {
            str_in = str_in.substr(0, pos);
        }
        if(str_in.empty()) continue;
        /// check if it is a section
        if('[' == str_in[0]) {
            int pos = str_in.find_first_of(']');
            if(string::npos != (size_t)pos) {
                // ignore [ ] 
                section = boost::trim_copy(str_in.substr(1, pos-1));
                continue;
            }
        }
        /// parse as k-v pair
        string key;
        vector<string> val_list;
        pos = str_in.find_first_of('=');        
        if(string::npos != pos) {
            key = boost::trim_copy(str_in.substr(0, pos));           
            string val_str = boost::trim_copy(str_in.substr(pos+1)); 
            boost::split(val_list, val_str, boost::is_any_of(","), 
                boost::token_compress_on );
            vector<string> trim_val_list;
            for(size_t i=0;i < val_list.size();++i){
                boost::trim(val_list[i]);
            }
            if( val_list.empty() || ( val_list.size() == 1 
                && val_list[0].empty())){
                /// 'key = ' is considered as empty
            } else {
                m_data[section][key] = val_list;
            }
        }
    }
    return true;
}

bool Configer::_WriteAsIni(ostream& out) const{
    for(CKVItor kvit = m_data.begin(); kvit != m_data.end(); ++kvit) {
        if( !kvit->first.empty())
            out <<"[ "<< kvit->first << " ]"<<endl;
        for(CMapItor mit = kvit->second.begin(); mit != kvit->second.end();
            ++mit) {
            out<<mit->first<<" = ";
            int i=0;
            for(i=0;i < (int)(mit->second.size()-1);++i) {
                out<<mit->second[i]<<", ";
            }
            out<<mit->second[i]<<endl;
        }
        out<<endl;
    }
    return true;
}

#if UNIT_TEST
#include <iostream>
#include <fstream>
using namespace std;

bool test_Configer() {
    cout<<"Unit test: Configer"<<endl;
    {
        cout<<"Testcase: Configer load and get data"<<endl;
        bool ret = true;
        ofstream fout("./test.conf");
        Configer::Example(fout);
        Configer conf;
        string conf_file = "./test.conf";
        ret &= conf.Load(conf_file);
        cout<<conf.Get("global_key")<<endl;
        cout<<conf.Get("key", "section_name")<<endl;
        cout<<conf.Get("key", "section2")<<endl;
        vector<string> vec = conf.GetList("key", "section2");
        copy(vec.begin(), vec.end(), 
            ostream_iterator<string>(cout, "\n"));  
        cout<<conf.Get("key2", "section2")<<endl;
        cout<<"configer string: \n"<<conf.ToStr()<<endl;
        cout<<"Pass?"<< boolalpha<<ret<<endl;
    }
    {
        cout<<"Testcase: Configer set and save data"<<endl;
        bool ret = true;
        Configer conf;
        string conf_file = "./test.conf";
        ret &= conf.Set("key1", "value 1");
        ret &= conf.Set("key2", "value 2");
        ret &= conf.Set("key1", "value 1", "");
        ret &= conf.Set("key1", "value 1", "section");
        vector<string> vec;
        vec.push_back("vec val 1");
        vec.push_back("vec val 2");
        ret &= conf.SetList("key3", vec);
        ret &= conf.Save(conf_file);
        cout<<"Pass?"<< boolalpha<<ret<<endl;
    }
    {
        cout<<"Testcase: get section and get key"<<endl;
        Configer conf;
        bool ret = true;
        vector<string> vec = conf.GetSections();
        ret &= vec.empty();
        ret &= conf.GetKeys().empty();
        conf.Set("key1", "");
        conf.Set("key2", "value", "section");
        ret &= (conf.GetSections().size() == 2);
        ret &= (conf.GetKeys().size() == 1);
        ret &= (conf.GetKeys("section").size() == 1);
        cout<<"Pass?"<< boolalpha<<ret<<endl;
    }
    return true;
}

#if TEST_CONFIGER
int main() {
    if( !test_Configer()) {
        cerr<<"Fail to pass test"<<endl;
        return -1;
    }
    return 0;
}
#endif // TEST_CONFIGER

#endif // UNIT_TEST
// I wanna sing a song for you, dead loop no escape
