#pragma once
#include <string>
struct UserSession {
    std::string username;
    std::string password;
    int port;
    std::string address;
    std::string dir;
};