#pragma once

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <iostream>
#include "duke.pb.h"

namespace duke {
  namespace protocol {

    using namespace google::protobuf;
    using namespace google::protobuf::io;
    using namespace std;

    static const Descriptor* descriptorFor(const MessageHolder &holder) {
      assert(holder.has_type_name());
      const Descriptor* pDescriptor = DescriptorPool::generated_pool()->FindMessageTypeByName(holder.type_name());
      return pDescriptor;
    }

    static Message* newInstanceFor(const Descriptor* pDescriptor) {
      if (!pDescriptor)
        return NULL;
      const Message* pPrototype = MessageFactory::generated_factory()->GetPrototype(pDescriptor);
      return pPrototype ? pPrototype->New() : NULL;
    }

    static void pack(const Message &msg, MessageHolder &holder, MessageHolder_Action action) {
      holder.Clear();
      holder.set_action(action);
      holder.set_type_name(msg.GetDescriptor()->full_name());
      msg.SerializeToString(holder.mutable_body());
    }

    static MessageHolder pack(const Message &msg, MessageHolder_Action action) {
      MessageHolder holder;
      pack(msg, holder, action);
      return holder;
    }

    static bool unpack(const MessageHolder &holder, Message& message) {
      const string &type_name(holder.type_name());
      const Descriptor *pDescriptor = descriptorFor(holder);
      if (!pDescriptor) {
        cerr << "Unable to find descriptor for class name " << type_name << endl;
        return false;
      }
      if (pDescriptor != message.GetDescriptor()) {
        cerr << "Expected message of type " << pDescriptor->name() << " but was " << message.GetDescriptor()->name() << endl;
        return false;
      }
      if (!message.ParseFromString(holder.body())) {
        cerr << "Unable to unmarshal MessageHolder's body into a " << type_name << ". Message looks corrupted." << endl;
        return false;
      }
      return true;
    }

template<typename Dst>
    Dst unpackTo(const MessageHolder &holder) {
      Dst object;
      unpack(holder, object);
      return object;
    }

template<typename T> 
    bool isType(const MessageHolder &holder) { 
      return descriptorFor(holder) == T::descriptor();
    }

} // namespace
} // namespace
