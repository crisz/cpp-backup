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
public:
    ServerConnectionAsio(std::string& address, int port) {
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(address), port);
        s.connect(endpoint);
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
