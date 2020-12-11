//
// Created by Andrea Vara on 05/11/2020.
//
// Classe SINGLETON che gestisce la connessione con il server
//
#include "ServerConnectionAsio.h"



// Il costruttore effettua la connect con il server
ServerConnectionAsio::ServerConnectionAsio(std::string &address, int port) {
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(address), port);
    s.connect(endpoint);
}

// Funzione che crea l'istanza della classe e la ritorna
std::shared_ptr<ServerConnectionAsio>
ServerConnectionAsio::get_instance_impl(std::string *const address, int *const port) {
    static std::shared_ptr<ServerConnectionAsio> instance{new ServerConnectionAsio{*address, *port}};
    return instance;
}

// Funzione che crea l'istanza della classe con i parametri di default e la ritorna
std::shared_ptr<ServerConnectionAsio> ServerConnectionAsio::get_instance() {
    return std::shared_ptr<ServerConnectionAsio>(get_instance_impl());
}

// Funzione che inizializza l'istanza della classe tramite la funzione get_instance_impl
void ServerConnectionAsio::init(std::string &address, int port) {
    get_instance_impl(&address, &port);
}

// Funzione che si occupa dell'invio di un messaggio passato come stringa
void ServerConnectionAsio::send(const std::string &m) {
    const char* message = (m).c_str();
    boost::system::error_code ec;
    boost::asio::write(s, boost::asio::buffer(message, m.size()),ec);
    if (ec.failed()) {
        throw ServerConnectionAsioException("Cannot send");
    }
}

// Funzione che si occupa dell'invio di un messaggio passato come char*
void ServerConnectionAsio::send(const char *message, int size) {
    boost::system::error_code ec;
    boost::asio::write(s, boost::asio::buffer(message, size), ec);

    if (ec.failed()) {
        throw ServerConnectionAsioException("Cannot send");
    }
}

// Funzione che si occupa della lettura di un messaggio ritornandolo come char*
char *ServerConnectionAsio::read(int length) {
    char* buffer = new char[length];
    boost::system::error_code ec;
    boost::asio::read(s, boost::asio::buffer(buffer, length), boost::asio::transfer_exactly(length), ec);

    if (ec.failed()) {
        throw ServerConnectionAsioException("Cannot read");
    }

    return buffer;
}

// Funzione che si occupa della lettura di un messaggio ritornandolo come stringa
std::string ServerConnectionAsio::read_as_str(int length) {
    char* buffer = new char[length+1];
    boost::system::error_code ec;
    boost::asio::read(s, boost::asio::buffer(buffer, length), boost::asio::transfer_exactly(length), ec);
    buffer[length] = 0;
    return std::string(buffer);
}
