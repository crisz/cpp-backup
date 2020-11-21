#pragma once
#include <boost/asio/post.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <thread>
#include <string>
#include <memory>
#include <arpa/inet.h>
#include "ServerCommand.h"
#include "../common/hash_file.h"
#include "CommandParser.h"
#include <tgmath.h>
#include "SessionContainer.h"
#include "UserData.h"


using boost::asio::ip::tcp;

class SingleUserConnection : public std::enable_shared_from_this<SingleUserConnection> { // TODO: splittare in ServerSocketManager (che contiene tutte le scritture e le letture)
                    // e CommandEncoder/CommandHandler/altro che contiene gli handle (handle_write, handle_read) e che utilizza il ServerSocketManager
    tcp::socket socket;
    boost::asio::streambuf buffer;
    ServerCommand currentCommand;
    CommandParser commandParser;


public:
    typedef std::shared_ptr<SingleUserConnection> pointer;

    static pointer create(boost::asio::thread_pool &io_context) {
        return pointer(new SingleUserConnection(io_context));
    }

    tcp::socket& get_socket() {
        return socket;
    }

    void send_response(const std::string message) {
        std::cout << "Sending response " << message << " with length " << message.size() << std::endl;
        auto on_write = boost::bind(&SingleUserConnection::handle_write, shared_from_this(),
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred);
        auto buffered_message = boost::asio::buffer(message, message.size());
        boost::asio::async_write(socket, buffered_message, on_write);
    }
    void send_response(char* message, int size) {
        std::cout << "Sending response " << message << " with length " << size << std::endl;
        auto on_write = boost::bind(&SingleUserConnection::handle_write, shared_from_this(),
                                    boost::asio::placeholders::error,
                                    boost::asio::placeholders::bytes_transferred);
        auto buffered_message = boost::asio::buffer(message, size);
        boost::asio::async_write(socket, buffered_message, on_write);
    }

    void put_on_read_command() {
    
        currentCommand.clear();
        auto on_read = boost::bind(&SingleUserConnection::handle_read_command, shared_from_this(),
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred);
            boost::asio::async_read(socket, buffer, boost::asio::transfer_exactly(8), on_read);

    }

    void put_on_read_parameter_name() {
        std::cout << "waiting for 12 bytes" << std::endl;
        auto on_read = boost::bind(&SingleUserConnection::handle_read_parameter_name, shared_from_this(),
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred);
        
        boost::asio::async_read(socket, buffer, boost::asio::transfer_exactly(12), on_read);
    }

    void put_on_read_parameter_value(std::string parameter_name, int n) {
        std::cout << "waiting for (pv) " << n << " bytes" << std::endl;

        auto on_read = boost::bind(&SingleUserConnection::handle_read_parameter_value, shared_from_this(),
                                    boost::asio::placeholders::error,
                                    boost::asio::placeholders::bytes_transferred,
                                    parameter_name);
        
        boost::asio::async_read(socket, buffer, boost::asio::transfer_exactly(n), on_read);
    }

    void put_on_read_file_data(int file_size) {
        std::cout << "PUT ON READ FILE DATA!!!!!!!!" << std::endl;
        int buffer_size = 20;
        int number_of_reads = ceil((double)file_size/(double)buffer_size);
        std::cout << "start send file with size " << file_size << std::endl;

        this->commandParser.start_send_file(socket, file_size, currentCommand);

        std::cout << "send file started" << std::endl;

        // std::thread t([this, buffer_size, number_of_reads]() { // TODO: indagare il motivo per cui non va bene
            boost::system::error_code ec; // TODO: gestire errore

            char* _buffer = new char[buffer_size + 1];
            for (int i=0; i<number_of_reads; i++) {
                std::cout << std::this_thread::get_id() << "!!!" << i << " out of " << number_of_reads << std::endl;
                if (i == number_of_reads - 1) {
                    buffer_size = file_size - i * buffer_size;
                }
                std::cout << "buffer size is  " << buffer_size << std::endl;

                int bytes_read = boost::asio::read(socket, boost::asio::buffer(_buffer, buffer_size), boost::asio::transfer_exactly(buffer_size), ec);
                if(ec) {
                    handle_error(ec);
                    return;
                }
                std::cout << "buffer size is  " << buffer_size << " and I read " << bytes_read << std::endl;
                _buffer[buffer_size] = 0;
                std::cout << "Received buffer " << _buffer << std::endl;

                this->commandParser.send_file_chunk(_buffer, bytes_read).get();
                std::cout << "Chunk saved" << _buffer << std::endl;

            }

            std::cout << "end of read from client " << std::endl;
            this->commandParser.end_send_file();
            // ignoriamo il valore poiché è sempre STOPFLOW0000.
            boost::asio::read(socket, boost::asio::buffer(_buffer,12), boost::asio::transfer_exactly(12), ec);
            delete[] _buffer;
            this->send_response("POSTFILE");
            this->send_response("__RESULT");
            this->send_response(encode_length(2), 4);
            this->send_response("OK");
            this->send_response("STOPFLOW");
            this->send_response(encode_length(0), 4);
            this->put_on_read_command();
        // });
        // t.detach();
    }
    char* encode_length(int size) { // TODO: muovere in utils
        char* result = new char[4];
        int length = htonl(size); // htonl serve per non avere problemi di endianess
        result[3] = (length & 0xFF);
        result[2] = (length >> 8) & 0xFF;
        result[1] = (length >> 16) & 0xFF;
        result[0] = (length >> 24) & 0xFF;
        return result;
    }
private:
    SingleUserConnection(boost::asio::thread_pool& io_context) : socket(io_context) {}

