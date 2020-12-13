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

    // Le due funzioni di inser permetto di aggiungere un parametro alla lista di parametri relativa ad un comando
    void insert(std::pair<std::string, std::string> parameter);

    void insert(std::string key, std::string value);

    // Funzione che ritorna il parametro (nome + valore) in base al nome del parametro stesso
    std::pair<std::string, std::string> find(std::string key);

    auto begin() const {
        return parameters.begin();
    }

    auto end() const {
        return parameters.end();
    }

    // Funzione che resetta il nome del comando e la lista di parametri
    void erase();

    // Funzione che ritorna la dimensione della lista di parametri
    size_t size();

};