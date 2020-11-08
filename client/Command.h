#include <string>
#include <future>
#include <map>
#include <functional>
#include "FileMetadata.h"

#define FILE_BUFFER_SIZE 256

class Command {
    std::future<bool> login(std::string username, std::string password) {
        std::future<bool> a;
        return a;
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