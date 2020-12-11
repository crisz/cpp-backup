#include "UserSocket.h"

UserSocket::pointer UserSocket::create(boost::asio::thread_pool &io_context) {
    return pointer(new UserSocket(io_context));
}

tcp::socket &UserSocket::get_socket() {
    return socket;
}

void UserSocket::send_response(const std::string &message) {
    //std::cout << "Sending response " << message << " with length " << message.size() << std::endl;
    auto on_write = boost::bind(&UserSocket::handle_write, shared_from_this(),
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred);
    auto buffered_message = boost::asio::buffer(message, message.size());
    boost::asio::async_write(socket, buffered_message, on_write);
}

void UserSocket::send_response(const char *message, int size) {
    //std::cout << "Sending response " << message << " with length " << size << std::endl;

    auto on_write = boost::bind(&UserSocket::handle_write, shared_from_this(),
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred);

    auto buffered_message = boost::asio::buffer(message, size);
    boost::asio::async_write(socket, buffered_message, on_write);
}

void UserSocket::put_on_read_command() {
    current_command.clear();
    auto on_read = boost::bind(&UserSocket::handle_read_command, shared_from_this(),
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred);
    boost::asio::async_read(socket, buffer, boost::asio::transfer_exactly(8), on_read);
}

void UserSocket::put_on_read_parameter_name() {
    auto on_read = boost::bind(&UserSocket::handle_read_parameter_name, shared_from_this(),
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred);
    boost::asio::async_read(socket, buffer, boost::asio::transfer_exactly(12), on_read);
}

void UserSocket::put_on_read_parameter_value(std::string parameter_name, int n) {
    auto on_read = boost::bind(&UserSocket::handle_read_parameter_value, shared_from_this(),
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred,
                               std::move(parameter_name));
    boost::asio::async_read(socket, buffer, boost::asio::transfer_exactly(n), on_read);
}

void UserSocket::put_on_read_file_data(int file_size) {
    int buffer_size = 1001;
    int number_of_reads = ceil((double)file_size/(double)buffer_size);

    try {
        this->command_parser.start_send_file(socket, file_size, current_command);

        boost::system::error_code ec;

        char *_buffer = new char[buffer_size + 1];
        for (int i = 0; i < number_of_reads; i++) {
            if (i == number_of_reads - 1) {
                buffer_size = file_size - i * buffer_size;
            }

            int bytes_read = boost::asio::read(socket, boost::asio::buffer(_buffer, buffer_size),
                                               boost::asio::transfer_exactly(buffer_size), ec);
            if (ec) {
                handle_error(ec);
                return;
            }
            _buffer[bytes_read] = 0;


            this->command_parser.send_file_chunk(_buffer, bytes_read).get();
        }

        bool result = this->command_parser.end_send_file(socket, current_command);

        // ignoriamo il valore poiché è sempre STOPFLOW0000.
        boost::asio::read(socket,
                boost::asio::buffer(_buffer, 12),
                boost::asio::transfer_exactly(12),
                ec);

        delete[] _buffer;
        this->send_response("POSTFILE");
        this->send_response("__RESULT");
        this->send_response(encode_length(2), 4);
        this->send_response(result ?"OK" : "KO");
        this->send_response("STOPFLOW");
        this->send_response(encode_length(0), 4);
        this->put_on_read_command();

    } catch (BufferedFileWriterException& bfwe) {
        std::cerr << "È avvenuto un errore nella scrittura del file: " << bfwe.what() << std::endl;
        handle_disconnection();
        return;
    } catch (...) {
        std::cerr << "È avvenuto un errore durante il tentativo di scrittura del file." << std::endl;
        handle_disconnection();
        return;
    }


}

UserSocket::UserSocket(boost::asio::thread_pool &io_context) : socket(io_context) {}

void UserSocket::handle_write(const boost::system::error_code &error, size_t bytes_transferred) {
}

