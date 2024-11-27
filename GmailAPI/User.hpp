#ifndef USER_HPP
#define USER_HPP

#include <iostream>

class User {
private:
    std::string name;
    std::string email;
    std::string subject;
    std::string body;
public:
    User(const std::string& name, const std::string& email, const std::string& subject, const std::string& body)
    : name(name), email(email), subject(subject), body(body) {}
    
    friend class GmailAPI;
};

#endif