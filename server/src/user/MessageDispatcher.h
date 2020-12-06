//
// Created by giuseppe on 08/11/20.
// Classe che permette l'invio dei comandi e dei relativi parametri dal server verso il client
//

#pragma once
#include <boost/asio.hpp>

#include <map>
#include <memory>
#include "common/Constants.h"
#include "common/encode_length_utils.h"
#include "server/src/command/SessionContainer.h"
#include "UserData.h"
#include "server/src/pool/ServerConf.h"
using boost::asio::ip::tcp;

class MessageDispatcher {
private:
    UserData ud;
public:
    MessageDispatcher(tcp::socket& socket);

    void dispatch(std::string& command, std::map<std::string, std::string>& parameters);

    void dispatch(std::string& command, std::multimap<std::string, std::string>& parameters);

    void dispatch(std::string& command, std::vector<std::pair<std::string, std::string>>& parameters);

    void send_command(std::string& commands);

    void send_parameter(std::string key, std::string value);

    void send_chunk(const char* chunk_data, int chunk_length);

    void stop_flow();

};


