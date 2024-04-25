#include <iostream>
#include <string>
#include <zmq.hpp>
#include <fstream>
#include <vector>
#include <limits>

// Assuming levenshtein and findClosestWord functions are defined here
using namespace std;
int levenshtein(const string &s1, int string_length1, const string &s2, int string_length2);
string findClosestWord(const string &inputWord, ifstream &dictionaryFile);

int main() {
    try {
        zmq::context_t context(1);

        zmq::socket_t subscriber(context, ZMQ_SUB);
        subscriber.connect("tcp://benternet.pxl-ea-ict.be:24042");
        string subscribeTopic = "dries>spelling>";
        subscriber.setsockopt(ZMQ_SUBSCRIBE, subscribeTopic.c_str(), subscribeTopic.length());
        zmq::socket_t responder(context, ZMQ_PUSH);
        responder.connect("tcp://benternet.pxl-ea-ict.be:24041");

        ifstream dictionaryFile("C:/Users/dries/OneDrive/Desktop/git/Levenshtein-distance/Benthernet/dictionary.txt");

        if (!dictionaryFile.is_open()) {
            cerr << "Failed to open dictionary file." << endl;
            return 1;
        }

        zmq::message_t message;
        while(true) {
            subscriber.recv(&message);
            string receivedMessage(static_cast<char*>(message.data()), message.size());
            cout << "Received: [" << receivedMessage << "]" << endl;

            string sentence = receivedMessage.substr(subscribeTopic.length(), receivedMessage.find_last_of(">") - subscribeTopic.length());

            istringstream iss(sentence);
            string word, correctedSentence, correctedWord;
            bool isFirstWord = true;
            while (iss >> word) {
                correctedWord = findClosestWord(word, dictionaryFile);
                if (!isFirstWord) correctedSentence += " ";
                correctedSentence += correctedWord;
                isFirstWord = false;
            }

            // Send the corrected sentence back
            string response = "dries>correct>" + correctedSentence + ">";
            responder.send(response.c_str(), response.size());
            cout << "Sent [" << response << "]" << endl;
        }
    }
    catch(zmq::error_t& e) {
        cerr << "Caught an exception: " << e.what() << endl;
    }

    return 0;
}

int levenshteinDP(const std::string& s1, const std::string& s2) {
    int len1 = s1.size(), len2 = s2.size();
    std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1));

    for (int i = 0; i <= len1; ++i) {
        dp[i][0] = i;
    }
    for (int j = 0; j <= len2; ++j) {
        dp[0][j] = j;
    }

    for (int i = 1; i <= len1; ++i) {
        for (int j = 1; j <= len2; ++j) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + cost});
        }
    }

    return dp[len1][len2];
}


// Function to find the closest word in the dictionary for a given word
string findClosestWord(const string &inputWord, ifstream &dictionaryFile) {
    dictionaryFile.clear();
    dictionaryFile.seekg(0, ios::beg);

    string dictionaryWord;
    vector<string> closestWords;
    int minDistance = numeric_limits<int>::max();

    while (getline(dictionaryFile, dictionaryWord)) {
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

    // At the moment we're just going to assume that the word with the least Levenshtein distance is the correct word,
    // not very efficient in finding the actual correct word but works well enough for now
    return closestWords.empty() ? inputWord : closestWords.front();
}

