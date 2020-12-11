#include "ClientCommand.h"

ClientCommand::ClientCommand() {
    std::cout << "constr cc " << std::endl;
}

std::future<bool> ClientCommand::login(std::string username, std::string password) {
    std::cout << "Init login " << std::endl;

    std::string command;
    CommandDTO parameters;
    parameters.erase();
    command = "LOGINSNC";
    parameters.insert(std::pair<std::string, std::string>(USERNAME, username));
    parameters.insert(std::pair<std::string, std::string>(PASSWORD, password));
    return std::async([this, command, parameters] () {
        std::cout << "Trying to dispatch login " << std::endl;
        CommandDTO result = cd.dispatch(command, parameters).get();
        std::cout << "result is " << result.find(("__RESULT")).second << std::endl;
        return result.find(("__RESULT")).second == "OK";
    });
}

std::future<bool> ClientCommand::signup(std::string username, std::string password) {
    std::cout << "Init signup " << std::endl;
    std::string command;
    CommandDTO parameters;
    parameters.erase();
    command = "SIGNUPNU";
    parameters.insert(std::pair<std::string, std::string>(USERNAME, username));
    parameters.insert(std::pair<std::string, std::string>(PASSWORD, password));
    return std::async([this, command, parameters] () {
        std::cout << "Trying to dispatch signup " << std::endl;
        CommandDTO result = cd.dispatch(command, parameters).get();
        std::cout << "result is " << result.find(("__RESULT")).second << std::endl;
        return result.find(("__RESULT")).second == "OK";
    });
}

std::future<std::vector<FileMetadata>> ClientCommand::require_tree() {
    std::string command;
    CommandDTO parameters;
    command = "REQRTREE";
    parameters.erase();
    return std::async([this, command, parameters] () {
        std::vector<FileMetadata> tree;
        std::cout << "WAITING FOR REQTREE" << std::endl;
        CommandDTO result = cd.dispatch(command, parameters).get();
        std::cout << "REQTREE RETURNED" << std::endl;
        std::cout << result.size() << " elements" << std::endl;
        FileMetadata fm;
        std::cout << fm.hash.size() << " hash size" << std::endl;

        for (const std::pair< const std::string, std::string>& item: result) {
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
                fm.path_to_send = fm.path;

                std::size_t found = fm.path.find_last_of("/\\");
                fm.name = fm.path.substr(found+1);
                continue;
            }
        }
        if (fm.hash != "" && fm.path != "") {
            tree.push_back(fm);
        }
        return tree;
    });
}

std::future<bool> ClientCommand::post_file(FileMetadata &file_metadata, const int buffer_size) {

    BufferedFileReader bfm{500, file_metadata.path}; // RAII
    std::string command;
    CommandDTO parameters;
    command = "POSTFILE";
    parameters.erase();
    parameters.insert(std::pair<std::string, std::string>("FILEPATH", file_metadata.path_to_send));
    parameters.insert(std::pair<std::string, std::string>("FILEHASH", file_metadata.hash));
    std::cout << "ACQUIRING LOCK RAW NOW" << std::endl;
    cd.lock_raw();
    cd.dispatch_partial(command, parameters);
    cd.send_raw("FILEDATA", 8);

    cd.send_raw(encode_length(bfm.get_file_size()), 4);

    std::promise<bool>& read_done = bfm.register_callback([&bfm, this] (bool done, char* data, int bytes_read) {
        std::cout << "cb: sending raw " << std::endl;
        this->cd.send_raw(data, bytes_read);
        std::cout << "cb: signaling" << std::endl;

        bfm.signal();
        if (done) {
            this->cd.send_parameter("STOPFLOW", "");
            this->cd.unlock_raw();
        }
    });

    bfm.run();
    read_done.get_future().get();

    return std::async([command, this]() {
        CommandDTO post_file_result = cd.wait_for_response(command).get();
        std::cout << "wait for response returnded" << std::endl;
        return post_file_result.find(("__RESULT")).second == "OK";
    });
}

std::future<bool> ClientCommand::remove_file(FileMetadata &file_metadata) {
    std::string command;
    CommandDTO parameters;
    parameters.erase();
    command = "REMVFILE";
    parameters.insert(std::pair<std::string, std::string>(FILEPATH,file_metadata.path_to_send));
    return std::async([this, command, parameters] () {
        std::cout << "Trying to dispatch remove file " << std::endl;
        CommandDTO result = cd.dispatch(command, parameters).get();
        std::cout << "result is " << result.find(("__RESULT")).second << std::endl;
        return result.find(("__RESULT")).second == "OK";
    });
}

std::future<void> ClientCommand::require_file(FileMetadata &file_metadata) {
    std::string command;
    CommandDTO parameters;
    parameters.erase();
    command = "REQRFILE";
    parameters.insert("FILEPATH", file_metadata.path_to_send);
    cd.lock_raw();
    cd.dispatch_partial(command, parameters);
    cd.send_parameter("STOPFLOW", "");
    cd.unlock_raw();

    std::string path = file_metadata.path;
    long size = file_metadata.size;

    return std::async([command, this, path, size]() {
        BufferedFileWriter bfw{path, size};
        std::function<void(char* buffer, int)> flush_buffer_fn = [&bfw](auto data, auto length) {
            std::cout << "Received chunk " << data << std::endl;
            bfw.append(data, length);
        };
        CommandDTO reqr_file_result = cd.wait_for_response(command, true, flush_buffer_fn).get();
        std::cout << "wait for response returnded" << std::endl;
        // return post_file_result.find(("__RESULT")).second == "OK";
    });
}
