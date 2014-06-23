#include "SocketIO.hpp"
#include "duke/protocol/pbhelpers.hpp"
#include <fstream>

using namespace std;
using namespace duke;

#define DECODE_WORKERS 3

namespace duke {
  void savePBMsgToDisk(protocol::MessageHolder& holder, const string& path) {
    std::fstream out(path, ios::out | ios::binary | ios::trunc);
    holder.SerializeToOstream(&out);
    out.close();
  } 

  void decode(protocol::MessageHolder& holder) {
    using namespace protocol;
    if(isType<Render>(holder)) {
      const Render& r = unpackTo<Render>(holder);
      cout << "| RENDER" << endl;
      cout << "|   camera: " << r.camera() << endl;
      cout << "|   frame: " << r.frame() << endl;
      cout << "|   width: " << r.description().width() << endl;
      cout << "|   height: " << r.description().height() << endl;
      stringstream ss;
      ss << "/tmp/duke/render_" << r.camera() << "_" << r.frame() << ".pb";
      savePBMsgToDisk(holder, ss.str());
    } else if(isType<Bucket>(holder)) {
      const Bucket& b = unpackTo<Bucket>(holder);
      cout << "| BUCKET " << endl;
      cout << "|   x: " << b.x() << endl;
      cout << "|   y: " << b.y() << endl;
      cout << "|   width: " << b.description().width() << endl;
      cout << "|   height: " << b.description().height() << endl;
      stringstream ss;
      ss << "/tmp/duke/bucket_" << b.x() << "_" << b.y() << ".pb";
      savePBMsgToDisk(holder, ss.str());
    }
  }

  class Worker {
    public:
      Worker(zmq::context_t &ctx, int sock_type)
      : m_context(ctx)
      , m_socket(m_context, sock_type) {}
    public:
      void work()
      {
        m_socket.connect("inproc://backend");
        zmq::pollitem_t items[] = {{m_socket, 0, ZMQ_POLLIN, 0}};
        try
        { 
          while (true)
          {
            zmq::poll(items, 1, 100); // timeout: 100 milliseconds
            if (items[0].revents & ZMQ_POLLIN) {
              zmq::message_t result;
              m_socket.recv(&result); // receive identity
              string identity(static_cast<char*>(result.data()), result.size());
              if(m_socket.recv(&result))  // receive data
              {
                protocol::MessageHolder holder;
                holder.ParseFromArray(result.data(), result.size());
                if(!holder.IsInitialized())
                  cout << "Received malformed message. skip." << endl;
                else
                  decode(holder);
              }
            }
          }
        } catch(std::exception &e) {}
      }
    private:
      zmq::context_t & m_context;
      zmq::socket_t m_socket;
  };
}


SocketIO::SocketIO() 
  : m_context(1)
  , m_frontend(m_context, ZMQ_ROUTER)
  , m_backend(m_context, ZMQ_DEALER)
{
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  m_thread = thread(&SocketIO::run, this);
  m_thread.detach();
}

SocketIO::~SocketIO() {
  m_backend.close();
  m_frontend.close();
  m_context.close();
}

void SocketIO::poll() {
}

void SocketIO::run() {
  m_frontend.bind("tcp://*:5570");
  m_backend.bind("inproc://backend");

  // workers
  std::vector<Worker*> workers;
  std::vector<std::thread*> workerThread;
  for (int i = 0; i < DECODE_WORKERS; ++i) {
    workers.push_back(new Worker(m_context, ZMQ_DEALER));
    workerThread.push_back(new thread(std::bind(&Worker::work, workers[i])));
    workerThread[i]->detach();
  }
  // proxy
  try {
    zmq::proxy(m_frontend, m_backend, nullptr);
  }
  catch (std::exception &e) {}
  // cleanup
  for (int i = 0; i < DECODE_WORKERS; ++i) {
    delete workers[i];
    delete workerThread[i];
  }
}
