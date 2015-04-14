#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H
/// head files include
#include "swstd.h"

/// namespace to limit the scope
class HttpHandler;

class CServerFrame;

class HttpServer {
    public:
        explicit HttpServer();
        ~HttpServer();

        bool Create(unsigned short port, unsigned short thread_num, 
                    HttpHandler* p_handler);
        bool Run();
        bool Close();
    private:
        CServerFrame* m_server_ptr;
        HttpHandler*   m_handler_ptr;
        // config
        unsigned short m_listen_port;
        unsigned short m_thread_num;

        /// Disallow copy and assign defaultly
        DISALLOW_COPY_AND_ASSIGN(HttpServer);
};

#endif // ~>.!.<~ 
