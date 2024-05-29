#include "spellcheckerservice.h"
#include "logger.h"
#include <sstream>
#include <limits>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <algorithm>

/**********************************
*  Spell Checker Service Class
**********************************/

// Constructor: Initialize ZMQ context and sockets, load dictionary
SpellCheckerService::SpellCheckerService(const std::string& dictPath)
    : context(1), subscriber(context, ZMQ_SUB), responder(context, ZMQ_PUSH), interactionCount(0) {
    dictionaryFile.open(dictPath);
    if (!dictionaryFile.is_open()) {
        throw std::runtime_error("Failed to open dictionary file.");
    }
    subscriber.connect("tcp://benternet.pxl-ea-ict.be:24042");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "spellingschecker<", 17);
    responder.connect("tcp://benternet.pxl-ea-ict.be:24041");
    Logger::log(Logger::Level::INFO, "Subscribed to topic: spellingschecker<");
}

// Destructor: Close the dictionary file if open
SpellCheckerService::~SpellCheckerService() {
    if (dictionaryFile.is_open()) {
        dictionaryFile.close();
    }
}

// Calculate the Levenshtein distance between two strings
int SpellCheckerService::levenshteinDP(const std::string& s1, const std::string& s2) {
    int len1 = s1.size(), len2 = s2.size();
    std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1));

    for (int i = 0; i <= len1; ++i) dp[i][0] = i;
    for (int j = 0; j <= len2; ++j) dp[0][j] = j;

    for (int i = 1; i <= len1; ++i) {
        for (int j = 1; j <= len2; ++j) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({ dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + cost });
        }
    }

    return dp[len1][len2];
}

// Find the closest word in the dictionary to the input word
std::string SpellCheckerService::findClosestWord(const std::string& inputWord) {
    dictionaryFile.clear();
    dictionaryFile.seekg(0, std::ios::beg);

    std::string dictionaryWord;
    std::vector<std::string> closestWords;
    int minDistance = std::numeric_limits<int>::max();

    while (std::getline(dictionaryFile, dictionaryWord)) {
        int lengthDifference = abs(static_cast<int>(inputWord.length()) - static_cast<int>(dictionaryWord.length()));
        if (lengthDifference <= 2) {
            int distance = levenshteinDP(inputWord, dictionaryWord);
            if (distance < minDistance) {
                minDistance = distance;
                closestWords.clear();
                closestWords.push_back(dictionaryWord);
            } else if (distance == minDistance) {
                closestWords.push_back(dictionaryWord);
            }
        }
    }
    return closestWords.empty() ? inputWord : closestWords.front();
}

// Log interactions
void SpellCheckerService::logInteraction(const std::string& userName) {
    interactionCount++;
    userNames.insert(userName);
    Logger::log(Logger::Level::INFO, "Total interactions: " + std::to_string(interactionCount));
    Logger::log(Logger::Level::INFO, "Users interacted: " + std::to_string(userNames.size()));
}

// Process incoming messages and generate appropriate responses
void SpellCheckerService::processMessages() {
    while (true) {
        try {
            zmq::message_t message;
            Logger::log(Logger::Level::INFO, "Waiting to receive message...");
            subscriber.recv(message);
            std::string receivedMessage(static_cast<char*>(message.data()), message.size());
            Logger::log(Logger::Level::INFO, "Received message: " + receivedMessage);

            // Check if the message is for this service
            if (receivedMessage.find("spellingschecker<") != 0) {
                Logger::log(Logger::Level::WARNING, "Ignoring unrelated message: " + receivedMessage);
                continue;
            }

            // Check if the message is a response and ignore it
            if (receivedMessage.find("response<") == 0) {
                Logger::log(Logger::Level::INFO, "Ignoring response message: " + receivedMessage);
                continue;
            }

            size_t nameStart = 17;  // Length of "spellingschecker<"
            size_t nameEnd = receivedMessage.find('<', nameStart);
            size_t sentenceEnd = receivedMessage.find('>', nameEnd);
            if (nameEnd == std::string::npos || sentenceEnd == std::string::npos) {
                Logger::log(Logger::Level::ERROR, "Malformed message received: " + receivedMessage);
                continue;
            }

            std::string userName = receivedMessage.substr(nameStart, nameEnd - nameStart);
            std::string sentence = receivedMessage.substr(nameEnd + 1, sentenceEnd - nameEnd - 1);

            logInteraction(userName);

            // Handle help request
            if (sentence == "-help") {
                std::string helpMessage =
                    "response<correctspelling<" + userName + "<Spell Checker Service:\n"
                                                             "This service checks and corrects the spelling of a sentence.\n"
                                                             "Usage:\n"
                                                             "    spellingschecker<username<sentence>\n"
                                                             "Example:\n"
                                                             "    spellingschecker<JohnDoe<Thiss is an exmple sentence>\n"
                                                             "Notable exceptions:\n"
                                                             "    - Too many requests: You can only make a limited number of requests within a given time period.\n"
                                                             "    - Malformed message: Ensure your message follows the correct format.>";
                responder.send(zmq::buffer(helpMessage), zmq::send_flags::none);
                continue;
            }

            // Rate limit check
            auto now = std::chrono::system_clock::now();
            auto& userInfo = userLimits[userName];

            if (now > userInfo.resetTime) {
                userInfo.resetTime = now + std::chrono::minutes(2);
                userInfo.requestCount = 0;
            }
            // Ok so this limits the user to 5 requests every 2 minutes
            // If they go over that... Too bad, gotta wait
            if (userInfo.requestCount >= 5) {
                auto remaining_time = std::chrono::duration_cast<std::chrono::seconds>(userInfo.resetTime - now).count();
                std::string errorMessage = "response<correctspelling<" + userName + "<Too many requests. Try again in " + std::to_string(remaining_time) + " seconds.>";
                responder.send(zmq::buffer(errorMessage), zmq::send_flags::none);
                // Quite cool no? It even sends how much time is left with std::to_string(remaining_time) hehehe
                Logger::log(Logger::Level::INFO, "Rate limit exceeded for user: " + userName);
                continue;
            }

            userInfo.requestCount++;

            std::istringstream iss(sentence);
            std::string word, correctedSentence, correctedWord;
            bool isFirstWord = true;
            while (iss >> word) {
                correctedWord = findClosestWord(word);
                if (!isFirstWord) correctedSentence += " ";
                correctedSentence += correctedWord;
                isFirstWord = false;
            }

            std::string responseMessage = "response<correctspelling<" + userName + "<" + correctedSentence + ">";
            responder.send(zmq::buffer(responseMessage), zmq::send_flags::none);
        } catch (const std::exception& ex) {
            Logger::log(Logger::Level::ERROR, std::string("Exception in processing message: ") + ex.what());
        }
    }
}
