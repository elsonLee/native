#include "Common.h"

#include "ProtobufCodec.h"
#include "SocketOps.h"
#include <cassert>

void
ProtobufCodec::onRecvMessage (const std::shared_ptr<TcpConnection>& connPtr, Buffer& buf)
{
    while (buf.readableBytes() >= kHeaderLen)
    {
        const int32_t len = buf.peekInt32();
        if (buf.readableBytes() >= kHeaderLen + len) {
            std::unique_ptr<protobuf::Message> messagePtr(_prototype->New());
            bool parseStatus = parseFromSlice(Slice(buf.peek(), kHeaderLen + len), messagePtr.get());
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
    buf.appendInt32(sockops::hostToNetwork32(byte_size));   // header: len

    buf.ensureWriteableBytes(byte_size);

    uint8_t* start = reinterpret_cast<uint8_t*>(buf.writePos());
    bool seriailizeStatus = message.SerializeToArray(start, byte_size);
    if (!seriailizeStatus) {
        std::cerr << "[ProtobufCodec] serializeToBuffer failed!" << std::endl;
    }
    buf.writeSize(byte_size);

    return byte_size;
}
