#include <string>
#include <map>
#include "ServerConnectionAsio.h"
#include <arpa/inet.h>
#include <future>
class CommandDispatcher {
private:
    std::shared_ptr<ServerConnectionAsio> sc;
    // std::string received_command;
public:
    CommandDispatcher() {
        sc = ServerConnectionAsio::get_instance();
    }

    std::future<std::map<std::string, std::string>> dispatch(std::string& command, std::map<std::string, std::string>& parameters) {
        std::cout << "Sending command " << command << std::endl;
        // lock acquire
        sc->send(command);
        
        for (auto it = parameters.begin(); it != parameters.end(); it++ ) {
            send_parameter(it->first, it->second);
        }

        send_parameter("STOPFLOW", "");

        // lock release

        return std::async([this]() {
            // lock acquire
            std::cout << "Waiting for resopnse" << std::endl;

            std::map<std::string, std::string> result;
            std::string received_command = sc->read(8);
            // wait until received_command == command
            std::cout << "Starting listening" << std::endl;

            while(true) {
                std::string parameter_name = sc->read(8);
                std::cout << "Received parameter " << parameter_name << std::endl;

                if (parameter_name == "STOPFLOW") {
                    return result;
                }
                int length = decode_length(sc->read(4));
                std::string parameter_value = sc->read(length);

                std::cout << "Received parameter value " << parameter_value << std::endl;

                result[parameter_name] = parameter_value;
            }
            // lock release
        });
    }

    void send_parameter(std::string key, std::string value) {
        std::cout << "Sending parameter with key " << key << " and value " << value << std::endl;

        sc->send(key);
        sc->send(encode_length(value.size()), 4);
        sc->send(value);
    }

    char* encode_length(int size) {
        std::cout << "ecndoing length " << size << std::endl;
        char* result = new char[4];
        int length = htonl(size); // htonl serve per non avere problemi di endianess
        result[3] = (length & 0xFF);
        result[2] = (length >> 8) & 0xFF;
        result[1] = (length >> 16) & 0xFF;
        result[0] = (length >> 24) & 0xFF;
        std::cout << "0: " << (int)result[0] << std::endl;
        std::cout << "1: " << (int)result[1] << std::endl;
        std::cout << "2: " << (int)result[2] << std::endl;
        std::cout << "3: " << (int)result[3] << std::endl;

  

        return result;
    }

    int decode_length(char* message_size_arr) {
        int message_size = 0;
        int shift_value = 24;

        //std::cout << "size string is " << parameter.substr(8, 12) << std::endl;

        for (int i=0; i<4; i++) {
            char x = (char)message_size_arr[i];
            //std::cout << "Summing: " << x << " with shift " << shift_value << std::endl;
            message_size += ((char)message_size_arr[i]) << shift_value;
            shift_value -= 8;
        }

        return ntohl(message_size);
    }
};