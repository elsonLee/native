#include "Header.h"
#include "address_book.pb.h"

#include <stdio.h>
#include <thread>
#include <iostream>

using namespace tutorial;

void
printAddressBook (const AddressBook& address_book)
{
    for (int i = 0; i < address_book.people_size(); i++) {
        const tutorial::Person& person = address_book.people(i);

        std::cout << "Person ID: " << person.id() << std::endl;
        std::cout << "  Name: " << person.name() << std::endl;
        if (person.has_email()) {
            std::cout << "  E-mail address: " << person.email() << std::endl;
        }

        for (int j = 0; j < person.phones_size(); j++) {
            const tutorial::Person::PhoneNumber& phone_number = person.phones(j);

            switch (phone_number.type()) {
                case tutorial::Person::MOBILE:
                    std::cout << "  Mobile phone #: ";
                    break;
                case tutorial::Person::HOME:
                    std::cout << "  Home phone #: ";
                    break;
                case tutorial::Person::WORK:
                    std::cout << "  Work phone #: ";
                    break;
            }
            std::cout << phone_number.number() << std::endl;
        }
  }
}

class Client : public TcpClient
{
    public:
        Client (const std::string& name, EventLoop* loop, const InetAddress& server_addr) :
            TcpClient(name, loop, server_addr),
            _codec(&AddressBook::default_instance(),
                   [this](const std::shared_ptr<TcpConnection>& connPtr,
                          std::unique_ptr<protobuf::Message> message)
                   { onMessage(connPtr, std::move(message)); })
        {
            setMessageCallback([this](const std::shared_ptr<TcpConnection>& connPtr, Buffer& buf)
                               { _codec.recvMessage(connPtr, buf); });

            setConnectCallback([this](const std::shared_ptr<TcpConnection>& connPtr)
                               { onConnect(connPtr); });
        }

        void onMessage (const std::shared_ptr<TcpConnection>&, std::unique_ptr<protobuf::Message>) {
            std::cout << "client recieved" << std::endl;
        }

        void onConnect (const std::shared_ptr<TcpConnection>& connPtr) {
            AddressBook address_book;
            auto person = address_book.add_people();
            person->set_name("lexun");
            person->set_id(1);
            person->set_email("lexun@xx.com");
            auto phone = person->add_phones();
            phone->set_number("12345678");
            phone->set_type(Person_PhoneType_HOME);
            _codec.sendMessage(connPtr, address_book);
        }

    private:
        ProtobufCodec   _codec;
};

class Server : public TcpServer
{
    public:
        Server (const std::string& name, EventLoop* loop, const InetAddress& listen_addr) :
            TcpServer(name, loop, listen_addr),
            _codec(&AddressBook::default_instance(),
                   [this](const std::shared_ptr<TcpConnection>& connPtr,
                          std::unique_ptr<protobuf::Message> message)
                   { onMessage(connPtr, std::move(message)); })
        {
            setMessageCallback([this](const std::shared_ptr<TcpConnection>& connPtr, Buffer& buf)
                               { _codec.recvMessage(connPtr, buf); });
        }

        void onMessage (const std::shared_ptr<TcpConnection>&, std::unique_ptr<protobuf::Message>);

    private:
        ProtobufCodec   _codec;
};

void
Server::onMessage (const std::shared_ptr<TcpConnection>& connPtr, std::unique_ptr<protobuf::Message> message)
{
    AddressBook* address_book = dynamic_cast<AddressBook*>(message.get());
    printAddressBook(*address_book);
}

int main (int argc, char* argv[])
{
    std::thread([]{
                EventLoop loop;
                Server server("server", &loop, InetAddress(9981));
                server.start();
                loop.run();
            }).detach();

    std::thread([]{
                EventLoop loop;
                Server server("server", &loop, InetAddress(9981));
                loop.run();
            }).detach();

    return 0;
}
