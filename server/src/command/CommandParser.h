#pragma once

#include "server/src/user/SingleUserConnection.h"
#include "server/src/user/MessageDispatcher.h"
#include "server/src/pool/ServerCommand.h"
#include "TreeManager.h"
#include "common/BufferedFileWriter.h"
#include "common/BufferedFileReader.h"
#include "common/encode_length_utils.h"
#include <map>
#include <boost/asio.hpp>
#include <memory>
#include "SessionContainer.h"
#include "server/src/user/UserData.h"
#include "server/src/pool/ServerConf.h"
#include "RemovalManager.h"

// LOGINSNC USERNAME _jD1 PEPPE PASSWORD 003 ABC STOPFLOW
// LOGINSNC __RESULT 0002 OK STOPFLOW

// REQRTREE
// REQRTREE FILEHASH ABC9 <HASH> FILEPATH 0123 <FULL PATH> FILEHASH A123 <HASH> FILEPATH FILEHASH

// POSTFILE FILEPATH 0123 <FULL PATH> FILEDATA <FULL DATA> FILEHASH A123 <HASH>
// POSTFILE __RESULT 0002 OK

// REMVFILE FILEPATH 0123 <FULL PATH>
// REMVFILE __RESULT 0002 OK

using boost::asio::ip::tcp;

class CommandParser {
private:
    BufferedFileWriter* bfw;

    std:: string get_file_path(tcp::socket& socket, ServerCommand& command) {
        auto parameters = command.getParameters();
        std::string dest_dir = ServerConf::get_instance().dest;
        SessionContainer& sc = SessionContainer::get_instance();
        UserData ud = sc.get_user_data(socket);
        std::string file_path = dest_dir+ud.username+parameters[FILEPATH];
        return file_path;
    }

public:

    void digest(tcp::socket& socket, ServerCommand& command) {
        std::string command_name=command.get_command_name();
        MessageDispatcher md{socket};
        auto parameters = command.getParameters();
        if (command_name == LOGINSNC) {
            if(parameters.find(USERNAME) != parameters.end() && parameters.find(PASSWORD) != parameters.end()){
                LoginManager lm;
                std::string username = parameters[USERNAME];
                std::string password = parameters[PASSWORD];
                bool result = lm.check_login(username, password).get();
                std::map<std::string, std::string> result_map;
                result_map[__RESULT] = result ? "OK" : "KO";
                if (result) {
                    SessionContainer& sc = SessionContainer::get_instance();
                    UserData ud = sc.get_user_data(socket);
                    ud.username = username;
                    std::cout << username << " si è connesso al server. Ci sono " << sc.get_number_users_connected() << " utenti connessi " << std::endl;
                    sc.set_user_data(socket, ud);
                }

                md.dispatch(command_name,result_map);
                command.clear();
            } else error();
            return;
        } 

        if (command_name == REQRTREE ){
            TreeManager tm;
            std::cout << boost::filesystem::current_path() << std::endl;
            std::string dest_dir = ServerConf::get_instance().dest;
            SessionContainer& sc = SessionContainer::get_instance();
            UserData ud = sc.get_user_data(socket);
            std::map<std::string, std::string> tree = tm.obtain_tree(dest_dir+ud.username).get();
            std::vector<std::pair<std::string, std::string>> parameters;
            for (std::pair<std::string, std::string> item: tree) {
                std::string file_path = item.first;
                std::string file_hash = item.second;
                parameters.push_back(std::pair<std::string, std::string>("FILEHASH", file_hash));
                parameters.push_back(std::pair<std::string, std::string>("FILEPATH", file_path));
            }

            md.dispatch(command_name, parameters);
            command.clear();
            return;
        }
        if (command_name== POSTFILE) {
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
                // SingleUserConnection suc;

                // MessageDispatcher md{suc};
                md.dispatch(command_name,result_map);
                command.clear();

            } else error();
            return;
        }
        if (command_name == REMVFILE) {
            if (parameters.find(FILEPATH) == parameters.end()) {
                error();
                return;
            }
            std::string file_path = get_file_path(socket, command);
            RemovalManager rm;
            auto result = rm.remove_file(file_path).get();
            std::map<std::string, std::string> result_map;
            result_map[__RESULT] = result ? "OK" : "KO";
            md.dispatch(command_name, result_map);
            command.clear();
            return;
        }
        
        if (command_name == "REQRFILE") {
            std::cout << "REQRFILE CALLED!" << std::endl;
            if (parameters.find(FILEPATH) == parameters.end()) {
                error();
                return;
            }

            std::cout << "getting file path" << std::endl;
            std::string file_path = get_file_path(socket, command);
            std::cout << "file path is " << file_path << std::endl;

            std::cout << "constructing bfr "  << std::endl;
            BufferedFileReader bfr{10, file_path}; // TODO: mettere in una costante (mettere 1024)
            std::cout << "bfr constructed"  << std::endl;

            std::cout << "sending command name" << std::endl;
            md.send_command(command_name);
            
            std::cout << "sending chunk filedata" << std::endl;

            md.send_chunk("FILEDATA", 8);

            std::cout << "sending length" << std::endl;

            md.send_chunk(encode_length(bfr.get_file_size()), 4);

            bfr.register_callback([&md, &bfr](bool done, char* data, int length){
                std::cout << "chunk received: " << data << std::endl;

                md.send_chunk(data, length);
                if (done) {
                    md.stop_flow();
                }
                bfr.signal();
            });
            
            bfr.run();
            return;
        }

    }

    void start_send_file(tcp::socket& socket, long file_size, ServerCommand& command) { // TODO: La gestione dei comandi va unificato in una classe. In queto momento sia CommandParser che SingleUserConnection stanno concorrendo alla gestione
        auto parameters = command.getParameters();
        std::string file_path = get_file_path(socket,command);
        std::cout<<"FILEPATH!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<< file_path<<std::endl;
        std::string file_hash = parameters[FILEHASH];
        bfw = new BufferedFileWriter(file_path, file_size);
    }

    std::future<void> send_file_chunk(char* buffer, int buffer_size) {
        if (bfw == nullptr) {
            throw "chiamare start_send_file prima"; // TODO std::exception
        }
        return bfw->append(buffer, buffer_size);
    }

    void end_send_file(tcp::socket& socket, ServerCommand& command) {
        delete bfw;

        std::string file_path = get_file_path(socket, command);
        std::string receved_file_hash = command.getParameters()[FILEHASH];
        std::string current_file_hash = hash_file(file_path);
        std::cout<< "Hash ricevuto: " << receved_file_hash<< std::endl;
        std::cout<< "Hash corrente: " << current_file_hash<< std::endl;
        if(receved_file_hash!=current_file_hash){
            std::cout<<"File: "<< file_path<< " corrotto, lo elimino!"<< std::endl;
            RemovalManager rm;
            rm.remove_file(file_path);
        }
    }

    void error(){
        std::cout << "Errore nel comando " << std::endl; // TODO
    }
    void sendResult(std::string & r){}

    std::string sendTree(std::string const& path) {
        std::map<std::string, std::string> local_tree;
        for(auto &file : boost::filesystem::recursive_directory_iterator(path)) {
            local_tree[file.path().string()] = hash_file(file.path().string());
        }

        std::string buffer;
        return buffer;
    }

    void rollback_command(tcp::socket& socket, ServerCommand& command){
        if(command.get_command_name() == "POSTFILE"){
            std::string file_path = get_file_path(socket, command);
            RemovalManager rm;
            rm.remove_file(file_path);
        }
    }


};