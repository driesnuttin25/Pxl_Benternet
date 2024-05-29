#ifndef BASECOMMUNICATION_H
#define BASECOMMUNICATION_H

#include <zmq.hpp>
#include <string>

/**********************************
*  Base Communication Class Declaration
**********************************/
class BaseCommunication {
protected:
    zmq::context_t context;
    zmq::socket_t socket;

public:
    // Constructor to initialize ZMQ context and socket type
    BaseCommunication(zmq::socket_type type);

    // Connect the socket to a specified address
    void connect(const std::string& address);

    // Bind the socket to a specified address
    void bind(const std::string& address);

    // Send a message through the socket
    void sendMessage(const std::string& message);

    // Receive a message from the socket
    std::string receiveMessage();
};

#endif // BASECOMMUNICATION_H
