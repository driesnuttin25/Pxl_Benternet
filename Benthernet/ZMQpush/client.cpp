#include "Client.h"

Client::Client(const std::string& address) : context(1), socket(context, zmq::socket_type::req) {
    socket.connect(address);
}

void Client::sendRequest(const std::string& request) {
    zmq::message_t zmqMessage(request.size());
    memcpy(zmqMessage.data(), request.c_str(), request.size());
    socket.send(zmqMessage, zmq::send_flags::none);
}

std::string Client::receiveResponse() {
    zmq::message_t message;
    socket.recv(message, zmq::recv_flags::none);
    return std::string(static_cast<char*>(message.data()), message.size());
}
