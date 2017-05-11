#include <udp.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h> // import ::close()
#include <netdb.h>
#include <sstream>

namespace udp
{

const uint32_t NULL_ADDRESS  = 0;
const uint16_t NULL_PORT     = 0;
const int NULL_SOCKET        = 0;
const int MAX_RECV_BUFF_SIZE = 256000; // i.e. 256Kbytes

Address::Address()
{
    storeAddress(NULL_ADDRESS, NULL_PORT);
}

Address::Address(const char *hostname, const char *service)
{
    addrinfo *result;
    getaddrinfo(hostname, service, nullptr, &result);

    if (AF_INET != result->ai_family) {
        std::cerr << "Address family is not AF_INET.";
    }

    sockaddr_in *sai = reinterpret_cast<sockaddr_in *>(result->ai_addr);

    storeAddress(ntohl(sai->sin_addr.s_addr), ntohs(sai->sin_port));

    freeaddrinfo(result);
}

Address::Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port)
{
    storeAddress((a << 24) | (b << 16) | (c << 8) | d, port);
}

Address::Address(uint32_t address, uint16_t port)
    : address_(address), port_(port)
{
    storeAddress(address, port);
}

Address::Address(const sockaddr_in &sain)
    : address_(ntohl(sain.sin_addr.s_addr)), port_(ntohs(sain.sin_port))
{
    storeAddress(ntohl(sain.sin_addr.s_addr), ntohs(sain.sin_port));
}

const sockaddr *Address::sockAddr() const
{
    return reinterpret_cast<const sockaddr *>(&sock_addr_);
}

socklen_t Address::sockAddrLength() const
{
    return sizeof(sockaddr_in);
}

uint32_t Address::getAddress() const
{
    return address_;
}

uint8_t Address::getA() const
{
    return address_ >> 24;
}

uint8_t Address::getB() const
{
    return address_ >> 16;
}

uint8_t Address::getC() const
{
    return address_ >> 8;
}

uint8_t Address::getD() const
{
    return address_ & 255;
}

const std::string &Address::getDisplayAddress() const
{
    return display_address_;
}

uint16_t Address::getPort() const
{
    return port_;
}

bool Address::operator==(const Address &other) const
{
    return (address_ == other.address_) && (port_ == other.port_);
}

bool Address::operator!=(const Address &other) const
{
    return !operator==(other);
}

void Address::storeAddress(uint32_t address, uint16_t port)
{
    address_ = address;
    port_    = port;

    sock_addr_.sin_family      = AF_INET;
    sock_addr_.sin_addr.s_addr = htonl(address_);
    sock_addr_.sin_port        = htons(port_);

    std::stringstream ss;
    ss << (int)getA() << "." << (int)getB() << "." << (int)getC() << "." << (int)getD() << ":" << getPort();
    display_address_ = ss.str();
}

Socket::Socket()
    : socket_(NULL_SOCKET)
{
}

Socket::Socket(uint16_t port)
    : socket_(NULL_SOCKET)
{
    open(port);
}

Socket::~Socket()
{
    close();
}

bool Socket::open(uint16_t port, bool non_blocking)
{
    if (isOpen()) {
        std::cerr << "Socket is already opened somewhere.";
        return false;
    }

    // create socket
    socket_ = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (0 >= socket_) {
        // failed to create socket
        socket_ = NULL_SOCKET;
        std::cerr << "Socket failed 0 at port: " << port;
        return false;
    }

    // enable broadcast
    int broadcast = 1;

    if (0 != ::setsockopt(socket_, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast)) {
        // failed to enable broadcast
        socket_ = NULL_SOCKET;
        std::cerr << "Socket failed 1 at port: " << port;
        return false;
    }

    int rcvbufsize = MAX_RECV_BUFF_SIZE;

    if (0 != ::setsockopt(socket_, SOL_SOCKET, SO_RCVBUF, &rcvbufsize, sizeof rcvbufsize)) {
        // failed to set rcvbuf
        socket_ = NULL_SOCKET;
        std::cerr << "Socket failed 2 at port: " << port;
        return false;
    }

    // bind to port
    Address address(INADDR_ANY, port);

    if (0 > ::bind(socket_, address.sockAddr(), address.sockAddrLength())) {
        //    failed to bind socket
        close();
        std::cerr << "Socket failed 3 at port: " << port;
        return false;
    }

    // set non-blocking io
    if (non_blocking) {
        int un_block = 1;

        if (-1 == ::fcntl(socket_, F_SETFL, O_NONBLOCK, un_block)) {
            //failed to set non-blocking socket
            close();
            std::cerr << "Socket failed 4 at port: " << port;
            return false;
        }
    }

    std::cerr << "Socket created at port: " << port;

    return true;
}

void Socket::close()
{
    if (isOpen()) {
        ::close(socket_);
        socket_ = NULL_SOCKET;
    }
}

void Socket::shutdown()
{
    if (isOpen()) {
        ::shutdown(socket_, 2); // 2 flag shuts down both send and receive
        socket_ = NULL_SOCKET;
    }
}

bool Socket::isOpen() const
{
    return socket_ != NULL_SOCKET;
}

bool Socket::send(const Address &destination, const void *data, int size) const
{
    if (data == nullptr || size <= 0) {
        std::cerr << "Outgoing data is empty.";
        return false;
    }

    if (socket_ == NULL_SOCKET) {
        std::cerr << "Socket is not opened.";
        return false;
    }

    int sent_bytes = ::sendto(socket_, data, size, 0, destination.sockAddr(),
                              destination.sockAddrLength());

    return sent_bytes == size;
}

int Socket::receive(Address &sender, void *data, int size) const
{
    if (data == nullptr || size <= 0) {
        std::cerr << "Buffer not big enough for incoming data";
        return false;
    }

    if (socket_ == NULL_SOCKET) {
        return false;
    }

    if (size > MAX_RECV_BUFF_SIZE) {
        std::cerr << "Buffer exceeds maximum allowed size.";
        return false;
    }

    sockaddr_in from;
    socklen_t fromLength = sizeof(from);

    int received_bytes = ::recvfrom(socket_, data, size, MSG_WAITALL,
                                    reinterpret_cast<sockaddr *>(&from), &fromLength);

    if (0 >= received_bytes) {
        std::cerr << "Received zero data.";
        return 0;
    }

    sender = std::move(Address(from));

    return received_bytes;
}
} // namespace udp