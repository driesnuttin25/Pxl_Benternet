#include "BaseCommunication.h"
#include <iostream>

/**********************************
*  Base Communication Class Implementation
**********************************/

// Constructor: Initializes the ZMQ context and socket
BaseCommunication::BaseCommunication(zmq::socket_type type) : context(1), socket(context, type) {}

// Connects the socket to the given address
void BaseCommunication::connect(const std::string& address) {
    socket.connect(address);
}

// Binds the socket to the given address
void BaseCommunication::bind(const std::string& address) {
    socket.bind(address);
}

// Sends a message through the socket
void BaseCommunication::sendMessage(const std::string& message) {
    zmq::message_t zmqMessage(message.size());
    memcpy(zmqMessage.data(), message.c_str(), message.size());
    socket.send(zmqMessage, zmq::send_flags::none);
}

// Receives a message from the socket
std::string BaseCommunication::receiveMessage() {
    zmq::message_t message;
    socket.recv(message, zmq::recv_flags::none);
    return std::string(static_cast<char*>(message.data()), message.size());
}
