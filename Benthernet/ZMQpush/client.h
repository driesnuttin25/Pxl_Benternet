#ifndef CLIENT_H
#define CLIENT_H

#include <zmq.hpp>
#include <string>

class Client {
public:
    Client(const std::string& address);
    void sendRequest(const std::string& request);
    std::string receiveResponse();
private:
    zmq::context_t context;
    zmq::socket_t socket;
};

#endif // CLIENT_H
