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

    // TODO: farlo diventare uno smart pointer
    std::pair<std::string, std::string> find(std::string key) { // TODO: ottimizzare passaggio per copia/rif. in tutta la classe
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