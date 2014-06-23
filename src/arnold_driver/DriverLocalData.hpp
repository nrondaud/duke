#pragma once

#include <ai.h>
#include "protocol/pbhelpers.hpp"
#include "protocol/zmq.hpp"

class DriverLocalData
{
  public:
    DriverLocalData();
    void send(const duke::protocol::Message&);
  private:
    zmq::context_t context; // zmq context
    zmq::socket_t socket; // zmq socket
    const char* identity; // client identity
};
