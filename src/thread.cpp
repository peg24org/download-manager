#include "thread.h"
#include "definitions.h"
void Thread::start()
{
	threadObject = new std::thread(&Thread::run,this);
}
Thread::Thread()
{
}
Thread::~Thread()
{
	try {
		if(threadObject)
			delete threadObject;
	}
	catch(...){
		std::cerr<<"Pointer delete exception."<<std::endl;
	}
}
void Thread::join()
{
	threadObject->join();
}
