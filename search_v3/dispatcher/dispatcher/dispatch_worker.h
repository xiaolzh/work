#ifndef DISPATCH_WORKER_H
#define DISPATCH_WORKER_H
/// head files include
#include "swstd.h"
#include "Daemon.h"

class Dispatcher;

/// namespace to limit the scope
class DispatchWorker : public CWorker {
    public:
        DispatchWorker(Dispatcher* dp);
        virtual ~DispatchWorker();

        virtual bool Init(HIS&his);
        virtual bool Run();
        virtual bool Dispose();
        virtual bool close();
    private:
        Dispatcher* m_worker_ptr;
        /// Disallow copy and assign defaultly
        DISALLOW_COPY_AND_ASSIGN(DispatchWorker);
};

#endif // ~>.!.<~ 
