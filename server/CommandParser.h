#pragma once

#include "SingleUserConnection.h"
#include "MessageDispatcher.h"
#include "ServerCommand.h"



class CommandParser {
public:
    void handleCommand(std::shared_ptr<SingleUserConnection> suc, ServerCommand& command) {  // TODO: digest
        // TODO: risolvere il problema del this (ad es. spostando la logica in command parser)
        std::string command_name=command.getCommand_name();
        auto parameters = command.getParameters();
        if (command_name == LOGINSNC) {
            if(parameters.find("USERNAME") != parameters.end() && parameters.find("PASSWORD") != parameters.end()){
                LoginManager lm;
                std::string username = parameters[USERNAME];
                std::string password = parameters[PASSWORD];
                bool result = lm.check_login(username, password).get();
                std::map<std::string, std::string> result_map;
                result_map[__RESULT] = result ? "OK" : "KO";
                MessageDispatcher md{suc};
                md.dispatch(command_name,result_map);
                command.clear();
            }else error();

        }else if(command_name == REQRTREE){

        } else if(command_name== POSTFILE) {
            if(parameters.find(FILEPATH) != parameters.end() &&
               parameters.find(FILEDATA) != parameters.end() &&
               parameters.find(FILEHASH) != parameters.end()){

                // PostFileManager pfm;
                std::string file_path = parameters[FILEPATH];
                std::string file_data = parameters[FILEDATA];
                std::string file_hash = parameters[FILEHASH];
                // bool result = pfm.check_post().get();
                std::map<std::string, std::string> result_map;
                // result_map[__RESULT] = result ? "OK" : "KO";
                MessageDispatcher md{suc};
                md.dispatch(command_name,result_map);
                command.clear();

            }else error();

        }else if(command_name == REMVFILE){
            if(parameters.find(FILEPATH)!=parameters.end()){

            }else error();
        }

    }


    void sendCredentials(std::string & username ,std::string & password ){
        std::cout<<"Credenziali ricevute us: "<<username << " pass: "<< password<<std::endl;
    }
    void error(){
        std::cout<<"Errore nel comando"<<std::endl;
    }
    void sendResult(std::string & r){}

    std::string sendTree(std::string const& path) {
        std::map<std::string, std::string> local_tree;
        for(auto &file : boost::filesystem::recursive_directory_iterator(path)) {
            local_tree[file.path().string()] = hash_file(file.path().string());
        }
        std::cout<<"sto mandando sto"<<std::endl;

        std::string buffer;
        // boost::iostreams::back_insert_device<std::string> inserter(buffer);
        // boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> ostr(inserter);
        // boost::archive::binary_oarchive oa(ostr);
        // oa << local_tree;
        // ostr.flush();
        // std::stringstream stream;
        // stream <<"REQRTREERESPTREE"<<std::hex <<std::setfill('0')<<std::setw(4)<<buffer.size()<<buffer<<"STOPFLOW0000";
        // std::string result( stream.str());
        // std::cout<<buffer.size()<<std::endl;
        // std::cout<<result<<std::endl;
        return buffer;
    }

    void saveFile(std::string & path ,std::string & data,std::string & hash){}
    void removeFile(std::string & path){}
    void saveTree(){}

};