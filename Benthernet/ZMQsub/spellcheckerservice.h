#ifndef SPELLCHECKERSERVICE_H
#define SPELLCHECKERSERVICE_H

#include "BaseCommunication.h"
#include <fstream>
#include <unordered_map>
#include <chrono>

class SpellCheckerService : public BaseCommunication {
private:
    std::ifstream dictionaryFile;
    std::unordered_map<std::string, std::chrono::system_clock::time_point> userLimits;

    int levenshteinDP(const std::string& s1, const std::string& s2);
    std::string findClosestWord(const std::string& inputWord);

public:
    explicit SpellCheckerService(const std::string& dictPath);
    ~SpellCheckerService();
    void processMessages();
};

#endif // SPELLCHECKERSERVICE_H
