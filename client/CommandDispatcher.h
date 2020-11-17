#include <string>
#include <map>
#include "ServerConnectionAsio.h"
#include <arpa/inet.h>
#include <future>
#include <vector>
#include <mutex>
#include "../common/Constants.h"
#include <condition_variable>

struct IncomingCommand {
    std::string command_name;
    std::multimap<std::string, std::string> parameters;
};
class CommandDispatcher {
private:
    std::shared_ptr<ServerConnectionAsio> sc;
    std::vector<IncomingCommand> pending_commands;
    std::mutex dispatch_mutex;
    std::condition_variable cv;

public:
    CommandDispatcher() {
        sc = ServerConnectionAsio::get_instance();
    }

    void dispatch_partial(std::string command, const std::multimap<std::string, std::string> parameters) {
        // std::cout << std::this_thread::get_id() << " ~~ " << "Acquiring lock in dispatch_partial " << command << std::endl;
        // lock acquire
        sc->send(command);

        // std::this_thread::sleep_for(std::chrono::milliseconds(4000));
        
        for (auto it = parameters.begin(); it != parameters.end(); it++ ) {
            send_parameter(it->first, it->second);
        }
    }

    std::future<std::multimap<std::string, std::string>> dispatch(std::string command, const std::multimap<std::string, std::string>& parameters) {
        std::cout << command << " is trying to acquire lock" << std::endl;
        std::unique_lock ul(dispatch_mutex); 
        std::cout << std::this_thread::get_id() << " ~~ " << "Acquiring lock in dispatch " << command << std::endl;


        // copia e incolla dispatch_partial. Soluzione: recursive_mutex
        
        // lock acquire
        sc->send(command);

        // std::this_thread::sleep_for(std::chrono::milliseconds(4000));
        
        for (auto it = parameters.begin(); it != parameters.end(); it++ ) {
            send_parameter(it->first, it->second);
        }

        // fine copia e incolla
        send_parameter("STOPFLOW", "");
        std::cout << std::this_thread::get_id() << " ~~ " << "Releasing lock in dispatch " << command << std::endl;
        ul.unlock();
        return wait_for_response(command);
    }

    

    std::future<std::multimap<std::string, std::string>> wait_for_response(std::string command) {
        return std::async([this, command]() {
            std::unique_lock ul(dispatch_mutex);
            std::cout << std::this_thread::get_id() << " ~~ " << "Acquiring lock in wait_for_response: " << command << std::endl;
            
            std::multimap<std::string, std::string> result;
            std::string received_command = sc->read_as_str(8);
            while(true) {
                std::string parameter_name = sc->read_as_str(8);

                if (parameter_name == "STOPFLOW") {
                    
                    //leggiamo 4 caratteri perché il server manda STOPFLOW0000
                    sc->read(4); // TODO: risolvere in maniera alternativa
                    IncomingCommand cc;
                    cc.command_name = received_command;
                    cc.parameters = result;
                    this->pending_commands.push_back(cc);
                    cv.wait(ul, [&command, this]() {
                        for (auto pending_command: this->pending_commands) {
                            if (pending_command.command_name == command) return true;
                        }
                        return false;
                    });
                    IncomingCommand cc_result;
                    for (auto pending_command: this->pending_commands) {
                        if (pending_command.command_name == command) {
                            cc_result = pending_command;
                            break;
                        }
                    }

                    std::cout << std::this_thread::get_id() << " ~~ " << "Releasing lock in wait_for_response: " << command << std::endl;
                    cv.notify_all();
                    return cc_result.parameters;
                }
                int length = decode_length(sc->read(4));
                std::string parameter_value = sc->read_as_str(length);

                
                result.insert(std::pair<std::string, std::string>(parameter_name, parameter_value));
            }
        });
    }

    void lock_raw() {
        std::cout << std::this_thread::get_id() << " ~~ " << "Acquiring lock raw" << std::endl;
        this->dispatch_mutex.lock();
    }
    
    void unlock_raw() {
        std::cout << std::this_thread::get_id() << " ~~ " << "Releasing lock raw" << std::endl;
        this->dispatch_mutex.unlock();
    }

    void send_raw(const char* raw_data, int size) {
        sc->send(raw_data, size);
    }

    void send_parameter(std::string key, std::string value) {

        sc->send(key);
        sc->send(encode_length(value.size()), 4);
        sc->send(value);
    }

    char* encode_length(long size) {
        
        char* result = new char[4];
        long length = htonl(size); // htonl serve per non avere problemi di endianess
        result[3] = (length & 0xFF);
        result[2] = (length >> 8) & 0xFF;
        result[1] = (length >> 16) & 0xFF;
        result[0] = (length >> 24) & 0xFF;
        
        return result;
    }

    long decode_length(char* message_size_arr) {
        long message_size = 0;
        int shift_value = 24;

        for (int i=0; i<4; i++) {
            char x = (char)message_size_arr[i];
            //
            message_size += ((char)message_size_arr[i]) << shift_value;
            shift_value -= 8;
        }

        return ntohl(message_size);
    }
};