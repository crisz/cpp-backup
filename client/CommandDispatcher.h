#include <string>
#include <map>
#include "ServerConnectionAsio.h"
#include <arpa/inet.h>
#include <future>
#include <vector>
#include <mutex>
#include "../common/Constants.h"
class CommandDispatcher {
private:
    std::shared_ptr<ServerConnectionAsio> sc;
    std::vector<std::string> pending_commands;
    std::mutex dispatch_mutex;
    std::condition_variable cv;
public:
    CommandDispatcher() {
        sc = ServerConnectionAsio::get_instance();
    }

    void dispatch_partial(std::string command, const std::multimap<std::string, std::string> parameters) {
        std::unique_lock ul(dispatch_mutex);

        std::cout<<std::this_thread::get_id() << " ~~ " << "Sending command " << command << std::endl;
        // lock acquire
        sc->send(command);

        // std::this_thread::sleep_for(std::chrono::milliseconds(4000));
        
        for (auto it = parameters.begin(); it != parameters.end(); it++ ) {
            send_parameter(it->first, it->second);
        }
    }

    std::future<std::multimap<std::string, std::string>> dispatch(std::string command, const std::multimap<std::string, std::string>& parameters) {
        std::unique_lock ul(dispatch_mutex); 

        // copia e incolla dispatch_partial. Soluzione: recursive_mutex
        std::cout<<std::this_thread::get_id() << " ~~ " << "Sending command " << command << std::endl;
        // lock acquire
        sc->send(command);

        // std::this_thread::sleep_for(std::chrono::milliseconds(4000));
        
        for (auto it = parameters.begin(); it != parameters.end(); it++ ) {
            send_parameter(it->first, it->second);
        }

        // fine copia e incolla
        send_parameter("STOPFLOW", "");
        ul.unlock();
        return wait_for_response(command);
    }

    std::future<std::multimap<std::string, std::string>> wait_for_response(std::string command) {
        return std::async([this, command]() {
            std::unique_lock ul(dispatch_mutex);
            std::cout<<std::this_thread::get_id() << " ~~ " << command << " is waiting for response" << std::endl;

            std::multimap<std::string, std::string> result;
            std::string received_command = sc->read_as_str(8);

            std::cout<<std::this_thread::get_id() << " ~~ " << "Thread " << std::this_thread::get_id() << " is about to wait " << std::endl;
            std::cout<<std::this_thread::get_id() << " ~~ " << "received_command is " << received_command << " while command is " << command << std::endl; 
            cv.wait(ul, [&command, &received_command]() { return command == received_command; });
            std::cout<<std::this_thread::get_id() << " ~~ " << "Thread " << std::this_thread::get_id() << " exited from wait " << std::endl;
            
            std::cout<<std::this_thread::get_id() << " ~~ " << "Starting listening" << std::endl;

            while(true) {
                std::string parameter_name = sc->read_as_str(8);
                std::cout<<std::this_thread::get_id() << " ~~ " << "Received parameter " << parameter_name << std::endl;

                if (parameter_name == "STOPFLOW") {
                    std::cout<<std::this_thread::get_id() << " ~~ " << "Thread " << std::this_thread::get_id() << " finished " << std::endl;
                    //leggiamo 4 caratteri perché il server manda STOPFLOW0000
                    sc->read(4); // TODO: risolvere in maniera alternativa
                    cv.notify_all();
                    for (const auto &x: result)
                        std::cout<<std::this_thread::get_id() << " ~~ "<< x.first <<":"<< x.second << std::endl;
                    return result;
                }
                int length = decode_length(sc->read(4));
                std::string parameter_value = sc->read_as_str(length);

                std::cout<<std::this_thread::get_id() << " ~~ " << "Received parameter value " << parameter_value << std::endl;
                result.insert(std::pair<std::string, std::string>(parameter_name, parameter_value));
            }
        });
    }

    void send_raw(const char* raw_data, int size) {
        std::cout<<std::this_thread::get_id() << " ~~ " << "Sending RAW " << raw_data << std::endl;
        sc->send(raw_data, size);
    }

    void send_parameter(std::string key, std::string value) {
        std::cout<<std::this_thread::get_id() << " ~~ " << "Sending parameter with key " << key << " and value " << value << std::endl;

        sc->send(key);
        sc->send(encode_length(value.size()), 4);
        sc->send(value);
    }

    char* encode_length(long size) {
        std::cout<<std::this_thread::get_id() << " ~~ " << "ecndoing length " << size << std::endl;
        char* result = new char[4];
        long length = htonl(size); // htonl serve per non avere problemi di endianess
        result[3] = (length & 0xFF);
        result[2] = (length >> 8) & 0xFF;
        result[1] = (length >> 16) & 0xFF;
        result[0] = (length >> 24) & 0xFF;
        std::cout<<std::this_thread::get_id() << " ~~ " << "0: " << (int)result[0] << std::endl;
        std::cout<<std::this_thread::get_id() << " ~~ " << "1: " << (int)result[1] << std::endl;
        std::cout<<std::this_thread::get_id() << " ~~ " << "2: " << (int)result[2] << std::endl;
        std::cout<<std::this_thread::get_id() << " ~~ " << "3: " << (int)result[3] << std::endl;

        return result;
    }

    long decode_length(char* message_size_arr) {
        long message_size = 0;
        int shift_value = 24;

        //std::cout<<std::this_thread::get_id() << " ~~ " << "size string is " << parameter.substr(8, 12) << std::endl;

        for (int i=0; i<4; i++) {
            char x = (char)message_size_arr[i];
            //std::cout<<std::this_thread::get_id() << " ~~ " << "Summing: " << x << " with shift " << shift_value << std::endl;
            message_size += ((char)message_size_arr[i]) << shift_value;
            shift_value -= 8;
        }

        return ntohl(message_size);
    }
};