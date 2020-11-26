#pragma once
#include <string>
#include <map>
#include "ServerConnectionAsio.h"
#include <arpa/inet.h>
#include <future>
#include <vector>
#include <mutex>
#include "../common/Constants.h"
#include <condition_variable>
#include "CommandDTO.h"

struct IncomingCommand {
    std::string command_name;
    CommandDTO parameters;
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

    void dispatch_partial(std::string command, const CommandDTO parameters) {
        // std::cout << std::this_thread::get_id() << " ~~ " << "Acquiring lock in dispatch_partial " << command << std::endl;
        // lock acquire
        sc->send(command);

        // std::this_thread::sleep_for(std::chrono::milliseconds(4000));
        
        for (auto it = parameters.begin(); it != parameters.end(); it++ ) {
            send_parameter(it->first, it->second);
        }
    }

    std::future<CommandDTO> dispatch(std::string command, const CommandDTO& parameters) {
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

    

    std::future<CommandDTO> wait_for_response(std::string command, std::function<void(std::string)> fn = NULL) {
        return std::async([this, command, &fn]() {
            std::cout << command << " is trying to take lock in wfr" << std::endl;
            std::unique_lock ul(dispatch_mutex);
            std::cout << std::this_thread::get_id() << " ~~ " << "Acquiring lock in wait_for_response: " << command << std::endl;
            
            CommandDTO result;
            std::string received_command = sc->read_as_str(8);
            while(true) {
                std::string parameter_name = sc->read_as_str(8);

                if (parameter_name == "STOPFLOW") {
                    sc->read(4); // viene letta la dimensione che sarà sempre \0\0\0\0
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
                    ul.unlock();
                    std::cout << "unlocked and notified" << std::endl;
                    return cc_result.parameters;
                }
                int length = decode_length(sc->read(4));
                if (length > 1024) { // TODO: buffer size parametric
                    while (length != 0 && fn) {
                        int size_to_read = length > 1024 ? 1024 : length;
                        length -= size_to_read;
                        std::string parameter_value = sc->read_as_str(size_to_read);
                        fn(parameter_value); // TODO: valutare lock finché il client non ha finito di scrivere su disco
                    }
                } else {
                    std::string parameter_value = sc->read_as_str(length);
                    result.insert(std::pair<std::string, std::string>(parameter_name, parameter_value));
                }
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