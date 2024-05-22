#include "Client.h"
#include <iostream>
#include <exception>

void testSpellChecker(Client& client) {
    std::string spellCheckRequest = "spellingschecker<Dries<Thiss is an exmple sentence>";
    std::cout << "Sending spell check request: " << spellCheckRequest << std::endl;
    client.sendRequest(spellCheckRequest);
    std::string spellCheckResponse = client.receiveResponse();
    std::cout << "Spell Checker Response: " << spellCheckResponse << std::endl;
}

void testRandomSentence(Client& client) {
    std::string randomSentenceRequest = "randomsentence<Imee<5>";
    std::cout << "Sending random sentence request: " << randomSentenceRequest << std::endl;
    client.sendRequest(randomSentenceRequest);
    std::string randomSentenceResponse = client.receiveResponse();
    std::cout << "Random Sentence Response: " << randomSentenceResponse << std::endl;
}

int main() {
    try {
        std::cout << "Creating client..." << std::endl;
        Client client("tcp://benternet.pxl-ea-ict.be:24041", "tcp://benternet.pxl-ea-ict.be:24042");
        std::cout << "Client created. Testing spell checker..." << std::endl;
        testSpellChecker(client);
        std::cout << "Testing random sentence generator..." << std::endl;
        testRandomSentence(client);
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }
    return 0;
}
