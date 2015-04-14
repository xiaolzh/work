#include "http_handler.h"
#include <assert.h>
#include <algorithm> 
#include <boost/lexical_cast.hpp>
#include "logger.h"
#include "global_define.h"

/// HttpHandler construction
HttpHandler::HttpHandler() {
    // TODO: initialize
}

/// HttpHandler destruction
HttpHandler::~HttpHandler() {
    // TODO: co-initialize
}

void HttpHandler::operator()(vector<char>& vec_in, vector<char>& vec_out,
    int cmd,int header_len) {
    if(GET == cmd) {
        vector<char> request;
        if( ! _ParseRequest(vec_in, header_len, request)) {
            LOG(LOG_ERROR, "Fail to parse http request, [%s], size=[%d]",
                &vec_in[0], vec_in.size());
            return;
        }
        vector<char> response;
        if( ! Work(request, response)) {
            LOG(LOG_ERROR, "Fail to response to request, [%s, size=[%d]]",
                &request[0], request.size());
            return;
        }
        /// @todo here will always return 200 OK no matter work right or 
        ///       wrong, which needs to be improved
        if( ! _PackResponse(response, cmd, vec_out)) {
            LOG(LOG_ERROR, "Fail to pack response, [%s], size=[%d]",
                &response[0], response.size());
            return;
        }
    }else{
        /// @todo parse and response more cmd [PUT, POST, ...]
        LOG(LOG_INFO, "Unknown http request type: [%d]", cmd);
    }
}

bool HttpHandler::_ParseRequest(const vector<char>& vec_in, int header_len,
    vector<char>& request) {
    /// @todo need a request parser
    typedef vector<char>::const_iterator CItor;
    CItor start_itor = std::find(vec_in.begin(), vec_in.end(), '/');
    CItor end_itor = std::find(start_itor, vec_in.end(), ' ');
    std::copy(start_itor, end_itor, back_inserter(request));
    request.push_back('\0');
    return true;
}

bool HttpHandler::_PackResponse(const vector<char>& response, int cmd,
    vector<char>& vec_out) {
    if( GET == cmd) {
        /// @todo change to adapt more return type
        string resp_header = "HTTP/1.0 200 OK\r\n"
                             "Connection: close\r\n"
                             "Server: FrameServer/1.0.0\r\n"
                             "Content-Type: text/plain\r\n"
                             "Content-Length: ";
        resp_header += boost::lexical_cast<string>(response.size());
        resp_header += "\r\n\r\n";
        vec_out.resize( resp_header.size() + response.size());
        memcpy(&vec_out[0], resp_header.data(), resp_header.size());
        memcpy(&vec_out[resp_header.size()], &response[0], response.size());
    }else{
        LOG(LOG_ERROR, "Unknown http type, [%d]", cmd);
        return false;
    }
    return true;
}

#ifdef UNIT_TEST
/// include head files for unit test
#include "iostream"
using namespace std;

bool test_HttpHandler() {
    cout<<"Unit test - HttpHandler"<<endl;
    {
        bool ret = true;
        cout<<"usecase 1: "<<endl;
        // TODO: add your test code here
        cout<<boolalpha<<ret<<endl;
        if( !ret)
            return false;
    }
    cout<<"Done unit test - HttpHandler"<<endl;
    return true;
}

#ifdef TEST_HTTP_HANDLER
int main() {
    if(!test_HttpHandler())
        return -1;
    return 0;
}
#endif // TEST_HTTP_HANDLER
#endif // UNIT_TEST

// I wanna sing a song for you, dead loop no escape
