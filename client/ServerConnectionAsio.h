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
public:
    ServerConnectionAsio(std::string& address, int port) {
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(address), port);
        s.connect(endpoint);
    }

    void send(const std::string& m) {
        const char* message = (m).c_str();
        boost::system::error_code ec;
        boost::asio::write(s, boost::asio::buffer(message,  m.size()),ec);
        if(ec){
            std::cout << ("Cannot send") << std::endl;
            exit(1);
        }

    }

    void send(char* message, int size) {
        // const char* message = (m).c_str();
        boost::system::error_code ec;
        boost::asio::write(s, boost::asio::buffer(message,  size),ec);
        if(ec){
            std::cout << ("Cannot send") << std::endl;
            exit(1);
        }
    }

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

};


#endif //CPP_BACKUP_SERVERCONNECTIONASIO_H
