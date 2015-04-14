#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H
/// head files include
#include "swstd.h"
#include "ServerFrameEx.h"

/// @todo namespace to limit the scope

/// A http handler wrapper
class HttpHandler : public SRequestHandler {
public:
    HttpHandler();
    virtual ~HttpHandler();

    virtual void operator()(std::vector<char>& vec_in, 
        std::vector<char>& vec_out, int cmd,int header_len);
        
    virtual bool Work(const std::vector<char>& request, 
        std::vector<char>& response) = 0;
protected:
    bool _ParseRequest(const std::vector<char>& vec_in, int header_len,
        std::vector<char>& request);
    bool _PackResponse(const std::vector<char>& response, int cmd,
        vector<char>& vec_out);
private:
    //static const char* ms_resp_header_template;

    /// Disallow copy and assign defaultly
    DISALLOW_COPY_AND_ASSIGN(HttpHandler);
};

#endif // ~>.!.<~ 
