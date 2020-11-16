//
// Created by giuseppe on 08/11/20.
//

#include "MessageDispatcher.h"
#include "SingleUserConnection.h"

MessageDispatcher::MessageDispatcher(tcp::socket& socket) {
    this->ud = SessionContainer::get_instance().get_user_data(socket);
}

void MessageDispatcher:: dispatch(std::string& command, std::map<std::string, std::string>& parameters){
    auto pbeg = parameters.begin();
    auto pend = parameters.end();
    std::multimap<std::string, std::string> multimap_parameters = std::multimap<std::string, std::string>(pbeg, pend);
    return dispatch(command, multimap_parameters);
}

void MessageDispatcher::dispatch(std::string& command, std::multimap<std::string, std::string>& parameters){

    ud.send_response_callback(command);

    for (auto it = parameters.begin(); it != parameters.end(); it++ ) {
        send_parameter(it->first, it->second);
    }

    send_parameter(STOPFLOW, "");

}

void MessageDispatcher::send_parameter(std::string key, std::string value) {
    std::cout << "Sending parameter with key " << key << " and value " << value << std::endl;

    ud.send_response_callback(key);
    ud.send_raw_response_callback(encode_length(value.size()), 4);
    ud.send_response_callback(value);
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
