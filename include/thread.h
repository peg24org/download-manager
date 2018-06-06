#ifndef _THREAD_H
#define _THREAD_H

#include <thread>

using namespace std;

class Thread{

	protected:
		virtual void run() = 0;
		thread	*threadObject = NULL;

	public:
		void start();
		Thread();
		virtual ~Thread();
		void join();
};

#endif
