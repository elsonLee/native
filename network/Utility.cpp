#include "Utility.h"

#include <google/protobuf/message.h>

namespace utility
{

google::protobuf::Message*
createMessageByTypeName (const std::string& type_name)
{
    google::protobuf::Message* message = nullptr;
    const auto desc =
        google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
    if (desc) {
        const auto prototype =
            google::protobuf::MessageFactory::generated_factory()->GetPrototype(desc);
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

}
