//
// Classe che si occupa della richiesta di esecuzione dei comandi sul server e attende il risultato
//

#include <common/hash_file.h>
#include "ClientCommand.h"

ClientCommand::ClientCommand() {
}

// Funzione che invia le credenziali per il login e ritorna la risposta del server
std::future<bool> ClientCommand::login(std::string username, std::string password) {
    std::string command;
    CommandDTO parameters;
    parameters.erase();
    command = LOGINSNC;
    parameters.insert(std::pair<std::string, std::string>(USERNAME, username));
    parameters.insert(std::pair<std::string, std::string>(PASSWORD, password));
    return std::async([this, command, parameters] () {
        CommandDTO result = cd.dispatch(command, parameters).get();
        bool success = result.find((__RESULT)).second == "OK";
        if (!success) {
            std::cerr << result.find(ERRORMSG).second << std::endl;
        }
        return success;
    });
}

// Funzione che invia le credenziali per il signup e ritorna la risposta del server
std::future<bool> ClientCommand::signup(std::string username, std::string password) {
    std::string command;
    CommandDTO parameters;
    parameters.erase();
    command = SIGNUPNU;
    parameters.insert(std::pair<std::string, std::string>(USERNAME, username));
    parameters.insert(std::pair<std::string, std::string>(PASSWORD, password));
    return std::async([this, command, parameters] () {
        CommandDTO result = cd.dispatch(command, parameters).get();
        return result.find((__RESULT)).second == "OK";
    });
}

// Funzione richiede al server il tree relativo all'utente loggato e, una volta ricevuto, lo ritorna
std::future<std::vector<FileMetadata>> ClientCommand::require_tree() {
    std::string command;
    CommandDTO parameters;
    command = REQRTREE;
    parameters.erase();
    return std::async([this, command, parameters] () {
        std::vector<FileMetadata> tree;
        CommandDTO result = cd.dispatch(command, parameters).get();
        FileMetadata fm;

        for (const std::pair< const std::string, std::string>& item: result) {
            if (fm.hash != "" && fm.path != "") {
                tree.push_back(fm);
                fm.hash.clear();
                fm.path.clear();
            }
            if (item.first == FILEHASH) {
                fm.hash = item.second;
                continue;
            }
            if (item.first == FILEPATH) {
                fm.path = item.second;
                fm.path_to_send = fm.path;

                std::size_t found = fm.path.find_last_of("/\\");
                continue;
            }
        }
        if (fm.hash != "" && fm.path != "") {
            tree.push_back(fm);
        }
        return tree;
    });
}

// Funzione che invia un file al server e ritorna un bool in base all'esito
std::future<bool> ClientCommand::post_file(FileMetadata &file_metadata, const int buffer_size) {

    try {
        BufferedFileReader bfm{FILE_BUFFER_SIZE, file_metadata.path}; // RAII
        std::string command;
        CommandDTO parameters;
        command = POSTFILE;
        parameters.erase();
        parameters.insert(std::pair<std::string, std::string>(FILEPATH, file_metadata.path_to_send));
        parameters.insert(std::pair<std::string, std::string>(FILEHASH, file_metadata.hash));

        cd.lock_raw();
        cd.dispatch_partial(command, parameters);
        cd.send_raw(FILEDATA, 8);

        cd.send_raw(encode_length(bfm.get_file_size()), 4);

        // viene registrata la calback che verrà invocata ogni volta che verranno letti "bytes_read" byte di un file
        std::promise<bool> &read_done = bfm.register_callback([&bfm, this](bool done, char *data, int bytes_read) {
            this->cd.send_raw(data, bytes_read);
            bfm.signal();
        });

        // viene avviata la lettura del file e il conseguente flush del buffer
        // (ovvero l'esecuzione della callback che si occupa dell'invio del buffer al server)
        bfm.run();
        read_done.get_future().get();

        this->cd.send_parameter(STOPFLOW, "");
        this->cd.unlock_raw();

        // viene richiamata la wait_for_response
        return std::async([command, this]() {
            CommandDTO post_file_result = cd.wait_for_response(command).get();
            return post_file_result.find((__RESULT)).second == "OK";
        });
    } catch (BufferedFileReaderException& bfre) {
        std::cerr << "Skipping the post of the following file: " << file_metadata.path << std::endl;
        std::cerr << "The reason is " << bfre.what() << std::endl;
        auto prom = std::promise<bool>();
        prom.set_value(false);
        return prom.get_future();
    } catch (...) {
        std::cerr << "Failed to post file: " << file_metadata.path << std::endl;
        throw;
    }
}

// Funzione che si occupa di richiedere la rimozione di un file sul server e ritorna la risposta del server
std::future<bool> ClientCommand::remove_file(FileMetadata &file_metadata) {
    std::string command;
    CommandDTO parameters;
    parameters.erase();
    command = REMVFILE;
    parameters.insert(std::pair<std::string, std::string>(FILEPATH,file_metadata.path_to_send));
    return std::async([this, command, parameters] () {
        CommandDTO result = cd.dispatch(command, parameters).get();
        return result.find((__RESULT)).second == "OK";
    });
}

// Funzione che si occupa della richiesta di un file dal server e ritorna un bool in base al fatto
// che il file ricevuto sia stato corrotto o  meno
std::future<bool> ClientCommand::require_file(FileMetadata &file_metadata) {
    std::string command;
    CommandDTO parameters;
    parameters.erase();
    command = REQRFILE;
    parameters.insert(FILEPATH, file_metadata.path_to_send);
    cd.lock_raw();
    cd.dispatch_partial(command, parameters);
    cd.send_parameter(STOPFLOW, "");
    cd.unlock_raw();

    std::string path = file_metadata.path;
    std::string prev_hash = file_metadata.hash;
    long size = file_metadata.size;

    return std::async([command, this, path, prev_hash, size]() {
        BufferedFileWriter bfw{path, size};
        std::function<void(char* buffer, int)> flush_buffer_fn = [&bfw](auto data, auto length) {
            bfw.append(data, length);
        };
        CommandDTO reqr_file_result = cd.wait_for_response(command, true, flush_buffer_fn).get();

        bfw.close();
        std::string hash = hash_file(path);

        bool is_corrupted = hash != prev_hash;
        if (is_corrupted) {
            boost::filesystem::remove_all(path);
        }
        return !is_corrupted;
    });
}
