

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

    tcp::socket socket;
    boost::asio::streambuf buffer;
public:
    typedef boost::shared_ptr<tcp_connection> pointer;

    static pointer create(boost::asio::io_service &io_service) {
        return pointer(new tcp_connection(io_service));
    }

    tcp::socket &get_socket() {
        return socket;
    }

    void put_on_read() {
        auto on_read = boost::bind(&tcp_connection::handle_read, shared_from_this(),
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred);
        boost::asio::async_read_until(socket, buffer, '\n', on_read);
    }
private:
    tcp_connection(boost::asio::io_service& io_service): socket(io_service){}

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
            ss << &buffer;
            ss.flush();
            messageP = ss.str();
        }

        std::cout << messageP << std::endl;
        if (messageP != "") {
            this->put_on_read();
        } else {
            // this->handle_disconnection();
        }

    }
};


class tcp_server{
    boost::asio::io_context& io_context;
    tcp::acceptor acceptor;
public:
    tcp_server(boost::asio::io_context& io_context, int port)
            : io_context(io_context),
              acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {
        this->start_accept();
    }

private:
    void start_accept() {
        std::cout << "waiting connection"<<std::endl;
        tcp_connection::pointer new_connection = tcp_connection::create(io_context);

        auto on_accept = boost::bind(&tcp_server::handle_accept, this, new_connection, boost::asio::placeholders::error);
        acceptor.async_accept(new_connection->get_socket(), on_accept);
    }

    void handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& error) {
        if (!error) {
            std::cout << "A client connected" << std::endl;
            new_connection->put_on_read();
        } else {
            // TODO: handle error
        }

        start_accept();
    }
};


