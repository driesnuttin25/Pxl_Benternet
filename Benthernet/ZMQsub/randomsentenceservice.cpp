#include "randomsentenceservice.h"
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <iterator>
#include <algorithm>

RandomSentenceService::RandomSentenceService(const std::string& dictPath)
    : context(1), subscriber(context, ZMQ_SUB), responder(context, ZMQ_PUSH) {
    dictionaryFile.open(dictPath);
    if (!dictionaryFile.is_open()) {
        throw std::runtime_error("Failed to open dictionary file.");
    }

    // Load dictionary words
    std::string word;
    while (dictionaryFile >> word) {
        words.push_back(word);
    }

    subscriber.connect("tcp://benternet.pxl-ea-ict.be:24042");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "randomsentence<", 15);
    responder.connect("tcp://benternet.pxl-ea-ict.be:24041");
    std::cout << "Subscribed to topic: randomsentence<" << std::endl;
}

RandomSentenceService::~RandomSentenceService() {
    if (dictionaryFile.is_open()) {
        dictionaryFile.close();
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
        try {
            zmq::message_t message;
            std::cout << "Waiting to receive message..." << std::endl;
            subscriber.recv(message);
            std::string receivedMessage(static_cast<char*>(message.data()), message.size());
            std::cout << "Received message: " << receivedMessage << std::endl;

            // Check if the message is a response and ignore it
            if (receivedMessage.find("response<") == 0) {
                std::cout << "Ignoring response message: " << receivedMessage << std::endl;
                continue;
            }

            size_t nameStart = 15;  // Length of "randomsentence<"
            size_t nameEnd = receivedMessage.find('<', nameStart);
            size_t countEnd = receivedMessage.find('>', nameEnd);
            std::cout << "nameStart: " << nameStart << ", nameEnd: " << nameEnd << ", countEnd: " << countEnd << std::endl;

            if (nameEnd == std::string::npos || countEnd == std::string::npos) {
                std::cerr << "Malformed message received: " << receivedMessage << std::endl;
                continue;
            }

            std::string userName = receivedMessage.substr(nameStart, nameEnd - nameStart);
            std::string wordCountStr = receivedMessage.substr(nameEnd + 1, countEnd - nameEnd - 1);

            // Handle help request
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
                std::cerr << "Invalid number format for word count: " << wordCountStr << std::endl;
                std::string errorMessage = "response<randomsentence<" + userName + "<Invalid number of words>";
                responder.send(zmq::buffer(errorMessage), zmq::send_flags::none);
                continue;
            } catch (const std::out_of_range& e) {
                std::cerr << "Number out of range for word count: " << wordCountStr << std::endl;
                std::string errorMessage = "response<randomsentence<" + userName + "<Number of words out of range>";
                responder.send(zmq::buffer(errorMessage), zmq::send_flags::none);
                continue;
            }

            if (wordCount <= 0) {
                std::string errorMessage = "response<randomsentence<" + userName + "<Invalid number of words>";
                responder.send(zmq::buffer(errorMessage), zmq::send_flags::none);
                std::cout << "Invalid word count for user: " << userName << std::endl;
                continue;
            }

            std::string generatedSentence = generateRandomSentence(wordCount);
            std::string responseMessage = "response<randomsentence<" + userName + "<" + generatedSentence + ">";
            std::cout << "Sending response: " << responseMessage << std::endl;
            responder.send(zmq::buffer(responseMessage), zmq::send_flags::none);
        } catch (const std::exception& ex) {
            std::cerr << "Exception in processing message: " << ex.what() << std::endl;
        }
    }
}

