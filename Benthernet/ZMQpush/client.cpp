#include "Client.h"
#include <iostream>

Client::Client(const std::string& pushAddress, const std::string& subAddress)
    : context(1), pusher(context, zmq::socket_type::push), subscriber(context, zmq::socket_type::sub) {
    std::cout << "Connecting to " << pushAddress << " and " << subAddress << std::endl;
    pusher.connect(pushAddress);
    subscriber.connect(subAddress);
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "response<", 8);  // Subscribe to all responses
    std::cout << "Subscribed to all response topics" << std::endl;
}

void Client::sendRequest(const std::string& request) {
    zmq::message_t zmqMessage(request.size());
    memcpy(zmqMessage.data(), request.c_str(), request.size());
    pusher.send(zmqMessage, zmq::send_flags::none);
    std::cout << "Request sent: " << request << std::endl;
}

std::string Client::receiveResponse() {
    zmq::message_t message;
    subscriber.recv(message, zmq::recv_flags::none);
    std::string response(static_cast<char*>(message.data()), message.size());
    std::cout << "Response received: " << response << std::endl;
    return response;
}
