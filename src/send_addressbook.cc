#include <iostream>
#include <fstream>
#include <string>
#include "addressbook.pb.h"
#include <udp.hpp>

int main(int argc, char *argv[])
{
    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    udp::Address my_address("localhost", "3999");
    udp::Socket sender(my_address.getPort());
    udp::Address recv_address("localhost", "4000");

    tutorial::AddressBook address_book;
    tutorial::Person *person = address_book.add_people();
    person->set_id(3);
    person->set_name("Yizhou Wang");
    person->set_email("wangyizhou3@gmail.com");

    int size     = address_book.ByteSize();
    uint8_t *buf = new uint8_t[size];
    address_book.SerializeToArray(buf, size);

    sender.send(recv_address, buf, size);

    // Only allow the listener thread to listen (blocking) for 0.1 seconds.
    sender.close();

    delete[] buf;
    return 0;
}