#include "SpellCheckerService.h"
#include <sstream>
#include <limits>

SpellCheckerService::SpellCheckerService(const std::string& dictPath) : BaseCommunication(zmq::socket_type::sub) {
    dictionaryFile.open(R"(C:\Users\dries\dictionary.txt)");
    if (!dictionaryFile.is_open()) {
        throw std::runtime_error("Failed to open dictionary file.");
    }
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
        std::string receivedMessage = receiveMessage();
        size_t nameStart = receivedMessage.find('<') + 1;
        size_t nameEnd = receivedMessage.find('<', nameStart);
        size_t sentenceEnd = receivedMessage.find('>', nameEnd);
        if (nameEnd == std::string::npos || sentenceEnd == std::string::npos) {
            continue; // malformed message
        }

        std::string userName = receivedMessage.substr(nameStart, nameEnd - nameStart);
        std::string sentence = receivedMessage.substr(nameEnd + 1, sentenceEnd - nameEnd - 1);

        // Rate limit check
        auto now = std::chrono::system_clock::now();
        auto& resetTime = userLimits[userName];

        if (now > resetTime) {
            resetTime = now + std::chrono::minutes(2);
        } else {
            std::string errorMessage = "error<spellChecker><" + userName + "><Too many requests. Try again later.>";
            sendMessage(errorMessage);
            continue;
        }

        std::istringstream iss(sentence);
        std::string word, correctedSentence, correctedWord;
        bool isFirstWord = true;
        while (iss >> word) {
            correctedWord = findClosestWord(word);
            if (!isFirstWord) correctedSentence += " ";
            correctedSentence += correctedWord;
            isFirstWord = false;
        }

        std::string responseMessage = "response<spellChecker><" + userName + "><" + correctedSentence + ">";
        sendMessage(responseMessage);
    }
}
