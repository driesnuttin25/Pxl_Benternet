#ifndef SPELLCHECKERSERVICE_H
#define SPELLCHECKERSERVICE_H

#include <zmq.hpp>
#include <string>
#include <fstream>
#include <unordered_map>
#include <chrono>
#include <set>

/**********************************
*  Spell Checker Service Class Declaration
**********************************/

// Structure to store user information for rate limiting
struct UserInfo {
    int requestCount = 0;
    std::chrono::system_clock::time_point resetTime;
};

class SpellCheckerService {
public:
    explicit SpellCheckerService(const std::string& dictPath);
    ~SpellCheckerService();

    // Process incoming messages and correct spellings
    void processMessages();

private:
    zmq::context_t context;
    zmq::socket_t subscriber;
    zmq::socket_t responder;
    std::ifstream dictionaryFile;
    std::unordered_map<std::string, UserInfo> userLimits;

    int interactionCount;
    std::set<std::string> userNames;

    // Calculate the Levenshtein distance between two strings
    int levenshteinDP(const std::string& s1, const std::string& s2);

    // Find the closest word in the dictionary to the input word
    std::string findClosestWord(const std::string& inputWord);

    // Log interactions
    void logInteraction(const std::string& userName);
};

#endif // SPELLCHECKERSERVICE_H
