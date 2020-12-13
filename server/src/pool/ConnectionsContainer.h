#pragma once

#include <map>
#include <boost/asio.hpp>
#include <string>
#include "server/src/user/UserData.h"
#include <iostream>
//
// Classe SINGLETON che contiene le informazioni relative a tutti gli utenti connessi (tramite le socket).
//

#pragma once
using boost::asio::ip::tcp;

class ConnectionsContainer {
private:
    ConnectionsContainer(ConnectionsContainer& source) = delete;
    ConnectionsContainer(ConnectionsContainer&& source) = delete;
    ConnectionsContainer(){}

    std::map<tcp::socket*, UserData> container;
public:
    // Ritorna l'istanza della classe
    static ConnectionsContainer& get_instance();

    // Ritorna le informazioni relative ad un utente tramite la socket associata ad esso.
    UserData& get_user_data(tcp::socket& socket);

    // Setta le informazioni relative ad un utente tramite la socket associata ad esso.
    void set_user_data(tcp::socket& socket, UserData user_data);

    // Aggiunge un utente generico.
    void add_user(tcp::socket& socket);

    // Rimuove le informazioni relative ad un utente tramite la socket associata ad esso.
    void remove_user(tcp::socket& socket);

    // Ritorna il numero di utenti connessi.
    int get_number_users_connected();
};