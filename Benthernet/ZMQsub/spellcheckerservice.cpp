#include "SpellCheckerService.h"
#include <sstream>
#include <limits>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <algorithm>

SpellCheckerService::SpellCheckerService(const std::string& dictPath)
    : context(1), subscriber(context, ZMQ_SUB), responder(context, ZMQ_PUSH) {
    dictionaryFile.open(dictPath);
    if (!dictionaryFile.is_open()) {
        throw std::runtime_error("Failed to open dictionary file.");
    }
    subscriber.connect("tcp://benternet.pxl-ea-ict.be:24042");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "spellingschecker<", 17);
    responder.connect("tcp://benternet.pxl-ea-ict.be:24041");
    std::cout << "Subscribed to topic: spellingschecker<" << std::endl;
}

SpellCheckerService::~SpellCheckerService() {
    if (dictionaryFile.is_open()) {
        dictionaryFile.close();
    }
}

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

void SpellCheckerService::processMessages() {
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

            size_t nameStart = 17;  // Length of "spellingschecker<"
            size_t nameEnd = receivedMessage.find('<', nameStart);
            size_t sentenceEnd = receivedMessage.find('>', nameEnd);
            if (nameEnd == std::string::npos || sentenceEnd == std::string::npos) {
                std::cerr << "Malformed message received: " << receivedMessage << std::endl;
                continue;
            }

            std::string userName = receivedMessage.substr(nameStart, nameEnd - nameStart);
            std::string sentence = receivedMessage.substr(nameEnd + 1, sentenceEnd - nameEnd - 1);

            std::cout << "Parsed userName: " << userName << std::endl;
            std::cout << "Parsed sentence: " << sentence << std::endl;

            // Rate limit check
            auto now = std::chrono::system_clock::now();
            auto& resetTime = userLimits[userName];

            if (now > resetTime) {
                resetTime = now + std::chrono::minutes(2);
            } else {
                std::string errorMessage = "response<correctspelling<" + userName + "<Too many requests. Try again later.>";
                responder.send(zmq::buffer(errorMessage), zmq::send_flags::none);
                std::cout << "Rate limit exceeded for user: " << userName << std::endl;
                continue;
            }

            std::istringstream iss(sentence);
            std::string word, correctedSentence, correctedWord;
            bool isFirstWord = true;
            while (iss >> word) {
                std::cout << "Processing word: " << word << std::endl;
                correctedWord = findClosestWord(word);
                std::cout << "Corrected word: " << correctedWord << std::endl;
                if (!isFirstWord) correctedSentence += " ";
                correctedSentence += correctedWord;
                isFirstWord = false;
            }

            std::string responseMessage = "response<correctspelling<" + userName + "<" + correctedSentence + ">";
            std::cout << "Sending response: " << responseMessage << std::endl;
            responder.send(zmq::buffer(responseMessage), zmq::send_flags::none);
        } catch (const std::exception& ex) {
            std::cerr << "Exception in processing message: " << ex.what() << std::endl;
        }
    }
}
