#ifndef CLIENT_H
#define CLIENT_H

#include <zmq.hpp>
#include <string>

class Client {
public:
    Client(const std::string& pushAddress, const std::string& subAddress);
    void sendRequest(const std::string& request);
    std::string receiveResponse();
private:
    zmq::context_t context;
    zmq::socket_t pusher;
    zmq::socket_t subscriber;
};

#endif // CLIENT_H
#ifndef CLIENT_H
#define CLIENT_H

#include <zmq.hpp>
#include <string>

class Client {
public:
    Client(const std::string& pushAddress, const std::string& subAddress);
    void sendRequest(const std::string& request);
    std::string receiveResponse();
private:
    zmq::context_t context;
    zmq::socket_t pusher;
    zmq::socket_t subscriber;
};

#endif // CLIENT_H
