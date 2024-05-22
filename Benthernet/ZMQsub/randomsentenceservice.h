#ifndef RANDOMSENTENCESERVICE_H
#define RANDOMSENTENCESERVICE_H

#include "BaseCommunication.h"
#include <fstream>
#include <vector>
#include <random>

class RandomSentenceService : public BaseCommunication {
private:
    std::ifstream dictionaryFile;
    std::vector<std::string> words;

public:
    explicit RandomSentenceService(const std::string& dictPath);
    std::string generateRandomSentence(int wordCount);
    void processMessages();
};

#endif // RANDOMSENTENCESERVICE_H
