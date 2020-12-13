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

    // Il costruttore effettua la connect con il server
    ServerConnectionAsio(std::string& address, int port);

    // Funzione che crea l'istanza della classe e la ritorna
    static std::shared_ptr<ServerConnectionAsio> get_instance_impl(std::string* const address = nullptr, int* const port = nullptr);

public:

    // Funzione che crea l'istanza della classe con i parametri di default e la ritorna
    static std::shared_ptr<ServerConnectionAsio> get_instance();

    // Funzione che inizializza l'istanza della classe tramite la funzione get_instance_impl
    static void init(std::string& address, int port);

    // Funzione che si occupa dell'invio di un messaggio passato come stringa
    void send(const std::string& m);

    // Funzione che si occupa dell'invio di un messaggio passato come char*
    void send(const char* message, int size);

    // Funzione che si occupa della lettura di un messaggio ritornandolo come char*
    char* read(int length);

    // Funzione che si occupa della lettura di un messaggio ritornandolo come stringa
    std::string read_as_str(int length);

};

#endif //CPP_BACKUP_SERVERCONNECTIONASIO_H
