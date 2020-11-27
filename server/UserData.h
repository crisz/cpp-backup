#pragma once
#include <string>

struct UserData {
    std::string username;
    std::function<void(const std::string)> send_response_callback;
    std::function<void(const char*, int)> send_raw_response_callback;
};