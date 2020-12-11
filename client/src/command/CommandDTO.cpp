//
// Created by giuseppe on 06/12/20.
//
// Classe che contiene il comando e i relativi paramentri da inviare o ricevuti dal server
//

#include "CommandDTO.h"

// Le due funzioni di inser permetto di aggiungere un parametro alla lista di parametri relativa ad un comando
void CommandDTO::insert(std::pair<std::string, std::string> parameter) {
    parameters.push_back(parameter);
}
void CommandDTO::insert(std::string key, std::string value) {
    this->insert(std::make_pair(key, value));
}

// Funzione che ritorna il parametro (nome + valore) in base al nome del parametro stesso
std::pair<std::string, std::string> CommandDTO::find(std::string key) {
    for (auto el: parameters) {
        if (el.first == key) return el;
    }
    return std::pair<std::string, std::string>{};
}

// Funzione che resetta il nome del comando e la lista di parametri
void CommandDTO::erase() {
    parameters.clear();
    command_name.clear();
}

// Funzione che ritorna la dimensione della lista di parametri
size_t CommandDTO::size() {
    return parameters.size();
}
