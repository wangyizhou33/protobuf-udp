#include <iostream>
#include <fstream>
#include <string>
#include "addressbook.pb.h"
#include <udp.hpp>

int main(int argc, char *argv[])
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    udp::Address my_address("localhost", "4000");
    udp::Socket listener;
    listener.open(my_address.getPort());
    udp::Address sender;

    tutorial::AddressBook address_book;
    int buffer_size = 1024;
    uint8_t *buffer = new uint8_t[buffer_size];
    int read_bytes;
    if (0 < (read_bytes = listener.receive(sender, buffer, buffer_size))) // this is blocking!
    {
        address_book.ParseFromArray(buffer, read_bytes);
    }
    listener.close();

    for (int i = 0; i < address_book.people_size(); i++) {
        const tutorial::Person &person = address_book.people(i);

        std::cout << "Person ID: " << person.id() << std::endl;
        std::cout << "  Name: " << person.name() << std::endl;
        if (person.email() != "") {
            std::cout << "  E-mail address: " << person.email() << std::endl;
        }
    }

    delete[] buffer;
    return 0;
}