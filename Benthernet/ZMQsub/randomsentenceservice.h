#ifndef RANDOMSENTENCESERVICE_H
#define RANDOMSENTENCESERVICE_H

#include <zmq.hpp>
#include <string>
#include <fstream>
#include <vector>
#include <random>
#include <unordered_map>
#include <set>
#include "passwordhandler.h"

class RandomSentenceService {
public:
    explicit RandomSentenceService(const std::string& dictPath);
    ~RandomSentenceService();

    void processMessages();

private:
    zmq::context_t context;
    zmq::socket_t subscriber;
    zmq::socket_t responder;
    std::ifstream dictionaryFile;
    std::vector<std::string> words;
    int interactionCount;
    std::set<std::string> userNames;
    PasswordHandler passwordHandler; // Add this line

    std::string generateRandomSentence(int wordCount);
    void logInteraction(const std::string& userName);
};

#endif // RANDOMSENTENCESERVICE_H
