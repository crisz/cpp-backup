//
// Classe che si occupa della richiesta di esecuzione dei comandi sul server e attende il risultato
//

#pragma once
#include <string>
#include <future>
#include <map>
#include <functional>
#include "client/src/file/FileMetadata.h"
#include "CommandDispatcher.h"
#include "common/Constants.h"
#include "common/BufferedFileReader.h"
#include "common/BufferedFileWriter.h"
#include "common/encode_length_utils.h"
#include <mutex>
#include "CommandDTO.h"

#define FILE_BUFFER_SIZE 512

class ClientCommand {
private:
    std::string command;
    CommandDispatcher cd;
    CommandDTO parameters;

public:
    ClientCommand();

    // Funzione che invia le credenziali per il login e ritorna la risposta del server
    std::future<bool> login(std::string username, std::string password);

    // Funzione che invia le credenziali per il signup e ritorna la risposta del server
    std::future<bool> signup(std::string username, std::string password);

    // Funzione richiede al server il tree relativo all'utente loggato e, una volta ricevuto, lo ritorna
    std::future<std::vector<FileMetadata>> require_tree();

    // Funzione che invia un file al server e ritorna un bool in base all'esito
    std::future<bool> post_file(FileMetadata& file_metadata, const int buffer_size=256);

    // Funzione che si occupa di richiedere la rimozione di un file sul server e ritorna la risposta del server
    std::future<bool> remove_file(FileMetadata& file_metadata);

    // Funzione che si occupa della richiesta di un file dal server e ritorna un bool in base al fatto
    // che il file ricevuto sia stato corrotto o  meno
    std::future<bool> require_file(FileMetadata& file_metadata);
};

