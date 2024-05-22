#include "RandomSentenceService.h"
#include <sstream>

RandomSentenceService::RandomSentenceService(const std::string& dictPath) : BaseCommunication(zmq::socket_type::sub) {
    dictionaryFile.open(R"(C:\Users\dries\dictionary.txt)");
    if (!dictionaryFile.is_open()) {
        throw std::runtime_error("Failed to open dictionary file.");
    }
    std::string word;
    while (dictionaryFile >> word) {
        words.push_back(word);
    }
}

std::string RandomSentenceService::generateRandomSentence(int wordCount) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, words.size() - 1);

    std::stringstream sentence;
    for (int i = 0; i < wordCount; ++i) {
        if (i > 0) sentence << " ";
        sentence << words[dis(gen)];
    }
    return sentence.str();
}

void RandomSentenceService::processMessages() {
    while (true) {
        std::string receivedMessage = receiveMessage();
        size_t nameStart = receivedMessage.find('<') + 1;
        size_t nameEnd = receivedMessage.find('<', nameStart);
        size_t countEnd = receivedMessage.find('>', nameEnd);
        if (nameEnd == std::string::npos || countEnd == std::string::npos) {
            continue; // malformed message
        }

        std::string userName = receivedMessage.substr(nameStart, nameEnd - nameStart);
        int wordCount = std::stoi(receivedMessage.substr(nameEnd + 1, countEnd - nameEnd - 1));

        if (wordCount <= 0) {
            std::string errorMessage = "error<randomSentence><" + userName + "><Invalid number of words>";
            sendMessage(errorMessage);
            continue;
        }

        std::string generatedSentence = generateRandomSentence(wordCount);
        std::string responseMessage = "response<randomSentence><" + userName + "><" + generatedSentence + ">";
        sendMessage(responseMessage);
    }
}
