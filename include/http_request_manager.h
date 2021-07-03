#ifndef _HTTP_REQUEST_MANAGER_H
#define _HTTP_REQUEST_MANAGER_H

#include <mutex>
#include <memory>
#include <atomic>
#include <vector>
#include <functional>

#include "request_manager.h"

class HttpRequestManager: public RequestManager
{
  public:
    using RequestManager::RequestManager;

  private:
    virtual void send_requests() override;
    Buffer generate_request_str(const Request& request);
};

#endif

