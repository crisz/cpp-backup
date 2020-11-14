#pragma once

#include "./SingleUserConnection.h"
#include <string>
#include <map>
#include <boost/filesystem.hpp>
#include "../common/hash_file.h"
#include "LoginManager.h"
#include "MessageDispatcher.h"
#include "../common/Constants.h"

class ServerCommand {
private:
    std::string command_name;
    std::map<std::string, std::string> parameters;

public:

    void setName(std::string name){
        command_name=name;
    }
    void addParameter(std::string par_name, std::string par_value){
        parameters[par_name]=par_value;
    }
    std::string getCommand_name(){
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