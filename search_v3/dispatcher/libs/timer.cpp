#include "timer.h"
#if UNIT_TEST
#include <iostream>
using namespace std;
#endif // UNIT_TEST

#if UNIT_TEST
void test_timer()
{
	cout<<"Unit test: timer"<<endl;
	{
		timer_counter test("Hello timer!");
		sleep(2);
	}

	{
		// linux
		precise_timer test;
		int cost = 0;
		test.start();
		for(int i=0; i != 100000; i++)
		{
			// do nothing
			if( i%10000 == 0)
			{
				test.end();
				cout<<test.cost()<<endl;
				test.start();
			}
		}
	}
}

#if TEST_TIMER
int main()
{
	test_timer();
	return 0;
}
#endif // TEST_TIMER

#endif //UNIT_TEST

// I wanna sing a song for you, dead loop no escape
