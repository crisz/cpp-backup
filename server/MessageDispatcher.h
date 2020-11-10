//
// Created by giuseppe on 08/11/20.
//

#pragma once


#include <map>
#include <memory>
#include "../common/Constants.h"
class SingleUserConnection;

class MessageDispatcher {

std::shared_ptr<SingleUserConnection> suc;
public:
    MessageDispatcher(std::shared_ptr<SingleUserConnection>);

    void dispatch (std::string& command, std::map<std::string, std::string>& parameters);

    void dispatch_tree (std::string& command, std::map<std::string, std::string>& tree);

    void send_parameter(std::string key, std::string value) ;

    char* encode_length(int size) ;

    int decode_length(char* message_size_arr);

};


