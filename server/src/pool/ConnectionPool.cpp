#include "ConnectionPool.h"

ConnectionPool::ConnectionPool(boost::asio::thread_pool &io_context) :
        io_context(io_context),
        acceptor(io_context, tcp::endpoint(tcp::v4(), ServerConf::get_instance().port)) {

    acceptor.listen(0);
    this->start_accept();
}

void ConnectionPool::start_accept() {
    std::cout << "waiting connection"<<std::endl;
    SingleUserConnection::pointer new_connection = SingleUserConnection::create(io_context);

    auto on_accept = boost::bind(&ConnectionPool::handle_accept, this, new_connection, boost::asio::placeholders::error);
    acceptor.async_accept(new_connection->get_socket(), on_accept);
}

void ConnectionPool::handle_accept(SingleUserConnection::pointer new_connection, const boost::system::error_code &error) {
    if (!error) {
        std::thread t([new_connection]() {
            std::cout << "A client connected" << std::endl;
            new_connection->put_on_read_command();
        });
        t.detach();
    } else {
        // TODO: handle error: stampare qualcosa o generare un file di log
    }

    start_accept();
}

void die(std::string message) {
    std::cout << message << std::endl;
    exit(-1);
}
