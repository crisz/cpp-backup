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
    std::future<bool> login(std::string username, std::string password);

    std::future<bool> signup(std::string username, std::string password);

    std::future<std::vector<FileMetadata>> require_tree();

    std::future<bool> post_file(FileMetadata& file_metadata, const int buffer_size=256);

    std::future<bool> remove_file(FileMetadata& file_metadata);

    std::future<bool> require_file(FileMetadata& file_metadata);
};

