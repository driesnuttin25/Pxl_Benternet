#include "passwordhandler.h"
#include <fstream>
#include <sstream>

PasswordHandler::PasswordHandler(const std::string& filePath) : filePath(filePath) {
    loadUsers();
}

PasswordHandler::~PasswordHandler() {
    saveUsers();
}

void PasswordHandler::loadUsers() {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string username, password;
        if (std::getline(iss, username, '<') && std::getline(iss, password)) {
            users[username] = password;
        }
    }
}

void PasswordHandler::saveUsers() {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return;
    }

    for (const auto& [username, password] : users) {
        file << username << '<' << password << '\n';
    }
}

bool PasswordHandler::registerUser(const std::string& username, const std::string& password) {
    if (users.find(username) != users.end()) {
        return false;
    }
    users[username] = password;
    saveUsers();
    return true;
}

bool PasswordHandler::validateUser(const std::string& username, const std::string& password) {
    auto it = users.find(username);
    return it != users.end() && it->second == password;
}
