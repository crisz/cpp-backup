#pragma once

#include <string>
#include <vector>

class CommandDTO {
private:
    std::string command_name;
    std::vector<std::pair<std::string, std::string>> parameters;
public:
    void insert(std::pair<std::string, std::string> parameter) {
        parameters.push_back(parameter);
    }

    void insert(std::string key, std::string value) {
        this->insert(std::make_pair(key, value));
    }


    std::pair<std::string, std::string> find(std::string key) {
        for (auto el: parameters) {
            if (el.first == key) return el;
        }
        return std::pair<std::string, std::string>{};
    }

    auto begin() const {
        return parameters.begin();
    }

    auto end() const {
        return parameters.end();
    }

    void erase() {
        parameters.clear();
        command_name.clear();
    }

    size_t size() {
        return parameters.size();
    }

};