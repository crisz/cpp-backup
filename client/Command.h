#include <string>
#include <future>
#include <map>
#include <functional>
#include "FileMetadata.h"
#include "CommandDispatcher.h"

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
        command = "LOGINSNC";
        parameters["USERNAME"] = username;
        parameters["PASSWORD"] = password;
        return std::async([this]() {
            std::map<std::string, std::string> result = cd.dispatch(command, parameters).get();
            std::cout << "result is " << result["__RESULT"] << std::endl;
            return result["__RESULT"] == "OK" ? true : false;
        });
    }

    std::future<std::vector<FileMetadata>> require_tree() {
        std::future<std::vector<FileMetadata>> a;
        return a;
    }

    std::future<bool> post_file(FileMetadata& file_metadata, std::function<char*()> file_fn) {
        std::future<bool> a;
        return a;
    }

    std::future<bool> remove_file(FileMetadata& file_metadata) {
        std::future<bool> a;
        return a;
    }
};