void UserSocket::handle_read_parameter_name(const boost::system::error_code &error, size_t bytes_transferred) {
    if (error) {
        handle_error(error);
        return;
    }

    std::ostringstream oss;
    oss << &buffer;
    oss.flush();
    std::string parameter = oss.str();

    std::string message_name = parameter.substr(0, 8);
    if(message_name.compare("STOPFLOW")==0){
        ConnectionsContainer& sc=ConnectionsContainer::get_instance();
        UserData ud;
        if(sc.get_user_data(socket).username!="") ud.username= sc.get_user_data(socket).username;

        ud.send_response_callback = [this](const std::string message) {
            this->send_response(message);
        };
        ud.send_raw_response_callback = [this](const char* message, int size) {
            this->send_response(message, size);
        };
        ConnectionsContainer::get_instance().set_user_data(socket, ud);
        command_parser.digest(socket, current_command);
        this->put_on_read_command();
        return;
    }

    const char* message_size_arr = parameter.substr(8, 12).c_str();


    int fixed_size = decode_length(message_size_arr);

//    int message_size = 0;
//    int shift_value = 24;
//
//    std::cout << "0: " << (int)message_size_arr[0] << std::endl;
//    std::cout << "1: " << (int)message_size_arr[1] << std::endl;
//    std::cout << "2: " << (int)message_size_arr[2] << std::endl;
//    std::cout << "3: " << (int)message_size_arr[3] << std::endl;
//
//    for (int i=0; i<4; i++) {
//        char x = (char)message_size_arr[i];
//        message_size += ((char)message_size_arr[i]) << shift_value;
//        shift_value -= 8;
//    }
//
//    int fixed_size = ntohl(message_size);
//  std::cout << "Received parameter " << message_name << " with length " << fixed_size << std::endl;

    if (message_name.compare(FILEDATA) == 0) {
        this->put_on_read_file_data(fixed_size);
    } else {
        this->put_on_read_parameter_value(message_name, fixed_size);
    }
}

void UserSocket::handle_read_parameter_value(const boost::system::error_code &error, size_t bytes_transferred,
                                             std::string parameter_name) {
    //std::cout << "Received parameter value with size " << bytes_transferred << std::endl ;
    if (error) {
        handle_error(error);
        return;
    }

    std::ostringstream oss;
    oss << &buffer;
    std::string parameter_value = oss.str();

    current_command.add_parameter(parameter_name, parameter_value);
    //std::cout << "value for parameter " << parameter_name << " is " << current_command.get_parameters()[parameter_name] << std::endl;
    this->put_on_read_parameter_name();
}

void UserSocket::handle_read_command(const boost::system::error_code &error, size_t bytes_transferred) {
    if (error) {
        handle_error(error);
        return;
    }

    std::string messageP;
    std::ostringstream oss;
    oss << &buffer;
    messageP = oss.str();

    current_command.set_name(messageP);
    std::cout << "Comando ricevuto: " << current_command.get_command_name() << std::endl;
    if (!messageP.empty()) {
        this->put_on_read_parameter_name();
    } else {
        handle_disconnection();
        return;
    }
}

void UserSocket::handle_error(const boost::system::error_code &error) {


    // Disconnessione ordinaria del client. Non è un errore, per cui non viene stampato nulla
    if (error == boost::asio::error::eof) {
        handle_disconnection();
        return;
    }

    std::cout << "Errore: " << error.message() << std::endl;
    std::cout << "Si è verificato un errore nel comando: " << current_command.get_command_name() << std::endl;
    if (current_command.get_parameters().empty()) {
        std::cout << "Non erano presenti parametri associati al comando"<< std::endl;
    } else {
        std::cout << "Ultimo parametro letto: " << (current_command.get_parameters().cbegin())->first << " con valore: "
                  << (current_command.get_parameters().cbegin())->second << std::endl;
    }
    handle_disconnection();
}

void UserSocket::handle_disconnection() {
    std::cout << "Il cliente ha chiuso la connessione" << std::endl;
    this->command_parser.rollback_command(socket, this->current_command);
    ConnectionsContainer::get_instance().remove_user(socket);
}
