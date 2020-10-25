#include <boost/asio/post.hpp>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <thread>
#include <string>
#include <memory>


using boost::asio::ip::tcp;

class SingleUserConnection : public boost::enable_shared_from_this<SingleUserConnection>{

    tcp::socket socket;
    boost::asio::streambuf buffer;
public:
    typedef boost::shared_ptr<SingleUserConnection> pointer;

    static pointer create(boost::asio::io_service &io_service) {
        return pointer(new SingleUserConnection(io_service));
    }

    tcp::socket &get_socket() {
        return socket;
    }

    void send_response(const std::string& message) {
        auto on_write = boost::bind(&SingleUserConnection::handle_write, shared_from_this(),
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred);
        auto buffered_message = boost::asio::buffer(message, message.size());
        boost::asio::async_write(socket, buffered_message, on_write);
    }

    void put_on_read() {
        auto on_read = boost::bind(&SingleUserConnection::handle_read, shared_from_this(),
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred);
        boost::asio::async_read_until(socket, buffer, '\n', on_read);
    }
private:
    SingleUserConnection(boost::asio::io_service& io_service): socket(io_service){}

    void handle_write(const boost::system::error_code& error, size_t bytes_transferred) {
        std::cout << "write completed" << std::endl;
    }

    void handle_read(const boost::system::error_code& error, size_t bytes_transferred) {

        std::cout << "Handle Read of connection\n";

        if (error && error != boost::asio::error::eof) {
            std::cout << "Error: " << error.message() << "\n";

            // TODO: handle error
            return;
        }

        std::string messageP;
        std::cout << messageP << std::endl;
        if (messageP != "") {

            this->put_on_read();
        } else {
            // TODO: this->handle_disconnection();
        }
    }
};