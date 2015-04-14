#include "mailer.h"
#include <errno.h>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include "logger.h"
using namespace std;
using boost::lexical_cast;

Mailer::Mailer(const string& receiver/* = ""*/) {
    /// @todo initialize
    m_receivers.insert(receiver);
}

Mailer::~Mailer() {
    /// @todo co-initialize
}

bool Mailer::AddReceiver(const string& receiver) {
    m_receivers.insert(receiver);
    return true;
}

void Mailer::RemoveReceiver(const string& receiver) {
    m_receivers.erase(receiver);
}

bool Mailer::Send(const string& title, const string& content)  const{
    if( m_receivers.empty()) return true; /// no need to send
    string cmd = string("mail -sv ") + title; 
    for(set<string>::const_iterator itor = m_receivers.begin();
        itor != m_receivers.end(); ++itor) {
        cmd += string(" ") + *itor;
    }
    string tmp_file = string("/dev/shm/mail_") + 
        lexical_cast<string>(getpid()) + string("_") + 
        lexical_cast<string>(pthread_self());
    ofstream fout(tmp_file.c_str());
    if(!fout) {
        LOG(LOG_ERROR, "Fail to save to temp file [%s]", tmp_file.c_str());
        return false;
    }
    fout<<content;
    fout.flush();
    fout.close();
    cmd += string(" < ") + tmp_file;
    int ret = system(cmd.c_str());
    if( 0 != ret) {
        LOG(LOG_ERROR, "Fail to cmd [%s], error [%s]", cmd.c_str(), 
            strerror(errno));
        return false;
    }
    cmd = string("rm -f ") + tmp_file;
    ret = system(cmd.c_str());
    if( 0 != ret) {
        LOG(LOG_ERROR, "Fail to remove file [%s]", cmd.c_str());
        return false;
    }
    return true;
}

#ifdef UNIT_TEST
#include <iostream>
using namespace std;
#include "mailer.h"

bool test_mailer() {
    cout<<"Unit test - mailer"<<endl;
    {
        bool ret = true;
        cout<<"usecase 1: "<<endl;
        Mailer mler("liliwu@dangdang.com");
        ret &= mler.Send("Hello Mailer", "It's a unit test");
        ret &= mler.AddReceiver("llwgod@163.com");
        ret &= mler.Send("Hello mailer 2", "It's a unit test 2");
        mler.RemoveReceiver("liliwu@dangdang.com");
        mler.RemoveReceiver("wrong@address");
        ret &= mler.Send("Hello mailer 3", "It's a unit test 3");
        cout<<boolalpha<<ret<<endl;
        if( !ret)
            return false;
    }
    cout<<"Done unit test - mailer"<<endl;
    return true;
}

#ifdef TEST_MAILER
int main() {
    if(!test_mailer())
        return -1;
    return 0;
}
#endif // TEST_MAILER
#endif // UNIT_TEST

// I wanna sing a song for you, dead loop no escape
