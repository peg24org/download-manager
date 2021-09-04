#include "transceiver.h"

#include <iostream>
#include <stdexcept>

using namespace std;

bool Transceiver::receive(Buffer& buffer, SocketOps* sock_ops,
                          bool& header_skipped)
{
  cerr << "--------------------------------------------------------------" << endl;
  throw runtime_error("error");
  return false;
}
