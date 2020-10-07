

#include <boost/asio/post.hpp>

#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include <thread>
#include <string>
#include <memory>

void die(std::string message) {
    std::cout << message << std::endl;
    exit(-1);
}

using boost::asio::ip::tcp;

class tcp_connection : public boost::enable_shared_from_this<tcp_connection>{
public:
    typedef boost::shared_ptr<tcp_connection> pointer;

    static pointer create(boost::asio::io_service &io_service) {
        return pointer(new tcp_connection(io_service));
    }

    tcp::socket &socket() {
        return socket_;
    }

    void start() {
        //Read from client, make json and send appropriate response
        boost::asio::async_read(socket_, message_,
                                boost::bind(&tcp_connection::handle_read, shared_from_this(),
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }
private:
    tcp_connection(boost::asio::io_service& io_service): socket_(io_service){}

    void handle_write(const boost::system::error_code& /*error*/,
                      size_t /*bytes_transferred*/)
    {
    }

    void handle_read(const boost::system::error_code& error, size_t bytes_transferred) {

        std::cout << "Handle Read of connection\n";

        if (error && error != boost::asio::error::eof) {
            std::cout << "Error: " << error.message() << "\n";
            return;
        }

        std::string messageP;
        {
            std::stringstream ss;
            ss << &message_;
            ss.flush();
            messageP = ss.str();
        }

        std::cout << messageP << std::endl;



    }

    tcp::socket socket_;
    boost::asio::streambuf message_;
};


class tcp_server{

public:
    tcp_server(boost::asio::io_context& io_context, int port)
            : io_context_(io_context),
              acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    {

        start_accept();
    }



private:
    void start_accept()
    {
            std::cout << "waiting connection"<<std::endl;
            tcp_connection::pointer new_connection =
                    tcp_connection::create(io_context_);

            acceptor_.async_accept(new_connection->socket(),
                                   boost::bind(&tcp_server::handle_accept, this, new_connection,
                                               boost::asio::placeholders::error));

    }

    void handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& error)
    {
        if (!error)
        {
            std::cout << "A client connected" << std::endl;
            new_connection->start();
        }

        start_accept();
    }

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;

};


