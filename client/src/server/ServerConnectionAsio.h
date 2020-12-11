//
// Created by Andrea Vara on 05/11/2020.
//
// Classe SINGLETON che gestisce la connessione con il server
//

#ifndef CPP_BACKUP_SERVERCONNECTIONASIO_H
#define CPP_BACKUP_SERVERCONNECTIONASIO_H
#include <iostream>
#include <boost/asio.hpp>



class ServerConnectionAsioException: public std::exception {
private:
    std::string message;
public:
    int code;
    ServerConnectionAsioException(std::string&& message, int code=0) : message{message}, code{code} {}

    const char* what() const throw() {
        return ("Non è possibile raggiungere il server: (" + message + ")").c_str();
    }
};

class ServerConnectionAsio {
private:
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket s{io_service};
    std::string address;
    int port;

    ServerConnectionAsio(ServerConnectionAsio const&) = delete;
    ServerConnectionAsio& operator=(ServerConnectionAsio const&) = delete;

    ServerConnectionAsio(std::string& address, int port);

    static std::shared_ptr<ServerConnectionAsio> get_instance_impl(std::string* const address = nullptr, int* const port = nullptr);

public:

    static std::shared_ptr<ServerConnectionAsio> get_instance();

    static void init(std::string& address, int port);

    void send(const std::string& m);

    void send(const char* message, int size);

    char* read(int length);

    std::string read_as_str(int length);

};

#endif //CPP_BACKUP_SERVERCONNECTIONASIO_H
