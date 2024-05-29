#ifndef RANDOMSENTENCESERVICE_H
#define RANDOMSENTENCESERVICE_H

#include <zmq.hpp>
#include <string>
#include <fstream>
#include <vector>
#include <random>
#include <unordered_map>
#include <set>

/**********************************
*  Random Sentence Service Class Declaration
**********************************/
class RandomSentenceService {
public:
    explicit RandomSentenceService(const std::string& dictPath);
    ~RandomSentenceService();

    // Process incoming messages and generate random sentences
    void processMessages();

private:
    zmq::context_t context;
    zmq::socket_t subscriber;
    zmq::socket_t responder;
    std::ifstream dictionaryFile;
    std::vector<std::string> words;

    int interactionCount;
    std::set<std::string> userNames;

    // Generate a random sentence with the specified word count
    std::string generateRandomSentence(int wordCount);

    // Log interactions
    void logInteraction(const std::string& userName);
};

#endif // RANDOMSENTENCESERVICE_H
