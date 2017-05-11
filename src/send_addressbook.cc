#include <iostream>
#include <fstream>
#include <string>
#include "addressbook.pb.h"
#include <udp.hpp>

int main(int argc, char *argv[])
{
    udp::Address my_address("localhost", "3999");
    udp::Socket sender(my_address.getPort());
    udp::Address recv_address("localhost", "4000");

    uint8_t msg[] = "abcd1234+*";
    sender.send(recv_address, msg, 10);

    // Only allow the listener thread to listen (blocking) for 0.1 seconds.
    sender.close();

    return 0;
}