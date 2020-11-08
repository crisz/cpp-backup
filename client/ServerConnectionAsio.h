//
// Created by Andrea Vara on 05/11/2020.
//

#ifndef CPP_BACKUP_SERVERCONNECTIONASIO_H
#define CPP_BACKUP_SERVERCONNECTIONASIO_H
#include <iostream>
#include <boost/asio.hpp>

class ServerConnectionAsio {
private:
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket s{io_service};
    ServerConnectionAsio(std::string& address, int port) {

        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(address), port);
        s.connect(endpoint);
    }
    std::string address;
    int port;
    ServerConnectionAsio(ServerConnectionAsio const&) = delete;
    ServerConnectionAsio& operator=(ServerConnectionAsio const&) = delete;

    static std::shared_ptr<ServerConnectionAsio> instance;

    static std::shared_ptr<ServerConnectionAsio> get_instance(std::string& address, int port) {
        static std::shared_ptr<ServerConnectionAsio> s{new ServerConnectionAsio{address, port}};
        return s;
    }

public:
    static std::shared_ptr<ServerConnectionAsio> get_instance() {
        std::string fake_address = "";
        static std::shared_ptr<ServerConnectionAsio> s{new ServerConnectionAsio{"", 0}};
        return s;
    }

    static std::shared_ptr<ServerConnectionAsio> init(std::string& address, int port) {
        return get_instance(address, port);
    }

    void send(const std::string& m) {
        const char* message = (m).c_str();
        boost::system::error_code ec;
        boost::asio::write(s, boost::asio::buffer(message,  m.size()),ec);
        if(ec){
            std::cout << ("Cannot send") << std::endl;
            exit(1);
        }

    }

    void send(char* message, int size) {
        // const char* message = (m).c_str();
        boost::system::error_code ec;
        boost::asio::write(s, boost::asio::buffer(message,  size),ec);
        if(ec){
            std::cout << ("Cannot send") << std::endl;
            exit(1);
        }
    }
};


#endif //CPP_BACKUP_SERVERCONNECTIONASIO_H
