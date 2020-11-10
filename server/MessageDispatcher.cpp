//
// Created by giuseppe on 08/11/20.
//

#include "MessageDispatcher.h"
#include "SingleUserConnection.h"

MessageDispatcher::MessageDispatcher(std::shared_ptr<SingleUserConnection> suc): suc{suc}{}

void MessageDispatcher:: dispatch (std::string& command, std::map<std::string, std::string>& parameters){

    suc->send_response(command);

    for (auto it = parameters.begin(); it != parameters.end(); it++ ) {
        send_parameter(it->first, it->second);
    }

    send_parameter(STOPFLOW, "");

}

void MessageDispatcher:: dispatch_tree (std::string& command, std::map<std::string, std::string>& tree){

    suc->send_response(command);

    for (auto it = tree.begin(); it != tree.end(); it++ ) {
        //send_parameter(FILEPATH,it->first);
        //send_parameter(FILEHASH, it->second);
    }

    send_parameter(STOPFLOW, "");

}

void MessageDispatcher::send_parameter(std::string key, std::string value) {
    std::cout << "Sending parameter with key " << key << " and value " << value << std::endl;

    suc->send_response(key);
    suc->send_response(encode_length(value.size()), 4);
    suc->send_response(value);
}

char* MessageDispatcher::encode_length(int size) {
    char* result = new char[4];
    int length = htonl(size); // htonl serve per non avere problemi di endianess
    result[3] = (length & 0xFF);
    result[2] = (length >> 8) & 0xFF;
    result[1] = (length >> 16) & 0xFF;
    result[0] = (length >> 24) & 0xFF;
    return result;
}

int MessageDispatcher::decode_length(char* message_size_arr) {
    int message_size = 0;
    int shift_value = 24;

    //std::cout << "size string is " << parameter.substr(8, 12) << std::endl;

    for (int i=0; i<4; i++) {
        char x = (char)message_size_arr[i];
        //std::cout << "Summing: " << x << " with shift " << shift_value << std::endl;
        message_size += ((char)message_size_arr[i]) << shift_value;
        shift_value -= 8;
    }

    return message_size;
}
