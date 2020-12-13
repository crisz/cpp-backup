//
// Classe che specifica il comportamento del server riguardo ad una specifica connessione con un client
//

#pragma once
#include <boost/asio/post.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <thread>
#include <string>
#include <memory>
#include <utility>
#include "server/src/command/ServerCommand.h"
#include "common/hash_file.h"
#include "../command/CommandParser.h"
#include <tgmath.h>
#include "server/src/pool/ConnectionsContainer.h"
#include "UserData.h"
#include "common/encode_length_utils.h"

using boost::asio::ip::tcp;

class UserSocket : public std::enable_shared_from_this<UserSocket> {
    tcp::socket socket;
    boost::asio::streambuf buffer;
    ServerCommand current_command;
    CommandParser command_parser{};
public:
    typedef std::shared_ptr<UserSocket> pointer;

    static pointer create(boost::asio::thread_pool &io_context);

    tcp::socket& get_socket();

    void send_response(const std::string& message);
    
    void send_response(const char* message, int size);

    void put_on_read_command();

    void put_on_read_parameter_name();

    void put_on_read_parameter_value(std::string parameter_name, int n);

    void put_on_read_file_data(int file_size);

private:
    UserSocket(boost::asio::thread_pool& io_context);

    void handle_write(const boost::system::error_code& error, size_t bytes_transferred);

    void handle_read_parameter_name(const boost::system::error_code& error, size_t bytes_transferred);

    void handle_read_parameter_value(const boost::system::error_code& error, size_t bytes_transferred, std::string parameter_name);

    void handle_read_command(const boost::system::error_code& error, size_t bytes_transferred);

    void handle_error(const boost::system::error_code& error);

    void handle_disconnection();
};