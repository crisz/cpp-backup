#pragma once

#include <map>
#include <boost/asio.hpp>
#include <string>
#include "server/src/user/UserData.h"
#include <iostream>

using boost::asio::ip::tcp;

class SessionContainer {
private:
    SessionContainer(SessionContainer& source) = delete;
    SessionContainer(SessionContainer&& source) = delete;
    SessionContainer(){}

    std::map<tcp::socket*, UserData> container;
public:
    static SessionContainer& get_instance();

    UserData& get_user_data(tcp::socket& socket);

    void set_user_data(tcp::socket& socket, UserData user_data);

    void add_user(tcp::socket& socket);

    void remove_user(tcp::socket& socket);

    int get_number_users_connected();
};