#ifndef CLIENT_H
#define CLIENT_H

#include <zmq.hpp>
#include <string>
#include <thread>
#include <atomic>

/**********************************
*  Client Class Declaration
**********************************/
class Client {
public:
    // Constructor that sets up the ZMQ push and sub sockets
    Client(const std::string& pushAddress, const std::string& subAddress);

    // Method to send a request to the server
    void sendRequest(const std::string& request);

    // Method to receive a response from the server
    std::string receiveResponse();

    // Method to check if a response is available
    bool isResponseAvailable();

    // Method to check for heartbeat messages
    bool isHeartbeatReceived();

private:
    zmq::context_t context; // ZMQ context for managing sockets
    zmq::socket_t pusher;   // ZMQ push socket for sending requests
    zmq::socket_t subscriber; // ZMQ sub socket for receiving responses

    std::string heartbeatTopic; // Topic for heartbeat messages
};

#endif // CLIENT_H
