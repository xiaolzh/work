#include "text_db.h"
#include <assert.h>
#include "logger.h"
using namespace std;

bool TextDB::Init(const string& db_path) {
    string index_file = db_path + string("/text_db.idx");
    string text_file = db_path + string("/text_db.ivt");
    const int text_page_size = 4096;
    if( ! m_text_store.LoadDetail(index_file.c_str(), text_file.c_str(),
        text_page_size)) {
        LOG(LOG_ERROR, "Fail to initialize the text db");
        return false;
    }
    string state_file = db_path + string("/text_db.state");
    int sz = 0;
    if( 0 != access(state_file.c_str(), F_OK)) {
        m_state_file.OpenMapPtr(state_file.c_str(), false, 0, sz, true);
        m_cursor_ptr = (int*)m_state_file.GetPtr();
        /// @see cursor start from 1, not 0
        *m_cursor_ptr = 1;
        m_state_file.Sync();
    }else {
        m_state_file.OpenMapPtr(state_file.c_str(), false, 0, sz, true);
        m_cursor_ptr = (int*)m_state_file.GetPtr();
    }
    LOG(LOG_INFO, "The text db's size is [%d]", *m_cursor_ptr);
    /// @todo this method is smelly, we should design a text db to avoid to
    ///       be too huge
    if( *m_cursor_ptr > 100000000) {
        LOG(LOG_ERROR, "Text db is abnormal huge [%d]", *m_cursor_ptr);
        return false;
    }
    return true;
}

bool TextDB::CoInit() {
    if( ! m_state_file.Sync()) return false;
    m_text_store.Dispose();
    return true;
}

int TextDB::WriteData(const vector<char>& text) {
    int ret_index = *m_cursor_ptr;
    if( ! m_text_store.WriteDetail(&text[0], text.size(), ret_index)) 
        return -1;
    (*m_cursor_ptr) += 1;
    m_state_file.Sync();
    return ret_index;
}

bool TextDB::ReadData(int index, vector<char>& text) {
    if( index >= *m_cursor_ptr) {
        LOG(LOG_ERROR, "index [%d] is bigger than cursor [%d]",
            index, *m_cursor_ptr);
        return false;
    }
    int len = m_text_store.GetDocLen(index);
    if( len <=0 ) {
        LOG(LOG_ERROR, "Wrong data len [%d]", len);
        return false;
    }
    SFetchInfo fi;
    const char* data_ptr = m_text_store.GetData(index, fi);
    if( NULL == data_ptr) {
        LOG(LOG_ERROR, "Can't get data: %d", index);
        return false;
    }
    copy(data_ptr, data_ptr+len, back_inserter(text));
    if( ! m_text_store.PutData(fi)) {
        LOG(LOG_WARN, "Fail to put data back");
    }
    return true;
}

void TextDB::Resize(int cursor) {
    assert(cursor > 0);
    *m_cursor_ptr = cursor;
    m_state_file.Sync();
}

#ifdef UNIT_TEST
/// include head files for unit test
#include <iostream>
#include <algorithm>
#include "utilities.h"
using namespace std;

bool test_TextDB() {
    cout<<"Unit test - TextDB"<<endl;
    //if(0)
    {
        bool ret = true;
        cout<<"usecase : write data and read data"<<endl;
        /// TODO
        string str1 = "hello";
        string str2 = "world";
        vector<char> vec;
        AddStrToVec(str1, vec);
        AddStrToVec(str2, vec);
        TextDB text_db;
        ret &= text_db.Init("./");
        for(size_t i=0; i<1000000;++i) {
            ret &= ( i == text_db.WriteData(vec));
        }
        vector<char> vec2;
        ret &= text_db.ReadData(0, vec2);
        ret &= ( vec2 == vec );
        cout<<boolalpha<<ret<<endl;
        if( !ret)
            return false;
    }
    cout<<"Done unit test - TextDB"<<endl;
    return true;
}

#ifdef TEST_TEXT_DB
int main() {
    if(!test_TextDB())
        return -1;
    return 0;
}
#endif // TEST_TEXT_DB
#endif // UNIT_TEST

// I wanna sing a song for you, dead loop no escape
