#include "Common.h"

#include "ProtobufCodec.h"
#include "SocketOps.h"
#include <cassert>

//! message format
//  int32_t len
//  int32_t name_len
//  char    typename[name_len]
//  char    protobuf_data[len-name_len-sizeof(name_len)]

protobuf::Message*
ProtobufCodec::createMessageByTypeName (const std::string& type_name)
{
    protobuf::Message* message = nullptr;
    const auto desc = protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
    if (desc) {
        const auto prototype = protobuf::MessageFactory::generated_factory()->GetPrototype(desc);
        if (prototype) {
            message = prototype->New();
        } else {
            std::cerr << "prototype is not found for typename " << type_name << std::endl;
        }
    } else {
        std::cerr << "typename " << type_name << "is not a valide message typename!" << std::endl;
    }

    return message;
}

void
ProtobufCodec::recvMessage (const std::shared_ptr<TcpConnection>& connPtr, Buffer& buf)
{
    while (buf.readableBytes() >= kHeaderLen)
    {
        const int32_t len = buf.peekInt32();
        if (buf.readableBytes() >= kHeaderLen + len) {
            buf.readSize(kHeaderLen);
            const int32_t name_len = buf.readInt32();
            std::string type_name = buf.readString(name_len);
            protobuf::Message* message = createMessageByTypeName(type_name);
            assert(message);
            std::unique_ptr<protobuf::Message> messagePtr(message);
            bool parseStatus = parseFromSlice(
                    Slice(buf.peek(), len-name_len-sizeof(int32_t)), messagePtr.get());
            if (parseStatus) {
                assert(_message_cb);
                _message_cb(connPtr, std::move(messagePtr));
                buf.retrieve(kHeaderLen + len);
            } else {
                std::cerr << "[ProtobufCodec] parse error" << std::endl;
                break;
            }
        } else {
            break;
        }
    }
}

bool
ProtobufCodec::parseFromSlice (const Slice& slice, protobuf::Message* message)
{
    return message->ParseFromArray(slice.data(), slice.size());
}

void
ProtobufCodec::sendMessage (const std::shared_ptr<TcpConnection>& connPtr,
                            const protobuf::Message& message)
{
    Buffer buf;
    serializeToBuffer(buf, message);
    connPtr->send(Slice(buf.peek(), buf.readableBytes()));
}

size_t
ProtobufCodec::serializeToBuffer (Buffer& buf,
                                  const protobuf::Message& message)
{
    assert(buf.empty());

    int byte_size = message.ByteSize();
    std::string type_name = message.GetTypeName();
    buf.appendInt32(sockops::hostToNetwork32(byte_size + type_name.length() + sizeof(int32_t))); // len
    buf.appendInt32(sockops::hostToNetwork32(type_name.length()));                               // typename len
    buf.appendString(type_name);                                                                 // typename

    buf.ensureWriteableBytes(byte_size);

    uint8_t* start = reinterpret_cast<uint8_t*>(buf.writePos());
    bool seriailizeStatus = message.SerializeToArray(start, byte_size);
    if (!seriailizeStatus) {
        std::cerr << "[ProtobufCodec] serializeToBuffer failed!" << std::endl;
    }
    buf.writeSize(byte_size);

    return byte_size;
}
