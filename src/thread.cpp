#include <iostream>

#include "thread.h"

void Thread::start()
{
	threadObject = new std::thread(&Thread::run, this);
}

Thread::Thread()
{
}

Thread::~Thread()
{
	delete threadObject;
}

void Thread::join()
{
	threadObject->join();
}
