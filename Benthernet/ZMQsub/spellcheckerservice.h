#ifndef SPELLCHECKERSERVICE_H
#define SPELLCHECKERSERVICE_H

#include <zmq.hpp>
#include <string>
#include <fstream>
#include <unordered_map>
#include <chrono>
#include <set>
#include "passwordhandler.h"

struct UserInfo {
    int requestCount = 0;
    std::chrono::system_clock::time_point resetTime;
};

class SpellCheckerService {
public:
    explicit SpellCheckerService(const std::string& dictPath);
    ~SpellCheckerService();
    void processMessages();

private:
    zmq::context_t context;
    zmq::socket_t subscriber;
    zmq::socket_t responder;
    std::ifstream dictionaryFile;
    std::unordered_map<std::string, UserInfo> userLimits;
    int interactionCount;
    std::set<std::string> userNames;
    PasswordHandler passwordHandler; // Add this line

    int levenshteinDP(const std::string& s1, const std::string& s2);
    std::string findClosestWord(const std::string& inputWord);
    void logInteraction(const std::string& userName);
};

#endif // SPELLCHECKERSERVICE_H
