#ifndef DISPATCHER_MAILER_H
#define DISPATCHER_MAILER_H
#include "swstd.h"
#include <string>
#include <set>

/// class define
class Mailer {
public:
    Mailer(const std::string& receiver = "");
    ~Mailer();

    bool AddReceiver(const std::string& receiver);
    void RemoveReceiver(const std::string& receiver);
    bool Send(const std::string& title, const std::string& content) const;

private:
    std::set<std::string> m_receivers;
    /// Disallow copy and assign defaultly
    DISALLOW_COPY_AND_ASSIGN(Mailer);
};

#endif // ~>.!.<~ 
