//
// Classe che contiene il comando e i relativi paramentri da inviare o ricevuti dal server
//

#pragma once

#include <string>
#include <vector>

class CommandDTO {
private:
    std::string command_name;
    std::vector<std::pair<std::string, std::string>> parameters;
public:
    void insert(std::pair<std::string, std::string> parameter);

    void insert(std::string key, std::string value);

    std::pair<std::string, std::string> find(std::string key);

    auto begin() const {
        return parameters.begin();
    }

    auto end() const {
        return parameters.end();
    }
    void erase();

    size_t size();

};