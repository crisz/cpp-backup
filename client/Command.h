#include <string>
#include <future>
#include <map>
#include <functional>
#include "FileMetadata.h"
#include "CommandDispatcher.h"
#include "../common/Constants.h"
#include "../common/BufferFileManager.h"

#define FILE_BUFFER_SIZE 256

class Command {
private:
    std::string command;
    CommandDispatcher cd;
    std::map<std::string, std::string> parameters;

public:

    std::future<bool> login(std::string username, std::string password) {
        std::cout << "Init login " << std::endl;

        parameters.erase(parameters.begin(), parameters.end());
        command = LOGINSNC;
        parameters[USERNAME] = username;
        parameters[PASSWORD] = password;
        return std::async([this]() {
            std::map<std::string, std::string> result = cd.dispatch(command, parameters).get();
            std::cout << "result is " << result[__RESULT] << std::endl;
            return result[__RESULT] == "OK" ? true : false;
        });
    }

    std::future<std::vector<FileMetadata>> require_tree() {
        std::future<std::vector<FileMetadata>> a;
        command = REQRTREE;
        parameters.erase(parameters.begin(), parameters.end());

        cd.dispatch(command, parameters);
        return a;
    }

    std::future<bool> post_file(FileMetadata& file_metadata, const int buffer_size=256) {
        // TODO: gestione degli errori lato server. Cosa succede se il client lascia l'invio a metà?
 
        BufferFileManager bfm{buffer_size, file_metadata.path}; // RAII
        command = POSTFILE;
        parameters.erase(parameters.begin(), parameters.end());
        parameters[FILEPATH] = file_metadata.path;
        parameters[FILEHASH] = file_metadata.hash;

        cd.dispatch_partial(command, parameters);
        cd.send_raw(FILEDATA, sizeof(FILEDATA));
        cd.send_raw(cd.encode_length(bfm.get_file_size()), 4);


        std::promise<bool>& done = bfm.register_callback([&bfm, this] (bool done, char* data, int bytes_read) {
            this->cd.send_raw(data, bytes_read);
            bfm.signal();
            if (done) {
                this->cd.send_parameter("STOPFLOW", "");
            }
        });

        bfm.run();
        
        return done.get_future();
    }

    std::future<bool> remove_file(FileMetadata& file_metadata) {
        std::future<bool> a;
        return a;
    }
};