//
// Created by giuseppe on 08/11/20.
// Classe che permette l'invio dei comandi e dei relativi parametri dal server verso il client
//

#include "MessageDispatcher.h"

//Il costruttore ottiene tutte le informazioni dell'utente associato alla socket corrente
MessageDispatcher::MessageDispatcher(tcp::socket& socket) {
    this->ud = ConnectionsContainer::get_instance().get_user_data(socket);
}

//Le tre funzioni distach seguenti si occupano dell'invio dei comandi e dei paramentri verso il client
void MessageDispatcher::dispatch(std::string& command, std::map<std::string, std::string>& parameters){
    auto pbeg = parameters.begin();
    auto pend = parameters.end();
    std::multimap<std::string, std::string> multimap_parameters = std::multimap<std::string, std::string>(pbeg, pend);
    return dispatch(command, multimap_parameters);
}

//Questo metodo in particolare si occupa dell'invio dell'albero da parte del server
void MessageDispatcher::dispatch(std::string& command, std::vector<std::pair<std::string,std::string>>& parameters){
    this->send_command(command);

    for (auto it = parameters.begin(); it != parameters.end(); it++ ) {
        send_parameter(it->first, it->second);
    }
    this->stop_flow();
}

void MessageDispatcher::dispatch(std::string& command, std::multimap<std::string, std::string>& parameters){
    this->send_command(command);

    for (auto it = parameters.begin(); it != parameters.end(); it++ ) {
        this->send_parameter(it->first, it->second);
    }
    this->stop_flow();
}

//Serve ad inviare il nome del comando
void MessageDispatcher::send_command(std::string& command) {
    ud.send_response_callback(command);
}

//Serve ad inviare un particolare parametro con il rispettivo formato TLV
void MessageDispatcher::send_parameter(std::string key, std::string value) {
    std::cout << "Sending parameter with key " << key << " and value " << value << std::endl;
    ud.send_response_callback(key);
    ud.send_raw_response_callback(encode_length(value.size()), 4);
    ud.send_response_callback(value);
}

//Serve ad inviare dei dati raw (es. la lunghezza del valore e il valore stesso di un parametro)
void MessageDispatcher::send_chunk(const char* chunk_data, int chunk_length) {
    ud.send_raw_response_callback(chunk_data, chunk_length);
}

//Serve ad inviare il parametro di terminazione di un comando
void MessageDispatcher::stop_flow() {
    send_parameter("STOPFLOW", "");
}
