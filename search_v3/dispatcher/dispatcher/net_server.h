#ifndef NET_SERVER_H
#define NET_SERVER_H
/// head files include
#include "swstd.h"
#include "http_handler.h"

/// namespace to limit the scope
class HttpServer;
class Dispatcher;

class NetServer : public HttpHandler {
public:
    NetServer();
    ~NetServer();

    virtual bool Work(const std::vector<char>& request, 
        std::vector<char>& response);
    bool Init(const std::string& config, Dispatcher* dispatcher_ptr);
    void CoInit();
    bool StartServer(const std::string& config);
    bool StopServer();
    void WaitForQuit();

private:
    bool _DataWork(const std::string& request, std::vector<char>& response);
    bool _ControlWork(const std::string& request, 
        std::vector<char>& response);
    bool _Help(const std::string& request, std::vector<char>& response);
    bool _PackHeader(int inc_num, int data_sz, int status, 
        std::vector<char>& resp);
    bool _GetFullData (const std::string& module, const std::string& key,
                       std::vector<char>& response);

private:
    static void* _Run(void* arg);

private:
    HttpServer* m_server_ptr;
    pthread_t m_server_thread;
    Dispatcher* m_dispatcher_ptr;
    int m_port;
    int m_thread_num;
    /// Disallow copy and assign defaultly
    DISALLOW_COPY_AND_ASSIGN(NetServer);
};

#endif // ~>.!.<~ 
