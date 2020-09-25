#include <iostream>
#include <exception>
#include <stdexcept>
#include "FileWatcher.h"
#include <boost/program_options.hpp>
#include "UserSession.h"
#include "ServerConnection.h"
#include <boost/any.hpp>
#include <iostream>

#define SYNCH_INTERVAL 1000

namespace po = boost::program_options;
void die(std::string message) {
    std::cout << message << std::endl;
    exit(-1);
}

void init_file_watcher(FileWatcher &fw, ServerConnection& sc) {
    fw.on_file_changed([&sc](std::string path_to_watch, FileStatus status) -> void {
        if (!boost::filesystem::is_regular_file(boost::filesystem::path(path_to_watch)) && status != FileStatus::erased) {
            return;
        }

        switch (status) {
            case FileStatus::created:
                std::cout << "File created: " << path_to_watch << std::endl;
                sc.send(path_to_watch);
                break;
            case FileStatus::modified:
                std::cout << "File modified: " << path_to_watch << std::endl;
                break;
            case FileStatus::erased:
                std::cout << "File erased: " << path_to_watch << '\n';
                break;
            default:
                std::cout << "Error! Unknown file status.\n";
        }
    });
}

int parse_sync_options(int argc, char** argv, UserSession& us) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "Shows this help menu")
        ("dir", po::value<std::string>()->required(), "The directory you want to sync")
        ("username", po::value<std::string>()->required(), "The username you want to login with")
        ("password", po::value<std::string>()->required(), "The password you want to login with")
        ("address", po::value<std::string>()->default_value("0.0.0.0"), "Address of the server")
        ("port", po::value<int>()->default_value(3333), "Port of the server")
    ;
    po::variables_map vm;
    try {
        auto clp = po::command_line_parser(argc, argv).options(desc);
        po::store(clp.run(), vm);
        po::notify(vm);
        std::string username = vm["username"].as<std::string>();
        std::string password = vm["password"].as<std::string>();
        std::string dir = vm["dir"].as<std::string>();
        std::string address = vm["address"].as<std::string>();
        int port = vm["port"].as<int>();
        us.username = std::move(username);
        us.password = std::move(password);
        us.dir = std::move(dir);
        us.address = std::move(address);
        us.port = std::move(port);
        return 0;
    } catch (po::error& e) {
        if (!vm.count("help")) {
            std::cout << "Comando non valido " << std::endl;
            std::cout << e.what() << std::endl;
        }
        std::cout << desc << std::endl;
        return -1;
    } catch (std::exception& e) {
        std::cout << "Si è verificato un errore: " << std::endl;
        std::cout << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cout << "uncaught" << std::endl;
        std::cout << *std::current_exception << std::endl;
        return -1;
    }
}

int main(int argc, char** argv) {
    if (argc < 1) {
        die("A command between sync and <tbd> is required");
    }
    std::string command = argv[1];

    if (command == "sync") {
        UserSession us;
        if (parse_sync_options(argc, argv, us)) return 0;

        FileWatcher fw{us.dir, std::chrono::milliseconds(SYNCH_INTERVAL)};
        // std::cout << "Your username is " << us.username << std::endl;
        ServerConnection sc{us.address, us.port};
        boost::any var = 12;
        // sc.send();

        init_file_watcher(fw, sc);
    }
}
