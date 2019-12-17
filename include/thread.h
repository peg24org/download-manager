#ifndef _THREAD_H
#define _THREAD_H

#include <thread>

using namespace std;

class Thread {
  public:
    virtual ~Thread();
    Thread();

    void start();
    void join();

  protected:
    virtual void run() = 0;
    thread* threadObject = nullptr;
};

#endif
