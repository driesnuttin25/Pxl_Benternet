#ifndef BASECOMMUNICATION_H
#define BASECOMMUNICATION_H

#include <zmq.hpp>
#include <string>

class BaseCommunication {
protected:
    zmq::context_t context;
    zmq::socket_t socket;

public:
    BaseCommunication(zmq::socket_type type);
    void connect(const std::string& address);
    void bind(const std::string& address);
    void sendMessage(const std::string& message);
    std::string receiveMessage();
};

#endif // BASECOMMUNICATION_H
