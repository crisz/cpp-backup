//
// Created by Andrea Vara on 05/11/2020.
//

#ifndef CPP_BACKUP_SERVERCONNECTIONASIO_H
#define CPP_BACKUP_SERVERCONNECTIONASIO_H
#include <iostream>
#include <boost/asio.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/map.hpp>



class ServerConnectionAsio {
private:
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket s{io_service};
    std::string address;
    int port;

    ServerConnectionAsio(ServerConnectionAsio const&) = delete;

    ServerConnectionAsio& operator=(ServerConnectionAsio const&) = delete;

    ServerConnectionAsio(std::string& address, int port) {
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(address), port);
        s.connect(endpoint);
    }

    static std::shared_ptr<ServerConnectionAsio> get_instance_impl(std::string* const address = nullptr, int* const port = nullptr) {
        static std::shared_ptr<ServerConnectionAsio> instance{new ServerConnectionAsio{*address, *port}};
        return instance;
    }

public:

    static std::shared_ptr<ServerConnectionAsio> get_instance() {
        return std::shared_ptr<ServerConnectionAsio>(get_instance_impl());
    }

    static void init(std::string& address, int port) {
        get_instance_impl(&address, &port);
    }


    void send(const std::string& m) {
        std::cout << "Sending " << m << " with size " << m.size() << std::endl;

        const char* message = (m).c_str();
        boost::system::error_code ec;
        boost::asio::write(s, boost::asio::buffer(message, m.size()),ec);
        if(ec){
            std::cout << "Cannot send" << std::endl;
            exit(1); // TODO: gestire eccezione
        }

    }

    void send(char* message, int size) {
        // const char* message = (m).c_str();
        std::cout << "Sending " << message << " with size " << size << std::endl;
        boost::system::error_code ec;
        boost::asio::write(s, boost::asio::buffer(message, size), ec);
        if(ec){
            std::cout << "Cannot send" << std::endl;
            exit(1);
        }
    }

<<<<<<< HEAD
    char* read(int length) {
        char* buffer = new char[length];
        boost::system::error_code ec;
        boost::asio::read(s, boost::asio::buffer(buffer, length), boost::asio::transfer_exactly(length), ec);
        return buffer;
    }
=======
    void receve(){
        boost::asio::streambuf buffer;
        boost::asio::read(s,buffer,boost::asio::transfer_exactly(153));
        std::ostringstream oss;
        oss << &buffer;
        oss.flush();
        std::string data = oss.str();
        boost::iostreams::basic_array_source<char>device(data.data(), data.size());
        boost::iostreams::stream<boost::iostreams::basic_array_source<char>> istr(device);
        boost::archive::binary_iarchive ia(istr);
        std::map<std::string, std::string> m2;
        ia >> m2;

        for(auto a: m2){
            std::cout<<a.first<<" : "<<a.second<<std::endl;
        }
    }

>>>>>>> origin/serverBoost-with-serialization
};






#endif //CPP_BACKUP_SERVERCONNECTIONASIO_H
