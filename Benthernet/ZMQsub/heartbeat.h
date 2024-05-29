#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include <thread>
#include <atomic>
#include <string>
#include <zmq.hpp>

class Heartbeat {
public:
    Heartbeat(int intervalSeconds, const std::string& message, const std::string& address);
    ~Heartbeat();

    // Start the heartbeat
    void start();

    // Stop the heartbeat
    void stop();

private:
    int intervalSeconds;
    std::string message;
    zmq::context_t context;
    zmq::socket_t publisher;
    std::thread heartbeatThread;
    std::atomic<bool> running;

    // Heartbeat loop
    void heartbeatLoop();
};

#endif // HEARTBEAT_H
