#include "DriverLocalData.hpp"

using namespace duke::protocol;

DriverLocalData::DriverLocalData() 
  : context(1) 
  , socket(context, ZMQ_DEALER)
  , identity("displaydriver")
{
  socket.setsockopt(ZMQ_IDENTITY, identity, strlen(identity));
  socket.connect("tcp://localhost:5570");
}

void DriverLocalData::send(const Message& pbMsg) 
{
  const MessageHolder & holder = pack(pbMsg, MessageHolder::CREATE);
  std::string str;
  holder.SerializeToString(&str);
  zmq::message_t zMsg(str.length());
  memcpy(zMsg.data(), str.c_str(), str.length());
  socket.send(zMsg);
}
