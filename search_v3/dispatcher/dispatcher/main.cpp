#include <stdio.h>
#include <string>
#include "dispatcher.h"
#include "dispatch_worker.h"
#include "configer.h"
#include "logger.h"
#include "global_define.h"
using namespace std;


#ifndef TEST_DISPATCHER

int main(int argc,char* argv[]) {
    if(argc<3)
    {
        fprintf(stderr, "usage ./dispatcher -k START|STOP\n");
        return -1;
    }
    freopen("../log/info.log", "a", stdout);
    freopen("../log/err.log", "a", stderr);
    Dispatcher dp;
    DispatchWorker worker(&dp);
    CDaemon daemon(&worker);
    return daemon.Run(argc,argv);  
    /*
    if( !InitLogger(CONFIG_FILE)) return -1;
    {
        bool ret = true;
        cout<<"usecase 1: start and stop a dispatcher"<<endl;
        Dispatcher dp;
        ret &= dp.Init("dispatcher.conf");
        ret &= dp.UpdateAll();
        ret &= dp.StartNetServer();
        while(true) {
            sleep(30);
        }
        ret &= dp.StopNetServer();
        ret &= dp.StopUpdateAll();
        ret &= dp.CoInit();
        cout<<boolalpha<<ret<<endl;
        if( !ret)
            return -1;
    }
    */
}

#endif // TEST_DISPATCHER

// I wanna sing a song for you, dead loop no escape
