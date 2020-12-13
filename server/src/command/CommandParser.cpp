//
// Classe che si occupa dell'interpretazione di un comando ricevuto, attua le azione necessarie e infine invoca la dispatch
// in modo da mandare una risposta al client.
//

#include <boost/asio/post.hpp>
#include <boost/asio.hpp>
#include <string>
#include <memory>
#include <tgmath.h>
#include "CommandParser.h"

// Serve per ottenere il path locale di un file relativo alla directory associata ad un utente lato server
std::string CommandParser::get_file_path(tcp::socket &socket, ServerCommand &command) {
    auto parameters = command.get_parameters();
    std::string dest_dir = ServerConf::get_instance().dest;
    ConnectionsContainer& sc = ConnectionsContainer::get_instance();
    UserData ud = sc.get_user_data(socket);
    std::string file_path = dest_dir + ud.username + parameters[FILEPATH];
    return file_path;
}

// Si occupa dell'interpretazione di un comando ricevuto, attua le azioni necessarie e infine invoca la dispatch
// in modo da mandare una risposta al client.
void CommandParser::digest(tcp::socket &socket, ServerCommand &command) {
    std::string command_name=command.get_command_name();
    MessageDispatcher md{socket};
    auto parameters = command.get_parameters();

    if (command_name == LOGINSNC) {
        if(parameters.find(USERNAME) != parameters.end() && parameters.find(PASSWORD) != parameters.end()){
            LoginManager lm;
            std::string username = parameters[USERNAME];
            std::string password = parameters[PASSWORD];
            bool result = lm.check_login(username, password).get();
            std::map<std::string, std::string> result_map;
            result_map[__RESULT] = result ? "OK" : "KO";
            if (result) {
                ConnectionsContainer& sc = ConnectionsContainer::get_instance();
                UserData ud = sc.get_user_data(socket);
                ud.username = username;
                std::cout <<"L'utente "<< username << " si è connesso al server. Ci sono " << sc.get_number_users_connected() << " utenti connessi " << std::endl;
                sc.set_user_data(socket, ud);
            }
            md.dispatch(command_name,result_map);
            command.clear();
        } else error(socket);
        return;
    }
    if (command_name == SIGNUPNU) {
        if(parameters.find(USERNAME) != parameters.end() && parameters.find(PASSWORD) != parameters.end()){
            SignupManager sm;
            std::string username = parameters[USERNAME];
            std::string password = parameters[PASSWORD];
            bool result = sm.signup(username, password).get();
            std::map<std::string, std::string> result_map;
            result_map[__RESULT] = result ? "OK" : "KO";
            std::cout<<"Registrazione del nuovo utente " << (result? "avvenuta con successo": "fallita" )<< std::endl;
            md.dispatch(command_name,result_map);
            command.clear();
        } else error(socket);
        return;
    }

    if (command_name == REQRTREE ){
        TreeManager tm;
        std::string dest_dir = ServerConf::get_instance().dest;
        ConnectionsContainer& sc = ConnectionsContainer::get_instance();
        UserData ud = sc.get_user_data(socket);
        std::map<std::string, std::string> tree = tm.obtain_tree(dest_dir+ud.username).get();
        std::vector<std::pair<std::string, std::string>> req_tree_parameters;
        for (const auto& item: tree) {
            std::string file_path = item.first;
            std::string file_hash = item.second;
            req_tree_parameters.emplace_back(FILEHASH, file_hash);
            req_tree_parameters.emplace_back(FILEPATH, file_path);
        }
        std::cout<<"Invio del tree richiesto dal client"<<std::endl;
        md.dispatch(command_name, req_tree_parameters);
        command.clear();
        return;
    }
    if (command_name == REMVFILE) {
        if (parameters.find(FILEPATH) == parameters.end()) {
            error(socket);
            return;
        }
        std::string file_path = get_file_path(socket, command);
        RemovalManager rm;
        auto result = rm.remove_file(file_path).get();
        std::map<std::string, std::string> result_map;
        result_map[__RESULT] = result ? "OK" : "KO";
        std::cout<<"Rimozione del file "<< file_path << (result? " avvenuta con successo": " fallita" )<< std::endl;
        md.dispatch(command_name, result_map);
        command.clear();
        return;
    }

    if (command_name == REQRFILE) {
        if (parameters.find(FILEPATH) == parameters.end()) {
            error(socket);
            return;
        }
        std::string file_path = get_file_path(socket, command);
        BufferedFileReader bfr{BUFFER_SIZE, file_path};
        std::cout<<"Invio al client del file richiesto: " << file_path<< std::endl;
        md.send_command(command_name);
        md.send_chunk(FILEDATA, 8);
        md.send_chunk(encode_length(bfr.get_file_size()), 4);

        bfr.register_callback([&md, &bfr](bool done, char* data, int length){
            md.send_chunk(data, length);
            if (done) {
                md.stop_flow();
            }
            bfr.signal();
        });

        bfr.run();
        return;
    }

    // Non è entrato in nessun if, per cui non è stato riconosciuto il comando e viene lanciato un errore
    error(socket);
}

// Predispone l'invio di un file
void CommandParser::start_send_file(tcp::socket &socket, long file_size, ServerCommand &command) {
    auto parameters = command.get_parameters();
    std::string file_path = get_file_path(socket,command);
    std::string file_hash = parameters[FILEHASH];
    bfw = new BufferedFileWriter(file_path, file_size);
}

// Svuota il buffer su disco
std::future<void> CommandParser::send_file_chunk(char *buffer, int buffer_size) {
    if (bfw == nullptr) {
        throw std::logic_error("Parametri necessari. chiamare start_send_file prima");
    }
    return bfw->append(buffer, buffer_size);
}

// Termina la scrittura del file
bool CommandParser::end_send_file(tcp::socket &socket, ServerCommand &command) {
    delete bfw;
    std::string file_path = get_file_path(socket, command);
    std::string received_file_hash = command.get_parameters()[FILEHASH];
    std::string current_file_hash = hash_file(file_path);
    if (received_file_hash != current_file_hash){
        std::cout<<"File: " << file_path<< " corrotto, lo elimino!" << std::endl;
        RemovalManager rm;
        rm.remove_file(file_path);
        return false;
    }
    std::cout<<"File: " << file_path<< " aggiunto correttamente" << std::endl;
    return true;
}

// Stampa un errore e disconnette il client
void CommandParser::error(tcp::socket& socket) {
    std::cout << "Errore nel comando." << std::endl;
    socket.close();
}

// Viene invocato in caso di errore. Annulla l'invocazione di un comando pendente in caso di errore
void CommandParser::rollback_command(tcp::socket &socket, ServerCommand &command) { 
    if (command.get_command_name() == POSTFILE){
        RemovalManager rm;
        std::string dest_dir = ServerConf::get_instance().dest;
        ConnectionsContainer& sc = ConnectionsContainer::get_instance();
        UserData ud = sc.get_user_data(socket);
        rm.remove_file(dest_dir + ud.username + command.get_parameters()[FILEPATH]).get();
    }
}
