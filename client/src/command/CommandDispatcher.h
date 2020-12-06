//
// Classe che gestisce la comunicazione con il server
//


#pragma once
#include <string>
#include <map>
#include "client/src/server/ServerConnectionAsio.h"
#include <future>
#include <vector>
#include <mutex>
#include "CommandDTO.h"
#include "common/Constants.h"
#include "common/Constants.h"
#include "common/BufferedFileWriter.h"
#include "common/encode_length_utils.h"
#include <condition_variable>
#define BUFFER_SIZE 1024

struct IncomingCommand {
    std::string command_name;
    CommandDTO parameters;
};
class CommandDispatcher {
private:
    std::shared_ptr<ServerConnectionAsio> sc;
    std::vector<IncomingCommand> pending_commands;
    std::mutex dispatch_mutex;
    std::condition_variable cv;

public:
    CommandDispatcher();

    void dispatch_partial(std::string command, const CommandDTO parameters);

    std::future<CommandDTO> dispatch(std::string command, const CommandDTO& parameters);

    std::future<CommandDTO> wait_for_response(std::string command, bool buffered = false, std::function<void(char* buffer, int)> fn = nullptr);

    void lock_raw();

    void unlock_raw();

    void send_raw(const char* raw_data, int size);

    void send_parameter(std::string key, std::string value);
};