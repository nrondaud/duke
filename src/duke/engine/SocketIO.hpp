#pragma once

#include "duke/commands/Commands.hpp"
#include "duke/protocol/zmq.hpp"
#include <concurrent/queue.hpp>
#include <thread>

class SocketIO {
  public:
    SocketIO();
    ~SocketIO();
  public:
    void poll();
  private:
    void run();
  private:
    zmq::context_t m_context;
    zmq::socket_t m_frontend;
    zmq::socket_t m_backend;
    std::thread m_thread;
};