    void handle_write(const boost::system::error_code& error, size_t bytes_transferred) {
        //std::cout << "write completed" << std::endl;
    }

    void handle_read_parameter_name(const boost::system::error_code& error, size_t bytes_transferred) {
        if (error) {
            handle_error(error);//TODO: implementare una gestione degli errori sensata con la disconnessione
            return;
        }

        std::ostringstream oss;
        oss << &buffer;
        oss.flush();
        std::string parameter = oss.str();

        std::string message_name = parameter.substr(0, 8);
        if(message_name.compare("STOPFLOW")==0){
            std::cout<<"Fine comando raggiunto"<<std::endl;
            // boost::system::error_code ec;
            // boost::asio::write(socket, boost::asio::buffer("LOGINSNC", 8), ec);
            // boost::asio::write(socket, boost::asio::buffer("__RESULT", 8), ec);
            // boost::asio::write(socket, boost::asio::buffer(encode_length(2), 4), ec);
            // boost::asio::write(socket, boost::asio::buffer("OK", 2), ec);
            // boost::asio::write(socket, boost::asio::buffer("STOPFLOW", 8), ec);
            SessionContainer& sc=SessionContainer::get_instance();
            UserData ud;
            if(sc.get_user_data(socket).username!="") ud.username= sc.get_user_data(socket).username;
            ud.send_response_callback = [this](const std::string message) {
                this->send_response(message);
            };
            ud.send_raw_response_callback = [this](char* message, int size) {
                this->send_response(message, size);
            };
            SessionContainer::get_instance().set_user_data(socket,ud);
            commandParser.handleCommand(socket, currentCommand);
            this->put_on_read_command();
            return;
        }

        const char* message_size_arr = parameter.substr(8, 12).c_str();
        int message_size = 0;
        int shift_value = 24;

        //std::cout << "size string is " << parameter.substr(8, 12) << std::endl;


        std::cout << "0: " << (int)message_size_arr[0] << std::endl;
        std::cout << "1: " << (int)message_size_arr[1] << std::endl;
        std::cout << "2: " << (int)message_size_arr[2] << std::endl;
        std::cout << "3: " << (int)message_size_arr[3] << std::endl;

        for (int i=0; i<4; i++) {
            char x = (char)message_size_arr[i];
            //std::cout << "Summing: " << x << " with shift " << shift_value << std::endl;
            message_size += ((char)message_size_arr[i]) << shift_value;
            shift_value -= 8;
        }

        int fixed_size = ntohl(message_size);

        std::cout << "Received parameter " << message_name << " with length " << fixed_size << std::endl;

        if (message_name.compare(FILEDATA) == 0) {
            this->put_on_read_file_data(fixed_size);
        } else {
            this->put_on_read_parameter_value(message_name, fixed_size);
        }
    }

    void handle_read_parameter_value(const boost::system::error_code& error, size_t bytes_transferred, std::string parameter_name) {
        //std::cout << "Received parameter value with size " << bytes_transferred << std::endl ;
        if (error) {
            handle_error(error);
            return;
        }

        std::ostringstream oss;
        oss << &buffer;
        std::string parameter_value = oss.str();


        currentCommand.addParameter(parameter_name, parameter_value);
        std::cout << "value for parameter " << parameter_name << " is " <<  currentCommand.getParameters()[parameter_name]<< std::endl;
        this->put_on_read_parameter_name();


    }


    void handle_read_command(const boost::system::error_code& error, size_t bytes_transferred) {
        std::cout << "Received command \n";
        if (error) {
            handle_error(error);
            // TODO: handle error
            return;
        }

        std::string messageP;
        std::ostringstream oss;
        oss << &buffer;
        messageP = oss.str();

        currentCommand.setName(messageP);
        std::cout << "Message:" << currentCommand.getCommand_name() << std::endl;
        if (messageP != "") {
            // std::shared_ptr<SingleUserConnection> this_ptr = shared_from_this();
            // this->handle_message_callback(this_ptr, messageP);
            // this->currentCommand = messageP;
            std::cout << "returning in read parameter name " << std::endl;
            this->put_on_read_parameter_name();
        } else {
            // TODO: this->handle_disconnection();
        }
    }


    void handle_error(const boost::system::error_code& error) {

        std::cout << "Error: " << error.message() << "\n";
        if(currentCommand.getCommand_name()=="POSTFILE"){
           // remove_file();
        }

        if (error == boost::asio::error::eof) {
            std::cout << "Il cliente ha chiuso la connessione" << std::endl;
            return;
        }
        std::cout << "Error on command :" << currentCommand.getCommand_name() << std::endl;
        if (currentCommand.getParameters().empty()) {
            std::cout << "Non ci sono parametri associati al comando"<<std::endl;
        } else {
            std::cout << "Last parameter read: " << (currentCommand.getParameters().cbegin())->first << " with value : "
                      << (currentCommand.getParameters().cbegin())->second << std::endl;
        }
    }

};