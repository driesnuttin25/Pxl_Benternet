#include "Client.h"
#include <iostream>
#include <thread>
#include <chrono>

/**********************************
*  Client Class
**********************************/

// Constructor: Initializes the ZMQ context and connects sockets
Client::Client(const std::string& pushAddress, const std::string& subAddress)
    : context(1), pusher(context, zmq::socket_type::push), subscriber(context, zmq::socket_type::sub), heartbeatTopic("<heartbeat<dries_project>") {
    std::cout << "Connecting to " << pushAddress << " and " << subAddress << std::endl;
    pusher.connect(pushAddress);
    subscriber.connect(subAddress);
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);  // Subscribe to all topics
    std::cout << "Subscribed to all topics" << std::endl;
}

// Sends a request to the server
void Client::sendRequest(const std::string& request) {
    zmq::message_t zmqMessage(request.size());
    memcpy(zmqMessage.data(), request.c_str(), request.size());

    int retries = 3; // Number of retries
    while (retries > 0) {
        try {
            pusher.send(zmqMessage, zmq::send_flags::none);
            std::cout << "Request sent: " << request << std::endl;
            break;
        } catch (const zmq::error_t& e) {
            std::cerr << "Failed to send request: " << e.what() << std::endl;
            if (--retries == 0) {
                std::cerr << "Failed to send request after retries." << std::endl;
            } else {
                std::this_thread::sleep_for(std::chrono::seconds(1)); // Wait before retrying
            }
        }
    }
}

// Receives a response from the server
std::string Client::receiveResponse() {
    zmq::message_t message;
    subscriber.recv(message, zmq::recv_flags::none);
    std::string response(static_cast<char*>(message.data()), message.size());
    std::cout << "Response received: " << response << std::endl;
    return response;
}

// Checks if a response is available from the server
bool Client::isResponseAvailable() {
    zmq::pollitem_t items[] = { {static_cast<void*>(subscriber), 0, ZMQ_POLLIN, 0} };
    zmq::poll(&items[0], 1, std::chrono::milliseconds(100));
    return items[0].revents & ZMQ_POLLIN;
}

// Checks if a heartbeat message is received
bool Client::isHeartbeatReceived() {
    if (isResponseAvailable()) {
        std::string response = receiveResponse();
        return response.find(heartbeatTopic) != std::string::npos;
    }
    return false;
}
