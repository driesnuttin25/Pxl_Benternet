#ifndef PASSWORDHANDLER_H
#define PASSWORDHANDLER_H

#include <string>
#include <unordered_map>

class PasswordHandler {
public:
    explicit PasswordHandler(const std::string& filePath);
    ~PasswordHandler();

    // Register a new user with username and password
    bool registerUser(const std::string& username, const std::string& password);

    // Check if a user exists with the given username and password
    bool validateUser(const std::string& username, const std::string& password);

private:
    std::string filePath;
    std::unordered_map<std::string, std::string> users; // Store usernames and passwords in memory

    // Load users from file
    void loadUsers();

    // Save users to file
    void saveUsers();
};

#endif // PASSWORDHANDLER_H
