#include "randomsentenceservice.h"
#include "logger.h"
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <iterator>
#include <algorithm>

/**********************************
*  Random Sentence Service Class
**********************************/

RandomSentenceService::RandomSentenceService(const std::string& dictPath)
    : context(1), subscriber(context, ZMQ_SUB), responder(context, ZMQ_PUSH), interactionCount(0) {
    dictionaryFile.open(dictPath);
    if (!dictionaryFile.is_open()) {
        throw std::runtime_error("Failed to open dictionary file.");
    }

    // Load the dictionary
    std::string word;
    while (dictionaryFile >> word) {
        words.push_back(word);
    }

    subscriber.connect("tcp://benternet.pxl-ea-ict.be:24042");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "randomsentence<", 15);
    responder.connect("tcp://benternet.pxl-ea-ict.be:24041");
    Logger::log(Logger::Level::INFO, "Subscribed to topic: randomsentence<");
}

// Destructor: Close the dictionary file if open
RandomSentenceService::~RandomSentenceService() {
    if (dictionaryFile.is_open()) {
        dictionaryFile.close();
    }
}

// This generates a random sentence with the specified word count, looks fancy right, it's not...?
std::string RandomSentenceService::generateRandomSentence(int wordCount) {
    std::random_device rd;
    // Look at me actually trying to use good randomeness
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, words.size() - 1);

    std::stringstream sentence;
    for (int i = 0; i < wordCount; ++i) {
        if (i > 0) sentence << " ";
        sentence << words[dis(gen)];
    }
    return sentence.str();
}

// Log interactions
void RandomSentenceService::logInteraction(const std::string& userName) {
    interactionCount++;
    userNames.insert(userName);
    Logger::log(Logger::Level::INFO, "Total interactions: " + std::to_string(interactionCount));
    Logger::log(Logger::Level::INFO, "Users interacted: " + std::to_string(userNames.size()));
}

// Process incoming messages and generate appropriate responses
void RandomSentenceService::processMessages() {
    while (true) {
        try {
            zmq::message_t message;
            Logger::log(Logger::Level::INFO, "Waiting to receive message...");
            subscriber.recv(message);
            std::string receivedMessage(static_cast<char*>(message.data()), message.size());
            Logger::log(Logger::Level::INFO, "Received message: " + receivedMessage);

            // Check if the message is for this service
            if (receivedMessage.find("randomsentence<") != 0) {
                Logger::log(Logger::Level::WARNING, "Ignoring unrelated message: " + receivedMessage);
                continue;
            }

            // Check if the message is a response and ignore it obviously...
            if (receivedMessage.find("response<") == 0) {
                Logger::log(Logger::Level::INFO, "Ignoring response message: " + receivedMessage);
                continue;
            }

            size_t nameStart = 15;  // Length of "randomsentence<" I know this is lazy but shhhh it's the easiest way
            size_t nameEnd = receivedMessage.find('<', nameStart);
            size_t countEnd = receivedMessage.find('>', nameEnd);
            if (nameEnd == std::string::npos || countEnd == std::string::npos) {
                Logger::log(Logger::Level::ERROR, "Malformed message received: " + receivedMessage);
                continue;
            }

            std::string userName = receivedMessage.substr(nameStart, nameEnd - nameStart);
            std::string wordCountStr = receivedMessage.substr(nameEnd + 1, countEnd - nameEnd - 1);

            logInteraction(userName);

            // Handle help request cause God knows we'll need some help with code like this.
            if (wordCountStr == "-help") {
                std::string helpMessage =
                    "response<randomsentence<" + userName + "<Random Sentence Service:\n"
                                                            "This service generates a random sentence of the specified word count.\n"
                                                            "Usage:\n"
                                                            "    randomsentence<username<word_count>\n"
                                                            "Example:\n"
                                                            "    randomsentence<JaneDoe<5>\n"
                                                            "Notable exceptions:\n"
                                                            "    - Invalid number of words: Ensure the word count is a positive integer.\n"
                                                            "    - Malformed message: Ensure your message follows the correct format.>";
                responder.send(zmq::buffer(helpMessage), zmq::send_flags::none);
                continue;
            }

            int wordCount;
            try {
                wordCount = std::stoi(wordCountStr);
            } catch (const std::invalid_argument& e) {
                Logger::log(Logger::Level::ERROR, "Invalid number format for word count: " + wordCountStr);
                std::string errorMessage = "response<randomsentence<" + userName + "<Invalid number of words>";
                responder.send(zmq::buffer(errorMessage), zmq::send_flags::none);
                continue;
            } catch (const std::out_of_range& e) {
                Logger::log(Logger::Level::ERROR, "Number out of range for word count: " + wordCountStr);
                std::string errorMessage = "response<randomsentence<" + userName + "<Number of words out of range>";
                responder.send(zmq::buffer(errorMessage), zmq::send_flags::none);
                continue;
            }

            if (wordCount <= 0) {
                std::string errorMessage = "response<randomsentence<" + userName + "<Invalid number of words>";
                responder.send(zmq::buffer(errorMessage), zmq::send_flags::none);
                Logger::log(Logger::Level::INFO, "Invalid word count for user: " + userName);
                continue;
            }

            std::string generatedSentence = generateRandomSentence(wordCount);
            std::string responseMessage = "response<randomsentence<" + userName + "<" + generatedSentence + ">";
            Logger::log(Logger::Level::INFO, "Sending response: " + responseMessage);
            responder.send(zmq::buffer(responseMessage), zmq::send_flags::none);
        } catch (const std::exception& ex) {
            Logger::log(Logger::Level::ERROR, std::string("Exception in processing message: ") + ex.what());
        }
    }
}
