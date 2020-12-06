//
// Classe contente tutte le informazioni relative al comando ricevuto o inviato dal server.
//
#pragma once

#include "server/src/user/SingleUserConnection.h"
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

    void setName(std::string name){
        command_name=name;
    }
    void addParameter(std::string par_name, std::string par_value){
        parameters.insert(std::pair<std::string, std::string>(par_name, par_value));
    }
    std::string get_command_name(){
        return command_name;
    }
    std::map<std::string, std::string> getParameters(){
        return parameters;
    }
    void clear (){
        command_name="";
        parameters.clear();
    }

};