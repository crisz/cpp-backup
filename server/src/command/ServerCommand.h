//
// Classe contenente tutte le informazioni relative al comando ricevuto o inviato dal server.
//
#pragma once

#include <string>
#include <map>
#include <boost/filesystem.hpp>
#include "common/hash_file.h"
#include "server/src/command/LoginManager.h"
#include "server/src/user/MessageDispatcher.h"
#include "common/Constants.h"

class ServerCommand {
private:
    std::string command_name;
    std::map<std::string, std::string> parameters;

public:

    void set_name(std::string name){
        command_name = name;
    }
    void add_parameter(const std::string& par_name, const std::string& par_value){
        parameters.insert(std::pair<std::string, std::string>(par_name, par_value));
    }
    std::string get_command_name(){
        return command_name;
    }
    std::map<std::string, std::string> get_parameters(){
        return parameters;
    }
    void clear() {
        command_name = "";
        parameters.clear();
    }

};