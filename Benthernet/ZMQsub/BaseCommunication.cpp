#include "BaseCommunication.h"
#include <iostream>

BaseCommunication::BaseCommunication(zmq::socket_type type) : context(1), socket(context, type) {}

void BaseCommunication::connect(const std::string& address) {
    socket.connect(address);
}

void BaseCommunication::bind(const std::string& address) {
    socket.bind(address);
}

void BaseCommunication::sendMessage(const std::string& message) {
    zmq::message_t zmqMessage(message.size());
    memcpy(zmqMessage.data(), message.c_str(), message.size());
    socket.send(zmqMessage, zmq::send_flags::none);
}

std::string BaseCommunication::receiveMessage() {
    zmq::message_t message;
    socket.recv(message, zmq::recv_flags::none);
    return std::string(static_cast<char*>(message.data()), message.size());
}
