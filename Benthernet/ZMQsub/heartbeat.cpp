#include "heartbeat.h"
#include "logger.h"
#include <chrono>
#include <thread>

Heartbeat::Heartbeat(int intervalSeconds, const std::string& message, const std::string& address)
    : intervalSeconds(intervalSeconds), message(message), context(1), publisher(context, ZMQ_PUB), running(false) {
    publisher.connect(address);
}

Heartbeat::~Heartbeat() {
    stop();
}

void Heartbeat::start() {
    running = true;
    heartbeatThread = std::thread(&Heartbeat::heartbeatLoop, this);
}

void Heartbeat::stop() {
    running = false;
    if (heartbeatThread.joinable()) {
        heartbeatThread.join();
    }
}

void Heartbeat::heartbeatLoop() {
    while (running) {
        publisher.send(zmq::buffer(message), zmq::send_flags::none);
        //Logger::log(Logger::Level::INFO, "Sent heartbeat: " + message);
        std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
    }
}
