#include "transceiver.h"

#include <iostream>
#include <stdexcept>

using namespace std;

bool Transceiver::receive(Buffer& buffer, SocketOps* sock_ops,
                          bool& header_skipped)
{
  return false;
}
