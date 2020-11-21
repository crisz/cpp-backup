//
// Created by giuseppe on 08/11/20.
//

#pragma once
#include <boost/asio.hpp>

#include <map>
#include <memory>
#include "../common/Constants.h"
#include "SessionContainer.h"
#include "UserData.h"
#include "ServerConf.h"
using boost::asio::ip::tcp;

class MessageDispatcher {
private:
    UserData ud;
public:
    MessageDispatcher(tcp::socket& socket);

    void dispatch(std::string& command, std::map<std::string, std::string>& parameters);

    void dispatch (std::string& command, std::multimap<std::string, std::string>& parameters);

    void send_parameter(std::string key, std::string value) ;

    void send_STOPFLOW();

    char* encode_length(int size) ;

    int decode_length(char* message_size_arr);

};


