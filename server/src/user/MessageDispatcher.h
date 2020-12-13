//
// Created by giuseppe on 08/11/20.
//
// Classe che permette l'invio dei comandi e dei relativi parametri dal server verso il client
//

#pragma once
#include <boost/asio.hpp>

#include <map>
#include <memory>
#include "common/Constants.h"
#include "common/encode_length_utils.h"
#include "server/src/pool/ConnectionsContainer.h"
#include "UserData.h"
#include "server/src/pool/ServerConf.h"
using boost::asio::ip::tcp;

class MessageDispatcher {
private:
    UserData ud;
public:
    // Il costruttore ottiene tutte le informazioni dell'utente associato alla socket corrente
    MessageDispatcher(tcp::socket& socket);

    // Le tre funzioni dispatch seguenti si occupano dell'invio dei comandi e dei paramentri verso il client
    void dispatch(std::string& command, std::map<std::string, std::string>& parameters);

    // Questo metodo in particolare si occupa dell'invio dell'albero da parte del server
    void dispatch(std::string& command, std::multimap<std::string, std::string>& parameters);

    void dispatch(std::string& command, std::vector<std::pair<std::string, std::string>>& parameters);

    // Serve ad inviare il nome del comando
    void send_command(std::string& commands);

    // Serve ad inviare un particolare parametro con il rispettivo formato TLV
    void send_parameter(std::string key, std::string value);

    // Serve ad inviare dei dati raw (es. la lunghezza del valore e il valore stesso di un parametro)
    // La necessità di questa funzione è dovuto al fatto che std::string ignora i caratteri non stampabili
    void send_chunk(const char* chunk_data, int chunk_length);

    // Serve ad inviare il parametro di terminazione di un comando
    void stop_flow();

};


