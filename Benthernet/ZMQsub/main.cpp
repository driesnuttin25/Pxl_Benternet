#include "randomsentenceservice.h"
#include "spellcheckerservice.h"
#include "logger.h"
#include "heartbeat.h"
#include <iostream>
#include <thread>

/**********************************
*  Main Entry Point
**********************************/

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

        std::cout << "Starting services..." << std::endl;
        Logger::log(Logger::Level::INFO, "Starting services...");

        // Create heartbeats
        Heartbeat generalHeartbeat(20, "<heartbeat<dries_project>", "tcp://benternet.pxl-ea-ict.be:24041");
        //Heartbeat randomSentenceHeartbeat(10, "<heartbeat<random_sentence_service>", "tcp://benternet.pxl-ea-ict.be:24041");
        //Heartbeat spellCheckerHeartbeat(10, "<heartbeat<spell_checker_service>", "tcp://benternet.pxl-ea-ict.be:24041");

        // Start heartbeats
        generalHeartbeat.start();
        //randomSentenceHeartbeat.start();
        //spellCheckerHeartbeat.start();

        // Create threads for each service
        std::thread spellCheckerThread(runSpellCheckerService);
        std::thread randomSentenceThread(runRandomSentenceService);

        // Join threads
        spellCheckerThread.join();
        randomSentenceThread.join();

        std::cout << "Services initialized and running..." << std::endl;
        Logger::log(Logger::Level::INFO, "Services initialized and running...");

        // Stop the heartbeats when services are done
        generalHeartbeat.stop();
        //randomSentenceHeartbeat.stop();
        //spellCheckerHeartbeat.stop();
    } catch (const std::exception& ex) {
        Logger::log(Logger::Level::ERROR, std::string("Exception in main: ") + ex.what());
    }

    Logger::close();  // Close the logger
    return 0;
}
