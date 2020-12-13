#pragma once
#include <string>
#include <iostream>

struct UserData {
    std::string username;
    std::function<void(const std::string)> send_response_callback;
    std::function<void(const char*, int)> send_raw_response_callback;

    void print_user_log(std::string log) {
        std::cout << this->username << " ~~ " << log << std::endl;
    }
};
