#include <boost/asio/post.hpp>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "SingleUserConnection.h"
#include <thread>
#include <string>
#include <memory>


void die(std::string message) {
    std::cout << message << std::endl;
    exit(-1);
}

class ConnectionPool {
    boost::asio::io_context& io_context;
    tcp::acceptor acceptor;
    std::function<void(std::shared_ptr<SingleUserConnection> user_connection, const std::string& message)> callback;
public:
    ConnectionPool(boost::asio::io_context& io_context, int port, std::function<void(std::shared_ptr<SingleUserConnection> user_connection, const std::string& message)> callback) : 
            io_context(io_context),
            acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {

        this->callback = callback;
        this->start_accept();
    }

private:
    void start_accept() {
        std::cout << "waiting connection"<<std::endl;
        SingleUserConnection::pointer new_connection = SingleUserConnection::create(io_context, this->callback);

        auto on_accept = boost::bind(&ConnectionPool::handle_accept, this, new_connection, boost::asio::placeholders::error);
        acceptor.async_accept(new_connection->get_socket(), on_accept);
    }

    void handle_accept(SingleUserConnection::pointer new_connection, const boost::system::error_code& error) {
        if (!error) {
            std::cout << "A client connected" << std::endl;
            new_connection->put_on_read_command();
        } else {
            // TODO: handle error: stampare qualcosa o generare un file di log
        }

        start_accept();
    }
};