#ifndef _HTTP_DOWNLOADER_H
#define _HTTP_DOWNLOADER_H

#include "http_general.h"
#include "definitions.h"
#include "node.h"

class HttpDownloader : public HttpGeneral{
  public:
  using HttpGeneral::HttpGeneral;

  private:
  bool check_error(int len) const override;
  void disconnect() override;
};
#endif
