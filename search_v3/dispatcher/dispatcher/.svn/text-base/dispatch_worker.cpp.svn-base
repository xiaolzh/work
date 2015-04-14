#include "dispatch_worker.h"
#include <assert.h>
#include "dispatcher.h"
#include "logger.h" 
#include "global_define.h"

/// DispatchWorker construction
DispatchWorker::DispatchWorker(Dispatcher* dp)
    : m_worker_ptr(dp) {
    assert(NULL != m_worker_ptr);
}

/// DispatchWorker destruction
DispatchWorker::~DispatchWorker() {
    // TODO: co-initialize
}

bool DispatchWorker::Init(HIS&his) {
    /// @todo initialize from console
    return m_worker_ptr->Init("no input yet");
}

bool DispatchWorker::Run() {
    return m_worker_ptr->Run();
}

bool DispatchWorker::Dispose() {
    //if( !m_worker_ptr->StopUpdateAll()) return false;
    //if( !m_worker_ptr->StopNetServer()) return false;
    //if( !m_worker_ptr->CoInit()) return false;
    return true;
}

bool DispatchWorker::close() {
    if( !m_worker_ptr->StopUpdateAll()) return false;
    if( !m_worker_ptr->StopNetServer()) return false;
    return true;
}

#ifdef UNIT_TEST
/// include head files for unit test
#include "iostream"
using namespace std;


bool test_DispatchWorker() {
    cout<<"Unit test - DispatchWorker"<<endl;
    {
        bool ret = true;
        cout<<"usecase 1: "<<endl;
        // TODO: add your test code here
        cout<<boolalpha<<ret<<endl;
        if( !ret)
            return false;
    }
    cout<<"Done unit test - DispatchWorker"<<endl;
    return true;
}

#ifdef TEST_DISPATCH_WORKER
int main() {
    if(!test_DispatchWorker())
        return -1;
    return 0;
}
#endif // TEST_DISPATCH_WORKER
#endif // UNIT_TEST

// I wanna sing a song for you, dead loop no escape
