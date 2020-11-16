#pragma once

#include <map>
#include <boost/asio.hpp>
#include <string>
#include "UserData.h"

using boost::asio::ip::tcp;

class SessionContainer {
private:
    SessionContainer(SessionContainer& source) = delete;
    SessionContainer(SessionContainer&& source) = delete;
    SessionContainer(){}

    std::map<tcp::socket*, UserData> container;
public:
    static SessionContainer& get_instance() {
        static SessionContainer instance;
        return instance;
    }

    UserData& get_user_data(tcp::socket& socket) {
        return container[&socket];
    }

    void set_user_data(tcp::socket& socket, UserData user_data) {
        container[&socket] = user_data;
    }
};