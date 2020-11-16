#include <string>
#include <future>
#include <map>
#include <functional>
#include "FileMetadata.h"
#include "CommandDispatcher.h"
#include "../common/Constants.h"
#include "../common/BufferedFileReader.h"
#include <mutex>

#define FILE_BUFFER_SIZE 256

class Command { // TODO: rinominare
private:
    std::string command;
    CommandDispatcher cd;
    std::multimap<std::string, std::string> parameters;

public:
    std::future<bool> login(std::string username, std::string password) {
        std::cout << "Init login " << std::endl;

        std::string command;
        std::multimap<std::string, std::string> parameters;
        parameters.erase(parameters.begin(), parameters.end());
        command = "LOGINSNC";
        parameters.insert(std::pair<std::string, std::string>(USERNAME, username));
        parameters.insert(std::pair<std::string, std::string>(PASSWORD, password));
        return std::async([this, command, parameters] () {
            std::cout << "Trying to dispatch login " << std::endl;
            std::multimap<std::string, std::string> result = cd.dispatch(command, parameters).get();
            std::cout << "result is " << result.find("__RESULT")->second << std::endl;
            return result.find("__RESULT")->second == "OK";
        });
    }

    std::future<std::vector<FileMetadata>> require_tree() {    
        std::string command;
        std::multimap<std::string, std::string> parameters;    
        command = "REQRTREE";
        parameters.erase(parameters.begin(), parameters.end());
        return std::async([this, command, parameters] () {
            std::vector<FileMetadata> tree;
            std::cout << "WAITING FOR REQTREE" << std::endl;
            std::multimap<std::string, std::string> result = cd.dispatch(command, parameters).get();
            std::cout << "REQTREE RETURNED" << std::endl;
            std::cout << result.size() << " elements" << std::endl;
            FileMetadata fm;
            for (std::pair<std::string, std::string> item: result) { // TODO: evitare copia
                std::cout << "item.first = " << item.first << "; item.second = " << item.second << std::endl;
                if (fm.hash != "" && fm.path != "") {
                    tree.push_back(fm);
                    fm.hash.clear();
                    fm.path.clear();
                }
                if (item.first == "FILEHASH") {
                    fm.hash = item.second;
                    continue;
                }
                if (item.first == "FILEPATH") {
                    fm.path = item.second;
                    // fm.name = fm.path.split("/")[-1]; // TODO
                    continue;
                }
            }
            if (fm.hash != "" && fm.path != "") {
                tree.push_back(fm);
            }
            return tree;
        });
    }

    std::future<bool> post_file(FileMetadata& file_metadata, const int buffer_size=256) {
        // TODO: gestione degli errori lato server. Cosa succede se il client lascia l'invio a metà? @Andrea
 
        BufferedFileReader bfm{10, file_metadata.path}; // RAII
        std::string command;
        std::multimap<std::string, std::string> parameters;
        command = "POSTFILE";
        parameters.erase(parameters.begin(), parameters.end());
        parameters.insert(std::pair<std::string, std::string>("FILEPATH", file_metadata.path));
        parameters.insert(std::pair<std::string, std::string>("FILEHASH", file_metadata.hash));
        std::cout << "ACQUIRING LOCK RAW NOW" << std::endl;
        cd.lock_raw();
        cd.dispatch_partial(command, parameters);
        cd.send_raw("FILEDATA", 8);
        cd.send_raw(cd.encode_length(bfm.get_file_size()), 4);

        std::promise<bool>& read_done = bfm.register_callback([&bfm, this] (bool done, char* data, int bytes_read) {
            this->cd.send_raw(data, bytes_read);
            bfm.signal();
            if (done) {
                this->cd.send_parameter("STOPFLOW", "");
                this->cd.unlock_raw();
            }
        });

        bfm.run();
        read_done.get_future().get();

        return std::async([command, this]() {
            std::multimap<std::string, std::string> post_file_result = cd.wait_for_response(command).get();
            return post_file_result.find("__RESULT")->second == "OK";
        });
    }

    std::future<bool> remove_file(FileMetadata& file_metadata) {
        std::future<bool> a;
        return a;
    }
};

