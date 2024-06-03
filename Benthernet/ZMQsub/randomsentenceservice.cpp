#include "randomsentenceservice.h"
#include "logger.h"
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <iterator>
#include <algorithm>

RandomSentenceService::RandomSentenceService(const std::string& dictPath)
    : context(1), subscriber(context, ZMQ_SUB), responder(context, ZMQ_PUSH), interactionCount(0),
    passwordHandler("C:/Users/dries/OneDrive/Desktop/git/Pxl_Benternet/Benthernet/user_data.txt") {
    dictionaryFile.open(dictPath);
    if (!dictionaryFile.is_open()) {
        throw std::runtime_error("Failed to open dictionary file.");
    }

    std::string word;
    while (dictionaryFile >> word) {
        words.push_back(word);
    }

    subscriber.connect("tcp://benternet.pxl-ea-ict.be:24042");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "randomsentence<", 15);
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "register<", 9);
    responder.connect("tcp://benternet.pxl-ea-ict.be:24041");
    Logger::log(Logger::Level::INFO, "Subscribed to topic: randomsentence< and register<");
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

void RandomSentenceService::logInteraction(const std::string& userName) {
    interactionCount++;
    userNames.insert(userName);
    Logger::log(Logger::Level::INFO, "Total interactions: " + std::to_string(interactionCount));
    Logger::log(Logger::Level::INFO, "Users interacted: " + std::to_string(userNames.size()));
}

void RandomSentenceService::processMessages() {
    while (true) {
        try {
            zmq::message_t message;
            Logger::log(Logger::Level::INFO, "Waiting to receive message...");
            subscriber.recv(message);
            std::string receivedMessage(static_cast<char*>(message.data()), message.size());
            Logger::log(Logger::Level::INFO, "Received message: " + receivedMessage);

            if (receivedMessage.find("register<") == 0) {
                size_t nameStart = 9;
                size_t nameEnd = receivedMessage.find('<', nameStart);
                size_t passEnd = receivedMessage.find('>', nameEnd + 1);

                if (nameEnd == std::string::npos || passEnd == std::string::npos) {
                    Logger::log(Logger::Level::ERROR, "Malformed registration message received: " + receivedMessage);
                    continue;
                }

                std::string userName = receivedMessage.substr(nameStart, nameEnd - nameStart);
                std::string password = receivedMessage.substr(nameEnd + 1, passEnd - nameEnd - 1);

                if (passwordHandler.registerUser(userName, password)) {
                    Logger::log(Logger::Level::INFO, "User registered: " + userName);
                    std::string responseMessage = "response<register<" + userName + "<Registration successful>";
                    responder.send(zmq::buffer(responseMessage), zmq::send_flags::none);
                } else {
                    Logger::log(Logger::Level::ERROR, "User already exists: " + userName);
                    std::string responseMessage = "response<register<" + userName + "<User already exists>";
                    responder.send(zmq::buffer(responseMessage), zmq::send_flags::none);
                }
                continue;
            }

            if (receivedMessage.find("randomsentence<") == 0) {
                size_t nameStart = 15;
                size_t nameEnd = receivedMessage.find('<', nameStart);
                size_t passEnd = receivedMessage.find('<', nameEnd + 1);
                size_t countEnd = receivedMessage.find('>', passEnd + 1);

                if (nameEnd == std::string::npos || passEnd == std::string::npos || countEnd == std::string::npos) {
                    Logger::log(Logger::Level::ERROR, "Malformed message received: " + receivedMessage);
                    continue;
                }

                std::string userName = receivedMessage.substr(nameStart, nameEnd - nameStart);
                std::string password = receivedMessage.substr(nameEnd + 1, passEnd - nameEnd - 1);
                std::string wordCountStr = receivedMessage.substr(passEnd + 1, countEnd - passEnd - 1);

                if (!passwordHandler.validateUser(userName, password)) {
                    Logger::log(Logger::Level::ERROR, "Invalid credentials for user: " + userName);
                    std::string errorMessage = "response<randomsentence<" + userName + "<Invalid credentials>";
                    responder.send(zmq::buffer(errorMessage), zmq::send_flags::none);
                    continue;
                }

                logInteraction(userName);

                if (wordCountStr == "-help") {
                    std::string helpMessage =
                        "response<randomsentence<" + userName + "<Random Sentence Service:\n"
                                                                "This service generates a random sentence of the specified word count.\n"
                                                                "Usage:\n"
                                                                "    randomsentence<username<password<word_count>\n"
                                                                "Example:\n"
                                                                "    randomsentence<JaneDoe<password<5>\n"
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
            }
        } catch (const std::exception& ex) {
            Logger::log(Logger::Level::ERROR, std::string("Exception in processing message: ") + ex.what());
        }
    }
}
