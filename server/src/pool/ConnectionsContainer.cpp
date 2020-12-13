//
// Classe SINGLETON che contiene le informazioni relative a tutti gli utenti connessi (tramite le socket).
//

#include "ConnectionsContainer.h"

#include <utility>

// Ritorna l'istanza della classe
ConnectionsContainer &ConnectionsContainer::get_instance() {
    static ConnectionsContainer instance;
    return instance;
}

// Ritorna le informazioni relative ad un utente tramite la socket associata ad esso.
UserData &ConnectionsContainer::get_user_data(tcp::socket &socket) {
    return container[&socket];
}

// Setta le informazioni relative ad un utente tramite la socket associata ad esso.
void ConnectionsContainer::set_user_data(tcp::socket &socket, UserData user_data) {
    container[&socket] = std::move(user_data);
}

// Aggiunge un utente generico.
void ConnectionsContainer::add_user(tcp::socket &socket) {
    UserData ud;
    container[&socket] = ud;
}

// Rimuove le informazioni relative ad un utente tramite la socket associata ad esso.
void ConnectionsContainer::remove_user(tcp::socket &socket) {
    container.erase(&socket);
    std::cout << "Ci sono " << this->get_number_users_connected() << " utenti connessi" << std::endl;
}

// Ritorna il numero di utenti connessi.
int ConnectionsContainer::get_number_users_connected() {
    return container.size() - 1;
}

bool ConnectionsContainer::check_user_connected(const std::string& username) {
    for (auto& pair: container) {
        if (pair.second.username == username) {
            return true;
        }
    }
    return false;
}
