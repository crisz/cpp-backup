#pragma once
#include <boost/asio/post.hpp>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "server/src/user/SingleUserConnection.h"
#include <thread>
#include <string>
#include <memory>
#include "ServerConf.h"


void die(std::string message);

class ConnectionPool {
    boost::asio::thread_pool& io_context;
    tcp::acceptor acceptor;
public:
    ConnectionPool(boost::asio::thread_pool& io_context);

private:
    void start_accept();

    void handle_accept(SingleUserConnection::pointer new_connection, const boost::system::error_code& error);
};