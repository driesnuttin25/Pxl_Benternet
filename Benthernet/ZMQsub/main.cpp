#include "spellcheckerservice.h"
#include "randomsentenceservice.h"
#include <iostream>
#include <thread>

/**********************************
*  Main Entry Point
**********************************/

void runSpellCheckerService() {
    try {
        SpellCheckerService spellCheckerService(R"(C:\Users\dries\dictionary.txt)");
        spellCheckerService.processMessages();
    } catch (const std::exception& ex) {
        std::cerr << "Exception in SpellCheckerService: " << ex.what() << std::endl;
    }
}

void runRandomSentenceService() {
    try {
        RandomSentenceService randomSentenceService(R"(C:\Users\dries\dictionary.txt)");
        randomSentenceService.processMessages();
    } catch (const std::exception& ex) {
        std::cerr << "Exception in RandomSentenceService: " << ex.what() << std::endl;
    }
}

int main() {
    try {
        std::cout << "Starting services..." << std::endl;

        // Create threads for each service
        std::thread spellCheckerThread(runSpellCheckerService);
        std::thread randomSentenceThread(runRandomSentenceService);

        // Join threads
        spellCheckerThread.join();
        randomSentenceThread.join();
        // No idea how I got that to work but it seems to work

        std::cout << "Services initialized and running..." << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Exception in main: " << ex.what() << std::endl;
    }
    return 0;
}
