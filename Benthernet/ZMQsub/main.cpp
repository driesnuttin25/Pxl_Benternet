#include "randomsentenceservice.h"
#include "spellcheckerservice.h"
#include "logger.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <zmq.hpp>

// Function to send heartbeat messages
void sendHeartbeat() {
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_PUSH);
    socket.connect("tcp://benternet.pxl-ea-ict.be:24041");

    while (true) {
        socket.send(zmq::buffer("<dries<heartbeat>"), zmq::send_flags::none);
        Logger::log(Logger::Level::INFO, "Sent heartbeat: <dries<heartbeat>");
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
}

// Function to run spell checker service
void runSpellCheckerService() {
    try {
        SpellCheckerService spellCheckerService(R"(C:\Users\dries\OneDrive\Desktop\git\Pxl_Benternet\Benthernet\dictionary.txt)");
        spellCheckerService.processMessages();
    } catch (const std::exception& ex) {
        Logger::log(Logger::Level::ERROR, std::string("Exception in SpellCheckerService: ") + ex.what());
    }
}

// Function to run random sentence service
void runRandomSentenceService() {
    try {
        RandomSentenceService randomSentenceService(R"(C:\Users\dries\OneDrive\Desktop\git\Pxl_Benternet\Benthernet\dictionary.txt)");
        randomSentenceService.processMessages();
    } catch (const std::exception& ex) {
        Logger::log(Logger::Level::ERROR, std::string("Exception in RandomSentenceService: ") + ex.what());
    }
}

int main() {
    try {
        Logger::init(R"(C:\Users\dries\OneDrive\Desktop\git\Pxl_Benternet\Benthernet\service.log)");  // Initialize the logger

        Logger::log(Logger::Level::INFO, "Starting services...");

        // Create threads for each service and heartbeat
        std::thread spellCheckerThread(runSpellCheckerService);
        std::thread randomSentenceThread(runRandomSentenceService);
        std::thread heartbeatThread(sendHeartbeat);

        // Join threads
        spellCheckerThread.join();
        randomSentenceThread.join();
        heartbeatThread.join();

        Logger::log(Logger::Level::INFO, "Services initialized and running...");
    } catch (const std::exception& ex) {
        Logger::log(Logger::Level::ERROR, std::string("Exception in main: ") + ex.what());
    }

    Logger::close();  // Close the logger
    return 0;
}
