#ifndef RANDOMSENTENCESERVICE_H
#define RANDOMSENTENCESERVICE_H

#include <zmq.hpp>
#include <string>
#include <fstream>
#include <vector>
#include <random>

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

    std::string generateRandomSentence(int wordCount);
};

#endif // RANDOMSENTENCESERVICE_H
