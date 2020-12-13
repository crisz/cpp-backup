//
//Lo scopo di questa classe è quello di gestire il pool di connessioni con i client.
//

#pragma once
#include <boost/asio/post.hpp>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "server/src/user/UserSocket.h"
#include <thread>
#include <string>
#include <memory>
#include "ServerConf.h"

//Funzione che gestisce la terminazione del server in caso di errore di connessione
void die(std::string message);

class ConnectionPool {
    boost::asio::thread_pool& io_context;
    tcp::acceptor acceptor;
public:
    //Il costruttore inizializza l'io_context e l'acceptor, si mette in ascolto sulla porta di listening
    //e inizia ad accettare le connessioni entranti
    ConnectionPool(boost::asio::thread_pool& io_context);

private:
    //Funzione  che crea un UserSocket e registra una callback per la gestione dell'accept per ogni connessione entrante.
    void start_accept();

    //Funzione che gestisce l'accept di una nuova connessione
    void handle_accept(UserSocket::pointer new_connection, const boost::system::error_code& error);
};