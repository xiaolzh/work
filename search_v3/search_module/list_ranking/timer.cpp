#include "timer.h"

#if UNIT_TEST
#include <iostream>
using namespace std;

void test_Timer()
{
    cout<<"Unit test: timer"<<endl;
    {
        TimerCounter test("Hello timer!");
        sleep(2);
    }

    {
        // linux
        PreciseTimer test;
        test.Start();
        for(int i=0; i != 100000; i++)
        {
            // do nothing
            if( i%10000 == 0)
            {
                test.End();
                cout<<test.Cost()<<endl;
                test.Start();
            }
        }
    }
}

#if TEST_TIMER
int main()
{
    test_Timer();
    return 0;
}
#endif // TEST_TIMER

#endif //UNIT_TEST

// I wanna sing a song for you, dead loop no escape
