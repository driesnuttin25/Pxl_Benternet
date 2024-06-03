#include "spellcheckerservice.h"
#include "logger.h"
#include <sstream>
#include <limits>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <algorithm>

SpellCheckerService::SpellCheckerService(const std::string& dictPath)
    : context(1), subscriber(context, ZMQ_SUB), responder(context, ZMQ_PUSH), interactionCount(0),
    passwordHandler("C:/Users/dries/OneDrive/Desktop/git/Pxl_Benternet/Benthernet/user_data.txt") {
    dictionaryFile.open(dictPath);
    if (!dictionaryFile.is_open()) {
        throw std::runtime_error("Failed to open dictionary file.");
    }
    subscriber.connect("tcp://benternet.pxl-ea-ict.be:24042");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "spellingschecker<", 17);
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "register<", 9);
    responder.connect("tcp://benternet.pxl-ea-ict.be:24041");
    Logger::log(Logger::Level::INFO, "Subscribed to topic: spellingschecker< and register<");
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

void SpellCheckerService::logInteraction(const std::string& userName) {
    interactionCount++;
    userNames.insert(userName);
    Logger::log(Logger::Level::INFO, "Total interactions: " + std::to_string(interactionCount));
    Logger::log(Logger::Level::INFO, "Users interacted: " + std::to_string(userNames.size()));
}

void SpellCheckerService::processMessages() {
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

            if (receivedMessage.find("spellingschecker<") == 0) {
                size_t nameStart = 17;
                size_t nameEnd = receivedMessage.find('<', nameStart);
                size_t passEnd = receivedMessage.find('<', nameEnd + 1);
                size_t limitEnd = receivedMessage.find('<', passEnd + 1);
                size_t sentenceEnd = receivedMessage.find('>', limitEnd + 1);

                if (nameEnd == std::string::npos || passEnd == std::string::npos || limitEnd == std::string::npos || sentenceEnd == std::string::npos) {
                    Logger::log(Logger::Level::ERROR, "Malformed message received: " + receivedMessage);
                    continue;
                }

                std::string userName = receivedMessage.substr(nameStart, nameEnd - nameStart);
                std::string password = receivedMessage.substr(nameEnd + 1, passEnd - nameEnd - 1);
                std::string limitStr = receivedMessage.substr(passEnd + 1, limitEnd - passEnd - 1);
                std::string sentence = receivedMessage.substr(limitEnd + 1, sentenceEnd - limitEnd - 1);

                if (!passwordHandler.validateUser(userName, password)) {
                    Logger::log(Logger::Level::ERROR, "Invalid credentials for user: " + userName);
                    std::string errorMessage = "response<spellingschecker<" + userName + "<Invalid credentials>";
                    responder.send(zmq::buffer(errorMessage), zmq::send_flags::none);
                    continue;
                }

                logInteraction(userName);

                if (limitStr == "-help") {
                    std::string helpMessage =
                        "response<spellingschecker<" + userName + "<Spell Checker Service:\n"
                                                                  "This service checks and corrects the spelling of a sentence.\n"
                                                                  "Usage:\n"
                                                                  "    spellingschecker<username<password<limit<sentence>\n"
                                                                  "Example:\n"
                                                                  "    spellingschecker<JohnDoe<password<1<Thiss is an exmple sentence>\n"
                                                                  "Notable exceptions:\n"
                                                                  "    - Too many requests: You can only make a limited number of requests within a given time period.\n"
                                                                  "    - Malformed message: Ensure your message follows the correct format.>";
                    responder.send(zmq::buffer(helpMessage), zmq::send_flags::none);
                    continue;
                }

                auto now = std::chrono::system_clock::now();
                auto& userInfo = userLimits[userName];

                if (now > userInfo.resetTime) {
                    userInfo.resetTime = now + std::chrono::minutes(2);
                    userInfo.requestCount = 0;
                }

                if (userInfo.requestCount >= 5) {
                    auto remaining_time = std::chrono::duration_cast<std::chrono::seconds>(userInfo.resetTime - now).count();
                    std::string errorMessage = "response<spellingschecker<" + userName + "<Too many requests. Try again in " + std::to_string(remaining_time) + " seconds.>";
                    responder.send(zmq::buffer(errorMessage), zmq::send_flags::none);
                    Logger::log(Logger::Level::INFO, "Rate limit exceeded for user: " + userName);
                    continue;
                }

                userInfo.requestCount++;

                int limit;
                try {
                    limit = std::stoi(limitStr);
                } catch (const std::invalid_argument& e) {
                    Logger::log(Logger::Level::ERROR, "Invalid number format for limit: " + limitStr);
                    std::string errorMessage = "response<spellingschecker<" + userName + "<Invalid limit>";
                    responder.send(zmq::buffer(errorMessage), zmq::send_flags::none);
                    continue;
                } catch (const std::out_of_range& e) {
                    Logger::log(Logger::Level::ERROR, "Number out of range for limit: " + limitStr);
                    std::string errorMessage = "response<spellingschecker<" + userName + "<Limit out of range>";
                    responder.send(zmq::buffer(errorMessage), zmq::send_flags::none);
                    continue;
                }

                std::istringstream iss(sentence);
                std::string word, correctedSentence;
                bool isFirstWord = true;
                int correctedCount = 0;

                while (iss >> word) {
                    std::string correctedWord = word;
                    if (correctedCount < limit) {
                        correctedWord = findClosestWord(word);
                        if (correctedWord != word) {
                            correctedCount++;
                        }
                    }

                    if (!isFirstWord) correctedSentence += " ";
                    correctedSentence += correctedWord;
                    isFirstWord = false;
                }

                std::string responseMessage = "response<spellingschecker<" + userName + "<" + std::to_string(limit) + "<" + correctedSentence + ">";
                responder.send(zmq::buffer(responseMessage), zmq::send_flags::none);
            }
        } catch (const std::exception& ex) {
            Logger::log(Logger::Level::ERROR, std::string("Exception in processing message: ") + ex.what());
        }
    }
}

