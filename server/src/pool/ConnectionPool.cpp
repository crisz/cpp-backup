//Lo scopo di questa classe è quello di gestire il pool di connessioni con i client.

#include "ConnectionPool.h"

//Il costruttore inizializza l'io_context e l'acceptor, si mette in ascolto sulla porta di listening
//e inizia ad accettare le connessioni entranti
ConnectionPool::ConnectionPool(boost::asio::thread_pool &io_context) :
        io_context(io_context),
        acceptor(io_context, tcp::endpoint(tcp::v4(), ServerConf::get_instance().port)) {

    acceptor.listen(0);
    this->start_accept();
}

//Funzione  che crea un UserSocket e registra una callback per la gestione dell'accept per ogni connessione entrante.
void ConnectionPool::start_accept() {
    std::cout << "waiting connection"<<std::endl;
    UserSocket::pointer new_connection = UserSocket::create(io_context);

    auto on_accept = boost::bind(&ConnectionPool::handle_accept, this, new_connection, boost::asio::placeholders::error);
    acceptor.async_accept(new_connection->get_socket(), on_accept);
}

//Funzione che gestisce l'accept di una nuova connessione
void ConnectionPool::handle_accept(UserSocket::pointer new_connection, const boost::system::error_code &error) {
    if (!error) {
        std::thread t([new_connection]() {
            std::cout << "A client connected" << std::endl;
            new_connection->put_on_read_command();
        });
        t.detach();
    } else {
        std::cerr<<"ERRORE nello stabilire la connessione con il client"<<std::endl;
    }
    start_accept();
}

//Funzione che gestisce la terminazione del server in caso di errore di connessione
void die(std::string message) {
    std::cout << message << std::endl;
    exit(-1);
}
