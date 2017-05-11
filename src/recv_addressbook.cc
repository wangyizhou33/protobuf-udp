#include <iostream>
#include <fstream>
#include <string>
#include "addressbook.pb.h"
#include <udp.hpp>

int main(int argc, char *argv[])
{

    udp::Address my_address("localhost", "4000");
    udp::Socket listener;
    listener.open(my_address.getPort());
    udp::Address sender;

    int buffer_size = 12;
    uint8_t *buffer = new uint8_t[buffer_size];
    int read_bytes;
    if (0 < (read_bytes = listener.receive(sender, buffer, buffer_size))) // this is blocking!
    {
        std::cout << "Received " << read_bytes << " bytes\n";
        std::cout << "Contents: " << buffer << std::endl;
    }
    listener.close();
    delete[] buffer;
    return 0;
}