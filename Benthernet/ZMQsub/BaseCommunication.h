#ifndef BASECOMMUNICATION_H
#define BASECOMMUNICATION_H

#include <zmq.hpp>
#include <string>
#include <iostream>

class BaseCommunication {
protected:
    zmq::context_t context;
    zmq::socket_t socket;

public:
    explicit BaseCommunication(zmq::socket_type type) : context(1), socket(context, type) {}
    virtual ~BaseCommunication() {}

    void connect(const std::string& address) {
        socket.connect(address);
    }

    void sendMessage(const std::string& message) {
        zmq::message_t zmqMessage(message.size());
        memcpy(zmqMessage.data(), message.c_str(), message.size());
        socket.send(zmqMessage, zmq::send_flags::none);
    }

    std::string receiveMessage() {
        zmq::message_t message;
        socket.recv(message, zmq::recv_flags::none);
        return std::string(static_cast<char*>(message.data()), message.size());
    }
};

#endif // BASECOMMUNICATION_H
