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
    static ConnectionsContainer& get_instance();

    UserData& get_user_data(tcp::socket& socket);

    void set_user_data(tcp::socket& socket, UserData user_data);

    void add_user(tcp::socket& socket);

    void remove_user(tcp::socket& socket);

    int get_number_users_connected();
};