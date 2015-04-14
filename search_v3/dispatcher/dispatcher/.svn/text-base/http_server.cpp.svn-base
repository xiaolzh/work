#include "http_server.h"
#include "http_handler.h"
#include "ServerFrameEx.h"
#include "logger.h"
#include "global_define.h"

/// HttpServer construction
HttpServer::HttpServer() 
    : m_server_ptr(NULL)
    , m_handler_ptr(NULL) {
}

/// HttpServer destruction
HttpServer::~HttpServer() {
    if(NULL != m_server_ptr) {
        LOG(LOG_DEBUG, "server is not NULL");
        delete m_server_ptr;
        m_server_ptr = NULL;
    }
    m_handler_ptr = NULL;
}

bool HttpServer::Create(unsigned short port, unsigned short thread_num,
                        HttpHandler* p_handler){
    if(NULL == m_server_ptr) {
        m_server_ptr = new CServerFrame();
    }
    return m_server_ptr->CreateServer(port, thread_num, p_handler);
}

bool HttpServer::Run() {
    if(NULL != m_server_ptr) {
        return m_server_ptr->RunServer();
    }else{
        LOG(LOG_DEBUG, "server is no created yet");
        return false;
    }
}

bool HttpServer::Close() {
    if(NULL != m_server_ptr) {
        return m_server_ptr->CloseServer();
    }else{
        LOG(LOG_DEBUG, "server is no created yet");
        return false;
    }
}

#ifdef UNIT_TEST
/// include head files for unit test
#include "iostream"
using namespace std;


bool test_HttpServer() {
    cout<<"Unit test - HttpServer"<<endl;
    {
        bool ret = true;
        cout<<"usecase 1: "<<endl;
        // TODO: add your test code here
        cout<<boolalpha<<ret<<endl;
        if( !ret)
            return false;
    }
    cout<<"Done unit test - HttpServer"<<endl;
    return true;
}

#ifdef TEST_HTTP_SERVER
int main() {
    if(!test_HttpServer())
        return -1;
    return 0;
}
#endif // TEST_HTTP_SERVER
#endif // UNIT_TEST

// I wanna sing a song for you, dead loop no escape
