#ifndef UDP_H
#define UDP_H

#include <netinet/in.h>
#include <iostream>

namespace udp
{

/// Internet Address to/from which an object of Socket class send/receive data
class Address
{
public:
    /// Set address_ and port_ to 0.
    Address();

    /// @param[in] hostname e.g. "localhost", the resulting address will be 127.0.0.1
    /// @param[in] service e.g. can be string of port number "8080", or descriptive "echo"
    Address(const char *hostname, const char *service = nullptr);

    /// @param[in] a first byte of address
    /// @param[in] b second byte of address
    /// @param[in] c third byte of address
    /// @param[in] d fourth byte of address
    /// @param[in] port port number
    explicit Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port);

    /// @param[in] address address in 32 bits unsigned int
    /// @param[in] port port number
    explicit Address(uint32_t address, uint16_t port);

    /// @param[in] sain of sockaddr_in type
    Address(const sockaddr_in &sain);

    /// @return socket address in type pointer to sockaddr
    const sockaddr *sockAddr() const;

    /// @return size of sockaddr_in
    socklen_t sockAddrLength() const;

    /// @return address_ in 32bits unsigned int
    uint32_t getAddress() const;

    /// @return first byte of address
    uint8_t getA() const;

    /// @return second byte of address
    uint8_t getB() const;

    /// @return third byte of address
    uint8_t getC() const;

    /// @return fourth byte of address
    uint8_t getD() const;

    /// @return address in string format e.g. "127.0.0.1:4000"
    const std::string &getDisplayAddress() const;

    /// @return port number
    uint16_t getPort() const;

    /// Equality comparison
    bool operator==(const Address &other) const;

    /// Inequality comparison
    bool operator!=(const Address &other) const;

private:
    /// Set address and port
    void storeAddress(uint32_t address, uint16_t port);

    /// address
    uint32_t address_;

    /// port
    uint16_t port_;

    sockaddr_in sock_addr_;

    /// e.g. "127.0.0.1:4000"
    std::string display_address_;
}; //class Address

/// Open/close socket at a given port and provide public APIs to receive/send msg in UDP protocol.
/// Receiver example:\n
/// udp::Address my_address("localhost","4000");\n
/// udp::Socket listener(my_address.getPort());\n
/// udp::Address sender;\n
/// int read_bytes = listener.receive(sender, buffer, buffer_size);\n
/// \n
/// Sender example:\n
/// udp::Address my_address("localhost","3999");\n
/// udp::Socket my_socket(my_address.getPort());\n
/// udp::Address recv_address("localhost","4000");\n
/// uint8_t msg[];\n
/// my_socket.send(recv_address, msg, msg_size);
class Socket
{
public:
    Socket();

    /// Constructor that open a socket at the given port
    /// @param[in] port port where we want to open a socket
    Socket(uint16_t port);

    ~Socket();

    /// Open socket
    /// @param[in] port port where we want to open a socket
    /// @param[in] non_blocking flag that indicates blockage
    /// @return true if socket is opened successfully
    bool open(uint16_t port, bool non_blocking = false);

    /// Close the opened socket
    void close();

    /// Shutdown the socket
    void shutdown();

    /// @return true if the socket is opened
    bool isOpen() const;

    /// API that sends a message to a destination in Class Address
    /// @param[in] destination recipient address in class Address
    /// @param[in] data pointer to message
    /// @param[in] size message size
    /// @return true if the the entire message is sent successfully
    bool send(const Address &destination, const void *data, int size) const;

    /// API that receives a message from a sender
    /// @param[out] sender sender address
    /// @param[out] data pointer to the data container
    /// @param[in] size reserved array size of data array
    /// @return number of bytes received
    int receive(Address &sender, void *data, int size) const;

private:
    int socket_;

}; // class Socket

} // namespace udp
#endif